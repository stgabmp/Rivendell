// rdpodcast.cpp
//
// Abstract a Rivendell Podcast
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdpodcast.cpp,v 1.6.2.1.2.1 2010/05/11 14:43:12 cvs Exp $
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

#include <rddb.h>
#include <rdpodcast.h>
#include <rdconf.h>
#include <rdescape_string.h>
#include <rdurl.h>


RDPodcast::RDPodcast(unsigned id)
{
  RDSqlQuery *q;
  QString sql;

  podcast_id=id;
  sql=QString().sprintf("select FEEDS.KEY_NAME from \
                         PODCASTS left join FEEDS \
                         on (PODCASTS.FEED_ID=FEEDS.ID) \
                         where PODCASTS.ID=%u",id);
  q=new RDSqlQuery(sql);
  if(q->first()) {
    podcast_keyname=q->value(0).toString();
  }
  delete q;
}


unsigned RDPodcast::id() const
{
  return podcast_id;
}


QString RDPodcast::keyName() const
{
  return podcast_keyname;
}


bool RDPodcast::exists() const
{
  return RDDoesRowExist("PODCASTS","ID",podcast_id);
}


unsigned RDPodcast::feedId() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"FEED_ID").
    toUInt();
}


void RDPodcast::setFeedId(unsigned id) const
{
  SetRow("FEED_ID",id);
}


QString RDPodcast::itemTitle() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"ITEM_TITLE").
    toString();
}


void RDPodcast::setItemTitle(const QString &str) const
{
  SetRow("ITEM_TITLE",str);
}


QString RDPodcast::itemDescription() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,
		       "ITEM_DESCRIPTION").toString();
}


void RDPodcast::setItemDescription(const QString &str) const
{
  SetRow("ITEM_DESCRIPTION",str);
}


QString RDPodcast::itemCategory() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"ITEM_CATEGORY").
    toString();
}


void RDPodcast::setItemCategory(const QString &str) const
{
  SetRow("ITEM_CATEGORY",str);
}


QString RDPodcast::itemLink() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"ITEM_LINK").
    toString();
}


void RDPodcast::setItemLink(const QString &str) const
{
  SetRow("ITEM_LINK",str);
}


QString RDPodcast::itemAuthor() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"ITEM_AUTHOR").
    toString();
}


void RDPodcast::setItemAuthor(const QString &str) const
{
  SetRow("ITEM_AUTHOR",str);
}


QString RDPodcast::itemComments() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"ITEM_COMMENTS").
    toString();
}


void RDPodcast::setItemComments(const QString &str) const
{
  SetRow("ITEM_COMMENTS",str);
}


QString RDPodcast::itemSourceText() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"ITEM_SOURCE_TEXT").
    toString();
}


void RDPodcast::setItemSourceText(const QString &str) const
{
  SetRow("ITEM_SOURCE_TEXT",str);
}


QString RDPodcast::itemSourceUrl() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"ITEM_SOURCE_URL").
    toString();
}


void RDPodcast::setItemSourceUrl(const QString &str) const
{
  SetRow("ITEM_SOURCE_URL",str);
}


QDateTime RDPodcast::originDateTime() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,
		       "ORIGIN_DATETIME").toDateTime();
}


void RDPodcast::setOriginDateTime(const QDateTime &datetime) const
{
  SetRow("ORIGIN_DATETIME",datetime.toString("yyyy-MM-dd hh:mm:ss"));
}


QDateTime RDPodcast::effectiveDateTime() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,
		       "EFFECTIVE_DATETIME").toDateTime();
}


void RDPodcast::setEffectiveDateTime(const QDateTime &datetime) const
{
  SetRow("EFFECTIVE_DATETIME",datetime.toString("yyyy-MM-dd hh:mm:ss"));
}


QString RDPodcast::audioFilename() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,
		       "AUDIO_FILENAME").toString();
}


void RDPodcast::setAudioFilename(const QString &str) const
{
  SetRow("AUDIO_FILENAME",str);
}


int RDPodcast::audioLength() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"AUDIO_LENGTH").
    toUInt();
}


void RDPodcast::setAudioLength(int len) const
{
  SetRow("AUDIO_LENGTH",len);
}


int RDPodcast::audioTime() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"AUDIO_TIME").
    toUInt();
}


void RDPodcast::setAudioTime(int msecs) const
{
  SetRow("AUDIO_TIME",msecs);
}


unsigned RDPodcast::shelfLife() const
{
  return RDGetSqlValue("PODCASTS","ID",podcast_id,"SHELF_LIFE").
    toUInt();
}


void RDPodcast::setShelfLife(unsigned days) const
{
  SetRow("SHELF_LIFE",days);
}


RDPodcast::Status RDPodcast::status() const
{
  return (RDPodcast::Status)RDGetSqlValue("PODCASTS","ID",
					  podcast_id,"STATUS").toUInt();
}


void RDPodcast::setStatus(RDPodcast::Status status)
{
  SetRow("STATUS",(unsigned)status);
}


