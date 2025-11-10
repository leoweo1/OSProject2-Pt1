// Daniel Onwuka
// Project 2 - Operating Systems
// Fall 2025

#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

// Shared memory structure
typedef struct {
    int bank_account;
    sem_t mutex;
    sem_t dad_deposit;
    sem_t student_withdraw;
} shared_data_t;

// Function prototypes
void dad_process(shared_data_t *shared);
void student_process(shared_data_t *shared);
void mom_process(shared_data_t *shared);
int get_random(int min, int max);

// Global variable for signal handling
volatile sig_atomic_t keep_running = 1;

void signal_handler(int sig) {
    keep_running = 0;
}

int main(int argc, char **argv) {
    int num_parents = 1;  // Default: only Dad
    int num_students = 1; // Default: one student
    
    // Parse command line arguments
    if (argc >= 3) {
        num_parents = atoi(argv[1]);
        num_students = atoi(argv[2]);
    }
    
    printf("Starting with %d parent(s) and %d student(s)\n", num_parents, num_students);
    
    // Set up signal handler for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create shared memory
    int fd = open("bank_account.dat", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    
    // Set size of shared memory
    ftruncate(fd, sizeof(shared_data_t));
    
    // Map shared memory
    shared_data_t *shared = mmap(NULL, sizeof(shared_data_t), 
                                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    close(fd);
    
    // Initialize shared data
    shared->bank_account = 0;
    
    // Initialize semaphores
    if (sem_init(&shared->mutex, 1, 1) == -1) {
        perror("sem_init mutex");
        exit(1);
    }
    
    // Create processes
    pid_t pid;
    int i;
    
    // Create parent processes (Dad and/or Mom)
    for (i = 0; i < num_parents; i++) {
        pid = fork();
        if (pid == 0) {
            if (i == 0) {
                // First parent is always Dad
                dad_process(shared);
            } else {
                // Additional parents are Mom
                mom_process(shared);
            }
            exit(0);
        } else if (pid < 0) {
            perror("fork");
            exit(1);
        }
    }
    
    // Create student processes
    for (i = 0; i < num_students; i++) {
        pid = fork();
        if (pid == 0) {
            student_process(shared);
            exit(0);
        } else if (pid < 0) {
            perror("fork");
            exit(1);
        }
    }
    
    // Parent process waits for signal to terminate children
    while (keep_running) {
        sleep(1);
    }
    
    printf("\nTerminating processes...\n");
    
    // Kill all child processes
    kill(0, SIGTERM);
    
    // Wait for all children to terminate
    for (i = 0; i < num_parents + num_students; i++) {
        wait(NULL);
    }
    
    // Cleanup
    sem_destroy(&shared->mutex);
    munmap(shared, sizeof(shared_data_t));
    remove("bank_account.dat");
    
    printf("Program terminated successfully.\n");
    return 0;
}

void dad_process(shared_data_t *shared) {
    int local_balance;
    int amount;
    
    srand(time(NULL) ^ getpid());
    
    while (keep_running) {
        // Sleep random time 0-5 seconds
        sleep(get_random(0, 5));
        
        printf("Dear Old Dad: Attempting to Check Balance\n");
        
        // Generate random decision
        int decision = get_random(0, 99);
        
        sem_wait(&shared->mutex);
        local_balance = shared->bank_account;
        
        if (decision % 2 == 0) { // Even - consider deposit
            if (local_balance < 100) {
                // Deposit money
                amount = get_random(0, 99);
                if (amount % 2 == 0) { // Even - deposit
                    amount = get_random(0, 100);
                    local_balance += amount;
                    shared->bank_account = local_balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, local_balance);
                } else { // Odd - no deposit
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", local_balance);
            }
        } else { // Odd - just check balance
            printf("Dear Old Dad: Last Checking Balance = $%d\n", local_balance);
        }
        
        sem_post(&shared->mutex);
    }
}

void mom_process(shared_data_t *shared) {
    int local_balance;
    int amount;
    
    srand(time(NULL) ^ getpid());
    
    while (keep_running) {
        // Sleep random time 0-10 seconds
        sleep(get_random(0, 10));
        
        printf("Lovable Mom: Attempting to Check Balance\n");
        
        sem_wait(&shared->mutex);
        local_balance = shared->bank_account;
        
        if (local_balance <= 100) {
            // Always deposit if balance is low
            amount = get_random(50, 125);
            local_balance += amount;
            shared->bank_account = local_balance;
            printf("Lovable Mom: Deposits $%d / Balance = $%d\n", amount, local_balance);
        } else {
            printf("Lovable Mom: Balance is sufficient ($%d)\n", local_balance);
        }
        
        sem_post(&shared->mutex);
    }
}

void student_process(shared_data_t *shared) {
    int local_balance;
    int need;
    
    srand(time(NULL) ^ getpid());
    
    while (keep_running) {
        // Sleep random time 0-5 seconds
        sleep(get_random(0, 5));
        
        printf("Poor Student: Attempting to Check Balance\n");
        
        // Generate random decision
        int decision = get_random(0, 99);
        
        sem_wait(&shared->mutex);
        local_balance = shared->bank_account;
        
        if (decision % 2 == 0) { // Even - attempt withdrawal
            need = get_random(0, 50);
            printf("Poor Student needs $%d\n", need);
            
            if (need <= local_balance) {
                local_balance -= need;
                shared->bank_account = local_balance;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, local_balance);
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", local_balance);
            }
        } else { // Odd - just check balance
            printf("Poor Student: Last Checking Balance = $%d\n", local_balance);
        }
        
        sem_post(&shared->mutex);
    }
}

int get_random(int min, int max) {
    return min + rand() % (max - min + 1);
}
