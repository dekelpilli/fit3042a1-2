/*
Reads stdin input to play an RLE video with the desired delay, performing the desired image manipulations.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <math.h>


void setFrame(FILE * channel, int dimx, int dimy, SDL_Surface *surface, float brightnessFactor, float contrastFactor, float saturationFactor);
int skipHeader(FILE* channel);
void skipComment(FILE* channel); 
void setPixel(int x, int y, SDL_Surface *surface, int r, int g, int b);
int min(int a, int b);
int max(int a, int b);


/**
 *main: reads information from the header of the first frame then initiates the video and controls the framerate.
 *
 *Params: a delay value (in milliseconds) and, optionally, brightness, contrast and saturation.  
 *Preconditions: stdin must be filled with the correctly formatted data of a decoded rle file.
 *Returns: confirmation of completion (0).
 */

int main(int argc, char* argv[]) {
  if (argc !=2 && argc!=5) {
    fprintf(stderr, "Expecting one or four inputs, %i provided.\n", argc);
    exit(0);
  }
  int delay = atoi(argv[1]);

  int brightness = 50;
  int contrast = 50;
  int saturation = 50;

  if(argc==5) { //sets the optional brightness, contrast and saturation values
    brightness = atoi(argv[2]);
    contrast = atoi(argv[3]);
    saturation = atoi(argv[4]);
    }

  if(brightness <0 || contrast <0 || saturation <0 || brightness >100 || contrast >100 || saturation >100) {
    brightness = 50;
    contrast = 50;
    saturation = 50;
    fprintf(stderr, "Brightness, Contrast and Saturation must be values between 0-100.");
  }


  char ptype[2]; //should be P6
  int count = 0;
  while(count < 2){
    ptype[count] = fgetc(stdin);
    count++;
  } 
  ptype[2] = '\0';

  if(strcmp(ptype, "P6") != 0){
    fprintf(stderr, "Error: incorrect input format %s\n", ptype);
    exit(0);
  }
  

  char current = fgetc(stdin); //will usually be a newline char, but may be a comment

  while(current==' '||current=='\t'||current=='\n'||current=='#') { //skip comments 
    if(current=='#') {
      skipComment(stdin);
    }
    current = fgetc(stdin);
  } 
  

  char dimensionx[5];
  char dimensiony[5];
  count = 0;
  
  while(current!=' ' && current!='\t' && current!='\n'&& current!='#'){ //get dimx
    dimensionx[count] = current;
    count++;
    current = fgetc(stdin);
  }
    
  int dimx = atoi(dimensionx);
  
  count = 0;
  
  while(current==' '||current=='\t'||current=='\n'||current=='#') { // skip comments 
    if(current=='#') {
      skipComment(stdin);
    }
    current = fgetc(stdin);
  } 


  while(current!=' ' && current!='\t' && current!='\n'&& current!='#'){ //get dimy 
    dimensiony[count] = current;
    count++;
    current = fgetc(stdin);
  } 
  
  int dimy = atoi(dimensiony);

  //printf("dimx: %i, dimy: %i\n", dimx, dimy); 

  while(current==' '||current=='\t'||current=='\n'||current=='#') { // skip comments 
    if(current=='#') {
      skipComment(stdin);
    }
    current = fgetc(stdin); 
  } 
  
  count = 0;
  char colours[3];
  colours[0] = current;
  while(count<2) {
    current = fgetc(stdin);
    count++;
    colours[count] = current;   
  }

  colours[3] = '\0';

  if(strcmp(colours, "255") != 0){
    fprintf(stderr, "Error: incorrect colour format %s\n", colours);
    exit(0);
  }
  
  current = fgetc(stdin);
  while(current!='\n') { // skip comments 
    if(current=='#') {
      skipComment(stdin);
    }
    else {
      current = fgetc(stdin); 
    }
  }

  
  //finished reading header

  
  SDL_Window *window = NULL;
  SDL_Surface *screenSurface = NULL; //initialise the window and surface
  
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  }

  else {
    window = SDL_CreateWindow("ppmplayer video",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,dimx, dimy,SDL_WINDOW_SHOWN);

    if (window == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }

    else {
      /* Get screen surface */
      screenSurface = SDL_GetWindowSurface(window);

      // Fill surface with white 
      SDL_FillRect(screenSurface, NULL, 
		   SDL_MapRGB(screenSurface->format, 0xff, 0xff, 0xff));

      /* Update the screen */
      SDL_UpdateWindowSurface(window);

    }
  }
  
  
  //finished initialising window



  clock_t start;
  int ms; //time taken to write the frame
  int tooSlow = 0; //how many times writing the frame took longer than the desired delay time
  int firstTime = 1; //don't skip the header on the first frame as that has already been done when its data was collected


  float brightnessFactor = (brightness+10)/(float)60; //having a modified brightness factor ensures that an input brightness of 0 doesn't produce a purely black video
  float contrastFactor = (contrast)/(float)50; //referenced http://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-5-contrast-adjustment/
  float saturationFactor = saturation/(float)50;



  while(!feof(stdin)) { 
    start = clock();
    setFrame(stdin, dimx, dimy, screenSurface, brightnessFactor, contrastFactor, saturationFactor);
    if(!firstTime) {
      if(!(skipHeader(stdin))) { //if file ended during header read, break
	break;
      }
 
    }
    
    ms = ((clock()-start)*1000)/CLOCKS_PER_SEC;
    //printf("ms: %d\n", ms);
       
    if(ms>delay) {
      tooSlow++;
      if(tooSlow==2) { //if more than one frame was delayed, send error message (only once).
	fprintf(stderr, "Can't keep up with demanded framerate. Video will be played with some unwanted delay.\n");
      }
    }
    else {
      SDL_Delay(delay-ms); //wait until the delay time to achieve desired framerate
      }
    
  SDL_UpdateWindowSurface(window);  
  firstTime = 0;  
}
  

  SDL_Delay(1000); //wait 1 second before destroying the window
  SDL_DestroyWindow(window); 
  SDL_Quit(); 
  return(0); 
 } 


