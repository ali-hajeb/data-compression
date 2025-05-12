#include <stddef.h>
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>

#define DEBUG 0
#define BUFFER_SIZE 256
#define DICTIONARY_SIZE 32 * 1024

typedef struct {
    uint16_t offset;
    uint8_t length;
    // unsigned char next_char;
} Output;

typedef struct {
    unsigned char* dictionary;
    unsigned char* buffer;
} Window;

size_t file_size = 0;

void memoryset(unsigned char*, unsigned char, size_t);
size_t strlength(const char*);
int strcomp(const char*, const char*);
void shift_buffer(unsigned char*, size_t, char, size_t);
size_t dict_push(unsigned char*, char, size_t*, const size_t);
int compress(FILE*, FILE*);
int compress_std(FILE*, FILE*);
int compress_v3(FILE*, FILE*);
int decompress(FILE*, FILE*);
int decompress_std(FILE*, FILE*);
int decompress_v3(FILE*, FILE*);

int main(int argc, char* argv[])
{
    if (argc == 3)
    {
        if (strcomp(argv[1], "-c") == 0)
        {
            FILE* original_file = fopen(argv[2], "rb");
            if (original_file == NULL)
            {
                fprintf(stderr, "[ERROR]]: main() {} -> Unable to open the file!\n");
                return 1;
            }

            // Calculating the file size
            fseek(original_file, 0, SEEK_END);
            file_size = ftell(original_file);
            fseek(original_file, 0, SEEK_SET);

            char* compressed_filename = malloc(strlength(argv[2]) + 5);
            if (compressed_filename == NULL)
            {
                fprintf(stderr, "[ERROR]]: main() {} -> Unable to allocate memory for comressed file name!\n");
                return 1;
            }
           
            sprintf(compressed_filename, "%s.lz7", argv[2]);

            FILE* compressed_file = fopen(compressed_filename, "wb");
            if (compressed_file == NULL)
            {
                fprintf(stderr, "[ERROR]]: main() {} -> Unable to create compressed file!\n");
                return 1;
            }

            int status = compress(original_file, compressed_file);
            if (status > -1)
            {
                printf("Compressed successfully!\n");
            } else {
                printf("Compressed failed!\n");
            }

            fclose(original_file);
            fclose(compressed_file);

            free(compressed_filename);
        }
        else if (strcomp(argv[1], "-d") == 0)
        {
            FILE* compressed_file = fopen(argv[2], "rb");
            if (compressed_file == NULL)
            {
                fprintf(stderr, "[ERROR]]: main() {} -> Unable to open the file!\n");
                return 1;
            }


            // Calculating the file size
            fseek(compressed_file, 0, SEEK_END);
            file_size = ftell(compressed_file);
            fseek(compressed_file, 0, SEEK_SET);

            char* original_filename = malloc(strlength(argv[2]) - 4);
            if (original_filename == NULL)
            {
                fprintf(stderr, "[ERROR]]: main() {} -> Unable to allocate memory for original file name!\n");
                return 1;
            }
           
            argv[2][strlength(argv[2]) - 4] = '\0';
            sprintf(original_filename, "%s", argv[2]);
            original_filename[strlength(argv[2])] = '\0';
            printf("%s\n", original_filename);

            FILE* original_file = fopen(original_filename, "wb");
            if (original_file == NULL)
            {
                fprintf(stderr, "[ERROR]]: main() {} -> Unable to create original file!\n");
                return 1;
            }

            int status = decompress(compressed_file, original_file);
            if (status > -1)
            {
                printf("Decompressed successfully!\n");
            } else {
                printf("Decompressed failed!\n");
            }

            fclose(original_file);
            fclose(compressed_file);

            free(original_filename);
        }
    }
    return EXIT_SUCCESS;
}

