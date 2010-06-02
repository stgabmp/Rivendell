// rlmhost.cpp
//
// A container class for a Rivendell Loadable Module host.
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rlmhost.cpp,v 1.1.2.5 2009/02/23 12:54:32 cvs Exp $
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

#include <dlfcn.h>

#include <rdconf.h>
#include <rdprofile.h>
#include <rdnownext.h>

#include <globals.h>
#include <rlmhost.h>


RLMHost::RLMHost(const QString &path,const QString &arg,
		 QSocketDevice *udp_socket,QObject *parent,const char *name)
  : QObject(parent,name)
{
  plugin_path=path;
  plugin_arg=arg;
  plugin_udp_socket=udp_socket;
  plugin_handle=NULL;
  plugin_start_sym=NULL;
  plugin_free_sym=NULL;
  plugin_pad_data_sent_sym=NULL;
  plugin_timer_expired_sym=NULL;

  //
  // Utility Timers
  //
  QSignalMapper *mapper=new QSignalMapper(this);
  connect(mapper,SIGNAL(mapped(int)),this,SLOT(timerData(int)));
  for(int i=0;i<RLM_MAX_TIMERS;i++) {
    plugin_callback_timers[i]=new QTimer(this);
    mapper->setMapping(plugin_callback_timers[i],i);
    connect(plugin_callback_timers[i],SIGNAL(timeout()),mapper,SLOT(map()));
  }
}


RLMHost::~RLMHost()
{
}


QString RLMHost::pluginPath() const
{
  return plugin_path;
}


QString RLMHost::pluginArg() const
{
  return plugin_arg;
}


void RLMHost::sendEvent(const QString &svcname,int lognum,RDLogLine **loglines,
			bool onair)
{
  if(plugin_pad_data_sent_sym!=NULL) {
    struct rlm_pad now;
    struct rlm_pad next;
    RLMHost::loadMetadata(loglines[0],&now);
    RLMHost::loadMetadata(loglines[1],&next); 
    plugin_pad_data_sent_sym(this,svcname,(int)onair,lognum,&now,&next);
  }
}


bool RLMHost::load()
{
  QString basename=RDGetBasePart(plugin_path);
  basename=basename.left(basename.findRev("."));
  if((plugin_handle=dlopen(plugin_path,RTLD_LAZY))==NULL) {
    return false;
  }
  *(void **)(&plugin_start_sym)=dlsym(plugin_handle,basename+"_RLMStart");
  *(void **)(&plugin_free_sym)=dlsym(plugin_handle,basename+"_RLMFree");
  *(void **)(&plugin_pad_data_sent_sym)=
    dlsym(plugin_handle,basename+"_RLMPadDataSent");
  *(void **)(&plugin_timer_expired_sym)=
    dlsym(plugin_handle,basename+"_RLMTimerExpired");
  if(plugin_start_sym!=NULL) {
    plugin_start_sym(this,plugin_arg);
  }
  return true;
}


void RLMHost::unload()
{
  if(plugin_free_sym!=NULL) {
    plugin_free_sym(this);
  }
}


void RLMHost::loadMetadata(const RDLogLine *logline,struct rlm_pad *pad)
{
  if(pad==NULL) {
    return;
  }
  memset(pad,0,sizeof(struct rlm_pad));
  if(logline==NULL) {
    return;
  }
  if(logline!=NULL) {
    pad->rlm_cartnum=logline->cartNumber();
    pad->rlm_len=logline->effectiveLength();
    if(!logline->year().isNull()) {
      sprintf(pad->rlm_year,"%s",
	      (const char *)logline->year().toString("YYYY").left(4));
    }
    if(!logline->groupName().isEmpty()) {
      sprintf(pad->rlm_group,"%s",(const char *)logline->groupName().left(10));
    }
    if(!logline->title().isEmpty()) {
      sprintf(pad->rlm_title,"%s",(const char *)logline->title().left(255));
    }
    if(!logline->artist().isEmpty()) {
      sprintf(pad->rlm_artist,"%s",(const char *)logline->artist().left(255));
    }
    if(!logline->label().isEmpty()) {
      sprintf(pad->rlm_label,"%s",(const char *)logline->label().left(64));
    }
    if(!logline->client().isEmpty()) {
      sprintf(pad->rlm_client,"%s",(const char *)logline->client().left(64));
    }
    if(!logline->agency().isEmpty()) {
      sprintf(pad->rlm_agency,"%s",(const char *)logline->agency().left(64));
    }
    if(!logline->composer().isEmpty()) {
      sprintf(pad->rlm_comp,"%s",(const char *)logline->composer().left(64));
    }
    if(!logline->publisher().isEmpty()) {
      sprintf(pad->rlm_pub,"%s",(const char *)logline->publisher().left(64));
    }
    if(!logline->userDefined().isEmpty()) {
      sprintf(pad->rlm_userdef,"%s",
	      (const char *)logline->userDefined().left(255));
    }
    if(!logline->album().isEmpty()) {
      sprintf(pad->rlm_album,"%s",(const char *)logline->album().left(255));
    }
  }
}


