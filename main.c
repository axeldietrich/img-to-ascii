#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize2.h"
#define ASCII_MAP "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,\"^`'. "
#define ASCII_MAP_SIZE (sizeof(ASCII_MAP) -1)

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s image_file\n", argv[0]);
		return 1;
	}

	char *restrict filename = argv[1];
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

	unsigned char *restrict resized_img = malloc(new_width * new_height * 3);
	if (resized_img == NULL) {
		printf("Memory allocation of resize image failed\n");
		return 1;
	}
	stbir_resize_uint8_srgb(img, width, height, 0, resized_img, new_width, new_height, 0, 3);

	char grayscale_to_ascii[256];
	float scale_factor = (ASCII_MAP_SIZE - 1) / 255.0f;
	for (int i = 0; i < 256; i++) {
		grayscale_to_ascii[i] = ASCII_MAP[(int)((255 - i) * scale_factor)];
	}

	int total_size = new_width * new_height * 3;
	float c = 1.8f;
	float b = 0.0f;

	size_t output_size = (new_width + 1) * new_height;
	char *restrict output = malloc((new_width + 1) * new_height);
	if (output == NULL) {
		printf("Memory allocation for output buffer failed\n");
		return 1;
	}

	for (int i = 0, j = 0, y = 0; i < total_size; i +=3, j++) {
		float val = (0.2126f * resized_img[i] + 0.7152f * resized_img[i + 1] + 0.0722f * resized_img[i + 2]);
		val = (val - 127.5f) * c + 127.5f + b;
		const int pixel_value = (unsigned char) fminf(fmaxf(val, 0.0f), 255.0f);
		output[y * (new_width + 1) + (j % new_width)] = grayscale_to_ascii[pixel_value];

		if ((j + 1) % new_width == 0) {
			output[y * (new_width + 1) + new_width] = '\n';
			y++;
		}
	}
	write(STDOUT_FILENO, output, output_size);

	free(resized_img);
	free(output);
	return 0;
}