#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include <stdlib.h>

// Function definition for read and validate decode args
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // argv[0] = ./a.out
    // argv[1] = -d
    // argv[2] = stego image name (must end with .bmp)
    // argv[3] = optional output file name

    if (argv[2] == NULL)
    {
        return d_failure;
    }

    // Check last extension is .bmp
    char *dot = strrchr(argv[2], '.');
    if (dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        return d_failure;
    }

    decInfo->decode_src_image_fname = argv[2];

    if (argv[3] != NULL)
        decInfo->decode_secret_fname = argv[3];
    else
        decInfo->decode_secret_fname = "decode.txt";  // ok for now

    return d_success;
}

// Function definition for open files for decoding
Status open_files_dec(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo -> fptr_d_src_image = fopen(decInfo -> decode_src_image_fname, "r");
    // Do Error handling
    if (decInfo -> fptr_d_src_image == NULL)
    {
	    perror("fopen");
	    fprintf(stderr, "ERROR : Unable to open file %s\n", decInfo -> decode_src_image_fname);
	    return d_failure;
    }
    // Destination file
    decInfo -> fptr_d_secret = fopen(decInfo -> decode_secret_fname, "w");
    // Do Error handling
    if (decInfo -> fptr_d_secret == NULL)
    {
	    perror("fopen");
	    fprintf(stderr, "ERROR : Unable to open file %s\n", decInfo -> decode_secret_fname);
	    return d_failure;
    }
    // If no failure then return e_success
    return d_success;
}

// Function definition for decode magic string
Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo -> fptr_d_src_image, 54, SEEK_SET);
    int len = strlen(MAGIC_STRING);
    decInfo -> magic_data = malloc( strlen(MAGIC_STRING) + 1 );
    decode_data_from_image( strlen(MAGIC_STRING), decInfo -> fptr_d_src_image, decInfo );
    decInfo -> magic_data[len] = '\0';
    if (strcmp(decInfo -> magic_data, MAGIC_STRING) == 0)
    {
	    return d_success;
    }
    else
    {
	    return d_failure;
    }
}

// Function definition for decoding data fom image
Status decode_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo)
{
    int i;
    char str[8];
    for (i = 0; i < size; i++)
    {
	    fread(str, 8, sizeof(char), fptr_d_src_image);
	    decode_byte_from_lsb(&decInfo -> magic_data[i], str);
    }
    return d_success;
}

// Function definition for decode byte from lsb
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    int bit = 7;
    unsigned char ch = 0x00;
    for (int i = 0; i < 8; i++)
    {
	    ch = ((image_buffer[i] & 0x01) << bit--) | ch;
    }
    *data = ch;
    return d_success;
}

// Function definition for decode file extn size
Status decode_file_extn_size(int *size, FILE *fptr_d_src_image)
{
    char str[32];

    // Try to read 32 bytes. Even if we get less, we'll still attempt decode.
    size_t n = fread(str, 32, sizeof(char), fptr_d_src_image);
    if (n == 0)
    {
        // nothing read at all → real failure
        return d_failure;
    }

    decode_size_from_lsb(str, size);
    return d_success;
}



// Function definition decode size from lsb
Status decode_size_from_lsb(char *buffer, int *size)
{
    int j = 31;
    int num = 0x00;
    for (int i = 0; i < 32; i++)
    {
	    num = ((buffer[i] & 0x01) << j--) | num;
    }
    *size = num;
    return d_success;
}

// Function definition for decode secret file extn
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    int extn_size;

    // 1. Get how many characters the extension has
    if (decode_file_extn_size(&extn_size, decInfo->fptr_d_src_image) != d_success)
        return d_failure;

    // 2. Allocate memory for extension (e.g. ".c", ".sh", ".txt")
    decInfo->decode_extn_secret_file = malloc(extn_size + 1);
    if (decInfo->decode_extn_secret_file == NULL)
        return d_failure;

    // 3. Decode the extension characters from the image
    if (decode_extension_data_from_image(extn_size,
                                         decInfo->fptr_d_src_image,
                                         decInfo) != d_success)
        return d_failure;

    // 4. Null-terminate
    decInfo->decode_extn_secret_file[extn_size] = '\0';

    // No forced ".txt" comparison – accept any extension
    return d_success;
}


// Function definition decode extension data from image
Status decode_extension_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo)
{
    char buffer[8];
    for (int i = 0; i < size; i++)
    {
	    fread(buffer, 8, sizeof(char), fptr_d_src_image);
        decode_byte_from_lsb(&decInfo->decode_extn_secret_file[i], buffer);
    }
    return d_success;
}

// Function definition for decode secret file size
Status decode_secret_file_size(int file_size, DecodeInfo *decInfo)
{
    char str[32];
    fread(str, 32, sizeof(char), decInfo -> fptr_d_src_image);
    decode_size_from_lsb(str, &file_size);
    decInfo -> size_secret_file = file_size;
    return d_success;
}

// Function definition for decode secret file data
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;
    char buffer[8];

    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(buffer, 8, sizeof(char), decInfo->fptr_d_src_image);
        decode_byte_from_lsb(&ch, buffer);
        fputc(ch, decInfo->fptr_d_secret);
    }
    return d_success;
}


// Function definition for do decoding
Status do_decoding(DecodeInfo *decInfo)
{
    //Calling open files function
    if (open_files_dec(decInfo) == d_success)
    {
	    printf("1. Open files. Done\n");
    }
    else
    {
	    printf(" Open files is a failure\n");
	    return d_failure;
    }

	// Calling magic string function
	if (decode_magic_string(decInfo) == d_success)
	{
	    printf("2. Decoded magic string. Done\n");
    }
    else
	{
	    printf(" Decoding of magic string is a failure\n");
	    return d_failure;
	}

// Decode extension (size + actual characters)
if (decode_secret_file_extn(decInfo) == d_success)
{
    printf("3. Decoded Secret File Extension: %s. Done\n",
           decInfo->decode_extn_secret_file);
}
else
{
    printf("Decode of Secret file extension is a failure\n");
    return d_failure;
}

		    
	// Calling secret file size function
	if (decode_secret_file_size(decInfo -> size_secret_file, decInfo) == d_success)
    {
		printf("5. Decoded secret file size. Done\n");
    }
    else
	{
		printf(" Decode of secret file size is a failure\n");
		return d_failure;
	}

	// Calling secret file data function
	if (decode_secret_file_data(decInfo) == d_success)
	{
	    printf("6. Decoded secret file data. Done\n");
	}
	else
	{
        printf(" Decoding of secret file data is a failure\n");
        return d_failure;  
	}

	return d_success;
}
