// -*- C++ -*-
//
// Package:    BRIL_ITsim/ITclusterAnalyzer
// Class:      ITclusterAnalyzer
//
/**\class ITclusterAnalyzer ITclusterAnalyzer.cc BRIL_ITsim/ITclusterAnalyzer/plugins/ITclusterAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Georg Auzinger
//         Created:  Wed, 05 Dec 2018 15:10:16 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"

#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"

#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/Common/interface/Handle.h"
//#include "DataFormats/Phase2TrackerCluster/interface/Phase2TrackerCluster1D.h"
//#include "DataFormats/Phase2TrackerDigi/interface/Phase2TrackerDigi.h"
#include "DataFormats/SiPixelDigi/interface/PixelDigi.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CommonTools/Utils/interface/TFileDirectory.h"

#include <TH2F.h>

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.



class ITclusterAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit ITclusterAnalyzer(const edm::ParameterSet&);
      ~ITclusterAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------
      edm::EDGetTokenT<edmNew::DetSetVector<SiPixelCluster>> m_tokenClusters;
      uint32_t m_maxBin;

      //array of TH2F
      TH2F* m_diskHistos[8];
      TH2F* m_trackerLayout;
      TH2F* m_trackerLayoutXY;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
ITclusterAnalyzer::ITclusterAnalyzer(const edm::ParameterSet& iConfig)
 :
  //m_tokenClusters(consumes<edmNew::DetSetVector<SiPixelCluster>> ("clusters"))
  m_tokenClusters(consumes<edmNew::DetSetVector<SiPixelCluster>> (iConfig.getParameter<edm::InputTag>("clusters"))),
  m_maxBin(iConfig.getUntrackedParameter<uint32_t>("maxBin",1000))

{
   //now do what ever initialization is needed


}


ITclusterAnalyzer::~ITclusterAnalyzer()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called once each job just before starting event loop  ------------
void
ITclusterAnalyzer::beginJob()
{
    edm::Service<TFileService> fs; 
    fs->file().cd("/");
    TFileDirectory td = fs->mkdir("PerDisk");

    //now lets create the histograms
    for(unsigned int i=0; i <8; i++)
    {
        int disk = (i<4)? i-4 : i-3;
        std::stringstream histoname;
        histoname << "Number of clusters for Disk " << disk <<";Ring;# of Clusters per event";
        std::stringstream histotitle;
        histotitle << "Number of clusters for Disk " << disk;
        //name, name, nbinX, Xlow, Xhigh, nbinY, Ylow, Yhigh
        m_diskHistos[i] = td.make<TH2F>(histotitle.str().c_str(),histoname.str().c_str(),5,.5,5.5, m_maxBin, 0, m_maxBin);
    }
    fs->file().cd("/");
    td = fs->mkdir("Common");
    m_trackerLayout = td.make< TH2F >("RVsZ", "R vs. z position", 6000, -300.0, 300.0, 600, 0.0, 30.0);
    m_trackerLayoutXY = td.make< TH2F >("XVsY", "x vs. y position", 1000, -50.0, 50.0, 1000, -50.0, 50.0);
}

// ------------ method called for each event  ------------
void
ITclusterAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    //get the clusters
    edm::Handle<edmNew::DetSetVector<SiPixelCluster>> tclusters;
    iEvent.getByToken(m_tokenClusters, tclusters);

    // Get the geometry
    edm::ESHandle< TrackerGeometry > tgeomHandle;
    iSetup.get< TrackerDigiGeometryRecord >().get("idealForDigi", tgeomHandle);
    
    // Get the topology
    edm::ESHandle< TrackerTopology > tTopoHandle;
    iSetup.get< TrackerTopologyRcd >().get(tTopoHandle);

    //get the pointers to geometry, topology and clusters
    const TrackerTopology* tTopo = tTopoHandle.product();
    //const TrackerGeometry* tkGeom = &(*geomHandle);
    const TrackerGeometry* tkGeom = tgeomHandle.product();
    const edmNew::DetSetVector<SiPixelCluster>* clusters = tclusters.product();

    //a 2D counter array to count the number of clusters per disk and per ring
    unsigned int cluCounter[8][5];
    memset(cluCounter, 0, sizeof(cluCounter));

    //loop the modules in the cluster collection
    for (typename edmNew::DetSetVector<SiPixelCluster>::const_iterator DSVit = clusters->begin(); DSVit != clusters->end(); DSVit++)
    {
        //get the detid
        unsigned int rawid(DSVit->detId());
        DetId detId(rawid);
        TrackerGeometry::ModuleType mType = tkGeom->getDetectorType(detId);
        if(mType != TrackerGeometry::ModuleType::Ph2PXF && detId.subdetId() != PixelSubdetector::PixelEndcap) continue;

        //find out which layer, side and ring
        unsigned int side = (tTopo->pxfSide(detId)); // values are 1 and 2 for -+Z
        unsigned int layer= (tTopo->pxfDisk(detId)); //values are 1 to 12 for disks TFPX1 to TFPX 8  and TEPX1 to TEPX 4
        unsigned int ring = (tTopo->pxfBlade(detId));

        //now make sure we are only looking at TEPX
        if(layer < 9) continue;

        //the index in my histogram map
        int hist_id = -1;
        unsigned int ring_id = ring-1;
        
        if(side == 1)
        {
            //this is a TEPX- hit on side1
            hist_id = layer - 9;
        }
        else if (side ==2)
        {
            //this is a TEPX+ hit on side 2
            hist_id = 4+layer-9;
        }

        // Get the geomdet
        const GeomDetUnit* geomDetUnit(tkGeom->idToDetUnit(detId));
        if (!geomDetUnit) continue;
        //std::cout << geomDetUnit << std::endl;
        
        unsigned int nClu = 0;
        //now loop the clusters for each detector
        for(edmNew::DetSet<SiPixelCluster>::const_iterator cluit = DSVit->begin(); cluit != DSVit->end(); cluit++)
        {
            nClu++;
            cluCounter[hist_id][ring_id]++;
            //here could run more checks or get local or global position using the GeomDetUnit to find overlaps etc
            // determine the position
            MeasurementPoint mpClu(cluit->x(), cluit->y());
            Local3DPoint localPosClu = geomDetUnit->topology().localPosition(mpClu);
            Global3DPoint globalPosClu = geomDetUnit->surface().toGlobal(localPosClu);

            //fill TkLayout histos
            m_trackerLayout->Fill(globalPosClu.z(), globalPosClu.perp());
            m_trackerLayoutXY->Fill(globalPosClu.x(), globalPosClu.y());
        }

        std::cout << "Found a Phase 2 TEPX module with " << nClu << " clusters for side "<< side << " layer " <<layer <<" ring " << ring << " will end in histogram "<<hist_id <<"!" << std::endl;
        m_diskHistos[hist_id]->Fill(ring, cluCounter[hist_id][ring_id]);
    }
}


// ------------ method called once each job just after ending the event loop  ------------
void
ITclusterAnalyzer::endJob()
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
ITclusterAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(ITclusterAnalyzer);
