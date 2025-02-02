#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize2.h"
#define ASCII_MAP "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,\"^`'. "
#define ASCII_MAP_SIZE (sizeof(ASCII_MAP) -1)

static const float CHAR_ASPECT_RATIO = 0.5f;
static const float CONTRAST_CONST = 1.8f;
static const float BRIGHTNESS_CONST = 0.0f;
static const float MAX_WIDTH = 315.f;
static const float MAX_HEIGHT = 200.f;
static const float SCALE_FACTOR = (ASCII_MAP_SIZE - 1) / 255.0f;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s image_file\n", argv[0]);
		return EXIT_FAILURE;
	}
	char *restrict filename = argv[1];
	int width, height, channels;

	unsigned char *img = stbi_load(filename, &width, &height, &channels, 3);
	if (!img) {
		fprintf(stderr, "Failed to load image: %s\n", filename);
		return EXIT_FAILURE;
	}
	float scale_x = MAX_WIDTH / width;
	float scale_y = MAX_HEIGHT / height;
	float scale = (scale_x < scale_y) ? scale_x : scale_y;

	int new_width = width * scale;
	int new_height = (int)(height * scale * CHAR_ASPECT_RATIO);

	unsigned char *restrict resized_img = malloc(new_width * new_height * 3);
	if (resized_img == NULL) {
		fprintf(stderr, "Memory allocation of resize image failed\n");
		stbi_image_free(img);
		return EXIT_FAILURE;
	}
	stbir_resize_uint8_srgb(img, width, height, 0, resized_img, new_width, new_height, 0, 3);
	stbi_image_free(img);

	char grayscale_to_ascii[256];
	for (int i = 0; i < 256; i++) {
		grayscale_to_ascii[i] = ASCII_MAP[(int)((255 - i) * SCALE_FACTOR)];
	}

	const int total_size = new_width * new_height * 3;

	size_t output_size = (new_width + 1) * new_height;
	char *restrict output = malloc((new_width + 1) * new_height);
	if (output == NULL) {
		fprintf(stderr, "Memory allocation for output buffer failed\n");
		return EXIT_FAILURE;
	}

	for (int i = 0, j = 0, y = 0; i < total_size; i +=3, j++) {
		float val = (0.2126f * resized_img[i] + 0.7152f * resized_img[i + 1] + 0.0722f * resized_img[i + 2]);
		val = (val - 127.5f) * CONTRAST_CONST + 127.5f + BRIGHTNESS_CONST;
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
	return EXIT_SUCCESS;
}