CC= gcc -g -Wall -ansi -pedantic
CCL= g++

LIB =  -lglui -lglut -lGL -lGLU -lm -lasound -lpthread

midi:   midmidi.c
	$(CCL) midmidi.c -o midi $(LIB)

clean:
	/bin/rm -f *~ *.o 
