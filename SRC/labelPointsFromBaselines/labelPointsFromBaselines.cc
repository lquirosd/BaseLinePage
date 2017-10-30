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

#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>

#include <boost/algorithm/string.hpp>
#include <opencv2/opencv.hpp>
#include <algorithm>

#include <pugixml.hpp>

using namespace cv;
using namespace std;


struct  k{
  bool operator() (cv::Point pt1, cv::Point pt2) { if(pt1.x < pt2.x) return true; else if (pt1.x > pt2.x) return false; else return pt1.x < pt2.y ;}
} sort_points_func;


//----------------------------------------------------------
inline int calc_y(Point startPoint, Point endPoint, int x){


  double slope= double(endPoint.y-startPoint.y)/(endPoint.x-startPoint.x);
  double desp = startPoint.y - slope * startPoint.x;

  int y = slope*x + desp + 0.5;
  return y;
}



//-------------------------------------------------------------------
vector <vector <cv::Point > > getBaselines(pugi::xml_document & page){
  vector< vector <cv::Point > > baselines;
  vector<cv::Point> tmp_baseline;

  int n_reg=0;
  for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region; text_region = text_region.next_sibling("TextRegion")){
    
    int n_lin=0;
    
    for (pugi::xml_node line_region = text_region.child("TextLine"); line_region; line_region = line_region.next_sibling("TextLine")){   
      tmp_baseline.clear();
      std::vector<std::string> string_values;
      string point_string = line_region.child("Baseline").attribute("points").value();
      if(point_string != ""){
	boost::split(string_values,point_string,boost::is_any_of(" "),boost::token_compress_on);
        
	for(unsigned int p = 0; p < string_values.size();p++){
	  std::vector<std::string> string_point;
	  boost::split(string_point,string_values[p],boost::is_any_of(","),boost::token_compress_on);
	  int x;
	  istringstream(string_point[0]) >> x;
	  int y; 
	  istringstream(string_point[1]) >> y;
	  tmp_baseline.push_back(cv::Point(x,y));
	}
	n_lin++;
	baselines.push_back(tmp_baseline);
      }
      
    }
    n_reg++;
  }
  return baselines;
}

//----------------------------------------------------------
vector <cv::Point > readMinimaPoints(ifstream &fitx){
  string line;
  // Read header (ignoring comments)
  vector <cv::Point> points;

  do
    getline(fitx,line);
  while (line[0]=='#' || line.empty());

  istringstream l(line);
  string extr;
  int length=0;
  l >> length >> extr;
 
  if(extr.compare("Min")!=0){
    cerr << "Wrong input format" << endl;
    return points;
  }

  if (length <= 0){
    cerr << "Number of points = 0" << endl;
    return points;
  }
    
  do
    getline(fitx,line);
  while (line[0]=='#' || line.empty());  


  int x,y;
  int i=1;
  
  istringstream(line) >> x >> y; 
  points.push_back(Point(x,y));

  
  do{
    fitx >> x >> y; 
    if (!(fitx.rdstate() & std::ifstream::eofbit))
      points.push_back(Point(x,y));
    i++;
  }while(i<length && !fitx.eof());
  
  return points;
}
//-------------------------------------------------------------------

void  completarLinies(vector< vector<Point> > & baseLines){
  vector<map<int,int> >marges;

  for (int l = 0; l < baseLines.size(); l++) { 
    //ordenem els punts d'esquerre a dreta
    std::sort(baseLines[l].begin(), baseLines[l].end(), sort_points_func);
  }  
  

  for (int l = 0; l < baseLines.size(); l++) { 
    vector<Point> line;

    for (int p = 0; p < baseLines[l].size()-1; p++) {
      for (int x = baseLines[l][p].x+1; x < baseLines[l][p+1].x; x++) {
	int y=calc_y(baseLines[l][p], baseLines[l][p+1], x);
	line.push_back(Point(x,y));
      }
    }

    for (int p = 0; p < line.size(); p++) {
      baseLines[l].push_back(line[p]);
    }

 
    std::sort(baseLines[l].begin(), baseLines[l].end(), sort_points_func);
  }

}
//----------------------------------------------------------
vector<vector<Point> > classificarPunts(vector<vector<cv::Point > > & baseLines, vector< cv::Point >  minimaLocal, int dist_maxima_UP, int dist_maxima_DOWN){
  vector<vector<Point> > classes(baseLines.size()+1);

 
  for (int p = 0; p < minimaLocal.size(); p++) {
    int classDOWN=-1, classUP=-1;
    float minimaDistUP=INT_MAX, minimaDistDOWN=INT_MAX;

    //Linies completades per a tot el rang de x  
    for (int l = 0; l < baseLines.size(); l++) {
      int x=minimaLocal[p].x;
      int pLines=x - baseLines[l][0].x;  //distancia on es troba en la polylinia l
   
      if (x >=  baseLines[l][0].x && x <=  baseLines[l][baseLines[l].size()-1].x){ //esta dins del domini de la x de la polylinia?

	float dist=minimaLocal[p].y-baseLines[l][pLines].y;
	if (dist<=0 && -dist <= minimaDistUP){
	  classUP=l;
	  minimaDistUP=-dist;
	} else  if (dist >= 0 && dist <= minimaDistDOWN){
	  classDOWN=l;
	  minimaDistDOWN=dist;
	  }
      }
    }
   
    if ((classUP < 0 && classDOWN <0) || (minimaDistUP > dist_maxima_UP && minimaDistDOWN > dist_maxima_DOWN))
      classes[0].push_back(Point(minimaLocal[p]));
    else 
      if (minimaDistUP <= dist_maxima_UP && minimaDistDOWN > dist_maxima_DOWN )
	classes[classUP+1].push_back(Point(minimaLocal[p]));
      else
	if (minimaDistUP > dist_maxima_UP && minimaDistDOWN <= dist_maxima_DOWN )
	  classes[classDOWN+1].push_back(Point(minimaLocal[p]));
	else 
	  if (minimaDistUP <= dist_maxima_UP && minimaDistDOWN <= dist_maxima_DOWN )	
	    classes[classUP+1].push_back(Point(minimaLocal[p]));
    
  }
  vector<vector<Point> > classesFinal;
  if (classes[0].size() == 0){
    vector<Point> v(0);
    classesFinal.push_back(v);
  } else
    classesFinal.push_back(classes[0]);

  for (int c = 1; c < classes.size(); c++) {
    if(classes[c].size()>0){
      classesFinal.push_back(classes[c]);
    }
  }
  
  return classesFinal;
}

