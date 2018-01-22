# PPM Player
This program takes a series of .ppm files (images) and plays them as a video. 

## Running the program

To compile the program: open the terminal in the location of `rledecode.c` and `Makefile` and type `make`.   

To run the program: open the terminal in the location of rledecode.c and Makefile and type `./rledecode _filename_ - | ./ppmplayer _delay_ _brightness_ _contrast_ _saturation_`, where filename is the name of the rlefile you want to decode. More detailed information about rledecode's functions is available in its README.  

Delay will be the time, in milliseconds, between each frame. For example, a 24fps video will have ~42ms delay.  
Brightness, contrast and saturation are values between 0-100, where 50 gives an unchanged picture in terms of that parameter.   

The RLE file specified must be in the same directory as rledecode.c and ppmplayer.c.  

Limitations:   
The program will not run correctly if the headers of the ppms produced by rledecode start with a comment, or gives incorrect or inconsistent values for the x and y dimensions.  