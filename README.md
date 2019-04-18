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
- [ ] Multithreading doesn't increase performance even on humongous data, quite the opposite - performance gets worse.
