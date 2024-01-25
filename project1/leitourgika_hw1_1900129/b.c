#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <sys/stat.h>        // For mode constants
#include <semaphore.h>

#include <pthread.h>

#include <string.h>

#include "threads.h"


int main(int argc, char *argv[]) {

    // Size the shared memory object
    int shm_fd;
    const char* shared_memory_name = "/shared_memory";

    // Open the shared memory object
    shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    // Map the shared memory object
    SharedMemory* shared_memory = (SharedMemory*)mmap(0, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    // Semaphore variables
    sem_t *sem1, *sem2, *sem3, *sem4;
    const char *sem_name1 = "/semaphore1";
    const char *sem_name2 = "/semaphore2";
    const char *sem_name3 = "/semaphore3";
    const char *sem_name4 = "/semaphore4";

    // Open existing semaphores
    sem1 = sem_open(sem_name1, 0); // Open existing semaphore
    if (sem1 == SEM_FAILED) {
        perror("sem_open failed for sem1");
        exit(EXIT_FAILURE);
    }

    sem2 = sem_open(sem_name2, 0); // Open existing semaphore
    if (sem2 == SEM_FAILED) {
        perror("sem_open failed for sem2");
        exit(EXIT_FAILURE);
    }

    sem3 = sem_open(sem_name3, 0); // Open existing semaphore
    if (sem2 == SEM_FAILED) {
        perror("sem_open failed for sem3");
        exit(EXIT_FAILURE);
    }

    sem4 = sem_open(sem_name4, 0); // Open existing semaphore
    if (sem4 == SEM_FAILED) {
        perror("sem_open failed for sem4");
        exit(EXIT_FAILURE);
    }


    // Create thread args
    ThreadArgs args;
    args.flag_indicator = 1;
    args.shared_data = shared_memory;
    args.sem1 = sem1;
    args.sem2 = sem2;
    args.sem3 = sem3;
    args.sem4 = sem4;
    // Check if a filename is provided as a command line argument
    if (argc > 1) {
        args.filename = argv[1];
    } else {
        args.filename = NULL; // No filename provided
    }

    // Allocate memory for ThreadStats to return
    ThreadStats stats;
    stats.sent_messages = 0;
    stats.total_messages = 0;
    stats.total_segments = 0;
    stats.total_waiting_time = 0.0;
    args.stats = &stats;

    // Create threads
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, send_message, (void *)&args);
    pthread_create(&thread2, NULL, receive_message, (void *)&args);

    // Wait for threads to complete
    pthread_join(thread2, NULL);

    // After thread2 has completed, cancel thread1
    pthread_cancel(thread1);

    // Process A print stats
    printf("Process B stats sent messages are: %d\n", stats.sent_messages);
    printf("Process B stats total_messages are: %d\n", stats.total_messages);
    printf("Process B stats total_segments are: %d\n", stats.total_segments);
    printf("Process B stats total_waiting_time are: %f\n", stats.total_waiting_time);

    // Close semaphores
    sem_close(sem1);
    sem_close(sem2);
    sem_close(sem3);
    sem_close(sem4);

    // Unmap the shared memory struct
    munmap(shared_memory, sizeof(SharedMemory));

    return 0;
}