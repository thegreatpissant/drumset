CFLAGS = `sdl-config --cflags` 
LIBS = `sdl-config --libs`
OPTS =  -Wall

main : main.c
	${CXX} ${OPTS} -o dset main.c ${CFLAGS} ${LIBS}

clean : dset 
	rm dset	
