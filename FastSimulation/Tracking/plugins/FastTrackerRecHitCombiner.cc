// system include files
#include <memory>

// math for matching extra RecHits
#include <cmath>
#include <typeinfo>

// frame work stuff
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

// specific for this producer
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "DataFormats/TrackerRecHit2D/interface/FastTrackerRecHitCollection.h"
#include "FastSimulation/Tracking/interface/FastTrackerRecHitSplitter.h"

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"

class FastTrackerRecHitCombiner : public edm::stream::EDProducer<> {
    public:

    explicit FastTrackerRecHitCombiner(const edm::ParameterSet&);
    ~FastTrackerRecHitCombiner() override{;}

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

    private:

    void beginStream(edm::StreamID) override{;}
    void produce(edm::Event&, const edm::EventSetup&) override;
    void endStream() override{;}

    // ----------member data ---------------------------
    edm::EDGetTokenT<edm::PSimHitContainer> simHitsToken; 
    edm::EDGetTokenT<FastTrackerRecHitRefCollection> simHit2RecHitMapToken;
    unsigned int minNHits;
};


FastTrackerRecHitCombiner::FastTrackerRecHitCombiner(const edm::ParameterSet& iConfig)
    : simHitsToken(consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("simHits")))
    , simHit2RecHitMapToken(consumes<FastTrackerRecHitRefCollection>(iConfig.getParameter<edm::InputTag>("simHit2RecHitMap")))
    , minNHits(iConfig.getParameter<unsigned int>("minNHits"))
{
    produces<FastTrackerRecHitCombinationCollection>();
}


void
    FastTrackerRecHitCombiner::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    // services
    edm::ESHandle<TrackerGeometry> geometry;
    iSetup.get<TrackerDigiGeometryRecord> ().get (geometry);

    // input
    edm::Handle<edm::PSimHitContainer> simHits;
    iEvent.getByToken(simHitsToken,simHits);

    edm::Handle<FastTrackerRecHitRefCollection> simHit2RecHitMap;
    iEvent.getByToken(simHit2RecHitMapToken,simHit2RecHitMap);
 
    // output
    std::unique_ptr<FastTrackerRecHitCombinationCollection> output(new FastTrackerRecHitCombinationCollection);
   
    // declare ahead
    float deltaFkrecSim;
    //int added;

    FastTrackerRecHitCombination currentCombination;
    for(unsigned int simHitCounter = 0;simHitCounter < simHits->size();simHitCounter++){
	
	// get simHit and recHit
	const PSimHit & simHit = (*simHits)[simHitCounter];
	const FastTrackerRecHitRef & recHit = (*simHit2RecHitMap)[simHitCounter];

        // get simHit pos
        const LocalPoint& simLocalPoint = simHit.localPosition();
        const DetId& simDetId = simHit.detUnitId();
        const GeomDet* simGeomDet = geometry->idToDet(simDetId);
        const GlobalPoint& simPos = simGeomDet->toGlobal(simLocalPoint);

	// add recHit to latest combination
	if(!recHit.isNull()){
	    currentCombination.push_back(recHit);}

        //added = 0;
        // Delta R determined addition
        for(unsigned int fksimHitCounter = simHitCounter; fksimHitCounter < simHits->size(); fksimHitCounter++){
            
            const PSimHit & fksimHit = (*simHits)[fksimHitCounter];
            if(fksimHit.trackId() != simHit.trackId()){
                const FastTrackerRecHitRef & fkrecHit = (*simHit2RecHitMap)[fksimHitCounter];
 
                if(!fkrecHit.isNull() && fkrecHit != recHit) {
                    const GlobalPoint& fkrecPos = fkrecHit->globalPosition();
                
                    deltaFkrecSim = sqrt(pow(simPos.x()-fkrecPos.x(),2)+pow(simPos.y()-fkrecPos.y(),2)+pow(simPos.z()-fkrecPos.z(),2));
                    //std::cout << "DISTANCE: " << deltaFkrecSim << std::endl;
                    if(deltaFkrecSim < 0.1){
                        //std::cout << "DISTANCE: " << deltaFkrecSim << std::endl;
                        currentCombination.push_back(fkrecHit);
                        //added++;
                        //if (added > 3) {break;}
                    }
                }
            }
        }

        //std::cout << "N fakes added: " << currentCombination.size() << std::endl;

	// if simTrackId is about to change, add combination
	if(simHits->size()-simHitCounter == 1 || simHit.trackId() != (*simHits)[simHitCounter+1].trackId() ){
	    // combination must have sufficient hits
	    if(currentCombination.size() >= minNHits){
		currentCombination.shrink_to_fit();
		output->push_back(currentCombination);
	    }
	    currentCombination.clear();
	}
    }

    // put output in event
    iEvent.put(std::move(output));
}
    

 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void FastTrackerRecHitCombiner::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    //The following says we do not know what parameters are allowed so do no validation
    // Please change this to state exactly what you do use, even if it is no parameters
    edm::ParameterSetDescription desc;
    desc.setUnknown();
    descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(FastTrackerRecHitCombiner);
