#!/bin/bash

if [ $# -lt 3 ]; then
  echo "Use: ${0##*/}  <corpus.lst> <Path_Dir_Output> <Path_Dir_Ground>"
  exit 1
fi


CORPUS_LST=$1
HYP_DIR=$2
GRD_DIR=$3

#vore si estan totes les dades

hyp_lst=$(mktemp /tmp/hyp_XXXXXX)
hyp2_lst=$(mktemp /tmp/hyp2_XXXXXX)

grd_lst=$(mktemp /tmp/grd_XXXXXX)
grd2_lst=$(mktemp /tmp/grd2_XXXXXX)


PAGE=`basename $HYP_DIR|sed "s/^Arbres_//"`



#####################################################
for file in `cat ${CORPUS_LST}`; do
   echo ${GRD_DIR}/${file}.xml >> ${grd_lst}
   echo ${HYP_DIR}/${file}.xml >> ${hyp_lst}
done

#grep -v ${PAGE}.lines ${grd_lst} > ${grd2_lst}
#grep -v ${PAGE}.lines.txt ${hyp_lst} > ${hyp2_lst}

#####################################################

mv ${hyp_lst} ${hyp2_lst}".lst"
mv ${grd_lst} ${grd2_lst}".lst"

java -jar ../../BIN/TranskribusBaseLineEvaluationScheme-0.1.3-jar-with-dependencies.jar ${grd2_lst}".lst" ${hyp2_lst}".lst" 2> /dev/null

rm ${hyp2_lst}".lst"  ${grd2_lst}".lst"
