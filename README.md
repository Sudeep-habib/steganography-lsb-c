# LSB Image Steganography in C
A complete implementation of Least Significant Bit (LSB) Steganography in C for hiding and extracting
ANY secret file inside a 24-bit BMP image.

## FEATURES
- Hide any file inside a BMP image (encoding)
- Extract the original file back (decoding)a
- Supports .txt, .c, .sh, .json, .bin and more
- Encodes magic string, extension, size, and file data
- Safe memory handling and modular code design

## COMPILATION
gcc encode.c decode.c test_encode.c common.c -o a.out
Do NOT compile secret files (program.c, script.sh). They are input files, not project code.

## USAGE

### 1) Encoding
./a.out -e [output_stego.bmp]
Examples:
./a.out -e beautiful.bmp secret.txt
./a.out -e beautiful.bmp secrets/program.c stego.c.bmp
./a.out -e beautiful.bmp secrets/script.sh stego.sh.bmp


### 2) Decoding
./a.out -d [output_secret_filename]
Examples:
./a.out -d stego.bmp
./a.out -d stego.c.bmp
./a.out -d stego.sh.bmp extracted_script.sh


## TEST CASES

Basic:

./a.out -e beautiful.bmp secret.txt
./a.out -d stego.bmp

C file:

./a.out -e beautiful.bmp secrets/program.c stego.c.bmp
./a.out -d stego.c.bmp

Shell script:

./a.out -e beautiful.bmp secrets/script.sh stego.sh.bmp
./a.out -d stego.sh.bmp

## INTERNAL ENCODING FORMAT

1. Magic String
2. Extension Size (32-bit)
3. Extension Characters
4. Secret File Size (32-bit)
5. Secret File Data (LSB embedded)


## REQUIREMENTS
- GCC compiler
- 24-bit uncompressed BMP
- Linux recommended

## CONCLUSION
A complete implementation of LSB steganography demonstrating bitwise operations, file I/O, BMP
manipulation, and modular C programming.