# The Lempel-Ziv Algorithm
The Lempel-Ziv Algorithm is a lossless data compression technique that forms the basis for many widely used compression formats, like ZIP, gzip, and PNG. Developed by Abraham Lempel and Jacob Ziv in the late 1970s, it works by identifying and replacing repeated patterns in data with shorter references, reducing the overall size of the data without losing any information.

## How It Works
The LZ77 File Compressor operates using the sliding window technique of the LZ77 algorithm:
1. **Input Processing**: The program reads the input file into a sliding window, split into a search buffer (past data) and a lookahead buffer (upcoming data).
2. **Pattern Matching**: It scans the search buffer for the longest substring that matches the start of the lookahead buffer.
3. **Encoding**:
   - If a match is found, it outputs a triple `(offset, length, next_char)`, where `offset` is the distance to the match, `length` is the number of matching characters, and `next_char` is the character following the match.
   - If no match is found, it outputs `(0, 0, current_char)` for the current character.
4. **Output**: The sequence of triples is written to a compressed file, optionally with additional encoding (e.g., bit-packing) to optimize size.
5. **Decompression**: The program reverses the process, reconstructing the original file by resolving references and copying literal characters.

The tool supports both compression (`compress <input> <output>`) and decompression (`decompress <input> <output>`) modes.

## When It's Useful
- **Text Files**: Excels at compressing text with repetitive patterns, such as logs, source code, or configuration files.
- **Archival**: Ideal for reducing storage needs for backups or transferring files over networks.
- **Embedded Systems**: Suitable for environments with limited memory, as LZ77 is lightweight and fast.
- **Pre-processing**: Useful as a first stage in pipelines where further compression (e.g., with Huffman coding) is applied, like in ZIP or gzip.

## Advantages
- **Lossless**: Guarantees exact reconstruction of the original file with no data loss.
- **Fast**: Offers quick compression and decompression, making it suitable for real-time applications.
- **Adaptive**: Automatically adjusts to data patterns without requiring pre-defined dictionaries.
- **Simple Implementation**: Easy to integrate into other systems due to its straightforward algorithm.
- **Low Memory Footprint**: Operates with a fixed-size sliding window, efficient for resource-constrained environments.

## Limitations
- **Compression Ratio**: Less effective for files with little repetition (e.g., already compressed files like JPEGs or MP4s), as it relies on redundant patterns.
- **Window Size Trade-off**: Smaller windows reduce memory use but may miss longer repetitions, lowering compression efficiency. Larger windows increase memory and computation needs.
- **Single-Pass**: Only examines local patterns within the sliding window, potentially missing global redundancies compared to algorithms like LZ78 or LZW.
- **No Entropy Coding**: Without additional encoding (e.g., Huffman), compression may not be optimal for some data types.
- **Not Ideal for Streaming**: The fixed window limits its ability to handle continuous data streams compared to modern algorithms like Brotli.

## Compile
In order to compile the source code, you can use `gcc` in this manner:
```
gcc ./main.c -o lz77
```

g++:
```
g++ ./main.c -o lz77
```

clang:
```
clang ./main.c -o lz77
```

## Usage
- Compressing a file: `./lz77 -c <file_path>`. 
- Decompressing a file: `./lz77 -d <file_path>`.

Example:
- `./lz77 -c c:/picture.bmp`
- `./lz77 -d ./picture.bmp`

## Test
For testing the program, I have written a test in c, which looks for every file in `test_files` directory and does a compression, decompression and comparison process for each file then prints the result. In order to test this, create `test_files` directory and put some files (i.e bitmap image file) in it, then compile `test.c` or if you're on windows `test-windows.c` and run it.

```
[TEST 1-1]: Compressing pic-1024.bmp
Processing: 3145782/3145782 -> 147135
Compressed successfully!
        1 file(s) moved.
[TEST 1-2]: Decompressing pic-1024.bmp.lz7
Processing: 147705/147321 -> 3145782
Decompressed successfully!
[TEST 1-3]: Verifying pic-1024.bmp
---[TEST 1]: PASSED - Decompressed file matches original

[TEST 2-1]: Compressing pic-256.bmp
Processing: 196662/196662 -> 10455
Compressed successfully!
        1 file(s) moved.
[TEST 2-2]: Decompressing pic-256.bmp.lz7
Processing: 10512/10482 -> 196662
Decompressed successfully!
[TEST 2-3]: Verifying pic-256.bmp
---[TEST 2]: PASSED - Decompressed file matches original

[TEST 3-1]: Compressing pic-64.bmp
Processing: 12342/12342 -> 3570
Compressed successfully!
        1 file(s) moved.
[TEST 3-2]: Decompressing pic-64.bmp.lz7
Processing: 3660/3651 -> 12342
Decompressed successfully!
[TEST 3-3]: Verifying pic-64.bmp
---[TEST 3]: PASSED - Decompressed file matches original

Testing complete.
```

## TODO
- [ ] feature: CLI
- [ ] Improve performance
- [ ] feature: Multi-Threading
