#pragma once

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define MAX_MESSAGE_LENGTH 15

// Structure for shared memory
typedef struct {
    char message[MAX_MESSAGE_LENGTH]; // Buffer to hold a message segment
    int flag_last_segment;  // Flag indicating if this is the last segment of a message (1 for true, 0 for false)
    int flag_proc;          // Flag indicating what process has write in share memory
} SharedMemory;

// Structure for statistical data
typedef struct {
    int sent_messages;       // Count of messages sent
    int total_messages;      // Count of total messages processed
    int total_segments;      // Count of total message segments processed
    double total_waiting_time; // Total waiting time
} ThreadStats;

// Structure for thread arguments
typedef struct {
    SharedMemory* shared_data; // Pointer to shared memory structure
    sem_t* sem1;               // Pointers to semaphores used for synchronization
    sem_t* sem2;
    sem_t* sem3;
    sem_t* sem4;
    ThreadStats* stats;        // Pointer to statistics structure
    int flag_indicator;        // Additional flag for use in thread processing
    char* filename;             // Add this line to include the filename
} ThreadArgs;


/**
 * Writes a message to a file or prints it if the filename is NULL.
 * 
 * @param message The message to be written or printed.
 * @param filename The name of the file to write to, or NULL to print the message.
 */
void message_handler(char *message, const char *filename) ;

/**
 * This function is intended to be used as a thread routine for sending messages.
 * It reads user input and sends it in segments using shared memory and synchronization
 * mechanisms like semaphores. It continues until '#BYE#' is entered.
 *
 * @param args A pointer to a user-defined ThreadArgs structure that contains 
 * the shared data, semaphores for synchronization, and other necessary information.
 *
 * @return This function returns Information using ThreadStats inside the args
 */
void *send_message(void* args);

/**
 * This function is intended to be used as a thread routine for receiving messages.
 * It constructs messages from segments received via shared memory and synchronized
 * with semaphores. It continues until '#BYE#' is received.
 *
 * @param args A pointer to a user-defined ThreadArgs structure that contains 
 * the shared data, semaphores for synchronization, and other necessary information.
 *
 * @return This function returns Information using ThreadStats inside the args
 */
void *receive_message(void* args);