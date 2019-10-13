#compiler
CC = gcc

#input files
INPUT = main.o graphics.o utility.o vecmat.o

#compiler flags
FLAGS = -g -Wall

#external libraries
LIBS = -lSDL2 -lSDL2main -lm

#output file
OUTPUT = raycaster

all: $(INPUT)
	$(CC) $(INPUT) $(FLAGS) $(LIBS) -o $(OUTPUT)
	
main.o: main.c
	gcc main.c -c $(FLAGS)
	
graphics.o: graphics.c
	gcc graphics.c -c $(FLAGS)
	
utility.o: utility.c
	gcc utility.c -c $(FLAGS)
	
vecmat.o: vecmat.c
	gcc vecmat.c -c $(FLAGS)
	
clean:
	rm -f $(INPUT)
	
cleanall:
	rm -f $(INPUT) $(OUTPUT)

