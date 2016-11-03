CC=gcc
CFLAGS=-W -Wall

all: thv4 thv3 thv2 thv1

thv4: thv4.o p1fxns.o
thv3: thv3.o p1fxns.o
thv2: thv2.o p1fxns.o
thv1: thv1.o p1fxns.o


thv1.o: thv1.c
thv2.o: thv2.c
thv3.o: thv3.c
thv4.o: thv4.c
p1fxns.o: p1fxns.c p1fxns.h 

clean:
	rm thv4 thv3 thv2 thv1 thv4.o thv3.o thv2.o thv1.o
