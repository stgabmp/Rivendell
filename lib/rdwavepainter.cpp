// rdwavepainter.cpp
//
// A Painter Class for Drawing Audio Waveforms
//
//   (C) Copyright 2002-2005 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdwavepainter.cpp,v 1.8 2007/12/29 18:16:10 fredg Exp $
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

#include <qpointarray.h>
#include <qpixmap.h>
#include <qfile.h>

#include <rdwavepainter.h>


RDWavePainter::RDWavePainter(const QPaintDevice *pd,const QString &wavname)
  : QPainter(pd), RDWaveFile(wavname)
{
  wave_name=wavname;
  RDWaveFile::openWave();
}


RDWavePainter::RDWavePainter()
{
}


RDWavePainter::~RDWavePainter()
{
  RDWaveFile::closeWave();
}


void RDWavePainter::nameWave(QString file_name)
{
  wave_name=file_name;
  RDWaveFile::nameWave(file_name);
}


bool RDWavePainter::begin(const QPaintDevice *pd,const QString &wavname)
{
  RDWaveFile::nameWave(wavname);
  RDWaveFile::openWave();
  return QPainter::begin(pd);
}


bool RDWavePainter::begin(const QPaintDevice *pd)
{
  return QPainter::begin(pd);
}


bool RDWavePainter::end()
{
 // RDWaveFile::closeWave();
  return QPainter::end();
}


void RDWavePainter::drawWaveBySamples(int x,int w,int startsamp,int endsamp,
				      int gain,Channel channel,
				      const QColor &color,
				      int startclip,int endclip)
{
  int startblock=startsamp/1152;
  int endblock=endsamp/1152;
  int startclipblock=-1;
  int endclipblock=-1;


  if((startblock>(int)energySize())||(energySize()==0)) {
    return;
  }
  if(startclip>=0) {
    startclipblock=startclip/1152;
  }
  if(endclip>=0) {
    endclipblock=endclip/1152;
  }

  double time_scale=1.0;
  double gain_scale=1.0;
  int dx=0;
  QPixmap *pix=(QPixmap *)device();
  int center=pix->height()/2;
  RDWavePainter::Channel effective_channel=channel;
  switch(channel) {
      case RDWavePainter::Left:
      case RDWavePainter::Right:
	if(getChannels()==1) {
	  effective_channel=RDWavePainter::Mono;
	}
	break;

      default:
	effective_channel=channel;
	break;
  }
  save();
  resetXForm();
  setPen(color);
  setBrush(color);
  QPointArray array(w+2);
  array.setPoint(0,0,center);
  array.setPoint(w+1,w+1,center);
  switch(effective_channel) {
      case RDWavePainter::Left:
	time_scale=(double)(endblock-startblock)/(double)w;
	gain_scale=(double)(pix->height()/65536.0)*
	  pow10((double)gain/2000.0);
	for(int i=0;i<w;i++) {
	  if(((dx=(2*((int)(time_scale*(double)i)+startblock)))<
	     (int)energySize())&&
	     ((startclipblock<0)||(dx>(startclipblock*2)))&&
	     ((endclipblock<0)||(dx<(endclipblock*2)))) {
	    array.setPoint(i+1,i+1,
			   center+(int)(gain_scale*(double)energy(dx)));
	  }
	  else {
	    array.setPoint(i+1,i+1,center);
	  }
	}
	break;

      case RDWavePainter::Right:
	time_scale=(double)(endblock-startblock)/(double)w;
	gain_scale=(double)(pix->height()/65536.0)*
	  pow10((double)gain/2000.0);
	for(int i=0;i<w;i++) {
	  if(((dx=(1+2*((int)(time_scale*(double)i)+startblock)))<
	     (int)energySize())&&
	     ((startclipblock<0)||(dx>(startclipblock*2)))&&
	     ((endclipblock<0)||(dx<(endclipblock*2)))) {
	    array.setPoint(i+1,i+1,
			   center+(int)(gain_scale*(double)energy(dx)));
	  }
	  else {
	    array.setPoint(i+1,i+1,center);
	  }
	}
	break;

      case RDWavePainter::Mono:
	switch(getChannels()) {
	    case 1:
	      time_scale=(double)(endblock-startblock)/(double)w;
	      gain_scale=(double)(pix->height()/65536.0)*
		pow10((double)gain/2000.0);
	      for(int i=0;i<w;i++) {
		if(((dx=((int)(time_scale*(double)i)+startblock))<
		   (int)energySize())&&
		   ((startclipblock<0)||(dx>startclipblock))&&
		   ((endclipblock<0)||(dx<endclipblock))) {
		  array.setPoint(i+1,i+1,
				 center+(int)(gain_scale*(double)energy(dx)));
		}
		else {
		  array.setPoint(i+1,i+1,center);
		}
	      }
	      break;

	    case 2:
	      time_scale=(double)(endblock-startblock)/(double)w;
	      gain_scale=(double)(pix->height()/65536.0)*
		pow10((double)gain/2000.0);
	      for(int i=0;i<w;i++) {
		if(((dx=(2*((int)(time_scale*(double)i)+startblock)))<
		   (int)energySize())&&
		   ((startclipblock<0)||(dx>(startclipblock*2)))&&
		   ((endclipblock<0)||(dx<(endclipblock*2)))) {
		  array.setPoint(i+1,i+1,
				 center+(int)(gain_scale*
					      ((double)energy(dx)+
					       (double)energy(dx+1))/2.0));
		}
		else {
		  array.setPoint(i+1,i+1,center);
		}
	      }
	      break;
	}
	break;
  }
  drawPolygon(array);
  for(int i=0;i<(w+2);i++) {
    array.setPoint(i,array.point(i).x(),2*center-array.point(i).y());
  }
  drawPolygon(array);
  restore();
}


void RDWavePainter::drawWaveByMsecs(int x,int w,int startmsecs,int endmsecs,
				    int gain,Channel channel,
				    const QColor &color,
				    int startclip,int endclip)
{
  drawWaveBySamples(x,w,
	     (unsigned)((double)startmsecs*(double)getSamplesPerSec()/1000.0),
	     (unsigned)((double)endmsecs*(double)getSamplesPerSec()/1000.0),
	     gain,channel,color,
	     (int)((double)startclip*(double)getSamplesPerSec()/1000.0),
	     (int)((double)endclip*(double)getSamplesPerSec()/1000.0));
}
