// rdfeed.cpp
//
// Abstract a Rivendell RSS Feed
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdfeed.cpp,v 1.7.2.6 2009/09/04 01:29:49 cvs Exp $
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

#include <qfile.h>

#include <rddb.h>
#include <rdfeed.h>
#include <rdconf.h>
#include <rdlibrary_conf.h>
#include <rdescape_string.h>
#include <rdwavefile.h>
#include <rdpodcast.h>
#include <rdcut.h>
#include <qurl.h>

RDFeed::RDFeed(const QString &keyname,QObject *parent,const char *name)
  : QObject(parent,name)
{
  RDSqlQuery *q;
  QString sql;

  feed_keyname=keyname;

  sql=QString().sprintf("select ID from FEEDS where KEY_NAME=\"%s\"",
			(const char *)RDEscapeString(keyname));
  q=new RDSqlQuery(sql);
  if(q->first()) {
    feed_id=q->value(0).toUInt();
  }
  delete q;
}


RDFeed::RDFeed(unsigned id,QObject *parent,const char *name)
  : QObject(parent,name)
{
  RDSqlQuery *q;
  QString sql;

  feed_id=id;

  sql=QString().sprintf("select KEY_NAME from FEEDS where ID=%u",id);
  q=new RDSqlQuery(sql);
  if(q->first()) {
    feed_keyname=q->value(0).toString();
  }
  delete q;
}


bool RDFeed::exists() const
{
  return RDDoesRowExist("FEEDS","NAME",feed_keyname);
}


QString RDFeed::keyName() const
{
  return feed_keyname;
}


unsigned RDFeed::id() const
{
  return feed_id;
}


QString RDFeed::channelTitle() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_TITLE").
    toString();
}


void RDFeed::setChannelTitle(const QString &str) const
{
  SetRow("CHANNEL_TITLE",str);
}


QString RDFeed::channelDescription() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_DESCRIPTION").
    toString();
}


void RDFeed::setChannelDescription(const QString &str) const
{
  SetRow("CHANNEL_DESCRIPTION",str);
}


QString RDFeed::channelCategory() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_CATEGORY").
    toString();
}


void RDFeed::setChannelCategory(const QString &str) const
{
  SetRow("CHANNEL_CATEGORY",str);
}


QString RDFeed::channelLink() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_LINK").
    toString();
}


void RDFeed::setChannelLink(const QString &str) const
{
  SetRow("CHANNEL_LINK",str);
}


QString RDFeed::channelCopyright() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_COPYRIGHT").
    toString();
}


void RDFeed::setChannelCopyright(const QString &str) const
{
  SetRow("CHANNEL_COPYRIGHT",str);
}


QString RDFeed::channelWebmaster() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_WEBMASTER").
    toString();
}



void RDFeed::setChannelWebmaster(const QString &str) const
{
  SetRow("CHANNEL_WEBMASTER",str);
}


QString RDFeed::channelLanguage() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_LANGUAGE").
    toString();
}


void RDFeed::setChannelLanguage(const QString &str)
{
  SetRow("CHANNEL_LANGUAGE",str);
}


QString RDFeed::baseUrl() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"BASE_URL").
    toString();
}


void RDFeed::setBaseUrl(const QString &str) const
{
  SetRow("BASE_URL",str);
}


QString RDFeed::basePreamble() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"BASE_PREAMBLE").
    toString();
}


void RDFeed::setBasePreamble(const QString &str) const
{
  SetRow("BASE_PREAMBLE",str);
}


QString RDFeed::purgeUrl() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"PURGE_URL").
    toString();
}


void RDFeed::setPurgeUrl(const QString &str) const
{
  SetRow("PURGE_URL",str);
}


QString RDFeed::purgeUsername() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"PURGE_USERNAME").
    toString();
}


void RDFeed::setPurgeUsername(const QString &str) const
{
  SetRow("PURGE_USERNAME",str);
}


QString RDFeed::purgePassword() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"PURGE_PASSWORD").
    toString();
}


