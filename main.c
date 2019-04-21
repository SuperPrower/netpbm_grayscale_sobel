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

#include "netpbm_gs.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

void print_usage(char *binary_name)
{
	printf("Usage: %s -i ifilename -o filename [-g] [-p n_threads] [-h] [-s value]\n"
		"\t-i\t- Input file name. Required.\n"
		"\t-o\t- Output file name. Required.\n"
		"\t-g\t- turn image to greyscale. Required for RGB images\n"
		"\t-p\t- split Sobel operator between n threads\n"
		"\t-h\t- show this message and exit\n"
		"\t-s\t- Apply Sobel operator to the image "
		"if value is != 0. Enabled by default\n",
		binary_name
	);
}

int main(int argc, char *argv[])
{
	// Parse arguments
	int c;
	extern char *optarg;

	char *ifilename = NULL;
	char *ofilename = NULL;
	unsigned long n_threads = 1;
	unsigned long do_sobel = 1;

	uint8_t do_greyscale = 0;

	while ((c = getopt(argc, argv, "i:o:p:ghs:")) != -1) {
		switch (c) {
		case 'i':
			/* Man page does not state whether optarg must be
			 * copied or not. In my case, it points to the argv
			 * element. However, to avoid problems with different
			 * implementation, I explicitly copy it.
			 */
			ifilename = strdup(optarg);
			break;
		case 'o':
			ofilename = strdup(optarg);
			break;
		case 'p':
			n_threads = strtoul(optarg, NULL, 10);
			break;
		case 'g':
			do_greyscale = 1;
			break;
		case 's':
			do_sobel = strtoul(optarg, NULL, 10);
			break;
		case 'h':
			print_usage(argv[0]);
			return 0;
		case ':':
			fprintf(stderr, "Option -%c requires an operand\n", optopt);
			break;
		case '?':
			fprintf(stderr, "Unrecognised option: -%c\n", optopt);
			return -1;
		}
	}

	if (n_threads == 0 || n_threads == ULONG_MAX) {
		fprintf(stderr, "Invalid number of threads\n");
		return -1;
	}

	if (ifilename == NULL) {
		fprintf(stderr, "Please specify input file using -i flag. "
				"Check -h flag for usage\n");
		return -1;
	}

	if (ofilename == NULL) {
		fprintf(stderr, "Please specify output file using -o flag. "
				"Check -h flag for usage\n");
		return -1;
	}

	netpbm_image_t image;

	if (read_netpbm_file(ifilename, &image) != 0)
		return -1;

	if (do_greyscale && netpbm_to_greyscale(&image) != 0)
		return -1;

	if (do_sobel) {
		struct timespec start, finish;
		clock_gettime(CLOCK_MONOTONIC, &start);

		if (netpbm_sobel(&image, n_threads) != 0)
			return -1;

		clock_gettime(CLOCK_MONOTONIC, &finish);

		time_t seconds = finish.tv_sec - start.tv_sec;
		// Nanoseconds can go negative, since they are the remainder.
		// Adjust for it here.
		long int nanoseconds = finish.tv_nsec - start.tv_nsec;
		while (nanoseconds < 0) {
			// Compiler doesn't like the engineering notation here
			nanoseconds += 1000000000;
			--seconds;
		}
		// Decimals not used for more precise comparisons
		printf("Sobel algorithm took %li seconds and %li nanoseconds\n", seconds, nanoseconds);

	}

	write_netpbm_file(ofilename, &image);

	free_netpbm_image(&image);
	free(ifilename);
	free(ofilename);

	return 0;
}
