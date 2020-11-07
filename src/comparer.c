#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "bmp.h"


int main(int argc, char *argv[])
{
    // Ensure correct input count from the user
    if (argc != 3)
    {
        fprintf(stderr, "Error. Correct usage: ./comparer inputfile.bmp outputfile.bmp");
        return 1;
    }

    // Remember filenames
    char *first_file = argv[1];
    char *second_file = argv[2];

    // Read the file into memory
    FILE *inputfile_1 = fopen(first_file, "r");
    if (inputfile_1 == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", first_file);
        return 2;
    }

    FILE *inputfile_2 = fopen(second_file, "r");
    if (inputfile_2 == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", second_file);
        return 3;
    }

    // Read inputfile_1's and inputfile_2's BITMAPFILEHEADER
    BITMAPFILEHEADER bf1, bf2;
    fread(&bf1, sizeof(BITMAPFILEHEADER), 1, inputfile_1);
    fread(&bf2, sizeof(BITMAPFILEHEADER), 1, inputfile_2);

    // Read inputfile_1's and inputfile_2's BITMAPINFOHEADER
    BITMAPINFOHEADER bi1, bi2;
    fread(&bi1, sizeof(BITMAPINFOHEADER), 1, inputfile_1);
    fread(&bi2, sizeof(BITMAPINFOHEADER), 1, inputfile_2);

    // Ensure inputfiles are (likely) 24-bit or 8-bit uncompressed BMP 4.0
    if (bf1.bfType != 0x4d42 || bi1.biSize != 40 ||
        (bi1.biBitCount != 8 && bi1.biBitCount != 24) || bi1.biCompression != 0
        ||
        bf2.bfType != 0x4d42 || bi2.biSize != 40 ||
        (bi2.biBitCount != 8 && bi2.biBitCount != 24) || bi2.biCompression != 0)
    {
        fclose(inputfile_1);
        fclose(inputfile_2);
        fprintf(stderr, "Unsupported file format.\n");
        return -1;
    }

    // Make sure they can be compared
    if (bi1.biBitCount != bi2.biBitCount || bi1.biHeight != bi1.biHeight || bi1.biWidth != bi2.biWidth)
    {
        fclose(inputfile_1);
        fclose(inputfile_2);
        fprintf(stderr, "Error: file don't share the same bitcount/height/width.\n");
        return -2;
    }
    
    if (bi1.biBitCount == 24)
    {
        int height = abs(bi1.biHeight);
        int width = bi1.biWidth;
        
        RGBTRIPLE(*image1)[width] = calloc(height, width * sizeof(RGBTRIPLE));
        RGBTRIPLE(*image2)[width] = calloc(height, width * sizeof(RGBTRIPLE));
            if (image1 == NULL || image2 == NULL)
            {
                fprintf(stderr, "Not enough memory to store image.\n");
                fclose(inputfile_1);
                fclose(inputfile_2);
                return 4;
            }

            // Determine padding for scanlines
            int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

            // Iterate over inputfile's scanlines
            for (int i = 0; i < height; i++)
            {
                // Read row into pixel array
                fread(image1[i], sizeof(RGBTRIPLE), width, inputfile_1);
                fread(image2[i], sizeof(RGBTRIPLE), width, inputfile_2);

                // Skip over padding
                fseek(inputfile_1, padding, SEEK_CUR);
                fseek(inputfile_2, padding, SEEK_CUR);
            }
        
        int counter = 0;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (image1[i][j].rgbtRed == image2[i][j].rgbtRed &&
                image1[i][j].rgbtGreen == image2[i][j].rgbtGreen &&
                image1[i][j].rgbtBlue == image2[i][j].rgbtBlue)
                {
                    continue;
                }
                else
                {
                    counter++;
                    fprintf(stderr, "%d %d\n", i, j);
                    
                    if (counter == 100)
                    {
                        break;
                    }
                }
            }
            
            if (counter == 100)
            {
                break;
            }
        }
        
        if (counter == 0)
        {
            fprintf(stderr, "The images are identical\n");
        }
    }
    else if (bi1.biBitCount == 8)
    {
        int pallete1[255], pallete2[255];
        fread(&pallete1, sizeof(pallete1), 1, inputfile_1);
        fread(&pallete2, sizeof(pallete2), 1, inputfile_2);
        
        int height = abs(bi1.biHeight);
        int width = bi1.biWidth;
        
        BYTE(*image1)[width] = calloc(height, width * sizeof(BYTE));
        BYTE(*image2)[width] = calloc(height, width * sizeof(BYTE));
            if (image1 == NULL || image2 == NULL)
            {
                fprintf(stderr, "Not enough memory to store image.\n");
                fclose(inputfile_1);
                fclose(inputfile_2);
                return 4;
            }

            // Determine padding for scanlines
            int padding = (4 - (width * sizeof(BYTE)) % 4) % 4;

            // Iterate over inputfile's scanlines
            for (int i = 0; i < height; i++)
            {
                // Read row into pixel array
                fread(image1[i], sizeof(BYTE), width, inputfile_1);
                fread(image2[i], sizeof(BYTE), width, inputfile_2);

                // Skip over padding
                fseek(inputfile_1, padding, SEEK_CUR);
                fseek(inputfile_2, padding, SEEK_CUR);
            }
        
        int counter = 0;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (image1[i][j] == image2[i][j])
                {
                    continue;
                }
                else
                {
                    counter++;
                    fprintf(stderr, "%d %d\n", i, j);
                    
                    if (counter == 100)
                    {
                        break;
                    }
                }
            }
            
            if (counter == 100)
            {
                break;
            }
        }
        if (counter == 0)
        {
            fprintf(stderr, "The images are identical\n");
        }
    }
    return 0;
}