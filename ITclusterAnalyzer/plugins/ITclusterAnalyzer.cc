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

#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
//#include "DataFormats/Phase2TrackerDigi/interface/Phase2TrackerDigi.h"
#include "DataFormats/SiPixelDigi/interface/PixelDigi.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CommonTools/Utils/interface/TFileDirectory.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include <TH2F.h>

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

// a struct to hold the residuals for each matched cluster
struct Residual {
    double dx;
    double dy;
    double dr;

    Residual(double x, double y)
        : dx(x)
        , dy(y)
    {
        dr = sqrt(pow(dx, 2) + pow(dy, 2));
    }
    void print()
    {
        std::cout << "(dx: " << dx << " dy: " << dy << " dr: " << dr << ") ";
    }
};

class ITclusterAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
    explicit ITclusterAnalyzer(const edm::ParameterSet&);
    ~ITclusterAnalyzer();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
    virtual void beginJob() override;
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    virtual void endJob() override;

    //bool findCoincidence(DetId, Global3DPoint, bool);
    bool findCoincidence2x(DetId, Global3DPoint);
    bool findCoincidence3x(DetId, Global3DPoint);

    // ----------member data ---------------------------
    edm::EDGetTokenT<edmNew::DetSetVector<SiPixelCluster>> m_tokenClusters;

    // the pointers to geometry, topology and clusters
    // these are members so all functions can access them without passing as argument
    const TrackerTopology* tTopo = NULL;
    const TrackerGeometry* tkGeom = NULL;
    const edmNew::DetSetVector<SiPixelCluster>* clusters = NULL;

    //max bins of Counting histogram
    uint32_t m_maxBin;
    //flag for checking coincidences
    bool m_docoincidence;

    //array of TH2F for clusters per disk per ring
    TH2F* m_diskHistosCluster[8];
    //tracker maps for clusters
    TH2F* m_trackerLayoutClustersZR;
    TH2F* m_trackerLayoutClustersYX;

    //array of TH2F for 2xcoinc per disk per ring
    TH2F* m_diskHistos2x[8];
    //tracker maps for 2xcoinc
    TH2F* m_trackerLayout2xZR;
    TH2F* m_trackerLayout2xYX;

    //array of TH2F for 3xcoinc per disk per ring
    TH2F* m_diskHistos3x[8];
    //tracker maps for 2xcoinc
    TH2F* m_trackerLayout3xZR;
    TH2F* m_trackerLayout3xYX;

    //simple residual histograms for the cuts
    TH1F* m_residualX;
    TH1F* m_residualY;
    TH1F* m_residualR;

    //the number of clusters per module
    TH1F* m_nClusters;

    //cuts for the coincidence
    double m_dx;
    double m_dy;
    double m_dz;

    //event counter
    uint32_t m_nevents;
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
    : //m_tokenClusters(consumes<edmNew::DetSetVector<SiPixelCluster>> ("clusters"))
    m_tokenClusters(consumes<edmNew::DetSetVector<SiPixelCluster>>(iConfig.getParameter<edm::InputTag>("clusters")))
    , m_maxBin(iConfig.getUntrackedParameter<uint32_t>("maxBin"))
    , m_docoincidence(iConfig.getUntrackedParameter<bool>("docoincidence"))
    , m_dx(iConfig.getParameter<double>("dx_cut"))
    , m_dy(iConfig.getParameter<double>("dy_cut"))
    , m_dz(iConfig.getParameter<double>("dz_cut"))
{
    //now do what ever initialization is needed
    m_nevents = 0;
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
void ITclusterAnalyzer::beginJob()
{
    edm::Service<TFileService> fs;

    fs->file().cd("/");
    TFileDirectory td = fs->mkdir("Residuals");

    m_residualX = td.make<TH1F>("ResidualsX", "ResidualsX;deltaX;counts", 1000, 0, 1);
    m_residualY = td.make<TH1F>("ResidualsY", "ResidualsY;deltaY;counts", 1000, 0, 1);
    m_residualR = td.make<TH1F>("ResidualsR", "ResidualsR;deltaR;counts", 1000, 0, 1);

    fs->file().cd("/");
    td = fs->mkdir("perModule");
    m_nClusters = td.make<TH1F>("Number of Clusters per module per event", "# of Clusters;# of Clusters; Occurence", 500, 0, 500);

    fs->file().cd("/");
    td = fs->mkdir("Clusters");

    //now lets create the histograms
    for (unsigned int i = 0; i < 8; i++) {
        int disk = (i < 4) ? i - 4 : i - 3;
        std::stringstream histoname;
        histoname << "Number of clusters for Disk " << disk << ";Ring;# of Clusters per event";
        std::stringstream histotitle;
        histotitle << "Number of clusters for Disk " << disk;
        //name, name, nbinX, Xlow, Xhigh, nbinY, Ylow, Yhigh
        m_diskHistosCluster[i] = td.make<TH2F>(histotitle.str().c_str(), histoname.str().c_str(), 5, .5, 5.5, m_maxBin, 0, m_maxBin);
    }
    m_trackerLayoutClustersZR = td.make<TH2F>("RVsZ", "R vs. z position", 6000, -300.0, 300.0, 600, 0.0, 30.0);
    m_trackerLayoutClustersYX = td.make<TH2F>("XVsY", "x vs. y position", 1000, -50.0, 50.0, 1000, -50.0, 50.0);

    if (m_docoincidence) {
        fs->file().cd("/");
        td = fs->mkdir("2xCoincidences");
        //now lets create the histograms
        for (unsigned int i = 0; i < 8; i++) {
            int disk = (i < 4) ? i - 4 : i - 3;
            std::stringstream histoname;
            histoname << "Number of 2x Coincidences for Disk " << disk << ";Ring;# of coincidences per event";
            std::stringstream histotitle;
            histotitle << "Number of 2x Coincidences for Disk " << disk;
            //name, name, nbinX, Xlow, Xhigh, nbinY, Ylow, Yhigh
            m_diskHistos2x[i] = td.make<TH2F>(histotitle.str().c_str(), histoname.str().c_str(), 5, .5, 5.5, m_maxBin, 0, m_maxBin);
        }
        m_trackerLayout2xZR = td.make<TH2F>("RVsZ", "R vs. z position", 6000, -300.0, 300.0, 600, 0.0, 30.0);
        m_trackerLayout2xYX = td.make<TH2F>("XVsY", "x vs. y position", 1000, -50.0, 50.0, 1000, -50.0, 50.0);
    }

    if (m_docoincidence) {
        fs->file().cd("/");
        td = fs->mkdir("3xCoincidences");
        //now lets create the histograms
        for (unsigned int i = 0; i < 8; i++) {
            int disk = (i < 4) ? i - 4 : i - 3;
            std::stringstream histoname;
            histoname << "Number of 3x Coincidences for Disk " << disk << ";Ring;# of coincidences per event";
            std::stringstream histotitle;
            histotitle << "Number of 3x Coincidences for Disk " << disk;
            //name, name, nbinX, Xlow, Xhigh, nbinY, Ylow, Yhigh
            m_diskHistos3x[i] = td.make<TH2F>(histotitle.str().c_str(), histoname.str().c_str(), 5, .5, 5.5, m_maxBin, 0, m_maxBin);
        }
        m_trackerLayout3xZR = td.make<TH2F>("RVsZ", "R vs. z position", 6000, -300.0, 300.0, 600, 0.0, 30.0);
        m_trackerLayout3xYX = td.make<TH2F>("XVsY", "x vs. y position", 1000, -50.0, 50.0, 1000, -50.0, 50.0);
    }
}

// ------------ method called for each event  ------------
void ITclusterAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    //get the clusters
    edm::Handle<edmNew::DetSetVector<SiPixelCluster>> tclusters;
    iEvent.getByToken(m_tokenClusters, tclusters);

    // Get the geometry
    edm::ESHandle<TrackerGeometry> tgeomHandle;
    iSetup.get<TrackerDigiGeometryRecord>().get("idealForDigi", tgeomHandle);

    // Get the topology
    edm::ESHandle<TrackerTopology> tTopoHandle;
    iSetup.get<TrackerTopologyRcd>().get(tTopoHandle);

    //get the pointers to geometry, topology and clusters
    tTopo = tTopoHandle.product();
    //const TrackerGeometry* tkGeom = &(*geomHandle);
    tkGeom = tgeomHandle.product();
    clusters = tclusters.product();

    //a 2D counter array to count the number of clusters per disk and per ring
    unsigned int cluCounter[8][5];
    memset(cluCounter, 0, sizeof(cluCounter));
    //counter for 2x coincidences
    unsigned int x2Counter[8][5];
    memset(x2Counter, 0, sizeof(x2Counter));

    unsigned int x3Counter[8][5];
    memset(x3Counter, 0, sizeof(x3Counter));

    //loop the modules in the cluster collection
    for (typename edmNew::DetSetVector<SiPixelCluster>::const_iterator DSVit = clusters->begin(); DSVit != clusters->end(); DSVit++) {
        //get the detid
        unsigned int rawid(DSVit->detId());
        DetId detId(rawid);

        //figure out the module type using the detID
        TrackerGeometry::ModuleType mType = tkGeom->getDetectorType(detId);
        if (mType != TrackerGeometry::ModuleType::Ph2PXF && detId.subdetId() != PixelSubdetector::PixelEndcap)
            continue;
        //std::cout << "DetID " << std::hex << "0x" << rawid << std::dec << " " << detId.det() << " " << detId.subdetId() << " " << ((rawid >> 23) & 0x3) << " " << ((rawid >> 18) & 0xF) << " " << ((rawid >> 12) & 0x3F) << " " << ((rawid >> 2) & 0xFF) << std::endl;

        //find out which layer, side and ring
        unsigned int side = (tTopo->pxfSide(detId));  // values are 1 and 2 for -+Z
        unsigned int layer = (tTopo->pxfDisk(detId)); //values are 1 to 12 for disks TFPX1 to TFPX 8  and TEPX1 to TEPX 4
        unsigned int ring = (tTopo->pxfBlade(detId));

        //now make sure we are only looking at TEPX
        if (layer < 9)
            continue;

        //the index in my histogram map
        int hist_id = -1;
        unsigned int ring_id = ring - 1;

        if (side == 1) {
            //this is a TEPX- hit on side1
            hist_id = layer - 9;
        } else if (side == 2) {
            //this is a TEPX+ hit on side 2
            hist_id = 4 + layer - 9;
        }

        // Get the geomdet
        const GeomDetUnit* geomDetUnit(tkGeom->idToDetUnit(detId));
        if (!geomDetUnit)
            continue;

        unsigned int nClu = 0;

        //fill the number of clusters for this module
        m_nClusters->Fill(DSVit->size());

        //now loop the clusters for each detector
        for (edmNew::DetSet<SiPixelCluster>::const_iterator cluit = DSVit->begin(); cluit != DSVit->end(); cluit++) {
            //increment the counters
            nClu++;
            cluCounter[hist_id][ring_id]++;

            // determine the position
            MeasurementPoint mpClu(cluit->x(), cluit->y());
            Local3DPoint localPosClu = geomDetUnit->topology().localPosition(mpClu);
            Global3DPoint globalPosClu = geomDetUnit->surface().toGlobal(localPosClu);

            //fill TkLayout histos
            m_trackerLayoutClustersZR->Fill(globalPosClu.z(), globalPosClu.perp());
            m_trackerLayoutClustersYX->Fill(globalPosClu.x(), globalPosClu.y());

            //std::cout << globalPosClu.x() << " " << globalPosClu.y() << std::endl;
            if (m_docoincidence) {
                bool found = this->findCoincidence2x(detId, globalPosClu);
                if (found) {
                    x2Counter[hist_id][ring_id]++;
                    m_trackerLayout2xZR->Fill(globalPosClu.z(), globalPosClu.perp());
                    m_trackerLayout2xYX->Fill(globalPosClu.x(), globalPosClu.y());

                    //done with 2 fold coincidences, now 3 fold
                    //only if we have a 2 fold coincidence we can search for a third one in another ring
                    found = false;
                    found = this->findCoincidence3x(detId, globalPosClu);
                    if (found) {
                        x3Counter[hist_id][ring_id]++;
                        m_trackerLayout3xZR->Fill(globalPosClu.z(), globalPosClu.perp());
                        m_trackerLayout3xYX->Fill(globalPosClu.x(), globalPosClu.y());
                    }
                }
            }
        } //end of cluster loop
    }     //end of module loop

    //ok, now I know the number of clusters per ring per disk and should fill the histogram once for this event
    for (unsigned int i = 0; i < 8; i++) {
        //loop the disks
        for (unsigned int j = 0; j < 5; j++) {
            //and the rings
            m_diskHistosCluster[i]->Fill(j + 1, cluCounter[i][j]);
            if (m_docoincidence) {
                m_diskHistos2x[i]->Fill(j + 1, x2Counter[i][j]);
                m_diskHistos3x[i]->Fill(j + 1, x3Counter[i][j]);
            }
        }
    }
    m_nevents++;
}

