[ -d Trees ] && rm -rf Trees
mkdir Trees
cp -l Corpus/*.jpg GroundTruth/*.xml Trees 
../../SCRIPTS/trainAllTreesOnPage.sh corpus.lst Trees/ ICDAR.cnf
cp -l Trees/*.ert Corpus/
rm -rf Trees

