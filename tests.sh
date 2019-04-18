#/usr/bin/env bash

[ -d test_out ] || mkdir test_out

declare -a inputs=(
	"p1_washington_ascii.pbm"
	"p2_f14_ascii.pgm"
	"p3_snail_ascii.ppm"
	"p4_washington_binary.pbm"
	"p5_lena_binary.pgm"
	"p6_underwater_bmx_binary.ppm"
)

declare -a outputs=(
	"p1_test_out.pbm"
	"p2_test_out.pgm"
	"p3_test_out.ppm"
	"p4_test_out.pbm"
	"p5_test_out.pgm"
	"p6_test_out.ppm"
)

echo Running reading and writing tests
for index in "${!inputs[@]}"; do
	echo Running test on image "${inputs[$index]}"
	./ngsobel -s 0 -i "test_in/${inputs[$index]}" -o "test_out/${outputs[$index]}"
done

echo ==============================
echo Running greyscale test on "${inputs[2]}"
./ngsobel -s 0 -i "test_in/${inputs[2]}" -o "test_out/p2_from_p3_greyscale.pgm" -g

echo Running greyscale test on "${inputs[5]}"
./ngsobel -s 0 -i "test_in/${inputs[5]}" -o "test_out/p5_from_p6_greyscale.pgm" -g

echo ==============================
echo Testing Sobel operator:

echo Testing 1-threaded performance
./ngsobel -i "test_in/p5_large.pgm" -o "test_out/p5_large.pgm" -p 1

echo Testing 2-threaded performance
./ngsobel -i "test_in/p5_large.pgm" -o "test_out/p5_large.pgm" -p 2

echo Testing 4-threaded performance
./ngsobel -i "test_in/p5_large.pgm" -o "test_out/p5_large.pgm" -p 4
