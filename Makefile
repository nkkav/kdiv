CC = gcc
CFLAGS = -Wall -O2
EXE = .exe

all: kdiv$(EXE)

kdiv$(EXE): kdiv.o
	$(CC) kdiv.o -o kdiv$(EXE)

kdiv.o: kdiv.c
	$(CC) $(CFLAGS) -c kdiv.c

tidy:
	rm -f *.o

clean:
	rm -f *.o kdiv$(EXE) kdiv_*.nac kdiv_u*.c kdiv_s*.c

