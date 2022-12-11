/************************************************************************
Lab 2 Satellite Localisation
Author: Toon Goedemé

util2.c:
This file contains routines for creating floating point images, and
reading and writing PGM files:

      Image CreateImage(row,cols) - Create an image data structure.
      ReadPGM(filep) - Returns list of images read from the PGM format file.
      WritePGM(filep, image) - Writes an image to a file in PGM format.
      CombineImagesVertically(Image im1, Image im2) - combines two images vertically in order to display matches
      DrawLine(Image image, int r1, int c1, int r2, int c2) - draw a line on an image
      ReadKeyFile(char *filename) - read in a keypoint file  
      LinearEquationsSolving(int nDim, double* pfMatr, double* pfVect, double* pfSolution) - Solve linear system using Gauss
*************************************************************************/

#include "util2.h"
//#include <stdarg.h>

/* -------------------- Local function prototypes ------------------------ */

float **AllocMatrix(int rows, int cols);
void SkipComments(FILE *fp);


/*----------------- Routines for image creation ------------------------*/

/* Create a new image with uninitialized pixel values.
*/
Image CreateImage(int rows, int cols)
{
    Image im;

    im = (Image) malloc(sizeof(struct ImageSt));
    im->rows = rows;
    im->cols = cols;
    im->pixels = AllocMatrix(rows, cols);
    im->next = NULL;
    return im;
}


/* Allocate memory for a 2D float matrix of size [row,col].  This returns
     a vector of pointers to the rows of the matrix, so that routines
     can operate on this without knowing the dimensions.
*/
float **AllocMatrix(int rows, int cols)
{
    int i;
    float **m, *v;

    m = (float **) malloc(rows * sizeof(float *));
    v = (float *) malloc(rows * cols * sizeof(float));
    for (i = 0; i < rows; i++) {
	m[i] = v;
	v += cols;
    }
    return (m);
}

/* Vrijgeven van gevraagd geheugen
*/
Image FreeImage(Image im)
{
    Image result=0;
    if(im)
    {
        result=im->next;
        free(*im->pixels);
        free(im->pixels);
        free(im);
    }
    return result;
}

void FreeImages(Image im)
{
    while(im)
        im=FreeImage(im);
}


/*----------------- Read and write PGM files ------------------------*/


/* This reads a PGM file from a given filename and returns the image.
*/
Image ReadPGMFile(char *filename)
{	
	Image result=0;
    FILE *file;

    /* The "b" option is for binary input, which is needed if this is
       compiled under Windows.  It has no effect in Linux.
    */
    file = fopen (filename, "rb");
    if (! file)
		fprintf(stderr, "Error: Could not open file\n");
	else
	{
		result=ReadPGM(file);
		fclose(file);
	}

    return result;
}


/* Read a PGM file from the given file pointer and return it as a
   float Image structure with pixels in the range [0,1].  If the file
   contains more than one image, then the images will be returned
   linked by the "next" field of the Image data structure.  
     See "man pgm" for details on PGM file format.  This handles only
   the usual 8-bit "raw" PGM format.  Use xv or the PNM tools (such as
   pnmdepth) to convert from other formats.
*/
Image ReadPGM(FILE *fp)
{
  int char1, char2, width, height, max, c1, c2, c3, r, c;
  Image image, nextimage;

  char1 = fgetc(fp);
  char2 = fgetc(fp);
  SkipComments(fp);
  c1 = fscanf(fp, "%d", &width);
  SkipComments(fp);
  c2 = fscanf(fp, "%d", &height);
  SkipComments(fp);
  c3 = fscanf(fp, "%d", &max);

  if (char1 != 'P' || char2 != '5' || c1 != 1 || c2 != 1 || c3 != 1 ||
      max > 255)
    fprintf(stderr, "Error:Input is not a standard raw 8-bit PGM file.\n");

  fgetc(fp);  /* Discard exactly one byte after header. */

  /* Create floating point image with pixels in range [0,1]. */
  image = CreateImage(height, width);
  for (r = 0; r < height; r++)
    for (c = 0; c < width; c++)
      image->pixels[r][c] = ((float) fgetc(fp)) / 255.0;

  /* Check if there is another image in this file, as the latest PGM
     standard allows for multiple images. */
  SkipComments(fp);
  if (getc(fp) == 'P') {
    ungetc('P', fp);
    nextimage = ReadPGM(fp);
    image->next = nextimage;
  }
  return image;
}


