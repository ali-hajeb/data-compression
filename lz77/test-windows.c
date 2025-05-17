#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_PATH_LENGTH 260 // Windows MAX_PATH
#define TEST_FILES_DIR "test_files"
#define TEST_RESULTS_DIR "test_results"

// Function to create a directory if it doesn't exist
int create_directory(const char *path) {
    if (!CreateDirectory(path, NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            fprintf(stderr, "Failed to create directory %s: %lu\n", path, GetLastError());
            return -1;
        }
    }
    return 0;
}

// Function to run a command and check its return status
int run_command(const char *cmd) {
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
        return -1;
    }
    return 0;
}

// Function to compare two files for equality
int compare_files(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        fprintf(stderr, "Failed to open files for comparison: %s, %s\n", file1, file2);
        return -1;
    }

    int equal = 1;
    int c1, c2;
    while ((c1 = fgetc(f1)) != EOF && (c2 = fgetc(f2)) != EOF) {
        if (c1 != c2) {
            printf("\t[DIFF] %X (%ld) != %X (%ld)\n\r", c1, ftell(f1), c2, ftell(f2));
            equal = 0;
            break;
        }
    }
    if (fgetc(f1) != EOF || fgetc(f2) != EOF) equal = 0;

    fclose(f1);
    fclose(f2);
    return equal;
}

int main() {
    // Compile the main program
    if (run_command("gcc -c main.c -o main.o -Wall -g") != 0 ||
        run_command("gcc main.o -o main.exe") != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 1;
    }

    // Create test_results directory
    if (create_directory(TEST_RESULTS_DIR) != 0) {
        return 1;
    }

    // Prepare to search test_files directory
    char search_path[MAX_PATH_LENGTH];
    snprintf(search_path, MAX_PATH_LENGTH, "%s\\*", TEST_FILES_DIR);

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(search_path, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to open test_files directory: %lu\n", GetLastError());
        return 1;
    }

    int test_number = 1;

    // Process each file in test_files
    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue; // Skip directories
        }

        // Create paths
        char input_path[MAX_PATH_LENGTH];
        char compressed_path[MAX_PATH_LENGTH];
        char decompressed_path[MAX_PATH_LENGTH];
        char test_dir[MAX_PATH_LENGTH];

        snprintf(input_path, MAX_PATH_LENGTH, "%s\\%s", TEST_FILES_DIR, ffd.cFileName);
        snprintf(test_dir, MAX_PATH_LENGTH, "%s\\test_%d", TEST_RESULTS_DIR, test_number);
        snprintf(compressed_path, MAX_PATH_LENGTH, "%s\\%s.lz7", test_dir, ffd.cFileName);
        snprintf(decompressed_path, MAX_PATH_LENGTH, "%s\\%s", test_dir, ffd.cFileName);

        // Create test-specific directory
        if (create_directory(test_dir) != 0) {
            FindClose(hFind);
            return 1;
        }

        // Run compression
        char cmd[MAX_PATH_LENGTH * 2];
        snprintf(cmd, sizeof(cmd), "main.exe -c \"%s\"", input_path);
        printf("[TEST %d-1]: Compressing %s\n", test_number, ffd.cFileName);
        if (run_command(cmd) != 0) {
            fprintf(stderr, "Compression failed for %s\n", ffd.cFileName);
            FindClose(hFind);
            return 1;
        }

        // Move compressed file to test directory
        char temp_compressed[MAX_PATH_LENGTH];
        snprintf(temp_compressed, MAX_PATH_LENGTH, "%s.lz7", input_path);
        snprintf(cmd, sizeof(cmd), "move \"%s\" \"%s\"", temp_compressed, compressed_path);
        if (run_command(cmd) != 0) {
            fprintf(stderr, "Failed to move compressed file for %s\n", ffd.cFileName);
            FindClose(hFind);
            return 1;
        }

        // Run decompression
        snprintf(cmd, sizeof(cmd), "main.exe -d \"%s\"", compressed_path);
        printf("[TEST %d-2]: Decompressing %s.lz7\n", test_number, ffd.cFileName);
        if (run_command(cmd) != 0) {
            fprintf(stderr, "Decompression failed for %s\n", ffd.cFileName);
            FindClose(hFind);
            return 1;
        }

        // Verify decompressed file matches original
        printf("[TEST %d-3]: Verifying %s\n", test_number, ffd.cFileName);
        if (compare_files(input_path, decompressed_path)) {
            printf("---[TEST %d]: PASSED - Decompressed file matches original\n", test_number);
        } else {
            printf("---[TEST %d]: FAILED - Decompressed file differs from original\n", test_number);
        }

        test_number++;
        printf("\n");
    } while (FindNextFile(hFind, &ffd) != 0);

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        fprintf(stderr, "Error reading test_files directory: %lu\n", GetLastError());
        FindClose(hFind);
        return 1;
    }

    FindClose(hFind);
    printf("Testing complete.\n");
    return 0;
}
