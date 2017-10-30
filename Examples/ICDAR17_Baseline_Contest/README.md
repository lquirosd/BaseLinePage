## To perform the test "taking one in":

1.- ./trainAll.sh       ## it will train a ert classifier per page
2.- ./getBaselines.sh   ## it will obtain the baselines
3.- ././drawResults.sh  DIR corpus.lst        ## Change dir to be the directory with baselines
4.- ./measures.sh corpus.lst DIR GroundTruth  ## Change dir to be the directory with results

############################################

## To train an ert classifier with more than one page
../../SCRIPTS/trainForestNPages.sh corpus.lst Corpus NumberOfPages ICDAR.cnf ## Change NumberOfPages to the apropiate value (#integer). It takes the NumberOfPages first pages from the corpus.lst

## To test an ert classifier with more than one page
../../SCRIPTS/test.sh corpus.lst Corpus Corpus/icdar_NumberOfPages.tree ICDAR.cnf ## change NumberOfPages to the apropiate value (#integer)