void RDFeed::setPurgePassword(const QString &str) const
{
  SetRow("PURGE_PASSWORD",str);
}


QString RDFeed::headerXml() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"HEADER_XML").
    toString();
}


void RDFeed::setHeaderXml(const QString &str)
{
  SetRow("HEADER_XML",str);
}


QString RDFeed::channelXml() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_XML").
    toString();
}


void RDFeed::setChannelXml(const QString &str)
{
  SetRow("CHANNEL_XML",str);
}


QString RDFeed::itemXml() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"ITEM_XML").
    toString();
}


void RDFeed::setItemXml(const QString &str)
{
  SetRow("ITEM_XML",str);
}


bool RDFeed::castOrder() const
{
  return RDBool(RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
			      "CAST_ORDER").toString());
}


void RDFeed::setCastOrder(bool state) const
{
  SetRow("CAST_ORDER",RDYesNo(state));
}


int RDFeed::maxShelfLife() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"MAX_SHELF_LIFE").toInt();
}


void RDFeed::setMaxShelfLife(int days)
{
  SetRow("MAX_SHELF_LIFE",days);
}


QDateTime RDFeed::lastBuildDateTime() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"LAST_BUILD_DATETIME").
    toDateTime();
}


void RDFeed::setLastBuildDateTime(const QDateTime &datetime) const
{
  SetRow("LAST_BUILD_DATETIME",datetime.toString("yyyy-MM-dd hh:mm:ss"));
}


QDateTime RDFeed::originDateTime() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"ORIGIN_DATETIME").
    toDateTime();
}


void RDFeed::setOriginDateTime(const QDateTime &datetime) const
{
  SetRow("ORIGIN_DATETIME",datetime.toString("yyyy-MM-dd hh:mm:ss"));
}


bool RDFeed::enableAutopost() const
{
  return RDBool(RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
			      "ENABLE_AUTOPOST").toString());
}


void RDFeed::setEnableAutopost(bool state) const
{
  SetRow("ENABLE_AUTOPOST",RDYesNo(state));
}


bool RDFeed::keepMetadata() const
{
  return RDBool(RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
			      "KEEP_METADATA").toString());
}


void RDFeed::setKeepMetadata(bool state)
{
  SetRow("KEEP_METADATA",RDYesNo(state));
}


RDSettings::Format RDFeed::uploadFormat() const
{
  return (RDSettings::Format)RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
					   "UPLOAD_FORMAT").toInt();
}


void RDFeed::setUploadFormat(RDSettings::Format fmt) const
{
  SetRow("UPLOAD_FORMAT",(int)fmt);
}


int RDFeed::uploadChannels() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_CHANNELS").
    toInt();
}


void RDFeed::setUploadChannels(int chans) const
{
  SetRow("UPLOAD_CHANNELS",chans);
}


int RDFeed::uploadQuality() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_QUALITY").
    toInt();
}


void RDFeed::setUploadQuality(int qual) const
{
  SetRow("UPLOAD_QUALITY",qual);
}


int RDFeed::uploadBitRate() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_BITRATE").
    toInt();
}


void RDFeed::setUploadBitRate(int rate) const
{
  SetRow("UPLOAD_BITRATE",rate);
}


int RDFeed::uploadSampleRate() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_SAMPRATE").
    toInt();
}


void RDFeed::setUploadSampleRate(int rate) const
{
  SetRow("UPLOAD_SAMPRATE",rate);
}


QString RDFeed::uploadExtension() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_EXTENSION").
    toString();
}


void RDFeed::setUploadExtension(const QString &str)
{
  SetRow("UPLOAD_EXTENSION",str);
}


QString RDFeed::uploadMimetype() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_MIMETYPE").
    toString();
}


void RDFeed::setUploadMimetype(const QString &str)
{
  SetRow("UPLOAD_MIMETYPE",str);
}


int RDFeed::normalizeLevel() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"NORMALIZE_LEVEL").
    toInt();
}