void shift_buffer(unsigned char* buffer, size_t buffer_size, char chr, size_t shift)
{
    for (size_t i = 0; i + shift < buffer_size; i++)
    {
        buffer[i] = buffer[i + shift];
    }
    buffer[buffer_size - shift] = chr;
    for (size_t i = buffer_size - shift + 1; i < buffer_size; i++)
    {
        buffer[i] = 0;
    }
}

int strcomp(const char* s1, const char* s2)
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

size_t dict_push(unsigned char* dict, char chr, size_t* dict_last_pos, const size_t dict_size)
{
    dict[*dict_last_pos] = chr;
    *dict_last_pos = (*dict_last_pos + 1) % dict_size;
    return 1;
}

int compress(FILE* original_file, FILE* compressed_file)
{
    Window slider;
    slider.buffer = malloc(BUFFER_SIZE);
    slider.dictionary = malloc(DICTIONARY_SIZE);

    unsigned char* output_buffer = malloc(3 * BUFFER_SIZE);

    if (slider.dictionary == NULL || slider.buffer == NULL || output_buffer == NULL)
    {
        fprintf(stderr, "[ERROR]: main() {} -> Unable to allocate memory for dictionary and buffer!\n");
        fclose(original_file);
        free(slider.buffer);
        free(slider.dictionary);
        free(output_buffer);
        exit(EXIT_FAILURE);
    }
    memoryset(slider.dictionary, 0, DICTIONARY_SIZE);

    size_t bytes_read = 0;
    size_t output_pos = 0;
    size_t dict_size = 0;
    size_t process = 0;

    while ((bytes_read = fread(slider.buffer, 1, BUFFER_SIZE, original_file)) > 0)
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            if (DEBUG) printf("[%04lld] (%2X):\t", process + i, slider.buffer[i]);
            unsigned char current_char = slider.buffer[i];
            size_t best_match_pos = dict_size;
            uint8_t best_match_length = 0;

            for (size_t j = 0; j < dict_size; j++)
            {
                if (current_char == slider.dictionary[j])
                {
                    size_t match_length = 0;
                    for (size_t k = j; k < dict_size && i + match_length < bytes_read - 1; k++)
                    {
                        if (slider.buffer[i + match_length] == slider.dictionary[k])
                        {
                            match_length++;
                        } else {
                            break;
                        }
                    }

                    if (match_length >= best_match_length)
                    {
                        best_match_length = match_length;
                        best_match_pos = j;
                    }
                } 
            }
            
            if (best_match_length)
            {
                Output out;
                out.offset = dict_size - best_match_pos;
                out.length = best_match_length;

                if (DEBUG) printf("MATCH\t <%04d, %04d>\t\t-> |", out.offset, out.length);
                for (size_t l = 0; l < best_match_length; l++)
                {
                    dict_push(slider.dictionary, slider.buffer[i + l], &dict_size, DICTIONARY_SIZE);
                    if (DEBUG) printf("%2X|", slider.buffer[i + l]);
                }

                output_buffer[output_pos++] = out.offset & 0xFF;        // Write low byte
                output_buffer[output_pos++] = (out.offset >> 8) & 0xFF; // Write high byte
                output_buffer[output_pos++] = out.length;        // Write low byte

                i += best_match_length - 1;
            } else {
                output_buffer[output_pos++] = 0;
                output_buffer[output_pos++] = 0;
                output_buffer[output_pos++] = slider.buffer[i];

                if (DEBUG) printf("LITERAL\t <0000, 0000, %2X>", slider.buffer[i]);
                dict_push(slider.dictionary, current_char, &dict_size, DICTIONARY_SIZE);
            }

            if (output_pos + 2 >= BUFFER_SIZE)
            {
                fwrite(output_buffer, sizeof(unsigned char), output_pos, compressed_file);
                output_pos = 0;
            }
            if (DEBUG) printf("\n");
        }

        process += bytes_read;
        if ( process % (100 * 256) == 0)
        {
            if (!DEBUG) printf("\rProcessing: %lld/%lld -> %ld", process, file_size, ftell(compressed_file));
        }
    }

	if (!DEBUG) printf("\rProcessing: %lld/%lld -> %ld\n", process, file_size, ftell(compressed_file));

    if (output_pos > 0)
    {
        fwrite(output_buffer, sizeof(unsigned char), output_pos, compressed_file);
    }

    free(slider.dictionary);
    free(slider.buffer);
    free(output_buffer);
    return 0;
}

