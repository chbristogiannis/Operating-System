# Operating System Project1
by Christos Bristogiannis 1115201900129


## Overview
This project is on implemting scheduler in xv6

## Features
- Scheduler on xv6 highest priority and round-robin
- ps.c (pstat.h)

## Kernel

### param.h
- Define constants:
- DEFAULT_PRIORITY 10
- MAX_PRIORITY 20
- MIN_PRIORITY 1

### proc.c
- Setting default value priority
- Implementing a new scheduler with Priority-Based and then Round-Robin

### proc.h
- Adding in struct proc the variable priority

### pstat.h
- Definition of the struct pstat used by the ps.c to retrieve process information.

### syscall.c
- Adding prototypes for the functions that handle system calls.

### syscall.h
- Adding new syscalls
- SYS_setpriority 22
- SYS_getpinfo 23

### sysproc.c
- Adding the implementation of the above fuctions
- uint64 sys_setpriority(void);
- uint64 sys_getpinfo(void);

## User-Level Programs

### ps.c
- A program to report the status of all running processes, utilizing the pstat structure.
- Makes system calls to display process information.

### usys.pl
- Updated to include the new system calls

## Makefile
- Same usage as before
- Adding some user programs in UPROGS