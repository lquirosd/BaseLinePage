#!/bin/bash

if [ $# -lt 2 ]; then
  echo "Usage: ${0##*/} <Path_Dir> <fileList>"

  exit 1
fi

for file in `cat $2` 
do 
   NOM=$1"/"$file
   echo $file; 
   ../../BIN/drawBaselines -i ${NOM}.jpg -x ${NOM}.xml -o ${NOM}_baselines.jpg; 
done

eog $1/*_baselines.jpg
