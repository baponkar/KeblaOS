#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <png.h>

void read_png_file(const char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        fprintf(stderr, "Error creating PNG read struct\n");
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        fprintf(stderr, "Error creating PNG info struct\n");
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, (png_infopp)NULL);
        fclose(fp);
        fprintf(stderr, "Error during PNG creation\n");
        return;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);
    png_byte color_type = png_get_color_type(png, info);

    // Convert to RGBA if not already
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_gray_1_2_4_to_8(png);
    }
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }
    if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGBA) {
        png_set_rgb_to_gray_fixed(png, 1, 0, 0);
    }
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    png_set_compression_level(png, 9);
    
    // Remove the filter heuristic setting line
    // png_set_filter_heuristic(png, PNG_FILTER_HEURISTIC_UNIVERSAL, NULL, 0);

    png_read_update_info(png, info);

    // Allocate memory for the pixel data
    uint32_t* image_data = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    if (!image_data) {
        png_destroy_read_struct(&png, &info, (png_infopp)NULL);
        fclose(fp);
        fprintf(stderr, "Error allocating memory for image data\n");
        return;
    }

    png_bytep row = (png_bytep)malloc(png_get_rowbytes(png, info));
    for (int y = 0; y < height; y++) {
        png_read_row(png, row, NULL);
        for (int x = 0; x < width; x++) {
            png_byte* ptr = &(row[x * 4]);
            image_data[y * width + x] = (ptr[0] << 16) | (ptr[1] << 8) | ptr[2]; // Convert to RGB
        }
    }

    // Print the image data to stdout
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("0x%08X ", image_data[y * width + x]); // Print as hexadecimal
        }
        printf("\n");
    }

    // Clean up
    free(row);
    free(image_data);
    png_destroy_read_struct(&png, &info, (png_infopp)NULL);
    fclose(fp);
}

int main() {
    read_png_file("KeblaOS_icon_320x200x32.png"); // Replace with your image file
    return 0;
}
