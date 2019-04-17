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
set -x
for index in "${!inputs[@]}"; do
	./ngsobel -i "test_in/${inputs[$index]}" -o "test_out/${outputs[$index]}"
done
set +x
