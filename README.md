# Netpbm with Greyscale convertion and Sobel operator

## Building
```shell
make
```

### Building with debug information
```shell
make DEBUG=1
```

### Usage example
```shell
./ngsobel -i test_in/p6_underwater_bmx_binary.ppm -g -o test_out/p5_from_p6_sobel.pgm -p 
```

### Testing
Run `tests.sh`

## Current Issues

- [ ] P1 format reader expects whitespace-separated digits
- [ ] No support for PAM (P7) format

## Sources

* [PBM ASCII](https://people.sc.fsu.edu/~jburkardt/data/pbma/pbma.html)
* [PGM ASCII](https://people.sc.fsu.edu/~jburkardt/data/pgma/pgma.html)
* [PPM ASCII](https://people.sc.fsu.edu/~jburkardt/data/ppma/ppma.html)
* [PBM Binary](https://people.sc.fsu.edu/~jburkardt/data/pbmb/pbmb.html)
* [PGM Binary](https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html)
* [PPM Binary](https://people.sc.fsu.edu/~jburkardt/data/ppmb/ppmb.html)
