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


#include <stdlib.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include "slopeClass.h"

// C++: void invertAffineTransform(InputArray M, OutputArray iM)

slopeClass::slopeClass(Mat & image) { 
    this->img=image.clone();   
    threshold = cv::threshold(this->img, this->img, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
}

slopeClass::slopeClass(Mat & image, int minX, int minY, int maxX, int maxY) {
  Rect roi(minX, minY, maxX-minX, maxY-minY);
  cv::Mat(image,roi).copyTo(this->img);

  threshold = cv::threshold(this->img, this->img, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
}



void slopeClass::rotar(double angle) {
  //int Extrcols=(sqrt(img.cols*img.cols+img.rows*img.rows)-img.cols);
  //  int Extrrows=img.rows*(sqrt(2)-1); // Para el caso mas desfavorable de 45º
  //int Extrrows=img.rows*sin(angle);
  // int Extrcols=0;
  // int Extrrows=0;
  //Mat rot_mat = getRotationMatrix2D(Point2f((img.cols+Extrcols)/2.0, (img.rows+Extrrows)/2.0), angle, 1.0);
  Mat rot_mat = getRotationMatrix2D(Point2f(img.cols/2.0, img.rows/2.0), angle, 1.0);
  rotada.create(img.rows,img.cols, CV_8UC(255));
  //warpAffine(img, rotada, rot_mat, Size(img.cols,img.rows));
  //warpAffine(img,rotada, rot_mat, img.size(), cv::INTER_CUBIC, BORDER_TRANSPARENT);//, 255);
   warpAffine(img, rotada, rot_mat,img.size(), cv::INTER_CUBIC, cv::BORDER_CONSTANT, Scalar(255,255,255));

}


inline float slopeClass::var_HProjection(int *HPr,int dim_proj) {
  int j;
  float sum=0, sum2=0, var, mean;
  int *H_Proj=HPr;
  
  for (j=0; j<dim_proj; HPr++,j++){
    sum += (float)(*HPr);
    sum2 += (float)(*HPr)*(*HPr);
  }
  mean=sum/dim_proj;
  var = sum2/dim_proj - mean*mean;
 
  return sqrt(var);
} 


int ** slopeClass::get_projections(){
  int row, col, dim_prj;
  int a, i, desp;
  int ** HProj;
  
  dim_prj = (int)(2*sqrt(img.cols*img.cols+img.rows*img.rows/4));

  /* Pedimos memoria para las proyecciones y las inicializamos */
  HProj = (int **)malloc(sizeof(int*)*(2*ANG_RANGE+1));
  for (i=0; i<=2*ANG_RANGE; i++) {
    HProj[i] = (int *)malloc(sizeof(int)*dim_prj);
    for (row=0; row<dim_prj; row++) HProj[i][row]=0; // Inicialización
  }

  
  /* Calculamos las proyecciones */
  for (col=0; col<img.cols; col++)
    for (row=0; row<img.rows; row++) 
      if (img.at<uchar>(row,col) <= threshold)  /* lo tenemos en cuenta */
	for (a=0; a<=2*ANG_RANGE; a++) {      /* si es suficientemente negro */
	  desp = (int)((img.rows/2.0-row)*cos((float)(a-ANG_RANGE)*M_PI/180.0) + \
	    col*sin((float)(a-ANG_RANGE)*M_PI/180.0));
	  if ((dim_prj/2+desp>=0) && (dim_prj/2+desp<dim_prj))
	    HProj[a][dim_prj/2+desp]++;
	}
  
  return HProj;
}

void slopeClass::MVPV() {
  float var_max, var[2*ANG_RANGE+1], sum, cont;
  int a, a_max;
  int ** HProj;
  int  dim_prj;
  

  dim_prj = 2*sqrt(img.cols*img.cols+img.rows*img.rows/4);

  // Adquiero las proyecciones entre -45º y 45º
  HProj = get_projections();

  /* Calculamos las varianzas de las proyecciones*/
  /* y nos quedamos con el angulo que da el maximo */
  var_max = var_HProjection(HProj[0],dim_prj);
  var[0] = var_max;
  a_max = -ANG_RANGE; 
  for (a=-ANG_RANGE+1; a<=ANG_RANGE; a++) {
    var[a+ANG_RANGE] = var_HProjection(HProj[a+ANG_RANGE],dim_prj);
    if (var_max < var[a+ANG_RANGE]) {
      var_max=var[a+ANG_RANGE];
      a_max=a;
    }
  }

  //cout << "Angle maxim " << a_max << endl;
  /* calculamos la media ponderada de todas las varianzas (centro de masas)
     que esten por debajo de un procentaje (threshold) del maximo */
  sum=0;
  cont=0;
  for (a=-ANG_RANGE; a<=ANG_RANGE; a++){
    if (var[a+ANG_RANGE] >= var_max) {
      sum += var[a+ANG_RANGE] * a;
      cont += var[a+ANG_RANGE];      
    }
  }  

    // cerr <<"#angle variance"<< endl;
    // cerr <<"#----------------"<<endl;
    // for (a=-ANG_RANGE; a<=ANG_RANGE; a++)
    //   cerr << a << "  " << var[a+ANG_RANGE] << endl;  


  /* Liberamos la memoria de las proyectiones */
  for (a=0; a<=2*ANG_RANGE; a++) free(HProj[a]);
  free(HProj);
  
  // Devuelve el valor de SLOPE del segmento en grados
  //cout << "sum = "<< sum << " cont = " << cont << endl;
  slopeAngle=(-sum/(1.0*cont));
  if (isnan(slopeAngle) || ! isfinite(slopeAngle) || slopeAngle == ANG_RANGE ||  slopeAngle == -ANG_RANGE){
    //cout << "DINS"<< endl;
    slopeAngle=0;
  }
}

void slopeClass::deslope(){

  /*****************************************
   * Calculo del angulo por maxima varianza 
   * de la proyección vertical
   *****************************************/
  MVPV();
 
}
