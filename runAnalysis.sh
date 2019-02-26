#!/bin/bash

PUIN=$1
DATAPATH=/eos/user/g/gauzinge/PUdata/

eval $(scramv1 runtime -sh) || echo "The command 'cmsenv' failed!"

echo '####################################################################'
echo "running BRIL IT Cluster analysis"
echo 'Picking up data files with PU ' ${PUIN} ' from directory: ' ${DATAPATH}
echo '####################################################################'
echo 'Processing files:'
for filename in ${DATAPATH}*.root; do
    #get the Proc ID
    TAG=$(echo ${filename} | cut -d'.' -f 3)
    tmp=$(echo ${filename} | cut -d'_' -f 4)
    MAIN=$(echo ${tmp} | cut -d'.' -f 1)
    DECIMAL=$(echo ${filename} | cut -d'.' -f 2)
    PU=${MAIN}.${DECIMAL}
    #if [[ "$PUIN" -eq "$PU" ]]; then
    if [ "$PUIN" = "$PU" ]; then
    echo ${filename} 'with PU ' ${PU}
    #if (( $(echo "$PUIN = $PU" | bc -l) )); then
        cmsRun ITclusterAnalyzer/python/ITclusterAnalyzer_cfg.py print \
            inputFiles=file:${filename} \
            outputFile=temp.root \
            tag=${TAG}
    #else
        #echo "Processing for PU ${PUIN} requested but this file: ${filename} has PU ${PU} in the name!"
    fi
done

echo "Now merging output histograms"
command="hadd summary_PU_${PUIN}.root"
for rootfile in ${PWD}/temp_?.root; do
    command+=" ${rootfile}"
done
echo $command
${command}
