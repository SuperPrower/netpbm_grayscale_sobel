CC = gcc
CCFLAGS = -Wall -Wextra -std=gnu11

all: ngsobel

ifeq ($(DEBUG), 1)
    CCFLAGS += -O0 -g -DDEBUG
else
    CCFLAGS += -O3
endif


ngsobel: main.o libnetpbm_gs.a
	$(CC) $(CCFLAGS) -o ngsobel main.o -lm -L. -lnetpbm_gs -pthread

main.o: main.c
	$(CC) $(CCFLAGS) -c main.c

libnetpbm_gs.a: netpbm_gs.o netpbm_fread.o netpbm_fwrite.o
	ar rcs libnetpbm_gs.a netpbm_gs.o netpbm_fread.o netpbm_fwrite.o

netpbm_gs.o: netpbm_gs.c
	$(CC) $(CCFLAGS) -c netpbm_gs.c -I. -lm -pthread

netpbm_fread.o: netpbm_fread.c
	$(CC) $(CCFLAGS) -c netpbm_fread.c -I.

netpbm_fwrite.o: netpbm_fwrite.c
	$(CC) $(CCFLAGS) -c netpbm_fwrite.c -I.

.PHONY: clean

clean:
	rm -f ngsobel *.o *.a *.gch
