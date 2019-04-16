CC = gcc
CCFLAGS = -Wall -Wextra -std=gnu11

all: ngsobel

debug: CCFLAGS += -DDEBUG -g
debug: ngsobel

ngsobel: main.o libnetpbm_gs.a
	$(CC) $(CCFLAGS) -o ngsobel main.o -L. -lnetpbm_gs

main.o: main.c
	$(CC) $(CCFLAGS) -c main.c

libnetpbm_gs.a: netpbm_gs.o
	ar rcs libnetpbm_gs.a netpbm_gs.o

netpbm_gs.o: netpbm_gs.c
	$(CC) $(CCFLAGS) -c netpbm_gs.c -I.

.PHONY: clean

clean:
	rm -f ngsobel *.o *.a *.gch
