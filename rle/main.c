#include <stdio.h>
#include <stdlib.h>

size_t strlength(const char*);
int strcompare(const char*, const char*);
ssize_t get_line(char **, size_t *, FILE *);
int extract_filename_format(const char *, char **, char **);
void compress(FILE *, const char*);
void decompress(FILE *, const char*);
void print_cli_example();

long file_size = 0;

int main(int argc, char* argv[])
{
	char *filename;
	char *fileformat;

    if (argc == 3)
    {
        if (strcompare(argv[1], "-c") == 0)
        {
            // Extracting filename and extention
            int res = extract_filename_format(argv[2], &filename, &fileformat);
            if (res < 0)
            {
                printf("[ERROR]: main() {} -> There was a problem extracting file name and extension!\n");
                free(filename);
                free(fileformat);
                exit(EXIT_FAILURE);
            }
            else 
            {
                // Creating a new name for the file
                char* compressed_filename = malloc(strlength(filename) + 5);
                if (compressed_filename == NULL)
                {
                    printf("[ERROR]: main() {} -> Unable to allocate memory for compressed filename buffer\n");
                    exit(1);
                }

                sprintf(compressed_filename, "%s.rle\n", argv[2]);
                compressed_filename[strlength(compressed_filename) - 1] = '\0';

                // Opening the input file
                FILE *original_file = fopen(argv[2], "rb");
                if (original_file == NULL)
                {
                    printf("[ERROR]]: main() {} -> Unable to open the file!\n");
                    return 1;
                }

                // Calculating the file size
                fseek(original_file, 0, SEEK_END);
                file_size = ftell(original_file);
                fseek(original_file, 0, SEEK_SET);

                // Compressing the file
                printf("Compressing...\n");
                compress(original_file, compressed_filename);
                printf("Compression completed!\n");

                // Memory cleaning
                fclose(original_file);

                free(filename);
                free(fileformat);
                free(compressed_filename);
            }
        }
        else if (strcompare(argv[1], "-d") == 0)
        {
            // Extracting filename and extention
            int res = extract_filename_format(argv[2], &filename, &fileformat);
            if (res < 0)
            {
                printf("[ERROR]: main() {} -> There was a problem extracting file name and extension!\n");
                free(filename);
                free(fileformat);
                exit(EXIT_FAILURE);
            }
            else 
            {
                // Opening the input file
                FILE *compressed_file = fopen(argv[2], "rb");
                if (compressed_file == NULL)
                {
                    printf("[ERROR]: main() {} -> Unable to create compressed file!\n");
                    return 1;
                }

                // Calculating the file size
                fseek(compressed_file, 0, SEEK_END);
                file_size = ftell(compressed_file);
                fseek(compressed_file, 0, SEEK_SET);

                // Decompressing the file
                printf("Decompressing...\n");
                decompress(compressed_file, filename);
                printf("Decompression completed!\n");

                // Memory cleaning
                fclose(compressed_file);

                free(filename);
                free(fileformat);
            }
 
        }
        else 
        {
            print_cli_example();
            exit(EXIT_FAILURE);
        }
    }
    else 
    {
        print_cli_example();
        exit(EXIT_FAILURE);
    }

   return 0;
}

ssize_t get_line(char **line_ptr, size_t *size, FILE *stream)
{
	// Checking if all parameters are valid and provided;
	if (line_ptr == NULL || size == NULL || stream == NULL)
	{
		printf("[ERROR]: get_line() {} -> Necessary parameters are not valid!\n");
		return -1;
	}

	// Defining cursor and input character
	size_t pos = 0;
	int ch;

	// Checking if buffer is allocated and if not, allocate a buffer
	if (*line_ptr == NULL)
	{
		*size = 128; // Initial buffer size
		*line_ptr = malloc(*size);
		if (*line_ptr == NULL)
		{
			printf("[ERROR]: get_line() {} -> Unable to initialize a buffer for input!\n");
			return -1;
		}
	}

	while ((ch = fgetc(stream)) != EOF)
	{
		// Checking if more memory is needed for buffer
		if (pos + 1 >= *size)
		{
			// Allocating more memory for buffer
			size_t new_size = 2 * (*size);
			char *temp = realloc(*line_ptr, new_size);
			if (temp == NULL)
			{
				printf("[ERROR]: get_line() {} -> Unable to reallocate more memory for input buffer!\n");
				return -1;
			}
			*line_ptr = temp;
			*size = new_size;
		}

		// Terminate input if user presses 'ENTER'
		if (ch == '\n')
		{
			break;
		}
		// Adding the input character to the buffer
		(*line_ptr)[pos++] = ch;
	}

	// Check if no data was read
	if (pos == 0 || ch == EOF)
	{
		return -1;
	}

	// Terminate the buffer with a null character
	(*line_ptr)[pos] = '\0';

	// Return the size of string
	return pos;
}