QString RDPodcast::audioUploadCommand(const QString &srcfile) const
{
  QString sql;
  RDSqlQuery *q;
  QString cmd;

  //
  // Get Cast Values
  //
  sql=QString().sprintf("select FEED_ID,AUDIO_FILENAME from PODCASTS \
                         where ID=%u",podcast_id);
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return QString();
  }
  unsigned feed_id=q->value(0).toUInt();
  QString audio_filename=q->value(1).toString();
  delete q;

  //
  // Get Feed Values
  //
  sql=QString().sprintf("select PURGE_URL,PURGE_USERNAME,PURGE_PASSWORD \
                         from FEEDS where ID=%u",feed_id);
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return QString();
  }
  QString purge_url=q->value(0).toString();
  QString purge_username=q->value(1).toString();
  QString purge_password=q->value(2).toString();
  delete q;

  //
  // Build Command
  //
  RDUrl url(purge_url+"/"+audio_filename);

  if(url.protocol()=="file") {
    cmd=QString().sprintf("cp %s %s",(const char *)srcfile,
			  (const char *)url.path());
  }

  if(url.protocol()=="ftp") {
    QString dir=RDGetPathPart(url.path());
    if(dir.right(dir.length()-1).isEmpty()) {
      cmd=QString().
	sprintf("lftp -e \"set net:max-retries 1;put %s -o %s;bye\"",
		(const char *)srcfile,
		(const char *)RDGetBasePart(url.path()));
    }
    else {
      cmd=QString().
	sprintf("lftp -e \"set net:max-retries 1;put -O %s %s -o %s;bye\"",
		(const char *)dir.right(dir.length()-1),
		(const char *)srcfile,
		(const char *)RDGetBasePart(url.path()));
    }
    if(!purge_username.isEmpty()) {
      cmd+=QString().sprintf(" -u \"%s:",(const char *)purge_username);
      if(purge_password.isEmpty()) {
	cmd+="\"";
      }
      else {
	cmd+=QString().sprintf("%s\"",(const char *)purge_password);
      }
    }
    cmd+=QString().sprintf(" %s",(const char *)url.host());
  }

  if(url.protocol()=="smb") {
    QString path=RDGetPathPart(url.path());
    cmd=QString().sprintf("smbclient \"%s\" ",(const char *)url.smbShare());
      if(!purge_password.isEmpty()) {
	cmd+=QString().sprintf("\"%s\" ",(const char *)purge_password);
      }
      if(!purge_username.isEmpty()) {
	cmd+=QString().sprintf("-U %s ",(const char *)purge_username);
      }
      cmd+=QString().sprintf("-c \"put %s %s\"",(const char *)srcfile,
			     (const char *)url.smbPath());
  }

  return cmd;
}


QString RDPodcast::audioPurgeCommand() const
{
  QString sql;
  RDSqlQuery *q;
  QString cmd;

  //
  // Get Cast Values
  //
  sql=QString().sprintf("select FEED_ID,AUDIO_FILENAME from PODCASTS \
                         where ID=%u",podcast_id);
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return QString();
  }
  unsigned feed_id=q->value(0).toUInt();
  QString audio_filename=q->value(1).toString();
  delete q;

  //
  // Get Feed Values
  //
  sql=QString().sprintf("select PURGE_URL,PURGE_USERNAME,PURGE_PASSWORD \
                         from FEEDS where ID=%u",feed_id);
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return QString();
  }
  QString purge_url=q->value(0).toString();
  QString purge_username=q->value(1).toString();
  QString purge_password=q->value(2).toString();
  delete q;

  RDUrl url(purge_url);

  if(url.protocol()=="file") {
    cmd=QString().sprintf("rm \"%s/%s\"",(const char *)url.path(),
			  (const char *)audio_filename);
  }
  if(url.protocol()=="ftp") {
    cmd=QString().sprintf("lftp -e \"set net:max-retries 1;rm %s;bye\"",
			  (const char *)audio_filename);
    if(!purge_username.isEmpty()) {
      cmd+=QString().sprintf(" -u \"%s:",(const char *)purge_username);
      if(purge_password.isEmpty()) {
	cmd+="\"";
      }
      else {
	cmd+=QString().sprintf("%s\"",(const char *)purge_password);
      }
    }
    cmd+=QString().sprintf(" ftp://%s%s",(const char *)url.host(),
				 (const char *)url.path());
    cmd+=" > /dev/null 2> /dev/null";
  }
  if(url.protocol()=="smb") {
    if(!url.validSmbShare()) {
      return QString();
    }
    QString path=RDGetPathPart(url.path());
    cmd=QString().sprintf("smbclient \"%s\" ",(const char *)url.smbShare());
    if(!purge_password.isEmpty()) {
      cmd+=QString().sprintf("\"%s\" ",(const char *)purge_password);
    }
    if(!purge_username.isEmpty()) {
      cmd+=QString().sprintf("-U %s ",(const char *)purge_username);
    }
    cmd+=QString().
      sprintf("-c \"cd %s;del %s\"",(const char *)url.smbPath(),
	      (const char *)audio_filename);
  }  

  return cmd;
}


QString RDPodcast::guid(const QString &url,const QString &filename,
			unsigned feed_id,unsigned cast_id)
{
  return QString().sprintf("%s/%s_%06u_%06u",
			   (const char *)url,(const char *)filename,
			   feed_id,cast_id);
}


QString RDPodcast::guid(const QString &full_url,unsigned feed_id,
			unsigned cast_id)
{
  return QString().sprintf("%s_%06u_%06u",
			   (const char *)full_url,feed_id,cast_id);
}


void RDPodcast::SetRow(const QString &param,int value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE PODCASTS SET %s=%d WHERE ID=%u",
			(const char *)param,
			value,
			podcast_id);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDPodcast::SetRow(const QString &param,const QString &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE PODCASTS SET %s=\"%s\" WHERE ID=%u",
			(const char *)param,
			(const char *)RDEscapeString(value),
			podcast_id);
  q=new RDSqlQuery(sql);
  delete q;
}
