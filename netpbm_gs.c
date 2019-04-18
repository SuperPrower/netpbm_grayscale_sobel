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
 * @file netpbm_gs.c
 * @author Sergey Koziakov
 * @brief implementation of Netpbm image processing functions
 */

#include "netpbm_gs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include <errno.h>
#include <math.h>


int netpbm_to_greyscale(netpbm_image_t *img)
{
	if (img->data == NULL) {
		fprintf(stderr, "Image structure is not initialized\n");
		return -1;
	}
	switch (img->type) {
	case NETPBM_ASCII_BITMAP:
	case NETPBM_ASCII_GREYMAP:
	case NETPBM_BINARY_BITMAP:
	case NETPBM_BINARY_GREYMAP:
		fprintf(stderr, "Image doesn't need greyscale convertion\n");
		return 0;
	case NETPBM_PAM:
		fprintf(stderr, "Not implemented\n");
		return -1;
	default:
		// continued below
		break;
	}

	size_t total_pixels = img->width * img->height;

	for (size_t i = 0; i < total_pixels; i++) {
		img->data[i]
			= 0.21 * NETPBM_RED(img->data[i])
			+ 0.72 * NETPBM_GREEN(img->data[i])
			+ 0.07 * NETPBM_BLUE(img->data[i]);

		if (img->data[i] > img->maxval)
			img->data[i] = img->maxval;
	}

	// It's now a greyscale image, not RGB, so adjust image type
	img->type -= 1;

	return 0;
}


/**
 * @brief Helper function that convolves a kernel with the image
 *
 * Apply kernel at ONE point to the data. Input data must be pre-padded if
 * necessary.
 *
 * @param[in] data_in - original data matrix.
 * @param[in] fx - focus point column in the input
 * @param[in] fy - focus point row in data matrix
 * @param[in] dw - matrix width
 * @param[in] dh - matrix height
 *
 * @param[in] kernel - kernel to apply
 * @param[in] kw - kernel width
 * @param[in] kh - kernel height
 *
 * @param[out] out - point to store results in
 *
 * @returns 0 if no problem occured, -1 otherwise
 */
int apply_kernel(
		uint32_t *data_in,
		uint32_t dw, uint32_t dh,
		uint32_t fx, uint32_t fy,
		uint32_t *kernel, uint32_t kw, uint32_t kh,
		uint32_t *out
)
{
	if (kh % 2 == 0 || kw % 2 == 0) {
		// are 2n sized kernels legal? I think they aren't...
		fprintf(stderr, "Kernel must be 2n+1 sized\n");
		return -1;
	}

	*out = 0;

	uint32_t ox = kw / 2;
	uint32_t oy = kh / 2;

	if ((int)(fx - ox) < 0
		|| (fx + ox) > dw
		|| (int)(fy - oy) < 0
		|| (fy + oy) > dh
	) {
		fprintf(stderr, "Coordinates out of bounds\n");
		return -1;
	}

	for (size_t i = 0; i < kh; i++) {
		for (size_t j = 0; j < kw; j++) {
			// first parenthesis offsets row, second column
			*out += data_in[((fy - oy + i) * dw) + (fx - ox + j)]
				* kernel[(kw * i) + (j)];
		}
	}

	return 0;
}

/**
 * @struct Data about image shared between worker threads
 */
struct kernel_task {
	uint32_t *p_data; /**< Padded data */
	uint32_t p_width, p_height;

	uint32_t *dest; /**< image data */
	uint32_t d_width, d_height;
};

/**
 * @struct Data local to the worker thread
 */
struct worker_info {
	struct kernel_task* task;

	uint32_t n_threads;
	uint32_t thread_id;
};

