def compare_matrices(file1, file2, size, tolerance=1e-9):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        for i in range(size):
            row1 = f1.readline().strip().split()
            row2 = f2.readline().strip().split()
            for j in range(size):
                val1 = float(row1[j])
                val2 = float(row2[j])
                if abs(val1 - val2) > tolerance:  # Allowing for floating-point precision error
                    print(f"Difference found at ({i}, {j}): {val1} != {val2}")
                    return False
    print("Matrices are identical.")
    return True

# Usage example: compare_matrices("sequential_output.txt", "parallel_element_output.txt", size)
if __name__ == "__main__":
    import sys
    if len(sys.argv) != 4:
        print("Usage: python compare_matrices.py <file1> <file2> <matrix_size>")
    else:
        file1 = sys.argv[1]
        file2 = sys.argv[2]
        size = int(sys.argv[3])
        compare_matrices(file1, file2, size)
