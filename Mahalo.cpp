/*
 *  Mahalo.cpp
 *  Sin
 *
 *  Created by blucia0a on 8/25/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Mahalo.h"
bool verbose = true;

#ifdef __APPLE__
OSStatus sappIOProc (AudioDeviceID  inDevice, const AudioTimeStamp*  inNow, const AudioBufferList*   inInputData, 
           const AudioTimeStamp*  inInputTime, AudioBufferList*  outOutputData, const AudioTimeStamp* inOutputTime, 
           void* defptr)
{    
    Mahalo *def = (Mahalo*)defptr; 
    int i;
    
    int numSamples = def->deviceBufferSize / def->deviceFormat.mBytesPerFrame;
    
    float *out = (float*)outOutputData->mBuffers[0].mData;
    if( out == NULL ){ return kAudioHardwareNoError;}
    
    for (i=0; i<numSamples; ++i) {
        
      if( def != NULL && def->src != NULL ){
        def->src->getNextSample(out);
      }else{
        out[0] = 0;
        out[1] = 0;
      }
      
      out[0] = out[0] * (1.0f - (float)def->panz);
      out[1] = out[1] * (float)def->panz;
      
      //move the output pointer ahead in the hardware buffer
      out++; out++;
      
      //update the zippered pan value
      def->panz  = 0.001 * def->pan  + 0.999 * def->panz;
        
    }
    
    return kAudioHardwareNoError;     
}

/*Mahalo Implementation*/
Mahalo::Mahalo(){
    src = NULL;
}

float Mahalo::getRate(){
  return (float)this->deviceFormat.mSampleRate;
}

void Mahalo::setSampleSource(SampleSource *s){
  src = s;
}

void Mahalo::setup(void){
  
    OSStatus      err = kAudioHardwareNoError;
    UInt32        count;    
    device = kAudioDeviceUnknown;
  
    initialized = false;
  
    // get the default output device for the HAL
    count = sizeof(device);    // it is required to pass the size of the data to be returned
    err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,  &count, (void *) &device);
    if (err != kAudioHardwareNoError) {
      fprintf(stderr, "get kAudioHardwarePropertyDefaultOutputDevice error %d\n", err);
        return;
    }
    
    // get the buffersize that the default device uses for IO
    count = sizeof(deviceBufferSize);  // it is required to pass the size of the data to be returned
    err = AudioDeviceGetProperty(device, 0, false, kAudioDevicePropertyBufferSize, &count, &deviceBufferSize);
    if (err != kAudioHardwareNoError) {
      fprintf(stderr, "get kAudioDevicePropertyBufferSize error %d\n", err);
        return;
    }
    fprintf(stderr, "deviceBufferSize = %d\n", deviceBufferSize);
  
    // get a description of the data format used by the default device
    count = sizeof(deviceFormat);  // it is required to pass the size of the data to be returned
    err = AudioDeviceGetProperty(device, 0, false, kAudioDevicePropertyStreamFormat, &count, &deviceFormat);
    if (err != kAudioHardwareNoError) {
      fprintf(stderr, "get kAudioDevicePropertyStreamFormat error %d\n", err);
        return;
    }
    if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
      fprintf(stderr, "mFormatID !=  kAudioFormatLinearPCM\n");
        return;
    }
    if (!(deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat)) {
      fprintf(stderr, "Sorry, currently only works with float format....\n");
        return;
    }
    
    initialized = true;

    this->panz = this->pan = 0.5;
  
    fprintf(stderr, "mSampleRate = %g\n", deviceFormat.mSampleRate);
    fprintf(stderr, "mFormatFlags = %08X\n", deviceFormat.mFormatFlags);
    fprintf(stderr, "mBytesPerPacket = %d\n", deviceFormat.mBytesPerPacket);
    fprintf(stderr, "mFramesPerPacket = %d\n", deviceFormat.mFramesPerPacket);
    fprintf(stderr, "mChannelsPerFrame = %d\n", deviceFormat.mChannelsPerFrame);
    fprintf(stderr, "mBytesPerFrame = %d\n", deviceFormat.mBytesPerFrame);
    fprintf(stderr, "mBitsPerChannel = %d\n", deviceFormat.mBitsPerChannel);

}


