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

 *  Created on: 20/10/2017
 *      Author: Moisés Pastor i Gadea
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

//per al sort
bool compare_pointsX(Point a, Point b){
  return a.x < b.x;
}

//-----------------------------------------------------------
vector<Point> * purge(vector<Point> &points, Mat & image){

  vector<Point>::iterator it = points.begin();
  //llevem duplicats
   std::sort(points.begin(), points.end(), compare_pointsX);
   it = points.begin();
   while(it != points.end()){
     if ((*it).y == (*(it+1)).y && (*it).x == (*(it+1)).x) {
      points.erase(it);
    }else{
      it++;
    }
  }

  //creem un multimap per trobar tots el punts amb una 'y' determinada
  multimap<int,int> map;
  it = points.begin();

  int y_max=0, y_min=INT_MAX;
  while(it != points.end()){
    map.insert (pair<int,int>((*it).y, (*it).x)); 
    if ((*it).y > y_max) y_max=(*it).y;
    if ((*it).y < y_min) y_min=(*it).y;
    it++;
  }


  vector<Point> * aDevolver = new vector<Point> ();

  //for (int row=0; row< image.rows; row++){
  for (int row=y_min; row<=y_max; row++){
    pair <multimap<int,int>::iterator, multimap<int,int>::iterator> ret;
    ret = map.equal_range(row);
    int numPoints=map.count(row);

    if (numPoints > 1){
      vector<Point> rep; 

      //convertim ret (y iguals a row) a un vector (rep)
      for (multimap<int,int>::iterator it2=ret.first; it2!=ret.second; ++it2){
	rep.push_back(Point(it2->second, it2->first));
      }
      
      //cada altura (y) ordenat per columna (x)
      std::sort(rep.begin(), rep.end(), compare_pointsX);

      //punts consecutius a la mateixa altura -> deixem el del mig
      it = rep.begin();
      int contPoints=0;
      while(contPoints < numPoints){
	int cont =1;
	while (cont <= numPoints && (*(it+cont-1)).x == (*(it+cont)).x-1){
	  cont++;
	  contPoints++;
	}
	//seleccionats.insert (Point((*it).x + (cont)/2,(*it).y));
	aDevolver->push_back(Point((*it).x + (cont)/2,(*it).y));

	if (cont == 1)
	  it++;
	else 
	  it+=(cont);
      
	contPoints++;
      }
    } else if ( numPoints == 1){
      aDevolver->push_back(Point(ret.first->second, row));
    }
  }
  
  return aDevolver;
}

//-----------------------------------------------------------
bool exterior(Point p, const Mat & image, bool isUp){
  float cont_down=0,cont_up=0;
  int y=p.y;
  for (y=p.y; y<image.rows && y<p.y+5; y++)
    cont_down+= image.at<uchar>(y,p.x);

  for (y=p.y; y>0 && y>p.y-5; y--)
    cont_up+= image.at<uchar>(y,p.x);

  
  if (isUp) return cont_up > cont_down;
  else return cont_up < cont_down;


}

//-----------------------------------------------------------
void findMin (vector<Point> points, int window, vector<Point> & selected_min, Mat & image, Mat & img_orig, int MinNumPuntsInWin=5){
 
  
  vector<Point> * purged = purge(points,image);


 vector<Point>  selected_min2;

  for ( int i=0; i<purged->size(); i++ ) {
    bool  min=true, context=false;
    int contPuntsDinsWin=0;
    
    for (int j = 0; j < points.size(); j++) {             
      if (abs(points[j].x - (*purged)[i].x) <= window/2 && abs(points[j].y - (*purged)[i].y) <= window){
  	  context=true;
	  contPuntsDinsWin++;
  	  if (points[j].y > (*purged)[i].y){
  	    min=false;
	    break;
	  }

  	}
      }
    
    if (context && min) // && contPuntsDinsWin>=MinNumPuntsInWin)
      if(exterior((*purged)[i],img_orig, false)){
  	 selected_min.push_back((*purged)[i]);	
      }
  }

  delete(purged); 
  
}

