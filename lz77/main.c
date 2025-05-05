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
void shift_buffer(char* buffer, char chr);
void compress();
void slide();
void search();
void decompress();

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
                    size_t last_match_pos = 0;
                    size_t match_length = 0;

                    for (size_t j = 0; j < DICTIONARY_SIZE; j++)
                    {
                        if (current_char == slider.dictionary[j])
                        {
                            last_match_pos = j;
                            for (size_t k = 0; k < DICTIONARY_SIZE - j; k++)
                            {
                                if (slider.buffer[i + k] == slider.dictionary[j + k])
                                {
                                    match_length++;
                                } else {
                                    match_length = 0;
                                    break;
                                }
                            }
                        }
                    }
                    
                    if (match_length)
                    {
                        Output* out =  malloc(sizeof(Output));
                        out->offset = i;
                        out->length = match_length;
                        out->next_char = current_char;
                    } else {
                        if (dict_size + 1 >= DICTIONARY_SIZE)
                        {
                            shift_buffer(slider.dictionary, current_char);
                        } else {
                            slider.dictionary[dict_size++] = current_char;
                        }
                    }
                    pos++;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

void shift_buffer(char* buffer, char chr)
{

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
