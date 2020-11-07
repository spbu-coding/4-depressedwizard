#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include "qdbmp.h"
#include "bmp.h"


void make_negative_24 (int height, int width, RGBTRIPLE image[height][width]);
void make_negative_8 (RGBTRIPLE_PALLETE colour_pallete[]);

int main(int argc, char *argv[])
{

    // Get input from the user and process the desired options
    int colour_format = 0;
    int algorithm = 0;

    const struct option colour_options[] =
    {
        {"8", 0, &colour_format, 8},
        {"24", 0, &colour_format, 24},
        {0, 0, 0, 0}
    };

    const struct option algorithm_options[] =
    {
        {"mine", 0, &algorithm, 1},
        {"theirs", 0, &algorithm, 2},
        {0, 0, 0, 0}
    };

    getopt_long(argc, argv, "", colour_options, NULL);
    getopt_long(argc, argv, "", algorithm_options, NULL);

    if (colour_format == 0 || algorithm == 0 || argc != 5)
    {
        fprintf(stderr, "Error. Correct usage: ./converter --ColourFormat --Algorithm inputfile.bmp outputfile.bmp");
        return 1;
    }

    // Remembering filenames
    char *original_file = argv[optind];
    char *new_file = argv[optind + 1];

    if (algorithm == 1)
    {
        FILE *inputfile = fopen(original_file, "r");
        if (inputfile == NULL)
        {
            fprintf(stderr, "Could not open %s.\n", original_file);
            return 2;
        }

        FILE *outputfile = fopen(new_file, "w");
        if (inputfile == NULL)
        {
            fprintf(stderr, "Could not open %s.\n", new_file);
            return 3;
        }

        // Read inputfile's BITMAPFILEHEADER
        BITMAPFILEHEADER bf;
        fread(&bf, sizeof(BITMAPFILEHEADER), 1, inputfile);

        // Read inputfile's BITMAPINFOHEADER
        BITMAPINFOHEADER bi;
        fread(&bi, sizeof(BITMAPINFOHEADER), 1, inputfile);

        // Ensuring inputfile is (likely) a 24-bit or an 8-bit uncompressed BMP 4.0
        if (bf.bfType != 0x4d42 || bi.biSize != 40 ||
            (bi.biBitCount != 8 && bi.biBitCount != 24) || bi.biCompression != 0)
        {
            fclose(outputfile);
            fclose(inputfile);
            fprintf(stderr, "Unsupported file format.\n");
            return -1;
        }

        int height = abs(bi.biHeight);
        int width = bi.biWidth;

        if (bi.biBitCount == 8)
        {
            RGBTRIPLE_PALLETE palette[255];
            fread(palette, sizeof(palette), 1, inputfile);

            BYTE(*image)[width] = calloc(height, width * sizeof(BYTE));
            if (image == NULL)
            {
                fprintf(stderr, "Not enough memory to store image.\n");
                fclose(outputfile);
                fclose(inputfile);
                return 4;
            }

            // Determine padding for scanlines
            int padding = (4 - (width * sizeof(BYTE)) % 4) % 4;

            // Iterate over inputfile's scanlines
            for (int i = 0; i < height; i++)
            {
                // Read row into pixel array
                fread(image[i], sizeof(BYTE), width, inputfile);

                // Skip over padding
                fseek(inputfile, padding, SEEK_CUR);
            }

            make_negative_8(palette);

            // Write outputfile's BITMAPFILEHEADER
            fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outputfile);

            // Write outputfile's BITMAPINFOHEADER
            fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outputfile);

            // Write outputfile's colour pallete
            fwrite(&palette, sizeof(palette), 1, outputfile);

            // Write new pixels to outfile
            for (int i = 0; i < height; i++)
            {
                // Write row to outfile
                fwrite(image[i], sizeof(BYTE), width, outputfile);

                // Write padding at end of row
                for (int k = 0; k < padding; k++)
                {
                    fputc(0x00, outputfile);
                }
            }
            free(image);
        }
        else if (bi.biBitCount == 24)
        {
            RGBTRIPLE(*image)[width] = calloc(height, width * sizeof(RGBTRIPLE));
            if (image == NULL)
            {
                fprintf(stderr, "Not enough memory to store image.\n");
                fclose(outputfile);
                fclose(inputfile);
                return 4;
            }

            // Determine padding for scanlines
            int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

            // Iterate over inputfile's scanlines
            for (int i = 0; i < height; i++)
            {
                // Read row into pixel array
                fread(image[i], sizeof(RGBTRIPLE), width, inputfile);

                // Skip over padding
                fseek(inputfile, padding, SEEK_CUR);
            }

            make_negative_24(height, width, image);

            // Write outfile's BITMAPFILEHEADER
            fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outputfile);

            // Write outfile's BITMAPINFOHEADER
            fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outputfile);

            // Write new pixels to outfile
            for (int i = 0; i < height; i++)
            {
                // Write row to outfile
                fwrite(image[i], sizeof(RGBTRIPLE), width, outputfile);

                // Write padding at end of row
                for (int k = 0; k < padding; k++)
                {
                    fputc(0x00, outputfile);
                }
            }
            free(image);
        }

    // Close infile
    fclose(inputfile);

    // Close outfile
    fclose(outputfile);
    }
    else if (algorithm == 2)
    {
        // Read the bmp file into memory
        BMP *bmp = BMP_ReadFile (original_file);
        BMP_CHECK_ERROR (stdout, - 1);
        
        // Get the image dimensions and bit depth
        UINT width = BMP_GetWidth (bmp);
        UINT height = BMP_GetHeight (bmp);
        USHORT depth = BMP_GetDepth (bmp);
        
        UCHAR R, G, B;
        
        if (depth == 24)
        {
            // Iterate through the image's pixels
            for (UINT i = 0; i < height; i++)
            {
                for (UINT j = 0; j < width; j++)
                {
                    BMP_GetPixelRGB (bmp, i, j, &R, &G, &B);
                    BMP_SetPixelRGB (bmp, i, j, 255 - R, 255 - G, 255 - B);
                }
            }
        }
        else if (depth == 8)
        {
            UCHAR value;
            for (UINT i = 0; i < 255; i++)
            {
                BMP_GetPaletteColor (bmp, i, &R, &G, &B);
                BMP_SetPaletteColor (bmp, i, ~R, ~G, ~B);
            }
        }
        
        // Save the changes to the file
        BMP_WriteFile(bmp, new_file);
        BMP_CHECK_ERROR(stdout, - 2 );
        
        // Free memory used by the image
        BMP_Free(bmp);
    }
    return 0;
}

void make_negative_24 (int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j].rgbtRed = 255 - image[i][j].rgbtRed;
            image[i][j].rgbtBlue = 255 - image[i][j].rgbtBlue;
            image[i][j].rgbtGreen = 255 - image[i][j].rgbtGreen;
        }
    }
    return;
}

void make_negative_8 (RGBTRIPLE_PALLETE colour_pallete[])
{
    RGBTRIPLE_PALLETE placeholder_pallete[255];
    for (int i = 0; i < 256; i++)
    {
        placeholder_pallete[i].rgbtRed = ~colour_pallete[i].rgbtRed;
        placeholder_pallete[i].rgbtGreen = ~colour_pallete[i].rgbtGreen;
        placeholder_pallete[i].rgbtBlue = ~colour_pallete[i].rgbtBlue;
    }
    for (int j = 0; j < 256; j++)
    {
        colour_pallete[j].rgbtRed = placeholder_pallete[j].rgbtRed;
        colour_pallete[j].rgbtGreen = placeholder_pallete[j].rgbtGreen;
        colour_pallete[j].rgbtBlue = placeholder_pallete[j].rgbtBlue;
    }
    return;
}