int decompress(FILE* compressed_file, FILE* original_file)
{
    unsigned char* output_buffer = malloc(DICTIONARY_SIZE + 1024);
    unsigned char* read_buffer = malloc(BUFFER_SIZE * 3);
    if (output_buffer == NULL || read_buffer == NULL)
    {
        fprintf(stderr, "[ERROR]: main() {} -> Unable to allocate memory for buffer!\n");
        fclose(original_file);
        free(output_buffer);
        free(read_buffer);
        exit(EXIT_FAILURE);
    }
    memoryset(output_buffer, 0, DICTIONARY_SIZE + 1024);

    size_t bytes_read = 0;
    size_t output_pos = 1024;
    size_t process = 0;

    while ((bytes_read = fread(read_buffer, sizeof(unsigned char), BUFFER_SIZE * 3, compressed_file)) > 0)
    {
        for (size_t i = 0; i < bytes_read; i++) 
        {
            // If offset is more than 0 then read only 2 bytes (offset, length)
            if (read_buffer[i])
            {
                if ( i + 4 >= bytes_read)
                {
                    fseek(compressed_file, i - bytes_read, SEEK_CUR);
                    break;
                }
                uint16_t offset = ((uint16_t) read_buffer[i + 1] << 8) | read_buffer[i]; // Read 2 bytes
                if (DEBUG) printf("[%04lld] <%03d, %03d>\t[%2X][%2X], %lld\t -> |", process + i, offset, read_buffer[i + 2], read_buffer[i], read_buffer[i + 1], output_pos);
                offset = output_pos - offset;
                i += 2;
                uint8_t length = read_buffer[i]; // Read 2 bytes

                // If output_buffer is full then shift it 1kb
                if (length + output_pos >= DICTIONARY_SIZE + 1024)
                {
                    fwrite(output_buffer, sizeof(unsigned char), output_pos, original_file);
                    shift_buffer(output_buffer, DICTIONARY_SIZE + 1024, 0, 1024);
                    output_pos -= 1024;
                }

                for (uint8_t j = 0; j < length; j++)
                {
                    if (DEBUG) printf("%2X|", output_buffer[offset + j]);
                    output_buffer[output_pos++] = output_buffer[offset + j];
                }
            } 
            // If offset is 0 then read 3 bytes (offset, length, next_char)
            else {
                if (DEBUG) printf("[%04lld] <000, 000, %2X>", process + i, read_buffer[i + 2]);
                // If output_buffer is full then shift it 1kb
                if (output_pos + 1 >= DICTIONARY_SIZE + 1024)
                {
                    fwrite(output_buffer, sizeof(unsigned char), output_pos, original_file);
                    shift_buffer(output_buffer, DICTIONARY_SIZE + 1024, 0, 1024);
                    output_pos -= 1024;
                }
                i += 2;
                output_buffer[output_pos++] = read_buffer[i];
            }
            if (DEBUG) printf("\n");
        }
        
        process += bytes_read;
        if ( process % (100 * 1024) == 0)
        {
            if (!DEBUG) printf("\rProcessing: %lld/%lld -> %ld", process, file_size, ftell(original_file));
        }
    }

    if (output_pos > 0)
    {
        fwrite(output_buffer, sizeof(unsigned char), output_pos, original_file);
    }
	if (!DEBUG) printf("\rProcessing: %lld/%lld -> %ld\n", process, file_size, ftell(original_file));
    free(output_buffer);
    free(read_buffer);
    return 1;
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

void memoryset(unsigned char* ptr, unsigned char value, size_t count)
{
    while (count-- > 0)
    {
        *ptr++ = value;
    }
}
