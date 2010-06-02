// rdcut.cpp
//
// Abstract a Rivendell Cut.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcut.cpp,v 1.67.2.5 2010/01/13 23:56:47 cvs Exp $
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

#ifndef WIN32
#include <unistd.h>
#include <syslog.h>
#endif  // WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <rd.h>
#include <rdconf.h>
#ifndef WIN32
#include <rdwavefile.h>
#endif

#include <rdcut.h>
#include <rdtextvalidator.h>
#include <rdconfig.h>
#include <rddb.h>
#include <rdescape_string.h>

//
// Global Classes
//
RDCut::RDCut(const QString &name,bool create,QSqlDatabase *db)
{
  RDSqlQuery *q;
  QString sql;

  cut_db=db;
  cut_name=name;

  cut_signal=new QSignal();

  if(create) {
    sql=QString().sprintf("INSERT INTO CUTS SET CUT_NAME=\"%s\"",
			  (const char *)cut_name);
    q=new RDSqlQuery(sql,cut_db);
    delete q;
  }
  sscanf((const char *)name+7,"%u",&cut_number);
}


RDCut::RDCut(unsigned cartnum,int cutnum,bool create,QSqlDatabase *db)
{
  RDSqlQuery *q;
  QString sql;

  cut_db=db;
  cut_name=QString().sprintf("%06u_%03d",cartnum,cutnum);

  cut_signal=new QSignal();

  if(create) {
    sql=QString().sprintf("INSERT INTO CUTS SET CUT_NAME=\"%s\"",
			  (const char *)cut_name);
    q=new RDSqlQuery(sql,cut_db);
    delete q;
  }
  cut_number=cutnum;
}


RDCut::~RDCut()
{
  delete cut_signal;
}


bool RDCut::exists() const
{
  return RDDoesRowExist("CUTS","CUT_NAME",cut_name,cut_db);
}


bool RDCut::audioExists() const
{
  if(!exists()) {
    return false;
  }
#ifndef WIN32
  RDWaveFile *wave=new RDWaveFile(RDCut::pathName(cut_name)); 
  if(!wave->openWave()) {
    delete wave;
    return false;
  }
  delete wave;
#endif
  return true;
}


bool RDCut::isValid() const
{
  return isValid(QDateTime(QDate::currentDate(),QTime::currentTime()));
}


bool RDCut::isValid(const QTime &time) const
{
  return isValid(QDateTime(QDate::currentDate(),time));
}


bool RDCut::isValid(const QDateTime &datetime) const
{
  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("select MON,TUE,WED,THU,FRI,SAT,SUN,EVERGREEN,\
                         START_DATETIME,END_DATETIME,START_DAYPART,END_DAYPART\
                         from CUTS where CUT_NAME=\"%s\"",
			(const char *)cut_name);
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return false;
  }
  if(q->value(7).toString()=="Y") {   // Evergreen
    delete q;
    return true;
  }
  if(q->value(datetime.date().dayOfWeek()-1).toString()!="Y") {  // Day of Week
    delete q;
    return false;
  }
  if((!q->value(8).isNull())&&(q->value(8).toDateTime()>datetime)) {
    delete q;
    return false;
  }
  if((!q->value(9).isNull())&&(q->value(9).toDateTime()<datetime)) {
    delete q;
    return false;
  }
  if((!q->value(10).isNull())&&(q->value(10).toTime()>datetime.time())) {
    delete q;
    return false;
  }
  if((!q->value(11).isNull())&&(q->value(11).toTime()<datetime.time())) {
    delete q;
    return false;
  }
  delete q;
  return true;
}


QString RDCut::cutName() const
{
  return cut_name;
}


unsigned RDCut::cutNumber() const
{
  return cut_number;
}


unsigned RDCut::cartNumber() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"CART_NUMBER",cut_db).
    toUInt();
}


void RDCut::setCartNumber(unsigned num) const
{
  SetRow("CART_NUMBER",num);
}


bool RDCut::evergreen() const
{
  return RDBool(RDGetSqlValue("CUTS","CUT_NAME",cut_name,"EVERGREEN").
	       toString());
}


void RDCut::setEvergreen(bool state) const
{
  SetRow("EVERGREEN",RDYesNo(state));
}


QString RDCut::description() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"DESCRIPTION",cut_db).
    toString();
}


void RDCut::setDescription(const QString &string) const
{
  SetRow("DESCRIPTION",string);
}


