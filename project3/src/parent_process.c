#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "../include/definitions.h"

void parent_process(const char *command_file, const char *text_file) {
    printf("I AM STARTING PARENT\n");

    // Seed the random number generator (for line selection)
    srand((unsigned) time(NULL));

    // Shared memory setup
    int shm_fd;
    size_t shm_size = sizeof(shared_memory_t);  
    shared_memory_t *shm_addr;

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Failed to create shared memory");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("Failed to set size of shared memory");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    shm_addr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_addr == MAP_FAILED) {
        perror("Failed to map shared memory");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory contents
    shm_addr->active_children = 0;
    shm_addr->shared_text[0] = '\0';
    if (sem_init(&shm_addr->lock, 1, 1) == -1) {
        perror("Failed to initialize lock semaphore in shared memory");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    // Semaphore setup for children
    int max_children = MAX_CHILDREN;
    sem_t *semaphores[max_children];
    char sem_name[32];

    for (int i = 0; i < max_children; i++) {
        snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_NAME_PREFIX, i);
        // Start each child semaphore locked, they'll wait for a signal to print
        semaphores[i] = sem_open(sem_name, O_CREAT | O_RDWR, 0666, 0);
        if (semaphores[i] == SEM_FAILED) {
            perror("Failed to create semaphore");
            // Clean up in case of failure
            for (int j = 0; j < i; j++) {
                snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_NAME_PREFIX, j);
                sem_unlink(sem_name);
            }
            shm_unlink(SHM_NAME);
            exit(EXIT_FAILURE);
        }
    }

    // Read instructions from the command file
    char com[256] = "./test_files/config_3_100.txt";
    FILE *cf = fopen(com, "r");
    if (!cf) {
        perror("Failed to open command file");
        exit(EXIT_FAILURE);
    }

    instruction_t instructions[1000];
    int total_instructions = 0;
    while (fscanf(cf, "%d %s %c", &instructions[total_instructions].timestamp,
                  instructions[total_instructions].process_label,
                  &instructions[total_instructions].command) == 3) {
        total_instructions++;
    }
    fclose(cf);

    // Read all lines from the text file into memory for random selection
    char txtFile[256] = "./test_files/mobydick.txt";
    FILE *tf = fopen(txtFile, "r");
    if (!tf) {
        perror("Failed to open text file");
        exit(EXIT_FAILURE);
    }

    char *lines[1000];
    int total_lines = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), tf)) {
        lines[total_lines] = strdup(buffer);
        total_lines++;
    }
    fclose(tf);

    // Array to keep track of children pids (initially no children active)
    pid_t pids[max_children];
    for (int i = 0; i < max_children; i++) pids[i] = -1;

    int current_time = 0;
    int next_instruction_index = 0;

    // Main loop: run until all instructions processed and no children remain
    while (1) {
        // Execute instructions for the current timestamp
        while (next_instruction_index < total_instructions && 
               instructions[next_instruction_index].timestamp == current_time) {
            instruction_t *inst = &instructions[next_instruction_index];

            if (inst->command == 'S') {
                // SPAWN a new child if possible
                for (int i = 0; i < max_children; i++) {
                    if (pids[i] == -1) {
                        pid_t cpid = fork();
                        if (cpid == 0) {
                            // Child
                            child_process(i);
                            exit(EXIT_SUCCESS);
                        } else if (cpid > 0) {
                            pids[i] = cpid;
                            shm_addr->active_children++;
                            break;
                        } else {
                            perror("Failed to fork new child");
                        }
                    }
                }
            } else if (inst->command == 'T') {
                // TERMINATE a specific child based on process_label
                // Assume process_label = "Ck" where k is index+1
                int child_index = atoi(inst->process_label + 1) - 1;
                if (child_index >= 0 && child_index < max_children && pids[child_index] > 0) {
                    // Send a signal or a special message
                    // For simplicity here, just send SIGTERM
                    kill(pids[child_index], SIGTERM);
                }
            }

            next_instruction_index++;
        }

        // If there are active children, send them a random line
        if (shm_addr->active_children > 0) {
            // Gather all active children
            int active_child_indices[MAX_CHILDREN];
            int count_active = 0;
            for (int i = 0; i < max_children; i++) {
                if (pids[i] > 0) {
                    // Check if the child has terminated asynchronously
                    int status;
                    pid_t res = waitpid(pids[i], &status, WNOHANG);
                    if (res == pids[i]) {
                        // Child exited
                        pids[i] = -1;
                        shm_addr->active_children--;
                    } else {
                        // Still active
                        active_child_indices[count_active++] = i;
                    }
                }
            }

            // If still any active children remain after checking
            if (count_active > 0) {
                int chosen_index = active_child_indices[rand() % count_active];
                // Write a random line to shared memory
                sem_wait(&shm_addr->lock);
                int line_idx = rand() % total_lines;
                strncpy(shm_addr->shared_text, lines[line_idx], sizeof(shm_addr->shared_text));
                sem_post(&shm_addr->lock);

                // Signal the child's semaphore so it can read and print
                sem_post(semaphores[chosen_index]);
            }
        }

        // Recheck if all children are done and no more instructions left
        int children_active = 0;
        for (int i = 0; i < max_children; i++) {
            if (pids[i] > 0) {
                int status;
                pid_t res = waitpid(pids[i], &status, WNOHANG);
                if (res == pids[i]) {
                    // Child exited
                    pids[i] = -1;
                    shm_addr->active_children--;
                } else {
                    children_active++;
                }
            }
        }

        if (next_instruction_index >= total_instructions && children_active == 0) {
            // No more instructions and no active children
            break;
        }

        current_time++;
        usleep(100000);
    }

    // Cleanup: free memory
    for (int i = 0; i < total_lines; i++) {
        free(lines[i]);
    }

    // Cleanup shared memory
    if (munmap(shm_addr, shm_size) == -1) {
        perror("Failed to unmap shared memory");
    }
    if (shm_unlink(SHM_NAME) == -1) {
        perror("Failed to unlink shared memory");
    }

    // Close and unlink semaphores
    for (int i = 0; i < max_children; i++) {
        if (sem_close(semaphores[i]) == -1) {
            perror("Failed to close semaphore");
        }
        snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_NAME_PREFIX, i);
        if (sem_unlink(sem_name) == -1) {
            perror("Failed to unlink semaphore");
        }
    }

    printf("I AM ENDING PARENT\n");
}