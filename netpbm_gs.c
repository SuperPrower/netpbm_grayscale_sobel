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

#include <errno.h>


int netpbm_to_greyscale(netpbm_image_t *img)
{
	return 0;
}

int netpbm_sobel(netpbm_image_t *img)
{
	return 0;
}

int netpbm_sobel_threaded(netpbm_image_t *img, unsigned long n_threads)
{
	return 0;
}

int apply_sobel_to(uint32_t **data_in, uint32_t dh, uint32_t dw, uint32_t *out)
{
	return 0;
}


int free_netpbm_image(netpbm_image_t *img)
{
	free(img->data);

	return 0;
}

