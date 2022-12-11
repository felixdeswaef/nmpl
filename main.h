#define STRLEN 20
#define MAXFOTO 4
#define MAXPLATECHARS 10
#define KPDESCLEN 128
#define FLOATMAX 9000000.0
#define _DEB_ 1
#include <stdio.h>
#include <stdlib.h>
#include "util2.h"

typedef struct rect{
	int bbxc;
	int bbyc;
	int bbb;
	int bbh;
	
}Rect;
typedef struct alpOEC{
	char CVal;
	Rect CRect;
}AlpOEC;
typedef struct alpOE{
	char Bestn[STRLEN];
	int FCharCnt;
	Rect PRect;
	AlpOEC FChar[MAXPLATECHARS];

}AlpOE;

int main(int,char**);
int read(char * s,AlpOE * res);
void readKP(Keypoint * KPP,int);
void FilterKpBbSet(AlpOE * ingelezen,Keypoint * keypoints, int count);
void CrossMatchKpSet(AlpOE * ingelezen,Keypoint * keypoints, int count);
Keypoint FilterKpBb(Rect r ,Keypoint KPP);
void DrawKp(Keypoint KPP,Image im);
int KpCount(Keypoint KPP);
int DrawKpS(Keypoint KPP,Image im);
void boxprintS(int w);
void boxprintE(int w);
void printBB(Rect r );

int inBB(Rect r,int x,int y);
Rect ScaleBoxF(Rect r,float Sx,float Sy);
Rect ScaleBox(Rect r,int Sx,int Sy);
