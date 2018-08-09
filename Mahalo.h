#ifndef _SIN_GEN_H_
#define _SIN_GEN_H_
/*
 *  Mahalo.h
 *  Mahalo -- 
 *  Multi-platform Audio Hardware Abstraction Layer (for Output) 
 *
 *  Created by blucia0a on 8/25/11.
 *  Copyright 2011-2018 Brandon Lucia. All rights reserved.
 *
 */

#ifdef __APPLE__
#include <CoreAudio/AudioHardware.h>
#else
#define Boolean bool
#endif
/*Currently no audio support outside of Apple...*/

/*
Unsure why these are here... BML 8/9/18
#include "Wave.h"
#include "tones.h"
*/

#include "SampleSource.h"

class Mahalo {

public:	

#ifdef __APPLE__	
  AudioDeviceID	device;		// the default device
  UInt32	deviceBufferSize;	// bufferSize returned by kAudioDevicePropertyBufferSize
  AudioStreamBasicDescription	deviceFormat;	// info about the default device
  double pan, panz;
#endif
  Boolean		initialized;	// successful init?
  Boolean		soundPlaying;	// playing now?
  SampleSource *src;
  
  Mahalo();
  void setup();
  bool sstart();
  bool sstop();
  void setSampleSource(SampleSource *s);
  float getRate();
	
	
};

#endif
