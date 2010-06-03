// batch.cpp
//
// Batch Routines for the Rivendell netcatcher daemon
//
//   (C) Copyright 2002-2007, 2010 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: batch.cpp,v 1.1.2.1 2010/05/11 13:06:20 cvs Exp $
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

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <netdb.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <vector>

#include <qapplication.h>
#include <qtimer.h>
#include <qsignalmapper.h>
#include <qsessionmanager.h>

#include <rddb.h>
#include <rdconf.h>
#include <rdurl.h>
#include <rdwavefile.h>
#include <rdcut.h>
#include <rdcatchd_socket.h>
#include <rdcatchd.h>
#include <rdrecording.h>
#include <rdttyout.h>
#include <rdmixer.h>
#include <rdcheck_daemons.h>
#include <rddebug.h>
#include <rddatedecode.h>
#include <rdcmd_switch.h>
#include <rdescape_string.h>
#include <rdpodcast.h>
#include <rdsettings.h>
#include <rdlibrary_conf.h>

void MainObject::catchConnectedData(int serial,bool state)
{
  if(!state) {
    LogLine(RDConfig::LogErr,"unable to connect to rdcatchd(8) daemon");
    exit(256);
  }

  //
  // Dispatch Handler
  //
  switch(batch_event->type()) {
  case RDRecording::Recording:
    RunImport(batch_event);
    break;

  case RDRecording::Download:
    RunDownload(batch_event);
    break;

  case RDRecording::Upload:
    RunUpload(batch_event);
    break;

  default:
    fprintf(stderr,"rdcatchd: nothing to do for this event type\n");
    exit(256);
  }

  exit(0);
}


void MainObject::RunBatch(RDCmdSwitch *cmd)
{
  bool ok=false;
  int id=-1;

  //
  // Get ID
  //
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--event-id") {
      id=cmd->value(i).toInt(&ok);
      if((!ok)||(id<0)) {
	fprintf(stderr,"rdcatchd: invalid event-id\n");
	exit(256);
      }
    }
  }
  if(id<0) {
    fprintf(stderr,"rdcatchd: missing event-id\n");
    exit(256);
  }

  //
  // Calculate Temporary Directory
  //
  catch_temp_dir=RDTempDir();

  //
  // Open Database
  //
  QString err (tr("ERROR rdcatchd aborting - "));

  catch_db=RDInitDb (&err);
  if(!catch_db) {
    printf(err.ascii());
    exit(1);
  }
  connect (RDDbStatus(),SIGNAL(logText(RDConfig::LogPriority,const QString &)),
	   this,SLOT(log(RDConfig::LogPriority,const QString &)));

  //
  // Load Event
  //
  QString sql=LoadEventSql()+QString().sprintf(" where ID=%d",id);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    fprintf(stderr,"rdcatchd: id %d not found\n",id);
    exit(256);
  }
  batch_event=new CatchEvent();
  LoadEvent(q,batch_event,false);
  delete q;

  //
  // Open Status Connection
  //
  catch_connect=new RDCatchConnect(0,this);
  connect(catch_connect,SIGNAL(connected(int,bool)),
	  this,SLOT(catchConnectedData(int,bool)));
  catch_connect->
    connectHost("localhost",RDCATCHD_TCP_PORT,catch_config->password());
}


void MainObject::RunImport(CatchEvent *evt)
{
  evt->setTempName(GetTempRecordingName(evt->id()));
  Import(evt);
  unlink(evt->tempName());
}


