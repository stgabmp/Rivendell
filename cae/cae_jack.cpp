// cae_jack.cpp
//
// The JACK Driver for the Core Audio Engine component of Rivendell
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: cae_jack.cpp,v 1.50.2.1 2009/07/01 23:18:58 cvs Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <math.h>

#include <samplerate.h>

#include <qsignalmapper.h>

#include <rd.h>
#include <rdringbuffer.h>
#include <rdprofile.h>
#include <rdmeteraverage.h>

#include <cae.h>
#include <rdcae.h>

#ifdef JACK
//
// Callback Variables
//
jack_client_t *jack_client;
RDMeterAverage *jack_input_meter[RD_MAX_PORTS][2];
RDMeterAverage *jack_output_meter[RD_MAX_PORTS][2];
//volatile jack_default_audio_sample_t jack_input_meter[RD_MAX_PORTS][2];
//volatile jack_default_audio_sample_t jack_output_meter[RD_MAX_PORTS][2];
volatile jack_default_audio_sample_t 
  jack_input_volume[RD_MAX_PORTS];
volatile jack_default_audio_sample_t 
  jack_output_volume[RD_MAX_PORTS][RD_MAX_STREAMS];
volatile jack_default_audio_sample_t
  jack_passthrough_volume[RD_MAX_PORTS][RD_MAX_PORTS];
volatile jack_default_audio_sample_t jack_input_vox[RD_MAX_PORTS];
jack_port_t *jack_input_port[RD_MAX_PORTS][2];
jack_port_t *jack_output_port[RD_MAX_PORTS][2];
volatile int jack_input_channels[RD_MAX_PORTS];
volatile int jack_output_channels[RD_MAX_STREAMS];
volatile jack_default_audio_sample_t *jack_input_buffer[RD_MAX_PORTS][2];
volatile jack_default_audio_sample_t *jack_output_buffer[RD_MAX_PORTS][2];
RDRingBuffer *jack_play_ring[RD_MAX_STREAMS];
RDRingBuffer *jack_record_ring[RD_MAX_PORTS];
volatile bool jack_playing[RD_MAX_STREAMS];
volatile bool jack_stopping[RD_MAX_STREAMS];
volatile bool jack_eof[RD_MAX_STREAMS];
volatile bool jack_recording[RD_MAX_PORTS];
volatile bool jack_ready[RD_MAX_PORTS];
volatile int jack_output_pos[RD_MAX_STREAMS];
volatile unsigned jack_output_sample_rate[RD_MAX_STREAMS];
volatile unsigned jack_sample_rate;
int jack_input_mode[RD_MAX_CARDS][RD_MAX_PORTS];
int jack_card_process;  // local copy of object member jack_card, for use by the callback process.


//
// Callback Buffers
//
jack_default_audio_sample_t jack_callback_buffer[RINGBUFFER_SIZE];

