#!/bin/bash


if [ $# -lt 3 ]; then
  echo "Use: ${0##*/} <List_test_files> <Path_Dir_Output> <configFile>"
  exit 1
fi

set -e

PATH=$PATH:../../BIN:.
DOUT=${2/\//}

source $3

for file in `cat $1`; do 

  EXT=`head -1 $1 |awk -F "." '{print $NF}'`
  #N=`basename $file .$EXT`
  #N=`basename $file .tif`
  N=`basename $file`
  NOM=`echo -e ${DOUT}"/"${N}`

  echo "Processing " $NOM

  imageLocalExtrema -i ${NOM}.jpg -w $MINIMA_WINDOW #-t $MIN_POINTS_CONTOUR


  labelPointsFromBaselines -b ${NOM}.xml -m ${NOM}.min -u $CLASS_XML_UPPER_BOUND -d $CLASS_XML_DOWN_BOUND > ${NOM}.qmin


  pointsToClassifierData -i ${NOM}.jpg   -p ${NOM}.qmin  > ${NOM}.data

  ext_rand_trees -i ${NOM}.data -m train -t ${NOM}.ert

done
#rm ${DOUT}/*.data ${DOUT}/*.qmin  ${DOUT}/*.min