bool Mahalo::sstart(){
  
    OSStatus    err = kAudioHardwareNoError;
  
    if (!initialized) return false;
    if (soundPlaying) return false;
    
    err = AudioDeviceAddIOProc(device, sappIOProc, (void *) this);  // setup our device with an IO proc
    if (err != kAudioHardwareNoError) return false;
    
    err = AudioDeviceStart(device, sappIOProc);        // start playing sound through the device
    if (err != kAudioHardwareNoError) return false;
  
    soundPlaying = true;            // set the playing status global to true
    return true;

}

bool Mahalo::sstop(){
  
    OSStatus   err = kAudioHardwareNoError;
    
    if (!initialized) return false;
    if (!soundPlaying) return false;
    
    err = AudioDeviceStop(device, sappIOProc);        // stop playing sound through the device
    if (err != kAudioHardwareNoError) return false;
  
    err = AudioDeviceRemoveIOProc(device, sappIOProc);      // remove the IO proc from the device
    if (err != kAudioHardwareNoError) return false;
    
    soundPlaying = false;            // set the playing status global to false
    return true;
  
}

#elif __linux__  /*end: __APPLE__; begin: __linux__*/
  /*Hacky quit function*/
  static void quit(int ret) {
    //mainloop_api->quit(mainloop_api, ret);
  }

  /* UNIX signal to quit recieved */
  static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata) {
    fprintf(stderr, "Got signal, exiting.\n");
    quit(0);
  }


