// rdcart.cpp
//
// Abstract a Rivendell Cart.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcart.cpp,v 1.61.2.5 2009/11/19 17:56:44 cvs Exp $
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

#include <vector>

#include <rd.h>
#include <rdconf.h>
#include <rdcart.h>
#include <rdcut.h>
#include <rdtextvalidator.h>
#include <rdescape_string.h>
#include <rdsystem.h>

RDCart::RDCart(unsigned number)
{
  cart_number=number;
}


bool RDCart::exists() const
{
  return RDDoesRowExist("CART","NUMBER",cart_number);
}


bool RDCart::selectCut(QString *cut) const
{
  return selectCut(cut,QTime::currentTime());
}


bool RDCart::selectCut(QString *cut,const QTime &time) const
{
  bool ret;

  if(!exists()) {
    ret=(*cut=="");
    *cut="";
#ifndef WIN32
    syslog(LOG_USER|LOG_WARNING,
	   "RDCart::selectCut(): cart doesn't exist, CUT=%s",
	   (const char *)cut);
#endif  // WIN32
    return ret;
  }

  if(!cut->isEmpty()) {
    RDCut *rdcut=new RDCut(*cut);
    delete rdcut;
  }

  QString sql;
  RDSqlQuery *q;
  QString cutname;
  QDate current_date=QDate::currentDate();
  QString datetime_str=QDateTime(current_date,time).
    toString("yyyy-MM-dd hh:mm:ss");

  if(type()==RDCart::Audio) {
    sql=QString().sprintf("select CUT_NAME,WEIGHT,LOCAL_COUNTER\
                           from CUTS  where (((START_DATETIME<=\"%s\")&&\
                           (END_DATETIME>=\"%s\"))||\
                           (START_DATETIME is null))&&\
                           (((START_DAYPART<=\"%s\")&&(END_DAYPART>=\"%s\")||\
                           START_DAYPART is null))&&\
                           (%s=\"Y\")&&(CART_NUMBER=%u)&&(EVERGREEN=\"N\")&&\
                           (LENGTH>0) order by LOCAL_COUNTER",
			  (const char *)datetime_str,
			  (const char *)datetime_str,
			  (const char *)datetime_str,
			  (const char *)datetime_str,
	(const char *)RDGetShortDayNameEN(current_date.dayOfWeek()).upper(),
			  cart_number);
    q=new RDSqlQuery(sql);
    cutname=GetNextCut(q);
    delete q;
  }
  if(cutname.isEmpty()) {   // No valid cuts, try the evergreen
#ifndef WIN32
    // syslog(LOG_USER|LOG_WARNING,"RDCart::selectCut(): no valid cuts, trying evergreen, SQL=%s",(const char *)sql);
#endif  // WIN32
    sql=QString().sprintf("select CUT_NAME,WEIGHT,LOCAL_COUNTER\
                           from CUTS where (CART_NUMBER=%u)&&\
                           (EVERGREEN=\"Y\")&&(LENGTH>0) \
                           order by LOCAL_COUNTER",
			  cart_number);
    q=new RDSqlQuery(sql);
    cutname=GetNextCut(q);
    delete q;
  }
  if(cutname.isEmpty()) {
#ifndef WIN32
    // syslog(LOG_USER|LOG_WARNING,"RDCart::selectCut(): no valid evergreen cuts, SQL=%s",(const char *)sql);
#endif  // WIN32
  }
  *cut=cutname;
  return true;
}


unsigned RDCart::number() const
{
  return cart_number;
}


QString RDCart::groupName() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"GROUP_NAME").
    toString();
}


void RDCart::setGroupName(const QString &name) const
{
  SetRow("GROUP_NAME",name);
}


RDCart::Type RDCart::type() const
{
  return (RDCart::Type)RDGetSqlValue("CART","NUMBER",cart_number,
				    "TYPE").toUInt();
}


void RDCart::setType(RDCart::Type type) const
{
  SetRow("TYPE",(unsigned)type);
}


QString RDCart::title() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"TITLE").toString();
}


void RDCart::setTitle(const QString &title) const
{
  SetRow("TITLE",VerifyTitle(title));
}


QString RDCart::artist() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"ARTIST").toString();
}


void RDCart::setArtist(const QString &artist) const
{
  SetRow("ARTIST",artist);
}


QString RDCart::album() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"ALBUM").toString();
}


