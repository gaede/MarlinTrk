#include "MarlinTrk/MarlinDDKalTest.h"
#include "MarlinTrk/MarlinDDKalTestTrack.h"

#include "kaltest/TKalDetCradle.h"
#include "kaltest/TVKalDetector.h"
#include "kaltest/THelicalTrack.h"

#include "DDKalTest/DDVMeasLayer.h"
#include "DDKalTest/DDKalDetector.h"

// #include "DDKalTest/DDSupportKalDetector.h"
// #include "DDKalTest/DDVXDKalDetector.h"

//SJA:FIXME: only needed for storing the modules in the layers map
#include <UTIL/BitField64.h>
#include "UTIL/ILDConf.h"

#include "DD4hep/LCDD.h"
#include "DDRec/SurfaceManager.h"

#include <math.h>
#include <cmath>

#include <utility>

#include "streamlog/streamlog.h"

//#include "DDKalTest/DDMeasurementSurfaceStoreFiller.h"

namespace MarlinTrk{
  
  
  MarlinDDKalTest::MarlinDDKalTest()  :
    _ipLayer(NULL) {
    
    streamlog_out( DEBUG4 ) << "  MarlinDDKalTest - initializing the detector ..." << std::endl ;
    
    _det = new TKalDetCradle ;  // from kaltest. TKalDetCradle inherits from TObjArray ... 
    _det->SetOwner( true ) ;    // takes care of deleting subdetector in the end ...
    
    is_initialised = false; 
    
    this->registerOptions() ;
    
    streamlog_out( DEBUG4 ) << "  MarlinDDKalTest - established " << std::endl ;
  }

