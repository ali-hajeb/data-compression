# Run-Length Encoding (RLE)
Run-length encoding (RLE) is a lossless data compression technique that replaces consecutive identical data values with a single count value. It's particularly effective for data with many runs, such as images, animations, and certain text files. 

## How it works
1. Identify runs: RLE scans data for sequences of identical values, which are called "runs". 
2. Replace with count and value: Instead of storing the repeated value multiple times, RLE stores the value and the number of times it repeats. 
- Example: A sequence like "AAAAABBBCCDAA" would be encoded as "5A3B2C1D2A". 

## When it's useful
- Images:
RLE is often used for compressing simple images, icons, and animations, especially those with large areas of the same color, like line drawings or black and white images. 
- Fax:
It's a common compression method for fax images, where long runs of black and white pixels are frequent. 
- Other data:
RLE can also be used for compressing data with long runs of identical values in other formats, such as some text files. 

## Advantages
- Simple to implement:
RLE is a straightforward compression algorithm to understand and implement.
- Lossless:
RLE doesn't lose any information during compression, so the original data can be perfectly reconstructed. 

## Limitations
- Ineffective with irregular data:
RLE might not be the most efficient compression method for data with short or irregular sequences of values.
- Size inflation:
In some cases, the encoded data might be larger than the original if the data doesn't have many runs.

## Compile
In order to compile the source code, you can use `gcc` in this manner:
```
gcc ./main.c -o rle
```

g++:
```
g++ ./main.c -o rle
```

clang:
```
clang ./main.c -o rle
```

## Usage
- Compressing a file: `./rle -c <file_path>`. 
- Decompressing a file: `./rle -d <file_path>`.

Example:
- `./rle -c c:/picture.bmp`
- `./rle -d ./picture.bmp`

## TODO
- [x] feature: CLI
- [x] Improve performance
- [ ] feature: Multi-Threading
