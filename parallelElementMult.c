#include "populate.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>  // For O_* constants

#define SEM_NAME "/matrix_elem_sem"

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

// Function to compute a single element of matrix C
void compute_element(int size, double* A, double* B, double* C, int* next_row, int* next_col, sem_t* sem) {
    int row, col;

    while (1) {
        // Synchronize access to the shared variables for element index
        sem_wait(sem);

        // Get the next element indices
        row = *next_row;
        col = *next_col;

        // Update indices for the next element
        if (++(*next_col) >= size) {
            *next_col = 0;
            (*next_row)++;
        }

        // Exit if no more elements to compute
        if (row >= size) {
            sem_post(sem);
            break;
        }

        sem_post(sem);

        // Compute the element of matrix C
        C[row * size + col] = 0;
        for (int k = 0; k < size; k++) {
            C[row * size + col] += A[row * size + k] * B[k * size + col];
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <matrix size> <number of processes>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    int num_processes = atoi(argv[2]);

    // Allocate shared memory for matrices and element indices
    double* A = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double* B = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double* C = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* next_row = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* next_col = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Initialize the element indices
    *next_row = 0;
    *next_col = 0;

    // Populate matrices A and B
    populate(size, A, B);

    // Create a semaphore for synchronizing access to the element indices
    sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);

    // Start the timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Fork child processes
    for (int i = 0; i < num_processes; i++) {
        if (fork() == 0) {
            // Child process
            compute_element(size, A, B, C, next_row, next_col, sem);
            exit(0);
        }
    }

    // Parent process waits for all children to complete
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
    save_matrix_to_file("parallel_element_output.txt", size, C);

    // Clean up
    munmap(A, size * size * sizeof(double));
    munmap(B, size * size * sizeof(double));
    munmap(C, size * size * sizeof(double));
    munmap(next_row, sizeof(int));
    munmap(next_col, sizeof(int));

    // Unlink and close the semaphore
    sem_close(sem);
    sem_unlink(SEM_NAME);

    return 0;
}
