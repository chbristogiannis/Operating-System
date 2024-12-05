#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <semaphore.h>

#define MAX_CHILDREN 5
#define SHM_NAME "/shared_memory"
#define SEM_NAME_PREFIX "/sem_"

// Shared memory structure
typedef struct {
    int active_children;    // Number of active child processes
    char shared_text[1024]; // Buffer for sharing text between parent and children
    sem_t lock;             // Semaphore for synchronizing access to shared memory
} shared_memory_t;


// Functions for parent and child processes
void parent_process(const char *command_file, const char *text_file);
void child_process(int child_index);

#endif // DEFINITIONS_H