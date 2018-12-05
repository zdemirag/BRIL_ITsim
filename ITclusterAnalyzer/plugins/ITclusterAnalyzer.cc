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
      TH2F* m_diskHistos[8];
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
    TFileDirectory td = fs->mkdir("Common");
    //now lets create the histograms
    for(unsigned int i=0; i <8; i++)
    {
        std::stringstream histoname;
        histoname << "Number of clusters for Disk " << i <<";Ring;# of Clusters";
        //name, name, nbinX, Xlow, Xhigh, nbinY, Ylow, Yhigh
        m_diskHistos[i] = td.make<TH2F>(histoname.str().c_str(),histoname.str().c_str(),5,0,5, m_maxBin, 0, m_maxBin);
    }
}

// ------------ method called for each event  ------------
void
ITclusterAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    //get the clusters
    edm::Handle<edmNew::DetSetVector<SiPixelCluster>> clusters;
    iEvent.getByToken(m_tokenClusters, clusters);

    // Get the geometry
         edm::ESHandle< TrackerGeometry > geomHandle;
         iSetup.get< TrackerDigiGeometryRecord >().get("idealForDigi", geomHandle);
         const TrackerGeometry* tkGeom = &(*geomHandle);
         edm::ESHandle< TrackerTopology > tTopoHandle;
         iSetup.get< TrackerTopologyRcd >().get(tTopoHandle);
         const TrackerTopology* tTopo = tTopoHandle.product();
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