void RDCart::setAlbum(const QString &album) const
{
  SetRow("ALBUM",album);
}


QDate RDCart::year() const
{
  QDate value;
  value=RDGetSqlValue("CART","NUMBER",cart_number,"YEAR").toDate();
  if(value.isValid()) {
    return value;
  }
  return QDate();
}


void RDCart::setYear(const QDate &date) const
{
  SetRow("YEAR",date);
}


QString RDCart::schedCodes() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"SCHED_CODES").toString();
}


void RDCart::setSchedCodes(const QString &sched_codes) const
{
  SetRow("SCHED_CODES",sched_codes);
}


void RDCart::updateSchedCodes(const QString &add_codes,const QString &remove_codes) const
{
  QString sched_codes;
  QString save_codes="";
  QString sql;
  RDSqlQuery *q;
  QString str;

  sched_codes=schedCodes();

  sql=QString().sprintf("select CODE from SCHED_CODES");
  q=new RDSqlQuery(sql);
  while(q->next()) {
  	QString wstr=q->value(0).toString();
  	wstr+="          ";
    wstr=wstr.left(11);
  	if((sched_codes.contains(wstr)>0||add_codes.contains(wstr)>0)&&remove_codes.contains(wstr)==0) {
  	  save_codes+=wstr;
  	}
  }
  delete q;

  save_codes+=".";
  SetRow("SCHED_CODES",save_codes);
}	

void RDCart::setYear() const
{
  SetRow("YEAR");
}


QString RDCart::label() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"LABEL").toString();
}


void RDCart::setLabel(const QString &label) const
{
  SetRow("LABEL",label);
}


QString RDCart::client() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"CLIENT").toString();
}


void RDCart::setClient(const QString &client) const
{
  SetRow("CLIENT",client);
}


QString RDCart::agency() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"AGENCY").toString();
}


void RDCart::setAgency(const QString &agency) const
{
  SetRow("AGENCY",agency);
}


QString RDCart::publisher() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "PUBLISHER").toString();
}


void RDCart::setPublisher(const QString &publisher) const
{
  SetRow("PUBLISHER",publisher);
}


QString RDCart::composer() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "COMPOSER").toString();
}


void RDCart::setComposer(const QString &composer) const
{
  SetRow("COMPOSER",composer);
}


QString RDCart::userDefined() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "USER_DEFINED").toString();
}


void RDCart::setUserDefined(const QString &string) const
{
  SetRow("USER_DEFINED",string);
}


RDCart::UsageCode RDCart::usageCode() const
{
  return (RDCart::UsageCode) RDGetSqlValue("CART","NUMBER",cart_number,
		      "USAGE_CODE").toInt();
}


void RDCart::setUsageCode(RDCart::UsageCode code)
{
  SetRow("USAGE_CODE",(int)code);
}


QString RDCart::notes() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"NOTES").toString();
}


void RDCart::setNotes(const QString &notes) const
{
  SetRow("NOTES",notes);
}


unsigned RDCart::forcedLength() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "FORCED_LENGTH").toUInt();
}


void RDCart::setForcedLength(unsigned length) const
{
  SetRow("FORCED_LENGTH",length);
}


unsigned RDCart::lengthDeviation() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "LENGTH_DEVIATION").toUInt();
}


void RDCart::setLengthDeviation(unsigned length) const
{
  SetRow("LENGTH_DEVIATION",length);
}


unsigned RDCart::calculateAverageLength(unsigned *max_dev) const
{
  unsigned total=0;
  unsigned count=0;
  unsigned high=0;
  unsigned low=0xFFFFFFFF;
  unsigned avg;
  unsigned weight;
  QDateTime end_datetime;

  QString sql=QString().sprintf("select LENGTH, WEIGHT,END_DATETIME from CUTS\
                                 where (CART_NUMBER=%u)&&(LENGTH>0)",
				cart_number);
  RDSqlQuery *q=new RDSqlQuery(sql);
  while(q->next()) {
    weight = q->value(1).toUInt();
    end_datetime = q->value(2).toDateTime();
    if (end_datetime.isValid() && (end_datetime <QDateTime::currentDateTime ())){
      // This cut has expired, it is no more, set its weight to zero.
      weight = 0;
    }
    total+=(q->value(0).toUInt() * weight);
    if((weight) && (q->value(0).toUInt()>high)) {
      high=q->value(0).toUInt();
    }
    if((weight) && (q->value(0).toUInt()<low)) {
      low=q->value(0).toUInt();
    }
    count += weight;    
  }
  delete q;
  if(count==0) {
    avg=0;
    low=0;
    high=0;
  }
  else {
    avg=total/count;
  }
  if(max_dev!=NULL) {
    if((high-avg)>(avg-low)) {
      *max_dev=high-avg;
    }
    else {
      *max_dev=avg-low;
    }
  }
  return avg;
}


