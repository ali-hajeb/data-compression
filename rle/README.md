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

### Linux

Use `make` command in the project's root directory to build the project.
```
$ make
```

### Windows

Run the following command in the project's root directory to build the project from the source.
```
gcc ./src/*.c main.c -Wall -g -o ./bin/rle
```

## Usage

Use the following flags:
- `-c`: compress file
- `-d`: decompress file
- `-o`: output file
- `-a`: use advance RLE algorithm
- `-b`: use basic RLE algorithm

Examples:
```
./rle -c ./pic.bmp # Compress pic.bmp and save it as pic.bmp.rle
```
```
./rlef -a -c ./pic.bmp -o ./pic.bmp.rle # Compress pic.bmp using advance algorithm and save it as pic.bmp.rle
```
Note: When you don't specify an output when using the `-d` flag to decompress a file, if the file extention is not `.rle`, it will decompress and **OVERWRITE** the original file.

## Test

For testing the program, I have written a test in c, which looks for every file in `test_files` directory and does a compression, decompression and comparison process for each file then prints the result. In order to test this, create `test_files` directory and put some files (i.e bitmap image file) in it, then compile `test.c` or if you're on windows `test-windows.c` and run it. also you can use `make test` command if you are on linux.
```
--------------------------|TEST 01|--------------------------
[TEST 1/6]: Compressing pic-1024.bmp
Processing: 3145782/3145782 bytes. -> 6103829 bytes (+94.03%)

        --->> Compression completed!

[TEST 2/6]: Compressing pic-1024.bmp (Advance mode)
Processing: 3145782/3145782 bytes. -> 3111521 bytes (-1.09%)

        --->> Compression completed!

[TEST 3/6]: Decompressing pic-1024.bmp.rle
Processing: 6103828/6103829 bytes. -> 3145782 bytes.

        --->> Decompression completed!

[TEST 4/6]: Decompressing a_pic-1024.bmp.rle
Processing: 6103828/6103829 bytes. -> 3145782 bytes.

        --->> Decompression completed!

[TEST 5/6]: Verifying pic-1024.bmp
--- [PASSED] - Decompressed file matches original
[TEST 6/6]: Verifying a_pic-1024.bmp
--- [PASSED] - Decompressed file matches original

--------------------------|TEST 02|--------------------------
[TEST 1/6]: Compressing pic-256.bmp
Processing: 196662/196662 bytes. -> 6087 bytes (-96.90%)

        --->> Compression completed!

[TEST 2/6]: Compressing pic-256.bmp (Advance mode)
Processing: 196662/196662 bytes. -> 7487 bytes (-96.19%)

        --->> Compression completed!

[TEST 3/6]: Decompressing pic-256.bmp.rle
Processing: 6086/6087 bytes. -> 196662 bytes.

        --->> Decompression completed!

[TEST 4/6]: Decompressing a_pic-256.bmp.rle
Processing: 6086/6087 bytes. -> 196662 bytes.

        --->> Decompression completed!

[TEST 5/6]: Verifying pic-256.bmp
--- [PASSED] - Decompressed file matches original
[TEST 6/6]: Verifying a_pic-256.bmp
--- [PASSED] - Decompressed file matches original

--------------------------|TEST 03|--------------------------
[TEST 1/6]: Compressing pic-64.bmp
Processing: 12342/12342 bytes. -> 529 bytes (-95.71%)

        --->> Compression completed!

[TEST 2/6]: Compressing pic-64.bmp (Advance mode)
Processing: 12342/12342 bytes. -> 574 bytes (-95.35%)

        --->> Compression completed!

[TEST 3/6]: Decompressing pic-64.bmp.rle
Processing: 528/529 bytes. -> 12342 bytes.

        --->> Decompression completed!

[TEST 4/6]: Decompressing a_pic-64.bmp.rle
Processing: 528/529 bytes. -> 12342 bytes.

        --->> Decompression completed!

[TEST 5/6]: Verifying pic-64.bmp
--- [PASSED] - Decompressed file matches original
[TEST 6/6]: Verifying a_pic-64.bmp
--- [PASSED] - Decompressed file matches original
```

## TODO
- [x] feature: CLI
- [x] Improve performance
- [ ] feature: Multi-Threading
