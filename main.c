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

/*
 * TODO:
 * ЗАДАНИЕ ОС / Портирование
 * - Дано: графический формат ppm https://en.wikipedia.org/wiki/Netpbm_format
 *
 * Программа принимает на вход путь до файла ppm, и путь до выходного файла
 * (результата)
 * Программа пишется под Linux, на С, с использованием Posix
 *
 *
 * [ ] 1. необходимо реализовать базовые операции работы с файлами данного
 * формата: как его прочитать, как изменить, сохранить.
 * [x] Файл описываем структурой, память под матрицу выделяем динамически.
 * [x] 2. Реализуем алгоритм преобразования изображения в черно-белое
 * [x] 3. Реализуем оператор Собеля для получаемого черно-белого изображения
 * https://en.wikipedia.org/wiki/Sobel_operator
 * [ ] 4. Добавить поддержку использования оператора Собеля в несколько потоков
 * (thread) - количество задается в командной строке
 * [x] 5. Выводим на экран время обработки изображения (нужен для сравнения
 * времени обработки в зависимости от количества потоков)
 *
 * Пункты 1-5 подразумевают, что приложение может иметь простой консольный
 * интерфейс; Опционально можно реализовать графический интерфейс используя QT
 * с возможностью отображения изображения
 *
 * Вариант передачи:
 * - результат передаётся в виде ссылки на репозиторий размещённый на
 * github или в виде архива содержащего репозиторий с исходными кодами, а
 * также описание тестов (как проверить)
 *
 * Вариант запуска программы:
 * $ ./my_converter -i my_photo.ppm -r my_photo_sobel.ppm -p 10
 * Time of algorithm execution: 3.2 ms
 */

void print_usage(char *binary_name)
{
	printf("Usage: %s -i ifilename -o filename [-g] [-p n_threads] [-h]\n"
		"\t-i\t- Input file name. Required.\n"
		"\t-o\t- Output file name. Required.\n"
		"\t-g\t- turn image to greyscale. Required for RGB images\n"
		"\t-p\t- split Sobel operator between n threads\n"
		"\t-h\t- show this message and exit",
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
		clock_t start = clock(), diff;

		if (netpbm_sobel(&image, n_threads) != 0)
			return -1;

		diff = clock() - start;

		int msec = diff * 1000 / CLOCKS_PER_SEC;
		printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
	}

	write_netpbm_file(ofilename, &image);

	free_netpbm_image(&image);
	free(ifilename);
	free(ofilename);

	return 0;
}