/* PGM files allow a comment starting with '#' to end-of-line.  Skip
   white space including any comments.
*/
void SkipComments(FILE *fp)
{
    int ch;

    fscanf(fp," ");      /* Skip white space. */
    while ((ch = fgetc(fp)) == '#') {
      while ((ch = fgetc(fp)) != '\n'  &&  ch != EOF)
	;
      fscanf(fp," ");
    }
    ungetc(ch, fp);      /* Replace last character read. */
}


/* This writes an image to a PGM file with a given filename
*/
void WritePGMFile(char *filename, Image image)
{
    FILE *file;

    /* The "b" option is for binary input, which is needed if this is
       compiled under Windows.  It has no effect in Linux.
    */
    file = fopen (filename, "wb");
    
    WritePGM(file, image);
	fclose(file);
}

/* Write an image to the file fp in PGM format.
*/
void WritePGM(FILE *fp, Image image)
{
    int r, c, val;

    fprintf(fp, "P5\n%d %d\n255\n", image->cols, image->rows);

    for (r = 0; r < image->rows; r++)
      for (c = 0; c < image->cols; c++) {
	val = (int) (255.0 * image->pixels[r][c]);
	fputc(MAX(0, MIN(255, val)), fp);
      }
}


/* Return a new image that contains the two images with im1 above im2.
*/
Image CombineImagesVertically(Image im1, Image im2)
{
    int rows, cols, r, c;
    Image result;

    rows = im1->rows + im2->rows;
    cols = MAX(im1->cols, im2->cols);
    result = CreateImage(rows, cols);

    /* Set all pixels to 0,5, so that blank regions are grey. */
    for (r = 0; r < rows; r++)
      for (c = 0; c < cols; c++)
	result->pixels[r][c] = 0.5;

    /* Copy images into result. */
    for (r = 0; r < im1->rows; r++)
      for (c = 0; c < im1->cols; c++)
	result->pixels[r][c] = im1->pixels[r][c];
    for (r = 0; r < im2->rows; r++)
      for (c = 0; c < im2->cols; c++)
	result->pixels[r + im1->rows][c] = im2->pixels[r][c];
    
    return result;
}

/* Draw a white line from (r1,c1) to (r2,c2) on the image.  Both points
   must lie within the image.
*/
void DrawLine(Image image, int r1, int c1, int r2, int c2)
{
    int i, dr, dc, temp;

    if (r1 == r2 && c1 == c2)  /* Line of zero length. */
      return;

    /* Is line more horizontal than vertical? */
    if (ABS(r2 - r1) < ABS(c2 - c1)) {

      /* Put points in increasing order by column. */
      if (c1 > c2) {
	temp = r1; r1 = r2; r2 = temp;
	temp = c1; c1 = c2; c2 = temp;
      }
      dr = r2 - r1;
      dc = c2 - c1;
      for (i = c1; i <= c2; i++)
	image->pixels[r1 + (i - c1) * dr / dc][i] = 1.0;

    } else {

      if (r1 > r2) {
	temp = r1; r1 = r2; r2 = temp;
	temp = c1; c1 = c2; c2 = temp;
      }
      dr = r2 - r1;
      dc = c2 - c1;
      for (i = r1; i <= r2; i++)
	image->pixels[i][c1 + (i - r1) * dc / dr] = 1.0;
    }
}

/* Draw a white rectangle from (r,c) with width w and height h on the image.  En
tire rectangle must lie within the image.
*/
void DrawRectangle(Image image, int r, int c, int w, int h)
{
    DrawLine(image, r, c, r+h, c);
    DrawLine(image, r+h, c, r+h, c+w);
    DrawLine(image, r+h, c+w, r, c+w);
    DrawLine(image, r, c, r, c+w);
}


/*---------------------- Read keypoint file ---------------------------*/

/* This reads a keypoint file from a given filename and returns the list
   of keypoints.
*/
Keypoint ReadKeyFile(char *filename)
{
	Keypoint result=0;
    FILE *file;

    file = fopen (filename, "r");
    if (! file)
		printf("Could not open file: %s\n", filename);
	else
	{
		result=ReadKeys(file);
		fclose(file);
	}

    return result;
}