void RLMHost::saveMetadata(const struct rlm_pad *pad,RDLogLine *logline)
{
  if(logline==NULL) {
    return;
  }
  logline->clear();
  if(pad==NULL) {
    return;
  }
  logline->setCartNumber(pad->rlm_cartnum);
  logline->setForcedLength(pad->rlm_len);
  logline->setYear(QDate(QString(pad->rlm_year).toInt(),1,1));
  logline->setGroupName(pad->rlm_group);
  logline->setTitle(pad->rlm_title);
  logline->setArtist(pad->rlm_artist);
  logline->setLabel(pad->rlm_label);
  logline->setClient(pad->rlm_client);
  logline->setAgency(pad->rlm_agency);
  logline->setComposer(pad->rlm_comp);
  logline->setPublisher(pad->rlm_pub);
  logline->setUserDefined(pad->rlm_userdef);
  logline->setAlbum(pad->rlm_album);
}


void RLMHost::timerData(int timernum)
{
  if(plugin_timer_expired_sym!=NULL) {
    plugin_timer_expired_sym(this,timernum);
  }
}


//
// RLM Utility Functions
//
void RLMSendUdp(void *ptr,const char *ipaddr,uint16_t port,
		const char *data,int len)
{
  RLMHost *host=(RLMHost *)ptr;
  QHostAddress addr;
  addr.setAddress(ipaddr);
  if(!addr.isNull()) {
    host->plugin_udp_socket->writeBlock(data,len,addr,port);
  }
}


int RLMOpenSerial(void *ptr,const char *port,int speed,int parity,
		  int word_length)
{
  RLMHost *host=(RLMHost *)ptr;
  host->plugin_tty_devices.push_back(new RDTTYDevice);
  host->plugin_tty_devices.back()->setName(port);
  host->plugin_tty_devices.back()->setSpeed(speed);
  host->plugin_tty_devices.back()->setParity((RDTTYDevice::Parity)parity);
  host->plugin_tty_devices.back()->setWordLength(word_length);
  if(host->plugin_tty_devices.back()->open(IO_ReadWrite)) {
    return (int)host->plugin_tty_devices.size()-1;
  }
  return -1;
}


void RLMSendSerial(void *ptr,int handle,const char *data,int len)
{
  RLMHost *host=(RLMHost *)ptr;
  if((handle<0)||(handle>=host->plugin_tty_devices.size())) {
    return;
  }
  host->plugin_tty_devices[handle]->writeBlock(data,len);
}


void RLMCloseSerial(void *ptr,int handle)
{
  RLMHost *host=(RLMHost *)ptr;

  //
  // FIXME: We really ought to take out the trash here!
  //
  host->plugin_tty_devices[handle]->close();
  delete host->plugin_tty_devices[handle];
  host->plugin_tty_devices[handle]=NULL;
}


const char *RLMDateTime(void *ptr,int offset_msecs,const char *format)
{
  RLMHost *host=(RLMHost *)ptr;
  QDateTime datetime=QDateTime(QDate::currentDate(),QTime::currentTime().
			       addMSecs(offset_msecs));
  strncpy(host->plugin_value_string,datetime.toString(format),1024);
  return host->plugin_value_string;
}


const char *RLMResolveNowNext(void *ptr,const struct rlm_pad *now,
			      const struct rlm_pad *next,const char *format)
{
  RLMHost *host=(RLMHost *)ptr;
  RDLogLine *loglines[2];
  QString str=format;

  loglines[0]=new RDLogLine();
  loglines[1]=new RDLogLine();
  RLMHost::saveMetadata(now,loglines[0]);
  RLMHost::saveMetadata(next,loglines[1]);
  RDResolveNowNext(&str,loglines);
  strncpy(host->plugin_value_string,str,1024);
  delete loglines[1];
  delete loglines[0];

  return host->plugin_value_string;
}


void RLMLog(void *ptr,int prio,const char *msg)
{
  LogLine((RDConfig::LogPriority)prio,msg);
}


void RLMStartTimer(void *ptr,int timernum,int msecs,int mode)
{
  RLMHost *host=(RLMHost *)ptr;
  if((timernum<0)||(timernum>=RLM_MAX_TIMERS)) {
    return;
  }
  if(host->plugin_callback_timers[timernum]->isActive()) {
    host->plugin_callback_timers[timernum]->stop();
  }
  host->plugin_callback_timers[timernum]->start(msecs,mode);
}


void RLMStopTimer(void *ptr,int timernum)
{
  RLMHost *host=(RLMHost *)ptr;
  if((timernum<0)||(timernum>=RLM_MAX_TIMERS)) {
    return;
  }
  if(host->plugin_callback_timers[timernum]->isActive()) {
    host->plugin_callback_timers[timernum]->stop();
  }
}


int RLMGetIntegerValue(void *ptr,const char *filename,const char *section,
		       const char *label,int default_value)
{
  RDProfile *p=new RDProfile();
  p->setSource(filename);
  int r=p->intValue(section,label,default_value);
  delete p;
  return r;
}


int RLMGetHexValue(void *ptr,const char *filename,const char *section,
		   const char *label,int default_value)
{
  RDProfile *p=new RDProfile();
  p->setSource(filename);
  int r=p->hexValue(section,label,default_value);
  delete p;
  return r;
}


int RLMGetBooleanValue(void *ptr,const char *filename,const char *section,
		       const char *label,int default_value)
{
  RDProfile *p=new RDProfile();
  p->setSource(filename);
  bool r=p->boolValue(section,label,default_value);
  delete p;
  return (int)r;
}


const char *RLMGetStringValue(void *ptr,const char *filename,
			      const char *section,const char *label,
			      const char *default_value)
{
  RLMHost *host=(RLMHost *)ptr;
  RDProfile *p=new RDProfile();
  p->setSource(filename);
  strncpy(host->plugin_value_string,
	  p->stringValue(section,label,default_value),1024);
  delete p;
  return host->plugin_value_string;
}
