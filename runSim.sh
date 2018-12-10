#!/bin/bash

# list_include_item "10 11 12" "2"
function list_include_item {
local list="$1"
local item="$2"
if [[ $list =~ (^|[[:space:]])"$item"($|[[:space:]])  ]] ; then
# yes, list include item
  result=0
else
  result=1
fi
return $result
}

PU=$1
NEVENTS=$2

if test -z "$PU" 
then
   echo "No Pileup number specified - please run as source runSim.sh PU"
   return
else
   echo "creating directory \$WORKDIR: "$PWD/mySamplePU_$PU
   WORKDIR=$PWD/mySamplePU_$PU
   mkdir $WORKDIR
   PUSTRING="AVE_"$PU"_BX_25ns"
   echo "Using pileup string "$PUSTRING
fi

if `list_include_item "0 10 20 25 30 35 40 45 50 70 75 100 125 140 150 175 200" $PU` ; then
  echo "PU value $PU available in list"
else 
  echo "ERROR, not a valid PU value! quitting!"
  return
fi

if test -z "$NEVENTS" 
then
   echo "No # of Events specified. Please run as runSim.sh PU NEVENTS if you want to use a number different than default"
   echo "The default value is 10 events"
   NEVENTS=10
   #return
else
   echo "running over $NEVENTS events "
fi
echo "Setting cmssw environment"

#cmsenv
eval `scramv1 runtime -sh`

echo "cd " $WORKDIR
cd $WORKDIR
echo "running Step 1"
cmsDriver.py SingleNuE10_cfi.py --conditions auto:phase2_realistic -n $NEVENTS --era Phase2 --eventcontent FEVTDEBUG --relval 9000,50 -s GEN,SIM --datatier GEN-SIM --beamspot HLLHC --geometry Extended2023D21 --fileout file:step1.root  > $WORKDIR/step1_PU$PU.log  2>&1

echo "running Step 2"
if [[ "$PU" -eq "0" ]]; then
    cmsDriver.py step2  --conditions auto:phase2_realistic -s DIGI:pdigi_valid,L1,L1TrackTrigger,DIGI2RAW,HLT:@fake2 --datatier GEN-SIM-DIGI-RAW -n $NEVENTS --geometry Extended2023D21 --era Phase2 --eventcontent FEVTDEBUGHLT --filein  file:step1.root  --fileout file:step2.root  > $WORKDIR/step2_PU$PU.log  2>&1
else
    cmsDriver.py step2  --conditions auto:phase2_realistic --pileup_input file:/afs/cern.ch/user/g/gauzinge/ITsim/CMSSW_10_4_0_pre2/src/myMinBiasSample/MinBias_14TeV_pythia8_TuneCUETP8M1_cfi_GEN_SIM.root -n $NEVENTS --era Phase2 --eventcontent FEVTDEBUGHLT -s DIGI:pdigi_valid,L1,L1TrackTrigger,DIGI2RAW,HLT:@fake2 --datatier GEN-SIM-DIGI-RAW --pileup $PUSTRING --geometry Extended2023D21 --filein  file:step1.root  --fileout file:step2.root  > $WORKDIR/step2_PU$PU.log  2>&1
fi

echo "running Step 3"
if [[ "$PU" -eq "0" ]]; then
    cmsDriver.py step3  --conditions auto:phase2_realistic -n $NEVENTS --era Phase2 --eventcontent FEVTDEBUGHLT,MINIAODSIM,DQM --runUnscheduled  -s RAW2DIGI,L1Reco,RECO,RECOSIM,PAT,VALIDATION:@phase2Validation+@miniAODValidation,DQM:@phase2+@miniAODDQM --datatier GEN-SIM-RECO,MINIAODSIM,DQMIO --geometry Extended2023D21 --filein  file:step2.root  --fileout file:step3.root  > $WORKDIR/step3_PU$PU.log  2>&1
else
    cmsDriver.py step3  --conditions auto:phase2_realistic --pileup_input file:/afs/cern.ch/user/g/gauzinge/ITsim/CMSSW_10_4_0_pre2/src/myMinBiasSample/MinBias_14TeV_pythia8_TuneCUETP8M1_cfi_GEN_SIM.root -n $NEVENTS --era Phase2 --eventcontent FEVTDEBUGHLT,MINIAODSIM,DQM --runUnscheduled  -s RAW2DIGI,L1Reco,RECO,RECOSIM,PAT,VALIDATION:@phase2Validation+@miniAODValidation,DQM:@phase2+@miniAODDQM --datatier GEN-SIM-RECO,MINIAODSIM,DQMIO --pileup $PUSTRING --geometry Extended2023D21 --filein  file:step2.root  --fileout file:step3.root  > $WORKDIR/step3_PU$PU.log  2>&1
fi

cd ..
echo "Done running the generation"