void RDFeed::setNormalizeLevel(int lvl) const
{
  SetRow("NORMALIZE_LEVEL",lvl);
}


QString RDFeed::redirectPath() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"REDIRECT_PATH").
    toString();
}


void RDFeed::setRedirectPath(const QString &str)
{
  SetRow("REDIRECT_PATH",str);
}


RDFeed::MediaLinkMode RDFeed::mediaLinkMode() const
{
  return (RDFeed::MediaLinkMode)RDGetSqlValue("FEEDS","KEY_NAME",
					      feed_keyname,"MEDIA_LINK_MODE").
    toUInt();
}
  

void RDFeed::setMediaLinkMode(RDFeed::MediaLinkMode mode) const
{
  SetRow("MEDIA_LINK_MODE",(unsigned)mode);
}


QString RDFeed::audioUrl(RDFeed::MediaLinkMode mode,unsigned cast_id)
{
  QUrl url(baseUrl());
  QString ret;
  RDPodcast *cast;

  switch(mode) {
    case RDFeed::LinkNone:
      ret="";
      break;

    case RDFeed::LinkDirect:
      cast=new RDPodcast(cast_id);
      ret=QString().sprintf("%s/%s",
			    (const char *)baseUrl(),
			    (const char *)cast->audioFilename());
      delete cast;
      break;

    case RDFeed::LinkCounted:
      ret=QString().sprintf("http://%s%s/rd-bin/rdfeed.%s?%s&cast_id=%d",
			    (const char *)basePreamble(),
			    (const char *)url.host(),
			    (const char *)uploadExtension(),
			    (const char *)keyName(),
			    cast_id);
      break;
  }
  return ret;
}


unsigned RDFeed::postCut(RDStation *station,const QString &cutname,Error *err)
{
  QString tmpfile;
  QString destfile;
  QString sql;
  RDSqlQuery *q;
  RDPodcast *cast=NULL;

  emit postProgressChanged(0);
  emit postProgressChanged(1);
  QString cut_filename=RDCut::pathName(cutname); 
  QString cmd=GetExportCommand(station,cut_filename,&tmpfile,err);
  // printf("CMD: %s\n",(const char *)cmd);
  if(system(cmd)!=0) {
    emit postProgressChanged(totalPostSteps());
    *err=RDFeed::ErrorGeneral;
    return 0;
    }
  emit postProgressChanged(2);
  QFile file(tmpfile);
  int length=file.size();
  RDCut *cut=new RDCut(cutname);
  unsigned cast_id=CreateCast(&destfile,length,cut->length());
  delete cut;
  cast=new RDPodcast(cast_id);
  cmd=cast->audioUploadCommand(tmpfile);
  if(system(cmd)!=0) {
    emit postProgressChanged(totalPostSteps());
    *err=RDFeed::ErrorUploadFailed;
    sql=QString().sprintf("delete from PODCASTS where ID=%u",cast_id);
    q=new RDSqlQuery(sql);
    delete q;
    delete cast;
    return 0;
  }
  emit postProgressChanged(3);
  unlink(tmpfile);
  delete cast;

  emit postProgressChanged(totalPostSteps());

  return cast_id;
}