QString RDCut::outcue() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"OUTCUE",cut_db).toString();
}


void RDCut::setOutcue(const QString &string) const
{
  SetRow("OUTCUE",string);
}


QString RDCut::isrc(IsrcFormat fmt) const
{
  QString str= RDGetSqlValue("CUTS","CUT_NAME",cut_name,"ISRC",cut_db).
    toString();
  if((fmt==RDCut::RawIsrc)||(str.length()!=12)) {
    return str;
  }
  str.insert(2,"-");
  str.insert(6,"-");
  str.insert(9,"-");
  return str;
}


void RDCut::setIsrc(const QString &isrc) const
{
  SetRow("ISRC",isrc);
}


unsigned RDCut::length() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"LENGTH",cut_db).
    toUInt();
}


void RDCut::setLength(int length) const
{
  SetRow("LENGTH",length);
}


QDateTime RDCut::originDatetime(bool *valid) const
{
  return 
    RDGetSqlValue("CUTS","CUT_NAME",cut_name,"ORIGIN_DATETIME",cut_db,valid).
    toDateTime();
}


void RDCut::setOriginDatetime(const QDateTime &datetime) const
{
  SetRow("ORIGIN_DATETIME",datetime);
}


QDateTime RDCut::startDatetime(bool *valid) const
{
  return 
    RDGetSqlValue("CUTS","CUT_NAME",cut_name,"START_DATETIME",cut_db,valid).
    toDateTime();
}


void RDCut::setStartDatetime(const QDateTime &datetime,bool valid) const
{
  if(valid) {
    SetRow("START_DATETIME",datetime);
  }
  else {
    SetRow("START_DATETIME");
  }
}


QDateTime RDCut::endDatetime(bool *valid) const
{
  return 
    RDGetSqlValue("CUTS","CUT_NAME",cut_name,"END_DATETIME",cut_db,valid).
    toDateTime();
}


void RDCut::setEndDatetime(const QDateTime &datetime,bool valid) const
{
  if(valid) {
    SetRow("END_DATETIME",datetime);
  }
  else {
    SetRow("END_DATETIME");
  }
}


QTime RDCut::startDaypart(bool *valid) const
{
  return 
    RDGetSqlValue("CUTS","CUT_NAME",cut_name,"START_DAYPART",cut_db,valid).
    toTime();
}


void RDCut::setStartDaypart(const QTime &time,bool valid) const
{
  if(valid) {
    SetRow("START_DAYPART",time);
  }
  else {
    SetRow("START_DAYPART");
  }
}


bool RDCut::weekPart(int dayofweek) const
{
  return RDBool(RDGetSqlValue("CUTS","CUT_NAME",cut_name,
			    RDGetShortDayNameEN(dayofweek).upper(),cut_db).
	       toString());
}


void RDCut::setWeekPart(int dayofweek,bool state) const
{
  SetRow(RDGetShortDayNameEN(dayofweek).upper(),RDYesNo(state));
}


QTime RDCut::endDaypart(bool *valid) const
{
  return 
    RDGetSqlValue("CUTS","CUT_NAME",cut_name,"END_DAYPART",cut_db,valid).
    toTime();
}


void RDCut::setEndDaypart(const QTime &time,bool valid) const
{
  if(valid) {
    SetRow("END_DAYPART",time);
  }
  else {
    SetRow("END_DAYPART");
  }
}


QString RDCut::originName() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"ORIGIN_NAME",cut_db).
    toString();
}


void RDCut::setOriginName(const QString &name) const
{
  SetRow("ORIGIN_NAME",name);
}


unsigned RDCut::weight() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"WEIGHT",cut_db).
    toUInt();
}


void RDCut::setWeight(int value) const
{
  SetRow("WEIGHT",value);
}


QDateTime RDCut::lastPlayDatetime(bool *valid) const
{
  return 
    RDGetSqlValue("CUTS","CUT_NAME",cut_name,"LAST_PLAY_DATETIME",cut_db,valid).
    toDateTime();
}


void RDCut::setLastPlayDatetime(const QDateTime &datetime,bool valid) const
{
  if(valid) {
    SetRow("LAST_PLAY_DATETIME",datetime);
  }
  else {
    SetRow("LAST_PLAY_DATETIME");
  }
}


unsigned RDCut::playCounter() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"PLAY_COUNTER",cut_db).
    toUInt();
}