/* This is called whenever new data may be written to the stream */
  static void stream_write_callback(pa_stream *s, size_t length, void *userdata) {


    fprintf(stderr,"trying to write some data\n");
    /*Check basic stream properties*/
    assert(s);
    assert(length > 0);


    void *audiobuffer = NULL;
    size_t len = -1;
    if (pa_stream_begin_write(s, &audiobuffer, &len) < 0) {
      fprintf(stderr, "pa_stream_begin_write() failed: %s\n", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
      quit(1);
      return;
    }
    

    Mahalo *def = (Mahalo*)userdata; 
    float *out = (float *)audiobuffer;
    int i;
    int numSamples = len / (pa_frame_size(&(def->sample_spec)));


    assert(def);
    assert(def->src);
    assert(out);
    assert(numSamples > 0);
    
    
    for (i=0; i<numSamples; ++i) {
        
      if( def != NULL && def->src != NULL ){

        def->src->getNextSample(out);
        

      }else{

        out[0] = 0;
        out[1] = 0;

      }
      
      out[0] = out[0] * (1.0f - (float)def->panz);
      out[1] = out[1] * (float)def->panz;
      
      //move the output pointer ahead in the hardware buffer
      out++; out++;
      
      //update the zippered pan value
      def->panz  = 0.001 * def->pan  + 0.999 * def->panz;
        
    }

    if (pa_stream_write(s, (uint8_t*) audiobuffer, len, NULL, 0, PA_SEEK_RELATIVE) < 0) {
        fprintf(stderr, "pa_stream_write() failed: %s\n", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
        quit(1);
        return;
    }
    
    fprintf(stderr,"done writing the data\n");

}

/* This routine is called whenever the stream state changes */
  static void stream_state_callback(pa_stream *s, void *userdata) {
    assert(s);
    Mahalo *m = (Mahalo *)userdata;

    switch (pa_stream_get_state(s)) {
        case PA_STREAM_CREATING:
        case PA_STREAM_TERMINATED:
            break;

        case PA_STREAM_READY:
                const pa_buffer_attr *a;
                char cmt[PA_CHANNEL_MAP_SNPRINT_MAX], sst[PA_SAMPLE_SPEC_SNPRINT_MAX];

                fprintf(stderr, "Stream successfully created.\n");

                if (!(a = pa_stream_get_buffer_attr(s)))
                    fprintf(stderr, "pa_stream_get_buffer_attr() failed: %s\n", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
                else {
                  fprintf(stderr, "Buffer metrics: maxlength=%u, tlength=%u, prebuf=%u, minreq=%u\n", a->maxlength, a->tlength, a->prebuf, a->minreq);
                }

                fprintf(stderr, "Using sample spec '%s', channel map '%s'.\n",
                        pa_sample_spec_snprint(sst, sizeof(sst), pa_stream_get_sample_spec(s)),
                        pa_channel_map_snprint(cmt, sizeof(cmt), pa_stream_get_channel_map(s)));

                fprintf(stderr, "Connected to device %s (%u, %ssuspended).\n",
                        pa_stream_get_device_name(s),
                        pa_stream_get_device_index(s),
                        pa_stream_is_suspended(s) ? "" : "not ");

                fprintf(stderr,"Initialized is now true.\n"); 
                m->initialized = true;

            break;

        case PA_STREAM_FAILED:
        default:
            fprintf(stderr, "Stream error: %s\n", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
            quit(1);
    }
  }

  static void stream_suspended_callback(pa_stream *s, void *userdata) {
    assert(s);

    if (verbose) {
        if (pa_stream_is_suspended(s))
            fprintf(stderr, "Stream device suspended.%s \n", CLEAR_LINE);
        else
            fprintf(stderr, "Stream device resumed.%s \n", CLEAR_LINE);
    }
}

  static void stream_underflow_callback(pa_stream *s, void *userdata) {
    assert(s);

    if (verbose)
        fprintf(stderr, "Stream underrun.%s \n",  CLEAR_LINE);
}

  static void stream_overflow_callback(pa_stream *s, void *userdata) {
    assert(s);

    if (verbose)
        fprintf(stderr, "Stream overrun.%s \n", CLEAR_LINE);
}

  static void stream_started_callback(pa_stream *s, void *userdata) {
    assert(s);

    if (verbose)
        fprintf(stderr, "Stream started.%s \n", CLEAR_LINE);
}

  static void stream_moved_callback(pa_stream *s, void *userdata) {
    assert(s);

    if (verbose)
        fprintf(stderr, "Stream moved to device %s (%u, %ssuspended).%s \n", pa_stream_get_device_name(s), pa_stream_get_device_index(s), pa_stream_is_suspended(s) ? "" : "not ",  CLEAR_LINE);
}

  static void stream_buffer_attr_callback(pa_stream *s, void *userdata) {
    assert(s);

    if (verbose)
        fprintf(stderr, "Stream buffer attributes changed.%s \n",  CLEAR_LINE);
}

  static void stream_event_callback(pa_stream *s, const char *name, pa_proplist *pl, void *userdata) {
    char *t;

    assert(s);
    assert(name);
    assert(pl);

    t = pa_proplist_to_string_sep(pl, ", ");
    fprintf(stderr, "Got event '%s', properties '%s'\n", name, t);
    pa_xfree(t);
}

/* This is called whenever the context status changes */
  static void context_state_callback(pa_context *c, void *userdata) {

    assert(c);

    Mahalo *m = (Mahalo *)userdata; 
    assert(m);
    fprintf(stderr,"In the context state callback\n");

    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_CONNECTING: {
            fprintf(stderr,"connecting!\n");
            break;
        }
        case PA_CONTEXT_AUTHORIZING: {
            fprintf(stderr,"authorizing!\n");
            break;
        }
        case PA_CONTEXT_SETTING_NAME: {
            fprintf(stderr,"setting name!\n");
            break;
        }
        case PA_CONTEXT_READY: {
            fprintf(stderr,"Ready!\n");
            int r;
            pa_buffer_attr buffer_attr;

            assert(c);
            assert(!m->stream);

            if (verbose)
                fprintf(stderr, "Connection established.%s \n", CLEAR_LINE);

            if (!(m->stream = pa_stream_new(c, m->stream_name, &(m->sample_spec), m->channel_map_set ? &(m->channel_map) : NULL))) {
                fprintf(stderr, "pa_stream_new() failed: %s\n", pa_strerror(pa_context_errno(c)));
                exit(1);
            }

            fprintf(stderr,"Context ready.  setting callbacks\n");
            pa_stream_set_state_callback(m->stream, stream_state_callback, m);
            pa_stream_set_write_callback(m->stream, stream_write_callback, m);
            pa_stream_set_suspended_callback(m->stream, stream_suspended_callback, NULL);
            pa_stream_set_moved_callback(m->stream, stream_moved_callback, NULL);
            pa_stream_set_underflow_callback(m->stream, stream_underflow_callback, NULL);
            pa_stream_set_overflow_callback(m->stream, stream_overflow_callback, NULL);
            pa_stream_set_started_callback(m->stream, stream_started_callback, NULL);
            pa_stream_set_event_callback(m->stream, stream_event_callback, NULL);
            pa_stream_set_buffer_attr_callback(m->stream, stream_buffer_attr_callback, NULL);

            if (m->latency > 0) {
                memset(&buffer_attr, 0, sizeof(buffer_attr));
                buffer_attr.tlength = (uint32_t) m->latency;
                buffer_attr.minreq = (uint32_t) m->process_time;
                buffer_attr.maxlength = (uint32_t) -1;
                buffer_attr.prebuf = (uint32_t) -1;
                buffer_attr.fragsize = (uint32_t) m->latency;
                m->flags = (pa_stream_flags_t)((uint32_t (m->flags)) | PA_STREAM_ADJUST_LATENCY);
            }

            pa_cvolume cv;
            if ((r = pa_stream_connect_playback(m->stream, m->device, m->latency > 0 ? &buffer_attr : NULL, m->flags, m->volume_is_set ? pa_cvolume_set(&cv, m->sample_spec.channels, m->volume) : NULL, NULL)) < 0) {
              fprintf(stderr, "pa_stream_connect_playback() failed: %s\n", pa_strerror(pa_context_errno(c)));
              exit(1);
            }


            break;
        }

        case PA_CONTEXT_TERMINATED:
            quit(0);
            break;

        case PA_CONTEXT_FAILED:
        default:
            fprintf(stderr, "Connection failure: %s\n", pa_strerror(pa_context_errno(c)));
            exit(1);
    }

    return;

fail:
    quit(1);

}


  Mahalo::Mahalo(){

    mode = PLAYBACK; 
    context = NULL;
    stream = NULL;
    mainloop_api = NULL;

    buffer = NULL;
    buffer_length = 0; 
    buffer_index = 0;

    stdio_event = NULL;

    stream_name = NULL; 
    client_name = NULL; 
    device = NULL;

    verbose = 1;
    volume = PA_VOLUME_NORM;
    volume_is_set = 1;

    sample_spec = {
      /*Default: 44kHz stereo 16 bit signed little endian audio data*/
      .format = PA_SAMPLE_FLOAT32LE,
      .rate = 44100,
      .channels = 2
    };

    channel_map_set = 0;

    flags = (pa_stream_flags_t)0;

    latency = 0; 
    process_time=0;
   
    initialized = false;
    soundPlaying = false;
     
    
  }

  void Mahalo::setup(){


    pa_mainloop *m;
    int ret = 1, r, c;
    char *bn, *server = NULL;
    pa_time_event *time_event = NULL;

    if (!pa_sample_spec_valid(&sample_spec)) {
      fprintf(stderr, "Invalid sample specification\n");
      quit(-1);
    }

    if (channel_map_set && 
        pa_channel_map_compatible(&channel_map, &sample_spec)) {
      fprintf(stderr, "Channel map doesn't match sample specification\n");
      quit(-1);
    }
    
    client_name = pa_xstrdup("Chango");
    stream_name = pa_xstrdup("Chango");
    
    /* Set up a new main loop */
    if (!(m = pa_mainloop_new())) {
        fprintf(stderr, "pa_mainloop_new() failed.\n");
        quit(-1);
    }
    this->pml = m;
    mainloop_api = pa_mainloop_get_api(m);
  
    /*Set up crash / signal handles*/  
    r = pa_signal_init(mainloop_api);
    assert(r == 0);
    
    /* Create a new connection context */
    if (!(context = pa_context_new(mainloop_api, client_name))) {
        fprintf(stderr, "pa_context_new() failed.\n");
        quit(-1);
    }

    pa_context_set_state_callback(context, context_state_callback, this);

    /* Connect the context */
    if (pa_context_connect(context, server, (pa_context_flags_t)0, NULL) < 0) {
        fprintf(stderr, "pa_context_connect() failed: %s\n", pa_strerror(pa_context_errno(context)));
        quit(-1);
    }
    

  }

  bool Mahalo::sstart(){

    soundPlaying = true;            // set the playing status global to true
    return true;

  }

  bool Mahalo::sstop(){

    if (!initialized){return false;}
    if (!soundPlaying){return false;}

    soundPlaying = false;            // set the playing status global to false
    return true;

  }

  void Mahalo::setSampleSource(SampleSource *s){
    src = s;
  }

  float Mahalo::getRate(){
    return 0;
  }

  void Mahalo::step_mainloop(){

    int ret = 1;
    if (pa_mainloop_run(this->pml, &ret) < 0) {
        fprintf(stderr, "pa_mainloop_run() failed.\n");
        exit(1);
    }

  }


#else
#error "Windows or Unknown OS not supported"
#endif
