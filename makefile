CC=gcc

projectmake: main.c
	gcc main.c -g -o main -Wmaybe-uninitialized -Wuninitialized -Wall -pedantic -Werror -g3 