unsigned RDFeed::postFile(RDStation *station,const QString &srcfile,Error *err)
{
  QString sql;
  RDSqlQuery *q;
  QString cmd;
  QString tmpfile;
  QString tmpfile2;
  QString destfile;
  int time_length;

  emit postProgressChanged(0);
  emit postProgressChanged(1);
  RDWaveFile *wave=new RDWaveFile(srcfile);
  if(!wave->openWave()) {
    emit postProgressChanged(totalPostSteps());
    delete wave;
    *err=RDFeed::ErrorUnsupportedType;
    return 0;
  }
  time_length=wave->getExtTimeLength();
  delete wave;

  cmd=GetImportCommand(station,srcfile,&tmpfile,err);
  if(*err!=RDFeed::ErrorOk) {
    return 0;
  }
  if(system(cmd)!=0) {
    emit postProgressChanged(totalPostSteps());
    unlink(tmpfile);
    *err=RDFeed::ErrorGeneral;
    return 0;
  }
  emit postProgressChanged(2);
  cmd=GetExportCommand(station,tmpfile,&tmpfile2,err);
  if(*err!=RDFeed::ErrorOk) {
    unlink(tmpfile);
    return 0;
  }
  if(system(cmd)!=0) {
    emit postProgressChanged(totalPostSteps());
    unlink(tmpfile);
    unlink(tmpfile2);
    *err=RDFeed::ErrorGeneral;
    return 0;
  }
  emit postProgressChanged(3);
  QFile file(tmpfile2);
  int length=file.size();

  unsigned cast_id=CreateCast(&destfile,length,time_length);
  RDPodcast *cast=new RDPodcast(cast_id);
  cmd=cast->audioUploadCommand(tmpfile2);
  if(system(cmd)!=0) {
    sql=QString().sprintf("delete from PODCASTS where ID=%u",cast_id);
    q=new RDSqlQuery(sql);
    delete q;
    delete cast;
    unlink(tmpfile);
    unlink(tmpfile2);
    *err=RDFeed::ErrorUploadFailed;
    return 0;
  }
  delete cast;
  unlink(QString().sprintf("%s.wav",(const char *)tmpfile));
  unlink(tmpfile);
  unlink(tmpfile2);
  emit postProgressChanged(totalPostSteps());

  *err=RDFeed::ErrorOk;
  return cast_id;
}


int RDFeed::totalPostSteps() const
{
  return RDFEED_TOTAL_POST_STEPS;
}


QString RDFeed::errorString(RDFeed::Error err)
{
  QString ret="Unknown Error";

  switch(err) {
  case RDFeed::ErrorOk:
    ret="Ok";
    break;

  case RDFeed::ErrorNoFile:
    ret="No such file or directory";
    break;

  case RDFeed::ErrorCannotOpenFile:
    ret="Cannot open file";
    break;

  case RDFeed::ErrorUnsupportedType:
    ret="Unsupported file format";
    break;

  case RDFeed::ErrorUploadFailed:
    ret="Upload failed";
    break;

  case RDFeed::ErrorGeneral:
    ret="General Error";
    break;
  }
  return ret;
}


QString RDFeed::GetImportCommand(RDStation *station,const QString &srcfile,
				 QString *destfile,Error *err) const
{
  int format_in=0;
  int temp_length;
  int finished_length;
  QString cmd;

  if(!QFile::exists(srcfile)) {
    *err=RDFeed::ErrorNoFile;
    return QString();
  }
  RDWaveFile *wave=new RDWaveFile(srcfile);
  if(!wave->openWave()) {
    *err=RDFeed::ErrorCannotOpenFile;
    delete wave;
    return QString();
  }
  if(wave->type()==RDWaveFile::Unknown) {
    *err=RDFeed::ErrorUnsupportedType;
    wave->closeWave();
    delete wave;
    return QString();
  }
  int samplerate=wave->getSamplesPerSec();
  switch(wave->getFormatTag()) {
      case WAVE_FORMAT_PCM:
	format_in=0;
	temp_length=wave->getSampleLength()*
	  wave->getChannels()*(wave->getBitsPerSample()/8);
	break;

      case WAVE_FORMAT_MPEG:
	format_in=wave->getHeadLayer();
	temp_length=wave->getSampleLength()*wave->getChannels()*2;
	break;

      case WAVE_FORMAT_FLAC:
	format_in=0;
	temp_length=wave->getSampleLength()*wave->getChannels()*2;
	break;
  }
  delete wave;

  int lib_fmt=0;
  finished_length=
    (int)(((double)temp_length/2.0)*(double)uploadChannels()*
	  ((double)uploadSampleRate())/44100.0);
  lib_fmt=0;
  *destfile=GetTempFilename();
  QString tempwav_name=*destfile+".wav";
  QString tempdat_name=*destfile+".dat";

  float normal=0.0;
  normal=pow(10.0,(double)(-1.0/20.0));
  RDLibraryConf *rdlibrary=new RDLibraryConf(station->name(),0);
  cmd=QString().
    sprintf("rd_import_file %6.4f %d %d %s %d %d %d %d %s %s %s %d",
	    normal,
	    format_in,
	    samplerate,
	    (const char *)RDEscapeString(srcfile.utf8()),  
	    lib_fmt,
	    uploadChannels(),
	    uploadSampleRate(),
	    (uploadChannels())*uploadBitRate()/1000,
	    (const char *)(*destfile),
	    (const char *)tempdat_name,
	    (const char *)tempwav_name,
	    rdlibrary->srcConverter());
  // printf("CMD: %s\n",(const char *)cmd);
  *err=RDFeed::ErrorOk;
  delete rdlibrary;

  return cmd;
}


