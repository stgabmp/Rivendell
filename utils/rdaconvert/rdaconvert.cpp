// rdaconvert.cpp
//
// Utility for channelizing, normallizing and sample-rate converting audio.
//
//   (C) Copyright 2009 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdaconvert.cpp,v 1.1.2.4 2009/09/04 11:40:08 cvs Exp $
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

#include <stdlib.h>
#include <unistd.h>

#include <qapplication.h>
#include <qwidget.h>

#include <rdwavefile.h>

#include <rdcmd_switch.h>
#include <rdaconvert.h>

//
// Globals
//
RDCmdSwitch *rdcmdswitch=NULL;


MainObject::MainObject(QObject *parent,const char *name)
{
  //
  // Read Command Options
  //
  operation=MainObject::OpsNone;
  output_channels=0;
  output_samplerate=0;
  gain_ratio=0.0;
  src_converter=SRC_SINC_MEDIUM_QUALITY;

  RDCmdSwitch *cmd=new RDCmdSwitch(qApp->argc(),qApp->argv(),"rdaconvert",
				   RDACONVERT_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--operation") {
      if(cmd->value(i).lower()=="ratio") {
	operation=MainObject::OpsRatio;
      }
      if(cmd->value(i).lower()=="convert") {
	operation=MainObject::OpsConvert;
      }
    }
    if(cmd->key(i)=="--channels") {
      output_channels=cmd->value(i).toUInt();
    }
    if(cmd->key(i)=="--sample-rate") {
      output_samplerate=cmd->value(i).toUInt();
    }
    if(cmd->key(i)=="--gain-ratio") {
      gain_ratio=cmd->value(i).toDouble();
    }
    if(cmd->key(i)=="--converter") {
      src_converter=cmd->value(i).toInt();
    }
  }
  switch(operation) {
  case MainObject::OpsRatio:
    if(cmd->keys()!=2) {
      fprintf(stderr,"rdaconvert: missing filename argument\n");
      exit(256);
    }
    RunRatio(cmd->key(1));
    break;

  case MainObject::OpsConvert:
    if(output_channels==0) {
      fprintf(stderr,"rdaconvert: missing/invalid --channels argument\n");
      exit(256);
    }
    if(output_samplerate==0) {
      fprintf(stderr,"rdaconvert: missing/invalid --sample-rate argument\n");
      exit(256);
    }
    if(gain_ratio==0.0) {
      fprintf(stderr,"rdaconvert: missing/invalid --gain-ratio argument\n");
      exit(256);
    }
    if(cmd->keys()!=7) {
      fprintf(stderr,"rdaconvert: missing filename argument(s)\n");
      exit(256);
    }
    RunConversion(cmd->key(5),cmd->key(6));
    break;

  default:
    fprintf(stderr,"rdaconvert: no operation specified\n");
    exit(256);
    break;
  }
  delete cmd;

  exit(0);
}


void MainObject::RunRatio(const QString &infile)
{
  RDWaveFile *inwave=NULL;
  int16_t *wavebuf;
  int16_t max_sample=0;
  size_t n;

  //
  // Open Input File
  //
  inwave=new RDWaveFile(infile);
  if(!inwave->openWave()) {
    fprintf(stderr,"rdaconvert: unable to open input file\n");
    delete inwave;
    exit(256);
  }
  input_channels=inwave->getChannels();

  //
  // Initialize Buffers
  //
  wavebuf=new int16_t[8*BUFFER_FRAMES];

  //
  // Find Max Amplitude
  //
  while((n=inwave->readWave(wavebuf,BUFFER_FRAMES*sizeof(int16_t)))>0) {
    int frames=n/sizeof(int16_t);
    for(int i=0;i<frames;i++) {
      if(abs(wavebuf[i])>max_sample) {
	max_sample=abs(wavebuf[i]);
      }
    }
  }

  //
  // Output and Clean Up
  //
  printf("%5.3lf\n",32768.0/(double)max_sample);
  inwave->closeWave();
}