/**
 *skipComment: skips the line of the current comment 
 *
 *Parameters: channel to be read from
 *Preconditions: current character is a # (to mark the beginning of the comment)
 *Postconditions: the current line of the comment has been skipped 
 */

void skipComment(FILE* channel) { 
  char current = fgetc(channel);
  while(current != '\n'){
    current = fgetc(channel);
  }
} 


/**
 *skipHeader: skips the header, including any comments that appear before the last line of the regular header
 *
 *Parameters: channel from which to read the information
 *Preconditions: channel is currently on the first character of the header
 *Postconditions: header is skipped 
 *Returns: 1 if the header was skippped, or 0 if the file ended before the header was skipped
 */

int skipHeader(FILE* channel) {
  int lines = 3; //non-comment lines left in header
  char current; 
  int commentLine = 0; //bool to indicate if the current line is entirely a comment (and therefore shouldn't count towards the line count)
  while(!feof(stdin)) {
    current = fgetc(channel);
    if(current == '\n'  && commentLine==0) {
      lines--;
      if(lines == 0) {
	return(1);
      }
      
      if(fgetc(channel) == '#') {
	commentLine = 1;
      }
      else {
	commentLine = 0;
      }
    }  
  }
  return(0);
}

/**
 *setFrame: finds the colours of each pixel in the frame and sets it
 *
 *Parameters: channel/stream from which to read pixel information, dimensions of the frame and brightness, contrast and saturation information for image manipulation
 *Preconditions: the channel is currently on the start of the frame 
 *Postconditions: the frame has been set 
 */

void setFrame(FILE * channel, int dimx, int dimy, SDL_Surface *surface, float brightnessFactor, float contrastFactor, float saturationFactor) {
  
  int red, green, blue;
  int x, y; //current x and y positions
  
  char current; 
  double P; //http://alienryderflex.com/saturation.html
  float pr = 0.299;
  float pg = 0.587;
  float pb = 0.114;

  for (y = 0; y < dimy; y++) {
    for (x = 0; x < dimx; x++) {
      
      if(y==0 && x==0) { //for first pixel in frame, check for comments
	current = fgetc(channel);
	while(current == '#') {
	  skipComment(channel);
	  current = fgetc(channel);
	}
	red = current;
      }
      else {
	red = fgetc(channel);
      }
      green = fgetc(channel);
      blue = fgetc(channel);
      
      if(brightnessFactor !=1) {
	red = min(255, red*brightnessFactor);
	green = min(255, green*brightnessFactor);
	blue = min(255, blue*brightnessFactor);
      }
      
      if(contrastFactor!=1) {
	red = max(0, min(255, contrastFactor*(red-128)+128));
	green = max(0, min(255, contrastFactor*(green-128)+128));
	blue = max(0, min(255, contrastFactor*(blue-128)+128));
	}
      
      if(saturationFactor!=1) {
	P = sqrt(red*red*pr + green*green*pg + blue*blue*pb);
	red = max(0, min(255, P+((red-P)*saturationFactor)));
	green = max(0, min(255, P+((green-P)*saturationFactor)));
	blue = max(0, min(255, P+((blue-P)*saturationFactor)));
      }
      
      setPixel(x, y, surface, red, green, blue);
    }
  }
}


/**
 *setPixel: sets the specified pixel to have the given colours 
 *
 *Parameters: x and y to indicate the position of the pixel to be set, the surface where the pixel will be and the colours of the pixel (RGB)
 *Preconditions: none
 *Postconditions: the specified pixel is set
 */

void setPixel(int x, int y, SDL_Surface *surface, int red, int green, int blue) {
  int *p = surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
  *p = SDL_MapRGB(surface->format, red, green, blue);
}

/**
 *min: returns the smaller of the two numbers given
 *
 *Parameters: two integers
 *Preconditions: none
 *Returns: the smaller integer of the two
 */
int min(int a, int b) {
  if(a>b) {
    return(b);
  }
  else {
    return(a);
  }
}

/**
 *max: returns the larger of the two numbers given
 *
 *Parameters: two integers
 *Preconditions: none
 *Returns: the larger integer of the two
 */

int max(int a, int b) {
  if(a>b) {
    return(a);
  }
  else {
    return(b);
  }
}
