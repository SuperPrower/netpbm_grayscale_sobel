CC = gcc
CFLAGS = -fPIC -Wall -Wextra -O2 -g
LDFLAGS = -shared

all: ngsobel

ngsobel: main.o # netpbm.so netpbm_sobel.so
	$(CC) -o ngsobel main.o # -L. -lntmpbm -lnetpbm_sobel

main.o: main.c
	$(CC) -c main.c

# netpbm_sobel.so: netpbm.so
# 	$(CC)
#
# netpbm.so:
#
#

.PHONY: clean

clean:
	rm -f ngsobel *.o *.a *.so *.gch