void MainObject::RunDownload(CatchEvent *evt)
{
  //
  // Resolve Wildcards
  //
  RDStation *station=new RDStation(catch_config->stationName());
  evt->resolveUrl(station->timeOffset());
  delete station;
  QString xload_cmd;
  QString import_cmd;
  RDUrl url(evt->resolvedUrl());
  QString protocol=url.protocol();
  QString temp_importname;

  //
  // Build Command Lines
  //
  if(protocol=="file") {
    evt->setTempName(url.path());
    evt->setDeleteTempFile(false);
  }
  if((protocol=="http")||(protocol=="https")) {
    evt->setTempName(BuildTempName(evt,"download"));
    evt->setDeleteTempFile(true);
    xload_cmd="wget -q ";
    if(!evt->urlUsername().isEmpty()) {
      xload_cmd+=QString().sprintf("--http-user %s ",
				   (const char *)
				   evt->urlUsername());
      if(!evt->urlPassword().isEmpty()) {
	xload_cmd+=QString().sprintf("--http-passwd \"%s\" ",
				     (const char *)
				     evt->urlPassword());
      }
    }
    xload_cmd+=QString().
      sprintf("-O %s \"%s\"",(const char *)evt->tempName(),
	      (const char *)evt->resolvedUrl());
  }
  if(protocol=="ftp") {
    evt->setTempName(BuildTempName(evt,"download"));
    evt->setDeleteTempFile(true);
    QString urlpath=url.path().right(url.path().length()-1);
    urlpath.replace(" ","\\ ");
    QString urlbase=RDGetBasePart(evt->tempName());
    urlbase.replace(" ","\\ ");
    xload_cmd=QString().sprintf("lftp -e \"set net:max-retries 1;get -O %s %s -o %s;bye\"",
				(const char *)RDGetPathPart(evt->tempName()),
				(const char *)urlpath,
				(const char *)urlbase);
    if(!evt->urlUsername().isEmpty()) {
      xload_cmd+=QString().sprintf(" -u \"%s:",
				   (const char *)evt->urlUsername());
      if(evt->urlPassword().isEmpty()) {
	xload_cmd+="\"";
      }
      else {
	xload_cmd+=QString().sprintf("%s\"",(const char *)evt->urlPassword());
      }
    }
    xload_cmd+=QString().sprintf(" %s",(const char *)url.host());
  }
  if(protocol=="smb") {
    if(!url.validSmbShare()) {
      catch_connect->setExitCode(evt->id(),RDRecording::ServerError);
      qApp->processEvents();
      LogLine(RDConfig::LogWarning,QString().
	      sprintf("url \"%s\" is invalid, download aborted, id=%d",
		      (const char *)evt->resolvedUrl(),
		      evt->id()));
      exit(0);
    }
    evt->setTempName(BuildTempName(evt,"download"));
    evt->setDeleteTempFile(true);
    xload_cmd=QString().sprintf("smbclient \"%s\" ",
				(const char *)url.smbShare());
    if(!evt->urlPassword().isEmpty()) {
      xload_cmd+=QString().sprintf("\"%s\" ",(const char *)
				   evt->urlPassword());
    }
    if(!evt->urlUsername().isEmpty()) {
      xload_cmd+=QString().sprintf("-U %s ",(const char *)
				   evt->urlUsername());
    }
    xload_cmd+=QString().
      sprintf("-c \"get %s %s\"",(const char *)url.smbPath(),
	      (const char *)evt->tempName());
  }
  // LogLine(QString().sprintf("Xload Cmd: %s",(const char *)xload_cmd));
  
  //
  // Execute Download
  //
  if(!xload_cmd.isEmpty()) {
    LogLine(RDConfig::LogInfo,QString().
	    sprintf("started download of %s to %s, id=%d, cmdline=\"%s\"",
		    (const char *)evt->resolvedUrl(),
		    (const char *)evt->tempName(),
		    evt->id(),
		    (const char *)xload_cmd));
    if(system(xload_cmd.utf8())!=0) {
      catch_connect->setExitCode(evt->id(),RDRecording::ServerError);
      qApp->processEvents();
      LogLine(RDConfig::LogErr,QString().
	   sprintf("download of %s returned an error, id=%d, xload_cmd: %s\n",
		   (const char *)evt->resolvedUrl(),
		   evt->id(),
		   (const char *)xload_cmd));
      exit(256);
    }
    LogLine(RDConfig::LogInfo,QString().
	    sprintf("finished download of %s to %s, id=%d",
		    (const char *)evt->resolvedUrl(),
		    (const char *)evt->tempName(),
		    evt->id()));
  }

  //
  // Execute Import
  //
  if(!Import(evt)) {
    SendExitErrorMessage(evt,tr("unknown file type"),catch_conf->errorRml());
    return;
  }
}


