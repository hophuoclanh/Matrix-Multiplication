#include "../populate.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>  // For detecting CPU cores

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

// Sequential matrix multiplication
void sequential_mult(int size, double* A, double* B, double* C) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            C[i * size + j] = 0;
            for (int k = 0; k < size; k++) {
                C[i * size + j] += A[i * size + k] * B[k * size + j];
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <matrix size>\n", argv[0]);
        return 1;
    }

    // Get matrix size from command line argument
    int size = atoi(argv[1]);

    // Detect the number of available CPU cores
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of CPU cores available: %d\n", num_cores);

    // Allocate memory for matrices A, B, and C
    double *A = malloc(size * size * sizeof(double));
    double *B = malloc(size * size * sizeof(double));
    double *C = malloc(size * size * sizeof(double));

    // Populate matrices A and B with random values
    populate(size, A, B);

    // Measure time taken for sequential multiplication
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Perform sequential matrix multiplication
    sequential_mult(size, A, B, C);

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = end.tv_usec - start.tv_usec;
    double elapsed = seconds + micros*1e-6;

    printf("Execution Time: %f seconds\n", elapsed);

    // Save result matrix C to a file
    save_matrix_to_file("sequential_output.txt", size, C);

    // Free allocated memory
    free(A);
    free(B);
    free(C);

    return 0;
}
