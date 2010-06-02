// sas64000gpi.cpp
//
// A Rivendell switcher driver for the SAS64000 connected via 
//   a GPI-1600
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: sas64000gpi.cpp,v 1.8 2008/10/01 13:10:15 fredg Exp $
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

#include <globals.h>
#include <sas64000gpi.h>


Sas64000Gpi::Sas64000Gpi(RDMatrix *matrix,QObject *parent,const char *name)
  : QObject(parent,name)
{
  //
  // Get Matrix Parameters
  //
  sas_matrix=matrix->matrix();
  sas_inputs=matrix->inputs();
  sas_outputs=matrix->outputs();
  sas_gpis=matrix->gpis();
  sas_gpos=matrix->gpos();

  //
  // Initialize the TTY Port
  //
  RDTty *tty=new RDTty(rdstation->name(),matrix->port(RDMatrix::Primary));
  sas_device=new RDTTYDevice();
  if(tty->active()) {
    sas_device->setName(tty->port());
    sas_device->setSpeed(tty->baudRate());
    sas_device->setWordLength(tty->dataBits());
    sas_device->setParity(tty->parity());
    sas_device->open(IO_Raw|IO_ReadWrite);
  }
  delete tty;

  //
  // Interval OneShots
  //
  sas_gpo_oneshot=new RDOneShot(this);
  connect(sas_gpo_oneshot,SIGNAL(timeout(void *)),
	  this,SLOT(gpoOneshotData(void*)));
}


Sas64000Gpi::~Sas64000Gpi()
{
  delete sas_device;
}


void Sas64000Gpi::processCommand(RDMacro *cmd)
{
  char str[9];
  char cmd_byte;

  switch(cmd->command()) {
      case RDMacro::ST:
	if((cmd->arg(1).toInt()<1)||(cmd->arg(1).toInt()>sas_inputs)||
	   (cmd->arg(2).toInt()<1)||(cmd->arg(2).toInt()>sas_outputs)) {
	  cmd->acknowledge(false);
	  emit rmlEcho(cmd);
	  return;
	}
	sprintf(str,"\xFE%c%c%c%c%c\xFF",
		(char)((cmd->arg(2).toInt()-1)/16)&0x0F,
		(char)((cmd->arg(2).toInt()-1)%16)&0x0F,
		(char)((cmd->arg(1).toInt()-1)%128),
		(char)((cmd->arg(1).toInt()-1)/128),
		(char)(((cmd->arg(1).toInt()-1)%128)+
		((cmd->arg(1).toInt()-1)/128))&0x7F);
	sas_device->writeBlock(str,7);
	cmd->acknowledge(true);
	emit rmlEcho(cmd);
	break;

      case RDMacro::GO:
	if((cmd->arg(1).toString().lower()!="o")||
	   (cmd->arg(2).toInt()<1)||(cmd->arg(2).toInt()>sas_gpos)) {
	  cmd->acknowledge(false);
	  emit rmlEcho(cmd);
	  return;
	}
	if(cmd->arg(4).toInt()==0) {   // Latch
	  if(cmd->arg(3).toInt()==0) {   // Off
	    cmd_byte=0xF9;
	    emit gpoChanged(sas_matrix,cmd->arg(2).toInt()-1,false);
	  }
	  else {
	    cmd_byte=0xFA;
	    emit gpoChanged(sas_matrix,cmd->arg(2).toInt()-1,true);
	  }
	}
	else {  // Pulse
	  if(cmd->arg(3).toInt()==0) {
	    cmd->acknowledge(false);
	    emit rmlEcho(cmd);
	    return;
	  }
	  cmd_byte=0xFB;
	  sas_gpo_oneshot->start((void *)(cmd->arg(2).toInt()-1),500);
	  emit gpoChanged(sas_matrix,cmd->arg(2).toInt()-1,true);
	}
	sprintf(str,"%c%c\xFF",cmd_byte,cmd->arg(2).toInt()-1);
	sas_device->writeBlock(str,3);
	cmd->acknowledge(true);
	emit rmlEcho(cmd);
	break;

      default:
	cmd->acknowledge(false);
	emit rmlEcho(cmd);
	break;
  }
}


void Sas64000Gpi::gpoOneshotData(void *data)
{
  emit gpoChanged(sas_matrix,(long)data,false);
}
