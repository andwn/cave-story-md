// Converts some of Cave Story's files to a more ideal format
//
// Build:
//  gcc littlebig.c -o littlebig

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t* fslurp(const char *fn, size_t *size) {
	FILE *f = fopen(fn, "rb");
	if(!f) return NULL;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t *data = malloc(*size);
	if(!data) return NULL;
	fread(data, 1, *size, f);
	fclose(f);
	return data;
}

uint8_t* do_pxm(const uint8_t *src, size_t src_size, size_t *dst_size) {
	*dst_size = src_size - 4;
	uint8_t *dst = malloc(*dst_size);
	if(!dst) {
		printf("Failed to allocate %zu bytes\n", *dst_size);
		return NULL;
	}
	dst[0] = src[5]; // Width
	dst[1] = src[4];
	dst[2] = src[7]; // Height
	dst[3] = src[6];
	memcpy(dst + 4, src + 8, *dst_size - 4);
	return dst;
}

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t id;
	uint16_t event;
	uint16_t type;
	uint16_t flags;
} PXE_Entry;

uint8_t* do_pxe(const uint8_t *src, size_t src_size, size_t *dst_size) {
	*dst_size = src_size - 6;
	//printf("dst_size: %zu\n", *dst_size);
	uint8_t *dst = malloc(*dst_size);
	if(!dst) {
		printf("Failed to allocate %zu bytes\n", *dst_size);
		return NULL;
	}
	uint16_t num = src[4] | (src[5] << 8); // Number of objects
	uint16_t new_num = num;
	//printf("num: %u\n", num);
	if(num) {
		const PXE_Entry *pxe = (const PXE_Entry *)(src + 8);
		int pos = 0;
		for(int i = 0; i < num; i++) {
			if(!pxe[i].id && !pxe[i].event && !pxe[i].type && !pxe[i].flags) {
				// It's nothing, just remove it
				new_num--;
				continue;
			}
			for(int j = 0; j < 6; j++) {
				dst[2 + pos * 12 + j * 2 + 0] = src[8 + i * 12 + j * 2 + 1];
				dst[2 + pos * 12 + j * 2 + 1] = src[8 + i * 12 + j * 2 + 0];
			}
			pos++;
		}
	}
	dst[0] = new_num >> 8;
	dst[1] = new_num & 0xFF;
	//printf("new_num: %u\n", new_num);
	*dst_size = new_num * 12 + 2;
	return dst;
}

int main(int argc, char *argv[]) {
	if(argc != 3) {
		puts("Usage: littlebig <in file> <out file>");
		return EXIT_FAILURE;
	}
	size_t src_size = 0;
	uint8_t *src = fslurp(argv[1], &src_size);
	if(!src) {
		printf("Failed to read %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	size_t dst_size = 0;
	uint8_t *dst = NULL;
	const char *ext = strrchr(argv[1], '.');
	if(ext && strnlen(ext, 4) == 4) {
		if(ext[1] == 'p' && ext[2] == 'x' && ext[3] == 'm') {
			dst = do_pxm(src, src_size, &dst_size);
		} else if(ext[1] == 'p' && ext[2] == 'x' && ext[3] == 'e') {
			dst = do_pxe(src, src_size, &dst_size);
		} else {
			printf("Unknown extension %s\n", ext);
			return EXIT_FAILURE;
		}
	} else {
		printf("Not a valid extension\n");
		return EXIT_FAILURE;
	}
	free(src);
	if(dst) {
		FILE *f = fopen(argv[2], "wb");
		if(!f) {
			printf("Failed to write %s\n", argv[2]);
			return EXIT_FAILURE;
		}
		fwrite(dst, 1, dst_size, f);
		fclose(f);
		free(dst);
	} else {
		printf("What the heck?\n");
	}
	return EXIT_SUCCESS;
}
