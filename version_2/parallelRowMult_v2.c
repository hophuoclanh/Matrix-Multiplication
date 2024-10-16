#include "../populate.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>  // For O_* constants

#define SEM_NAME "/matrix_row_sem"

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

// Function to compute one row of matrix C
void compute_row(int size, double* A, double* B, double* C, int* row_index, sem_t* sem) {
    int row;
    while (1) {
        // Lock the semaphore before accessing shared row index
        sem_wait(sem);
        row = *row_index;
        (*row_index)++;
        // Unlock the semaphore after updating the shared variable
        sem_post(sem);

        // Exit condition: if all rows are processed
        if (row >= size) break;

        // Compute the current row of matrix C
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
    int requested_processes = atoi(argv[2]);

    // Detect the number of available CPU cores
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of CPU cores available: %d\n", num_cores);

    // Limit the number of processes to the available CPU cores
    int num_processes = (requested_processes > num_cores) ? num_cores : requested_processes;
    printf("Using %d processes for parallel row multiplication\n", num_processes);

    // Allocate shared memory for matrices and the shared row index
    double* A = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double* B = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double* C = mmap(NULL, size * size * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* row_index = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Initialize the row index
    *row_index = 0;

    // Populate matrices A and B with random values
    populate(size, A, B);

    // Create a semaphore to synchronize row assignment
    sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);

    // Start the timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Fork child processes to perform row-level parallel multiplication
    for (int i = 0; i < num_processes; i++) {
        if (fork() == 0) {
            // Child process
            compute_row(size, A, B, C, row_index, sem);
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
    save_matrix_to_file("parallel_row_output.txt", size, C);

    // Clean up: Unmap shared memory and unlink semaphore
    munmap(A, size * size * sizeof(double));
    munmap(B, size * size * sizeof(double));
    munmap(C, size * size * sizeof(double));
    munmap(row_index, sizeof(int));
    sem_close(sem);
    sem_unlink(SEM_NAME);

    return 0;
}