//-----------------------------------------------------------
void usage(char * nomProg){
  cerr << "Usage: "<<nomProg << " options" << endl;
  cerr << "      options:" << endl;
  cerr << "             -i inputfile" << endl;
  cerr << "            [-t minum number of points per contourn (by default 25)]" << endl;
  cerr << "            [-w width of half the window (by default 10)]" << endl;
  cerr << "            [-v verbosity(by default none)]" << endl;
}

//-----------------------------------------------------------
int main(int argc, char* argv[]) {

  string inFileName;
  ofstream minFile;

  int option, window=5;
  unsigned int pTallMin=25; 
  bool verbosity=false;


  while ((option=getopt(argc,argv,"h:i:t:w:v"))!=-1)
    switch (option)  {
    case 'i':
      inFileName = optarg;
      break;
    case 't':
      pTallMin=atoi(optarg);
      break; 
    case 'w':
      window=atoi(optarg);
      break;
    case 'v':
      verbosity=true;
      break;
    case 'h':
    default:
      usage(argv[0]);
      exit(1);
    }

  if (inFileName.size() ==0){
    usage(argv[0]);
    exit(1);
  }
   
  Mat img=imread( inFileName.c_str(), CV_LOAD_IMAGE_GRAYSCALE );
  if (!img.data) {
    cerr << "ERROR reading the image file "<< inFileName<< endl;
    return -1;
  }

  threshold(img, img, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

  Mat kernel = getStructuringElement(  MORPH_ELLIPSE,  Size(1,5),  Point( 0, 0 ) );
  erode(img,img,kernel);

  GaussianBlur( img, img, Size( 5, 5 ),3,3);
  
  Mat img_edge(img.rows, img.cols, CV_8UC1, 1);

  threshold(img, img_edge, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
 

  // obtain contours
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  findContours(img_edge, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
 
  //finding local min points
   vector<Point>  selected_min;

   //  cerr << "contour: Total Contours Detected: "<<  contours.size() << endl;
   for (unsigned int c=0; c < contours.size(); c++){
     if (contours[c].size() > pTallMin && hierarchy[c][3] != -1  ) { //hierarchy[c][3] != -1 -> no parents
       findMin ( contours[c], window,  selected_min, img_edge, img);
     }
   }

  // output files
  string minFileName;
  minFileName=inFileName.substr(0,inFileName.length()-3)+"min";

  minFile.open(minFileName.c_str());

  if (!minFile){
    cerr << "Error creating "<< minFileName << endl;
    return -1;
  }

  // writing head min points file
  minFile << "# Number and class of extrema points" << endl;
  minFile << selected_min.size() << " ";
  minFile << "Min" << endl;
  minFile << "# Points" << endl;

  // writing min points
  sort(selected_min.begin(), selected_min.end(), compare_pointsX);
  for(unsigned int i=0;i<selected_min.size();i++)
    minFile << selected_min[i].x << " " << selected_min[i].y << endl;

  // clossing files
  minFile.close();

  if (verbosity){
    
    cv::Mat image(img.size(),CV_8U,cv::Scalar(255));

    for (unsigned int c=0; c < contours.size(); c++){
      if ( hierarchy[c][3] != -1 ||hierarchy[c][3]==0 ) 
	if (contours[c].size() > pTallMin)
	  for(unsigned int p=0; p<contours[c].size(); p++ ) {
	    if (contours[c][p].y > 1 && contours[c][p].y < image.rows - 2 && contours[c][p].x > 1 && contours[c][p].x < image.cols - 2) {
	      image.at<uchar>(contours[c][p].y, contours[c][p].x)=0;
	      
	    } 
	  }
    }
    string contourFileName=inFileName.substr(0,inFileName.length()-3)+"contornos.jpg";
      
    cv::imwrite(contourFileName, image);

    cv::Mat tmp = img_edge ^ 0xFF;
    string filtradaFileName=inFileName.substr(0,inFileName.length()-3)+"filtrada.jpg";
    imwrite(filtradaFileName,tmp);

    string imgPrepFileName=inFileName.substr(0,inFileName.length()-3)+"prep.jpg";
    imwrite(imgPrepFileName,img);
  }
  
  return 0;
}
