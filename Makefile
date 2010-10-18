all:meta.c
	gcc -Dbench -std=gnu99 -g -fwhole-program -O3 -ffast-math -lglfw -fopenmp -march=native meta.c -o meta