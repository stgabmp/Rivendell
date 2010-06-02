// rdaconvert.h
//
// Utility for channelizing, normallizing and sample-rate converting audio.
//
//   (C) Copyright 2009 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdaconvert.h,v 1.1.2.3.2.1 2010/03/10 17:17:36 cvs Exp $
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

#ifndef RDACONVERT_H
#define RDACONVERT_H

#include <stdint.h>

#include <samplerate.h>

#include <qmainwindow.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qsocketdevice.h>
#include <qlabel.h>
#include <qtimer.h>

#include <rd.h>

//
// Settings
//
#define RDACONVERT_USAGE "--operation=ratio|convert --channels=<chans> --sample-rate=<samp-rate> --gain-ratio=<ratio> --converter=<num> <infile> <outfile>\n\nWhere <chans> is the number of audio channels and <samp-rate> is the\nsample rate of the PCM16 data to be written to be\nwritten to <outfile>.\n"
#define RDACONVERT_BUFFER_SIZE 512
#define BUFFER_FRAMES 512

class MainObject : public QObject
{
  Q_OBJECT
 public:
  enum Operation {OpsNone=0,OpsRatio=1,OpsConvert=2};
  MainObject(QObject *parent=0,const char *name=0);
  ~MainObject();

 private:
  void RunRatio(const QString &infile);
  void RunConversion(const QString &infile,const QString &outfile);
  int ChannelConvert(int inchans,float *inbuf,int outchans,float *outbuf,
		     int frames);
  int RateConvert(float *inbuf,float *outbuf,int frames,bool eof=false);
  int GainConvert(int channels,float *inbuf,float *outbuf,int frames);
  void FlipBuffers(float **buf1,float **buf2);
  void Int16ToFloat(int16_t *inbuf,float *outbuf,int frames);
  void FloatToInt16(float *inbuf,int16_t *outbuf,int frames);
  Operation operation;
  double gain_ratio;
  int input_channels;
  int input_samplerate;
  int output_channels;
  int output_samplerate;
  int src_converter;
  SRC_STATE *src_state;
  SRC_DATA src_data;
};


#endif  // RDACONVERT_H
