// rdwavepainter.h
//
// A Painter Class for Drawing Audio Waveforms
//
//   (C) Copyright 2002-2005 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdwavepainter.h,v 1.7 2007/12/29 18:16:10 fredg Exp $
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

#ifndef RDWAVEPAINTER_H
#define RDWAVEPAINTER_H

#include <qpainter.h>
#include <rdwavefile.h>


class RDWavePainter : public QPainter,public RDWaveFile
{
 public:
  enum Channel {Mono=0,Left=1,Right=2};
  RDWavePainter(const QPaintDevice *pd,const QString &wavname);
  RDWavePainter();
  ~RDWavePainter();
  void nameWave(QString file_name);
  bool begin(const QPaintDevice *pd,const QString &wavname);
  bool begin(const QPaintDevice *pd);
  bool end();
  void drawWaveBySamples(int x,int w,int startsamp,int endsamp,int gain,
			 Channel channel,const QColor &color,
			 int startclip=-1,int endclip=-1);
  void drawWaveByMsecs(int x,int w,int startmsecs,int endmsecs,int gain,
		       Channel channel,const QColor &color,
		       int startclip=-1,int endclip=-1);

 private:
  QString wave_name;
};


#endif  // RDWAVEPAINTER_H
