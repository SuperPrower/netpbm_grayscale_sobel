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
 * @file netpbm_fread.c
 * @author Sergey Koziakov
 * @brief implementation of file reading NETPBM_GS function
 */

#include "netpbm_gs.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <errno.h>

/* File Processing Helper Functions */

static inline int FIND_EOL(FILE *ifile)
{
	uint8_t byte;

	while (1) {
		if (fread(&byte, sizeof(uint8_t), 1, ifile) != 1)
			return -1;

		if (byte != '\n' && byte != '\r')
			continue;
		else
			break;
	}

	return 0;
}

static inline int SKIP_EOL(FILE *ifile)
{
	uint8_t byte;

	while (1) {

		if (fread(&byte, sizeof(uint8_t), 1, ifile) != 1)
			return -1;

		if (byte == '\n' || byte == '\r')
			continue;

		else {
			fseek(ifile, -1, SEEK_CUR);
			break;
		}
	}
	return 0;
}

static inline int SKIP_WHITESPACE(FILE *ifile)
{
	uint8_t byte;

	while (1) {
		if (fread(&byte, sizeof(uint8_t), 1, ifile) != 1)
			return -1;

		if (byte == '#') {
			if (FIND_EOL(ifile) != 0)
				return -1;
			if (SKIP_EOL(ifile) != 0)
				return -1;
			continue;

		} else if (isspace(byte)) {
			continue;

		} else if (byte == '\n' || byte == '\r') {
			SKIP_EOL(ifile);
			continue;

		} else {
			fseek(ifile, -1, SEEK_CUR);
			break;

		};
	}

	return 0;
}

static inline int READ_BYTE(FILE *ifile, uint32_t *dest)
{
	uint8_t byte;

	if (fread(&byte, sizeof(char), 1, ifile) != 1)
		return -1;

	*dest = byte;

	return 0;
}

static inline int READ_NUMBER(FILE *ifile, uint32_t *dest)
{
	uint8_t byte;
	uint8_t n_digits = 0;
	char number[8];

	while (1) {
		if (fread(&byte, sizeof(char), 1, ifile) != 1) {
			printf("Errno: %d", ferror(ifile));
			return -1;
		}
		if (byte >= '0' && byte <= '9') {
			number[n_digits++] = byte;
		} else {
			number[n_digits] = '\0';
			fseek(ifile, -1, SEEK_CUR);
			*dest = strtol(number, NULL, 0);
			break;
		}
	}

	return 0;
}

static inline int READ_PIXEL_WORD(FILE *ifile, uint32_t maxval, uint32_t *dest)
{
	uint32_t number;

	if (READ_NUMBER(ifile, &number) != 0)
		return -1;

	if (SKIP_WHITESPACE(ifile) != 0 && feof(ifile) == 0)
		return -1;

	if (number > maxval)
		return -1;

	*dest = number;
	return 0;
}

/* End File Processing Helper Functions */

int read_netpbm_file(char *filename, netpbm_image_t *img)
{
	FILE *ifile = fopen(filename, "rb");

	if (ifile == NULL) {
		fprintf(stderr, "Unable to open file: error %d\n", errno);
		return -1;
	}

	/*
	 * 1. A "magic number" for identifying the file type:
	 * -- An ASCII PBM file's magic number is the two characters "P1".
	 * -- An ASCII PGM file's magic number is the two characters "P2".
	 * -- An ASCII PPM file's magic number is the two characters "P3".
	 * -- A binary PBM file's magic number is the two characters "P4".
	 * -- A binary PGM file's magic number is the two characters "P5".
	 * -- A binary PPM file's magic number is the two characters "P6".
	 */

	uint8_t magic[2];

	if (fread(magic, sizeof(uint8_t), 2, ifile) != 2)
		goto error;

	if (magic[0] != 'P' || magic[1] < '1' || magic[1] > '7') {
		fprintf(stderr, "Unable to identify magic number\n");
		fclose(ifile);
		return -1;
	}

	img->type = magic[1] - '0';

	/* 2. Whitespace (blanks, TABs, CRs, LFs). */
	if (SKIP_WHITESPACE(ifile) != 0)
		goto error;

	/* 3. A width, formatted as ASCII characters in decimal. */
	if (READ_NUMBER(ifile, &img->width) != 0)
		goto error;

	/* 4. Whitespace. */
	if (SKIP_WHITESPACE(ifile) != 0)
		goto error;

	/* 5. A height, again in ASCII decimal. */
	if (READ_NUMBER(ifile, &img->height))
		goto error;

	/* 6. Whitespace. */
	if (SKIP_WHITESPACE(ifile) != 0)
		goto error;

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
		READ_NUMBER(ifile, &img->maxval);
		SKIP_WHITESPACE(ifile);
	} else {
		img->maxval = 1;
	}

