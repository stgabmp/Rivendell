// local_gpio.cpp
//
// A Rivendell switcher driver for MeasurementComputing GPIO cards.
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: local_gpio.cpp,v 1.11 2008/10/01 13:10:15 fredg Exp $
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
#include <local_gpio.h>


LocalGpio::LocalGpio(RDMatrix *matrix,QObject *parent,const char *name)
  : QObject(parent,name)
{
  //
  // Get Matrix Parameters
  //
  gpio_matrix=matrix->matrix();
  gpio_gpis=matrix->gpis();
  gpio_gpos=matrix->gpos();

  //
  // Initialize data structures
  //
  for(int i=0;i<GPIO_MAX_LINES;i++) {
    gpio_gpi_mask[i]=false;
  }

  //
  // Initialize the card
  //
  gpio_gpio=new RDGpio(this,"gpio_gpio");
  gpio_gpio->setDevice(matrix->gpioDevice());
  if(gpio_open=gpio_gpio->open()) {
    if(gpio_gpis==0) {
      gpio_gpio->setMode(RDGpio::Output);
    }
    if(gpio_gpos==0) {
      gpio_gpio->setMode(RDGpio::Input);
    }
    connect(gpio_gpio,SIGNAL(inputChanged(int,bool)),
	    this,SLOT(gpiChangedData(int,bool)));
    connect(gpio_gpio,SIGNAL(outputChanged(int,bool)),
	    this,SLOT(gpoChangedData(int,bool)));
  }

  //
  // Interval OneShot
  //
  gpio_gpi_oneshot=new RDOneShot(this);
  connect(gpio_gpi_oneshot,SIGNAL(timeout(void *)),
	  this,SLOT(gpiOneshotData(void*)));
}


LocalGpio::~LocalGpio()
{
  delete gpio_gpio;
  delete gpio_gpi_oneshot;
}


void LocalGpio::processCommand(RDMacro *cmd)
{
  switch(cmd->command()) {
      case RDMacro::GO:
	if((!gpio_open)||(cmd->argQuantity()!=5)||
	   ((cmd->arg(1).toString().lower()!="i")&&
	    (cmd->arg(1).toString().lower()!="o"))||
	   (cmd->arg(2).toInt()<1)||(cmd->arg(2).toInt()>gpio_gpos)||
	   ((cmd->arg(3).toInt()!=1)&&(cmd->arg(3).toInt()!=0)&&
	    (cmd->arg(3).toInt()!=-1))||(cmd->arg(4).toInt()<0)) {
	  cmd->acknowledge(false);
	  emit rmlEcho(cmd);
	  return;
	}
	if(cmd->arg(1).toString().lower()=="i") {
	  if(cmd->arg(3).toInt()==0) {
	    emit gpiChanged(gpio_matrix,cmd->arg(2).toInt()-1,false);
	    gpio_gpi_mask[cmd->arg(2).toInt()-1]=true;
	    if(cmd->arg(4).toInt()>0) {
	      gpio_gpi_oneshot->
		start((void *)(cmd->arg(2).toInt()-1),cmd->arg(4).toInt());
	    }
	  }
	  else {
	    if(cmd->arg(3).toInt()==1) {
	      emit gpiChanged(gpio_matrix,cmd->arg(2).toInt()-1,true);
	      gpio_gpi_mask[cmd->arg(2).toInt()-1]=true;
	      if(cmd->arg(4).toInt()>0) {
		gpio_gpi_oneshot->
		  start((void *)(cmd->arg(2).toInt()-1),cmd->arg(4).toInt());
	      }
	    }
	    else {
	      gpiOneshotData((void *)(cmd->arg(2).toInt()-1));
	    }
	  }
	  cmd->acknowledge(true);
	  emit rmlEcho(cmd);
	  return;
	}
	if(cmd->arg(1).toString().lower()=="o") {
	  if(cmd->arg(3).toInt()==0) {
	    gpio_gpio->gpoReset(cmd->arg(2).toInt()-1,cmd->arg(4).toInt());
	  }
	  else {
	    if(cmd->arg(3).toInt()==1) {
	      gpio_gpio->gpoSet(cmd->arg(2).toInt()-1,cmd->arg(4).toInt());
	    }
	    else {
	      cmd->acknowledge(false);
	      emit rmlEcho(cmd);
	      return;
	    }
	  }
	  cmd->acknowledge(true);
	  emit rmlEcho(cmd);
	  return;
	}
	break;

      default:
	cmd->acknowledge(false);
	emit rmlEcho(cmd);
	break;
  }
}


void LocalGpio::gpiChangedData(int line,bool state)
{
  emit gpiChanged(gpio_matrix,line,state);
}


void LocalGpio::gpoChangedData(int line,bool state)
{
  emit gpoChanged(gpio_matrix,line,state);
}


void LocalGpio::gpiOneshotData(void *data)
{
  gpio_gpi_mask[(long)data]=false;
  gpiChangedData((long)data,gpio_gpio->inputState((long)data));
}
