CC=gcc
CFLAGS=-lrt -lm -L/usr/local/lib -lwiringPi

all: linefollow objectavoid

linefollow:
	$(CC) -o LineFollow.out "LineFollow\LineFollow.c" $(CFLAGS)

objectavoid:
	$(CC) -o ObjectAvoid.out "ObjectAvoid\ObjectAvoid.c" $(CFLAGS)