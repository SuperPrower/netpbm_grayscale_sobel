/*
 * NetPBM to Grayscale with Sobel algorithm
 * Copyright (C) 2019 Sergey Koziakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file netpbm_fwrite.c
 * @author Sergey Koziakov
 * @brief implementation of Netpbm file writing function
 */

#include "netpbm_gs.h"

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>

/* Helper Functions */
static inline int __WRITE_BYTE(FILE *ofile, uint8_t byte)
{
	if (fwrite(&byte, sizeof(uint8_t), 1, ofile) != 1) return -1;
	return 0;
}

static inline int __WRITE_ASCII_NUMBER(FILE *ofile, uint32_t number)
{
	// edge case
	if (number == 0) return __WRITE_BYTE(ofile, '0');

	// split number into digits characters array
	uint8_t digits[16];
	uint8_t n_digits = 0;

	while (number > 0) {
		digits[n_digits++] = '0' + (number % 10);
		number /= 10;
	}

	for (int c = n_digits - 1; c >= 0; c--) {
		if (__WRITE_BYTE(ofile, digits[c]) != 0) return -1;
	}

	return 0;
}


int write_netpbm_file(char *filename, netpbm_image_t *img)
{
	FILE *ofile = fopen(filename, "wb");
	if (ofile == NULL) {
		fprintf(stderr, "Unable to open file: error %d\n", errno);
	}

#define WRITE_BYTE(X)	\
	do {\
		if (__WRITE_BYTE(ofile, (X)) != 0) goto error; \
	} while (0)

#define WRITE_ASCII_NUMBER(X)	\
	do {\
		if (__WRITE_ASCII_NUMBER(ofile, (X)) != 0) goto error; \
	} while (0)


#define PUT_WHITESPACE() WRITE_BYTE('\n')
#define PUT_SPACE() WRITE_BYTE(' ')

	/*
	 * 1. A "magic number" for identifying the file type:
	 * -- An ASCII PBM file's magic number is the two characters "P1".
	 * -- An ASCII PGM file's magic number is the two characters "P2".
	 * -- An ASCII PPM file's magic number is the two characters "P3".
	 * -- A binary PBM file's magic number is the two characters "P4".
	 * -- A binary PGM file's magic number is the two characters "P5".
	 * -- A binary PPM file's magic number is the two characters "P6".
	 */
	WRITE_BYTE('P');
	WRITE_BYTE(img->type + '0');

	/* 2. Whitespace (blanks, TABs, CRs, LFs). */
	PUT_WHITESPACE();

	/* 3. A width, formatted as ASCII characters in decimal. */
	WRITE_ASCII_NUMBER(img->width);

	/* 4. Whitespace. */
	PUT_WHITESPACE();

	/* 5. A height, again in ASCII decimal. */
	WRITE_ASCII_NUMBER(img->height);

	/* 6. Whitespace. */
	PUT_WHITESPACE();

	/*
	 * 7.1.
	 * -- P2: The maximum gray value, again in ASCII decimal.
	 * -- P3: The maximum color-component value, again in ASCII decimal.
	 * -- P5: The maximum gray value, MAXVAL, again in ASCII decimal.
	 * 		MAXVAL must be between 0 and 255.
	 * -- P6: The maximum color-component value MAXVAL, again in ASCII decimal.
	 * 		For the binary format, MAXVAL cannot be any greater than 255.
	 *
	 * 7.2. (not for P1 and P4)
	 * A single character of whitespace, typically a newline;
	 */
	if (img->type != NETPBM_ASCII_BITMAP && img->type != NETPBM_BINARY_BITMAP) {
		WRITE_ASCII_NUMBER(img->maxval);
		PUT_WHITESPACE();
	}

	 /* 8.
	 * -- P1: Width x Height bits, each either '1' or '0', starting at
	 * 		the top-left corner of the bitmap, proceeding in normal
	 * 		English reading order.
	 *
	 * -- P2: Width x Height gray values, each in ASCII decimal,
	 * 		between 0 and the specified maximum value, separated
	 * 		by whitespace, starting at the top-left corner of the
	 * 		graymap, proceeding in normal English reading order.
	 * 		A value of 0 means black, and the maximum value means white.
	 *
	 * -- P3: Width x Height pixels, each three ASCII decimal values
	 * 		between 0 and the specified maximum value, starting at
	 * 		the top-left corner of the pixmap, proceeding in normal
	 * 		English reading order. The three values for each pixel
	 * 		represent red, green, and blue, respectively; a value of
	 * 		0 means that color is off, and the maximum value means
	 * 		that color is maxed out.
	 *
	 * -- P4: Width x Height bits, stored 8 bits per byte, high bit
	 * 		first and low bit last, starting at the top-left corner
	 * 		of the bitmap, proceeding in normal English reading order.
	 *
	 * -- P5: Width x Height gray values, each stored as a plain byte,
	 * 		between 0 and the specified maximum value, separated by
	 * 		whitespace, starting at the top-left corner of the graymap,
	 * 		proceeding in normal English reading order. A value of 0
	 * 		means black, and the maximum value means white.
	 *
	 * -- P6: Width x Height pixels, each pixel being described by 3
	 * 		bytes, each between 0 and the specified maximum value,
	 * 		starting at the top-left corner of the pixmap, proceeding
	 * 		in normal English reading order. The three values for
	 * 		each pixel represent red, green, and blue, respectively;
	 * 		a value of 0 means that color is off, and the maximum
	 * 		value means that color is maxed out.
	 *
	 * [ ] TODO: PAM format
	 */

	uint32_t cp = 0;
	const uint32_t total_pixels = img->width * img->height;

	// for binary bitmap format
	uint8_t byte = 0;
	uint8_t bits_filled = 0;
	uint32_t column = 0;
	uint8_t byte_ready = 0;

	while (cp < total_pixels) {
		if (img->type == NETPBM_ASCII_BITMAP || img->type == NETPBM_ASCII_GREYMAP) {
			WRITE_ASCII_NUMBER(img->data[cp]);
			PUT_SPACE();

		} else if (img->type == NETPBM_ASCII_PIXMAP) {
			WRITE_ASCII_NUMBER(NETPBM_RED(img->data[cp]));
			PUT_SPACE();
			WRITE_ASCII_NUMBER(NETPBM_GREEN(img->data[cp]));
			PUT_SPACE();
			WRITE_ASCII_NUMBER(NETPBM_BLUE(img->data[cp]));
			PUT_WHITESPACE();

		} else if (img->type == NETPBM_BINARY_BITMAP) {
			// If amount of columns isn't divisible by 8,
			// We allocate all required bytes, and ignore
			// last bits

			if (img->data[cp] > 0)
				byte |= (1U << (7 - bits_filled));

			bits_filled++;
			column++;

			if (column == img->width) {
				column = 0;
				byte_ready = 1;
			}

			if (bits_filled == 8) {
				byte_ready = 1;
			}

			if (byte_ready) {
				byte_ready = 0;
				bits_filled = 0;
				WRITE_BYTE(byte);
				byte = 0;
			}

		} else if (img->type == NETPBM_BINARY_GREYMAP) {
			WRITE_BYTE(img->data[cp]);

		} else if (img->type == NETPBM_BINARY_PIXMAP) {
			WRITE_BYTE((uint8_t)(NETPBM_RED(img->data[cp])));
			WRITE_BYTE((uint8_t)(NETPBM_GREEN(img->data[cp])));
			WRITE_BYTE((uint8_t)(NETPBM_BLUE(img->data[cp])));
		}

		cp++;
	}

	fclose(ofile);
	return 0;

error:
	fprintf(stderr, "Error writing file\n");
	fclose(ofile);
	return -1;
}
