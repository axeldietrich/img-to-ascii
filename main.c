#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize2.h"

unsigned char to_grayscale(unsigned char r, unsigned char g, unsigned char b) {
	return (unsigned char)(0.2126 * r + 0.7152 * g + b * 0.0722);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s image_file\n", argv[0]);
	}

	char *filename = argv[1];
	float max_width = 316.f;
	float max_height = 200.f;
	int width, height, channels;

	unsigned char *img = stbi_load(filename, &width, &height, &channels, 3);
	if (!img) {
		printf("Failed to load image: %s\n", filename);
		return 1;
	}
	float scale_x = max_width / width;
	float scale_y = max_height / height;
	float scale = (scale_x < scale_y) ? scale_x : scale_y;

	int new_width = width * scale;
	float char_aspect = 0.5f;
	int new_height = (int)(height * scale * char_aspect);

	unsigned char *resized_img = malloc(new_width * new_height * 3);
	if (resized_img == NULL) {
		printf("Memory allocation of resize image failed\n");
		return 1;
	}
	stbir_resize_uint8_srgb(img, width, height, 0, resized_img, new_width, new_height, 0, 3);

	unsigned char *grayscale_image = malloc(new_width * new_height);
	if (grayscale_image == NULL) {
		printf("Memory allocation for grayscale failed\n");
		return 1;
	}

	char *ascii_characters = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
	int ascii_length = strlen(ascii_characters);

	int total_size = new_width * new_height * 3;
	for (int i = 0, j = 0; i < total_size; i += 3, j++) {
		grayscale_image[j] = to_grayscale(resized_img[i], resized_img[i + 1], resized_img[i +2]);
		float val = grayscale_image[j] / 255.0f;
		float c = 1.8f;
		float b = 0.0f;
		val = (val - 0.5f) * c + 0.5f + b;
		if (val < 0.0f) val = 0.0f;
		if (val > 1.0f) val = 1.0f;
		grayscale_image[j] = (unsigned char)(val * 255);
	}

	for (int y = 0; y < new_height; y++) {
		for (int x = 0; x < new_width; x++) {
			int pixel_value = grayscale_image[y * new_width + x];
			int index = (ascii_length - 1) - (pixel_value * (ascii_length - 1) / 255);
			putchar(ascii_characters[index]);
		}
		printf("\n");
	}

	free(grayscale_image);
	free(resized_img);
	return 0;
}