/* Read keypoints from the given file pointer and return the list of
   keypoints.  The file format starts with 2 integers giving the total
   number of keypoints and the size of descriptor vector for each
   keypoint (currently assumed to be 128). Then each keypoint is
   specified by 4 floating point numbers giving subpixel row and
   column location, scale, and orientation (in radians from -PI to
   PI).  Then the descriptor vector for each keypoint is given as a
   list of integers in range [0,255].

*/
Keypoint ReadKeys(FILE *fp)
{
    int i, j, num, len, val;
    Keypoint k, keys = NULL;

    if (fscanf(fp, "%d %d", &num, &len) != 2)
	printf("Invalid keypoint file beginning.\n");

    if (len != 128)
	printf("Keypoint descriptor length invalid (should be 128).\n");

    for (i = 0; i < num; i++) {
      /* Allocate memory for the keypoint. */
      k = (Keypoint) malloc(sizeof(struct KeypointSt));
      k->next = keys;
      keys = k;
      k->descrip = malloc(len);

      if (fscanf(fp, "%f %f %f %f", &(k->row), &(k->col), &(k->scale),
		 &(k->ori)) != 4)
	printf("Invalid keypoint file format.\n");

      for (j = 0; j < len; j++) {
	if (fscanf(fp, "%d", &val) != 1 || val < 0 || val > 255)
	  printf("Invalid keypoint file value.\n");
	k->descrip[j] = (unsigned char) val;
      }
    }
    return keys;
}

/* Vrijgeven van gevraagd geheugen
*/
Keypoint FreeKeypoint(Keypoint kp)
{
    Keypoint result=0;
    if(kp)
    {
        result=kp->next;
        free(kp->descrip);
        free(kp);
    }
    return result;
}

void FreeKeypoints(Keypoint kp)
{
    while(kp)
        kp=FreeKeypoint(kp);
}

void WriteKeypoints(char *filename,Keypoint kp)
{
	FILE *file;
	Keypoint k=NULL;
	int len=128;		
	int tel=0;

    file = fopen (filename, "wb");
	for(k=kp;k!=NULL; k=k->next)
	{
		fprintf(file,"%8.3f %8.3f %8.3f %8.3f\n",k->row,k->col,k->scale,k->ori);
		for(int j=0;j<len;j++)
		{
			fprintf(file,"%4d",k->descrip[j]);
		}
		fprintf(file,"\n");
		tel++;
	}
	fseek(file,0,SEEK_SET);
	fprintf(file,"%d %d\n",tel,len);
	fclose(file);
}
//==============================================================================
// return 1 if system not solving
// nDim - system dimension
// pfMatr - matrix with coefficients
// pfVect - vector with free members
// pfSolution - vector with system solution
// pfMatr becames trianglular after function call
// pfVect changes after function call
//
// Developer: Henry Guennadi Levkin
//
//==============================================================================
int LinearEquationsSolving(int nDim, double* pfMatr, double* pfVect, double* pfSolution)
{
  double fMaxElem;
  double fAcc;

  int i , j, k, m;


  for(k=0; k<(nDim-1); k++) // base row of matrix
  {
    // search of line with max element
    fMaxElem = fabs( pfMatr[k*nDim + k] );
    m = k;
    for(i=k+1; i<nDim; i++)
    {
      if(fMaxElem < fabs(pfMatr[i*nDim + k]) )
      {
        fMaxElem = pfMatr[i*nDim + k];
        m = i;
      }
    }
    
    // permutation of base line (index k) and max element line(index m)
    if(m != k)
    {
      for(i=k; i<nDim; i++)
      {
        fAcc               = pfMatr[k*nDim + i];
        pfMatr[k*nDim + i] = pfMatr[m*nDim + i];
        pfMatr[m*nDim + i] = fAcc;
      }
      fAcc = pfVect[k];
      pfVect[k] = pfVect[m];
      pfVect[m] = fAcc;
    }

    if( pfMatr[k*nDim + k] == 0.) return 1; // needs improvement !!!

    // triangulation of matrix with coefficients
    for(j=(k+1); j<nDim; j++) // current row of matrix
    {
      fAcc = - pfMatr[j*nDim + k] / pfMatr[k*nDim + k];
      for(i=k; i<nDim; i++)
      {
        pfMatr[j*nDim + i] = pfMatr[j*nDim + i] + fAcc*pfMatr[k*nDim + i];
      }
      pfVect[j] = pfVect[j] + fAcc*pfVect[k]; // free member recalculation
    }
  }

  for(k=(nDim-1); k>=0; k--)
  {
    pfSolution[k] = pfVect[k];
    for(i=(k+1); i<nDim; i++)
    {
      pfSolution[k] -= (pfMatr[k*nDim + i]*pfSolution[i]);
    }
    pfSolution[k] = pfSolution[k] / pfMatr[k*nDim + k];
  }

  return 0;
}
