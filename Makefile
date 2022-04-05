CC=gcc
CFLAGS=-Ofast -Wall
OBJ=mem.o parser.o scanner.o

all: lib

lib: $(OBJ) 
	ar rcs libjson.a $(OBJ)
	mkdir -p build
	mv libjson.a build
	cp parser.h build/json.h
	rm *.o

json: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o json

debug: CFLAGS=-Og -g -Wall
debug: json clean

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $*.c

clean:
	rm *.o