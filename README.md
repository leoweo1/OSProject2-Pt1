# Poor Student - Dear Old Dad Problem

**Daniel Onwuka**  
**Fall 2025**  
**Operating Systems Project Part 1**

## Description
A solution to the IPC synchronization problem implementing the "Poor Student - Dear Old Dad" scenario using semaphores in C. The program demonstrates process synchronization between parent processes (Dad/Mom) depositing money and child processes (Students) withdrawing money from a shared bank account.

## Features
- Mutual exclusion using semaphores
- Multiple parent and child processes
- Random deposit/withdrawal logic  
- Graceful signal handling
- Extra credit: Support for Lovable Mom and multiple students

## Compilation
```bash
gcc -o psdd psdd.c -lpthread
```
## Usage
bash

## Basic: 1 Dad + 1 Student
./psdd 1 1

## Extra credit: Mom + Dad + 3 Students
./psdd 2 3

## Files
psdd.c - Main solution implementation

example.c - Reference semaphore example

shm_processes.c - Shared memory example
