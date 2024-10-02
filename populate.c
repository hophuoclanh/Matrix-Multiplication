#include <stdlib.h>
#include "populate.h"

void populate(int size, double* A, double* B) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            A[i * size + j] = rand();
            B[i * size + j] = rand();
        }
    }
}