unsigned RDCart::averageLength() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "AVERAGE_LENGTH").toUInt();
}


void RDCart::setAverageLength(unsigned length) const
{
  SetRow("AVERAGE_LENGTH",length);
}


unsigned RDCart::averageSegueLength() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "AVERAGE_SEGUE_LENGTH").toUInt();
}


void RDCart::setAverageSegueLength(unsigned length) const
{
  SetRow("AVERAGE_SEGUE_LENGTH",length);
}


unsigned RDCart::averageHookLength() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "AVERAGE_HOOK_LENGTH").toUInt();
}


void RDCart::setAverageHookLength(unsigned length) const
{
  SetRow("AVERAGE_HOOK_LENGTH",length);
}


unsigned RDCart::cutQuantity() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "CUT_QUANTITY").toUInt();
}


void RDCart::setCutQuantity(unsigned quan) const
{
  SetRow("CUT_QUANTITY",quan);
}


unsigned RDCart::lastCutPlayed() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,
		      "LAST_CUT_PLAYED").toUInt();
}


void RDCart::setLastCutPlayed(unsigned cut) const
{
  SetRow("LAST_CUT_PLAYED",cut);
}


RDCart::PlayOrder RDCart::playOrder() const
{
  return (RDCart::PlayOrder)RDGetSqlValue("CART","NUMBER",cart_number,
					 "PLAY_ORDER").toUInt();
}


void RDCart::setPlayOrder(RDCart::PlayOrder order) const
{
  SetRow("PLAY_ORDER",(unsigned)order);
}


RDCart::Validity RDCart::validity() const
{
  return (RDCart::Validity)RDGetSqlValue("CART","NUMBER",cart_number,
					 "VALIDITY").toUInt();
}


void RDCart::setValidity(RDCart::Validity state)
{
  SetRow("VALIDITY",(unsigned)state);
}


QDateTime RDCart::startDateTime() const
{
  QDateTime value;
  value=RDGetSqlValue("CART","NUMBER",cart_number,
		     "START_DATETIME").toDateTime();
  if(value.isValid()) {
    return value;
  }
  return QDateTime(QDate(),QTime());
}


void RDCart::setStartDateTime(const QDateTime &time) const
{
  SetRow("START_DATETIME",time);
}


void RDCart::setStartDateTime() const
{
  SetRow("START_DATETIME");
}


QDateTime RDCart::endDateTime() const
{
  QDateTime value;
  value=RDGetSqlValue("CART","NUMBER",cart_number,
		     "END_DATETIME").toDateTime();
  if(value.isValid()) {
    return value;
  }
  return QDateTime(QDate(),QTime());
}


void RDCart::setEndDateTime(const QDateTime &time) const
{
  SetRow("END_DATETIME",time);
}


void RDCart::setEndDateTime() const
{
  SetRow("END_DATETIME");
}


bool RDCart::enforceLength() const
{
  return RDBool(RDGetSqlValue("CART","NUMBER",cart_number,
			    "ENFORCE_LENGTH").toString());
}


void RDCart::setEnforceLength(bool state) const
{
  SetRow("ENFORCE_LENGTH",RDYesNo(state));
}


bool RDCart::preservePitch() const
{
  return RDBool(RDGetSqlValue("CART","NUMBER",cart_number,
			    "PRESERVE_PITCH").toString());
}


void RDCart::setPreservePitch(bool state) const
{
  SetRow("PRESERVE_PITCH",RDYesNo(state));
}


bool RDCart::asyncronous() const
{
  return RDBool(RDGetSqlValue("CART","NUMBER",cart_number,
			    "ASYNCRONOUS").toString());
}


void RDCart::setAsyncronous(bool state) const
{
  SetRow("ASYNCRONOUS",RDYesNo(state));
}


QString RDCart::owner() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"OWNER").toString();
}


void RDCart::setOwner(const QString &owner) const
{
  SetRow("OWNER",owner);
}


