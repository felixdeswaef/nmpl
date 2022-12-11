#include "main.h"


int main(int argc,char** argv)
{
	if(argc < 2) {
		printf("te weinig args\n");
		return -1;
	}
	AlpOE ingelezen[MAXFOTO];
    Keypoint keypoints[MAXFOTO];
	printf("lees %s \n",(char*)argv[1]);
	int cnt = read((char *)argv[1],ingelezen);
    readKP(keypoints,cnt);
    FilterKpBbSet(ingelezen,keypoints,cnt);
    CrossMatchKpSet(ingelezen,keypoints,cnt);


}
float fdistS(float x1,float y1,float x2,float y2)
{
    return (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
}
void CrossMatchKpSet(AlpOE * ingelezen,Keypoint keypoints[], int count) {
    for (int i = 0;i<1;i++) //i<count -1
    {
        int matchcnt = 0;
        //crosscheck for all
        Keypoint knp,nknp;
        Keypoint bestk,sbestk;
        Keypoint k = keypoints[i];
        Keypoint nk = keypoints[i+1];
        float best = FLOATMAX;
        float sbest = FLOATMAX;
        float th = 0.7;
        float ths = th * th;
        int ncc = 0;
        do{
            knp = (k->next);
            best = FLOATMAX;
            sbest = FLOATMAX;
            do{

                nknp = (nk->next);
                //doesnt work
                if (strncmp(k->descrip,nk->descrip,KPDESCLEN))
                {

                    float  dist = fdistS(k->row,k->col,nk->row,nk->col);
                    if(matchcnt<2)printf("matched %f dist %f	",dist,best);

                    if (dist<best)
                    {
                        sbest = best;
                        best = dist;
                        sbestk = bestk;
                        bestk = nk;

                    }else if(dist<sbest)
                    {
                        sbestk = nk;
                        sbest = dist;
                    }else ncc++;

                }

                nk = nknp;
            }while(nknp != NULL);
            //threshhold
            //printf("match dist %f %f ncc%d		",best,sbest,ncc);
            if(best/sbest<= ths )
            {
                matchcnt++;
            }
            k = knp;
        }while(knp != NULL);
        printf("tot %d\n",matchcnt);

    }
}
int KpCount(Keypoint KPP){
    int cnt = 0;
    Keypoint * np;
    do{
        np = (KPP->next);
        cnt++;
        KPP = np;
    }while(np != NULL);
    return cnt;
}

void ImageMult(Image i,float Mult){
    for (int r = 0;r<i->rows;r++)
        for (int c = 0;c<i->cols;c++)
            i->pixels[r][c]=i->pixels[r][c]*Mult;
}
void ImageFill(Image i,int fill){
    for (int r = 0;r<i->rows;r++)
        for (int c = 0;c<i->cols;c++)
            i->pixels[r][c]=fill;
}
int Clamp(int coord,int max)
{
    return(coord <= max)?((coord >= 0)?coord:0):max;
}
Rect ClampRect(Rect r,int max_X ,int max_Y)
{
    int sx = r.bbxc;
    int sy = r.bbyc;
    int ex = r.bbxc+r.bbb;
    int ey = r.bbyc+r.bbh;

    sx = Clamp(sx,max_X);
    ex = Clamp(ex,max_X);
    sy = Clamp(sy,max_Y);
    ey = Clamp(ey,max_Y);

    r.bbxc = sx;
    r.bbyc = sy;
    r.bbb = ex-sx;
    r.bbh = ey-sy;
    return r;

}
void FilterKpBbSet(AlpOE * ingelezen,Keypoint * keypoints, int count){
    for (int i = 0;i<count;i++)
    {
        Image im = ReadPGMFile(ingelezen[i].Bestn);
        ImageMult(im,0.2F);
        Rect r = ScaleBox(ingelezen[i].PRect,3,5);
        r = ClampRect(r,im->cols,im->rows);
        DrawRectangle(im,

                      ingelezen[i].PRect.bbyc,
                      ingelezen[i].PRect.bbxc,
                      ingelezen[i].PRect.bbb,
                      ingelezen[i].PRect.bbh
                      );

        DrawRectangle(im,
                       r.bbyc,
                       r.bbxc,
                       r.bbb,
                       r.bbh);
        char fn[STRLEN] ;
        sprintf(fn,"outfile%d.pgm" , i+1);
        printf("wrote %s\n",fn);

        //DrawKpS(keypoints[i],im);
        ImageMult(im,0.5F);
        keypoints[i] = FilterKpBb(r,keypoints[i]);

        int kcnt = DrawKpS(keypoints[i],im);
        printf("%d\n",kcnt);
        WritePGMFile(fn,im);
    }
}
void DrawKp(Keypoint KPP,Image im){
    DrawRectangle(im,KPP->row,KPP->col,2,2);
}
int DrawKpS(Keypoint KPP,Image im)
{
    int cnt = 0;
    Keypoint * np;
    do{
        np = (KPP->next);
            DrawKp(KPP,im);
            cnt++;
        KPP = np; ///YOU DUMB FUCK
    }while(np != NULL);
    return cnt;
}
Keypoint FilterKpBb(Rect r ,Keypoint KPP)
{
    //TODO rewrite with first value as a return value. making shure not to rewrite since it seems to break the array higher up
    //KPP is first pointer &KPP is its adress
    // so switching out the first means writing to &KPP instead of next
    Keypoint first = KPP;//contains first pointer if it doesnt get dumped it will be returned
    Keypoint * PrevNextField; //contains next pointer for previous //this might be where it goes wrong
    //ugly first run loop
    Keypoint np = (KPP->next);
    int count= 0;
    int cn1 = 0;
    int cn2 = 0;
    int flag = 0;//mark past 1st
    do{
        np = (KPP->next); //next element to filter
        //if(_DEB_)printf("%d\t%d\n ",np,KPP);
        if(inBB(r,(int)KPP->col,(int)KPP->row))
        {//pass so keep
            if(!flag) flag = 1; // mark flag if pass
            count++;
            PrevNextField = &(KPP->next); //adress of the field containing next pointer (if we need to change it we'll write to this)
            //if(_DEB_)printf("keeping %d\t%d\n ",np,count);
        }
        else //dump
        if (flag)//normal
        {
            *PrevNextField = np; //write to next field in previous accepted keypoint
            cn2++;
        }else
        {//dump first in row
            first = np;// first has changed
            cn1++;
        }

        KPP = np;
    }while(np != NULL);
    if(_DEB_)printf("keeping %d dumpf %d dmpn %d \n",count,cn1,cn2);
    return first;
}
void readKP(Keypoint * KPP,int cnt)
{
    char nm[STRLEN];
    for(int i = 0; i<cnt;i++)
    {
        sprintf(nm,"keypoints%d.key",i+1);
        KPP[i] = ReadKeyFile(nm);
    }
}
Rect ScaleBoxF(Rect r,float Sx,float Sy){

    //printBB(r);
    return ScaleBox(r,(int)Sx,(int)Sy);
}
Rect ScaleBox(Rect r,int Sx,int Sy){
    float newb = r.bbb*Sx;
    float newg = r.bbh*Sy;

    Rect out;

    out.bbb = newb;
    out.bbh = newg;

    out.bbyc = r.bbyc-((out.bbh-r.bbh)/2.0f);//-((out.bbb-r.bbb)/2.0f);
    out.bbxc = r.bbxc-((out.bbb-r.bbb)/2.0f);


    //printBB(r);
    //printBB(out);
    return out;
}
void boxprintS(int w){
    printf("\xDA");
    for(int i = 0;i<w;i++)printf("--------");
    printf("\xBF\n");
}
void boxprintE(int w){
    printf("\xC0");
    for(int i = 0;i<w;i++)printf("--------");
    printf("\xD9\n");
}
void printBB(Rect r ){
    printf("width:%d height:%d \n",r.bbb,r.bbh);
    boxprintS(4);
    printf("|%3d:%3d \t\t%3d:%3d  |\n",r.bbxc,r.bbyc,r.bbxc+r.bbb,r.bbyc);
    printf("|\t%3d:%3d \t\t |\n",r.bbxc+(r.bbb/2.0f),r.bbyc+(r.bbh/2.0f));
    printf("|%3d:%3d \t\t%3d:%3d  |\n",r.bbxc,r.bbyc+r.bbh,r.bbxc+r.bbb,r.bbyc+r.bbh);
    boxprintE(4);
}
int inBB(Rect r,int x,int y)
{
    return ((x>=r.bbxc&&x<(r.bbxc+r.bbb))&&(y>=r.bbyc&&y<(r.bbyc+r.bbh)));
}


int read(char * s,AlpOE * res)
{
    int cnt = 0;
	FILE * fp; 
        fp = fopen( s ,"r");
	printf("%s geopend\n",s);
	while(fscanf(fp,"%s %d\n",res->Bestn,&res->FCharCnt)!=EOF)
	{
		printf("reading %s w/ %d results \n",res->Bestn,res->FCharCnt);
		//bounding box
		fscanf (fp,"%d %d %d %d\n",
                	&(res->PRect.bbxc),              
                        &(res->PRect.bbyc),
                        &(res->PRect.bbb), 
                        &(res->PRect.bbh));
		//chars
		for(int i = 0 ; i < res->FCharCnt;i++) 
		{	
			fscanf (fp,"%c %d %d %d %d\n",
				&res->FChar[i].CVal,
				&res->FChar[i].CRect.bbxc,
				&res->FChar[i].CRect.bbyc,
				&res->FChar[i].CRect.bbb,
				&res->FChar[i].CRect.bbh);
			printf(	"read %c \n",res->FChar[i].CVal);
		}
        cnt++;
		res++;

	}
	fclose(fp);
    return cnt;


}
