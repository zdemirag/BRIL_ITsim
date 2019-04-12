#!/bin/bash

for i in "$@"
do
    case $i in
        -s=*|--source=*)
            SOURCE="${i#*=}"

            ;;
        -d=*|--destination=*)
            DESTINATION="${i#*=}"
            ;;
        *)
            # unknown option
            ;;
    esac
done

echo SOURCE = ${CMSSW_BASE}/${SOURCE}
echo DESTINATION = ${CMSSW_BASE}/${DESTINATION}

#this is the Tracker Common Data
cp --verbose ${CMSSW_BASE}/${SOURCE}/pixbar.xml ${CMSSW_BASE}/${DESTINATION}Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${CMSSW_BASE}/${SOURCE}/pixel.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${CMSSW_BASE}/${SOURCE}/pixelStructureTopology.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${CMSSW_BASE}/${SOURCE}/pixfwd.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${CMSSW_BASE}/${SOURCE}/tracker.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${CMSSW_BASE}/${SOURCE}/trackerStructureTopology.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
#now the Tracker Reco Data
cp --verbose ${CMSSW_BASE}/${SOURCE}/trackerRecoMaterial.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerRecoData/data/PhaseII/TiltedTracker404/
# now the Tracker Sim Data
cp --verbose ${CMSSW_BASE}/${SOURCE}/pixelProdCuts.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerSimData/data/PhaseII/TiltedTracker404/
cp --verbose ${CMSSW_BASE}/${SOURCE}/pixelsens.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerSimData/data/PhaseII/TiltedTracker404/
cp --verbose ${CMSSW_BASE}/${SOURCE}/trackerProdCuts.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerSimData/data/PhaseII/TiltedTracker404/
cp --verbose ${CMSSW_BASE}/${SOURCE}/trackersens.xml ${CMSSW_BASE}/${DESTINATION}/Geometry/TrackerSimData/data/PhaseII/TiltedTracker404/
cp --verbose ${CMSSW_BASE}/${SOURCE}/PixelSkimmedGeometryT6.txt ${CMSSW_BASE}/${DESTINATION}/SLHCUpgradeSimulations/Geometry/data/PhaseII/Tilted/


