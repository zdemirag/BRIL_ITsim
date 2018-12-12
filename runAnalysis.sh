#!/bin/bash

DATAPATH=/afs/cern.ch/work/g/gauzinge/public/condorout/

echo "running BRIL IT Cluster analysis"
eval $(scramv1 runtime -sh) || echo "The command 'cmsenv' failed!"

for filename in ${DATAPATH}*.root; do
    #get the Proc ID
    TAG=$(echo ${filename} | cut -d'.' -f 3)
    tmp=$(echo ${filename} | cut -d'_' -f 2)
    PU=$(echo ${tmp} | cut -d'.' -f 1)
    echo ${TAG} ${PU}
#cmsRun ITclusterAnalyzer/python/ITclusterAnalyzer_cfg.py print \
    #inputFiles=file:${filename} \
    #outputFile=summary.root \
    #tag=${TAG}
done

echo "Now merging output histograms"
command="hadd summary_PU${PU}.root"
for rootfile in ${PWD}/*.root; do
    command+=" ${rootfile}"
done
echo $command
${command}