QString RDCart::macros() const
{
  return RDGetSqlValue("CART","NUMBER",cart_number,"MACROS").toString();
}


void RDCart::setMacros(const QString &cmds) const
{
  SetRow("MACROS",cmds);
}


void RDCart::getMetadata(RDWaveData *data)
{
  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("select TITLE,ARTIST,ALBUM,YEAR,LABEL,CLIENT,\
                         AGENCY,PUBLISHER,COMPOSER,USER_DEFINED\
                         from CART where NUMBER=%u",cart_number);
  q=new RDSqlQuery(sql);
  if(q->first()) {
    data->setTitle(q->value(0).toString());
    data->setArtist(q->value(1).toString());
    data->setAlbum(q->value(2).toString());
    data->setReleaseYear(q->value(3).toInt());
    data->setLabel(q->value(4).toString());
    data->setClient(q->value(5).toString());
    data->setAgency(q->value(6).toString());
    data->setPublisher(q->value(7).toString());
    data->setComposer(q->value(8).toString());
    data->setUserDefined(q->value(9).toString());
    data->setMetadataFound(true);
  }
  delete q;
}


void RDCart::setMetadata(RDWaveData *data)
{
  QString sql="update CART set ";
  if(!data->title().isEmpty()) {
    sql+=QString().sprintf("TITLE=\"%s\",",(const char *)
			   RDEscapeString(VerifyTitle(data->title())).utf8());
  }
  if(!data->artist().isEmpty()) {
    sql+=QString().sprintf("ARTIST=\"%s\",",(const char *)
			   RDEscapeString(data->artist()).utf8());
  }
  if(!data->album().isEmpty()) {
    sql+=QString().sprintf("ALBUM=\"%s\",",(const char *)
			   RDEscapeString(data->album()).utf8());
  }
  if(data->releaseYear()>0) {
    sql+=QString().sprintf("YEAR=\"%04d-01-01\",",data->releaseYear());
  }
  if(!data->label().isEmpty()) {
    sql+=QString().sprintf("LABEL=\"%s\",",(const char *)
			   RDEscapeString(data->label()).utf8());
  }
  if(!data->client().isEmpty()) {
    sql+=QString().sprintf("CLIENT=\"%s\",",(const char *)
			   RDEscapeString(data->client()).utf8());
  }
  if(!data->agency().isEmpty()) {
    sql+=QString().sprintf("AGENCY=\"%s\",",(const char *)
			   RDEscapeString(data->agency()).utf8());
  }
  if(!data->publisher().isEmpty()) {
    sql+=QString().sprintf("PUBLISHER=\"%s\",",(const char *)
			   RDEscapeString(data->publisher()).utf8());
  }
  if(!data->composer().isEmpty()) {
    sql+=QString().sprintf("COMPOSER=\"%s\",",(const char *)
			   RDEscapeString(data->composer()).utf8());
  }
  if(!data->userDefined().isEmpty()) {
    sql+=QString().sprintf("USER_DEFINED=\"%s\",",(const char *)
			   RDEscapeString(data->userDefined()).utf8());
  }
  if(sql.right(1)==",") {
    sql=sql.left(sql.length()-1);
    sql+=QString().sprintf(" where NUMBER=%u",cart_number);
    RDSqlQuery *q=new RDSqlQuery(sql);
    delete q;
  }
}


void RDCart::updateLength() const
{
  updateLength(enforceLength(),forcedLength());
}