void MainObject::RunUpload(CatchEvent *evt)
{
  //
  // Resolve Wildcards
  //
  RDStation *station=new RDStation(catch_config->stationName());
  evt->resolveUrl(station->timeOffset());
  delete station;

  QString xload_cmd;
  QString export_cmd;
  RDUrl url(evt->resolvedUrl());
  QString protocol=url.protocol();
  QString temp_exportname;
  QString urlpath;
  QString dirpath;

  //
  // Build Command Lines
  //
  if(protocol=="file") {
    evt->setTempName(url.path());
    evt->setDeleteTempFile(false);
  }
  if(protocol=="ftp") {
    evt->setTempName(BuildTempName(evt,"upload"));
    evt->setDeleteTempFile(true);
    QString dir=RDGetPathPart(url.path());
    if(dir=="/") {
      dir="//";
    }
    urlpath=RDGetBasePart(url.path());
    urlpath.replace(" ","\\ ");
    if(dir.right(dir.length()-1)=="/") {
      xload_cmd=QString().
	sprintf("lftp -e \"set net:max-retries 1;put %s -o %s;bye\"",
		(const char *)evt->tempName(),
		(const char *)urlpath);
    }
    else {
      dirpath=dir.right(dir.length()-1);
      dirpath.replace(" ","\\ ");
      xload_cmd=QString().
	sprintf("lftp -e \"set net:max-retries 1;put -O %s %s -o %s;bye\"",
		(const char *)dirpath,
		(const char *)evt->tempName(),
		(const char *)urlpath);
    }
    if(!evt->urlUsername().isEmpty()) {
      xload_cmd+=QString().sprintf(" -u \"%s:",
				   (const char *)evt->urlUsername());
      if(evt->urlPassword().isEmpty()) {
	xload_cmd+="\"";
      }
      else {
	xload_cmd+=QString().sprintf("%s\"",
				     (const char *)evt->urlPassword());
      }
    }
    xload_cmd+=QString().sprintf(" %s",(const char *)url.host());
  }
  if(protocol=="smb") {
    if(!url.validSmbShare()) {
      catch_connect->setExitCode(evt->id(),RDRecording::ServerError);
      LogLine(RDConfig::LogWarning,QString().
	      sprintf("url \"%s\" is invalid, upload aborted, id=%d",
		      (const char *)evt->resolvedUrl(),
		      evt->id()));
      qApp->processEvents();
      exit(256);
    }
    urlpath=url.smbPath();
    urlpath.replace(" ","\\ ");
    evt->setTempName(BuildTempName(evt,"upload"));
    evt->setDeleteTempFile(true);
    QString path=RDGetPathPart(url.path());
    xload_cmd=QString().sprintf("smbclient \"%s\" ",
				(const char *)url.smbShare());
    if(!evt->urlPassword().isEmpty()) {
      xload_cmd+=QString().sprintf("\"%s\" ",(const char *)
				   evt->urlPassword());
    }
    if(!evt->urlUsername().isEmpty()) {
      xload_cmd+=QString().sprintf("-U %s ",(const char *)
				   evt->urlUsername());
    }
    xload_cmd+=QString().
      sprintf("-c \"put %s %s\"",(const char *)evt->
	      tempName(),(const char *)urlpath);
  }
  
  //
  // Execute Export
  //
  LogLine(RDConfig::LogInfo,QString().
	  sprintf("started export of cut %s to %s, id=%d",
		  (const char *)evt->cutName(),
		  (const char *)evt->tempName(),
		  evt->id()));
  if(!Export(evt)) {
    LogLine(RDConfig::LogWarning,QString().
	    sprintf("export of cut %s returned an error, id=%d",
		    (const char *)evt->cutName(),
		    evt->id()));
    catch_connect->setExitCode(evt->id(),RDRecording::InternalError);
    qApp->processEvents();
    return;
  }
  LogLine(RDConfig::LogInfo,QString().
	  sprintf("finished export of cut %s to %s, id=%d",
		  (const char *)evt->cutName(),
		  (const char *)evt->tempName(),
		  evt->id()));
  //
  // Load Podcast Parameters
  //
  if(evt->feedId()>0) {
    QFile *file=new QFile(evt->tempName());
    evt->setPodcastLength(file->size());
    delete file;
    RDWaveFile *wave=new RDWaveFile(evt->tempName());
    if(wave->openWave()) {
      evt->setPodcastTime(wave->getExtTimeLength());
    }
    delete wave;
  }
  
  //
  // Execute Upload
  //
  if(!xload_cmd.isEmpty()) {
    LogLine(RDConfig::LogInfo,QString().
	    sprintf("started upload of %s to %s, id=%d, cmdline=\"%s\"",
		    (const char *)evt->tempName(),
		    (const char *)evt->
		    resolvedUrl(),
		    evt->id(),
		    (const char *)xload_cmd));
    if(system(xload_cmd.utf8())!=0) {
      unlink(evt->tempName());
      LogLine(RDConfig::LogDebug,QString().sprintf("deleted file %s",
						   (const char *)evt->tempName()));
      catch_connect->setExitCode(evt->id(),RDRecording::ServerError);
      qApp->processEvents();
      LogLine(RDConfig::LogWarning,QString().
	      sprintf("upload of %s returned an error, id=%d",
		      (const char *)evt->tempName(),
		      evt->id()));
      unlink(QString().sprintf("%s.%s",(const char *)temp_exportname,
			       RDConfiguration()->audioExtension().ascii()));
      unlink(temp_exportname+".dat");
      exit(256);
    }
    LogLine(RDConfig::LogInfo,QString().
	    sprintf("finished upload of %s to %s, id=%d",
		    (const char *)evt->tempName(),
		    (const char *)evt->
		    resolvedUrl(),
		    evt->id()));
  }
    
  //
  // Clean Up
  //
  if(evt->feedId()>0) {
    CheckInPodcast(evt);
  }
  if(evt->deleteTempFile()) {
    unlink(evt->tempName());
    LogLine(RDConfig::LogDebug,QString().sprintf("deleted file %s",
						 (const char *)evt->tempName()));
  }
  else {
    chown(evt->tempName(),catch_config->uid(),
	  catch_config->gid());
  }
  catch_connect->setExitCode(evt->id(),RDRecording::Ok);
  qApp->processEvents();
}


