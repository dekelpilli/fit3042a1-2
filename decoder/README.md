# RLE Decoder
This program takes a series of .ppm files (images) and plays them as a video. This was my first non-trivial program on C, and on Linux.

## Running the program

To compile the program: open the terminal in the location of `rledecode.c` and `Makefile` and type `make`.   

To run the program: open the terminal in the location of rledecode.c and Makefile and type `./rledecode _filename_ _prefix_`, where `filename` is the name of the rlefile you want to decode and `prefix` is the prefix the ppm file output will save to.   
The prefix must be no longer than 30 charcters.  
The file specified must be in the same directory as rledecode.c.  

In order to save the output to stdout, use `-` for the prefix.  

This does not include support for tweening and scaling.  
