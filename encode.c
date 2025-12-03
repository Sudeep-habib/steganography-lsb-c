#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "common.h"
#include "types.h"
#include <unistd.h>

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    //printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if(encInfo->fptr_src_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->src_image_fname);

	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if(encInfo->fptr_secret == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->secret_fname);

	return e_failure;
    }
    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if(encInfo->fptr_stego_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->stego_image_fname);

	return e_failure;
    }

    //No failure return e_success
    return e_success;
}

// To check operation type whether user enetered option -e or -d
OperationType check_operation_type(char *argv[])
{
    if( !strcmp(argv[1], "-e") )
    {
	return e_encode;
    }
    else if(!strcmp(argv[1], "-d"))
    {
	return e_decode;
    }
    else
    {
	return e_unsupported;
    }
}

// To read and validate the arguments
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Checking argument is passed or not
    if(argv[2] != NULL)
    {
	// Checking file is BMP file or not
	char * ch = strstr(argv[2], ".bmp");
	if(ch == NULL)
	{
	    return e_failure;
	}
	else
	{
	    encInfo -> src_image_fname = argv[2];
	}
    }
    else
    {
	return e_failure;
    }

    // Checking argument is passed or not
    if(argv[3] != NULL)
    {
	encInfo -> secret_fname = argv[3];
    }
    else
    {
	return e_failure;
    }

    // Checking argument is passed or not
    // If it is not passed then creating a new file name as stego.bmp
    if (argv[4] != NULL)
    {
	encInfo -> stego_image_fname = argv[4];
    }
    else
    {
	encInfo -> stego_image_fname = "stego.bmp";
    }
    return e_success;
}

// To get secret file size
uint get_file_size(FILE *fptr)
{
    // Seek file pointer to 0
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);
}

// To get a extension of a secret file
Status get_secret_file_extn(EncodeInfo *encInfo)
{
    int secret_fname_size = strlen(encInfo -> secret_fname);
    char fname[secret_fname_size + 1];
    strcpy(fname, encInfo -> secret_fname);
    strtok(fname, ".");
    char *  extn = strtok(NULL, ",");
    strcpy(encInfo -> extn_secret_file, extn);
    return e_success;
}

// check capacity of beautiful.bmp file
Status check_capacity(EncodeInfo *encInfo)
{

    // get image size for bmp and storing return value
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);

    // get file size and stroing return value
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);

    int total_bytes = 54 + strlen(MAGIC_STRING) * 8 + 32 + 32 + 32 + encInfo -> size_secret_file * 8;

    // Checking beautiful.bmp image file size is greater than secret file
    if(total_bytes <= encInfo -> image_capacity)
    {
	return e_success;
    }
    else
    {
	return e_failure;
    }
}

//header files copying
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char str[54];

    // Seek file pointer to 0
    fseek(fptr_src_image, 0, SEEK_SET);

    // Reading 54 bytes(headerfiles) from source file to arr
    fread(str, 54, 1, fptr_src_image);

    // Writing those 54 bytes(headerfiles) into destination image
    fwrite(str, 54, 1, fptr_dest_image); 
    return e_success;
}

// Function defnition for encoding data to output image file
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char str[8];
    for(int i = 0; i < size; i++)
    {
	fread(str, 8, 1, fptr_src_image);
	encode_byte_to_lsb(data[i], str);
	fwrite(str, 8, 1, fptr_stego_image);
    }
    return e_success;
}
// Function defnition for encoding magic string to output image file
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image((char*) magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

// Function defnition for encoding secret file extension size to output image file
Status encode_secret_file_extn_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char str[32];
    fread(str, 32, 1, fptr_src_image);
    encode_size_to_lsb(size, str);
    fwrite(str, 32, 1, fptr_stego_image);
    return e_success;
}

// Function defnition for encoding secret file extension to output image file
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image((char *)file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

// Function defnition for encoding secret file size
Status encode_secret_file_size(long int size, EncodeInfo *encInfo)
{
    char str[32];
    fread(str, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, str);
    fwrite(str, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

// Function defnition for encoding secret file data to output image file
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    char str[encInfo->size_secret_file];
    fread(str, encInfo->size_secret_file, 1, encInfo->fptr_secret);
    encode_data_to_image(str, strlen(str), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

//Function defintion for encoding each byte of magic string to lsb
Status encode_byte_to_lsb ( char data, char *image_buffer)
{
    int count = 0;
    for (int i = 7; i >= 0; i--)
    {
	if ((data >> i) & 1)
	{
	    image_buffer[count] |= 1;
	}
	else
	{
	    image_buffer[count] &= ~1;
	}
	count++;
    }
    return e_success;
}

//Function defnition for encoding size to lsb
Status encode_size_to_lsb (int size, char *image_buffer)
{
    int count = 0;
    for (int i = 31; i >= 0; i--)
    {
	if ((size >> i) & 1)
	{
	    image_buffer[count] |= 1;
	}
	else
	{
	    image_buffer[count] &= ~1;
	}
	count++;
    }
    return e_success;
}
//Function defnition for copying remaining data of beautiful.bmp file to output image file
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(( fread ( &ch, 1, 1, fptr_src) ) > 0)
    {
	fwrite( &ch, 1, 1, fptr_dest);
    }
    return e_success;
}

//Function encoding
Status do_encoding(EncodeInfo *encInfo)
{
    //Function call to check capacity function
    if(open_files(encInfo) == e_success)
    {
	    printf("1. Open files. Done\n");
        usleep(900000);
    }
    else
    {
	    printf(" Open files is failed\n");
	    return e_failure;
    }

	//Function call to check capacity function
	if(check_capacity(encInfo) == e_success)
	{
	    printf("2. Check Capacity. Done\n");
        usleep(800000);
    }
    else
	{
	    printf(" File Check capacity is failed\n");
        return e_failure;
	}
	//Function call for copying bmp header
	if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
	{
	    printf("3. Copying bmp header. Done\n");
        usleep(700000);
    }
    else
	{
	    printf(" Copying bmp header is failed\n");
        return e_failure;
	}
	//Function call to encode magic string
	if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
	{
	    printf("4. Encoded Magic string. Done\n");
        usleep(600000);
        strcpy(encInfo->extn_secret_file,strstr(encInfo->secret_fname,"."));
        printf("   Got secret file extension\n");
    }
    else
	{
	    printf("-> Magic string is not copied\n");
        return e_failure;
	}
	//Function call for encode secret file extention size
	if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
	{
		printf("5. Encoding Secret file extension size. Done\n");
    }
    else
	{
	    printf(" Encoding Secret file extention size is failed\n");
        return e_failure;
	}

	//Function call for Copying secret file extention
	if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
	{
	    printf("6. Secret file extention is encoded. Done\n");
    }
    else
	{
        printf(" Encoding Secret file extention is failed\n");
        return e_failure;
	}

    // Function call for encode secret file size
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo ) == e_success)
    {
    	printf("7. Secret file size is encoded. Done\n");
    }
    else
    {
    	printf(" Encoding Secret file size is failed\n");
        return e_failure;
	}

	//Function call to encode secret file data
	if(encode_secret_file_data(encInfo) == e_success)
	{
	    printf("8. Secret file data is encoded. Done\n");
    }
    else
	{
	    printf(" Encoding Secret file data is failed\n");
        return e_failure;
	}
    //Function call for copying remaining image data function
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
		printf("9. Remaining image data is copied. Done\n");
    }
    else
    {
    	printf("-> Remaining image data is not copied\n");
        return e_failure;
    }
    return e_success;
}
