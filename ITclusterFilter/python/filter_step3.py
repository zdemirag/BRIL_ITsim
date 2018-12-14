# imports
import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing


# create a new CMS process
process = cms.Process("ITclusterAnalyzer")

# set up the options
options = VarParsing.VarParsing('analysis')
#set up the defaults
options.inputFiles = 'file:/afs/cern.ch/work/g/gauzinge/public/PU100/step3_100.0.root'
# 'file:/afs/cern.ch/work/g/gauzinge/public/condorout/step3_100.1.root',
# options.tag='0'
options.outputFile='slimmed.root'
options.maxEvents = -1 #all events

#get and parse command line arguments
options.parseArguments()
process.load('Configuration.EventContent.EventContent_cff')


# initialize MessageLogger and output report
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.threshold = 'INFO'
process.MessageLogger.categories.append('ITclusterFilter')
process.MessageLogger.cerr.INFO = cms.untracked.PSet(
    limit=cms.untracked.int32(-1)
)

process.options = cms.untracked.PSet(wantSummary=cms.untracked.bool(False))

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))

# the input file
process.source = cms.Source("PoolSource",
                            fileNames=cms.untracked.vstring(options.inputFiles)
                            )


process.myoutput = cms.OutputModule("PoolOutputModule",
                                            splitLevel = cms.untracked.int32(0),
                                            eventAutoFlushCompressedSize = cms.untracked.int32(5242880),
                                            outputCommands = process.RAWSIMEventContent.outputCommands,
                                            fileName = cms.untracked.string(options.outputFile),
                                            dataset = cms.untracked.PSet(
                                                    filterName = cms.untracked.string(''),
                                                    dataTier = cms.untracked.string('GEN-SIM')
                                            )
                                    )


process.myoutput.outputCommands.append('drop  *')
process.myoutput.outputCommands.append('keep  *_*_TrackerHitsPixelEndcapHighTof_*')
process.myoutput.outputCommands.append('keep  *_*_TrackerHitsPixelEndcapLowTof_*')
process.myoutput.outputCommands.append('keep  *_*_TrackerHitsPixelBarrelHighTof_*')
process.myoutput.outputCommands.append('keep  *_*_TrackerHitsPixelBarrelLowTof_*')
process.myoutput.outputCommands.append('keep  *_simSiPixelDigis_Pixel_*')
process.myoutput.outputCommands.append('keep  *_siPixelClusters_*_*')

process.e = cms.EndPath(process.myoutput)
