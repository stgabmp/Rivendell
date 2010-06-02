// livewire.cpp
//
// A Rivendell switcher driver for LiveWire networks.
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: livewire.cpp,v 1.4.2.1 2009/02/24 23:40:08 cvs Exp $
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
#include <rddb.h>
#include <globals.h>
#include <livewire.h>


LiveWire::LiveWire(RDMatrix *matrix,QObject *parent,const char *name)
  : QObject(parent,name)
{
  QString sql;
  RDSqlQuery *q;

  //
  // Get Matrix Parameters
  //
  livewire_stationname=rdstation->name();
  livewire_matrix=matrix->matrix();

  //
  // Interval Oneshots
  //
  livewire_gpi_oneshot=new RDOneShot(this);
  connect(livewire_gpi_oneshot,SIGNAL(timeout(void *)),
	  this,SLOT(gpiOneshotData(void *)));
  livewire_gpo_oneshot=new RDOneShot(this);
  connect(livewire_gpo_oneshot,SIGNAL(timeout(void *)),
	  this,SLOT(gpoOneshotData(void *)));
  
  //
  // Load The Node List
  //
  sql=QString().sprintf("select HOSTNAME,TCP_PORT,PASSWORD,BASE_OUTPUT \
                         from SWITCHER_NODES where (STATION_NAME=\"%s\")&&\
                         (MATRIX=%d)",(const char *)livewire_stationname,
			livewire_matrix);
  q=new RDSqlQuery(sql);
  while(q->next()) {
    livewire_nodes.push_back(new RDLiveWire(livewire_nodes.size(),this));
    connect(livewire_nodes.back(),SIGNAL(connected(unsigned)),
	    this,SLOT(nodeConnectedData(unsigned)));
    connect(livewire_nodes.back(),
	    SIGNAL(sourceChanged(unsigned,RDLiveWireSource *)),
	    this,
	    SLOT(sourceChangedData(unsigned,RDLiveWireSource *)));
    connect(livewire_nodes.back(),
	    SIGNAL(destinationChanged(unsigned,RDLiveWireDestination *)),
	    this,
	    SLOT(destinationChangedData(unsigned,RDLiveWireDestination *)));
    connect(livewire_nodes.back(),
	    SIGNAL(gpoConfigChanged(unsigned,unsigned,unsigned)),
	    this,
	    SLOT(gpoConfigChangedData(unsigned,unsigned,unsigned)));
    connect(livewire_nodes.back(),
	    SIGNAL(gpiChanged(unsigned,unsigned,unsigned,bool)),
	    this,
	    SLOT(gpiChangedData(unsigned,unsigned,unsigned,bool)));
    connect(livewire_nodes.back(),
	    SIGNAL(gpoChanged(unsigned,unsigned,unsigned,bool)),
	    this,
	    SLOT(gpoChangedData(unsigned,unsigned,unsigned,bool)));
    connect(livewire_nodes.back(),
	    SIGNAL(watchdogStateChanged(unsigned,const QString &)),
	    this,SLOT(watchdogStateChangedData(unsigned,const QString &)));
    livewire_nodes.back()->connectToHost(q->value(0).toString(),
					 q->value(1).toInt(),
					 q->value(2).toString(),
					 q->value(3).toUInt());
  }
  delete q;
}


LiveWire::~LiveWire()
{
  for(unsigned i=0;i<livewire_nodes.size();i++) {
    delete livewire_nodes[i];
  }
  livewire_nodes.clear();
}


