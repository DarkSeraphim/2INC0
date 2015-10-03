#
#	Makefile for 2INC0 Interprocess Communication
#
#	(c) Fontys 2010, Joris Geurts
#

BINARIES = prime

CC = gcc
CFLAGS = -Wall -g -c
DEPS = prime.h
LDLIBS = -lpthread

all:	$(BINARIES)

clean:
	rm -f *.o $(BINARIES)

prime: prime.o 

prime.o: prime.c