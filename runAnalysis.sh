#!/bin/bash

PUIN=$1
DATAPATH=/eos/user/g/gauzinge/PUdata/

echo "running BRIL IT Cluster analysis"
eval $(scramv1 runtime -sh) || echo "The command 'cmsenv' failed!"

for filename in ${DATAPATH}*.root; do
    #get the Proc ID
    hasDec=$(echo $PUIN | grep ".")
    PUINint=$(echo $PUIN | cut -d'.' -f 1)
    #echo $PUIN
    if [[ `expr index "$PUIN" \.` -gt 0 ]]; then
        PUINdec=$(echo $PUIN | cut -d'.' -f 2)
    else 
        PUINdec=0
    fi
    #echo `expr index "$PUIN" \.`
    #echo ${PUINint} ${PUINdec}
    TAG=$(echo ${filename} | cut -d'.' -f 2)
    tmp=$(echo ${filename} | cut -d'_' -f 4)
    PU=$(echo ${tmp} | cut -d'.' -f 1)
    dec=$(echo ${tmp} | cut -d'.' -f 2)
    #echo ${PU} ${dec}
    if [[ ${PUINint} -eq ${PU} && ${PUINdec} -eq ${dec} ]]; then
        echo "Processing for PU ${PUIN}. File: ${filename} matches the criteria!"
        cmsRun ITclusterAnalyzer/python/ITclusterAnalyzer_cfg.py print \
            inputFiles=file:${filename} \
            outputFile=summary.root \
            tag=${TAG}
    else
        echo "Processing for PU ${PUIN}. Skipping file: ${filename}."
    fi
done

echo "Now merging output histograms"
command="hadd summary_PU${PUIN}.root"
for rootfile in ${PWD}/*.root; do
    command+=" ${rootfile}"
done
echo $command
${command}