void LiveWire::processCommand(RDMacro *cmd)
{
  QString label;
  unsigned input;
  unsigned output;
  unsigned line;
  unsigned dest_slot=0;
  unsigned dest_line=0;
  RDLiveWire *node=NULL;
  RDLiveWire *dest_node=NULL;

  switch(cmd->command()) {
      case RDMacro::ST:
	input=cmd->arg(1).toUInt();
	if(input>RD_LIVEWIRE_MAX_SOURCE) {
	  cmd->acknowledge(false);
	  emit rmlEcho(cmd);
	  return;
	}
	output=cmd->arg(2).toUInt();
	if(output>RD_LIVEWIRE_MAX_SOURCE) {
	  cmd->acknowledge(false);
	  emit rmlEcho(cmd);
	  return;
	}

	//
	// Find the destination node
	//
	for(unsigned i=0;i<livewire_nodes.size();i++) {
	  node=livewire_nodes[i];
	  if((output>=node->baseOutput())&&
	     (output<(node->baseOutput()+node->destinations()))) {
	    dest_node=node;
	  }
	}
	if(dest_node==NULL) {  // No such output!
	  cmd->acknowledge(false);
	  emit rmlEcho(cmd);
	  return;
	}
	dest_node->setRoute(input,output-dest_node->baseOutput());
	cmd->acknowledge(true);
	emit rmlEcho(cmd);
	break;

      case RDMacro::GO:
	if((cmd->argQuantity()!=5)||
	   ((cmd->arg(1).toString().lower()!="i")&&
	    (cmd->arg(1).toString().lower()!="o"))) {
	  cmd->acknowledge(false);
	  emit rmlEcho(cmd);
	  return;
	}

	line=cmd->arg(2).toUInt();
	if(line>RD_LIVEWIRE_MAX_SOURCE) {
	  cmd->acknowledge(false);
	  emit rmlEcho(cmd);
	  return;
	}

	if(cmd->arg(1).toString().lower()=="i") {
	  //
	  // Find the destination node
	  //
	  for(unsigned i=0;i<livewire_nodes.size();i++) {
	    node=livewire_nodes[i];
	    for(int j=0;j<node->gpos();j++) {
	      for(int k=0;k<RD_LIVEWIRE_GPIO_BUNDLE_SIZE;k++) {
		if(node->gpiChannel(j,k)==line) {
		  dest_node=node;
		  dest_slot=j;
		  dest_line=k;
		}
	      }
	    }
	  }
	  if(dest_node==NULL) {  // No such GPI!
	    cmd->acknowledge(false);
	    emit rmlEcho(cmd);
	    return;
	  }
	  
	  if(cmd->arg(3).toInt()==0) {  // GPI Off
	    dest_node->gpiReset(dest_slot,dest_line,cmd->arg(4).toUInt());
	    emit gpiChanged(livewire_matrix,line-1,false);
	  }
	  else {
	    dest_node->gpiSet(dest_slot,dest_line,cmd->arg(4).toUInt());
	    if(cmd->arg(4).toUInt()>0) {
	      livewire_gpi_oneshot->start((void *)(line-1),500);
	    }
	    emit gpiChanged(livewire_matrix,line-1,true);
	  }
	}
	if(cmd->arg(1).toString().lower()=="o") {
	  //
	  // Find the destination node
	  //
	  for(unsigned i=0;i<livewire_nodes.size();i++) {
	    node=livewire_nodes[i];
	    for(int j=0;j<node->gpos();j++) {
	      for(int k=0;k<RD_LIVEWIRE_GPIO_BUNDLE_SIZE;k++) {
		LogLine(RDConfig::LogErr,QString().sprintf("LINE: %d\n",node->gpoChannel(j,k)));
		if(node->gpoChannel(j,k)==line) {
		  dest_node=node;
		  dest_slot=j;
		  dest_line=k;
		}
	      }
	    }
	  }
	  if(dest_node==NULL) {  // No such GPO!
	    cmd->acknowledge(false);
	    emit rmlEcho(cmd);
	    return;
	  }
	  
	  if(cmd->arg(3).toInt()==0) {  // GPO Off
	    dest_node->gpoReset(dest_slot,dest_line,cmd->arg(4).toUInt());
	    emit gpoChanged(livewire_matrix,line-1,false);
	  }
	  else {
	    dest_node->gpoSet(dest_slot,dest_line,cmd->arg(4).toUInt());
	    if(cmd->arg(4).toUInt()>0) {
	      livewire_gpo_oneshot->start((void *)(line-1),500);
	    }
	    emit gpoChanged(livewire_matrix,line-1,true);
	  }
	}
	cmd->acknowledge(true);
	emit rmlEcho(cmd);
	break;

      default:
	cmd->acknowledge(false);
	emit rmlEcho(cmd);
	break;
  }
}


void LiveWire::sendGpi()
{
  for(unsigned i=0;i<livewire_nodes.size();i++) {
    for(int j=0;j<livewire_nodes[i]->gpis();j++) {
      for(int k=0;k<RD_LIVEWIRE_GPIO_BUNDLE_SIZE;k++) {
	emit gpiState(livewire_matrix,livewire_nodes[i]->gpiChannel(j,k)-1,
		      livewire_nodes[i]->gpiState(j,k));
      }
    }
  }
}


void LiveWire::sendGpo()
{
  for(unsigned i=0;i<livewire_nodes.size();i++) {
    for(int j=0;j<livewire_nodes[i]->gpos();j++) {
      for(int k=0;k<RD_LIVEWIRE_GPIO_BUNDLE_SIZE;k++) {
	emit gpoState(livewire_matrix,livewire_nodes[i]->gpoChannel(j,k)-1,
		      livewire_nodes[i]->gpoState(j,k));
      }
    }
  }
}


void LiveWire::nodeConnectedData(unsigned id)
{
  LogLine(RDConfig::LogInfo,QString().
	  sprintf("connection established to LiveWire node at \"%s\"",
		  (const char *)livewire_nodes[id]->hostname()));
}