void RDCart::updateLength(bool enforce_length,unsigned length) const
{
  //
  // Update Length
  //
  long long total=0;
  long long segue_total=0;
  long long hook_total=0;
  unsigned weight_total=0;
  unsigned weight = 0;
  QDateTime end_date;

  bool dow_active[7]={false,false,false,false,false,false,false};
  bool time_ok=true;
  QString sql=QString().
    sprintf("select LENGTH,SEGUE_START_POINT,SEGUE_END_POINT,START_POINT,\
             SUN,MON,TUE,WED,THU,FRI,SAT,START_DAYPART,END_DAYPART,\
             HOOK_START_POINT,HOOK_END_POINT,WEIGHT,END_DATETIME \
             from CUTS where (CUT_NAME like \"%06d%%\")&&(LENGTH>0)",
	    cart_number);
  RDSqlQuery *q=new RDSqlQuery(sql);
  while(q->next()) {
    for(unsigned i=0;i<7;i++) {
      dow_active[i]|=RDBool(q->value(4+i).toString());
    }
    weight = q->value(15).toUInt();
    end_date = q->value(16).toDateTime();
    if (end_date.isValid() && (end_date <QDateTime::currentDateTime ())){
      // This cut has expired, it is no more, set its weight to zero.
      weight = 0;
    }
    total+=q->value(0).toUInt() * weight;
    if((q->value(1).toInt()<0)||(q->value(2).toInt()<0)) {
      segue_total+=q->value(0).toUInt() * weight;
    }
    else {
      segue_total+=(q->value(1).toInt()-q->value(3).toInt()) * weight;
    }
    hook_total+=(q->value(14).toUInt()-q->value(13).toUInt()) * weight;
    weight_total += weight;
  }
  if(weight_total>0) {
    setAverageLength(total/weight_total);
    setAverageSegueLength(segue_total/weight_total);
    setAverageHookLength(hook_total/weight_total);
    if(!enforce_length) {
      setForcedLength(total/weight_total);
    }
  }
  else {
    setAverageLength(0);
    setAverageSegueLength(0);
    setAverageHookLength(0);
    if(!enforce_length) {
      setForcedLength(0);
    }
  }
  setCutQuantity(q->size());
  delete q;

  //
  // Update Validity
  //
  RDCut::Validity cut_validity=RDCut::NeverValid;
  RDCart::Validity cart_validity=RDCart::NeverValid;
  bool evergreen=true;
  QDateTime start_datetime;
  QDateTime end_datetime;
  RDSqlQuery *q1;
  QDateTime valid_until;
  bool dates_valid=true;

  sql=QString().sprintf("select CUT_NAME,START_DAYPART,END_DAYPART,LENGTH,\
                         SUN,MON,TUE,WED,THU,FRI,SAT,EVERGREEN,\
                         START_DATETIME,END_DATETIME from CUTS\
                         where CART_NUMBER=%u",
			cart_number);
  q=new RDSqlQuery(sql);
  while(q->next()) {
    cut_validity=ValidateCut(q,enforce_length,length,&time_ok);
    sql=QString().sprintf("update CUTS set VALIDITY=%u where CUT_NAME=\"%s\"",
			  cut_validity,(const char *)q->value(0).toString());
    q1=new RDSqlQuery(sql);
    delete q1;
    evergreen&=RDBool(q->value(11).toString());
    if((int)cut_validity>(int)cart_validity) {
      cart_validity=(RDCart::Validity)cut_validity;
    }
    if((cut_validity!=RDCut::NeverValid)&&(q->value(13).isNull())) {
      dates_valid=false;
    }
    if(!q->value(12).isNull()) {
      if((start_datetime>q->value(12).toDateTime())||
	 start_datetime.isNull()) {
	start_datetime=q->value(12).toDateTime();
      }
    }
    if(!q->value(13).isNull()) {
      if((end_datetime<q->value(13).toDateTime())||
	 (end_datetime.isNull())) {
	end_datetime=q->value(13).toDateTime();
      }
    }
  }
  delete q;
  if(cart_validity==RDCart::ConditionallyValid) {  // Promote to Always?
    bool all_dow=true;
    for(unsigned i=0;i<7;i++) {
      all_dow&=dow_active[i];
    }
    if(all_dow&&time_ok) {
      cart_validity=RDCart::AlwaysValid;
    }
  }
  if(evergreen) {  // Promote to Evergreen?
    cart_validity=RDCart::EvergreenValid;
  }

  //
  // Set start/end datetimes
  //
  sql="update CART set ";
  if(start_datetime.isNull()||(!dates_valid)) {
    sql+="START_DATETIME=NULL,";
  }
  else {
    sql+=QString().sprintf("START_DATETIME=\"%s\",",
		(const char *)start_datetime.toString("yyyy-MM-dd hh:mm:ss"));
  }
  if(end_datetime.isNull()||(!dates_valid)) {
    sql+="END_DATETIME=NULL,";
  }
  else {
    sql+=QString().sprintf("END_DATETIME=\"%s\",",
		(const char *)end_datetime.toString("yyyy-MM-dd hh:mm:ss"));
  }
  sql+=QString().sprintf("VALIDITY=%u where NUMBER=%u",
			 cart_validity,cart_number);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDCart::resetRotation() const
{
  QString sql=
    QString().sprintf("update CUTS set LOCAL_COUNTER=0 where CART_NUMBER=%d",
		      cart_number);
  RDSqlQuery *q=new RDSqlQuery(sql);
  delete q;
}


int RDCart::addCut(unsigned format,unsigned samprate,
		   unsigned bitrate,unsigned chans) const
{
  RDSqlQuery *q;
  QString sql;

  int next=GetNextFreeCut();
  QString next_name=QString().sprintf("%06d_%03d",cart_number,next);
  sql=QString().sprintf("insert into CUTS set CUT_NAME=\"%s\",\
                         CART_NUMBER=%d,DESCRIPTION=\"Cut %03d\",LENGTH=0,\
                         CODING_FORMAT=%d,SAMPLE_RATE=%d,BIT_RATE=%d,\
                         CHANNELS=%d",
			(const char *)next_name,
			cart_number,
			next,
			format,
			samprate,
			bitrate,
			chans);
  q=new RDSqlQuery(sql);
  delete q;

  setCutQuantity(cutQuantity()+1);
  updateLength();
  resetRotation();
  return next;
}


void RDCart::removeAllCuts()
{
  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("select CUT_NAME from CUTS where CART_NUMBER=%u",
			cart_number);
  q=new RDSqlQuery(sql);
  while(q->next()) {
    removeCut(q->value(0).toString());
  }
  delete q;
}


void RDCart::removeCut(const QString &cutname)
{
  if(!exists()) {
    return;
  }

  QString sql;
  RDSqlQuery *q;
  QString filename;

  filename = RDCut::pathName(cutname); 
  unlink(filename);
  unlink(filename+".energy");
  sql=QString().sprintf("delete from CUTS where CUT_NAME=\"%s\"",
			(const char *)cutname);
  q=new RDSqlQuery(sql);
  delete q;
  setCutQuantity(cutQuantity()-1);
}


bool RDCart::create(const QString &groupname,RDCart::Type type) const
{
  QString sql=QString().sprintf("insert into CART set NUMBER=%d,TYPE=%d,\
                                 GROUP_NAME=\"%s\"",cart_number,type,
				(const char *)groupname);
  RDSqlQuery *q=new RDSqlQuery(sql);
  bool ret=q->isActive();
  delete q;

  return ret;
}


void RDCart::remove() const
{
  QString sql;
  RDSqlQuery *q;

  if(type()==RDCart::Audio) {
    sql=QString().sprintf("select CUT_NAME from CUTS where CART_NUMBER=%u",
			  cart_number);
    q=new RDSqlQuery(sql);
    while(q->next()) {
      unlink(RDCut::pathName(QString(q->value(0).toString())).ascii()); 
      unlink((RDCut::pathName(QString(q->value(0).toString()))+".energy").ascii()); 
    }
    delete q;
    sql=QString().sprintf("delete from CUTS where CART_NUMBER=%u",cart_number);
    q=new RDSqlQuery(sql);
    delete q;
  }
  sql=QString().sprintf("delete from CART where NUMBER=%u",cart_number);
  q=new RDSqlQuery(sql);
  delete q;
}


bool RDCart::exists(unsigned cartnum)
{
  RDSqlQuery *q=new RDSqlQuery(QString().sprintf("select NUMBER from CART\
                                                where NUMBER=%u",cartnum));
  bool ret=q->first();
  delete q;
  return ret;
}


QString RDCart::playOrderText(RDCart::PlayOrder order)
{
  switch(order) {
      case RDCart::Sequence:
	return QT_TR_NOOP("Sequentially");

      case RDCart::Random:
	return QT_TR_NOOP("Randomly");
  }
  return QString QT_TR_NOOP("Unknown");
}


QString RDCart::usageText(RDCart::UsageCode usage)
{
  switch(usage) {
      case RDCart::UsageFeature:
	return QT_TR_NOOP("Feature");

      case RDCart::UsageOpen:
	return QT_TR_NOOP("Theme Open");

      case RDCart::UsageClose:
	return QT_TR_NOOP("Theme Close");

      case RDCart::UsageTheme:
	return QT_TR_NOOP("Theme Open/Close");

      case RDCart::UsageBackground:
	return QT_TR_NOOP("Background");

      case RDCart::UsagePromo:
	return QT_TR_NOOP("Commercial/Jingle/Promo");

      case RDCart::UsageLast:
	return QT_TR_NOOP("Unknown");  
	break;
  }
  return QT_TR_NOOP("Unknown");  
}


QString RDCart::GetNextCut(RDSqlQuery *q) const
{
  QString cutname;
  double ratio;
  double play_ratio=100000000.0;
  std::vector<int> eligibles;


  while(q->next()) {
    if((ratio=q->value(2).toDouble()/q->value(1).toDouble())<play_ratio) {
      play_ratio=ratio;
      cutname=q->value(0).toString();
    }
  }
  return cutname;
}


int RDCart::GetNextFreeCut() const
{
  RDSqlQuery *q;
  QString sql;
  unsigned num=1;

  sql=QString().sprintf("select CUT_NAME from CUTS where CART_NUMBER=%d\
                         order by CUT_NAME",
			cart_number);
  q=new RDSqlQuery(sql);
  if(q->last()) {
    sscanf(((const char *)q->value(0).toString())+7,"%d",&num);
    num++;
  }
  delete q;
  return num;
}


RDCut::Validity RDCart::ValidateCut(RDSqlQuery *q,bool enforce_length,
				    unsigned length,bool *time_ok) const
{
  RDCut::Validity ret=RDCut::AlwaysValid;
  QDateTime current_datetime=
    QDateTime(QDate::currentDate(),QTime::currentTime());

  if(q->value(3).toUInt()==0) {
    return RDCut::NeverValid;
  }
  if(q->value(11).toString()=="N") {  // No Evergreen Cuts!
    //
    // Dayparts
    //
    if((!q->value(1).isNull())||(!q->value(2).isNull())) { 
      *time_ok=false;
      ret=RDCut::ConditionallyValid;
    }
    
    //
    // Days of the Week
    //
    bool dow_found=false;
    bool all_dow_found=true;
    for(int i=4;i<11;i++) {
      if(q->value(i).toString()=="Y") {
	dow_found=true;
      }
      else {
	all_dow_found=false;
      }
    }
    if(!dow_found) {
      return RDCut::NeverValid;
    }
    if(!all_dow_found) {
      ret=RDCut::ConditionallyValid;
    }

    //
    // Start/End DayTimes
    //
    if(!q->value(13).isNull()) {
      *time_ok=false;
      if(q->value(13).toDateTime()<current_datetime) {
	return RDCut::NeverValid;
      }
      ret=RDCut::ConditionallyValid;
    }
  }

  //
  // Timescaling
  //
  if(enforce_length) {
    double len=(double)length;
    if(((q->value(3).toDouble()*RD_TIMESCALE_MAX)<len)||
       ((q->value(3).toDouble()*RD_TIMESCALE_MIN)>len)) {
      *time_ok=false;
      return RDCut::NeverValid;
    }
  }

  return ret;
}


QString RDCart::VerifyTitle(const QString &title) const
{
  QString ret=title;
  QString sql;
  RDSqlQuery *q;
  RDSystem *system=new RDSystem();

  if(!system->allowDuplicateCartTitles()) {
    int n=1;
    while(1==1) {
      sql=QString().sprintf("select NUMBER from CART \
                             where (TITLE=\"%s\")&&(NUMBER!=%u)",
			    (const char *)RDEscapeString(ret),cart_number);
      q=new RDSqlQuery(sql);
      if(!q->first()) {
	delete q;
	return ret;
      }
      delete q;
      ret=title+QString().sprintf(" [%d]",n++);
    }
  }
  delete system;
  return ret;
}


void RDCart::SetRow(const QString &param,const QString &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CART SET %s=\"%s\" WHERE NUMBER=%u",
			(const char *)param,
			(const char *)RDEscapeString(value.utf8()),
			cart_number);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDCart::SetRow(const QString &param,unsigned value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CART SET %s=%d WHERE NUMBER=%u",
			(const char *)param,
			value,
			cart_number);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDCart::SetRow(const QString &param,const QDateTime &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CART SET %s=\"%s\" WHERE NUMBER=%u",
			(const char *)param,
			(const char *)value.toString("yyyy-MM-dd hh:mm:ss"),
			cart_number);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDCart::SetRow(const QString &param,const QDate &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CART SET %s=\"%s\" WHERE NUMBER=%u",
			(const char *)param,
			(const char *)value.toString("yyyy-MM-dd"),
			cart_number);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDCart::SetRow(const QString &param) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE CART SET %s=NULL WHERE NUMBER=%u",
			(const char *)param,
			cart_number);
  q=new RDSqlQuery(sql);
  delete q;
}
