#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "common.h"

int main(int argc, char *argv[])
{
    // Declaring encoding structure variable
    EncodeInfo encInfo;

    // Declaring decoding structure variable
    DecodeInfo decInfo;

    // To check any other arguments passed along with ./a.out file or not
    if (argc < 3)
    {
        fprintf(stderr, "ERROR: Insufficient arguments.\n");
        fprintf(stderr, "USAGE:\n");
        fprintf(stderr, "  Encoding:\n");
        fprintf(stderr, "    %s -e <input.bmp> <secret_file> [output_stego.bmp]\n", argv[0]);
        fprintf(stderr, "  Decoding:\n");
        fprintf(stderr, "    %s -d <stego.bmp> [output_secret_filename]\n", argv[0]);
        return 1;
    }

    OperationType e_d_un_type = check_operation_type(argv);

    // Return type is e_encode
    if (e_d_un_type == e_encode)
    {
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("\n\t\t<< Encoding Process >>\t\t\n\n");

            // Encoding process begins
            if (do_encoding(&encInfo) == e_success)
            {
                printf("\n\t<< Encoding successfully done >>\t\n\n");
            }
            else
            {
                printf("ERROR: Failed to encode.\n");
            }
        }
        else
        {
            printf("ERROR: Read and validation failed.\n");
        }
    }
    // Return type is e_decode
    else if (e_d_un_type == e_decode)
    {
        if (read_and_validate_decode_args(argv, &decInfo) == d_success)
        {
            printf("\n\t\t<< Decoding Process >>\t\t\n\n");

            // Decoding process begins
            if (do_decoding(&decInfo) == d_success)
            {
                printf("\n\t<< Decoding successfully done >>\t\n\n");
            }
            else
            {
                printf("ERROR: Failed to decode.\n");
            }
        }
        else
        {
            printf("ERROR: Read and validation failed.\n");
        }
    }
    // Return type is e_unsupported
    else
    {
        printf("ERROR: Please pass sufficient number of arguments.\n");
    }

    return 0;
}