void LiveWire::sourceChangedData(unsigned id,RDLiveWireSource *src)
{
  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("delete from INPUTS where \
                         (STATION_NAME=\"%s\")&&\
                         (MATRIX=%d)&&\
                         (NODE_HOSTNAME=\"%s\")&&\
                         (NODE_TCP_PORT=%d)&&\
                         (NODE_SLOT=%d)",
			(const char *)livewire_stationname,
			livewire_matrix,
			(const char *)livewire_nodes[id]->hostname(),
			livewire_nodes[id]->tcpPort(),
			src->slotNumber());
  q=new RDSqlQuery(sql);
  delete q;

  if(src->rtpEnabled()) {
    sql=QString().sprintf("insert into INPUTS set \
                           STATION_NAME=\"%s\",\
                           MATRIX=%d,\
                           NODE_HOSTNAME=\"%s\",\
                           NODE_TCP_PORT=%d,\
                           NODE_SLOT=%d,\
                           NAME=\"%s\",\
                           NUMBER=%d",
			  (const char *)livewire_stationname,
			  livewire_matrix,
			  (const char *)livewire_nodes[id]->hostname(),
			  livewire_nodes[id]->tcpPort(),
			  src->slotNumber(),
			  (const char *)src->primaryName(),
			  src->channelNumber());
    q=new RDSqlQuery(sql);
    delete q;
  }
}


void LiveWire::destinationChangedData(unsigned id,RDLiveWireDestination *dst)
{
  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("delete from OUTPUTS where \
                         (STATION_NAME=\"%s\")&&\
                         (MATRIX=%d)&&\
                         (NODE_HOSTNAME=\"%s\")&&\
                         (NODE_TCP_PORT=%d)&&\
                         (NODE_SLOT=%d)",
			(const char *)livewire_stationname,
			livewire_matrix,
			(const char *)livewire_nodes[id]->hostname(),
			livewire_nodes[id]->tcpPort(),
			dst->slotNumber());
  q=new RDSqlQuery(sql);
  delete q;

  sql=QString().sprintf("insert into OUTPUTS set \
                         STATION_NAME=\"%s\",\
                         MATRIX=%d,\
                         NODE_HOSTNAME=\"%s\",\
                         NODE_TCP_PORT=%d,\
                         NODE_SLOT=%d,\
                         NAME=\"%s\",\
                         NUMBER=%d",
			(const char *)livewire_stationname,
			livewire_matrix,
			(const char *)livewire_nodes[id]->hostname(),
			livewire_nodes[id]->tcpPort(),
			dst->slotNumber(),
			(const char *)dst->primaryName(),
			livewire_nodes[id]->baseOutput()+dst->slotNumber()-1);
  q=new RDSqlQuery(sql);
  delete q;
}


void LiveWire::gpoConfigChangedData(unsigned id,unsigned slot,unsigned chan)
{
  //
  // FIXME: This creates entries fine, but how can we delete stale ones
  //        without trashing user configs?
  //
  CreateGpioEntry("GPIS",chan);
  CreateGpioEntry("GPOS",chan);
}


void LiveWire::gpiChangedData(unsigned id,unsigned slot,unsigned line,
			      bool state)
{
  emit gpiChanged(livewire_matrix,livewire_nodes[id]->gpiChannel(slot,line)-1,
		  state);
}


void LiveWire::gpoChangedData(unsigned id,unsigned slot,unsigned line,
			      bool state)
{
  // printf("gpoChanged(%d,%d,%d)\n",livewire_matrix,livewire_nodes[id]->gpoChannel(slot,line)-1,state);
  emit gpoChanged(livewire_matrix,livewire_nodes[id]->gpoChannel(slot,line)-1,
		  state);
}


void LiveWire::watchdogStateChangedData(unsigned id,const QString &msg)
{
  LogLine(RDConfig::LogNotice,msg);
}


void LiveWire::gpiOneshotData(void *data)
{
  emit gpiChanged(livewire_matrix,(long)data,false);
}


void LiveWire::gpoOneshotData(void *data)
{
  emit gpoChanged(livewire_matrix,(long)data,false);
}


void LiveWire::CreateGpioEntry(const QString &tablename,int chan)
{
  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("select ID from %s where \
                         (STATION_NAME=\"%s\")&&\
                         (MATRIX=%d)&&\
                         (NUMBER=%d)",
			(const char *)tablename,
			(const char *)livewire_stationname,
			livewire_matrix,
			chan);
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    sql=QString().sprintf("insert into %s set \
                           STATION_NAME=\"%s\",\
                           MATRIX=%d,\
                           NUMBER=%d",
			  (const char *)tablename,
			  (const char *)livewire_stationname,
			  livewire_matrix,
			  chan);
    q=new RDSqlQuery(sql);
  }
  delete q;
}