QString RDFeed::GetExportCommand(RDStation *station,const QString &srcfile,
				 QString *destfile,RDFeed::Error *err) const
{
  int format_in=0;
  int len;
  RDSettings settings;
  QString custom_cmd;

  *destfile=GetTempFilename();
  QFile file(srcfile);
  if(!file.exists()) {
    *err=RDFeed::ErrorCannotOpenFile;
    return QString();
  }
  RDWaveFile *wave=new RDWaveFile(srcfile);
  if(!wave->openWave()) {
    *err=RDFeed::ErrorUnsupportedType;
    delete wave;
    return QString();
  }
  int samplerate=wave->getSamplesPerSec();
  switch(wave->getFormatTag()) {
      case WAVE_FORMAT_PCM:
	format_in=0;
	len=wave->getSampleLength()*wave->getChannels()*
	  (wave->getBitsPerSample()/8);
	break;

      case WAVE_FORMAT_MPEG:
	format_in=wave->getHeadLayer();
	len=wave->getSampleLength()*wave->getChannels()*2;
	break;
  }
  wave->closeWave();
  delete wave;

  QString cmd;
  float normal=0.0;
  RDLibraryConf *rdlibrary=new RDLibraryConf(station->name(),0);
  if(normalizeLevel()<=0) {
    normal=pow(10.0,(double)(normalizeLevel()/20000.0));
    cmd=QString().
      sprintf("rd_export_file %6.4f %d %d %s %d %d %d %d %d %s %s.dat %s.%s %d",
	      normal,
	      format_in,
	      samplerate,
	      (const char *)srcfile,
	      uploadFormat(),
	      uploadChannels(),
	      uploadSampleRate(),
	      uploadBitRate()/1000,
	      uploadQuality(),
	      (const char *)RDEscapeString(*destfile),
	      (const char *)(*destfile),
	      (const char *)(*destfile),
	      RDConfiguration()->audioExtension().ascii(),
	      rdlibrary->srcConverter());
  }
  else {
    cmd=QString().
      sprintf("rd_export_file 0 %d %d %s %d %d %d %d %d %s %s.dat %s.%s %d",
	      format_in,
	      samplerate,
	      (const char *)srcfile,
	      uploadFormat(),
	      uploadChannels(),
	      uploadSampleRate(),
	      uploadBitRate()/1000,
	      uploadQuality(),
	      (const char *)RDEscapeString(*destfile),
	      (const char *)(*destfile),
	      (const char *)(*destfile),
	      RDConfiguration()->audioExtension().ascii(),
	      rdlibrary->srcConverter());
  }
  delete rdlibrary;

  switch(uploadFormat()) {  // Custom format?
    case RDSettings::Pcm16:
    case RDSettings::MpegL1:
    case RDSettings::MpegL2:
    case RDSettings::MpegL3:
    case RDSettings::Flac:
    case RDSettings::OggVorbis:
      break;

    default:
      settings.setFormat(uploadFormat());
      settings.setChannels(uploadChannels());
      settings.setSampleRate(uploadSampleRate());
      settings.setBitRate(uploadBitRate());
      custom_cmd=settings.resolvedCustomCommandLine(*destfile);
      if(custom_cmd.isEmpty()) {
	return QString();
      }
      cmd+=" \""+custom_cmd+"\"";
      break;
  }

  // printf("CMD: %s",(const char *)cmd);
  *err=RDFeed::ErrorOk;

  return cmd;
}


