#include "../populate.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>  // For O_* constants

#define SEM_NAME "/matrix_sem"

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

void printm(int size, double* C) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%f ", C[i * size + j]);
        }
        printf("\n");
    }
}

void compute_row(int size, double* A, double* B, double* C, int* row_index, sem_t* sem) {
    int row;
    while (1) {
        // Lock the semaphore before accessing the shared variable
        sem_wait(sem);
        row = *row_index;
        (*row_index)++;
        // Unlock the semaphore after updating the shared variable
        sem_post(sem);

        if (row >= size) break; // No more rows to compute

        // Compute the row of matrix C
        for (int j = 0; j < size; j++) {
            C[row * size + j] = 0;
            for (int k = 0; k < size; k++) {
                C[row * size + j] += A[row * size + k] * B[k * size + j];
            }
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

    // Allocate shared memory for matrices and row index
    double* A = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double* B = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double* C = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* row_index = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Initialize the row index
    *row_index = 0;

    // Populate matrices A and B
    populate(size, A, B);

    // Create a semaphore for synchronizing access to row_index
    sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);

    // Start the timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Fork child processes
    for (int i = 0; i < num_processes; i++) {
        if (fork() == 0) {
            // Child process
            compute_row(size, A, B, C, row_index, sem);
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
    save_matrix_to_file("parallel_output.txt", size, C);

    // Clean up
    munmap(A, size * size * sizeof(double));
    munmap(B, size * size * sizeof(double));
    munmap(C, size * size * sizeof(double));
    munmap(row_index, sizeof(int));

    // Unlink and close the semaphore
    sem_close(sem);
    sem_unlink(SEM_NAME);

    return 0;
}
