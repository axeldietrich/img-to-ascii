#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize2.h"
#define ASCII_MAP "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,\"^`'. "
#define ASCII_MAP_SIZE (sizeof(ASCII_MAP) -1)

static const float SCALE_FACTOR = (ASCII_MAP_SIZE - 1) / 255.0f;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s image_file options\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Default values
	float brightness = 0.0f;
	float contrast = 1.2f;
	float char_aspect_ratio = 0.5f;
	float max_width = 315.f;
	float max_height = 200.f;

	// Get filename and options
	char *restrict filename = argv[1];
	optind = 2;
	int opt;
	while ((opt = getopt(argc, argv, "b:c:w:h:a:")) != -1) {
		switch (opt) {
			case 'b':
				brightness = atof(optarg);
				break;
			case 'c':
				contrast = atof(optarg);
				break;
			case 'w':
				max_width = atof(optarg);
				break;
			case 'h':
				max_height = atof(optarg);
				break;
			case 'a':
				char_aspect_ratio = atof(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s image path -b brightness -c contrast -w max width -h max height -a char aspect ratio\n", argv[0]);
				return EXIT_FAILURE;
		}
	}

	int width, height, channels;

	// Load image
	unsigned char *img = stbi_load(filename, &width, &height, &channels, 3);
	if (!img) {
		fprintf(stderr, "Failed to load image: %s\n", filename);
		return EXIT_FAILURE;
	}
	// Scale down to a size printable on terminal
	const float scale_x = max_width / width;
	const float scale_y = max_height / height;
	const float scale = (scale_x < scale_y) ? scale_x : scale_y;

	// Scale width and height
	const int new_width = width * scale;
	const int new_height = (int)(height * scale * char_aspect_ratio);

	unsigned char *restrict resized_img = malloc(new_width * new_height * 3);
	if (resized_img == NULL) {
		fprintf(stderr, "Memory allocation of resize image failed\n");
		stbi_image_free(img);
		return EXIT_FAILURE;
	}
	// Resize image to new dimensions
	stbir_resize_uint8_srgb(img, width, height, 0, resized_img, new_width, new_height, 0, 3);
	stbi_image_free(img);

	// Compute a look up table for grayscale -> ascii conversions to avoid
	// computing them for each pixel
	char grayscale_to_ascii[256];
	for (int i = 0; i < 256; i++) {
		grayscale_to_ascii[i] = ASCII_MAP[(int)((255 - i) * SCALE_FACTOR)];
	}

	const int total_size = new_width * new_height * 3;

	// Allocate a new buffer for the output to reduce I/O and print the whole image at once
	size_t output_size = (new_width + 1) * new_height;
	char *restrict output = malloc((new_width + 1) * new_height);
	if (output == NULL) {
		fprintf(stderr, "Memory allocation for output buffer failed\n");
		return EXIT_FAILURE;
	}

	// Convert each pixel to grayscale and then mapping the value to an ASCII character
	// adjusting brightness and contrast as specified
	for (int i = 0, j = 0, y = 0; i < total_size; i += 3, j++) {
		float val = (0.2126f * resized_img[i] + 0.7152f * resized_img[i + 1] + 0.0722f * resized_img[i + 2]);
		val = (val - 127.5f) * contrast + 127.5f + brightness;
		const int pixel_value = (unsigned char) fminf(fmaxf(val, 0.0f), 255.0f);
		output[y * (new_width + 1) + (j % new_width)] = grayscale_to_ascii[pixel_value];

		if ((j + 1) % new_width == 0) {
			output[y * (new_width + 1) + new_width] = '\n';
			y++;
		}
	}
	// Print the image
	write(STDOUT_FILENO, output, output_size);

	free(resized_img);
	free(output);
	return EXIT_SUCCESS;
}