int JackProcess(jack_nframes_t nframes, void *arg)
{
  unsigned n=0;
  jack_default_audio_sample_t in_meter[2];
  jack_default_audio_sample_t out_meter[2];

  //
  // Ensure Buffers are Valid
  //
  for(int i=0;i<RD_MAX_PORTS;i++) {
    for(int j=0;j<2;j++) {
      jack_input_buffer[i][j]=(jack_default_audio_sample_t *)
	jack_port_get_buffer(jack_input_port[i][j],nframes);
      jack_output_buffer[i][j]=(jack_default_audio_sample_t *)
	jack_port_get_buffer(jack_output_port[i][j],nframes);
    }
  }

  //
  // Zero Output Ports
  //
  for(int i=0;i<RD_MAX_PORTS;i++) {
    for(int j=0;j<2;j++) {
      for(unsigned k=0;k<nframes;k++) {
	jack_output_buffer[i][j][k]=0.0;
      }
    } 
  }

  //
  // Process Passthroughs
  //
  for(int i=0;i<RD_MAX_PORTS;i++) {
    for(int j=0;j<RD_MAX_PORTS;j++) {
      if(jack_passthrough_volume[i][j]>0.0) {
	for(int k=0;k<2;k++) {
	  for(unsigned l=0;l<nframes;l++) {
	    jack_output_buffer[j][k][l]+=
	      jack_input_buffer[i][k][l]*jack_passthrough_volume[i][j];
	  }
	}
      }
    }
  }

  //
  // Process Input Streams
  //
  for(int i=0;i<RD_MAX_PORTS;i++) {
    if(jack_recording[i]) {
      switch(jack_input_channels[i]) {
          case 1: // mono
            for(unsigned j=0;j<nframes;j++) {
              switch(jack_input_mode[jack_card_process][i]) {
                  case 3: // R only
                    jack_callback_buffer[j]=jack_input_volume[i]*
                      jack_input_buffer[i][1][j];
                    break;
                  case 2: // L only
                    jack_callback_buffer[j]=jack_input_volume[i]*
                      jack_input_buffer[i][0][j];
                    break;
                  case 1: // swap, sum R+L
                  case 0: // normal, sum L+R
                  default:
                    jack_callback_buffer[j]=jack_input_volume[i]*
                      (jack_input_buffer[i][0][j]+jack_input_buffer[i][1][j]);
                    break;
              }
            } // for nframes
            n=jack_record_ring[i]->
              write((char *)jack_callback_buffer,
                    nframes*sizeof(jack_default_audio_sample_t))/
                sizeof(jack_default_audio_sample_t);
            break;

          case 2: // stereo
            for(unsigned j=0;j<nframes;j++) {
              switch(jack_input_mode[jack_card_process][i]) {
                  case 3: // R only
                    memset(&jack_callback_buffer[2*j],0,sizeof(jack_input_buffer[i][0][j]));
                    jack_callback_buffer[2*j+1]=jack_input_volume[i]*
                      jack_input_buffer[i][1][j];
                    break;
                  case 2: // L only
                    jack_callback_buffer[2*j]=jack_input_volume[i]*
                      jack_input_buffer[i][0][j];
                    memset(&jack_callback_buffer[2*j+1],0,sizeof(jack_input_buffer[i][1][j]));
                    break;
                  case 1: // swap
                    jack_callback_buffer[2*j]=jack_input_volume[i]*
                      jack_input_buffer[i][1][j];
                    jack_callback_buffer[2*j+1]=jack_input_volume[i]*
                      jack_input_buffer[i][0][j];
                    break;
                  case 0: // normal
                  default:
                    jack_callback_buffer[2*j]=jack_input_volume[i]*
                      jack_input_buffer[i][0][j];
                    jack_callback_buffer[2*j+1]=jack_input_volume[i]*
                      jack_input_buffer[i][1][j];
                    break;
              }
            } // for nframes
            n=jack_record_ring[i]->
              write((char *)jack_callback_buffer,
                    2*nframes*sizeof(jack_default_audio_sample_t))/
                (2*sizeof(jack_default_audio_sample_t));
            break;
      }
    }
  }

  //
  // Process Output Streams
  //
  for(int i=0;i<RD_MAX_STREAMS;i++) {
    if(jack_playing[i]) {
      switch(jack_output_channels[i]) {
	  case 1:
	    n=jack_play_ring[i]->
	      read((char *)jack_callback_buffer,
		   nframes*sizeof(jack_default_audio_sample_t))/
	      sizeof(jack_default_audio_sample_t);
	    break;

	  case 2:
	    n=jack_play_ring[i]->
	      read((char *)jack_callback_buffer,
		   2*nframes*sizeof(jack_default_audio_sample_t))/
	      (2*sizeof(jack_default_audio_sample_t));
	    break;
      }
      for(int j=0;j<RD_MAX_PORTS;j++) {
	if(jack_output_volume[j][i]>0.0) {
	  switch(jack_output_channels[i]) {
	      case 1:
		for(unsigned k=0;k<n;k++) {
		  jack_output_buffer[j][0][k]=
		    jack_output_buffer[j][0][k]+jack_output_volume[j][i]*
		    jack_callback_buffer[k];
		  jack_output_buffer[j][1][k]=
		    jack_output_buffer[j][1][k]+jack_output_volume[j][i]*
		    jack_callback_buffer[k];
		}
		if(n!=nframes && jack_eof[i]) {
		  jack_stopping[i]=true;
		  jack_playing[i]=false;
		}
		break;

	      case 2:
		for(unsigned k=0;k<n;k++) {
		  jack_output_buffer[j][0][k]=
		    jack_output_buffer[j][0][k]+jack_output_volume[j][i]*
		    jack_callback_buffer[k*2];
		  jack_output_buffer[j][1][k]=
		    jack_output_buffer[j][1][k]+jack_output_volume[j][i]*
		    jack_callback_buffer[k*2+1];
		}
		if(n!=nframes && jack_eof[i]) {
		  jack_stopping[i]=true;
		  jack_playing[i]=false;
		}
		break;
	  }
	}
      }
      double ratio=(double)jack_output_sample_rate[i]/(double)jack_sample_rate;
      jack_output_pos[i]+=(int)(((double)n*ratio)+0.5);
    }
  }

  //
  // Process Meters
  //
  for(int i=0;i<RD_MAX_PORTS;i++) {
    // input meters (taking input mode into account)
    in_meter[0]=0.0;
    in_meter[1]=0.0;
    for(unsigned k=0;k<nframes;k++) {
      switch(jack_input_mode[jack_card_process][i]) {
          case 3: // R only
            if(jack_input_buffer[i][1][k]>in_meter[1]) 
              in_meter[1]=jack_input_buffer[i][1][k];
            break;
          case 2: // L only
            if(jack_input_buffer[i][0][k]>in_meter[0]) 
              in_meter[0]=jack_input_buffer[i][0][k];
            break;
          case 1: // swap
            if(jack_input_buffer[i][0][k]>in_meter[1]) 
              in_meter[1]=jack_input_buffer[i][0][k];
            if(jack_input_buffer[i][1][k]>in_meter[0]) 
              in_meter[0]=jack_input_buffer[i][1][k];
            break;
          case 0: // normal
          default:
            if(jack_input_buffer[i][0][k]>in_meter[0]) 
              in_meter[0]=jack_input_buffer[i][0][k];
            if(jack_input_buffer[i][1][k]>in_meter[1]) 
              in_meter[1]=jack_input_buffer[i][1][k];
            break;
      }
    } // for nframes
    jack_input_meter[i][0]->addValue(in_meter[0]);
    jack_input_meter[i][1]->addValue(in_meter[1]);

    // output meters
    for(int j=0;j<2;j++) {
      out_meter[j]=0.0;
      for(unsigned k=0;k<nframes;k++) {
	if(jack_output_buffer[i][j][k]>out_meter[j]) 
	  out_meter[j]=jack_output_buffer[i][j][k];
      }
      jack_output_meter[i][j]->addValue(out_meter[j]);
    }

  } // for RD_MAX_PORTS
  return 0;
}


