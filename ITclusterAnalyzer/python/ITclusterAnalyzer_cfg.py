# imports
import FWCore.ParameterSet.Config as cms

# create a new CMS process
process = cms.Process("ITclusterAnalyzer")

# Import all the necessary files
# process.load('Configuration.StandardSequences.Services_cff')
# process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.Geometry.GeometryExtended2023D21Reco_cff')
# process.load('Configuration.StandardSequences.MagneticField_38T_cff')
# process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# from Configuration.AlCa.GlobalTag import GlobalTag
# process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic', '')


# initialize MessageLogger and output report
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.threshold = 'INFO'
process.MessageLogger.categories.append('ITclusterAnalyzer')
process.MessageLogger.cerr.INFO = cms.untracked.PSet(
    limit=cms.untracked.int32(-1)
)

process.options = cms.untracked.PSet(wantSummary=cms.untracked.bool(True))

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(-1))

# thge input file
process.source = cms.Source("PoolSource",
                            # replace 'myfile.root' with the source file you want to use
                            fileNames=cms.untracked.vstring(

                                'file:/afs/cern.ch/user/g/gauzinge/ITsim/myPU_35sample/step3.root'
                            )
                            )

# the config of my analyzer
process.clusterAnalysis = cms.EDAnalyzer('ITclusterAnalyzer',
                                         clusters=cms.InputTag(
                                             "siPixelClusters"),
                                         maxBin=cms.untracked.uint32(1000)
                                         )

# the TFIleService that produces the output root files
process.TFileService = cms.Service("TFileService",
                                   fileName=cms.string('histos.root')
                                   )


process.p = cms.Path(process.clusterAnalysis)
