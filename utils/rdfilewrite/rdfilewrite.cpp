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
  unsigned sample_rate=44100;
  unsigned channels=2;
  QString filename;
  bool add_mode=false;
  double normalize_level=1.0;
  bool do_energy=false;
  unsigned filename_pos=1;
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
      filename_pos++;
    }
    if(cmd->key(i)=="--sample-rate") {
      sample_rate=cmd->value(i).toUInt();
      filename_pos++;
    }
    if(cmd->key(i)=="--add-mode") {
      add_mode=true;
      filename_pos++;
    }
    if(cmd->key(i)=="--normalize") {
      normalize_level=cmd->value(i).toDouble();
      if(normalize_level==0.0) {
        normalize_level=1.0;
        }
      do_energy=true;
      filename_pos++;
    }
  }
  if(channels==0) {
    fprintf(stderr,"rdfilewrite: missing/invalid --channels argument\n");
    exit(1);
  }
  if(sample_rate==0) {
    fprintf(stderr,"rdfilewrite: missing/invalid --sample-rate argument\n");
    exit(1);
  }
  if(cmd->keys()!=(filename_pos)) {
    fprintf(stderr,"rdfilewrite: missing filename argument\n");
    exit(1);
  }
  filename=cmd->key(filename_pos-1);
  delete cmd;
  //
  // Create Output File
  //
  if(do_energy){
    unlink(filename+".energy");
  }
  RDWaveFile *wavefile=new RDWaveFile(filename);
  if(!add_mode) {
  wavefile->nameWave(filename);
  wavefile->setBitsPerSample(16);
  wavefile->setChannels(channels);
  wavefile->setSamplesPerSec(sample_rate);
    wavefile->setFormatTag(WAVE_FORMAT_PCM);
  }  
  wavefile->setBextChunk(true);
  wavefile->setLevlChunk(true);
  if(do_energy){
    wavefile->setEnergyTag(1);
  }
  else {
    wavefile->setEnergyTag(0);
  }
  if(!add_mode) {
  if(!wavefile->createWave()) {
    fprintf(stderr,"rdfilewrite: unable to open output file\n");
    wavefile->closeWave();
    delete wavefile;
      exit(1);
  }

  //
  // Transfer Data
  //
  
  while((n=read(0,buffer,RDFILEWRITE_BUFFER_SIZE))>0) {
    wavefile->writeWave(buffer,n);
  }
  }
  else {
    if(!wavefile->openWave()) {
      fprintf(stderr,"rdfilewrite: unable to open output file\n");
      delete wavefile;
      exit(1);
    }
    wavefile->recreateEnergy();
    if(wavefile->energySize()==0) {
      printf("rdfilewrite: no energy created\n");
      exit(1);
    }  
  }

  //
  // Clean Up and Exit
  //
  if(normalize_level!=1.0f)
    wavefile->normalize(normalize_level);
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