int JackSampleRate(jack_nframes_t nframes, void *arg)
{
  jack_sample_rate=nframes;

  return 0;
}


void JackError(const char *desc)
{
  fprintf(stderr,"caed: Jack error: %s\n",desc);
}


void JackShutdown(void *arg)
{
}


void JackInitCallback()
{
  int avg_periods=(int)(330.0*jack_get_sample_rate(jack_client)/
			(1000.0*jack_get_buffer_size(jack_client)));
  for(int i=0;i<RD_MAX_PORTS;i++) {
    jack_recording[i]=false;
    jack_ready[i]=false;
    jack_input_volume[i]=1.0;
    jack_input_vox[i]=0.0;
    for(int j=0;j<2;j++) {
      jack_input_meter[i][j]=new RDMeterAverage(avg_periods);
      jack_output_meter[i][j]=new RDMeterAverage(avg_periods);
      jack_input_buffer[i][j]=NULL;
      jack_output_buffer[i][j]=NULL;
    }
    for(int j=0;j<RD_MAX_STREAMS;j++) {
      jack_output_volume[i][j]=1.0;
    }
    for(int j=0;j<RD_MAX_PORTS;j++) {
      jack_passthrough_volume[i][j]=0.0;
    }
    jack_record_ring[i]=NULL;
  }
  for(int i=0;i<RD_MAX_STREAMS;i++) {
    jack_play_ring[i]=NULL;
    jack_playing[i]=false;
  }
}
#endif  // JACK


void MainObject::jackStopTimerData(int stream)
{
#ifdef JACK
  jackStopPlayback(jack_card,stream);
  statePlayUpdate(jack_card,stream,2);
#endif  // JACK
}


void MainObject::jackFadeTimerData(int stream)
{
#ifdef JACK
  int level;
  if(!jack_fade_up[stream]) {
    level=jack_output_volume_db[jack_fade_port[stream]][stream]-
      jack_fade_increment[stream];
    if(level<=jack_fade_volume_db[stream]) {
      level=jack_fade_volume_db[stream];
      jack_fade_timer[stream]->stop();
    }
  }
  else {
    level=jack_output_volume_db[jack_fade_port[stream]][stream]+
      jack_fade_increment[stream];
    if(level>=jack_fade_volume_db[stream]) {
      level=jack_fade_volume_db[stream];
      jack_fade_timer[stream]->stop();
    }
  }
  jackSetOutputVolume(jack_card,stream,jack_fade_port[stream],level);
#endif  // JACK
}


void MainObject::jackRecordTimerData(int stream)
{
#ifdef JACK
  jackStopRecord(jack_card,stream);
  stateRecordUpdate(jack_card,stream,2);
#endif  // JACK
}