#if DEBUG
	printf("Read structure:\n"
		"\tType: %u\n"
		"\tMaxval: %u\n"
		"\tHeight: %u\n"
		"\tWidth: %u\n",
		img->type,
		img->maxval,
		img->height,
		img->width
	);
#endif // DEBUG

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
	  * Characters from a "#" to the next end-of-line are ignored (comments).
	  * [ ] TODO: PAM format
	  */

	/* allocate data */
	img->data = (uint32_t *) malloc(sizeof(uint32_t *) * img->width * img->height);

	uint32_t cp = 0;
	// for P4
	uint32_t column = 0;
	uint32_t byte = 0;
	uint32_t bits_read = 0;
	uint32_t byte_needed = 1;

	const uint32_t total_pixels = img->width * img->height;

	while (cp < total_pixels) {
		if (img->type == NETPBM_ASCII_BITMAP || img->type == NETPBM_ASCII_GREYMAP) {
			// read image word by word
			// TODO: expects spaces between numbers,
			// which isn't true for P1
			if (READ_PIXEL_WORD(ifile, img->maxval, &(img->data[cp])) != 0)
				goto error;

		} else if (img->type == NETPBM_ASCII_PIXMAP) {
			// read image 3 words at a time
			uint32_t red, green, blue;

			if (READ_PIXEL_WORD(ifile, img->maxval, &red) != 0)
				goto error;

			if (READ_PIXEL_WORD(ifile, img->maxval, &green) != 0)
				goto error;

			if (READ_PIXEL_WORD(ifile, img->maxval, &blue) != 0)
				goto error;

			img->data[cp]
				= (red & 0xff)
				+ ((green & 0xff) << 8)
				+ ((blue & 0xff) << 16);

		} else if (img->type == NETPBM_BINARY_BITMAP) {
			// read byte, split into 8 bits
			// If image size isn't exactly divisible by 8, we
			// ignore last bits in the byte, like here:
			// http://fejlesztek.hu/pbm-p4-image-file-format/

			if (byte_needed) {
				bits_read = 0;
				byte_needed = 0;
				if (READ_BYTE(ifile, &byte) != 0)
					goto error;
			}

			img->data[cp] = ((byte >> (7 - bits_read)) & 1U)
				? 255U : 0;

			column++;
			bits_read++;

			if (column == img->width) {
				column = 0;
				byte_needed = 1;
			}

			if (bits_read == 8)
				byte_needed = 1;

		} else if (img->type == NETPBM_BINARY_GREYMAP) {
			if (READ_BYTE(ifile, &(img->data[cp])) != 0)
				goto error;

		} else if (img->type == NETPBM_BINARY_PIXMAP) {
			uint32_t red, green, blue;
			if (READ_BYTE(ifile, &red) != 0)
				goto error;
			if (READ_BYTE(ifile, &green) != 0)
				goto error;
			if (READ_BYTE(ifile, &blue) != 0)
				goto error;

			img->data[cp]
				= (red & 0xff)
				+ ((green & 0xff) << 8)
				+ ((blue & 0xff) << 16);
		}

		cp++;
	}

	fclose(ifile);
	return 0;

error:
	fprintf(stderr, "Error reading file\n");
	fclose(ifile);
	return -1;
}
