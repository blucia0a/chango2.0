#ifndef _MAHALO_H_
#define _MAHALO_H_
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
#elif __linux__
#define Boolean bool
#include <pulse/pulseaudio.h>
#include <pulse/rtclock.h>
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
#define CLEAR_LINE "\x1B[K"



class Mahalo {

public:	

#ifdef __APPLE__	
  AudioDeviceID	device;		// the default device
  UInt32	deviceBufferSize;	// bufferSize returned by kAudioDevicePropertyBufferSize
  AudioStreamBasicDescription	deviceFormat;	// info about the default device
#elif __linux__
  enum { RECORD, PLAYBACK } mode; 

  pa_context *context;
  pa_stream *stream;
  pa_threaded_mainloop *pml;
  pa_mainloop_api *mainloop_api;

  void *buffer;
  size_t buffer_length, buffer_index;

  pa_io_event* stdio_event;

  char *stream_name, *client_name, *device;

  int verbose;
  pa_volume_t volume;
  int volume_is_set;

  pa_sample_spec sample_spec;

  pa_channel_map channel_map;
  int channel_map_set;

  pa_stream_flags_t flags;

  size_t latency, process_time;
  
  void step_mainloop();

#endif

  volatile Boolean		initialized;	// successful init?
  Boolean		soundPlaying;	// playing now?
  SampleSource *src;
  double pan, panz;
  
  Mahalo();
  void setup();
  bool sstart();
  bool sstop();
  void setSampleSource(SampleSource *s);
  float getRate();
	
	
};

#endif
