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
 * @file netpbm_gs.h
 * @author Sergey Koziakov
 * @brief declares structures and functions for NETPBM_GS library
 */

#ifndef NETPBM_GS_H
#define NETPBM_GS_H

#include <stdint.h>

enum NETPBM_TYPE {
	NETPBM_ASCII_BITMAP = 1,
	NETPBM_ASCII_GREYMAP = 2,
	NETPBM_ASCII_PIXMAP = 3,
	NETPBM_BINARY_BITMAP = 4,
	NETPBM_BINARY_GREYMAP = 5,
	NETPBM_BINARY_PIXMAP = 6,
	NETPBM_PAM = 7
};

#define NETPBM_TYPE_IS_ASCII(X) ((X) < (4))
#define NETPBM_TYPE_IS_BINARY(X) ((X) > (3) && (X) < (7))


#define NETPBM_RED(X) ((X) & 0xff)
#define NETPBM_GREEN(X) (((X) >> 8) & 0xff)
#define NETPBM_BLUE(X) (((X) >> 16) & 0xff)

#define NETPBM_GREY(X) NETPBM_RED(X)

/**
 * @brief structure describing loaded Netpbm image
 */
typedef struct {
	enum NETPBM_TYPE type; /**< Type of the image */
	uint32_t maxval; /**< Maximal value of pixel or color component */

	uint32_t height; /**< Image height, in pixels */
	uint32_t width; /**< Image width, in pixels */

	uint32_t *data; /**< Pixel data in row-major order */
} netpbm_image_t;


/**
 * @brief Load Netpbm image from a file
 *
 * Loads image from file into the given structure. Automatically allocates
 * memory for the data field. It's up to user to later call free_netpbm_image()
 * to free allocated memory.
 *
 * @param[in] filename - image filename/path
 * @param[out] img - pre-allocated netpbm image structure.
 *
 * @return 0 if no problem occured, -1 otherwise
 *
 */
int read_netpbm_file(char *filename, netpbm_image_t *img);

/**
 * @brief turn netpbm image into greyscale, if applicable
 *
 * Turns given netpbm image into greyscale image using luminosity method.
 * Only works for P3/P6/P7 images, for others does nothing.
 *
 * @param[in,out] img - Netpbm image structure to be turned greyscale.
 *
 * @return 0 if no problem occured, -1 otherwise
 */
int netpbm_to_greyscale(netpbm_image_t *img);

/**
 * @brief apply Sobel operator to the greyscale Netpbm image.
 *
 * Apply Sobel operator to the greyscale Netpbm image. If image is not greyscale,
 * function exits with error code 1. In that case, use netpbm_to_greyscale()
 * function. Image size is retained by padding original image.
 * If n_threads is given, job would be split between n threads
 *
 * @param[in,out] img - Netpbm image structure to be turned greyscale.
 * @param[in] n_threads - request creating at least n threads.
 *
 * @return 0 if no problem occured, 1 if image is not greyscale, -1 otherwise
 */
int netpbm_sobel(netpbm_image_t *img, unsigned long n_threads);
/**
 * @brief Write Netpbm image to the file
 *
 * Writes image from the structure to the given file.
 *
 * @param[in] filename - output image filename/path
 * @param[in] img - netpbm image structure to be written.
 *
 * @return 0 if no problem occured, -1 otherwise
 *
 */
int write_netpbm_file(char *filename, netpbm_image_t *img);

/**
 * @brief Frees allocated memory in Netpbm image structure.
 *
 * Frees memory allocated for the data field in the structure.
 * Structure shouldn't be used afterwards.
 *
 * @param[in] img - Netpbm image structure to be freed.
 *
 * @return 0 if no problem occured, -1 otherwise
 */
int free_netpbm_image(netpbm_image_t *img);

#endif // NETPBM_GS_H
