#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>

int *getColours(int dimx,int dimy,FILE* file);
char *getSuff(int suffLen, int sufNum);
int writeToFile(int* red, int* green, int* blue, int dimx, int dimy, char* prefix, char* suffix);

/**
 *main: Gets colour values for each colour for each frame in the video. Most of the logic is done by functions called by main(), rather than inside main.
 *
 *Parameters: argv[] should include the name of the rle file to be read and the prefix of the output files, or a dash (-) for outputting to stdout.
 *Preconditions: rle file specified exists and is formatted correctly.
 *Returns: confirmation of completion (0).
 */


int main(int argc, char* argv[]) {
  if (access(argv[1], F_OK) == -1) {
    printf("%s", "Error: RLE File name given does not exist.\n");
    exit(0);
  }
    
  FILE *fp = fopen(argv[1], "r");

  if (strlen(argv[2])>34) {
    printf("%s", "Error: Prefix string is too long\n");
    exit(0);
  }
  char* prefix = malloc(sizeof(char) * (strlen(argv[2])+1));
  prefix = argv[2];

  char end; //used as a dummy var, then to check if there are remaining frames
  
  while(end  != '\n') { //skip first line
    end = fgetc(fp);
  }
 
  int dimx, dimy; //x and y dimensions of each frame, not to be confused with the dimensions of the 2D array colours
  
  fscanf(fp, "%d", &dimx);
  fscanf(fp, "%d", &dimy); //record the given x and y dimensions
  int pixels = dimx * dimy;
  end = fgetc(fp); //get rid of the \n seperating <header> and <frames>
  end = fgetc(fp); //get rid of first "K"
  int count = 1; //will keep track of how many files have been created(+1) for the purpose of naming
  while (end == 'K') { //while there is another frame
    int* red = getColours(dimx, dimy, fp);
    int* green = getColours(dimx, dimy, fp);
    int* blue = getColours(dimx, dimy, fp);
    


    if (strcmp(prefix, "-")==0) { //write to stdout if the argument given was a dash, or save to files otherwise
      writeToStdout(red, green, blue, dimx, dimy); 
    }
    else {
      
      char* suffix = getSuff(5,count);
      writeToFile(red, green, blue, dimx, dimy, prefix, suffix); 
      count++;   
      
    }
    
    free(red);
    free(green);
    free(blue);

    end = fgetc(fp); //will always be K for new frame or E for end of file
  }
  if (end == 'K'){
    printf("%s", "Error: reading the file stopped prematurely\n");
    exit(0);
  }
  fclose(fp);
          
  return(0);

}



  



/**
 *getColours: Reads all the colour values for one colour for a single frame
 *
 *Parameters: the dimensions of frames in the rlefile, dimx and dimy, and a pointer to the file itself.
 *Preconditions: The next character at the time of calling this function must be the first of a colour representation for a frame.
 *Returns: an array of integers representing one colour's 0-255 value for each pixel in a frame.
 */


int *getColours(int dimx,int dimy,FILE* rlefile) {
  int *colour;
  int pixels = dimx*dimy;
  int c; //colour value
  int i, j; //used for handling block header instructions
  char n; //represents the block header
  int index = 0;

  colour  = (int *)malloc(sizeof(int)*(pixels+1));
  while (index<pixels) { //repeat for each pixel in a frame
    n = fgetc(rlefile); //read block header
	if (n >=  0){
	  j = n + 1;
	  for (i=0;i<j;i++){ //copy the next n + 1 symbols
	    c = fgetc(rlefile);
	    colour[index] = c;
	    index++;
	  }
	  
	}
	else if (n<0){
	  j = 2 - n;
 	  c = fgetc(rlefile);
	  for (i=0;i<j;i++){ //write 2 - n copies of the next symbol
	    colour[index] = c;
	    index++;
	  }
	}
    }
  
  return(colour);
}

/**
 *Create a string to represent the next suffix
 *
 *Parameters: the desired length of the suffix and how many suffixes have already been created.
 *Returns: a string representing the suffix for a ppm file. 
 */


char *getSuff(int suffLen, int suffNum) {
  char* suff = malloc(sizeof(char) * suffLen+1);
  suff[0] = '-'; //start each suffix with -
  int count = 0;
  
  while (count < suffLen) {//fill suffix array with 0s
    count++;
    suff[count] = '0';
  }
  
  int numLen = ceil(log10(suffNum+1))*sizeof(char); //find how many characters are in the suffNum (eg 100 will be 3, whereas 99 will be 2)
  char* val = malloc((sizeof(char) * numLen)+1);
  sprintf(val, "%d", suffNum); //suffNum as a string
  count = 0;
  
  while (count <= numLen) {
    suff[suffLen-count] = val[numLen-count]; //replace the 0s in suffix with appropriate suffNum values, from end of string to the start
    count++;
  }
 
  //free(val);
  return(suff);
}

/**
 *Write a frame to stdout to a ppm file based on the prefix and suffix given.
 *
 *Parameters: the colour values of each pixel in the frame, as well as frame dimensions and file name details.
 *Returns: a confirmation of a successful write.
 */


int writeToFile(int* red, int* green, int* blue, int dimx, int dimy, char* prefix, char* suffix) {
  char* filename = malloc(sizeof(char) * (strlen(prefix)+strlen(suffix)+1)); //allocate space for the filename, no bigger than 30 characters
  sprintf(filename, "%s%s", prefix, suffix); //combine the prefix and the suffix to make the filename
  int pixels = dimx * dimy;
  FILE*  ppm = fopen(filename, "w+");
  fprintf(ppm, "P6\n%d %d\n%d\n",dimx, dimy, 255); //write the header to the ppm file
  int write = 0; //keeps track of how many pixels have been written already
  
  while (write<(pixels-1)) { //for each pixel
    static unsigned char pixel[3]; //used https://rosettacode.org/wiki/Bitmap/Write_a_PPM_file#C
    pixel[0] = red[write];
    pixel[1] = green[write];
    pixel[2] = blue[write];
    write++;
    (void) fwrite(pixel, 1, 3, ppm); //write the pixel's RGB values to the ppm
  }
  
  if (write>pixels) {
    printf("%s", "Error: more pixels read than should be in a frame\n");
    exit(0);
  }
  fclose(ppm);
  free(filename);
  return(0);

}

/**
 *Write a frame to stdout, following ppm convetions
 *
 *Parameters: the colour values of each pixel in the frame, as well as frame dimensions
 *Returns: a confirmation of a successful write.
 */

int writeToStdout(int * red, int* green, int* blue, int dimx, int dimy) {
  fprintf(stdout, "P6\n%d %d\n%d\n",dimx, dimy, 255); //write the header to stdout
  int write = 0;//keeps track of how many pixels have been written already
  int pixels = dimx * dimy;
  
  while (write<(pixels-1)) { //for each pixel
    static unsigned char pixel[3];
    pixel[0] = red[write];
    pixel[1] = green[write];
    pixel[2] = blue[write];
    write++;
    (void) fwrite(pixel, 1, 3, stdout); //write the pixel's RGB values to stdout
  }
  
  if (write>pixels) {
    printf("%s", "Error: more pixels read than should be in a frame\n");
    exit(0);
  }
  //fprintf(stdout, "%c", -1);
  int sep = -1;
  fwrite(&sep, sizeof(int), 1, stdout);
  return(0);
}
