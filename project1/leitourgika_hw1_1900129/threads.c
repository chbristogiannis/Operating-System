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

#include <time.h>


/**
 * Writes a message to a file or prints it if the filename is NULL.
 * 
 * @param message The message to be written or printed.
 * @param filename The name of the file to write to, or NULL to print the message.
 */
void message_handler(char *message, const char *filename) {
    if (filename == NULL) {
        // If filename is NULL, print the message
        printf("Complete message is: %s\n", message);
    } else {
        // Attempt to open the file in append mode
        FILE *file = fopen(filename, "a");
        if (file == NULL) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        // Write the message to the file
        fprintf(file, "%s\n", message);

        // Close the file
        if (fclose(file) != 0) {
            perror("Error closing file");
            exit(EXIT_FAILURE);
        }
    }
}


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
void *send_message(void* args) {
    // Cast the void pointer back to the appropriate type
    ThreadArgs* thread_args = (ThreadArgs*)args;

    // Input handling variables
    char* input = NULL;
    size_t input_size = 0;
    ssize_t line_size;

    // Prompt for user input
    printf("Enter a text (type #BYE# to exit):  \n");

    // Infinite loop for reading and processing input
    while (1) {
        // Read a line from stdin
        line_size = getline(&input, &input_size, stdin);

        // Check for read error
        if (line_size == -1) {
            perror("Error reading input");
            free(input);
            exit(EXIT_FAILURE);
        }

        // Replace newline character with string terminator
        if (input[line_size - 1] == '\n') {
            input[line_size - 1] = '\0';
        }

         // Start index for segmenting the input message
        int start = 0;
        // Flag to indicate exit condition
        int flag_exit = 0;

        // Process and send each segment of the message
        while (start < line_size-1) {
            // Wait for semaphore to become available
            sem_wait(thread_args->sem1);

            // Set the flag for processing
            thread_args->shared_data->flag_proc = thread_args->flag_indicator;
            
            // Signal and wait for semaphores to synchronize with receiver
            sem_post(thread_args->sem4);
            sem_wait(thread_args->sem2);

            // Copy a segment of the message to shared memory
            strncpy(thread_args->shared_data->message, input + start, MAX_MESSAGE_LENGTH);
            thread_args->shared_data->message[MAX_MESSAGE_LENGTH] = '\0';
            // Set flag for last segment
            thread_args->shared_data->flag_last_segment = ((start + MAX_MESSAGE_LENGTH) < line_size-1) ? 0 : 1;

            // Check for exit condition
            if (strcmp(thread_args->shared_data->message, "#BYE#") == 0) {
                thread_args->stats->sent_messages--; // The message that has the string #BYE# doesnt count
                flag_exit = 1;
                sem_post(thread_args->sem3);
                break;
            }

            // Move to the next segment
            start += MAX_MESSAGE_LENGTH;
            sem_post(thread_args->sem3);
        }

        // Increment the count of sent messages
        thread_args->stats->sent_messages++;
        // Exit
        if (flag_exit == 1) {
            break;
        }
    }
}


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
void *receive_message(void* args) {

    // Cast the void pointer back to the appropriate type
    ThreadArgs *thread_args = (ThreadArgs *)args;

    // Buffer for accumulating the complete message
    char *complete_message = NULL;
    // Index for the next character position in complete_message
    int message_index, message_size;

    // Variables for measuring time - currently unused
    struct timespec wait_start, wait_end;
    thread_args->stats->total_waiting_time = 0.0;

    // Main loop to continuously receive messages
    while (1) {
        int flag_exit = 0;
        message_index = 0;
        message_size = 0;
        int flag_new_message = 1; // Flag to indicate the start of a new message

        // Inner loop to construct a complete message from segments
        while(1) {
            // Wait for a message segment to become available
            sem_wait(thread_args->sem4);

            // Check if it's this thread's turn to process the message
            if (thread_args->flag_indicator == thread_args->shared_data->flag_proc) {
                sem_post(thread_args->sem4);
                continue;
            }
            clock_gettime(CLOCK_MONOTONIC, &wait_start);

            // Signal and wait for semaphores to synchronize with sender
            sem_post(thread_args->sem2);
            sem_wait(thread_args->sem3);

            // Get the length of the received message segment
            int segmentLength = strlen(thread_args->shared_data->message);
            // Reallocate the complete_message buffer if necessary
            if (message_index + segmentLength >= message_size) {
                message_size += MAX_MESSAGE_LENGTH;
                char *newBuffer = realloc(complete_message, message_size * sizeof(char));
                if (newBuffer == NULL) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }
                complete_message = newBuffer;
            }

            // Copy the received segment to the complete_message buffer
            strncpy(complete_message + message_index, thread_args->shared_data->message, segmentLength);
            message_index += segmentLength;
            complete_message[message_index] = '\0';

            // Print the received segment for debug purposes
            // printf("Received process: %s\n", thread_args->shared_data->message);
            // Increment the count of total segments
            thread_args->stats->total_segments++;

            // Check if the '#BYE#' message was received to exit
            if (strcmp(thread_args->shared_data->message, "#BYE#") == 0) {
                clock_gettime(CLOCK_MONOTONIC, &wait_end);
                thread_args->stats->total_segments--; // This segment doesnt count
                thread_args->stats->total_messages--; // This message doesnt count
                flag_exit = 1;
                sem_post(thread_args->sem3);
                sem_post(thread_args->sem4);
                // Toggle the processing flag for the next sender
                thread_args->shared_data->flag_proc = thread_args->flag_indicator == 0 ? 0 : 1;
                break;
            }

            // If the message is new, calculate the wait time
            if (flag_new_message) {
                clock_gettime(CLOCK_MONOTONIC, &wait_end);
                thread_args->stats->total_waiting_time += (wait_end.tv_sec - wait_start.tv_sec) +
                                                          (wait_end.tv_nsec - wait_start.tv_nsec) / 1e9;
                flag_new_message = 0; // Set flag for the next new message
            }

            // Check if this is the last segment of the message
            if (thread_args->shared_data->flag_last_segment == 1) {
                // Handle the complete message
                message_handler(complete_message, thread_args->filename);
                // Free the buffer and prepare for the next message
                free(complete_message);
                complete_message = NULL;
                sem_post(thread_args->sem1);
                break;
            }

            // Signal the sender to send the next segment
            sem_post(thread_args->sem1);
        }

        // Increment the count of total messages received
        thread_args->stats->total_messages++;
        if (flag_exit == 1) {
            break;
        }
    }
}