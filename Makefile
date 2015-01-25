CC = g++
CFLAGS = `paragui-config --cflags` ` pkg-config --cflags sigc++-1.2`
LIBS = `paragui-config --libs` `pkg-config --libs sigc++-1.2`
OPTS =  -Wall

main : main.c
	${CC} ${OPTS} -o drumset main.c ${CFLAGS} ${LIBS}
	