bool MainObject::Export(CatchEvent *evt)
{
  QString temp_exportname;
  QString export_cmd=GetExportCmd(evt,&temp_exportname);
  if(system(export_cmd.utf8())!=0) {
    unlink(QString().sprintf("%s.%s",(const char *)temp_exportname,
			     RDConfiguration()->audioExtension().ascii()));
    unlink(temp_exportname+".dat");
    return false;
  }
  unlink(QString().sprintf("%s.%s",(const char *)temp_exportname,
			   RDConfiguration()->audioExtension().ascii()));
  unlink(temp_exportname+".dat");
  return true;
}


QString MainObject::GetExportCmd(CatchEvent *evt,QString *tempname)
{
  int format_in=0;
  RDSettings settings;
  QString custom_cmd;

  //
  // Calculate Temporary Filenames
  //
  *tempname=QString().sprintf("%s/rdcatchd-export-%d",
			      (const char *)catch_temp_dir,
			      evt->id());

  QString local_filename=RDCut::pathName(evt->cutName()); 
  QFile file(local_filename);
  if(!file.exists()) {
    return QString();
  }
  RDWaveFile *wave=new RDWaveFile(local_filename);
  if(!wave->openWave()) {
    delete wave;
    return QString();
  }
  int samplerate=wave->getSamplesPerSec();
  switch(wave->getFormatTag()) {
      case WAVE_FORMAT_PCM:
	format_in=0;
	evt->
	  setTempLength(wave->getSampleLength()*
			wave->getChannels()*(wave->getBitsPerSample()/8));
	break;

      case WAVE_FORMAT_MPEG:
	format_in=wave->getHeadLayer();
	evt->
	  setTempLength(wave->getSampleLength()*wave->getChannels()*2);
	break;

      case WAVE_FORMAT_VORBIS:
 	format_in=5;
 	evt->
 	  setTempLength(wave->getSampleLength()*wave->getChannels()*2);
 	break;
 
  }
  wave->closeWave();
  delete wave;

  QString cmd;
  float normal=0.0;
  RDLibraryConf *rdlibrary=new RDLibraryConf(catch_config->stationName(),0);
  if(evt->format()<99) {
    if(evt->normalizeLevel()<=0) {
      normal=pow(10.0,(double)(evt->normalizeLevel()/2000.0));
      cmd=QString().
        sprintf("rd_export_file %6.4f %d %d %s %d %d %d %d %d %s %s.dat %s.%s %d",
	      normal,
	      format_in,
	      samplerate,
	      (const char *)local_filename,
	      evt->format(),
	      evt->channels(),
	      evt->sampleRate(),
	      evt->bitrate()/1000,
	      evt->quality(),
	      (const char *)RDEscapeString(evt->tempName().utf8()),
	      (const char *)(*tempname),
	      (const char *)(*tempname),
	      RDConfiguration()->audioExtension().ascii(),
	      rdlibrary->srcConverter());
    }
    else {
      cmd=QString().
        sprintf("rd_export_file 0 %d %d %s %d %d %d %d %d %s %s.dat %s.%s %d",
	      format_in,
	      samplerate,
	      (const char *)local_filename,
	      evt->format(),
	      evt->channels(),
	      evt->sampleRate(),
	      evt->bitrate()/1000,
	      evt->quality(),
	      (const char *)RDEscapeString(evt->tempName().utf8()),
	      (const char *)(*tempname),
	      (const char *)(*tempname),
	      RDConfiguration()->audioExtension().ascii(),
	      rdlibrary->srcConverter());
    }
  }
  else { 
    cmd=QString().
      sprintf("cp %s %s",(const char *)local_filename,
	      (const char *)RDEscapeString(evt->tempName().utf8()));
  }
  delete rdlibrary;
  switch(evt->format()) {  // Custom format?
    case RDSettings::Pcm16:
    case RDSettings::MpegL1:
    case RDSettings::MpegL2:
    case RDSettings::MpegL3:
    case RDSettings::Flac:
    case RDSettings::OggVorbis:
    case RDSettings::Copy:
      break;

    default:
      settings.setFormat((RDSettings::Format)evt->format());
      settings.setChannels(evt->channels());
      settings.setSampleRate(evt->sampleRate());
      settings.setBitRate(evt->bitrate());
      custom_cmd=settings.resolvedCustomCommandLine(
	RDEscapeString(evt->tempName()));
      if(custom_cmd.isEmpty()) {
	return QString();
      }
      cmd+=" \""+custom_cmd+"\"";
      break;
  }

  // LogLine(RDConfig::LogNotice,QString().sprintf("CMD: %s",(const char *)cmd));
  return cmd;
}


