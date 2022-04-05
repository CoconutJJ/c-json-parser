CC=gcc
CFLAGS=-Ofast -Wall
OBJ=mem.o parser.o scanner.o

all: json clean 

json: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o json

debug: CFLAGS=-Og -g -Wall
debug: json clean

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $*.c

clean:
	rm *.o