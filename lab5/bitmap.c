#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"

/*
 * Read in the location of the pixel array, the image width, and the image 
 * height in the given bitmap file.
 */
void read_bitmap_metadata(FILE *image, int *pixel_array_offset, int *width, int *height)
{
    // First seeking the array offset value that starts at 10
    fseek(image, 10, SEEK_SET);
    int err_offset = fread(pixel_array_offset, 4, 1, image);
    if (err_offset != 1)
    {
        exit(1);
    }
    // Next, We seek the width that starts at 18
    fseek(image, 18, SEEK_SET);
    int err_width = fread(width, 4, 1, image);
    if (err_width != 1)
    {
        exit(1);
    }
    // Finally we seek the height that starts at 22
    fseek(image, 22, SEEK_SET);
    int err_height = fread(height, 4, 1, image);
    if (err_height != 1)
    {
        exit(1);
    }
}

/*
 * Read in pixel array by following these instructions:
 *
 * 1. First, allocate space for m `struct pixel *` values, where m is the 
 *    height of the image.  Each pointer will eventually point to one row of 
 *    pixel data.
 * 2. For each pointer you just allocated, initialize it to point to 
 *    heap-allocated space for an entire row of pixel data.
 * 3. Use the given file and pixel_array_offset to initialize the actual 
 *    struct pixel values. Assume that `sizeof(struct pixel) == 3`, which is 
 *    consistent with the bitmap file format.
 *    NOTE: We've tested this assumption on the Teaching Lab machines, but 
 *    if you're trying to work on your own computer, we strongly recommend 
 *    checking this assumption!
 * 4. Return the address of the first `struct pixel *` you initialized.
 */
struct pixel **read_pixel_array(FILE *image, int pixel_array_offset, int width, int height)
{
    // Allocating space for struct pixel
    struct pixel **temp = (struct pixel **)malloc(sizeof(struct pixel *) * height);
    // Pointing the pointer to one row (= width) of pixel data
    for (int j = 0; j < height; j++)
    {
        temp[j] = (struct pixel *)malloc(sizeof(struct pixel) * width);
    }

    // Initializing actual struct pixel values.
    fseek(image, pixel_array_offset, SEEK_SET);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Reading B, G, R values
            int error_blue = fread(&(temp[i][j].blue), 1, 1, image);
            if (error_blue != 1)
            {
                exit(1);
            }
            int error_green = fread(&(temp[i][j].green), 1, 1, image);
            if (error_green != 1)
            {
                exit(1);
            }
            int error_red = fread(&(temp[i][j].red), 1, 1, image);
            if (error_red != 1)
            {
                exit(1);
            }
        }
    }
    return temp;
}

/*
 * Print the blue, green, and red colour values of a pixel.
 * You don't need to change this function.
 */
void print_pixel(struct pixel p)
{
    printf("(%u, %u, %u)\n", p.blue, p.green, p.red);
}

