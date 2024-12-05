#include "../include/definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

void parent_process(const char *command_file, const char *text_file) {

    printf("I AM STARTING PARENT\n");

    // Shared memory setup
    int shm_fd;
    size_t shm_size = sizeof(shared_memory_t);  // Use struct size for shared memory segment
    shared_memory_t *shm_addr;

    // Create shared memory segment
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Failed to create shared memory");
        exit(EXIT_FAILURE);
    }

    // Set the size of the shared memory segment
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("Failed to set size of shared memory");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    // Map the shared memory segment into the address space
    shm_addr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_addr == MAP_FAILED) {
        perror("Failed to map shared memory");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    // Semaphore setup
    int max_children = MAX_CHILDREN; // Use value from definitions.h
    sem_t *semaphores[max_children];
    char sem_name[32];

    for (int i = 0; i < max_children; i++) {
        snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_NAME_PREFIX, i);
        semaphores[i] = sem_open(sem_name, O_CREAT | O_RDWR, 0666, 1);
        if (semaphores[i] == SEM_FAILED) {
            perror("Failed to create semaphore");
            // Clean up previously created semaphores
            for (int j = 0; j < i; j++) {
                snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_NAME_PREFIX, j);
                sem_unlink(sem_name);
            }
            shm_unlink(SHM_NAME);
            exit(EXIT_FAILURE);
        }
    }

    // Start child processes
    pid_t pids[max_children];
    for (int i = 0; i < max_children; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("Failed to fork child process");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            // Child process
            child_process(i);
            exit(EXIT_SUCCESS);
        }
    }

    // Start of logic to manage child processes based on commands (left blank)
    // This section would contain code for reading the command file and spawning/terminating child processes.

    // Cleanup
    // Wait for all children to finish
    for (int i = 0; i < max_children; i++) {
        waitpid(pids[i], NULL, 0);
    }
    // // Wait for all children to finish
    // while (wait(NULL) > 0);

    // Close and unlink shared memory
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