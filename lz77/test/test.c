
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PATH 256
#define TEST_FILES_DIR "./test/test_files"
#define TEST_RESULTS_DIR "./test/test_results"

// Function to create a directory if it doesn't exist
int create_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        if (mkdir(path, 755) != 0) {
            perror("Failed to create directory");
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
    if (run_command("make all") != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 1;
    }

    // Create test_results directory
    if (create_directory(TEST_RESULTS_DIR) != 0) {
        return 1;
    }

    // Open test_files directory
    DIR *dir = opendir(TEST_FILES_DIR);
    if (!dir) {
        perror("Failed to open test_files directory");
        return 1;
    }

    struct dirent *entry;
    int test_number = 1;

    // Process each file in test_files
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Create paths
        char input_path[MAX_PATH];
        char compressed_path[MAX_PATH];
        char adv_compressed_path[MAX_PATH];
        char decompressed_path[MAX_PATH];
        char adv_decompressed_path[MAX_PATH];
        char test_dir[MAX_PATH];

        snprintf(input_path, MAX_PATH, "%s/%s", TEST_FILES_DIR, entry->d_name);
        snprintf(test_dir, MAX_PATH, "%s/test_%d", TEST_RESULTS_DIR, test_number);
        snprintf(compressed_path, MAX_PATH, "%s/%s.lz7", test_dir, entry->d_name);
        snprintf(adv_compressed_path, MAX_PATH, "%s/a_%s.lz7", test_dir, entry->d_name);
        snprintf(decompressed_path, MAX_PATH, "%s/%s", test_dir, entry->d_name);
        snprintf(adv_decompressed_path, MAX_PATH, "%s/a_%s", test_dir, entry->d_name);

        // Create test-specific directory
        if (create_directory(test_dir) != 0) {
            closedir(dir);
            return 1;
        }

        printf("\n--------------------------|TEST %02d|--------------------------\n", test_number);

        // Run compression
        char cmd[MAX_PATH * 2];
        snprintf(cmd, sizeof(cmd), "./bin/lz7 -c %s -o %s", input_path, compressed_path);
        printf("[TEST 1/3]: Compressing %s\n", entry->d_name);
        if (run_command(cmd) != 0) {
            fprintf(stderr, "Compression failed for %s\n", entry->d_name);
            closedir(dir);
            return 1;
        }

        // Run decompression
        snprintf(cmd, sizeof(cmd), "./bin/lz7 -d %s -o %s", compressed_path, decompressed_path);
        printf("[TEST 2/3]: Decompressing %s.lz7\n", entry->d_name);
        if (run_command(cmd) != 0) {
            fprintf(stderr, "Decompression failed for %s\n", entry->d_name);
            closedir(dir);
            return 1;
        }

        // Verify decompressed file matches original
        printf("[TEST 3/3]: Verifying %s\n", entry->d_name);
        if (compare_files(input_path, decompressed_path)) {
            printf("--- [PASSED] - Decompressed file matches original\n");
        } else {
            printf("--- [FAILED] - Decompressed file differs from original\n");
        }

        test_number++;
    }
    printf("\n-------------------------------------------------------------\n");

    closedir(dir);
    printf("Testing complete.\n");
    return 0;
}
