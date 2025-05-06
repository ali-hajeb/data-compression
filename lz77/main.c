#include <stdio.h> 
#include <stdlib.h>

#define BUFFER_SIZE 256
#define DICTIONARY_SIZE 32 * 1024

typedef struct output {
    size_t offset;
    size_t length;
    unsigned char next_char;
} Output;

typedef struct window {
    char* dictionary;
    char* buffer;
} Window;

int strcomp(const char*, const char*);
void shift_buffer(char*, size_t, char);
size_t dict_push(char*, char, size_t*, const size_t);

int main(int argc, char* argv[])
{
    if (argc == 3)
    {
        if (strcomp(argv[1], "-c") == 0)
        {
            FILE* original_file = fopen(argv[2], "rb");
            Window slider;
            slider.buffer = malloc(BUFFER_SIZE);
            slider.dictionary = malloc(DICTIONARY_SIZE);
            if (slider.dictionary == NULL || slider.buffer == NULL)
            {
                fprintf(stderr, "[ERROR]: main() {} -> Unable to allocate memory for dictionary and buffer!\n");
                fclose(original_file);
                exit(EXIT_FAILURE);
            }

            size_t bytes_read = 0;
            size_t pos = 0;
            size_t dict_size = 0;

            while ((bytes_read = fread(slider.buffer, 1, BUFFER_SIZE, original_file)) > 0)
            {
                for (size_t i = 0; i < bytes_read; i++)
                {
                    unsigned char current_char = slider.buffer[i];
                    size_t best_match_pos = dict_size;
                    size_t best_match_length = 0;

                    for (size_t j = 0; j < DICTIONARY_SIZE; j++)
                    {
                        if (current_char == slider.dictionary[j])
                        {
                            size_t match_length = 0;
                            for (size_t k = 0; k < DICTIONARY_SIZE - j; k++)
                            {
                                if (slider.buffer[i + k] == slider.dictionary[j + k])
                                {
                                    match_length++;
                                } else {
                                    break;
                                }
                            }

                            if (match_length > best_match_length)
                            {
                                best_match_length = match_length;
                                best_match_pos = j;
                            }
                        }
                    }
                    
                    if (best_match_length)
                    {
                        Output* out =  malloc(sizeof(Output));
                        out->offset = dict_size - best_match_pos;
                        out->length = best_match_length;
                        out->next_char = slider.buffer[i + best_match_length];

                        for (size_t l = 0; l < best_match_length; l++)
                        {
                            dict_push(slider.dictionary, slider.buffer[i + l], &dict_size, DICTIONARY_SIZE);
                        }
                        
                        i += best_match_length - 1;

                        printf("MATCH: (%lld)", best_match_length);
                        for (size_t z = 0; z < best_match_length; z++)
                        {
                            printf("%c", slider.dictionary[best_match_pos + z]);
                        }
                        printf("|\n");

                        printf("DICTIONARY: (%lld)", dict_size);
                        for (size_t zz = 0; zz < dict_size; zz++) 
                        {
                            printf("%c", slider.dictionary[zz]);
                        }
                        printf("|\n");

                        printf("<%d, %lld, %c>\n", out->offset, out->length, out->next_char);
                    } else {
                        printf("<%d, 0, %c>\n", dict_size - best_match_pos, slider.buffer[i + best_match_length]);
                        printf("DICTIONARY: (%lld)", dict_size);
                        for (size_t zz = 0; zz < dict_size; zz++) 
                        {
                            printf("%c", slider.dictionary[zz]);
                        }
                        printf("|\n");
                        dict_push(slider.dictionary, current_char, &dict_size, DICTIONARY_SIZE);
                    }
                    printf("----------------\n");
                    pos++;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

void shift_buffer(char* buffer, size_t buffer_size, char chr)
{
    for (size_t i = 0; i < buffer_size - 1; i++)
    {
        buffer[i] = buffer[i + 1];
        buffer[i + 1] = chr;
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

size_t dict_push(char* dict, char chr, size_t* dict_last_pos, const size_t dict_size)
{
    if (*dict_last_pos + 1 >= dict_size)
    {
        shift_buffer(dict, dict_size, chr);
    } else {
        dict[(*dict_last_pos)++] = chr;
    }

    return 1;
}
