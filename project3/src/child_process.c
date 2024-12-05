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

    printf("child process starting\n");

    // Shared memory setup
    int shm_fd;
    size_t shm_size = sizeof(shared_memory_t);  // Use struct size for shared memory segment
    shared_memory_t *shm_addr;

    // Open the shared memory segment
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Failed to open shared memory");
        exit(EXIT_FAILURE);
    }

    // Map the shared memory segment into the address space
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

    // Start of child process logic (left blank)
    // This section would contain code for receiving messages from the parent process
    // and printing lines from the shared memory, as well as handling termination.

    // Cleanup
    // Close semaphore
    if (sem_close(sem) == -1) {
        perror("Failed to close semaphore");
    }

    // Unmap shared memory
    if (munmap(shm_addr, shm_size) == -1) {
        perror("Failed to unmap shared memory");
    }

    printf("child process ending\n");
}