// ------------ method called once each job just after ending the event loop  ------------
void ITclusterAnalyzer::endJob()
{
    std::cout << "IT cluster Analyzer processed " << m_nevents << " events!" << std::endl;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void ITclusterAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions)
{
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

bool ITclusterAnalyzer::findCoincidence2x(DetId thedetid, Global3DPoint theglobalPosClu)
{
    bool found = false;
    uint32_t rawid = thedetid.rawId();
    uint32_t newid = rawid;
    //now I have the raw ID and can mess with the bits
    //the side, layer and ring are the same and I just have to increment or decrement the module number
    unsigned int themodule = (tTopo->pxfModule(thedetid));
    unsigned int thering = (tTopo->pxfBlade(thedetid));

    //in order to avoid duplicates, only look in the next module clockwise
    //depending on the ring, if I am already on the module with the highest module id in the ring, I need to go to the first one
    uint32_t newmodule = themodule + 1;
    if (thering == 1 && themodule == 20)
        newmodule = 1;
    else if (thering == 2 && themodule == 28)
        newmodule = 1;
    else if (thering == 3 && themodule == 36)
        newmodule = 1;
    else if (thering == 4 && themodule == 44)
        newmodule = 1;
    else if (thering == 5 && themodule == 48)
        newmodule = 1;

    //now encode
    newid = (newid & 0xFFFFFC03) | ((newmodule & 0xFF) << 2);

    //now I have a raw id of the module I want to use
    DetId id(newid);
    unsigned int ring = (tTopo->pxfBlade(id));
    unsigned int module = (tTopo->pxfModule(id));

    edmNew::DetSetVector<SiPixelCluster>::const_iterator theit = clusters->find(id);
    if (theit == clusters->end()) {
        return false;
    }

    // Get the geomdet
    const GeomDetUnit* geomDetUnit(tkGeom->idToDetUnit(id));

    unsigned int nClu = 0;
    //at the end of the day, need to find the closest coincidence hit, so store the minimum 2D distance in a temporary variable and a vector for all values
    double r_min = 1000.;
    std::vector<Residual> r_vec;

    for (edmNew::DetSet<SiPixelCluster>::const_iterator cluit = theit->begin(); cluit != theit->end(); cluit++) {

        // determine the position
        MeasurementPoint mpClu(cluit->x(), cluit->y());
        Local3DPoint localPosClu = geomDetUnit->topology().localPosition(mpClu);
        Global3DPoint globalPosClu = geomDetUnit->surface().toGlobal(localPosClu);

        //now check that the global position is within the cuts
        if (fabs(globalPosClu.x() - theglobalPosClu.x()) < m_dx
            && fabs(globalPosClu.y() - theglobalPosClu.y()) < m_dy
            && fabs(globalPosClu.z() - theglobalPosClu.z()) < m_dz) {
            nClu++;

            double delta_x = fabs(globalPosClu.x() - theglobalPosClu.x());
            double delta_y = fabs(globalPosClu.y() - theglobalPosClu.y());
            Residual r(delta_x, delta_y);
            r_vec.push_back(r);

            if (r.dr < r_min)
                r_min = r.dr;
            found = true;

            //std::cout << "Found matching cluster # " << nClu << std::endl;

            //std::cout << "Original x: " << theglobalPosClu.x() << " y: " << theglobalPosClu.y() << " z: " << theglobalPosClu.z() << " ring: " << thering << " module: " << themodule << std::endl;
            //std::cout << "New      x: " << globalPosClu.x() << " y: " << globalPosClu.y() << " z: " << globalPosClu.z() << " ring: " << ring << " module: " << module << std::endl;
        }
    }
    if (nClu > 1) {
        //std::cout << "Warning, found " << nClu << "Clusters within the cuts - the minimum distance is " << r_min << "!" << std::endl;
        //std::cout << "All distances: ";
        for (auto r : r_vec) {
            //r.print();
            if (r.dr == r_min) {
                m_residualX->Fill(r.dx);
                m_residualY->Fill(r.dy);
                m_residualR->Fill(r.dr);
            }
        }
        std::cout << std::endl;
    }
    return found;
}

bool ITclusterAnalyzer::findCoincidence3x(DetId thedetid, Global3DPoint theglobalPosClu)
{
    bool found = false;
    uint32_t rawid = thedetid.rawId();
    uint32_t newid = rawid;
    //now I have the raw ID and can mess with the bits
    //the side and layer are the same and I just have to look in a lower ring
    unsigned int themodule = (tTopo->pxfModule(thedetid));
    unsigned int thering = (tTopo->pxfBlade(thedetid));

    unsigned int maxmodule = 0;
    unsigned int newring = 0;

    if (thering > 1 && thering <= 5) {
        newring = thering - 1; //look in a lower ring
        //now get the max range of modules for the respective ring
        if (newring == 1)
            maxmodule = 20;
        else if (newring == 2)
            maxmodule = 28;
        else if (newring == 3)
            maxmodule = 36;
        else if (newring == 4)
            maxmodule = 44;
    } else
        return false;

    for (uint32_t newmodule = 1; newmodule <= maxmodule; newmodule++) {
        newid = (newid & 0xFFFC0C03) | ((newring & 0x3F) << 12) | ((newmodule & 0xFF) << 2);

        DetId id(newid);
        unsigned int ring = (tTopo->pxfBlade(id));
        unsigned int module = (tTopo->pxfModule(id));

        edmNew::DetSetVector<SiPixelCluster>::const_iterator theit = clusters->find(id);
        if (theit == clusters->end()) {
            return false;
        }
        // Get the geomdet
        const GeomDetUnit* geomDetUnit(tkGeom->idToDetUnit(id));
        if (!geomDetUnit)
            continue;

        unsigned int nClu = 0;
        //to make sure we only use the closest hit
        double r_min = 1000.;
        std::vector<Residual> r_vec;

        for (edmNew::DetSet<SiPixelCluster>::const_iterator cluit = theit->begin(); cluit != theit->end(); cluit++) {

            // determine the position
            MeasurementPoint mpClu(cluit->x(), cluit->y());
            Local3DPoint localPosClu = geomDetUnit->topology().localPosition(mpClu);
            Global3DPoint globalPosClu = geomDetUnit->surface().toGlobal(localPosClu);

            //now check that the global position is within the cuts
            if (fabs(globalPosClu.x() - theglobalPosClu.x()) < m_dx
                && fabs(globalPosClu.y() - theglobalPosClu.y()) < m_dy
                && fabs(globalPosClu.z() - theglobalPosClu.z()) < m_dz) {
                nClu++;
                found = true;
                double delta_x = fabs(globalPosClu.x() - theglobalPosClu.x());
                double delta_y = fabs(globalPosClu.y() - theglobalPosClu.y());
                Residual r(delta_x, delta_y);
                r_vec.push_back(r);

                if (r.dr < r_min)
                    r_min = r.dr;
                //std::cout << "Found matching cluster # " << nClu << " which is a 3x coincidence" << std::endl;
                //std::cout << "Original x: " << theglobalPosClu.x() << " y: " << theglobalPosClu.y() << " z: " << theglobalPosClu.z() << " ring: " << thering << " module: " << themodule << std::endl;
                //std::cout << "New      x: " << globalPosClu.x() << " y: " << globalPosClu.y() << " z: " << globalPosClu.z() << " ring: " << ring << " module: " << module << std::endl;
            }
        }
        //if (nClu > 1) {
        //std::cout << "3X COINCIDENCE - Warning, found " << nClu << "Clusters within the cuts - the minimum distance is " << r_min << "!" << std::endl;
        //std::cout << "All distances: ";
        //for (auto r : r_vec) {
        //r.print();
        //}
        //std::cout << std::endl;
        //}
    }
    return found;
}

//bool ITclusterAnalyzer::findCoincidence(DetId thedetid, Global3DPoint theglobalPosClu, bool is3x)
//{
//bool found = false;
////get the side, layer & ring of the original cluster that I want to match to
//unsigned int theside = (tTopo->pxfSide(thedetid));
//unsigned int thelayer = (tTopo->pxfDisk(thedetid));
//unsigned int thering = (tTopo->pxfBlade(thedetid));
//unsigned int themodule = (tTopo->pxfModule(thedetid));

////loop the modules in the cluster collection
//for (typename edmNew::DetSetVector<SiPixelCluster>::const_iterator DSVit = clusters->begin(); DSVit != clusters->end(); DSVit++) {
////get the detid
//unsigned int rawid(DSVit->detId());
//DetId detId(rawid);
//TrackerGeometry::ModuleType mType = tkGeom->getDetectorType(detId);
//if (mType != TrackerGeometry::ModuleType::Ph2PXF && detId.subdetId() != PixelSubdetector::PixelEndcap)
//continue;

////find out which layer, side and ring
//unsigned int side = (tTopo->pxfSide(detId)); // values are 1 and 2 for -+Z
//if (side != theside)
//continue;
//unsigned int layer = (tTopo->pxfDisk(detId)); //values are 1 to 12 for disks TFPX1 to TFPX 8  and TEPX1 to TEPX 4
//if (layer != thelayer)
//continue;
//unsigned int ring = (tTopo->pxfBlade(detId));
//unsigned int module = (tTopo->pxfModule(detId));

////if we are looking for 2x, only accept the same ring
//if (!is3x) {
//if (ring != thering)
//continue;
////check that it is a neighboring module
//if (fabs(module - themodule) != 1)
//continue;
//} else
////if we do 3x, only accept ring +-1
//{
//if (fabs(thering - ring) != 1)
//continue;
//}

//// Get the geomdet
//const GeomDetUnit* geomDetUnit(tkGeom->idToDetUnit(detId));
//if (!geomDetUnit)
//continue;

//unsigned int nClu = 0;
////now loop the clusters for each detector
//for (edmNew::DetSet<SiPixelCluster>::const_iterator cluit = DSVit->begin(); cluit != DSVit->end(); cluit++) {
//// determine the position
//MeasurementPoint mpClu(cluit->x(), cluit->y());
//Local3DPoint localPosClu = geomDetUnit->topology().localPosition(mpClu);
//Global3DPoint globalPosClu = geomDetUnit->surface().toGlobal(localPosClu);

////now check that the global position is within the cuts
//if (fabs(globalPosClu.x() - theglobalPosClu.x()) < m_dx
//&& fabs(globalPosClu.y() - theglobalPosClu.y()) < m_dy
//&& fabs(globalPosClu.z() - theglobalPosClu.z()) < m_dz) {
//nClu++;
//found = true;
//std::cout << "Found matching cluster # " << nClu << " ";
//if (is3x)
//std::cout << "- this is a 3x coincidence!" << std::endl;
//else
//std::cout << std::endl;

//std::cout << "Original x: " << theglobalPosClu.x() << " y: " << theglobalPosClu.y() << " z: " << theglobalPosClu.z() << " ring: " << thering << " module: " << themodule << std::endl;
//std::cout << "New      x: " << globalPosClu.x() << " y: " << globalPosClu.y() << " z: " << globalPosClu.z() << " ring: " << ring << " module: " << module << std::endl;

//if (!is3x) {
//m_residualX->Fill(fabs(globalPosClu.x() - theglobalPosClu.x()));
//m_residualY->Fill(fabs(globalPosClu.y() - theglobalPosClu.y()));
//m_residualZ->Fill(fabs(globalPosClu.z() - theglobalPosClu.z()));
//}
//}
//} //end of cluster loop
//if (nClu > 1)
//std::cout << "Warning, found " << nClu << "Clusters within the cuts!" << std::endl;
////else if (found && nClu == 1)
////std::cout << "Found a Clusters within the cuts!" << std::endl;
//} //end of module loop
//return found;
//}

//define this as a plug-in
DEFINE_FWK_MODULE(ITclusterAnalyzer);
