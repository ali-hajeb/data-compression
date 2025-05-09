#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>

#define BUFFER_SIZE 256
#define DICTIONARY_SIZE 32 * 1024

typedef struct {
    uint8_t offset;
    uint8_t length;
    // unsigned char next_char;
} Output;

typedef struct {
    unsigned char* dictionary;
    unsigned char* buffer;
} Window;

void memoryset(unsigned char*, unsigned char, size_t);
size_t strlength(const char*);
int strcomp(const char*, const char*);
void shift_buffer(unsigned char*, size_t, char);
size_t dict_push(unsigned char*, char, size_t*, const size_t);
int compress(FILE*, FILE*);

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
                printf("Compressed successfully!\n");
            }

            fclose(original_file);
            fclose(compressed_file);

            free(compressed_filename);
        }
    }
    return EXIT_SUCCESS;
}

void shift_buffer(unsigned char* buffer, size_t buffer_size, char chr)
{
    for (size_t i = 0; i < buffer_size - 1; i++)
    {
        buffer[i] = buffer[i + 1];
    }
        buffer[buffer_size - 1] = chr;
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
    if (*dict_last_pos + 1 >= dict_size)
    {
        shift_buffer(dict, dict_size, chr);
    } else {
        dict[(*dict_last_pos)++] = chr;
    }

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

    while ((bytes_read = fread(slider.buffer, 1, BUFFER_SIZE, original_file)) > 0)
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            // printf("%lld/%lld - CHECKING %c\n", i, bytes_read, slider.buffer[i]);
            unsigned char current_char = slider.buffer[i];
            size_t best_match_pos = dict_size;
            size_t best_match_length = 0;

            for (size_t j = 0; j < DICTIONARY_SIZE; j++)
            {
                if (current_char == slider.dictionary[j])
                {
                    size_t match_length = 0;
                    for (size_t k = 0; k < DICTIONARY_SIZE - j && i + k < bytes_read; k++)
                    {
                        if (slider.buffer[i + k] == slider.dictionary[j + k])
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
            
//                    printf("DICTIONARY: (%lld)", dict_size);
//                    for (size_t zz = 0; zz < dict_size; zz++) 
//                    {
//                        printf("%c", slider.dictionary[zz]);
//                    }
//                    printf("|\n");
//
            if (best_match_length)
            {
                Output* out =  malloc(sizeof(Output));
                out->offset = dict_size - best_match_pos;
                out->length = best_match_length;
                // out->next_char = slider.buffer[i + best_match_length];

                
                for (size_t l = 0; l < best_match_length; l++)
                {
                    dict_push(slider.dictionary, slider.buffer[i + l], &dict_size, DICTIONARY_SIZE);
                }
                
                i += best_match_length - 1;

//                        printf("MATCH: (%lld)", best_match_length);
//                        for (size_t z = 0; z < best_match_length; z++)
//                        {
//                            printf("%c", slider.dictionary[best_match_pos + z]);
//                        }
//                        printf("|\n");

                printf("<%lld, %lld>\n", out->offset, out->length);
                output_buffer[output_pos++] = out->offset;
                output_buffer[output_pos++] = out->length;
                
                free(out);
            } else {
                printf("<0, 0, %c>\n", slider.buffer[i + best_match_length]);
                output_buffer[output_pos++] = 0;
                output_buffer[output_pos++] = 0;
                output_buffer[output_pos++] = slider.buffer[i + best_match_length];

                dict_push(slider.dictionary, current_char, &dict_size, DICTIONARY_SIZE);
            }

            if (output_pos + 2 >= BUFFER_SIZE)
            {
                fwrite(output_buffer, sizeof(unsigned char), output_pos, compressed_file);
                output_pos = 0;
            }
            printf("----------------\n");
        }
    }

    if (output_pos > 0)
    {
        fwrite(output_buffer, sizeof(unsigned char), output_pos, compressed_file);
    }

    free(slider.dictionary);
    free(slider.buffer);
    free(output_buffer);
    return 0;
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