  MarlinDDKalTest::~MarlinDDKalTest(){
    
#ifdef MARLINTRK_DIAGNOSTICS_ON
    _diagnostics.end();
#endif
    
    delete _det ;
  }
  
  
  void MarlinDDKalTest::init() {
    
    streamlog_out( DEBUG4 ) << "  MarlinDDKalTest - call  this init " << std::endl ;
    
    DD4hep::Geometry::LCDD& lcdd = DD4hep::Geometry::LCDD::getInstance();

    DD4hep::Geometry::DetElement world = lcdd.world() ;

    DD4hep::Geometry::DetElement::Children detectors = world.children() ;

    for ( DD4hep::Geometry::DetElement::Children::const_iterator it=detectors.begin() ; it != detectors.end() ; ++it ){

      DD4hep::Geometry::DetElement det = (*it).second ;

      DDKalDetector* kalDet = new DDKalDetector( det ) ;

      this->storeActiveMeasurementModuleIDs( kalDet ) ;

      _det->Install( *kalDet ) ;
   }


      // MeasurementSurfaceStore& surfstore = _gearMgr->getMeasurementSurfaceStore();
    
    // // Check if the store is filled if not fill it. NOTE: In the case it is filled we just take what we are given and in debug print a message
    // if( surfstore.isFilled() == false ) {

    //   ILDMeasurementSurfaceStoreFiller filler( *_gearMgr );
    //   streamlog_out( DEBUG4 ) << "  MarlinDDKalTest - set up gear surface store using " << filler.getName() << std::endl ;
    //   surfstore.FillStore(&filler);
      
    // }
    // else {
      
    //    streamlog_out( DEBUG4 ) << "  MarlinDDKalTest - MeasurementSurfaceStore is already full. Using store as filled by MeasurementSurfaceStoreFiller " << surfstore.getFillerName() << std::endl ;
      
    // }

    // if (_gearMgr -> getDetectorName() == "LPTPC") {
    //   try{
    //     kaldet::LCTPCKalDetector* tpcdet = new kaldet::LCTPCKalDetector( *_gearMgr )  ;
    //     // store the measurement layer id's for the active layers
    //     this->storeActiveMeasurementModuleIDs(tpcdet);
    //     _det->Install( *tpcdet ) ;
    //   }
    //   catch( gear::UnknownParameterException& e){
    //     streamlog_out( MESSAGE ) << "  MarlinDDKalTest - TPC missing in gear file: TPC Not Built " << std::endl ;
    //   }
    // }
    // else {
    
    //   try{
    //     ILDSupportKalDetector* supportdet = new ILDSupportKalDetector( *_gearMgr )  ;
    //     // get the dedicated ip layer
    //     _ipLayer = supportdet->getIPLayer() ;
    //     // store the measurement layer id's for the active layers, Calo is defined as active
    //     this->storeActiveMeasurementModuleIDs(supportdet);
    //     _det->Install( *supportdet ) ;
    //   }
    //   catch( gear::UnknownParameterException& e){

    //     streamlog_out( ERROR ) << "MarlinDDKalTest - Support Material missing in gear file: Cannot proceed as propagations and extrapolations for cannonical track states are impossible: exit(1) called" << std::endl ;
    //     exit(1);

    //   }

    //   try{
    //     ILDVXDKalDetector* vxddet = new ILDVXDKalDetector( *_gearMgr )  ;
    //     // store the measurement layer id's for the active layers
    //     this->storeActiveMeasurementModuleIDs(vxddet);
    //     _det->Install( *vxddet ) ;
    //   }
    //   catch( gear::UnknownParameterException& e){
    //     streamlog_out( MESSAGE ) << "  MarlinDDKalTest - VXD missing in gear file: VXD Material Not Built " << std::endl ;
    //   }
      
      
    //   bool SIT_found = false ;
    //   try{
    //     ILDSITKalDetector* sitdet = new ILDSITKalDetector( *_gearMgr )  ;
    //     // store the measurement layer id's for the active layers 
    //     this->storeActiveMeasurementModuleIDs(sitdet);
    //     _det->Install( *sitdet ) ;
    //     SIT_found = true ;
    //   }
    //   catch( gear::UnknownParameterException& e){
    //     streamlog_out( MESSAGE ) << "  MarlinDDKalTest - SIT missing in gear file: SIT Not Built " << std::endl ;
    //   }

    //   if( ! SIT_found ){
    //     try{
    //       ILDSITCylinderKalDetector* sitdet = new ILDSITCylinderKalDetector( *_gearMgr )  ;
    //       // store the measurement layer id's for the active layers
    //       this->storeActiveMeasurementModuleIDs(sitdet);
    //       _det->Install( *sitdet ) ;
    //     }
    //     catch( gear::UnknownParameterException& e){
    //       streamlog_out( MESSAGE ) << "  MarlinDDKalTest - Simple Cylinder Based SIT missing in gear file: Simple Cylinder Based SIT Not Built " << std::endl ;
    //     }
    //   }

    //   try{
    //     ILDSETKalDetector* setdet = new ILDSETKalDetector( *_gearMgr )  ;
    //     // store the measurement layer id's for the active layers
    //     this->storeActiveMeasurementModuleIDs(setdet);
    //     _det->Install( *setdet ) ;
    //   }
    //   catch( gear::UnknownParameterException& e){   
    //     streamlog_out( MESSAGE ) << "  MarlinDDKalTest - SET missing in gear file: SET Not Built " << std::endl ;
    //   }


    //   bool FTD_found = false ;
    //   try{
    //     ILDFTDKalDetector* ftddet = new ILDFTDKalDetector( *_gearMgr )  ;
    //     // store the measurement layer id's for the active layers 
    //     this->storeActiveMeasurementModuleIDs(ftddet);
    //     _det->Install( *ftddet ) ;    
    //     FTD_found = true ;
    //   }
    //   catch( gear::UnknownParameterException& e){
    //     streamlog_out( MESSAGE ) << "  MarlinDDKalTest - Petal Based FTD missing in gear file: Petal Based FTD Not Built " << std::endl ;
    //   }

    //   if( ! FTD_found ){
    //     try{
    //       ILDFTDDiscBasedKalDetector* ftddet = new ILDFTDDiscBasedKalDetector( *_gearMgr )  ;
    //       // store the measurement layer id's for the active layers
    //       this->storeActiveMeasurementModuleIDs(ftddet);
    //       _det->Install( *ftddet ) ;
    //     }
    //     catch( gear::UnknownParameterException& e){
    //       streamlog_out( MESSAGE ) << "  MarlinDDKalTest - Simple Disc Based FTD missing in gear file: Simple Disc Based FTD Not Built " << std::endl ;
    //     }
    //   }

    //   try{
    //     ILDTPCKalDetector* tpcdet = new ILDTPCKalDetector( *_gearMgr )  ;
    //     // store the measurement layer id's for the active layers
    //     this->storeActiveMeasurementModuleIDs(tpcdet);
    //     _det->Install( *tpcdet ) ;
    //   }
    //   catch( gear::UnknownParameterException& e){   
    //     streamlog_out( MESSAGE ) << "  MarlinDDKalTest - TPC missing in gear file: TPC Not Built " << std::endl ;
    //   }
    // }

    _det->Close() ;          // close the cradle
    _det->Sort() ;           // sort meas. layers from inside to outside
    
    streamlog_out( DEBUG4 ) << "  MarlinDDKalTest - number of layers = " << _det->GetEntriesFast() << std::endl ;
    
    streamlog_out( DEBUG4 ) << "Options: " << std::endl << this->getOptions() << std::endl ;
    
    this->includeMultipleScattering( getOption(IMarlinTrkSystem::CFG::useQMS) ) ;  
    this->includeEnergyLoss( getOption(IMarlinTrkSystem::CFG::usedEdx) ) ; 
    
    
    is_initialised = true; 
    
  }
  
