/*
 *   Copyright 2017, Moisés Pastor i Gadea
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *  Created on: 20/10/2017
 *      Author: Moisés Pastor i Gadea
 */

#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

int NUMCOLS_CONTEXT=100;
int NUMROWS_CONTEXT=50; 

const int FINALCOLS=50;
const int FINALROWS=30;


class labeledpoint{
public:
  int x,y;
  int label;
  bool operator <  (const labeledpoint & d)const {return y < d.y;}

};
//-------------------------------------------------------------
labeledpoint *readpoints(istream &fitx, int &length, bool &max){
  labeledpoint *points=0;
  string line;

  // Read header (ignoring comments)
  int cont=0;
  do{
    getline(fitx,line);
    cont++;
  }while (cont < 200 && (line[0]=='#' || line.empty()));
  istringstream l(line);
  string extr;
  l >> length >> extr;


  if(extr.compare("Min")==0)
    max = false;
  else if(extr.compare("Max")==0)
    max = true;
  else{
    cerr << "Wrong input format" << endl;
    length = -1;
    return points;
  }

  if (length <= 0){
    cerr << "Number of points = 0" << endl;
    return points;
  }
    
  do
    getline(fitx,line);
  while (line[0]=='#' || line.empty());  
  
  points = new labeledpoint[length];

  //just in case there is not a labeled point file
  for (int i = 0; i < length; i++) {
    points[i].label=0;
  }
  
  int i=0;
  do{
    if (line[0] != '#'){
      istringstream(line) >> points[i].x >> points[i].y >> points[i].label;
      i++;
    }
  }while(i<length && getline(fitx,line) );

  return points;
}


void sampleImage(const Mat arr, float idx0, float idx1, Scalar& res){

  if(idx0 < 0 || idx1 < 0 || idx0 >= arr.rows || idx1 >= arr.cols){
    res.val[0]=255;
    res.val[1]=255;
    res.val[2]=255;
    res.val[3]=255;
    return;
  }


 float idx0_fl=floor(idx0);
  float idx0_cl=ceil(idx0);
  float idx1_fl=floor(idx1);
  float idx1_cl=ceil(idx1);


  Scalar s1=arr.at<uchar>((int)idx0_fl,(int)idx1_fl);
  Scalar s2=arr.at<uchar>((int)idx0_fl,(int)idx1_cl);
  Scalar s3=arr.at<uchar>((int)idx0_cl,(int)idx1_cl);
  Scalar s4=arr.at<uchar>((int)idx0_cl,(int)idx1_fl);

  float x = idx0 - idx0_fl;
  float y = idx1 - idx1_fl;


  res.val[0]= s1.val[0]*(1-x)*(1-y) + s2.val[0]*(1-x)*y + s3.val[0]*x*y + s4.val[0]*x*(1-y);
 
}

float xscale;
float yscale;
float xshift;
float yshift;
//----------------------------------------------------------
float getRadialX(float x,float y,float cx,float cy,float k){
  x = (x*xscale+xshift);
  y = (y*yscale+yshift);
  float res = x+((x-cx)*k*((x-cx)*(x-cx)+(y-cy)*(y-cy)));
  return res;
}
//----------------------------------------------------------
float getRadialY(float x,float y,float cx,float cy,float k){
  x = (x*xscale+xshift);
  y = (y*yscale+yshift);
  float res = y+((y-cy)*k*((x-cx)*(x-cx)+(y-cy)*(y-cy)));
  return res;
}

//----------------------------------------------------------
float calc_shift(float x1,float x2,float cx,float k){
  float thresh = 1;
  float x3 = x1+(x2-x1)*0.5;
  float res1 = x1+((x1-cx)*k*((x1-cx)*(x1-cx)));
  float res3 = x3+((x3-cx)*k*((x3-cx)*(x3-cx)));

  //  std::cerr<<"x1: "<<x1<<" - "<<res1<<" x3: "<<x3<<" - "<<res3<<std::endl;

  if(res1>-thresh and res1 < thresh)
    return x1;
  if(res3<0){
    return calc_shift(x3,x2,cx,k);
  }
  else{
    return calc_shift(x1,x3,cx,k);
  }
}
//----------------------------------------------------------
void fishEye(Mat & img, Mat & dst, float K_V, float K_H){
  int centerX=img.cols/2;
  int centerY=img.rows/2;

  int width = img.cols;
  int height =img.rows ;

  xshift = calc_shift(0,centerX-1,centerX,K_H);
  float newcenterX = width-centerX;
  float xshift_2 = calc_shift(0,newcenterX-1,newcenterX,K_H);

  yshift = calc_shift(0,centerY-1,centerY,K_V);
  float newcenterY = height-centerY;
  float yshift_2 = calc_shift(0,newcenterY-1,newcenterY,K_V);
  //  scale = (centerX-xshift)/centerX;

  xscale = (width-xshift-xshift_2)/width;
  yscale = (height-yshift-yshift_2)/height;

  //aplicar ull de peix
  for(int r=0;r<dst.rows;r++){
    for(int c=0; c<dst.cols; c++){
      Scalar s;
      float x = getRadialX((float)c,(float)r,centerX,centerY,K_H);
      float y = getRadialY((float)c,(float)r,centerX,centerY,K_V);
      
      sampleImage(img,y,x,s);
      dst.at<uchar>(r,c)=s.val[0];
      //cvSet2D(dst,j,i,s);

    }
  }
}
//----------------------------------------------------------
void copyContext(Mat & img, Mat & img_context, int centerX, int centerY){

  for (int r = 0 ; r < NUMROWS_CONTEXT; r++) {
    for (int c = 0; c < NUMCOLS_CONTEXT ; c++) {
      int cOrig=centerX - (NUMCOLS_CONTEXT/2) + c;
      int rOrig=centerY - (NUMROWS_CONTEXT/2) + r;
      if (rOrig>=0 && rOrig<img.rows && cOrig>=0 && cOrig<img.cols)
	img_context.at<uchar>(r,c)=img.at<uchar>(rOrig,cOrig);
    }
  }
}
//----------------------------------------------------------
void getFeatures(Mat & img, int label){
  cout << label<<",";
  
  int cont=0;
  int maxCont=img.rows * img.cols;

  for(int r=0; r < img.rows; r++)
    for(int c=0; c<img.cols ;c++){
      cout << float(img.at<uchar>(r,c))/255;
      if (cont < maxCont-1)
      	cout << ",";
      cont++;
    }
    cout << endl;
}

