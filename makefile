CC = gcc
FLAGS = -Wall

main : 
	$(CC) $(CFLAGS) stack.c maze.c -o maze
	gcc -Wall -fopenmp -o mazep maze.c stack.c

run : main
	./maze

runp : main
	./mazep 4 

mem : main
	valgrind ./maze

clean : 
	-rm maze
	-rm mazep
	clear