  MarlinTrk::IMarlinTrack* MarlinDDKalTest::createTrack()  {
    
    if ( ! is_initialised ) {
      
      std::stringstream errorMsg;
      errorMsg << "MarlinDDKalTest::createTrack: Fitter not initialised. MarlinDDKalTest::init() must be called before MarlinDDKalTest::createTrack()" << std::endl ; 
      throw MarlinTrk::Exception(errorMsg.str());
      
    }
    
    return new MarlinDDKalTestTrack(this) ;
    
  }
  
  void MarlinDDKalTest::includeMultipleScattering( bool msOn ) {
    
    if( msOn == true ) {
      _det->SwitchOnMS();
    }
    else{
      _det->SwitchOffMS();
    } 
    
  } 
  
  void MarlinDDKalTest::includeEnergyLoss( bool energyLossOn ) {
    
    if( energyLossOn == true ) {
      _det->SwitchOnDEDX();
    }
    else{
      _det->SwitchOffDEDX();
    } 
    
  } 
  
  void MarlinDDKalTest::getSensitiveMeasurementModulesForLayer( int layerID, std::vector< const DDVMeasLayer *>& measmodules) const {
    
    if( ! measmodules.empty() ) {
      
      std::stringstream errorMsg;
      errorMsg << "MarlinDDKalTest::getSensitiveMeasurementModulesForLayer vector passed as second argument is not empty " << std::endl ; 
      throw MarlinTrk::Exception(errorMsg.str());
      
    }
    
    streamlog_out( DEBUG0 ) << "MarlinDDKalTest::getSensitiveMeasurementModulesForLayer: layerID = " << layerID << std::endl;
    
    std::multimap<Int_t, const DDVMeasLayer *>::const_iterator it; //Iterator to be used along with ii
    
    
    
    //  for(it = _active_measurement_modules_by_layer.begin(); it != _active_measurement_modules_by_layer.end(); ++it) {
    //    streamlog_out( DEBUG0 ) << "Key = "<< ttdecodeILD(it->first) <<"    Value = "<<it->second << std::endl ;
    //  }
    
    
    std::pair<std::multimap<Int_t, const DDVMeasLayer *>::const_iterator, std::multimap<Int_t, const DDVMeasLayer *>::const_iterator> ii;  
    
    // set the module and sensor bit ranges to zero as these are not used in the map 
    lcio::BitField64 bf(  UTIL::ILDCellID0::encoder_string ) ;
    bf.setValue( layerID ) ;
    bf[lcio::ILDCellID0::module] = 0 ;
    bf[lcio::ILDCellID0::sensor] = 0 ;
    layerID = bf.lowWord();
    
    ii = this->_active_measurement_modules_by_layer.equal_range(layerID); // set the first and last entry in ii;
    
    for(it = ii.first; it != ii.second; ++it) {
      //    streamlog_out( DEBUG0 ) <<"Key = "<< it->first <<"    Value = "<<it->second << std::endl ;
      measmodules.push_back( it->second ) ; 
    }
    
  }
  