void RDCut::setPlayCounter(unsigned count) const
{
  SetRow("PLAY_COUNTER",count);
}


RDCut::Validity RDCut::validity() const
{
  return (RDCut::Validity)
    RDGetSqlValue("CUTS","CUT_NAME",cut_name,"VALIDITY").toUInt();
}


void RDCut::setValidity(RDCut::Validity state)
{
  SetRow("VALIDITY",(int)state);
}


unsigned RDCut::localCounter() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"LOCAL_COUNTER",cut_db).
    toUInt();
}


void RDCut::setLocalCounter(unsigned count) const
{
  SetRow("LOCAL_COUNTER",count);
}


unsigned RDCut::codingFormat() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"CODING_FORMAT",cut_db).
    toUInt();
}


void RDCut::setCodingFormat(unsigned format) const
{
  SetRow("CODING_FORMAT",format);
}


unsigned RDCut::sampleRate() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"SAMPLE_RATE",cut_db).
    toUInt();
}


void RDCut::setSampleRate(unsigned rate) const
{
  SetRow("SAMPLE_RATE",rate);
}


unsigned RDCut::bitRate() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"BIT_RATE",cut_db).
    toUInt();
}


void RDCut::setBitRate(unsigned rate) const
{
  SetRow("BIT_RATE",rate);
}


unsigned RDCut::channels() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"CHANNELS",cut_db).
    toUInt();
}


void RDCut::setChannels(unsigned chan) const
{
  SetRow("CHANNELS",chan);
}


int RDCut::playGain() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"PLAY_GAIN",cut_db).
    toInt();
}


void RDCut::setPlayGain(int gain) const
{
  SetRow("PLAY_GAIN",gain);
}


int RDCut::startPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"START_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"START_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return 0;
}


void RDCut::setStartPoint(int point) const
{
  SetRow("START_POINT",point);
}


int RDCut::endPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"END_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"END_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return (int)length();
}


void RDCut::setEndPoint(int point) const
{
  SetRow("END_POINT",point);
}


int RDCut::fadeupPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"FADEUP_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"FADEUP_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return 0;
}


void RDCut::setFadeupPoint(int point) const
{
  SetRow("FADEUP_POINT",point);
}


int RDCut::fadedownPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"FADEDOWN_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"FADEDOWN_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return effectiveEnd();
}


void RDCut::setFadedownPoint(int point) const
{
  SetRow("FADEDOWN_POINT",point);
}


int RDCut::segueStartPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"SEGUE_START_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"SEGUE_START_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return 0;
}


void RDCut::setSegueStartPoint(int point) const
{
  SetRow("SEGUE_START_POINT",point);
}


int RDCut::segueEndPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"SEGUE_END_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"SEGUE_END_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return effectiveEnd();
}


void RDCut::setSegueEndPoint(int point) const
{
  SetRow("SEGUE_END_POINT",point);
}


int RDCut::segueGain() const
{
  return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"SEGUE_GAIN",cut_db).toInt();
}


void RDCut::setSegueGain(int gain) const
{
  SetRow("SEGUE_GAIN",gain);
}


int RDCut::hookStartPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"HOOK_START_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"HOOK_START_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return 0;
}


void RDCut::setHookStartPoint(int point) const
{
  SetRow("HOOK_START_POINT",point);
}


int RDCut::hookEndPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"HOOK_END_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"HOOK_END_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return effectiveEnd();
}


void RDCut::setHookEndPoint(int point) const
{
  SetRow("HOOK_END_POINT",point);
}


int RDCut::talkStartPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"TALK_START_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"TALK_START_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return 0;
}


void RDCut::setTalkStartPoint(int point) const
{
  SetRow("TALK_START_POINT",point);
}


int RDCut::talkEndPoint(bool calc) const
{
  int n;

  if(!calc) {
    return RDGetSqlValue("CUTS","CUT_NAME",cut_name,"TALK_END_POINT",cut_db).
      toInt();
  }
  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"TALK_END_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return effectiveEnd();
}


void RDCut::setTalkEndPoint(int point) const
{
  SetRow("TALK_END_POINT",point);
}


int RDCut::effectiveStart() const
{
  int n;

  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"START_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return 0;
}


int RDCut::effectiveEnd() const
{
  int n;

  if((n=RDGetSqlValue("CUTS","CUT_NAME",cut_name,"END_POINT",cut_db).
      toInt())!=-1) {
    return n;
  }
  return (int)length();
}


