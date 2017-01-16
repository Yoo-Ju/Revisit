OBJECTS=main.o debug.o Hcubes.o Tsequences.o utilities.o

CFLAGS=-Wall -O3
LIBS=-lm
CC=g++

PROGRAM_NAME=mista

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $< 

$(PROGRAM_NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(PROGRAM_NAME) $(OBJECTS) $(LIBS)
	
clean:
	rm -f *.o $(PROGRAM_NAME)
