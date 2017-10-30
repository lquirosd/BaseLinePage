/*
 *   Copyright 2017, Moisés Pastor i Gadea and Jorge Martínez
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
 *      Author: Moisés Pastor i Gadea and Jorge Martínez
 */


#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <opencv/ml.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#define MODE_TRAIN 0
#define MODE_CLASSIF 1

using namespace cv;

using namespace std;

void usage(char * nomProg){
    std::cerr << "Usage: " << nomProg << " [options]" << std::endl;
    std::cerr << "    options:" << std::endl;
    std::cerr << "       -i input filename (REQUIRED, data for training or classifying)" << std::endl;
    std::cerr << "       -l imput file list (REQUIRED, data for training or classifying)" << std::endl;
    std::cerr << "       -m mode (\"train\" or \"classify\", default \"classify\")" << std::endl;
    std::cerr << "       -t tree filename (REQUIRED, in training mode, the file to save the tree, in test/classification, the tree to use)" << std::endl;
    std::cerr << "       -n number of samples required to split a node (used in training, default 2)" << std::endl;
    std::cerr << "       -k number of random splits at each node (used in training, default sqrt(number_of_samples))" << std::endl;
    std::cerr << "       -s number of trees (used in training, default 100)" << std::endl;
}
void displayTrainingParameters(string &inFile, string &treeFile, int max_depth, int nmin, int K, int M){
    std::cout << "Training ERT with data from " << inFile << ", with parameters:" << std::endl;
    std::cout << "   nmin = " << nmin << std::endl;
    std::cout << "   K = " << K << std::endl;
    std::cout << "   M = " << M << std::endl;
    std::cout << "Output trained ERT to " << treeFile << std::endl;
}

void trainERT(string &inFile, string &treeFile, int nmin, int K, int M){
    CvMLData dataManager;
    // Read the training data
    int ret_val = dataManager.read_csv(inFile.c_str());
    if(ret_val == -1){
        std::cerr << "Error reading the input data " << std::endl;
        return;
    }
    dataManager.change_var_type(0, CV_VAR_CATEGORICAL); // The first column has the labels
    dataManager.set_response_idx(0); 

    int max_depth = 1000;
    int max_categories = 3;
    bool calc_var_importance = false;
    float forest_accuracy = 0; // Not used
    int term_criteria = CV_TERMCRIT_ITER;
    CvRTParams* parameters = new CvRTParams(max_depth, nmin, 0.0, false, max_categories, 0, calc_var_importance, 
                                            K, M, forest_accuracy, term_criteria); // For setting the ERT parameters
    CvERTrees ert;
    displayTrainingParameters(inFile, treeFile, max_depth, nmin, K, M);
    ert.train(&dataManager,*parameters);
    ert.save(treeFile.c_str()); // Save the trees
    delete parameters;
}


void classifyUsingERT( std::vector<string> & files, string &treeFile, float minProb){
  CvERTrees ert;
  ert.load(treeFile.c_str()); 
  
  for(int i=0;i<files.size();i++){
    CvMLData dataManager;
    int ret_val = dataManager.read_csv(files[i].c_str()); // Read the data
	    
    if(ret_val == -1){
      std::cerr << "Error reading the input data for file " << files[i] << std::endl;
    }else{
      Mat* data = new Mat(dataManager.get_values());
      int ndata = data->rows;
	      
      string outname = files[i];
      outname.append(".clas");
      std::ofstream out;
      out.open(outname.c_str());

      for(int j=0;j<ndata;j++){  //CvForestTree* get_tree(int i) const;
	float clas = ert.predict(data->row(j));

	
	int cont=0;
	int ntrees = ert.get_tree_count();
	for (int t=0; t < ntrees; t++){
	  CvForestTree* tree = ert.get_tree(t);
	  CvDTreeNode* node= tree->predict( data->row(j));
	  if (node->value == clas)
	    cont ++;
	}
	if (cont/(float)ntrees >=minProb)
	  out << clas << std::endl;
	else 
	  out << "10" << std::endl;
      }
      out.close();
      delete data;
    }    
  }
}


int main(int argc, char ** argv)
{
    int option;
    string inFile="";
    string listInFile="";
    string treeFile="";
    int mode=2; // Default
    
    // ERT parameters for training
    int nmin = 2;
    int K = 0; // Default value of sqrt N
    int M = 100;
    float minProb=0;

    string m; // Auxiliar
    if(argc<3){
        usage(argv[0]);
        return -1;
    }
    while ((option=getopt(argc,argv,"hi:l:m:t:n:k:s:p:"))!=-1)
        switch (option)  {
            case 'i': // Input data for training, testing or classification
                inFile=optarg;
                break;
	     case 'l':
                listInFile=optarg;
                break;
	     case 'm': // Select mode
                m=optarg;
                if(m=="train")
                    mode = MODE_TRAIN;
                else if(m=="classify")
                    mode = MODE_CLASSIF;
                else{
                    std::cerr << "Wrong mode. Possible choices are \"train\" or \"classify\"" << std::endl;
                    return -1;
                }            
                break;
            case 't': // tree file name (for saving in training, or the file that contains the trained tree for test/classification
                treeFile=optarg;
                break;
            case 'n': // Number of samples required to split a node
                nmin = atoi(optarg);
                break;
            case 'k': // Number of random splits at each node
                K = atoi(optarg);
                break;
            case 's': // Number of trees (size of the forest)
                M = atoi(optarg);
                break;
	    case 'p':
	        minProb=atof(optarg);
	        break;
            case 'h':
            default:
                usage(argv[0]);
                return 1;
        }

    if(inFile=="" && listInFile==""){
        std::cerr << "Error: no input data filename" << std::endl;
        return -1;
    }

    if(treeFile==""){
        std::cerr << "Error: no tree filename" << std::endl;
        return -1;
    }
    
    switch(mode){
        case MODE_TRAIN:
            trainERT(inFile, treeFile, nmin, K, M);
            break;
        case MODE_CLASSIF:

	  if (! std::ifstream(treeFile.c_str())){
	    cerr << "ERROR: TreeFile "<< treeFile << " cannot be oppened" << std::endl;
	    return -1;
	  }

	  std::vector<string> files;
	  
	  if(inFile.size()!=0)
	    files.push_back(inFile);
	  else{
	    std::ifstream in;
	    in.open(listInFile.c_str());
	    if (!in){
	      cerr << "ERROR: File " << listInFile.c_str() << " cannot be oppened" << endl;
	      exit(-1);
	    }
	    while(in.good()){
	      string tmp;
	      in >> tmp;
	      if(tmp!="")
                files.push_back(tmp);
	    }
	    in.close();
	  }
	  classifyUsingERT(files, treeFile, minProb);
	  break;
    } 
    return 0;
}
