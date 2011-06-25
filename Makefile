ifeq ($(NOBENCH),)
B=-DBENCH
else
B=
endif
all:meta.c
	gcc -std=gnu99 -fwhole-program -O2 -ffast-math -lglfw -fopenmp -march=native meta.c -o meta ${B}