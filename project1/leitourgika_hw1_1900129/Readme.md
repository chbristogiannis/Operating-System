# Operating System Project1
by Christos Bristogiannis 1115201900129


## Overview
This project is a messaging application demonstrating the use of shared memory and semaphores in a multithreaded environment.

## Features
- Multithreaded Design
- Shared Memory IPC
- Semaphore Synchronization

## Makefile Explanation
Our project uses a Makefile for efficient compilation of the source code. Here's an overview of how the Makefile is structured and what each part does:

### Compiler and Flags
- CC = gcc: This specifies that the GNU Compiler Collection (GCC) is used for compiling the C files.
- CFLAGS = -pthread -lrt: These are the compiler flags used when compiling the source files. The -pthread flag is used for compiling with the POSIX threads library, and -lrt is for the real-time extensions library, often required for time-related functions and POSIX IPC.

### Object Files
- OBJ_A = a.o threads.o: Defines the object files for program A. These are the compiled versions of the source files.
- OBJ_B = b.o threads.o: Defines the object files for program B.

### Executable Names
- EXEC_A = a: The name of the executable that will be created for program A.
- EXEC_B = b: The name of the executable for program B.

### Compilation Rules
- all: $(EXEC_A) $(EXEC_B): The default target that is built when you run make without arguments. It will build both executables.
- $(EXEC_A): $(OBJ_A): Specifies how to build the first executable. It depends on the object files listed in OBJ_A.
- $(EXEC_B): $(OBJ_B): Similar to the above, but for the second executable, depending on OBJ_B.
- Each executable is compiled with the command $(CC) -o $@ $^ $(CFLAGS), where $@ is the target (executable name), $^ are the dependencies (object files), and $(CFLAGS) are the compiler flags.

### Pattern Rule for Object Files
- %.o: %.c: A pattern rule for creating .o (object) files from .c (source) files. This rule states that each .o file depends on a .c file with the same name.

### Cleaning Up
- clean: This target is for cleaning up the build directory. It removes all object files and executables, keeping your directory clean. You can invoke this command using make clean.
- rm -f *.o $(EXEC_A) $(EXEC_B): The command to remove the object files and executables.

### Phony Targets
- .PHONY: all clean: This line tells make that all and clean are not files but rather commands. It's a standard practice to prevent conflicts with files of the same name and to ensure that the targets are always executed.

### Usage
- To compile the project, simply run: make
- To clean up the build (remove all compiled files), use: make clean



## threads.h - Header File Explanation
The threads.h file is a crucial part of our project, acting as the central header file that defines structures and function prototypes used across the application. Here's a breakdown of its contents:

### Constants
- MAX_MESSAGE_LENGTH: Defines the maximum length of a message segment that can be handled by the application.

### Structures
1. SharedMemory:
    - Used for storing message segments in shared memory.
    - message: A character array to hold a segment of the message.
    - flag_last_segment: Indicates if the current segment is the last part of the message.
    - flag_proc: Indicates which process is currently writing to the shared memory.

2. ThreadStats:
    - Used for tracking statistical data regarding message processing.
    - sent_messages: The count of messages sent.
    - total_messages: The total number of messages processed.
    - total_segments: The total number of message segments processed.
    - total_waiting_time: The cumulative waiting time experienced during processing.

3. ThreadArgs:
    - Used to pass arguments to thread routines.
    - shared_data: Pointer to SharedMemory.
    - sem1, sem2, sem3, sem4: Pointers to semaphores for synchronization.
    - stats: Pointer to ThreadStats for tracking thread-specific statistics.
    - flag_indicator: Additional flag for use in thread processing
    - filename: A pointer to a filename string used in message handling.

### Function Prototypes
- void message_handler(char *message, const char *filename): Handles messages by writing them to a file or printing them based on the filename argument.
- void *send_message(void* args): Thread function for sending messages. It uses ThreadArgs to access shared resources and synchronization mechanisms.
- void *receive_message(void* args): Thread function for receiving and processing messages. Similar to send_message, it uses ThreadArgs for its operation.


## threads.c - Thread Routines

### send_message(void* args)

1. Purpose: This function is a thread routine for sending messages. It reads user input from the standard input, segments it, and sends each segment through shared memory.
2. Process:
- Reads input line by line.
- Segments the input into predefined sizes (MAX_MESSAGE_LENGTH).
- Uses semaphores (sem1, sem2, sem3, sem4) for synchronization with the receiving thread.
- The sending process continues until the user inputs #BYE#.
- Keeps track of the number of messages sent.

### receive_message(void* args)

1. Purpose: This function serves as a thread routine for receiving messages. It constructs complete messages from segments received via shared memory.
2. Process:
- Waits for message segments to be available.
- Reconstructs the complete message from segments.
- Uses semaphores for synchronization with the sending thread.
- The receiving process continues until #BYE# is received.
- Records the number of messages and segments received, as well as total waiting time.

### Synchronization and Shared Data and #BYE#
The Synchronization and access to the Shared Data happens in 2 different places. The first time happens when the sender informs the share data that he is going to write the message and then it waits for the proper receiver to be available to read the message. In case the wrong receiver gains access it restart the proccess of which receiver is going to gain access. After that they communicate together to send and receive the message. In case of the command #BYE# the receiver allows the other receiver to see also the command #BYE# and they both terminate and they cancel their receiver.

### message_handler(char *message, const char *filename)
1. Purpose: Writes a message to a file or prints it, depending on whether a filename is provided.
2. Process:
- If filename is NULL, the message is printed to the standard output.
- If filename is provided, the message is appended to the specified file.


## a.c - Process Overview for a.c
### Initialization:
- Sets up a shared memory object for storing message data.
- Initializes four semaphores to control access and synchronization between threads.

### Thread Creation and Execution:
- Creates two threads:
- send_message: Responsible for reading user input, segmenting it, and sending it to the shared memory.
- receive_message: Waits for message segments, reconstructs them into complete messages, and optionally writes them to a file or standard output.
- The threads communicate via shared memory, with synchronization handled by semaphores.

### User Interaction:
- The user is prompted to enter text, which is then processed by the send_message thread.
- The process continues until the user enters #BYE#, signaling the end of input.

### Termination and Cleanup:
- Once #BYE# is received, the receive_message thread completes its operation and exits.
- The send_message thread is then cancelled.
- The program prints statistics about the messages processed.
- Cleans up resources by closing and unlinking semaphores, unmapping, and unlinking the shared memory.

## b.c - Process Overview for b.c

### Shared Memory and Semaphore Initialization:
- Connects to an existing shared memory object for message storage.
- Opens existing semaphores to synchronize with the counterpart program.

### Thread Operation:
- Creates two threads:
- Sender (send_message): Reads and sends user input to shared memory.
- Receiver (receive_message): Receives message segments, assembles them, and outputs complete messages.
- Threads communicate via shared memory, with synchronization managed by semaphores.

### Termination and Resource Cleanup:
- Waits for the receiver thread to finish.
- Cancels the sender thread upon completion of the receiver.
- Prints statistics about message processing.
- Closes semaphores and unmaps the shared memory object.