void MainObject::jackInit(RDStation *station)
{
#ifdef JACK
  jack_connected=false;
  jack_activated=false;

  //
  // Get Next Available Card Number
  //
  for(jack_card=0;jack_card<RD_MAX_CARDS;jack_card++) {
    if(cae_driver[jack_card]==RDStation::None) {
      break;
    }
  }
  if(jack_card==RD_MAX_CARDS) {
    LogLine(RDConfig::LogInfo,"no more RD cards available");
    return;
  }
  QString name=QString().sprintf("rivendell_%d",jack_card);

  //
  // Attempt to Connect to Jack Server
  //

  if((jack_client=jack_client_new((const char *)name))==0) {
    fprintf (stderr, "no connection to JACK server\n");
    LogLine(RDConfig::LogNotice,"no connection to JACK server");
    return;
  }
  jack_connected=true;
  jack_set_process_callback(jack_client,JackProcess,0);
  jack_set_sample_rate_callback(jack_client,JackSampleRate,0);
  jack_on_shutdown(jack_client,JackShutdown,0);

  //
  // Tell the database about us
  //
  if(jack_connected) {
    station->setCardDriver(jack_card,RDStation::Jack);
    station->setCardName(jack_card,"JACK Audio Connection Kit");
    station->setCardInputs(jack_card,RD_MAX_PORTS);
    station->setCardOutputs(jack_card,RD_MAX_PORTS);
  }

  //
  // Initialize Data Structures
  //
  for(int i=0;i<RD_MAX_STREAMS;i++) {
    for(int j=0;j<RD_MAX_PORTS;j++) {
      jack_output_volume_db[j][i]=0;
    }
  }
  for(int i=0;i<RD_MAX_PORTS;i++) {
    jack_input_volume_db[i]=0;
    for(int j=0;j<RD_MAX_PORTS;j++) {
      jack_passthrough_volume_db[j][i]=-10000;
    }
  }
  for(int i=0;i<RD_MAX_CARDS;i++) {
    for(int j=0;j<RD_MAX_PORTS;j++) {
      jack_input_mode[i][j]=0;
    }
  }
  jack_card_process = jack_card; // populate variable used by callback process

  //
  // Stop & Fade Timers
  //
  QSignalMapper *stop_mapper=new QSignalMapper(this,"stop_mapper");
  connect(stop_mapper,SIGNAL(mapped(int)),this,SLOT(jackStopTimerData(int)));
  QSignalMapper *fade_mapper=new QSignalMapper(this,"fade_mapper");
  connect(fade_mapper,SIGNAL(mapped(int)),this,SLOT(jackFadeTimerData(int)));
  QSignalMapper *record_mapper=new QSignalMapper(this,"record_mapper");
  connect(record_mapper,SIGNAL(mapped(int)),
	  this,SLOT(jackRecordTimerData(int)));
  for(int i=0;i<RD_MAX_STREAMS;i++) {
    jack_stop_timer[i]=new QTimer(this);
    stop_mapper->setMapping(jack_stop_timer[i],i);
    connect(jack_stop_timer[i],SIGNAL(timeout()),stop_mapper,SLOT(map()));
    jack_fade_timer[i]=new QTimer(this);
    fade_mapper->setMapping(jack_fade_timer[i],i);
    connect(jack_fade_timer[i],SIGNAL(timeout()),fade_mapper,SLOT(map()));
  }
  for(int i=0;i<RD_MAX_PORTS;i++) {
    jack_record_timer[i]=new QTimer(this);
    record_mapper->setMapping(jack_record_timer[i],i);
    connect(jack_record_timer[i],SIGNAL(timeout()),record_mapper,SLOT(map()));
  }

  //
  // Register Ports
  //
  for(int i=0;i<RD_MAX_PORTS;i++) {
    name=QString().sprintf("playout_%dL",i);
    jack_output_port[i][0]=
      jack_port_register(jack_client,(const char *)name,
			 JACK_DEFAULT_AUDIO_TYPE,
			 JackPortIsOutput|JackPortIsTerminal,0);
    name=QString().sprintf("playout_%dR",i);
    jack_output_port[i][1]=
      jack_port_register(jack_client,(const char *)name,
			 JACK_DEFAULT_AUDIO_TYPE,
			 JackPortIsOutput|JackPortIsTerminal,0);
    name=QString().sprintf("record_%dL",i);
    jack_input_port[i][0]=
      jack_port_register(jack_client,(const char *)name,
			 JACK_DEFAULT_AUDIO_TYPE,
			 JackPortIsInput|JackPortIsTerminal,0);
    name=QString().sprintf("record_%dR",i);
    jack_input_port[i][1]=
      jack_port_register(jack_client,(const char *)name,
			 JACK_DEFAULT_AUDIO_TYPE,
			 JackPortIsInput|JackPortIsTerminal,0);
  }

  //
  // Allocate Temporary Buffers
  //
  JackInitCallback();
  jack_wave_buffer=new short[RINGBUFFER_SIZE];
  jack_sample_buffer=new jack_default_audio_sample_t[RINGBUFFER_SIZE];
  jack_resample_buffer=new jack_default_audio_sample_t[2*RINGBUFFER_SIZE];

  //
  // Join the Graph
  //
  if(jack_activate(jack_client)) {
    return;
  }
  jack_sample_rate=jack_get_sample_rate(jack_client);
  jack_activated=true;
  cae_driver[jack_card]=RDStation::Jack;
  JackSessionSetup();

#endif  // JACK
}


