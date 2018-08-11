#include "WaveChango.h"
#include "Wave.h"
#include "tones.h"
#include <stdio.h>

static double tones[NUM_WAVES] = { C3, D3, E3, F3, G3, 
	                           A3, B3, C4, D4, E4, 
				   F4, G4, A4, B4, C5, 
				   D5, E5, F5, G5, A5, 
				   B5, C6, D6, E6, F6};

extern float amplitude_threshold;

WaveChango::WaveChango(Mahalo *M){

  this->m = M;

  SampleMixer *s = new SampleMixer();

  for ( int i = 0; i < NUM_WAVES; i++ ){

    Wave *b = new Wave(tones[i], this->m->getRate(), 0.0); 
    this->srcs[i] = b;
    s->Add(b);

  }
 
  m->setSampleSource(s); 

}

WaveChango::~WaveChango(){
    
}


void WaveChango::tune(long which, float freq){

    ((Wave*)this->srcs[which])->setFreqVal( freq );
    
}

void WaveChango::update(float *vals){

  for( int i = 0; i < NUM_WAVES; i++ ){
   
    if( vals[i] > amplitude_threshold ){
      this->srcs[i]->setAmpVal( vals[i] );
    }else{
      this->srcs[i]->setAmpVal( 0.0f );
    }
  }
  
}
