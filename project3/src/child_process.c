#include "../include/definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

void child_process(int child_index) {
    printf("child process %d starting\n", child_index);

    // Shared memory setup
    int shm_fd;
    size_t shm_size = sizeof(shared_memory_t);
    shared_memory_t *shm_addr;

    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Failed to open shared memory");
        exit(EXIT_FAILURE);
    }

    shm_addr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_addr == MAP_FAILED) {
        perror("Failed to map shared memory");
        exit(EXIT_FAILURE);
    }

    // Semaphore setup
    char sem_name[32];
    snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_NAME_PREFIX, child_index);
    sem_t *sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED) {
        perror("Failed to open semaphore");
        munmap(shm_addr, shm_size);
        exit(EXIT_FAILURE);
    }

    // Tracking variables
    int message_count = 0;
    time_t start_time = time(NULL);
    time_t end_time;

    // Main loop for receiving messages
    while (1) {
        // Wait until the parent signals that a message is available
        if (sem_wait(sem) == -1) {
            // If interrupted by a signal, continue waiting
            if (errno == EINTR) continue;
            perror("sem_wait failed");
            break;
        }

        // Lock shared memory for safe reading
        if (sem_wait(&shm_addr->lock) == -1) {
            perror("Failed to lock shared memory");
            break;
        }

        // Check the shared memory for the message
        char line[1024];
        strncpy(line, shm_addr->shared_text, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';

        // Unlock shared memory after reading
        if (sem_post(&shm_addr->lock) == -1) {
            perror("Failed to unlock shared memory");
            break;
        }

        // Check if this is a termination message
        // You should define a protocol for termination, for example:
        // if the parent writes "TERMINATE" into shared memory.
        if (strcmp(line, "TERMINATE") == 0) {
            // Parent is requesting termination
            break;
        }

        // Print the received line
        printf("Child %d received: %s", child_index, line);
        fflush(stdout);

        // Increment message count
        message_count++;
    }

    // Calculate total active time
    end_time = time(NULL);
    double active_duration = difftime(end_time, start_time);

    // Report statistics
    printf("Child %d terminating. Messages received: %d, Active time: %.2f seconds\n",
           child_index, message_count, active_duration);

    // Cleanup
    if (sem_close(sem) == -1) {
        perror("Failed to close semaphore");
    }

    if (munmap(shm_addr, shm_size) == -1) {
        perror("Failed to unmap shared memory");
    }

    printf("child process %d ending\n", child_index);
    exit(EXIT_SUCCESS);
}