unsigned RDFeed::CreateCast(QString *filename,int bytes,int msecs) const
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;
  unsigned cast_id=0;
  QDateTime current_datetime=
    QDateTime(QDate::currentDate(),QTime::currentTime());

  sql=QString().sprintf("select CHANNEL_TITLE,CHANNEL_DESCRIPTION,\
                         CHANNEL_CATEGORY,CHANNEL_LINK,MAX_SHELF_LIFE,\
                         UPLOAD_FORMAT,UPLOAD_EXTENSION from FEEDS \
                         where ID=%u",feed_id);
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return 0;
  }

  //
  // Create Entry
  //
  sql=QString().sprintf("insert into PODCASTS set \
                         FEED_ID=%u,\
                         ITEM_TITLE=\"%s\",\
                         ITEM_DESCRIPTION=\"%s\",\
                         ITEM_CATEGORY=\"%s\",\
                         ITEM_LINK=\"%s\",\
                         SHELF_LIFE=%d,\
                         EFFECTIVE_DATETIME=\"%s\",\
                         ORIGIN_DATETIME=\"%s\"",
			feed_id,
			(const char *)RDEscapeString(q->value(0).toString()),
			(const char *)RDEscapeString(q->value(1).toString()),
			(const char *)RDEscapeString(q->value(2).toString()),
			(const char *)RDEscapeString(q->value(3).toString()),
			q->value(4).toInt(),
			(const char *)current_datetime.
			toString("yyyy-MM-dd hh:mm:ss"),
			(const char *)current_datetime.
			toString("yyyy-MM-dd hh:mm:ss"));
  q1=new RDSqlQuery(sql);
  delete q1;

  //
  // Get the ID
  //
  sql=QString().sprintf("select ID from PODCASTS \
                         where (FEED_ID=%u)&&(ORIGIN_DATETIME=\"%s\")",
			feed_id,
			(const char *)current_datetime.
			toString("yyyy-MM-dd hh:mm:ss"));
  q1=new RDSqlQuery(sql);
  if(q1->first()) {
    cast_id=q1->value(0).toUInt();
  }
  delete q1;

  //
  // Generate the Filename
  //
  *filename=QString().
    sprintf("%s.%s",
	    (const char *)QString().sprintf("%06u_%06u",feed_id,cast_id),
	    (const char *)q->value(6).toString());
  sql=QString().sprintf("update PODCASTS set AUDIO_FILENAME=\"%s\",\
                         AUDIO_LENGTH=%d,\
                         AUDIO_TIME=%d where ID=%u",
			(const char *)(*filename),
			bytes,msecs,cast_id);
  q1=new RDSqlQuery(sql);
  delete q1;
  delete q;
  return cast_id;
}


QString RDFeed::GetTempFilename() const
{
  char tempname[PATH_MAX];

  sprintf(tempname,"%s/podcastXXXXXX",(const char *)RDTempDir());
  if(mkstemp(tempname)<0) {
    return QString();
  }

  return QString(tempname);
}


void RDFeed::SetRow(const QString &param,int value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE FEEDS SET %s=%d WHERE KEY_NAME=\"%s\"",
			(const char *)param,
			value,
			(const char *)feed_keyname);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDFeed::SetRow(const QString &param,const QString &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE FEEDS SET %s=\"%s\" WHERE KEY_NAME=\"%s\"",
			(const char *)param,
			(const char *)RDEscapeString(value),
			(const char *)feed_keyname);
  q=new RDSqlQuery(sql);
  delete q;
}