void RDCut::logPlayout() const
{
  QString sql=
    QString().sprintf("update CUTS set LAST_PLAY_DATETIME=\"%s\",\
                       PLAY_COUNTER=%d,LOCAL_COUNTER=%d where CUT_NAME=\"%s\"",
		      (const char *)QDateTime(QDate::currentDate(),
		        QTime::currentTime()).toString("yyyy-MM-dd hh:mm:ss"),
		      playCounter()+1,localCounter()+1,(const char *)cut_name);
  RDSqlQuery *q=new RDSqlQuery(sql);
  delete q;
}


bool RDCut::copyTo(const QString &cutname) const
{
#ifndef WIN32
  QString sql;
  RDSqlQuery *q;

  //
  // Copy the Database Record
  //
  sql=
    QString().sprintf("select DESCRIPTION,OUTCUE,LENGTH,\
                       ORIGIN_DATETIME,ORIGIN_NAME,CODING_FORMAT,SAMPLE_RATE,\
                       BIT_RATE,CHANNELS,PLAY_GAIN,START_POINT,END_POINT,\
                       FADEUP_POINT,FADEDOWN_POINT,SEGUE_START_POINT,\
                       SEGUE_END_POINT,HOOK_START_POINT,HOOK_END_POINT,\
                       TALK_START_POINT,TALK_END_POINT from CUTS\
                       where CUT_NAME=\"%s\"",(const char *)cut_name);
  q=new RDSqlQuery(sql);
  if(q->first()) {
    sql=QString().sprintf("update CUTS set\
                           DESCRIPTION=\"%s\",\
                           OUTCUE=\"%s\",\
                           LENGTH=%u,\
                           ORIGIN_DATETIME=\"%s\",\
                           ORIGIN_NAME=\"%s\",\
                           CODING_FORMAT=%u,\
                           SAMPLE_RATE=%u,\
                           BIT_RATE=%u,\
                           CHANNELS=%u,\
                           PLAY_GAIN=%d,\
                           START_POINT=%d,\
                           END_POINT=%d,\
                           FADEUP_POINT=%d,\
                           FADEDOWN_POINT=%d,\
                           SEGUE_START_POINT=%d,\
                           SEGUE_END_POINT=%d,\
                           HOOK_START_POINT=%d,\
                           HOOK_END_POINT=%d,\
                           TALK_START_POINT=%d,\
                           TALK_END_POINT=%d where CUT_NAME=\"%s\"",
			  (const char *)q->value(0).toString().utf8(),
			  (const char *)q->value(1).toString().utf8(),
			  q->value(2).toUInt(),
			  (const char *)q->value(3).toString().utf8(),
			  (const char *)q->value(4).toString().utf8(),
			  q->value(5).toUInt(),
			  q->value(6).toUInt(),
			  q->value(7).toUInt(),
			  q->value(8).toUInt(),
			  q->value(9).toInt(),
			  q->value(10).toInt(),
			  q->value(11).toInt(),
			  q->value(12).toInt(),
			  q->value(13).toInt(),
			  q->value(14).toInt(),
			  q->value(15).toInt(),
			  q->value(16).toInt(),
			  q->value(17).toInt(),
			  q->value(18).toInt(),
			  q->value(19).toInt(),			  
			  (const char *)cutname);
  }
  delete q;
  q=new RDSqlQuery(sql);
  delete q;

  //
  // Copy the Audio
  //
  QString srcname=RDCut::pathName(cut_name); 
  QString destname=RDCut::pathName(cutname); 
  FileCopy(srcname,destname);

#endif
  return true;
}


void RDCut::getMetadata(RDWaveData *data)
{
  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("select DESCRIPTION,OUTCUE,ISRC,ORIGIN_DATETIME,\
                         START_DATETIME,END_DATETIME,SEGUE_START_POINT,\
                         SEGUE_END_POINT,TALK_START_POINT,TALK_END_POINT,\
                         START_POINT,END_POINT where CUT_NAME=\"%s\"",
			(const char *)cut_name);
  q=new RDSqlQuery(sql);
  if(q->first()) {
    data->setTitle(q->value(0).toString());
    data->setOutCue(q->value(1).toString());
    data->setIsrc(q->value(2).toString());
    data->setOriginationDate(q->value(3).toDate());
    data->setOriginationTime(q->value(3).toTime());
    data->setStartDate(q->value(4).toDate());
    data->setStartTime(q->value(4).toTime());
    data->setEndDate(q->value(5).toDate());
    data->setEndTime(q->value(5).toTime());
    data->setSegueStartPos(q->value(6).toInt());
    data->setSegueEndPos(q->value(7).toInt());
    data->setIntroStartPos(q->value(8).toInt());
    data->setIntroEndPos(q->value(9).toInt());
    data->setStartPos(q->value(10).toInt());
    data->setEndPos(q->value(11).toInt());
    data->setMetadataFound(true);
  }
  delete q;
}