bool MainObject::Import(CatchEvent *evt)
{
  QString temp_importname;

  QString import_cmd=GetImportCmd(evt,&temp_importname);
  if(import_cmd.isEmpty()) {
    catch_connect->setExitCode(evt->id(),RDRecording::ServerError);
    qApp->processEvents();
    LogLine(RDConfig::LogWarning,QString().
	    sprintf("import of %s failed to start, id=%d",
		    (const char *)evt->tempName(),
		    evt->id()));
    return false;
  }
  LogLine(RDConfig::LogInfo,QString().
	  sprintf("started import of %s to cut %s, id=%d, cmd=\"%s\"",
		  (const char *)evt->tempName(),
		  (const char *)evt->cutName(),
		  evt->id(),
		  (const char *)import_cmd));
  if(system(import_cmd.utf8())!=0) {
    catch_connect->setExitCode(evt->id(),RDRecording::ServerError);
    qApp->processEvents();
    LogLine(RDConfig::LogWarning,QString().
	    sprintf("import of %s returned an error, id=%d",
		    (const char *)evt->tempName(),
		    evt->id()));
    unlink(QString().sprintf("%s.%s",(const char *)temp_importname,
			     RDConfiguration()->audioExtension().ascii()));
    unlink(temp_importname+".dat");
    exit(256);
  }
  unlink(QString().sprintf("%s.%s",(const char *)temp_importname,
			   RDConfiguration()->audioExtension().ascii()));
  unlink(temp_importname+".dat");
  chown(RDCut::pathName(evt->cutName()),catch_config->uid(),
	catch_config->gid());
  chmod(RDCut::pathName(evt->cutName()),
	S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  CheckInRecording(evt->cutName(),evt->trimThreshold(),evt->normalizeLevel());
//  if(evt->deleteTempFile()) {
    unlink(evt->tempName());
    LogLine(RDConfig::LogDebug,
	    QString().sprintf("deleted file %s",(const char *)evt->tempName()));
//  }
  catch_connect->setExitCode(evt->id(),RDRecording::Ok);
  qApp->processEvents();
  LogLine(RDConfig::LogInfo,QString().
	  sprintf("finished import of %s to cut %s, id=%d",
		  (const char *)evt->tempName(),
		  (const char *)evt->cutName(),
		  evt->id()));
  return true;
}


QString MainObject::GetImportCmd(CatchEvent *evt,QString *tempname)
{
  int format_in=0;
  int format_out=0;
  bool open_failed=false;
  
        QString sql=QString().sprintf("select DEFAULT_FORMAT,DEFAULT_CHANNELS,\
                               DEFAULT_SAMPRATE,DEFAULT_LAYER,DEFAULT_BITRATE,\
                               RIPPER_LEVEL\
                               from RDLIBRARY where STATION=\"%s\"",
			      (const char *)catch_config->stationName());
	RDSqlQuery *q=new RDSqlQuery(sql);
	if(q->first())
	{
	  catch_default_format=q->value(0).toInt();
	  catch_default_channels=q->value(1).toInt();
	  catch_default_samplerate=q->value(2).toInt();
	  catch_default_layer=q->value(3).toInt();
	  catch_default_bitrate=q->value(4).toInt();
	  catch_ripper_level=q->value(5).toInt();

          evt->setFormat((RDCae::AudioCoding)q->value(0).toInt());
          evt->setSampleRate(q->value(2).toInt());
          evt->setChannels(q->value(1).toInt());
          evt->setBitrate(q->value(4).toInt()*q->value(1).toInt());
          evt->setNormalizeLevel(q->value(5).toInt());
	}
	else {
	  LogLine(RDConfig::LogWarning,
		  "unable to load import audio configuration");
	  delete q;
	  return "";
	}
	delete q;

  //
  // Calculate Temporary Filenames
  //
  *tempname=QString().sprintf("%s/rdcatchd-import-%d",
			      (const char *)catch_temp_dir,
			      evt->id());

  QFile file(evt->tempName());
  if(!file.exists()) {
    return QString();
  }
  RDWaveFile *wave=new RDWaveFile(evt->tempName());
  if(!wave->openWave()) {
    if(evt->format()==3 || evt->format()==5) {
      open_failed=true;
    }
    else {
      delete wave;
      LogLine(RDConfig::LogWarning,QString().
	    sprintf("unable to open temporary file %s for importing, id=%d",
		    (const char *)evt->tempName(),
		    evt->id()));
      return QString();
    }
  }
  if(wave->type()==RDWaveFile::Unknown) {
    if(evt->format()==3 || evt->format()==5 ) {
      open_failed=true;
    }
    else {
      wave->closeWave();
      delete wave;
      LogLine(RDConfig::LogWarning,QString().
	    sprintf("unrecognized format in temporary file %s, id=%d",
		    (const char *)evt->tempName(),
		    evt->id()));
      return QString();
    }
  }
  int samplerate=wave->getSamplesPerSec();
  switch(wave->getFormatTag()) {
      case WAVE_FORMAT_PCM:
	format_in=0;
	break;

      case WAVE_FORMAT_MPEG:
	format_in=wave->getHeadLayer();
	break;

     case WAVE_FORMAT_FLAC:
       format_in=4;
       break;

     case WAVE_FORMAT_VORBIS:
       format_in=5;
       break;

  }
  delete wave;

  switch(evt->format()) {
      case 0:  // PCM16
	evt->
	  setFinalLength((int)(((double)evt->tempLength()/2.0)*
			    (double)catch_default_channels*
			    (double)catch_default_samplerate/44100.0));
	break;
      case 1:  // MPEG-1 Layer 2
      case 2:  // MPEG-1 Layer 3
      case 3:
      case 4:
      case 5:
	evt->
	  setFinalLength((int)((double)evt->tempLength()*
			       (double)catch_default_channels*
			       (double)catch_default_bitrate/1411200.0));
	break;
  }
  file.close();
  switch(evt->format()) {
      case 0:  // PCM16
	format_out=0;
	break;

      case 2:  // MPEG L2
	format_out=1;
	break;

      case 3:  // MPEG L3
	format_out=3;
	break;

      case 4:  
	format_out=4;
	break;

      case 5:  
	format_out=5;
	break;

      default:
	LogLine(RDConfig::LogWarning,QString().
		sprintf("unknown output format %d, id=%d",
			evt->format(),
			evt->id()));
	break;
  }

  QString cmd;
  float normal=0.0;
  RDLibraryConf *rdlibrary=new RDLibraryConf(catch_config->stationName(),0);
  if(evt->normalizeLevel()!=0) {
    normal=pow(10.0,(double)(evt->normalizeLevel()/2000.0));
    if(format_out!=3 && format_out!=5) {
      cmd=QString().
        sprintf("rd_import_file %6.4f %d %d %s %d %d %d %d %s %s.dat %s.%s %d",
	      normal,
	      format_in,
	      samplerate,
	      (const char *)RDEscapeString(evt->tempName()),  
	      format_out,
	      evt->channels(),
	      evt->sampleRate(),
	      evt->bitrate()/1000,
	      RDCut::pathName(evt->cutName()).ascii(),
	      (const char *)*tempname,
	      (const char *)*tempname,
	      RDConfiguration()->audioExtension().ascii(),
	      rdlibrary->srcConverter());
    }
    else {
      if((format_in!=3 && format_in!=5) || open_failed || samplerate!=evt->sampleRate()) {
        cmd=QString().
          sprintf("rd_import_encode %s %s %d %d %d %d %f %s %s",
	        (const char *)RDEscapeString(evt->tempName().utf8()),  
	        RDCut::pathName(evt->cutName()).ascii(),  
	        format_out,
	        evt->sampleRate(),
	        evt->channels(),
	        evt->bitrate()/1000,
	        normal,
	        catch_config->audioOwner().ascii(),
	        catch_config->audioGroup().ascii());
      }
      else {
        cmd=QString().
          sprintf("rd_import_copy %s %s %f %s %s",
	        (const char *)RDEscapeString(evt->tempName().utf8()),  
	        RDCut::pathName(evt->cutName()).ascii(),  
	        normal,
	        catch_config->audioOwner().ascii(),
	        catch_config->audioGroup().ascii());
      }  	      
    }	      
  }
  else {
    if(format_out!=3 && format_out!=5) {
    cmd=QString().
      sprintf("rd_import_file 0 %d %d %s %d %d %d %d %s %s.dat %s.%s %d",
	      format_in,
	      samplerate,
	      (const char *)RDEscapeString(evt->tempName()),
	      format_out,
	      evt->channels(),
	      evt->sampleRate(),
	      evt->bitrate()/1000,
	      RDCut::pathName(evt->cutName()).ascii(),
	      (const char *)*tempname,
	      (const char *)*tempname,
	      RDConfiguration()->audioExtension().ascii(),
	      rdlibrary->srcConverter());
    }
  }
  //LogLine(RDConfig::LogErr,QString().sprintf("CMD: %s",(const char *)cmd));
  delete rdlibrary;
  return cmd;
}


