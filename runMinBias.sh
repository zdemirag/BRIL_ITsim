#!/bin/bash

NEVENTS=$1
EVENTCONTENT=FEVTDEBUG
#replacinge pdigi_valid with pdigi

##### change me to your needs #####
NTHREADS=10
PUFILE=/afs/cern.ch/work/g/gauzinge/public/minBiasFile300k_bak.root
##################################

if test -z "$NEVENTS" 
then
   echo "No # of Events specified. Please run as runSim.sh PU NEVENTS if you want to use a number different than default"
   echo "The default value is 10 events"
   NEVENTS=10
   #return
else
   echo "running over $NEVENTS events "
fi

#########################
#Setup CMSSW framework
#########################
#Extract sandbox
tar -xf sandbox.tar.bz2
#Keep track of release sandbox version
basedir=$PWD
rel=$(echo CMSSW_*)
arch=$(ls $rel/.SCRAM/|grep slc) || echo "Failed to determine SL release!"
old_release_top=$(awk -F= '/RELEASETOP/ {print $2}' $rel/.SCRAM/slc*/Environment) || echo "Failed to determine old releasetop!"
 
# Creating new release
# This isdone so e.g CMSSW_BASE and other variables are not hardcoded to the sandbox setting paths
# which will not exist here
  
echo ">>> creating new release $rel"
mkdir tmp
cd tmp
scramv1 project -f CMSSW $rel
new_release_top=$(awk -F= '/RELEASETOP/ {print $2}' $rel/.SCRAM/slc*/Environment)
cd $rel
echo ">>> preparing sandbox release $rel"
  
#for i in bin cfipython config lib module python src; do
for i in bin cfipython config lib python src; do
    rm -rf "$i"
    mv "$basedir/$rel/$i" .
done
     
echo ">>> fixing python paths"
for f in $(find -iname __init__.py); do
   sed -i -e "s@$old_release_top@$new_release_top@" "$f"
done
         
eval $(scramv1 runtime -sh) || echo "The command 'cmsenv' failed!"
cd "$basedir"
echo "[$(date '+%F %T')] wrapper ready"

#########################
#Run Generation
#########################
#now the actual commands for the generation
echo "Generating MinBias Sampel with "${NEVENTS}" Events"

cmsDriver.py MinBias_14TeV_pythia8_TuneCUETP8M1_cfi  --conditions auto:phase2_realistic -n ${NEVENTS} --era Phase2 --eventcontent ${EVENTCONTENT} --relval 90000,100 -s GEN,SIM --datatier GEN-SIM --beamspot HLLHC14TeV --geometry Extended2023D21 --nThreads ${NTHREADS} --fileout file:${PUFILE}

echo "Done running the generation"
echo "Cleaning up behing me"
rm -rf tmp
rm -rf CMSSW_10_4_0_pre2