void RDCut::setMetadata(RDWaveData *data)
{
  QString sql="update CUTS set ";
  if(!data->title().isEmpty()) {
    sql+=QString().sprintf("DESCRIPTION=\"%s\",",
	      (const char *)RDTextValidator::stripString(data->title()).utf8());
  }
  if(!data->outCue().isEmpty()) {
    sql+=QString().sprintf("OUTCUE=\"%s\",",
    (const char *)RDTextValidator::stripString(data->outCue()).utf8());
  }
  else {
    switch(data->endType()) {
	case RDWaveData::FadeEnd:
	  sql+="OUTCUE=\"[music fades]\",";
	  break;
	  
	case RDWaveData::ColdEnd:
	  sql+="OUTCUE=\"[music ends cold]\",";
	  break;
	  
	case RDWaveData::UnknownEnd:
	  break;
    }
  }
  if(!data->isrc().isEmpty()) {
    sql+=QString().sprintf("ISRC=\"%s\",",
    (const char *)RDTextValidator::stripString(data->isrc()).utf8());
  }
  if(data->startPos()>=0) {
    sql+=QString().sprintf("START_POINT=%d,",data->startPos());
  }
  if(data->endPos()>=0) {
    sql+=QString().sprintf("END_POINT=%d,",data->endPos());
  }
  if((data->introStartPos()==data->startPos())&&
     (data->introEndPos()==data->endPos())) {
    sql+="TALK_START_POINT=-1,TALK_END_POINT=-1,";
  }
  else {
    if(data->introStartPos()>=0) {
      if(data->introStartPos()<data->startPos()) {
	sql+=QString().sprintf("TALK_START_POINT=%d,",data->startPos());
      }
      else {
	sql+=QString().sprintf("TALK_START_POINT=%d,",data->introStartPos());
      }
    }
    if(data->introEndPos()>=0) {
      if(data->introEndPos()>data->endPos()) {
	sql+=QString().sprintf("TALK_END_POINT=%d,",data->endPos());
      }
      else {
	sql+=QString().sprintf("TALK_END_POINT=%d,",data->introEndPos());
      }
    }
  }
  if(((data->segueStartPos()==data->startPos())&&
      (data->segueEndPos()==data->endPos()))||(data->segueStartPos()==0)) {
    sql+="SEGUE_START_POINT=-1,SEGUE_END_POINT=-1,";
  }
  else {
    if(data->segueStartPos()>=0) {
      if(data->segueStartPos()<data->startPos()) {
	sql+=QString().sprintf("SEGUE_START_POINT=%d,",data->startPos());
      }
      else {
	sql+=QString().sprintf("SEGUE_START_POINT=%d,",data->segueStartPos());
      }
    }
    if(data->segueEndPos()>=0) {
      if(data->segueEndPos()>data->endPos()) {
	sql+=QString().sprintf("SEGUE_END_POINT=%d,",data->endPos());
      }
      else {
	sql+=QString().sprintf("SEGUE_END_POINT=%d,",data->segueEndPos());
      }
    }
  }
  if(data->startDate().isValid()&&data->endDate().isValid()&&
     (data->startTime().isNull())&&(data->endTime().isNull())) {
    data->setEndTime(QTime(23,59,59));
  }
  if((data->startDate()>QDate(1900,1,1))&&(data->endDate().year()<8000)) {
    if(data->startTime().isValid()) {
    sql+=QString().sprintf("START_DATETIME=\"%s %s\",",
			   (const char *)data->startDate().
			   toString("yyyy-MM-dd"),
			   (const char *)data->startTime().
			   toString("hh:mm:ss"));
    }
    else {
      sql+=QString().sprintf("START_DATETIME=\"%s 00:00:00\",",
			     (const char *)data->startDate().
			     toString("yyyy-MM-dd"));
    }
    if(data->endDate().isValid()&&(data->endDate().year()<8000)) {
      if(data->endTime().isValid()) {
	sql+=QString().sprintf("END_DATETIME=\"%s %s\",",
			       (const char *)data->endDate().
			       toString("yyyy-MM-dd"),
			       (const char *)data->endTime().
			       toString("hh:mm:ss"));
      }
      else {
	sql+=QString().sprintf("END_DATETIME=\"%s 23:59:59\",",
			       (const char *)data->endDate().
			       toString("yyyy-MM-dd"));
      }
    }
  }
  if(sql.right(1)==",") {
    sql=sql.left(sql.length()-1);
    sql+=QString().
      sprintf(" where CUT_NAME=\"%s\"",(const char *)cut_name.utf8());
    RDSqlQuery *q=new RDSqlQuery(sql);
    delete q;
  }
}