  void MarlinDDKalTest::getSensitiveMeasurementModules( int moduleID , std::vector< const DDVMeasLayer *>& measmodules ) const {
    
    if( ! measmodules.empty() ) {
      
      std::stringstream errorMsg;
      errorMsg << "MarlinDDKalTest::getSensitiveMeasurementLayer vector passed as second argument is not empty " << std::endl ; 
      throw MarlinTrk::Exception(errorMsg.str());
      
    }
    
    std::pair<std::multimap<int, const DDVMeasLayer *>::const_iterator, std::multimap<Int_t, const DDVMeasLayer *>::const_iterator> ii;
    ii = this->_active_measurement_modules.equal_range(moduleID); // set the first and last entry in ii;
    
    std::multimap<int,const DDVMeasLayer *>::const_iterator it; //Iterator to be used along with ii
    
    
    for(it = ii.first; it != ii.second; ++it) {
      //      std::cout<<"Key = "<<it->first<<"    Value = "<<it->second << std::endl ;
      measmodules.push_back( it->second ) ; 
    }
  }
  
  
  void MarlinDDKalTest::storeActiveMeasurementModuleIDs(TVKalDetector* detector) {
    
    Int_t nLayers = detector->GetEntriesFast() ;
    
    for( int i=0; i < nLayers; ++i ) {
      
      const DDVMeasLayer* ml = dynamic_cast<const DDVMeasLayer*>( detector->At( i ) ); 
      
      if( ! ml ) {
        std::stringstream errorMsg;
        errorMsg << "MarlinDDKalTest::storeActiveMeasurementLayerIDs dynamic_cast to DDVMeasLayer* failed " << std::endl ; 
        throw MarlinTrk::Exception(errorMsg.str());
      }
      
      if( ml->IsActive() ) {
        
        // then get all the sensitive element id's assosiated with this DDVMeasLayer and store them in the map 
        std::vector<int>::const_iterator it = ml->getCellIDs().begin();
        
        while ( it!=ml->getCellIDs().end() ) {
          
          int sensitive_element_id = *it;
          this->_active_measurement_modules.insert(std::pair<int,const DDVMeasLayer*>( sensitive_element_id, ml ));        
          ++it;
          
        }
        
        int subdet_layer_id = ml->getLayerID() ;
        
        this->_active_measurement_modules_by_layer.insert(std::pair<int ,const DDVMeasLayer*>(subdet_layer_id,ml));
        
        streamlog_out(DEBUG0) << "MarlinDDKalTest::storeActiveMeasurementLayerIDs added active layer with "
        << " LayerID = " << subdet_layer_id << " and DetElementIDs  " ;
        
        for (it = ml->getCellIDs().begin(); it!=ml->getCellIDs().end(); ++it) {
          
          streamlog_out(DEBUG0) << " : " << *it ;
          
        }
        
        streamlog_out(DEBUG0) << std::endl;
        
        
        
        
      }
      
    }
    
  }
  
  const DDVMeasLayer*  MarlinDDKalTest::getLastMeasLayer(THelicalTrack const& hel, TVector3 const& point) const {
    
    THelicalTrack helix = hel;
    
    double deflection_to_point = 0 ;
    helix.MoveTo(  point, deflection_to_point , 0 , 0) ;
    
    bool isfwd = ((helix.GetKappa() > 0 && deflection_to_point < 0) || (helix.GetKappa() <= 0 && deflection_to_point > 0)) ? true : false;
    
    int mode = isfwd ? -1 : +1 ;
    
    //  streamlog_out( DEBUG4 ) << "  MarlinDDKalTest - getLastMeasLayer deflection to point = " << deflection_to_point << " kappa = " << helix.GetKappa()  << "  mode = " << mode << std::endl ;
    //  streamlog_out( DEBUG4 ) << " Point to move to:" << std::endl;
    //  point.Print();
    
    int nsufaces =  _det->GetEntriesFast();
    
    const DDVMeasLayer* ml_retval = 0;
    double min_deflection = DBL_MAX;
    
    for(int i=0; i<nsufaces; ++i) {
      
      const DDVMeasLayer   &ml  = *dynamic_cast< const DDVMeasLayer *>(_det->At(i)); 
      
      double defection_angle = 0 ;
      TVector3 crossing_point ;   
      
      const TVSurface *sfp = dynamic_cast<const TVSurface *>(&ml);  // surface at destination       
      
      
      int does_cross = sfp->CalcXingPointWith(helix, crossing_point, defection_angle, mode) ;
      
      if( does_cross ) {
        
        const double deflection = fabs( deflection_to_point - defection_angle ) ;
        
        if( deflection < min_deflection ) {
          
          //      streamlog_out( DEBUG4 ) << "  MarlinDDKalTest - crossing found for suface = " << ml.GetMLName() 
          //                              << std::endl
          //                              << "  min_deflection = " << min_deflection
          //                              << "  deflection = " << deflection
          //                              << "  deflection angle = " << defection_angle 
          //                              << std::endl 
          //                              << " x = " << crossing_point.X() 
          //                              << " y = " << crossing_point.Y() 
          //                              << " z = " << crossing_point.Z() 
          //                              << " r = " << crossing_point.Perp() 
          //                              << std::endl ;
          
          min_deflection = deflection ;
          ml_retval = &ml ;
        }
        
      }
      
    }
    
    return ml_retval;
  }
  
