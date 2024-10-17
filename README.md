# Matrix Multiplication in Parallel and Sequential Modes

This project demonstrates matrix multiplication using different approaches: sequential computation, parallel computation at the row level, parallel computation at the element level, and parallel computation with block-based optimization. The project is implemented in C and uses techniques such as `fork()`, semaphores for synchronization, and memory-mapped files (`mmap()`) for shared memory.

## Table of Contents
1. [Project Structure](#project-structure)
2. [Installation and Compilation](#installation-and-compilation)
3. [Usage](#usage)
4. [Algorithms and Implementations](#algorithms-and-implementations)
5. [Results and Analysis](#results-and-analysis)

## Project Structure

The project has the following structure:

```
Matrix-Multiplication/
├── version_1/
│   ├── parallelElementMult.c
│   ├── parallelRowMult.c
│   ├── sequentialMult.c
├── version_2/
│   ├── parallelBlockedMult.c
├── populate.c
├── populate.h
└── README.md
```

- **version_1**: Contains different versions of the matrix multiplication algorithm: sequential, parallel (element level), and parallel (row level).
- **version_2**: Contains the parallel blocked matrix multiplication implementation.
- **populate.c / populate.h**: Utility functions for initializing the matrices with random values.

## Installation and Compilation

Make sure you have a C compiler installed (e.g., `gcc`), as well as support for POSIX threads (`pthread` library). 

### Compilation

Use the following commands to compile each file:

1. **Sequential Multiplication**:
   ```bash
   gcc -o sequential_mult version_1/sequentialMult.c populate.c -lpthread
   ```

2. **Parallel Row Multiplication**:
   ```bash
   gcc -o parallel_row_mult version_1/parallelRowMult.c populate.c -lpthread
   ```

3. **Parallel Element Multiplication**:
   ```bash
   gcc -o parallel_element_mult version_1/parallelElementMult.c populate.c -lpthread
   ```

4. **Parallel Blocked Multiplication**:
   ```bash
   gcc -o parallel_block_mult version_2/parallelBlockedMult.c populate.c -lpthread
   ```

## Usage

To run each version of the program, use the following commands:

1. **Sequential Multiplication**:
   ```bash
   ./sequential_mult <matrix_size>
   ```

2. **Parallel Row Multiplication**:
   ```bash
   ./parallel_row_mult <matrix_size> <number_of_processes>
   ```

3. **Parallel Element Multiplication**:
   ```bash
   ./parallel_element_mult <matrix_size> <number_of_processes>
   ```

4. **Parallel Blocked Multiplication**:
   ```bash
   ./parallel_block_mult <matrix_size> <block_size> <number_of_processes>
   ```

- `<matrix_size>`: Specifies the size of the matrix (e.g., `100`, `1000`).
- `<number_of_processes>`: Specifies how many child processes to create for parallel execution.
- `<block_size>`: Specifies the size of the block for block-based multiplication (e.g., `64`, `128`).

### Example Usage:
```bash
./parallel_row_mult 1000 12
./parallel_block_mult 1000 64 8
```

## Algorithms and Implementations

### 1. **Sequential Computation (`sequentialMult.c`)**
   - This implementation computes the result matrix element by element in a single process. It is suitable for small matrices but becomes inefficient as the matrix size increases.

### 2. **Parallel Row-Level Computation (`parallelRowMult.c`)**
   - This version divides the computation by rows, where each child process is responsible for computing one or more rows of the result matrix. A semaphore is used to synchronize access to the shared row index.

### 3. **Parallel Element-Level Computation (`parallelElementMult.c`)**
   - This implementation computes each element of the matrix in parallel, assigning elements to child processes. Semaphores synchronize access to shared variables that track the next element to be computed.

### 4. **Parallel Blocked Computation (`parallelBlockedMult.c`)**
   - This approach divides the matrix into smaller blocks, with each child process computing one block at a time. This method improves cache performance and efficiency by working on smaller chunks of the matrix.

## Results and Analysis

To measure the performance, run each algorithm with various matrix sizes (e.g., `10`, `100`, `1000`, etc.) and different numbers of processes. Analyze the results by comparing the execution times:

- For sequential execution, note the time it takes to complete matrix multiplication as the matrix size grows.
- For parallel execution (row, element, and block levels), observe the speed-up as more processes are used.
- Plot the results and compare the execution times of the algorithms.

### Recommendations
- **Small Matrices**: Use the sequential algorithm as the overhead of parallelization is not justified.
- **Medium Matrices**: Parallel row or element-based computation can significantly reduce computation time.
- **Large Matrices**: Blocked matrix multiplication is recommended as it optimizes cache usage and reduces the amount of data each process handles at once.

## Notes
- Make sure to adjust the number of processes based on the number of available cores on your system for optimal performance.
- For very large matrices, consider using smaller block sizes to fit into the cache.

## Conclusion
This project illustrates how parallelization strategies can significantly improve the performance of matrix multiplication. By utilizing processes and optimizing memory access patterns, even large matrices can be computed efficiently.