bool RDCut::checkInRecording(const QString &stationname) const
{
#ifdef WIN32
  return false;
#else
  QString sql;
  RDSqlQuery *q;

  RDWaveFile *wavefile=new RDWaveFile(RDCut::pathName(cut_name));
  if(!wavefile->openWave()) {
    syslog(LOG_WARNING,"RDCut::unable to check in file: %s",
	   (const char *)RDCut::pathName(cut_name));
    delete wavefile;
    return false;
  }
  int format=0;
  if(wavefile->getFormatTag()==WAVE_FORMAT_MPEG) {
    format=1;
  }
  sql=QString().sprintf("update CUTS set START_POINT=0,END_POINT=%d,\
                         FADEUP_POINT=-1,FADEDOWN_POINT=-1,\
                         SEGUE_START_POINT=-1,SEGUE_END_POINT=-1,\
                         TALK_START_POINT=-1,TALK_END_POINT=-1,\
                         HOOK_START_POINT=-1,HOOK_END_POINT=-1,\
                         PLAY_GAIN=0,PLAY_COUNTER=0,LOCAL_COUNTER=0,\
                         CODING_FORMAT=%d,SAMPLE_RATE=%d,\
                         BIT_RATE=%d,CHANNELS=%d,LENGTH=%d,\
                         ORIGIN_DATETIME=\"%s %s\",ORIGIN_NAME=\"%s\" \
                         where CUT_NAME=\"%s\"",
			wavefile->getExtTimeLength(),
			format,
			wavefile->getSamplesPerSec(),
			wavefile->getHeadBitRate(),
			wavefile->getChannels(),
			wavefile->getExtTimeLength(),
			(const char *)QDate::currentDate().
			toString("yyyy-MM-dd"),
			(const char *)QTime::currentTime().
			toString("hh:mm:ss"),
			(const char *)stationname,
			(const char *)cut_name);
  q=new RDSqlQuery(sql);
  delete q;
  wavefile->closeWave();
  delete wavefile;
  return true;
#endif  // WIN32
}


void RDCut::autoTrim(RDCut::AudioEnd end,int level)
{
#ifndef WIN32
  int point;
  int start_point=0;
  int end_point=-1;

  if(!exists()) {
    return;
  }
  QString wavename=RDCut::pathName(cut_name); 
  RDWaveFile *wave=new RDWaveFile(wavename);
  if(!wave->openWave()) {
    delete wave;
    return;
  }
  if(level>=0) {
    if((end==RDCut::AudioHead)||(end==RDCut::AudioBoth)) {
      setStartPoint(0);
    }
    if((end==RDCut::AudioTail)||(end==RDCut::AudioBoth)) {
      setEndPoint(wave->getExtTimeLength());
    }
    setLength(endPoint()-startPoint());
    delete wave;
    return;
  }
  if((end==RDCut::AudioHead)||(end==RDCut::AudioBoth)) {
    if((point=wave->startTrim(REFERENCE_LEVEL-level))>-1) {
      start_point=(int)(1000.0*(double)point/(double)wave->getSamplesPerSec());
    }
  }
  if((end==RDCut::AudioTail)||(end==RDCut::AudioBoth)) {
    if((point=wave->endTrim(+REFERENCE_LEVEL-level))>-1) {
      end_point=(int)(1000.0*(double)point/(double)wave->getSamplesPerSec());
    }
    else {
      end_point=wave->getExtTimeLength();
    }
  }
  else {
    end_point=wave->getExtTimeLength();
  }
  setStartPoint(start_point);
  setEndPoint(end_point);
  if(segueEndPoint()>end_point) {
    setSegueEndPoint(end_point);
  }
  if(segueStartPoint()>end_point) {
    setSegueStartPoint(-1);
    setSegueEndPoint(-1);
  }
  setLength(end_point-start_point);
  delete wave;
#endif  // WIN32
}


