#include "populate.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <math.h>

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
        printf("Usage: %s <size>\n", argv[0]);
        return 1;
    }
    
    int size = atoi(argv[1]);
    double *A = malloc(size * size * sizeof(double));
    double *B = malloc(size * size * sizeof(double));
    double *C = malloc(size * size * sizeof(double));
    
    populate(size, A, B);
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    sequential_mult(size, A, B, C);
    
    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = end.tv_usec - start.tv_usec;
    double elapsed = seconds + micros*1e-6;
    
    printf("Execution Time: %f seconds\n", elapsed);

    // Save matrix C to file
    save_matrix_to_file("sequential_output.txt", size, C);
    
    free(A);
    free(B);
    free(C);
    
    return 0;
}
