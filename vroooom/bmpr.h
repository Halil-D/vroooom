#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
int civi_bmp_read_pixels(RGBTRIPLE *pixels, BITMAPINFOHEADER *bmiHeader, FILE *bmp_file) {
    // Seeking to BMP info header
    fseek(bmp_file, sizeof(BITMAPFILEHEADER), SEEK_SET);

    // Reading bitmap info headers
    int result = fread(bmiHeader, sizeof(BITMAPINFOHEADER), 1, bmp_file);

    // Calculating the size and allocating the row buffer
    int padded_row_size = (bmiHeader->biWidth * 3 + 3) & (~3); // Row size padded to multiple of 4 bytes
    unsigned char *row_buffer = (unsigned char *)malloc(padded_row_size);
    if (row_buffer == NULL) {
        printf("Error: Can't allocate row buffer for image encoding.\n");
        return 1; // -3, Can't allocate row buffer
    }


    // Reading pixel data row by row
    for (int y = 0; y < bmiHeader->biHeight; y++) {
        int buffer_index = 0;
        fread(row_buffer, sizeof(unsigned char), padded_row_size, bmp_file);
        int row_start = (bmiHeader->biHeight - y - 1) * bmiHeader->biWidth; // Pre-calculate row start index, height - y since BMPs are bottom-up
        for (int x = 0; x < bmiHeader->biWidth; x++) {
            int pixel_index = row_start + x;
            pixels[pixel_index].rgbtBlue  = row_buffer[buffer_index++];
            pixels[pixel_index].rgbtGreen = row_buffer[buffer_index++];
            pixels[pixel_index].rgbtRed = row_buffer[buffer_index++];
        }
    }
    free(row_buffer);

    return 0; // Return total bytes read
}