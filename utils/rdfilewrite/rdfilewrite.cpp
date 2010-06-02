// rdfilewrite.cpp
//
// A utility for sending RML Commands
//
//   (C) Copyright 2002-2005 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdfilewrite.cpp,v 1.6 2008/07/26 00:01:42 fredg Exp $
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
#include <rdfilewrite.h>

//
// Globals
//
RDCmdSwitch *rdcmdswitch=NULL;


MainObject::MainObject(QObject *parent,const char *name)
{
  unsigned sample_rate=0;
  unsigned channels=0;
  QString filename;
  char buffer[RDFILEWRITE_BUFFER_SIZE];
  size_t n;

  //
  // Read Command Options
  //
  RDCmdSwitch *cmd=new RDCmdSwitch(qApp->argc(),qApp->argv(),"rdfilewrite",
				   RDFILEWRITE_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--channels") {
      channels=cmd->value(i).toUInt();
    }
    if(cmd->key(i)=="--sample-rate") {
      sample_rate=cmd->value(i).toUInt();
    }
  }
  if(channels==0) {
    fprintf(stderr,"rdfilewrite: missing/invalid --channels argument\n");
    exit(256);
  }
  if(sample_rate==0) {
    fprintf(stderr,"rdfilewrite: missing/invalid --sample-rate argument\n");
    exit(256);
  }
  if(cmd->keys()!=3) {
    fprintf(stderr,"rdfilewrite: missing filename argument\n");
    exit(256);
  }
  filename=cmd->key(2);
  delete cmd;

  //
  // Create Output File
  //
  RDWaveFile *wavefile=new RDWaveFile(filename);
  wavefile->nameWave(filename);
  wavefile->setFormatTag(WAVE_FORMAT_PCM);
  wavefile->setBitsPerSample(16);
  wavefile->setChannels(channels);
  wavefile->setSamplesPerSec(sample_rate);
  wavefile->setBextChunk(true);
  wavefile->setLevlChunk(true);
  if(!wavefile->createWave()) {
    fprintf(stderr,"rdfilewrite: unable to open output file\n");
    delete wavefile;
    exit(256);
  }

  //
  // Transfer Data
  //
  while((n=read(0,buffer,RDFILEWRITE_BUFFER_SIZE))>0) {
    wavefile->writeWave(buffer,n);
  }

  //
  // Clean Up and Exit
  //
  wavefile->closeWave();
  delete wavefile;
  exit(0);
}


MainObject::~MainObject()
{
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  new MainObject(NULL,"main");
  return a.exec();
}