void RDCut::autoSegue(int level,int length)
{
#ifndef WIN32
  int point;
  int start_point;

  if(!exists()) {
    return;
  }
  QString wavename=RDCut::pathName(cut_name); 
  RDWaveFile *wave=new RDWaveFile(wavename);
  if(!wave->openWave()) {
    delete wave;
    return;
  }
  if(level<0) {
    if((point=wave->endTrim(+REFERENCE_LEVEL-level))>-1) {
      start_point=(int)(1000.0*(double)point/(double)wave->getSamplesPerSec());
      setSegueStartPoint(start_point);
      if(length>0 && (start_point+length)<endPoint()){
        setSegueEndPoint(start_point+length);
        }
      else {
        setSegueEndPoint(endPoint());
        }
      }
    }
  else {
    if(length>0) {
       if((endPoint()-length)>startPoint()){
          setSegueStartPoint(endPoint()-length);
          setSegueEndPoint(endPoint());
          }
       else {
          setSegueStartPoint(startPoint());
          setSegueEndPoint(endPoint());
          }
       }
    }
  delete wave;
#endif  // WIN32
}


void RDCut::reset() const
{
#ifndef WIN32
  QString sql;
  RDSqlQuery *q;
  int format;

  if(!exists()) {
    return;
  }
  RDWaveFile *wave=new RDWaveFile(RDCut::pathName(cut_name)); 
  if(wave->openWave()) {
    switch(wave->getFormatTag()) {
	case WAVE_FORMAT_MPEG:
	  format=wave->getHeadLayer()-1;
	  break;
	default:
	  format=0;
	  break;
    }
    sql=QString().sprintf("update CUTS set LENGTH=%u,\
                           ORIGIN_DATETIME=NOW(),\
                           ORIGIN_NAME=\"\",\
                           LAST_PLAY_DATETIME=NULL,PLAY_COUNTER=0,\
                           CODING_FORMAT=%d,SAMPLE_RATE=%u,BIT_RATE=%u,\
                           CHANNELS=%u,PLAY_GAIN=0,\
                           START_POINT=0,END_POINT=%u,FADEUP_POINT=-1,\
                           FADEDOWN_POINT=-1,\
                           SEGUE_START_POINT=-1,SEGUE_END_POINT=-1,\
		           SEGUE_GAIN=%d,\
                           HOOK_START_POINT=-1,HOOK_END_POINT=-1,\
                           TALK_START_POINT=-1,TALK_END_POINT=-1 \
                           where CUT_NAME=\"%s\"",
			  wave->getExtTimeLength(),
			  format,
			  wave->getSamplesPerSec(),
			  wave->getHeadBitRate(),
			  wave->getChannels(),
			  wave->getExtTimeLength(),
			  RD_FADE_DEPTH,
			  (const char *)cut_name);
  }
  else {
    sql=QString().sprintf("update CUTS set LENGTH=0,\
                           ORIGIN_DATETIME=NULL,\
                           ORIGIN_NAME=\"\",\
                           LAST_PLAY_DATETIME=NULL,PLAY_COUNTER=0,\
                           CODING_FORMAT=0,SAMPLE_RATE=0,BIT_RATE=0,\
                           CHANNELS=0,PLAY_GAIN=0,\
                           START_POINT=-1,END_POINT=-1,FADEUP_POINT=-1,\
                           FADEDOWN_POINT=-1,\
                           SEGUE_START_POINT=-1,SEGUE_END_POINT=-1,\
		           SEGUE_GAIN= %d,\
                           HOOK_START_POINT=-1,HOOK_END_POINT=-1,\
                           TALK_START_POINT=-1,TALK_END_POINT=-1 \
                           where CUT_NAME=\"%s\"",
			   RD_FADE_DEPTH,
			  (const char *)cut_name);
  }
  q=new RDSqlQuery(sql,cut_db);
  delete q;
  wave->closeWave();
  delete wave;
#endif  // WIN32
}


void RDCut::connect(QObject *receiver,const char *member) const
{
  cut_signal->connect(receiver,member);
}


