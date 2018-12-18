#!/bin/bash

PUIN=$1
DATAPATH=/eos/user/g/gauzinge/PUdata/

echo "running BRIL IT Cluster analysis"
eval $(scramv1 runtime -sh) || echo "The command 'cmsenv' failed!"

for filename in ${DATAPATH}*.root; do
    #get the Proc ID
    TAG=$(echo ${filename} | cut -d'.' -f 2)
    tmp=$(echo ${filename} | cut -d'_' -f 4)
    PU=$(echo ${tmp} | cut -d'.' -f 1)
    echo ${TAG} ${PU}
    if [[ "$PUIN" -eq ${PU} ]]; then
        cmsRun ITclusterAnalyzer/python/ITclusterAnalyzer_cfg.py print \
            inputFiles=file:${filename} \
            outputFile=summary.root \
            tag=${TAG}
    else
        echo "Processing for PU ${PUIN} requested but this file: ${filename} has PU ${PU} in the name!"
    fi
done

echo "Now merging output histograms"
command="hadd summary_PU${PU}.root"
for rootfile in ${PWD}/*.root; do
    command+=" ${rootfile}"
done
echo $command
#${command}
