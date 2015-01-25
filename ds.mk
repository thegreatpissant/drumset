CC = g++
CFLAGS = `sdl-config --cflags` 
LIBS = `sdl-config --libs`
OPTS =  -Wall

main : main.c
	${CC} ${OPTS} -o dset main.c ${CFLAGS} ${LIBS}
	
