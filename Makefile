all:meta.c
	gcc -s -std=gnu99 -fwhole-program -O2 -ffast-math -lGL -lglfw -fopenmp -march=native meta.c -o meta ${B}