void MainObject::jackFree()
{
#ifdef JACK
  if(jack_activated) {
    jack_deactivate(jack_client);
  }
#endif  // JACK
}


bool MainObject::jackLoadPlayback(int card,QString wavename,int *stream)
{
#ifdef JACK
  if((*stream=GetJackOutputStream())<0) {
    LogLine(RDConfig::LogErr,QString().sprintf(
            "Error: jackLoadPlayback(%s)   GetJackOutputStream():%d <0",
            (const char *) wavename,
            *stream) );
    return false;
  }
  jack_play_wave[*stream]=new RDWaveFile(wavename);
  if(!jack_play_wave[*stream]->openWave()) {
    LogLine(RDConfig::LogNotice,QString().sprintf(
            "Error: jackLoadPlayback(%s)   openWave() failed to open file",
            (const char *) wavename) );
    delete jack_play_wave[*stream];
    jack_play_wave[*stream]=NULL;
    FreeJackOutputStream(*stream);
    *stream=-1;
    return false;
  }
  switch (jack_play_wave[*stream]->type()){
      case RDWaveFile::Wave:
      case RDWaveFile::Ogg:
      case RDWaveFile::Mpeg:
        break;
     default:
    LogLine(RDConfig::LogNotice,QString().sprintf(
            "Error: jackLoadPlayback(%s)   getFormatTag()%d || getBistsPerSample()%d failed",
            (const char *) wavename,
            jack_play_wave[*stream]->getFormatTag(),
            jack_play_wave[*stream]->getBitsPerSample() ));
    delete jack_play_wave[*stream];
    jack_play_wave[*stream]=NULL;
    FreeJackOutputStream(*stream);
    *stream=-1;
    return false;
    break;
  }
  jack_output_channels[*stream]=jack_play_wave[*stream]->getChannels();
  jack_output_sample_rate[*stream]=jack_play_wave[*stream]->getSamplesPerSec();
  jack_stopping[*stream]=false;
  jack_offset[*stream]=0;
  jack_output_pos[*stream]=0;
  jack_eof[*stream]=false;
  FillJackOutputStream(*stream);
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackUnloadPlayback(int card,int stream)
{
#ifdef JACK
  if ((stream <0) || (stream >= RD_MAX_STREAMS)){
    return false;
  }
  if(jack_play_ring[stream]==NULL) {
    return false;
  }
  jack_playing[stream]=false;
  jack_play_wave[stream]->closeWave();
  delete jack_play_wave[stream];
  jack_play_wave[stream]=NULL;
  FreeJackOutputStream(stream);
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackPlaybackPosition(int card,int stream,unsigned pos)
{
#ifdef JACK
  if ((stream <0) || (stream >= RD_MAX_STREAMS)){
    return false;
  }
  if(jack_playing[stream]) {
  }
  jack_eof[stream]=false;
  jack_play_ring[stream]->reset();
  unsigned offset=
    (unsigned)((double)jack_play_wave[stream]->getSamplesPerSec()*
	       (double)jack_play_wave[stream]->getBlockAlign()*
	       (double)pos/1000);
  jack_offset[stream]=offset/jack_play_wave[stream]->getBlockAlign();
  offset=jack_offset[stream]*jack_play_wave[stream]->getBlockAlign();
  if(jack_offset[stream]>(int)jack_play_wave[stream]->getSampleLength()) {
    return false;
  }
  jack_output_pos[stream]=0;
  jack_play_wave[stream]->seekWave(offset,SEEK_SET);
  FillJackOutputStream(stream);
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackPlay(int card,int stream,int length,int speed,bool pitch,
			 bool rates)
{
#ifdef JACK
  if((stream <0) || (stream >= RD_MAX_STREAMS) || 
     (jack_play_ring[stream]==NULL)||jack_playing[stream]||
     (speed!=RD_TIMESCALE_DIVISOR)) {
    return false;
  }
  jack_playing[stream]=true;
  if(length>0) {
    jack_stop_timer[stream]->start(length,true);
  }
  statePlayUpdate(card,stream,1);
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackStopPlayback(int card,int stream)
{
#ifdef JACK
  if((stream <0) || (stream>=RD_MAX_STREAMS) || 
     (jack_play_ring[stream]==NULL)||(!jack_playing[stream])) {
    return false;
  }
  jack_playing[stream]=false;
  jack_stop_timer[stream]->stop();
  statePlayUpdate(card,stream,2);
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackTimescaleSupported(int card)
{
#ifdef JACK
  return false;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackLoadRecord(int card,int stream,int coding,int chans,
			       int samprate,int bitrate,QString wavename)
{
#ifdef JACK
  if ((stream <0) || (stream >=RD_MAX_PORTS)){
    return false;
  }
  jack_record_wave[stream]=new RDWaveFile(wavename);
  jack_record_wave[stream]->setFormatTag(WAVE_FORMAT_PCM);
  jack_record_wave[stream]->setChannels(chans);
  jack_record_wave[stream]->setSamplesPerSec(samprate);
  jack_record_wave[stream]->setBitsPerSample(16);
  jack_record_wave[stream]->setLevlChunk(true);
  if(coding==RDCae::OggVorbis || coding==RDCae::MpegL3) {
    jack_record_wave[stream]->setEnergyTag(1);
    }
  else {
    jack_record_wave[stream]->setBextChunk(true);
    } 
  if(!jack_record_wave[stream]->createWave()) {
    delete jack_record_wave[stream];
    jack_record_wave[stream]=NULL;
    return false;
  }
  chown((const char *)wavename,rd_config->uid(),rd_config->gid());
  if(coding==RDCae::OggVorbis || coding==RDCae::MpegL3) {
    chown((const char *)(wavename+".energy"),rd_config->uid(),rd_config->gid());
    }
  jack_input_channels[stream]=chans; 
  jack_record_ring[stream]=new RDRingBuffer(RINGBUFFER_SIZE);
  jack_record_ring[stream]->reset();
  jack_ready[stream]=true;
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackUnloadRecord(int card,int stream)
{
#ifdef JACK
  if ((stream <0) || (stream >= RD_MAX_PORTS)){
    return false;
  }
  jack_recording[stream]=false;
  jack_ready[stream]=false;
  EmptyJackInputStream(stream);
  jack_record_wave[stream]->closeWave();
  delete jack_record_wave[stream];
  jack_record_wave[stream]=NULL;
  delete jack_record_ring[stream];
  jack_record_ring[stream]=NULL;
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackRecord(int card,int stream,int length,int thres)
{
#ifdef JACK
  if ((stream <0) || (stream >= RD_MAX_PORTS)){
    return false;
  }
  if(!jack_ready[stream]) {
    return false;
  }
  jack_recording[stream]=true;
  if(jack_input_vox[stream]==0.0) {
    if(length>0) {
      jack_record_timer[stream]->start(length,true);
    }
    stateRecordUpdate(card,stream,4);
  }
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackStopRecord(int card,int stream)
{
#ifdef JACK
  if ((stream <0) || (stream >= RD_MAX_PORTS)){
    return false;
  }
  if(!jack_recording[stream]) {
    return false;
  }
  jack_recording[stream]=false;
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackSetInputVolume(int card,int stream,int level)
{
#ifdef JACK
  if ((stream <0) || (stream >= RD_MAX_STREAMS)){
    return false;
  }
  if(level>-10000) {
    jack_input_volume[stream]=
      (jack_default_audio_sample_t)pow10((double)level/2000.0);
    jack_input_volume_db[stream]=level;
  }
  else {
    jack_input_volume[stream]=0.0;
    jack_input_volume_db[stream]=-10000;
  }
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackSetOutputVolume(int card,int stream,int port,int level)
{
#ifdef JACK
  if ((stream <0) ||(stream >= RD_MAX_STREAMS) || 
      (port <0) || (port >= RD_MAX_PORTS)){
    return false;
  }
  if(level>-10000) {
    jack_output_volume[port][stream]=
      (jack_default_audio_sample_t)pow10((double)level/2000.0);
    jack_output_volume_db[port][stream]=level;
  }
  else {
    jack_output_volume[port][stream]=0.0;
    jack_output_volume_db[port][stream]=-10000;
  }
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackFadeOutputVolume(int card,int stream,int port,int level,
				     int length)
{
#ifdef JACK
  int diff;
  if ((stream <0) ||(stream >= RD_MAX_STREAMS) || 
      (port <0) || (port >= RD_MAX_PORTS)){
    return false;
  }
  if(jack_fade_timer[stream]->isActive()) {
    jack_fade_timer[stream]->stop();
  }
  if(level>jack_output_volume_db[port][stream]) {
    jack_fade_up[stream]=true;
    diff=level-jack_output_volume_db[port][stream];
  }
  else {
    jack_fade_up[stream]=false;
    diff=jack_output_volume_db[port][stream]-level;
  }
  jack_fade_volume_db[stream]=level;
  jack_fade_port[stream]=port;
  jack_fade_increment[stream]=diff*RD_JACK_FADE_INTERVAL/length;
  jack_fade_timer[stream]->start(RD_JACK_FADE_INTERVAL);
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackSetInputLevel(int card,int port,int level)
{
#ifdef JACK
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackSetOutputLevel(int card,int port,int level)
{
#ifdef JACK
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackSetInputMode(int card,int stream,int mode)
{
#ifdef JACK
  jack_input_mode[card][stream]=mode;
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackSetOutputMode(int card,int stream,int mode)
{
#ifdef JACK
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackSetInputVoxLevel(int card,int stream,int level)
{
#ifdef JACK
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackSetInputType(int card,int port,int type)
{
#ifdef JACK
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackGetInputStatus(int card,int port)
{
#ifdef JACK
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackGetInputMeters(int card,int port,short levels[2])
{
#ifdef JACK
  jack_default_audio_sample_t meter;
  if ((port <0) || (port >= RD_MAX_PORTS)){
    return false;
  }
  for(int i=0;i<2;i++) {
    meter=jack_input_meter[port][i]->average();
    if(meter==0.0) {
      levels[i]=-10000;
    }
    else {
      levels[i]=(short)(2000.0*log10(meter));
      if(levels[i]<-10000) {
	levels[i]=-10000;
      }
    }
  }
  return true;
#else
  return false;
#endif  // JACK
}


bool MainObject::jackGetOutputMeters(int card,int port,short levels[2])
{
#ifdef JACK
  jack_default_audio_sample_t meter;
  if ((port <0) || (port >= RD_MAX_PORTS)){
    return false;
  }

  for(int i=0;i<2;i++) {
    meter=jack_output_meter[port][i]->average();
    if(meter==0.0) {
      levels[i]=-10000;
    }
    else {
      levels[i]=(short)(2000.0*log10(meter));
      if(levels[i]<-10000) {
	levels[i]=-10000;
      }
    }
  }
  return true;
#else
  return false;
#endif  // JACK
}


void MainObject::jackGetOutputPosition(int card,unsigned *pos)
{
#ifdef JACK
  for(int i=0;i<RD_MAX_STREAMS;i++) {
//    pos[i]=(((unsigned long long)jack_offset[i]+jack_output_pos[i])*1000)/
//	    jack_sample_rate;
    pos[i]=((unsigned long long)jack_offset[i]+jack_output_pos[i]);
  }
#endif  // JACK
}

bool MainObject::jackSetPassthroughLevel(int card,int in_port,int out_port,
					int level)
{
#ifdef JACK
  if ((in_port <0) || (in_port >= RD_MAX_PORTS) || 
      (out_port <0) || (out_port >= RD_MAX_PORTS)){
    return false;
  }
  if(level>-10000) {
    jack_passthrough_volume[in_port][out_port]=
      (jack_default_audio_sample_t)pow10((double)level/2000.0);
    jack_passthrough_volume_db[in_port][out_port]=level;
  }
  else {
    jack_passthrough_volume[in_port][out_port]=0.0;
    jack_passthrough_volume_db[in_port][out_port]=-10000;
  }
  return true;
#else
  return false;
#endif  // JACK
}


int MainObject::GetJackOutputStream()
{
#ifdef JACK
  for(int i=0;i<RD_MAX_STREAMS;i++) {
    if(jack_play_ring[i]==NULL) {
      jack_play_ring[i]=new RDRingBuffer(RINGBUFFER_SIZE);
      return i;
    }
  }
  return -1;
#else
  return -1;
#endif
}


void MainObject::FreeJackOutputStream(int stream)
{
#ifdef JACK
  if ((stream <0) || (stream >= RD_MAX_STREAMS)){
    return;
  }
  delete jack_play_ring[stream];
  jack_play_ring[stream]=NULL;
#else
  return;
#endif
}


void MainObject::EmptyJackInputStream(int stream)
{
#ifdef JACK
  if ((stream <0) || (stream >= RD_MAX_STREAMS)){
    return;
  }
  int n=jack_record_ring[stream]->
    read((char *)jack_sample_buffer,RINGBUFFER_SIZE)/
    sizeof(jack_default_audio_sample_t);
  if(n==0) {
    return;
  }
  if(jack_record_wave[stream]->getSamplesPerSec()!=jack_sample_rate) {
    double ratio=(double)jack_record_wave[stream]->getSamplesPerSec()/
      (double)jack_sample_rate;
    n=Resample(jack_sample_buffer,jack_resample_buffer,
	       n/jack_input_channels[stream],ratio,
	       jack_input_channels[stream])*jack_input_channels[stream];
#ifdef HAVE_SRC_CONV
    src_float_to_short_array(jack_resample_buffer,jack_wave_buffer,n);
#else
    for(int i=0;i<n;i++) {
      jack_wave_buffer[i]=(short)(jack_sample_buffer[i]*32768.0);
    }
#endif  // HAVE_SRC_CONV
  }
  else {
#ifdef HAVE_SRC_CONV
    src_float_to_short_array(jack_sample_buffer,jack_wave_buffer,n);
#else
    for(int i=0;i<n;i++) {
      jack_wave_buffer[i]=(short)(jack_sample_buffer[i]*32768.0);
    }
#endif  // HAVE_SRC_CONV
  }
  jack_record_wave[stream]->writeWave(jack_wave_buffer,n*sizeof(short));
#endif  // JACK
}


void MainObject::FillJackOutputStream(int stream)
{
#ifdef JACK
  if ((stream <0) || (stream >= RD_MAX_STREAMS)){
    return;
  }
  int free=
    jack_play_ring[stream]->writeSpace()/sizeof(jack_default_audio_sample_t)-1;
  if((free<=0)||(jack_eof[stream]==true)) {
    return;
  }
  double ratio=(double)jack_sample_rate/
    (double)jack_play_wave[stream]->getSamplesPerSec();
  free=(int)((double)free/ratio)/jack_output_channels[stream]*
    jack_output_channels[stream];
  int n=jack_play_wave[stream]->readWave(jack_wave_buffer,sizeof(short)*free)/
    sizeof(short);
  if(n!=free) {
    jack_eof[stream]=true;
    jack_stop_timer[stream]->stop();
  }
#ifdef HAVE_SRC_CONV
  src_short_to_float_array(jack_wave_buffer,jack_sample_buffer,n);
#else
  for(int i=0;i<n;i++) {
    jack_sample_buffer[i]=
      ((jack_default_audio_sample_t)(jack_wave_buffer[i]))/32768.0;
  }
#endif  // HAVE_SRC_CONV
  if(jack_play_wave[stream]->getSamplesPerSec()!=jack_sample_rate) {
    n=Resample(jack_sample_buffer,jack_resample_buffer,
	       n/jack_output_channels[stream],ratio,
	       jack_output_channels[stream])*jack_output_channels[stream];
    jack_play_ring[stream]->
      write((char *)jack_resample_buffer,
	    n*sizeof(jack_default_audio_sample_t));
  }
  else {
    jack_play_ring[stream]->
      write((char *)jack_sample_buffer,n*sizeof(jack_default_audio_sample_t));
  }
#endif  // JACK
}


void MainObject::JackClock()
{
#ifdef JACK
  for(int i=0;i<RD_MAX_STREAMS;i++) {
    if(jack_stopping[i]) {
      jack_stopping[i]=false;
      statePlayUpdate(jack_card,i,2);
    }
    if(jack_playing[i]&&((jack_clock_phase%4)==0)) {
      FillJackOutputStream(i);
    }
  }
  jack_clock_phase++;
  for(int i=0;i<RD_MAX_PORTS;i++) {
    if(jack_recording[i]) {
      EmptyJackInputStream(i);
    }
  }
#endif  // JACK
}


void MainObject::JackSessionSetup()
{
#ifdef JACK
  int count=0;
  RDProfile *profile=new RDProfile();
  profile->setSource(RD_CONF_FILE);
  bool src_ok=false;
  bool dest_ok=false;
  QString src_tag="Source1";
  QString dest_tag="Destination1";
  QString src=profile->stringValue("JackSession",src_tag,"",&src_ok);
  QString dest=profile->stringValue("JackSession",dest_tag,"",&dest_ok);
  while(src_ok&&dest_ok) {
    if(jack_connect(jack_client,(const char *)src,(const char *)dest)!=0) {
      LogLine(RDConfig::LogNotice,QString().
	      sprintf("unable to connect %s to %s",
		      (const char *)src,(const char *)dest));
    }
    count++;
    src_tag=QString().sprintf("Source%d",count+1);
    dest_tag=QString().sprintf("Destination%d",count+1);
    src=profile->stringValue("JackSession",src_tag,"",&src_ok);
    dest=profile->stringValue("JackSession",dest_tag,"",&dest_ok);
  }
  delete profile;
#endif  // JACK
}
