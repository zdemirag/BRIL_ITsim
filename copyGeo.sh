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
echo SOURCE = ${SOURCE}
echo DESTINATION = ${DESTINATION}


#this is the Tracker Common Data
cp --verbose ${SOURCE}/pixbar.xml ${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${SOURCE}/pixel.xml ${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${SOURCE}/pixelStructureTopology.xml ${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${SOURCE}/pixfwd.xml ${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${SOURCE}/tracker.xml ${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
cp --verbose ${SOURCE}/trackerStructureTopology.xml ${DESTINATION}/Geometry/TrackerCommonData/data/PhaseII/TiltedTracker404/
#now the Tracker Reco Data
cp --verbose ${SOURCE}/trackerRecoMaterial.xml ${DESTINATION}/Geometry/TrackerRecoData/data/PhaseII/TiltedTracker404/
# now the Tracker Sim Data
cp --verbose ${SOURCE}/pixelProdCuts.xml ${DESTINATION}/Geometry/TrackerSimData/data/PhaseII/TiltedTracker404/
cp --verbose ${SOURCE}/pixelsens.xml ${DESTINATION}/Geometry/TrackerSimData/data/PhaseII/TiltedTracker404/
cp --verbose ${SOURCE}/trackerProdCuts.xml ${DESTINATION}/Geometry/TrackerSimData/data/PhaseII/TiltedTracker404/
cp --verbose ${SOURCE}/trackersens.xml ${DESTINATION}/Geometry/TrackerSimData/data/PhaseII/TiltedTracker404/
cp --verbose ${SOURCE}/PixelSkimmedGeometryT6.txt ${DESTINATION}/SLHCUpgradeSimulations/Geometry/data/PhaseII/Tilted/