//----------------------------------------------------------
void usage (char * programName){
 
  cerr << "Usage: "<< programName << " options " << endl;
  cerr << "      options:" << endl;
  cerr << "             -i imageFileName" << endl;
  cerr << "             -p pointsFileName read points from a file "<< endl;
  cerr << "             [-c #int NUMCOLS_CONTEXT (by default 100)]" << endl;
  cerr << "             [-r #int NUMROWS_CONTEXT (by default 50)]" << endl;
  // cerr << "             [-k #float] (by default 0.001)" << endl;
  // cerr << "             [-V #float] vertical curvature (by default 0)"<< endl;
  // cerr << "             [-H #float] horizontal curvature (by default 0)" << endl
    ;
  //cerr << "             [-l] labeledPoints file (by default not labeled)" << endl;
  //cerr << "             [-R] raw image, no fish eye applied (by default false)" << endl;
  cerr << "             [-x coordinate x] of center (demo)" << endl;
  cerr << "             [-y coordinate y] of center (demo)" << endl;

  cerr << "             [-o image outputfile] (demo) (by default stdout, if not -p file)" << endl;
  //cerr << "             [-v #int verbosity] " << endl;
}

int main (int argc, char** argv)   {

  string inFileName,outFileName,pointsFileName;
  int option;
  float K=0.01;
  float K_V=0, K_H=0;
  float centerX=-1,  centerY=-1;
  //bool applyFishEye=true;
  bool demo=true;

  if(argc == 1){
    usage(argv[0]);
    return -1;
  }
  

  while ((option=getopt(argc,argv,"hi:r:c:p:o:x:y:k:V:H:R"))!=-1)
    switch (option)  {
    case 'i':
      inFileName=optarg;
      break;
    case 'p':
      pointsFileName=optarg;
      demo=false;
      break;
    case 'r':
      NUMROWS_CONTEXT=atoi(optarg);
      break;
    case 'c':
      NUMCOLS_CONTEXT=atoi(optarg);
      break;
    // case 'k':
    //   K_V=atof(optarg);
    //   K_H=K_V;
    //   break;
      // case 'R':
      //   applyFishEye=false;
      //   break;
    // case 'H':
    //   K_H=atof(optarg);
    //   break;
    // case 'V':
    //   K_V=atof(optarg);
    //   break;
    case 'x':
      centerX = atoi(optarg);
      break;
    case 'y':
      centerY = atoi(optarg);
      break;
    case 'o':
      outFileName=optarg;
      demo=true;
      break;
    case 'h':
    default:
      usage(argv[0]);
      return 1;
    }
  
  if(demo && (centerX<0 || centerY<0)){
    cerr << "Error: Lens center position needed" << endl;
    return -1;
  }

  Mat img=imread(inFileName,0);
  if (!img.data) {
    cerr << "ERROR reading the image file "<< inFileName<< endl;
    return -1;
  }


  if (!demo){

    ifstream pfd;
    pfd.open(pointsFileName.c_str());
    if (!pfd){
      cerr << "Error: File \""<<pointsFileName << "\" could not be open "<< endl;
      return -1;
    }

    bool max;
    int npoints;
    labeledpoint * points = readpoints(pfd, npoints, max);
    

    if (npoints == 0){
      cerr << "fisheye. WARNING: no points to be processed" << endl;
      return -1;
    } 

    for(int p=0;p<npoints;p++){

      Mat img_context(Size(NUMCOLS_CONTEXT ,NUMROWS_CONTEXT),img.type());
      copyContext(img, img_context, points[p].x, points[p].y);

      //Mat img_fish(img_context.size(), img.type(), Scalar(255));

      //if (applyFishEye)	
      //fishEye(img_context,img_fish,K_V,K_H);
	// else 
	//img_fish=img_context;

	//submostrejar
      
      //resize(img_fish,img_fish,Size(FINALCOLS,FINALROWS));
     
      resize(img_context,img_context,Size(FINALCOLS,FINALROWS));
	// if (isLabeled) 
	//   label=points[p].label;

      //getFeatures(img_fish, points[p].label);
      getFeatures(img_context, points[p].label);
    }
  }else{
     Mat img_context(Size(NUMCOLS_CONTEXT,NUMROWS_CONTEXT),img.type());
     copyContext(img, img_context, centerX,  centerY);
     
    //aplicar ull de peix
    //Mat img_fish(img_context.size(), img.type(), Scalar(255));

    //if (applyFishEye)
    //fishEye(img_context,img_fish,K_V,K_H);
      //else 
      //img_fish=img_context;

    //submostrejar
    //resize(img_fish,img_fish,Size(FINALCOLS,FINALROWS));

     resize(img_context,img_context,Size(FINALCOLS,FINALROWS), 0, 0, INTER_CUBIC);
    
    //escriure a fitxer
    //imwrite(outFileName.c_str(),img_fish);
    imwrite(outFileName.c_str(),img_context);
    
  }
}
