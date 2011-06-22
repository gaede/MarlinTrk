#include "RefitProcessor.h"
#include <iostream>

#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/LCFlagImpl.h>
#include <EVENT/Track.h>

#include <IMPL/LCRelationImpl.h>
#include <UTIL/LCRelationNavigator.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

//---- GEAR ----
#include "marlin/Global.h"
#include "gear/GEAR.h"

#include <ILDDetectorIDs.h>


#include "MarlinKalTest.h"
#include "MarlinKalTestTrack.h"

#include "ILDTPCMeasLayer.h"

using namespace lcio ;
using namespace marlin ;


RefitProcessor aRefitProcessor ;


RefitProcessor::RefitProcessor() : Processor("RefitProcessor") {
  
  // modify processor description
  _description = "RefitProcessor refits an input track collection, producing a new collection of tracks." ;
  

  // register steering parameters: name, description, class-variable, default value

  registerInputCollection( LCIO::TRACK,
			   "InputTrackCollectionName" , 
			   "Name of the input track collection"  ,
			   _input_track_col_name ,
			   std::string("LDCTracks") ) ;

  registerInputCollection( LCIO::LCRELATION,
			   "InputTrackRelCollection" , 
			   "Name of the MCParticle-Track Relations collection for input tracks"  ,
			   _input_track_rel_name ,
			   std::string("LDCTracksMCP") ) ;

  registerOutputCollection( LCIO::TRACK,
			   "OutputTrackCollectionName" , 
			   "Name of the output track collection"  ,
			   _output_track_col_name ,
			   std::string("RefittedLDCTracks") ) ;

  registerOutputCollection( LCIO::LCRELATION,
			   "OutputTrackRelCollection" , 
			   "Name of the MCParticle-Track Relations collection for output tracks"  ,
			   _output_track_rel_name ,
			   std::string("RefittedLDCTracksMCP") ) ;

registerProcessorParameter("MultipleScatteringOn",
			     "Use MultipleScattering in Fit",
			     _MSOn,
			     bool(true));

registerProcessorParameter("EnergyLossOn",
			     "Use Energy Loss in Fit",
			     _ElossOn,
			     bool(true));

}


void RefitProcessor::init() { 

  streamlog_out(DEBUG) << "   init called  " 
		       << std::endl ;
    
  // usually a good idea to
  printParameters() ;

  // set up the geometery needed by KalTest
  _kaltest = new MarlinKalTest( *marlin::Global::GEAR, _MSOn, _ElossOn) ;

  _n_run = 0 ;
  _n_evt = 0 ;
  
}

void RefitProcessor::processRunHeader( LCRunHeader* run) { 

  ++_n_run ;
} 

void RefitProcessor::processEvent( LCEvent * evt ) { 

    
  //-- note: this will not be printed if compiled w/o MARLINDEBUG=1 !

  streamlog_out(DEBUG) << "   processing event: " << _n_evt 
		       << std::endl ;
   
  // get input collection and relations 
  LCCollection* input_track_col = this->GetCollection( evt, _input_track_col_name ) ;

  //  LCRelationNavigator* input_track_rels = this->GetRelations( evt, _input_track_rel_name ) ;

  if( input_track_col != 0 ){

    // establish the track collection that will be created 
    LCCollectionVec* trackVec = new LCCollectionVec( LCIO::TRACK )  ;    

    // if we want to point back to the hits we need to set the flag
    LCFlagImpl trkFlag(0) ;
    trkFlag.setBit( LCIO::TRBIT_HITS ) ;
    trackVec->setFlag( trkFlag.getFlag()  ) ;

    // establish the track relations collection that will be created 
    LCCollectionVec* trackRelVec = new LCCollectionVec( LCIO::LCRELATION )  ;

    int nTracks = input_track_col->getNumberOfElements()  ;

    // loop over the input tacks and refit using KalTest    
    for(int i=0; i< nTracks ; ++i)
      {
      
	Track* track = dynamic_cast<Track*>( input_track_col->getElementAt( i ) ) ;
		
	MarlinTrk::IMarlinTrack* marlin_trk = new MarlinKalTestTrack(track, _kaltest) ;
	
	bool fit_success = marlin_trk->fit( false ) ; // SJA:FIXME: false means from out to in here i.e. backwards. This would be better if had a more meaningful name perhaps fit_fwd and fit_rev

	if( fit_success ){ 

	  TrackImpl* refittedTrack = marlin_trk->getIPFit() ;

	  // assign the relations previously assigned to the input tracks  
	  //	LCObjectVec objVec = input_track_rels->getRelatedToObjects( track );
	  //	FloatVec weights = input_track_rels->getRelatedToWeights( track ); 
	  
	  //	for( unsigned int irel=0 ; irel < objVec.size() ; ++irel )
	  //	  {
	  //	    LCRelationImpl* rel = new LCRelationImpl ;
	  //	    rel->setFrom (refittedTrack) ;
	  //            rel->setTo ( objVec[irel] ) ;
	  //	    rel->setWeight(weights[irel]) ; 
	  //	    trackRelVec->addElement( rel );
	  //	  }

	  //	//SJA:FIXME: This has to go away. The use of hardcoded number here is completely error prone ...
	  refittedTrack->subdetectorHitNumbers().resize(12);
	  for ( unsigned int detIndex = 0 ;  detIndex < refittedTrack->subdetectorHitNumbers().size() ; detIndex++ ) 
	    {
	      refittedTrack->subdetectorHitNumbers()[detIndex] = track->getSubdetectorHitNumbers()[detIndex] ;
	  }
	  
	  trackVec->addElement( refittedTrack );
	}

	delete marlin_trk;
	
      } 
    evt->addCollection( trackVec , _output_track_col_name) ;
    //    evt->addCollection( trackRelVec , _output_track_rel_name) ;
  }  
  ++_n_evt ;
}



void RefitProcessor::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void RefitProcessor::end(){ 
  
  streamlog_out(DEBUG) << "RefitProcessor::end()  " << name() 
 	    << " processed " << _n_evt << " events in " << _n_run << " runs "
 	    << std::endl ;

}

LCCollection* RefitProcessor::GetCollection( LCEvent * evt, std::string colName ){

  LCCollection* col = NULL;
  
  int nElements = 0;

  try{
    col = evt->getCollection( colName.c_str() ) ;
    nElements = col->getNumberOfElements()  ;
    streamlog_out( DEBUG4 ) << " --> " << colName.c_str() << " track collection found in event = " << col << " number of elements " << col->getNumberOfElements() << std::endl;
  }
  catch(DataNotAvailableException &e){
    streamlog_out( DEBUG4 ) << " --> " << colName.c_str() <<  " collection absent in event" << std::endl;     
  }

  return col; 

}

LCRelationNavigator* RefitProcessor::GetRelations(LCEvent * evt , std::string RelName ) {

  LCRelationNavigator* nav = NULL ;

  try{
    nav = new LCRelationNavigator(evt->getCollection( RelName.c_str() ));
    streamlog_out( DEBUG2 ) << "RefitProcessor --> " << RelName << " track relation collection in event = " << nav << std::endl;
  }
  catch(DataNotAvailableException &e){
    streamlog_out( DEBUG2 ) << "RefitProcessor --> " << RelName.c_str() << " track relation collection absent in event" << std::endl;     
  }
  
  return nav;
  
}

