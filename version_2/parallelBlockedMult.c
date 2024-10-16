#include "../populate.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>  // For O_* constants

#define SEM_NAME "/matrix_block_sem"

// Function to save matrix to file
void save_matrix_to_file(const char* filename, int size, double* C) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not open file %s for writing.\n", filename);
        exit(1);
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%f ", C[i * size + j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

// Function to compute a block of matrix C
void compute_block(int size, int block_size, double* A, double* B, double* C, int* next_block, sem_t* sem) {
    int block;
    while (1) {
        // Synchronize access to shared block index
        sem_wait(sem);
        block = *next_block;
        (*next_block)++;
        sem_post(sem);

        // Exit condition: if all blocks are processed
        if (block >= (size / block_size) * (size / block_size)) break;

        // Compute the block coordinates (i, j)
        int block_i = block / (size / block_size);
        int block_j = block % (size / block_size);

        // Compute each element inside the block
        for (int i = block_i * block_size; i < (block_i + 1) * block_size; i++) {
            for (int j = block_j * block_size; j < (block_j + 1) * block_size; j++) {
                C[i * size + j] = 0;
                for (int k = 0; k < size; k++) {
                    C[i * size + j] += A[i * size + k] * B[k * size + j];
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <matrix size> <block size> <number of processes>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    int block_size = atoi(argv[2]);
    int requested_processes = atoi(argv[3]);

    // Detect the number of available CPU cores
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of CPU cores available: %d\n", num_cores);

    // Limit the number of processes to the available CPU cores
    int num_processes = (requested_processes > num_cores) ? num_cores : requested_processes;
    printf("Using %d processes for parallel blocked matrix multiplication\n", num_processes);

    // Allocate shared memory for matrices and block index
    double* A = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double* B = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double* C = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* next_block = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Initialize the block index
    *next_block = 0;

    // Populate matrices A and B with random values
    populate(size, A, B);

    // Create a semaphore to synchronize block assignment
    sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);

    // Start the timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Fork child processes to perform block-level parallel multiplication
    for (int i = 0; i < num_processes; i++) {
        if (fork() == 0) {
            // Child process
            compute_block(size, block_size, A, B, C, next_block, sem);
            exit(0);
        }
    }

    // Parent process waits for all child processes to complete
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    // Stop the timer
    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = end.tv_usec - start.tv_usec;
    double elapsed = seconds + micros * 1e-6;

    // Print the execution time
    printf("Execution Time: %f seconds\n", elapsed);

    // Save matrix C to file
    save_matrix_to_file("parallel_block_output.txt", size, C);

    // Clean up: Unmap shared memory and unlink semaphore
    munmap(A, size * size * sizeof(double));
    munmap(B, size * size * sizeof(double));
    munmap(C, size * size * sizeof(double));
    munmap(next_block, sizeof(int));
    sem_close(sem);
    sem_unlink(SEM_NAME);

    return 0;
}