  const DDVMeasLayer* MarlinDDKalTest::findMeasLayer( EVENT::TrackerHit * trkhit) const {
    
    const TVector3 hit_pos( trkhit->getPosition()[0], trkhit->getPosition()[1], trkhit->getPosition()[2]) ;
    
    return this->findMeasLayer( trkhit->getCellID0(), hit_pos ) ;
    
  }
  
  const DDVMeasLayer* MarlinDDKalTest::findMeasLayer( int detElementID, const TVector3& point) const {
    
    const DDVMeasLayer* ml = 0; // return value 
    
    std::vector<const DDVMeasLayer*> meas_modules ;
    
    // search for the list of measurement layers associated with this CellID
    this->getSensitiveMeasurementModules( detElementID, meas_modules ) ; 
    
    if( meas_modules.size() == 0 ) { // no measurement layers found 
      
      UTIL::BitField64 encoder( UTIL::ILDCellID0::encoder_string ) ; 
      encoder.setValue(detElementID) ;
      
      std::stringstream errorMsg;
      errorMsg << "MarlinDDKalTest::findMeasLayer module id unkown: moduleID = " << detElementID 
	       << " [" << encoder.valueString() << "]" << std::endl ; 
       throw MarlinTrk::Exception(errorMsg.str());
      
    } 
    else if (meas_modules.size() == 1) { // one to one mapping 
      
      ml = meas_modules[0] ;
      
    }
    else { // layer has been split 
      
      bool surf_found(false);
      
      // loop over the measurement layers associated with this CellID and find the correct one using the position of the hit
      for( unsigned int i=0; i < meas_modules.size(); ++i) {
        
        
        
        const TVSurface* surf = 0;
        
        if( ! (surf = dynamic_cast<const TVSurface*> (  meas_modules[i] )) ) {
          std::stringstream errorMsg;
          errorMsg << "MarlinDDKalTest::findMeasLayer dynamic_cast failed for surface type: moduleID = " << detElementID << std::endl ; 
          throw MarlinTrk::Exception(errorMsg.str());
        }
        
        bool hit_on_surface = surf->IsOnSurface(point);
        
        if( (!surf_found) && hit_on_surface ){
          
          ml = meas_modules[i] ;
          surf_found = true ;
          
        }
        else if( surf_found && hit_on_surface ) {  // only one surface should be found, if not throw 
          
          std::stringstream errorMsg;
          errorMsg << "MarlinDDKalTest::findMeasLayer point found to be on two surfaces: moduleID = " << detElementID << std::endl ; 
          throw MarlinTrk::Exception(errorMsg.str());
        }      
        
      }
      if( ! surf_found ){ // print out debug info
        streamlog_out(DEBUG1) << "MarlinDDKalTest::findMeasLayer point not found to be on any surface matching moduleID = "
        << detElementID
        << ": x = " << point.x()
        << " y = " << point.y()
        << " z = " << point.z()
        << std::endl ;
      }
      else{
        streamlog_out(DEBUG1) << "MarlinDDKalTest::findMeasLayer point found to be on surface matching moduleID = "
        << detElementID
        << ": x = " << point.x()
        << " y = " << point.y()
        << " z = " << point.z()
        << std::endl ;
      }
    }
    
    return ml ;
    
  }
  
} // end of namespace MarlinTrk