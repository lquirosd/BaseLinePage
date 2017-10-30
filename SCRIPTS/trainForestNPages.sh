#!/bin/bash


if [ ! $# -eq 4 ]; then
  echo "Use: ${0##*/} <List_test_files> <Path_Dir_Output> numPagesToTrain <configFile>"
  exit 1
fi

set -e

PATH=$PATH:../../BIN:.

DOUT=${2/\\/}

NUM=$3

source $4

head -$NUM $1 > /tmp/train$NUM.lst

[ -e train_$NUM.data ] && rm train_$NUM.data

for file in `cat /tmp/train$NUM.lst`; do 

  N=`basename $file .qmin`
  NOM=`echo -e ${DOUT}"/"${N}`

  echo "Processing " $NOM
  #### test
  #convert ${file} $NOM.clean.png

#  [ -e ${NOM}.min ] || \ 
  imageLocalExtrema -i ${NOM}.jpg -w $MINIMA_WINDOW #-t $MIN_POINTS_CONTOUR

 # [ -e ${NOM}.qmin ] || \
  labelPointsFromBaselines -b ${NOM}.xml -m ${NOM}.min -u $CLASS_XML_UPPER_BOUND -d $CLASS_XML_DOWN_BOUND > ${NOM}.qmin

#  [ -e ${NOM}.data ] || \
  pointsToClassifierData -i ${NOM}.jpg   -p ${NOM}.qmin  > ${NOM}.data

  cat ${NOM}.data  >> train_$NUM.data
done


ext_rand_trees -i train_$NUM.data -m train -t $DOUT/icdar_$NUM.ert
rm train_$NUM.data
