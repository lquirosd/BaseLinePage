#!/bin/bash

if [ $# -lt 3 ]; then
  echo "Use: ${0##*/} <List_test_files> <Path_Dir_Output> <configFile>"

  exit 1
fi


#set -x : Display commands and their arguments as they are executed.
#set -v : Display shell input lines as they are read.
#set -n : Read commands but do not execute them. This may be used to check a shell script for syntax errors.
#set -e stop the script if an error occurs

PATH=$PATH:../../BIN:.
DOUT=${2/\//}

source $3

# classifying 

for file in `cat $1`; do

  Nom1=`basename $file .$EXT`
  NOM1=`echo -e ${DOUT}"/"${Nom}`
  echo ${DOUT}_$Nom1
    
 [ -e ${DOUT}_$Nom1 ] && continue

  mkdir ${DOUT}_$Nom1
 
  cp -l $DOUT/*.jpg  ${DOUT}_$Nom1
  cp  $DOUT/*.xml ${DOUT}_$Nom1
 
  mindata=$(mktemp /tmp/mindata.XXXXXX)

  for file in `cat $1`; do
    echo "getting points for "$file
    Nom=`basename $file .$EXT`
    NOM=`echo -e ${DOUT}_$Nom1"/"${Nom}`

    [ -f ${NOM}.min ] || \
    imageLocalExtrema -i ${NOM}.jpg -w $MINIMA_WINDOW #-t $MIN_POINTS_CONTOUR

    [ -f ${NOM}.data ] || \
    pointsToClassifierData -i ${NOM}.jpg   -p ${NOM}.min > ${NOM}.data

    [ -f ${NOM}.data ] && echo ${NOM}.data>> ${mindata}
   done
 
   echo "Classifing..."
   ext_rand_trees -l ${mindata} -m classify -t ${DOUT}/${Nom1}.ert  -p $MINIM_PROB_TO_TRUE
   status=$?
   if test ${status} -ne 0
   then
      exit -1;
   fi

   rm ${mindata}


#joining points, classifying into lines and segmenting
   set +e
   for file in `cat $1`; do
     Nom=`basename $file .$EXT`
     NOM=`echo -e ${DOUT}_$Nom1"/"${Nom}`

     echo "clustering points $file"
     echo -e "\n\n" > ${NOM}.cmin
     #si ha hagut algun problema el .clas no s ha creat
     [ -e ${NOM}.data.clas ] && cat  ${NOM}.data.clas >> ${NOM}.cmin

     paste -d " " ${NOM}.min ${NOM}.cmin > ${NOM}.qmin

     mdbscan -p ${NOM}.qmin  -i ${NOM}.jpg -x ${NOM}.xml -d $DBSCAN_EPSILON_DIST -m $DBSCAN_MIN_POINTS_PER_LINE -a $DBSCAN_MAX_ANGLE_INTO_A_LINE -D $DBSCAN_DECREMENT_STEP -N $DBSCAN_FINAL_POINTS_PER_LINE $DBSCAN_STRAIT_BASELINES

   done
   rm ${DOUT}_$Nom1/*.data
   rm ${DOUT}_$Nom1/*.data.clas
   rm ${DOUT}_$Nom1/*.qmin
   rm ${DOUT}_$Nom1/*.cmin  
   rm ${DOUT}_$Nom1/*.min
 done
