# baseLinePage
Toolkit to locate the baselines in handwritten text pages
* A toolkit to find the text baselines in pages.

* BaseLinPages is written in C++. 

* BaseLinPages uses [OpenV](https://opencv.org) and [Bash shell](https://es.wikipedia.org/wiki/Bash)

* Some features:
        * Flexible.
        * Easy and fast to set up. Roughly, around 15 minutes to anotate a page, 3 minutes to train an [Extremely Randomized Trees Classifier](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.65.7485&rep=rep1&type=pdf) and 3 seconds per page to get the baselines
        * Mainly for academic use. 


## Usage
#### To train an ert classifier with more than one page
* trainForestNPages.sh corpus.lst CorpusDir NumberOfPages [ConfigFile.cnf](https://github.com/RXXXXParedesPalacios/Layers/tree/master/Tutorial)
  * Change *NumberOfPages* to the apropiate value (#integer). It takes the *NumberOfPages* first pages from the corpus.lst

#### To get the baselines: 
* test.sh corpus.lst CorpusDir PathToErtClassifier ConfigFile 
    * Change *NumberOfPages* to the apropiate value (#integer)
#### To get the scores
*  measures.sh corpus.lst HypotesisDir GroundTruthDir
    * This tool is based on the java program used in [ICDAR 2017 Competition on Baseline Detection (cBAD)](https://github.com/Transkribus/TranskribusBaseLineEvaluationScheme)
    * In this [paper](https://arxiv.org/pdf/1705.03311.pdf) the method used is explained.
  

#### ConfigFile.cnf: a set of environtment variables in a plain text file
>  #local minima
* MINIMA_WINDOW=15
* MIN_POINTS_CONTOUR=35 #by default
>  #vars for the labelPointsFromBaselines
* CLASS_XML_DOWN_BOUND=8
* CLASS_XML_UPPER_BOUND=10
> #vars for the ERT classifier
* MINIM_PROB_TO_TRUE=0 
* K=0   #by default
> #vars for the mdbscan
* DBSCAN_MIN_DENSITY=2 # by default
* DBSCAN_EPSILON_DIST=400    #0  for automaticaly estimated
* DBSCAN_MIN_POINTS_PER_LINE=2
* DBSCAN_MAX_ANGLE_INTO_A_LINE=60
* DBSCAN_DECREMENT_STEP=2 #by default
* DBSCAN_STRAIT_BASELINES= #-d to activate
* DBSCAN_FINAL_POINTS_PER_LINE=10 # final baseline points

## Authors

* BaseLinPages is written by [**Moisés Pastor**](http://users.dsic.upv.es/~mpastorg/)

## Contributors

## Install and compiling
* cd SRC
* make
