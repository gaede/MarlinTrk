#ifndef MarlinTrkDiagnostics_h
#define MarlinTrkDiagnostics_h 

//// switch to turn on diagnostics code
//#define MARLINTRK_DIAGNOSTICS_ON 

#ifdef MARLINTRK_DIAGNOSTICS_ON

#include "lcio.h"
#include "EVENT/SimTrackerHit.h"


namespace MarlinTrk{

  
  // LCIO Extension creating a pointer to the simhit for trackerhits 
  struct MCTruth4HitExtStruct{
    MCTruth4HitExtStruct() : simhit(0) {}
    EVENT::SimTrackerHit* simhit;
  } ; 
  struct MCTruth4HitExt : lcio::LCOwnedExtension<MCTruth4HitExt, MCTruth4HitExtStruct> {} ;
  
  
}

#endif

#endif