void RDCut::disconnect(QObject *receiver,const char *member) const
{
  cut_signal->disconnect(receiver,member);
}


bool RDCut::exists(const QString &cutname)
{
  RDSqlQuery *q=new RDSqlQuery(QString().sprintf("select CUT_NAME from CUTS\
                                                where CUT_NAME=\"%s\"",
					       (const char *)cutname));
  bool ret=q->first();
  delete q;
  return ret;
}


QString RDCut::pathName(const QString &cutname)
{
  return RDConfiguration()->audioFileName(cutname); 
}


bool RDCut::FileCopy(const QString &srcfile,const QString &destfile) const
{
#ifndef WIN32
  int src_fd;
  int dest_fd;
  struct stat src_stat;
  struct stat dest_stat;
  char *buf=NULL;
  int n;
  unsigned bytes=0;
  int previous_step=0;
  int step=0;

  if((src_fd=open((const char *)srcfile.utf8(),O_RDONLY))<0) {
    return false;
  }
  if(fstat(src_fd,&src_stat)<0) {
    close(src_fd);
    return false;
  }
  if((dest_fd=open((const char *)destfile.utf8(),O_RDWR|O_CREAT,src_stat.st_mode))
     <0) {
    close(src_fd);
    return false;
  }
  if(fstat(dest_fd,&dest_stat)<0) {
    close(src_fd);
    close(dest_fd);
    return false;
  }
  buf=(char *)malloc(dest_stat.st_blksize);
  while((n=read(src_fd,buf,dest_stat.st_blksize))==dest_stat.st_blksize) {
    write(dest_fd,buf,dest_stat.st_blksize);
    bytes+=dest_stat.st_blksize;
    if((step=10*bytes/src_stat.st_size)!=previous_step) {
      cut_signal->setValue(step);
      cut_signal->activate();
      previous_step=step;
    }
  }
  write(dest_fd,buf,n);
  cut_signal->setValue(10);
  cut_signal->activate();
  free(buf);
  close(src_fd);
  close(dest_fd);
#endif
  return true;
}


void RDCut::SetRow(const QString &param,const QString &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CUTS SET %s=\"%s\" WHERE CUT_NAME=\"%s\"",
			(const char *)param,
			(const char *)RDEscapeString(value.utf8()),
			(const char *)cut_name);
  q=new RDSqlQuery(sql,cut_db);
  delete q;
}


void RDCut::SetRow(const QString &param,unsigned value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CUTS SET %s=%u WHERE CUT_NAME=\"%s\"",
			(const char *)param,
			value,
			(const char *)cut_name);
  q=new RDSqlQuery(sql,cut_db);
  delete q;
}


void RDCut::SetRow(const QString &param,int value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CUTS SET %s=%d WHERE CUT_NAME=\"%s\"",
			(const char *)param,
			value,
			(const char *)cut_name);
  q=new RDSqlQuery(sql,cut_db);
  delete q;
}


void RDCut::SetRow(const QString &param,const QDateTime &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CUTS SET %s=\"%s\" WHERE CUT_NAME=\"%s\"",
			(const char *)param,
			(const char *)value.toString("yyyy-MM-dd hh:mm:ss"),
			(const char *)cut_name);
  q=new RDSqlQuery(sql,cut_db);
  delete q;
}


void RDCut::SetRow(const QString &param,const QDate &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CUTS SET %s=\"%s\" WHERE CUT_NAME=\"%s\"",
			(const char *)param,
			(const char *)value.toString("yyyy-MM-dd"),
			(const char *)cut_name);
  q=new RDSqlQuery(sql,cut_db);
  delete q;
}


void RDCut::SetRow(const QString &param,const QTime &value) const
{
  RDSqlQuery *q;
  QString sql;
  sql=QString().sprintf("UPDATE CUTS SET %s=\"%s\" WHERE CUT_NAME=\"%s\"",
			(const char *)param,
			(const char *)value.toString("hh:mm:ss"),
			(const char *)cut_name);
  q=new RDSqlQuery(sql,cut_db);
  delete q;
}


void RDCut::SetRow(const QString &param) const
{
  RDSqlQuery *q;
  QString sql;
  sql=QString().sprintf("UPDATE CUTS SET %s=NULL WHERE CUT_NAME=\"%s\"",
			(const char *)param,
			(const char *)cut_name);
  q=new RDSqlQuery(sql,cut_db);
  delete q;
}
