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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <limits.h>
#include <errno.h>

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
 * 1. необходимо реализовать базовые операции работы с файлами данного
 * формата: как его прочитать, как изменить, сохранить.
 * Файл описываем структурой, память под матрицу выделяем динамически.
 * 2. Реализуем алгоритм преобразования изображения в черно-белое
 * 3. Реализуем оператор Собеля для получаемого черно-белого изображения
 * https://en.wikipedia.org/wiki/Sobel_operator
 * 4. Добавить поддержку использования оператора Собеля в несколько потоков
 * (thread) - количество задается в командной строке
 * 5. Выводим на экран время обработки изображения (нужен для сравнения
 * времени обработки в зависимости от количества потоков)
 *
 * Пункты 1-5 подразумевают, что приложение может иметь простой консольный
 * интерфейс;
 *
 * Дополнительно:
 * - приложение должно собираться при помощи Makefile;
 * - исходные коды должны находиться под системой контроля версий git и
 * содержать историю коммитов;
 * - (опционально) реализовать графический интерфейс используя QT с
 * возможностью отображения изображения
 *
 *
 * Вариант передачи:
 * - результат передаётся в виде ссылки на репозиторий размещённый на
 * github или в виде архива содержащего репозиторий с исходными кодами, а
 * также описание тестов (как проверить)
 *
 * Вариант запуска программы:
 *  > my_converter -i my_photo.ppm -r my_photo_sobel.ppm -p 10
 * Time of algorithm execution: 3.2 ms
 */

int main(int argc, char *argv[])
{
	// Parse arguments
	int c;
	extern char *optarg;

	char *ifilename = NULL;
	char *ofilename = NULL;
	unsigned long n_threads = 1;

	while ((c = getopt(argc, argv, "i:o:p:")) != -1) {
		switch (c) {
		case 'i':
			/* Man page does not state whether optarg must be copied or not.
			In my case, it points to the argv element. However, to avoid
			problems with different implementation, I explicitly copy it. */
			ifilename = strdup(optarg);
			break;
		case 'o':
			ofilename = strdup(optarg);
			break;
		case 'p':
			n_threads = strtoul(optarg, NULL, 10);
			break;
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
	printf("Will create %lu threads\n", n_threads);

	// Open files
	FILE *ifile = NULL;
	FILE *ofile = NULL;

	if (ifilename == NULL) {
		ifile = stdin;
	} else {
		ifile = fopen(ifilename, "r");
		free(ifilename);
	}

	if (ofilename == NULL) {
		ofile = stdout;
	} else {
		ofile = fopen(ofilename, "w");
		free(ofilename);
	}

	// Free FILE's
	if (ifile != stdin) {
		fclose(ifile);
	}

	if (ofile != stdout) {
		fclose(ofile);
	}

	return 0;
}
