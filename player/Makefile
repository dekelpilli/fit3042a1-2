CFLAGS=`sdl2-config --cflags`
SDLLIBS=`sdl2-config --libs`


all:
	gcc -o rledecode rledecode.c -lm
	gcc -Wall -o ppmplayer ppmplayer.c $(CFLAGS) $(SDLLIBS) -lm

clean:
	rm -rf *o rledecode
