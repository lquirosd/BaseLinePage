#!/bin/bash

if [ ! $# -eq 4 ]; then
  echo "Use: ${0##*/} <testFileList> <Path_Dir_Output> <NAME_ERT_classifier> <configFile>"
  exit 1
fi

#set -x : Display commands and their arguments as they are executed.
#set -v : Display shell input lines as they are read.
#set -n : Read commands but do not execute them. This may be used to check a shell script for syntax errors.
#set -e stop the script if an error occurs

PATH=$PATH:../../BIN:.
DOUT=${2/\//}
ERT=$3
source $4

# classifying 

   if ! [ -e $NAME_ERT ]
   then
      echo $NAME_ERT " cannot be oppened"
      exit -1
   fi

   mindata=$(mktemp /tmp/mindata.XXXXXX)

   NAME_ERT=`basename $ERT`
   mkdir ${DOUT}_$NAME_ERT
   echo ${DOUT}_$NAME_ERT

   cp -l $DOUT/*.jpg  ${DOUT}_$NAME_ERT
   cp -l $DOUT/*.xml ${DOUT}_$NAME_ERT

  echo "getting points"
  for file in `cat $1`; do
     Nom=`basename $file .$EXT`
     NOM=`echo -e ${DOUT}_$NAME_ERT"/"${Nom}`
     echo $NOM
     [ -f ${NOM}.min ] || \
     imageLocalExtrema -i ${NOM}.jpg -w $MINIMA_WINDOW #-t $MIN_POINTS_CONTOUR

     [ -f ${NOM}.data ] || \
     pointsToClassifierData -i ${NOM}.jpg   -p ${NOM}.min  > ${NOM}.data

     [ -f ${NOM}.data ] && echo ${NOM}.data>> $mindata
  done

   echo "classifying"
   ext_rand_trees -l ${mindata} -m classify -t ${ERT}  -p $MINIM_PROB_TO_TRUE
   status=$?
   if test ${status} -ne 0
   then
      exit -1;
   fi


#joining points, classifying into lines and segmenting
   set +e
   for file in `cat $1`; do
     Nom=`basename $file .$EXT`
     NOM=`echo -e ${DOUT}_$NAME_ERT"/"${Nom}`

     echo "clustering points $file"
     echo -e "\n\n" > ${NOM}.cmin
     #si ha hagut algun problema el .clas no s ha creat
     [ -e ${NOM}.data.clas ] && cat  ${NOM}.data.clas >> ${NOM}.cmin

     paste -d " " ${NOM}.min ${NOM}.cmin > ${NOM}.qmin

      mdbscan -p ${NOM}.qmin  -i ${NOM}.jpg -x ${NOM}.xml -d $DBSCAN_EPSILON_DIST -m $DBSCAN_MIN_POINTS_PER_LINE -a $DBSCAN_MAX_ANGLE_INTO_A_LINE -D $DBSCAN_DECREMENT_STEP -N $DBSCAN_FINAL_POINTS_PER_LINE $DBSCAN_STRAIT_BASELINES

   done

   rm ${DOUT}_$NAME_ERT/*.data  
   rm ${DOUT}_$NAME_ERT/*.cmin 
   rm $mindata