//----------------------------------------------------------
void usage (char * programName){

  cerr << "Usage: "<<programName << " options " << endl;
  cerr << "      options:" << endl;
  cerr << "             -b xml_baselinesFileName" << endl;
  cerr << "             -m localMinimaPointsFileName" << endl;
  cerr << "            [-u #int max dist. alowed up to the baseline (10 by default)]"<<endl;
  cerr << "            [-d #int max dist. allowd down to the basseline (5 by default)]" << endl;
  //cerr << "             -v verbosity" << endl;
  cerr << "            [-o outputLinesFileName] lines format " << endl;
}
//----------------------------------------------------------
int main(int argc,  char ** argv) {
 string minimaFileName="", baselinesFileName="";
 string imageFileName="", outputLinesFileName="";

  bool verbosity=false;
  int option, dist_max_up=10, dist_max_down=5;

  if(argc == 1){
    usage(argv[0]);
    return -1;
  }

  while ((option=getopt(argc,argv,"b:m:o:d:u:v"))!=-1)
    switch (option)  {
    case 'o':
       outputLinesFileName = optarg;
       break;
    case 'm':
      minimaFileName = optarg;
      break;
    case 'b':
      baselinesFileName = optarg;
      break;
    case 'u':
      dist_max_up=atoi(optarg);
      break;
    case 'd':
      dist_max_down=atoi(optarg);
      break;
    case 'v':
      verbosity=true;
      break;
    default:
      usage(argv[0]);
      return(-1);
    }

  if (minimaFileName.size()==0 || baselinesFileName.size()==0){
    cerr << argv[0] << " Error: baselines in xml file and points to be calssificated names must be provided" << endl;
    usage(argv[0]);
    return (-1);
  }
  
  pugi::xml_document page;
  pugi::xml_parse_result result = page.load_file(baselinesFileName.c_str());

  if (!result){
    cerr << "Error: file " << baselinesFileName << " not found" << endl;
    exit(-1);
  }
  vector <vector <cv::Point> >  baseLines= getBaselines(page);

  ifstream pfd(minimaFileName.c_str());
  if (!pfd){
        cerr << "Error: File \""<<optarg << "\" could not be open "<< endl;
        return -1;
      }



  vector<cv::Point> minimaLocal=readMinimaPoints(pfd);
  completarLinies(baseLines);


  vector<vector<Point> > clases=classificarPunts(baseLines,minimaLocal,dist_max_up, dist_max_down);

  if (outputLinesFileName.size()>=0) {
    ofstream output;
    output.open(outputLinesFileName.c_str());

      output << "# Number of lines and type of points"<< endl;
      output <<clases.size()-1<< "   Num"<< endl;
      
      
      for (int c = 1; c < clases.size(); c++) {  // Noise is labeled as a belonging to 0 class
	output <<"# Number of points"<< endl;
	output << clases[c].size()<< endl;
	output << "# Points"<< endl;

	for (int p = 0; p < clases[c].size(); p++) 
	  output << clases[c][p].x<<" "<<clases[c][p].y  << endl; 
      }
      output.close();
    }


  int numPuntsTotal=0;
  for (int c = 0; c < clases.size(); c++) 
    numPuntsTotal+=clases[c].size();

  cout << "# Number points"<< endl;
  cout << numPuntsTotal << "   Min"<< endl;
  for (int c = 1; c < clases.size(); c++) 
      for (int p = 0; p < clases[c].size(); p++) 
  	cout << clases[c][p].x<<" "<<clases[c][p].y << "  2" << endl; 

  //Noisy points
    for (int p = 0; p < clases[0].size(); p++) 
      cout << clases[0][p].x<<" "<<clases[0][p].y << "  10" << endl; 


}
