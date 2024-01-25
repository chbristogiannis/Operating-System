# Compiler and Compiler Flags
CC = gcc
CFLAGS = -pthread -lrt

# Object Files
OBJ_A = a.o threads.o
OBJ_B = b.o threads.o

# Executable Names
EXEC_A = a
EXEC_B = b

# Default target
all: $(EXEC_A) $(EXEC_B)

# Compile rules for program A
$(EXEC_A): $(OBJ_A)
	$(CC) -o $@ $^ $(CFLAGS)

# Compile rules for program B
$(EXEC_B): $(OBJ_B)
	$(CC) -o $@ $^ $(CFLAGS)

# Rule for object files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

# Clean target
clean:
	rm -f *.o $(EXEC_A) $(EXEC_B)

# Phony targets
.PHONY: all clean