void *thread_task(void * arguments)
{
	struct worker_info *info = (struct worker_info *) arguments;
	struct kernel_task *task = (struct kernel_task *) info->task;
	/* Sobel kernel components */
	uint32_t x_kernel[] = {
		-1, 0, 1,
		-2, 0, 2,
		-1, 0, 1
	};

	uint32_t y_kernel[] = {
		-1, -2, -1,
		 0,  0,  0,
		 1,  2,  1
	};

	for (size_t i = info->thread_id;
		i < (task->d_width * task->d_height);
		i+= info->n_threads
	) {
		uint32_t out_x = 0;
		uint32_t out_y = 0;

		if (apply_kernel(task->p_data,
				task->p_width, task->p_height,
				(i % task->d_width) + 1, (i / task->d_width) + 1,
				x_kernel, 3, 3,
				&out_x
		) != 0) {
			fprintf(stderr, "Unable to apply kernel to the data!\n");
			exit(EXIT_FAILURE);
		};

		if (apply_kernel(task->p_data,
				task->p_width, task->p_height,
				(i % task->d_width) + 1, (i / task->d_width) + 1,
				y_kernel, 3, 3,
				&out_y
		) != 0) {
			fprintf(stderr, "Unable to apply kernel to the data!\n");
			exit(EXIT_FAILURE);
		};

		uint32_t val = sqrt(out_x * out_x + out_y * out_y);

		task->dest[i] = val;
	}


	return NULL;
}

int netpbm_sobel(netpbm_image_t *img, unsigned long n_threads)
{
	if (img->data == NULL) {
		fprintf(stderr, "Image structure is not initialized\n");
		return -1;
	}

	if (img->type == NETPBM_ASCII_PIXMAP || img->type == NETPBM_BINARY_PIXMAP) {
		fprintf(stderr, "Turn image into greyscale first using -g flag\n");
		return -1;
	}

	if (n_threads == 0 || n_threads == ULONG_MAX) {
		fprintf(stderr, "Invalid amount of threads!\n");
		return -1;
	}

	/* Pad the data */
	uint32_t p_height = img->height + 2;
	uint32_t p_width = img->width + 2;
	uint32_t p_elems = p_width * p_height;
	uint32_t *p_data = (uint32_t *) malloc(sizeof(uint32_t) * p_elems);

	// fill data with zeroes
	// TODO: implement some other kind of padding?
	memset(p_data, 0, sizeof(uint32_t) * p_elems);

	/* Copy image data to the center of padded array line by line,
	 * starting from the second element of the second row
	 * of the padded data array
	 */
	for (size_t row = 0; row < img->height; row++) {
		memcpy((p_data + p_width * (row + 1) + 1),
			(img->data + row * img->width),
			sizeof(uint32_t) * img->width
		);
	}

	struct kernel_task task = {
		p_data,
		p_width, p_height,

		img->data,
		img->width, img->height
	};

	struct worker_info *w_info = (struct worker_info *)
		malloc(sizeof(struct worker_info) * n_threads);

	pthread_t *threads = (pthread_t *)
		malloc(sizeof(pthread_t) * n_threads);

	for (size_t t = 0; t < n_threads; t++) {
		/* fill worker info */
		w_info[t].n_threads = n_threads;
		w_info[t].thread_id = t;
		w_info[t].task = &task;

		if (pthread_create(
			&threads[t], NULL,
			thread_task,
			(void*)(&w_info[t])
		) != 0) {
			fprintf(stderr, "Unable to create thread %lu!\n", t);
			return -1;
		}
	}

	for (size_t t = 0; t < n_threads; t++) {
		switch (pthread_join(threads[t], NULL)) {
		case EDEADLK:
			fprintf(stderr, "Deadlock occured!\n");
			return -1;

		case EINVAL:
			fprintf(stderr,
				"Thread %lu is not a joinable thread, "
				"or another thread is already waiting to "
				"join it!\n", t
			);
			return -1;

		case ESRCH:
			fprintf(stderr, "Thread %lu could not be found\n", t);
			return -1;

		case 0:
			break;
		}
	}

	free(threads);
	free(w_info);

	free(p_data);

	/* Normalize data up to maxval */
	/* XXX: Normalization results in low contrast, although really clean
	 * picture. Since it only affects ASCII format, I'll just put a filter
	 * in the output.
	 */
#if 0
	uint32_t local_maxval = 0;

	for (size_t cp = 0; cp < (img->height * img->width); cp++) {
		if (img->data[cp] > local_maxval)
			local_maxval = img->data[cp];
	}

	for (size_t cp = 0; cp < (img->height * img->width); cp++) {
		double val = (double)(img->data[cp]) / (double)(local_maxval);
		img->data[cp] = round(val * img->maxval);
	}
#endif

	return 0;
}

int free_netpbm_image(netpbm_image_t *img)
{
	free(img->data);
	return 0;
}

