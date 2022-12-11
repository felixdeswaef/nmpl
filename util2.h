/************************************************************************
Lab 2 Satellite Localisation
Author: Toon Goedemé

util2.h:
This file contains the headers for a set of utilities that can be used in
the second laboratory of the course on Image Interpretation
*************************************************************************/

/* From the standard C library: */
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

/*------------------------------ Macros  ---------------------------------*/

#define ABS(x)    (((x) > 0) ? (x) : (-(x)))
#define MAX(x,y)  (((x) > (y)) ? (x) : (y))
#define MIN(x,y)  (((x) < (y)) ? (x) : (y))


/*---------------------------- Structures --------------------------------*/

/* Data structure for a float image.
*/
typedef struct ImageSt {
  int rows, cols;          /* Dimensions of image. */
  float **pixels;          /* 2D array of image pixels. */
  struct ImageSt *next;    /* Pointer to next image in sequence. */
} *Image;


/* Data structure for a keypoint.  Lists of keypoints are linked
   by the "next" field.
*/
typedef struct KeypointSt {
  float row, col;             /* Subpixel location of keypoint. */
  float scale, ori;           /* Scale and orientation (range [-PI,PI]) */
  unsigned char *descrip;     /* Vector of descriptor values */
  struct KeypointSt *next;    /* Pointer to next keypoint in list. */
} *Keypoint;


/*-------------------------- Function prototypes -------------------------*/
/* These are prototypes for the external functions that are shared
   between files.
*/

/* From util2.c */
Image CreateImage(int rows, int cols);
void FreeImages(Image im);
Image FreeImage(Image im);
Image ReadPGMFile(char *filename);
Image ReadPGM(FILE *fp);
void WritePGMFile(char *filename, Image image);
void WritePGM(FILE *fp, Image image);
Image CombineImagesVertically(Image im1, Image im2);
void DrawLine(Image image, int r1, int c1, int r2, int c2);
void DrawRectangle(Image image, int r, int c, int w, int h);
Keypoint ReadKeyFile(char *filename);
Keypoint ReadKeys(FILE *fp); 
void FreeKeypoints(Keypoint kp);
Keypoint FreeKeypoint(Keypoint kp);
void WriteKeypoints(char *filename,Keypoint kp);
int LinearEquationsSolving(int nDim, double* pfMatr, double* pfVect, double* pfSolution);