int extract_filename_format(const char *filepath, char **filename, char **fileformat)
{
	int result = -1; // -1: Error, 0: Successfully extracted only file name, 1: Successfully extracted only file extension, 2: Successfully extracted both file name and extension
	const char *p = filepath;
	ssize_t last_slash_pos = -1;
	ssize_t last_dot_pos = -1;

	// Finding last slash and last dot
	size_t pos = 0;
	while (*p != '\0')
	{
		if (*p == '/' || *p == '\\')
		{
			last_slash_pos = pos;
		}
		if (*p == '.')
		{
			last_dot_pos = pos;
		}

		pos++;
		p++;
	}

	// Checking if last slash was found
	if (last_slash_pos == -1)
	{
		printf("[ERROR]: extract_filename_format() {} -> Invalid file path! (All file path must contain '/' or '\\'.)\n");
		printf("EXAMPLES:\n\t- C:\\...\\[file]\n\t- ./[file]\n\r");
		return -1;
	}
	// Checking if last dot was found
	if (last_dot_pos < 1)
	{
		// Assigning filepath end position to the last_dot_pos
		last_dot_pos = pos;
	}

	// Checking if file name has one or more characters. (Some files might be like '.ext')
	size_t filename_size = last_dot_pos - last_slash_pos - 1;
	if (filename_size > 1)
	{
		*filename = malloc(filename_size + 1);
		if (*filename == NULL)
		{
			printf("[ERROR]: extract_filename_format() {} -> Unable to allocate memory for filename!\n");
			return -1;
		}

		// Getting file name
		for (size_t i = 0; i < filename_size; i++)
		{
			(*filename)[i] = filepath[i + last_slash_pos + 1];
		}
		(*filename)[filename_size] = '\0';
		// printf("%s\n", *filename);
		// Setting result = 0
		result = 0;
	}

	// Checking if file has an extension
	if (last_dot_pos > -1 && pos - last_dot_pos > 1)
	{
		size_t fileformat_size = pos - last_dot_pos - 1;
		*fileformat = malloc(fileformat_size + 1);
		if (*fileformat == NULL)
		{
			printf("[ERROR]: extract_filename_format() {} -> Unable to allocate memory for fileformat!\n");
			return -1;
		}

		// Getting file format
		for (size_t i = 0; i < fileformat_size; i++)
		{
			(*fileformat)[i] = filepath[i + last_dot_pos + 1];
		}
		(*fileformat)[fileformat_size] = '\0';
		// printf("%s\n", *fileformat);

		// Setting the appropriate result
		// If file name was found:
		if (result == 0)
		{
			result = 2;
		}
		// If file only has extension
		else
		{
			result = 1;
		}
	}

	return result;
}

void compress(FILE *original_file, const char* filename)
{    
	FILE *compressed_file = fopen(filename, "wb");
	if (compressed_file == NULL)
	{
		printf("[ERROR]: compress() {} -> Unable to create compressed file!\n");
		exit(1);
	}

	unsigned char flag_byte, current_byte;
	long cur = 0;
	int counter = 0;

	if (fread(&flag_byte, sizeof(unsigned char), 1, original_file) == 1)
	{
		counter = 1;
		while (fread(&current_byte, sizeof(unsigned char), 1, original_file) == 1)
		{
			printf("\rProcessing: %ld/%ld -> %ld", cur, file_size, ftell(compressed_file));
			cur++;
			if (flag_byte == current_byte)
			{
				counter++;

				if (counter == 255)
				{
					fwrite(&counter, sizeof(unsigned char), 1, compressed_file);
					fwrite(&flag_byte, sizeof(unsigned char), 1, compressed_file);
					counter = 0;
				}
			}
			else
			{
				fwrite(&counter, sizeof(unsigned char), 1, compressed_file);
				fwrite(&flag_byte, sizeof(unsigned char), 1, compressed_file);
				counter = 0;
				flag_byte = current_byte;
				counter = 1;
			}
		}
	}

	if (counter > 0)
	{
		fwrite(&counter, sizeof(unsigned char), 1, compressed_file);
		fwrite(&flag_byte, sizeof(unsigned char), 1, compressed_file);
		cur++;
	}

	printf("\rProcessing: %ld/%ld -> %ld\n", cur, file_size, ftell(compressed_file));
	fclose(compressed_file);
}

void decompress(FILE *compressed_file, const char* original_filename)
{
	FILE *decompressed_file = fopen(original_filename, "wb");
	if (decompressed_file == NULL)
	{
		printf("[ERROR]: decompress() {} -> Unable to create decompressed file!\n");
		exit(1);
	}

	unsigned char counter_byte, flag_byte;
    long cur = 0;

	while (fread(&counter_byte, sizeof(unsigned char), 1, compressed_file) == 1)
	{
        printf("\rProcessing: %ld/%ld -> %ld", cur, file_size, ftell(decompressed_file));
        cur += 2;

		if (fread(&flag_byte, sizeof(unsigned char), 1, compressed_file) == 1)
		{
			for (int i = 0; i < counter_byte; i++)
			{
				fwrite(&flag_byte, sizeof(unsigned char), 1, decompressed_file);
			}
		}
		else
		{
            printf("[ERROR]: decompress() {} -> Unexpected error during decompressing the file!\n");
			fclose(decompressed_file);
			return;
		}
	}

	printf("\rProcessing: %ld/%ld -> %ld\n", cur, file_size, ftell(decompressed_file));
	fclose(decompressed_file);
}

size_t strlength(const char* str)
{
    size_t length = 0;
    while (*str != '\0')
    {
        str++;
        length++;
    }
    return length;
}

int strcompare(const char* s1, const char* s2)
{
    unsigned char c1, c2;
    do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;

      if (c1 == '\0') return c1 - c2;
    }
  while (c1 == c2);

  return c1 - c2;
}

void print_cli_example()
{
    printf("\r[Usage]:\n");
    printf("\r\t-c <file path>: Compress file and save it.\n");
    printf("\r\t-d <file path>: Decompress file and save it.\n");
    printf("\r\t[EXAMPLE]: ./rle -c image.bmp\n");
    printf("\n");
}