void MainObject::RunConversion(const QString &infile,const QString &outfile)
{
  RDWaveFile *inwave=NULL;
  float *inbuf;
  RDWaveFile *outwave=NULL;
  float *outbuf;
  int16_t *wavebuf=NULL;
  size_t n;

  //
  // Open Input File
  //
  inwave=new RDWaveFile(infile);
  if(!inwave->openWave()) {
    fprintf(stderr,"rdaconvert: unable to open input file\n");
    delete inwave;
    exit(256);
  }
  input_channels=inwave->getChannels();
  input_samplerate=inwave->getSamplesPerSec();

  //
  // Create Output File
  //
  if(outfile!="-") {
    outwave=new RDWaveFile(outfile);
    outwave->nameWave(outfile);
    outwave->setFormatTag(WAVE_FORMAT_PCM);
    outwave->setBitsPerSample(16);
    outwave->setChannels(output_channels);
    outwave->setSamplesPerSec(output_samplerate);
    outwave->setBextChunk(true);
    outwave->setLevlChunk(true);
    if(!outwave->createWave()) {
      fprintf(stderr,"rdaconvert: unable to open output file\n");
      delete outwave;
      delete inwave;
      exit(256);
    }
  }

  //
  // Initialize Buffers
  //
  inbuf=new float[8*BUFFER_FRAMES];
  outbuf=new float[8*BUFFER_FRAMES];
  wavebuf=new int16_t[8*BUFFER_FRAMES];

  //
  // Initialize Sample Rate Converter
  //
  int err=0;
  src_state=NULL;
  if(input_samplerate!=output_samplerate) {
    //
    // Optimize so we process the least amount of data
    //
    int chans=input_channels;
    if(output_channels<chans) {
      chans=output_channels;
    }
    if((src_state=src_new(src_converter,chans,&err))==NULL) {
      fprintf(stderr,"rdaconvert: SRC Initialization Error - %s\n",
	      src_strerror(err));
      delete outwave;
      delete inwave;
      exit(256);
    }
    memset(&src_data,0,sizeof(src_data));
    src_data.src_ratio=(double)output_samplerate/(double)input_samplerate;
  }

  //
  // Conversion Loop
  //
  int frames;
  while((n=inwave->
     readWave(wavebuf,BUFFER_FRAMES*input_channels*sizeof(int16_t)))>0) {
    frames=n/(input_channels*sizeof(int16_t));
    Int16ToFloat(wavebuf,outbuf,frames*input_channels);
    float *in=inbuf;
    float *out=outbuf;

    if((src_state!=NULL)&&(input_channels<=output_channels)) {
      FlipBuffers(&in,&out);
      frames=RateConvert(in,out,frames);
    }
    if(input_channels!=output_channels) {
      FlipBuffers(&in,&out);
      frames=ChannelConvert(input_channels,in,output_channels,out,frames);
    }
    if(gain_ratio!=1.0) {
      FlipBuffers(&in,&out);
      frames=GainConvert(output_channels,in,out,frames);
    }
    if((src_state!=NULL)&&(input_channels>output_channels)) {
      FlipBuffers(&in,&out);
      frames=RateConvert(in,out,frames);
    }
    FloatToInt16(out,wavebuf,frames*output_channels);
    if(outwave==NULL) {
      write(1,wavebuf,frames*output_channels*sizeof(int16_t));
    }
    else {
      outwave->writeWave(wavebuf,frames*output_channels*sizeof(int16_t));
    }
  }
  if(src_state!=NULL) {
    float *in=inbuf;
    float *out=outbuf;
    frames=RateConvert(in,out,frames,true);
    if(frames>0) {
      if(input_channels!=output_channels) {
	FlipBuffers(&in,&out);
	frames=ChannelConvert(input_channels,in,output_channels,out,frames);
      }
      if(gain_ratio!=1.0) {
	FlipBuffers(&in,&out);
	frames=GainConvert(output_channels,in,out,frames);
      }
      FloatToInt16(out,wavebuf,frames*output_channels);
      if(outwave==NULL) {
	write(1,wavebuf,frames*output_channels*sizeof(int16_t));
      }
      else {
	outwave->writeWave(wavebuf,frames*output_channels*sizeof(int16_t));
      }
    }
  }

  //
  // Clean Up and Exit
  //
  if(src_state!=NULL) {
    src_delete(src_state);
  }
  if(outwave!=NULL) {
    outwave->closeWave();
    delete outwave;
  }
  inwave->closeWave();
  delete inwave;
}


MainObject::~MainObject()
{
}


int MainObject::ChannelConvert(int inchans,float *inbuf,int outchans,
			       float *outbuf,int frames)
{
  if(inchans==outchans) {  // Straight copy
    for(int i=0;i<(frames*inchans);i++) {
      outbuf[i]=inbuf[i];
    }
    return frames;
  }
  if((inchans==1)&&(outchans==2)) {  // Upmix
    for(int i=0;i<frames;i++) {
      outbuf[2*i]=inbuf[i];
      outbuf[2*i+1]=inbuf[i];
    }
    return frames;
  }
  if((inchans==2)&&(outchans==1)) {  // Downmix
    for(int i=0;i<(2*frames);i+=2) {
      outbuf[i/2]=(inbuf[i]+inbuf[i+1])/2.0;
    }
    return frames;
  }
  return 0;
}


int MainObject::RateConvert(float *inbuf,float *outbuf,int frames,bool eof)
{
  int err;


  src_data.data_in=inbuf;
  src_data.data_out=outbuf;
  src_data.input_frames=frames;
  src_data.output_frames=4*BUFFER_FRAMES;
  src_data.end_of_input=eof;
  if((err=src_process(src_state,&src_data))!=0) {
    fprintf(stderr,"SRC Process Error - %s\n",src_strerror(err));
  }
  return src_data.output_frames_gen;
}


int MainObject::GainConvert(int channels,float *inbuf,float *outbuf,int frames)
{
  for(int i=0;i<(frames*channels);i++) {
    outbuf[i]=gain_ratio*inbuf[i];
    if(outbuf[i]>1.0) {
      outbuf[i]=1.0;
    }
    if(outbuf[i]<-1.0) {
      outbuf[i]=-1.0;
    }
  }
  return frames;
}


void MainObject::FlipBuffers(float **buf1,float **buf2)
{
  float *buf=*buf1;
  *buf1=*buf2;
  *buf2=buf;
}


void MainObject::Int16ToFloat(int16_t *inbuf,float *outbuf,int frames)
{
#ifdef HAVE_SRC_CONV
  src_short_to_float_array(inbuf,outbuf,frames);
#else
  for(int i=0;i<frames;i++) {
    outbuf[i]=((float)(inbuf[i]))/32768.0;
  }
#endif  // HAVE_SRC_CONV
}


void MainObject::FloatToInt16(float *inbuf,int16_t *outbuf,int frames)
{
#ifdef HAVE_SRC_CONV
  src_float_to_short_array(inbuf,outbuf,frames);
#else
  for(int i=0;i<frames;i++) {
    outbuf[i]=(short)(inbuf[i]*32768.0);
  }
#endif  // HAVE_SRC_CONV
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  new MainObject(NULL,"main");
  return a.exec();
}
