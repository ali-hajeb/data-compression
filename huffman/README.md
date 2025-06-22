## Huffman Encoding

Huffman encoding is a prefix-free, variable-length coding scheme used for lossless data compression. Developed by David A. Huffman in 1952, it assigns shorter binary codes to more frequent symbols and longer codes to less frequent ones, reducing the overall size of the encoded data. The algorithm ensures that no code is a prefix of another, allowing unambiguous decoding.

## How It Works

The Huffman encoding process involves the following steps:

1. **Frequency Analysis**: Read the input data and calculate the frequency of each symbol (e.g., characters in a text file).
2. **Build a Priority Queue**: Create a min-heap (or priority queue) where each node represents a symbol, its frequency, and pointers to left and right child nodes (initially null). Nodes with lower frequencies have higher priority.
3. **Construct the Huffman Tree**:
   - Repeatedly extract the two nodes with the lowest frequencies from the queue.
   - Create a new internal node with a frequency equal to the sum of the two extracted nodes' frequencies.
   - Set the two extracted nodes as the left and right children of the new node.
   - Insert the new node back into the queue.
   - Continue until only one node remains, which is the root of the Huffman tree.
4. **Generate Codes**: Traverse the Huffman tree from the root to each leaf (symbol). Assign a `0` for each left branch and a `1` for each right branch. The path to each leaf forms the binary code for the corresponding symbol.
5. **Encode the Data**: Replace each symbol in the input data with its corresponding Huffman code.
6. **Store or Transmit**: Save the encoded data along with metadata (e.g., the Huffman tree or frequency table) to enable decoding.

For decoding, the Huffman tree is reconstructed from the metadata, and the encoded binary string is traversed bit by bit to retrieve the original symbols.

## When It Is Useful

Huffman encoding is effective in scenarios where:
- Data contains symbols with unequal probabilities (e.g., text files where certain characters like 'e' or 'a' appear more frequently).
- Lossless compression is required, such as in file compression utilities (e.g., ZIP, GZIP).
- Storage or bandwidth is limited, and reducing data size is critical (e.g., network transmission, archival storage).
- It is used in multimedia formats like JPEG and MP3 for compressing metadata or entropy-coded data.

## Advantages

- **Efficient Compression**: Achieves near-optimal compression for symbols with skewed frequency distributions.
- **Lossless**: Guarantees exact reconstruction of the original data.
- **Prefix-Free Codes**: Eliminates ambiguity during decoding, requiring no delimiters between codes.
- **Adaptability**: Can be applied to any symbol set (e.g., bytes, pixels, or custom alphabets).

## Limitations

- **Overhead for Small Data**: The metadata (e.g., Huffman tree or frequency table) can be significant for small inputs, reducing compression efficiency.
- **Single-Pass Inefficiency**: Requires two passes (frequency analysis and encoding) for dynamic Huffman coding, or a precomputed table for static coding.
- **Not Ideal for Uniform Data**: Provides minimal compression when symbol frequencies are nearly equal.
- **Computational Complexity**: Building the Huffman tree and encoding/decoding can be slower than simpler algorithms for small datasets.
- **Vulnerability to Errors**: Bit errors in the encoded data can corrupt the entire output, as decoding relies on sequential bit traversal.

## Compile

### Linux

Use `make` command in the project's root directory to build the project.
```
$ make
```

### Windows

Run the following command in the project's root directory to build the project from the source.
```
gcc ./src/*.c main.c -Wall -g -o ./bin/huffman
```

## Usage

Use the following flags:
- `-c`: compress file
- `-d`: decompress file
- `-o`: output file

Examples:
```
./huffman -c ./pic.bmp # Compress pic.bmp and save it as pic.bmp.huf
```
```
./huffman -c ./pic.bmp -o ./pic.bmp.huf # Compress pic.bmp and save it as pic.bmp.huf
```
Note: When you don't specify an output when using the `-d` flag to decompress a file, if the file extention is not `.huf`, it will decompress and **OVERWRITE** the original file.

## Test

For testing the program, I have written a test in c, which looks for every file in `test_files` directory and does a compression, decompression and comparison process for each file then prints the result. In order to test this, create `test_files` directory and put some files (i.e bitmap image file) in it, then compile `test.c` or if you're on windows `test-windows.c` and run it. also you can use `make test` command if you are on linux.
```
--------------------------|TEST 01|--------------------------
[TEST 1-1]: Compressing pic-1024.bmp
Processing: 3145782/3145782 bytes. -> 1619593 bytes (48.52%)

--->> Compression completed!
[TEST 1-2]: Decompressing pic-1024.bmp.huf
Processing: 1619592/1619601 bytes. -> 3145782 bytes.

--->> Decompression completed!
[TEST 1-3]: Verifying pic-1024.bmp
--- [PASSED] - Decompressed file matches original

--------------------------|TEST 02|--------------------------
[TEST 2-1]: Compressing pic-256.bmp
Processing: 196662/196662 bytes. -> 102005 bytes (48.13%)

--->> Compression completed!
[TEST 2-2]: Decompressing pic-256.bmp.huf
Processing: 102005/102013 bytes. -> 196662 bytes.

--->> Decompression completed!
[TEST 2-3]: Verifying pic-256.bmp
--- [PASSED] - Decompressed file matches original

--------------------------|TEST 03|--------------------------
[TEST 3-1]: Compressing pic-64.bmp
Processing: 12342/12342 bytes. -> 8224 bytes (33.37%)

--->> Compression completed!
[TEST 3-2]: Decompressing pic-64.bmp.huf
Processing: 8223/8232 bytes. -> 12342 bytes.

--->> Decompression completed!
[TEST 3-3]: Verifying pic-64.bmp
--- [PASSED] - Decompressed file matches original

-------------------------------------------------------------
Testing complete.
```

## Compressed file structure

Every compressed file, containes a header, which includes these information.

- Table size - 1 Byte
- Frequency table - (Table size * 2) Bytes
- Encoded data
- Total bit count - last 8 bytes

In order to store the table size in 1 byte, the saved value is decreased by 1.

The Frequency table is a series of two-byte data which the first byte is the binary value and the second byte is the frequency of that value. All the frequency values are scaled down to fit in one byte.
