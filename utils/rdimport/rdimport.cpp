// rdimport.cpp
//
// A Batch Importer for Rivendell.
//
//   (C) Copyright 2002-2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdimport.cpp,v 1.23.2.6 2009/11/24 21:25:58 cvs Exp $
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

#include <limits.h>
#include <glob.h>
#include <signal.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <qapplication.h>
#include <qdir.h>

#include <rd.h>
#include <rdcut.h>
#include <rdconf.h>
#include <rdimport.h>
#include <rdlibrary_conf.h>
#include <rdescape_string.h>
#include <rddbheartbeat.h>


volatile bool import_run=true;

void SigHandler(int signo)
{
  switch(signo) {
      case SIGTERM:
      case SIGINT:
      case SIGHUP:
	import_run=false;
	break;
  }
}


MainObject::MainObject(QObject *parent,const char *name)
  :QObject(parent,name)
{
  //
  // Initialize Data Structures
  //
  bool ok=false;
  int n=0;
  import_file_key=0;
  import_group=NULL;
  import_verbose=false;
  import_log_mode=false;
  import_single_cart=false;
  import_use_cartchunk_cutid=false;
  import_cart_number=0;
  import_title_from_cartchunk_cutid=false;
  import_delete_source=false;
  import_delete_cuts=false;
  import_drop_box=false;
  import_stdin_specified=false;
  import_startdate_offset=0;
  import_enddate_offset=0;
  import_fix_broken_formats=false;
  import_persistent_dropbox_id=-1;

  //
  // Read Command Options
  //
  import_cmd=
    new RDCmdSwitch(qApp->argc(),qApp->argv(),"rdimport",RDIMPORT_USAGE);
  if(import_cmd->keys()<2) {
    fprintf(stderr,"\n");
    fprintf(stderr,"%s",RDIMPORT_USAGE);
    fprintf(stderr,"\n");
    delete import_cmd;
    exit(256);
  }
  for(unsigned i=0;i<import_cmd->keys()-2;i++) {
    if(import_cmd->key(i)=="--verbose") {
      import_verbose=true;
    }
    if(import_cmd->key(i)=="--log-mode") {
      import_verbose=true;
      import_log_mode=true;
    }
    if(import_cmd->key(i)=="--to-cart") {
      import_cart_number=import_cmd->value(i).toUInt(&ok);
      if((!ok)||(import_cart_number<1)||(import_cart_number>999999)) {
	fprintf(stderr,"rdimport: invalid cart number\n");
	delete import_cmd;
	exit(256);
      }
      if(import_use_cartchunk_cutid) {
	fprintf(stderr,"rdimport: '--to-cart' and '--use-cartchunk-cutid' are mutually exclusive\n");
	delete import_cmd;
	exit(256);
      }
      import_single_cart=true;
    }
    if(import_cmd->key(i)=="--use-cartchunk-cutid") {
      if(import_cart_number!=0) {
	fprintf(stderr,"rdimport: '--to-cart' and '--use-cartchunk-cutid' are mutually exclusive\n");
	delete import_cmd;
	exit(256);
      }
      import_use_cartchunk_cutid=true;
    }
    if(import_cmd->key(i)=="--title-from-cartchunk-cutid") {
      import_title_from_cartchunk_cutid=true;
    }
    if(import_cmd->key(i)=="--delete-source") {
      import_delete_source=true;
    }
    if(import_cmd->key(i)=="--delete-cuts") {
      import_delete_cuts=true;
    }
    if(import_cmd->key(i)=="--startdate-offset") {
      import_startdate_offset=import_cmd->value(i).toInt(&ok);
      if(!ok) {
	fprintf(stderr,"rdimport: invalid startdate-offset\n");
	delete import_cmd;
	exit(256);
      }
    }
    if(import_cmd->key(i)=="--enddate-offset") {
      import_enddate_offset=import_cmd->value(i).toInt(&ok);
      if(!ok) {
	fprintf(stderr,"rdimport: invalid enddate-offset\n");
	delete import_cmd;
	exit(256);
      }
    }
    if(import_cmd->key(i)=="--drop-box") {
      import_drop_box=true;
      if(import_persistent_dropbox_id<0) {
	import_delete_source=true;
      }
    }
    if(import_cmd->key(i)=="--metadata-pattern") {
      import_metadata_pattern=import_cmd->value(i);
      if(!VerifyPattern(import_metadata_pattern)) {
	fprintf(stderr,"rdimport: invalid metadata pattern\n");
	delete import_cmd;
	exit(256);
      }
    }
    if(import_cmd->key(i)=="--fix-broken-formats") {
      import_fix_broken_formats=true;
    }
    if(import_cmd->key(i)=="--persistent-dropbox-id") {
      import_persistent_dropbox_id=import_cmd->value(i).toInt(&ok);
      if(!ok) {
	fprintf(stderr,"rdimport: invalid persistent dropbox id\n");
	delete import_cmd;
	exit(256);
      }
    }
  }

  //
  // Read Configuration
  //
  import_config=new RDConfig();
  import_config->load();

  //
  // Open Database
  //
  QSqlDatabase *db=QSqlDatabase::addDatabase(import_config->mysqlDriver());
  if(!db) {
    fprintf(stderr,"rdimport: unable to initialize connection to database\n");
    delete import_cmd;
    exit(256);
  }
  db->setDatabaseName(import_config->mysqlDbname());
  db->setUserName(import_config->mysqlUsername());
  db->setPassword(import_config->mysqlPassword());
  db->setHostName(import_config->mysqlHostname());
  if(!db->open()) {
    fprintf(stderr,"rdimport: unable to connect to database\n");
    db->removeDatabase(import_config->mysqlDbname());
    exit(256);
  }
  new RDDbHeartbeat(import_config->mysqlHeartbeatInterval(),this);

  //
  // Verify Group
  //
  for(unsigned i=0;i<import_cmd->keys();i++) {
    if(import_cmd->key(i).left(2)!="--") {
      import_group=new RDGroup(import_cmd->key(i));
      if(!import_group->exists()) {
	fprintf(stderr,"rdimport: invalid group specified\n");
	delete import_group;
	delete import_cmd;
	exit(256);
      }
      import_file_key=i+1;
      i=import_cmd->keys();
    }
  }
  if(import_group==NULL) {
    fprintf(stderr,"rdimport: invalid group specified\n");
    delete import_cmd;
    exit(256);
  }
  if(import_cart_number>0) {
    if(!import_group->cartNumberValid(import_cart_number)) {
      fprintf(stderr,"rdimport: invalid cart number for group\n");
      delete import_group;
      delete import_cmd;
      exit(256);
    }
  }

  //
  // Get Audio Parameters
  //
  RDLibraryConf *library_conf=new RDLibraryConf(import_config->stationName(),0);
  import_format=library_conf->defaultFormat();
  import_samprate=library_conf->defaultSampleRate();
  import_bitrate=library_conf->defaultBitrate();
  import_channels=library_conf->defaultChannels();
  import_normalization_level=library_conf->ripperLevel();
  import_autotrim_level=library_conf->trimThreshold();
  import_src_converter=library_conf->srcConverter();
  delete library_conf;
  import_segue_level=0;
  import_segue_length=0;


  for(unsigned i=0;i<import_cmd->keys();i++) {
    if(import_cmd->key(i)=="--normalization-level") {
      n=import_cmd->value(i).toInt(&ok);
      if(ok&&(n<=0)) {
	import_normalization_level=100*n;
      }
      else {
	fprintf(stderr,"rdimport: invalid normalization level\n");
	delete import_cmd;
	exit(256);
      }
    }
    if(import_cmd->key(i)=="--autotrim-level") {
      n=import_cmd->value(i).toInt(&ok);
      if(ok&&(n<=0)) {
	import_autotrim_level=100*n;
      }
      else {
	fprintf(stderr,"rdimport: invalid autotrim level\n");
	delete import_cmd;
	exit(256);
      }
    }
    if(import_cmd->key(i)=="--segue-level") {
      n=import_cmd->value(i).toInt(&ok);
      if(ok&&(n<=0)) {
	import_segue_level=100*n;
        }
      else {
	fprintf(stderr,"rdimport: invalid segue level\n");
	delete import_cmd;
	exit(256);
        }
      }
    if(import_cmd->key(i)=="--segue-length") {
      n=import_cmd->value(i).toInt(&ok);
      if(ok&&(n>=0)) {
	import_segue_length=n;
      }
      else {
	fprintf(stderr,"rdimport: invalid segue length\n");
	delete import_cmd;
	exit(256);
      }
    }
    if(import_cmd->key(i)=="--single-cart") {
      import_single_cart=true;
    }
  }

  //
  // Print Status Messages
  //
  if(import_verbose) {
    printf("\n");
    if(import_log_mode) {
      PrintLogDateTime(stdout);
      printf("rdimport started\n");
    }
    printf("RDImport v%s\n",VERSION);
    if(import_log_mode) {
      printf(" Log mode is ON\n");
    }
    else {
      printf(" Log mode is OFF\n");
    }
    if(import_normalization_level==0) {
      printf(" Normalization is OFF\n");
    }
    else {
      printf(" Normalization level = %d dB\n",import_normalization_level/100);
    }
    if(import_autotrim_level==0) {
      printf(" AutoTrim is OFF\n");
    }
    else {
      printf(" AutoTrim level = %d dB\n",import_autotrim_level/100);
    }
    if(import_cart_number==0) {
      if(import_use_cartchunk_cutid) {
	printf(" Destination cart is taken from CartChunk CutID\n");
      }
      else {
	printf(" Destination cart is AUTO\n");
      }
    }
    else {
      printf(" Destination cart is %06u\n",import_cart_number);
    }
    if(import_single_cart) {
      printf(" Single cart mode is ON\n");
    }
    else {
      printf(" Single cart mode is OFF\n");
    }
    if(import_title_from_cartchunk_cutid) {
      printf(" Destination cart title is taken from CartChunk CutID\n");
    }
    if(import_delete_source) {
      printf(" Delete source mode is ON\n");
    }
    else {
      printf(" Delete source mode is OFF\n");
    }
    if(import_delete_cuts) {
      printf(" Delete cuts mode is ON\n");
    }
    else {
      printf(" Delete cuts mode is OFF\n");
    }
    if(import_drop_box) {
      printf(" DropBox mode is ON\n");
    }
    else {
      printf(" DropBox mode is OFF\n");
    }
    if(!import_metadata_pattern.isEmpty()) {
      printf(" Using metadata pattern: %s\n",
	     (const char *)import_metadata_pattern);
    }
    printf(" Start Date Offset = %d days\n",import_startdate_offset);
    printf(" End Date Offset = %d days\n",import_enddate_offset);
    if(import_fix_broken_formats) {
      printf(" Broken format workarounds are ENABLED\n");
    }
    else {
      printf(" Broken format workarounds are DISABLED\n");
    }
    if(import_persistent_dropbox_id>=0) {
      printf(" Persistent DropBox ID = %d\n",import_persistent_dropbox_id);
    }
    printf(" Files to process:\n");
    for(unsigned i=import_file_key;i<import_cmd->keys();i++) {
      printf("   \"%s\"\n",(const char *)import_cmd->key(i));
    }
    printf("\n");
    fflush(stdout);
  }

  // 
  // Setup Signal Handling 
  //
  ::signal(SIGTERM,SigHandler);
  ::signal(SIGINT,SigHandler);
  ::signal(SIGHUP,SigHandler);

  //
  // Process Files
  //
  if(import_drop_box) {
    RunDropBox();
  }
  else {
    for(unsigned i=import_file_key;i<import_cmd->keys();i++) {
      ProcessFileList(import_cmd->key(i));
    }
    if(import_stdin_specified) {
      bool quote_mode=false;
      bool escape_mode=false;
      char buffer[PATH_MAX];
      unsigned ptr=0;
      while((ptr<PATH_MAX)&&(read(0,buffer+ptr,1)==1)) {
	if(quote_mode) {
	  if(buffer[ptr]=='\"') {
	    quote_mode=false;
	  }
	  else {
	    ptr++;
	  }
	}
	else {
	  if(escape_mode) {
	    ptr++;
	    escape_mode=false;
	  }
	  else {
	    if(buffer[ptr]=='\"') {
	      quote_mode=true;
	    }
	    else {
	      if(buffer[ptr]=='\\') {
		escape_mode=true;
	      }
	      else {
		if(isspace(buffer[ptr])) {
		  buffer[ptr]=0;
		  ProcessFileList(buffer);
		  ptr=0;
		}
		else {
		  ptr++;
		}
	      }
	    }
	  }
	}
      }
      if(ptr>0) {
	buffer[ptr]=0;
	ProcessFileList(buffer);
      }
    }
  }

  //
  // Clean Up and Exit
  //
  delete import_group;
  delete import_cmd;

  exit(0);
}


void MainObject::RunDropBox()
{
  do {
    //
    // Clear the Checked Flag
    //
    for(std::list<struct DropboxList *>::const_iterator 
	  ci=import_dropbox_list.begin();
	ci!=import_dropbox_list.end();ci++) {
      (*ci)->checked=false;
    }

    //
    // Scan for Eligible Imports
    //
    for(unsigned i=import_file_key;i<import_cmd->keys();i++) {
      ProcessFileList(import_cmd->key(i));
    }

    //
    // Take Out the Trash
    //
    for(std::list<struct DropboxList *>::iterator 
	  ci=import_dropbox_list.begin();
	ci!=import_dropbox_list.end();ci++) {
      if(!(*ci)->checked) {
	delete *ci;
	import_dropbox_list.erase(ci);
	ci=import_dropbox_list.end();
      }
    }

    sleep(RDIMPORT_DROPBOX_SCAN_INTERVAL);
  } while(import_run);
  if(import_log_mode) {
    PrintLogDateTime();
    printf("rdimport stopped\n");
    fflush(stdout);
  }
}


void MainObject::ProcessFileList(const QString &flist)
{
  QString entry;

  for(unsigned i=0;i<flist.length();i++) {
    entry+=flist.at(i);
  }
  ProcessFileEntry(entry);
}


void MainObject::ProcessFileEntry(const QString &entry)
{
  glob_t globbuf;
  int gflags=GLOB_MARK;

  if(entry=="-") {
    import_stdin_specified=true;
    return;
  }
  globbuf.gl_offs=RDIMPORT_GLOB_SIZE;
  while((globbuf.gl_pathc==RDIMPORT_GLOB_SIZE)||(gflags==GLOB_MARK)) {
    glob(RDEscapeString(entry),gflags,NULL,&globbuf);
    if((globbuf.gl_pathc==0)&&(gflags==GLOB_MARK)&&(!import_drop_box)) {
      PrintLogDateTime(stderr);
      fprintf(stderr," Unable to open \"%s\", skipping...\n",
	      (const char *)entry);
      fflush(stderr);
      globfree(&globbuf);
    }
    for(size_t i=0;i<globbuf.gl_pathc;i++) {
      if(globbuf.gl_pathv[i][strlen(globbuf.gl_pathv[i])-1]!='/') {
	if(!import_single_cart) {
	  import_cart_number=0;
	}
	if(import_drop_box) {
	  VerifyFile(QString::fromUtf8(globbuf.gl_pathv[i]),&import_cart_number);
	}
	else {
	  ImportFile(QString::fromUtf8(globbuf.gl_pathv[i]),&import_cart_number);
	}
      }
    }
    gflags=GLOB_MARK|GLOB_APPEND;
  }
  globfree(&globbuf);
}


MainObject::Result MainObject::ImportFile(const QString &filename,
					  unsigned *cartnum)
{
  bool cart_created=false;
  unsigned length_deviation=0;
  QString effective_filename;
  bool found_cart=false;
  QDateTime dt;
  bool ok=false;
  RDGroup *effective_group=new RDGroup(import_group->name());
  RDWaveData *wavedata=new RDWaveData();
  RDWaveFile *wavefile=new RDWaveFile(filename);
  if(wavefile->openWave(wavedata)) {
    effective_filename=filename;
  }
  else {
    if(import_fix_broken_formats) {
      if(import_verbose) {
	PrintLogDateTime();
	printf(" File \"%s\" appears to be malformed, trying workaround ... ",
	       (const char *)RDGetBasePart(filename).utf8());
	fflush(stdout);
      }
      delete wavefile;
      if((wavefile=FixFile(filename,wavedata))==NULL) {
	if(import_verbose) {
	  printf("failed.\n");
	}
	PrintLogDateTime(stderr);
	fprintf(stderr,
		" File \"%s\" is not readable or not a recognized format, skipping...\n",
		(const char *)RDGetBasePart(filename).utf8());
	fflush(stderr);
	delete wavefile;
	delete wavedata;
	delete effective_group;
	if(!import_run) {
	  exit(0);
	}
	if(!import_temp_fix_filename.isEmpty()) {
//	  printf("Fixed Name: %s\n",(const char *)import_temp_fix_filename);
	  QFile::remove(import_temp_fix_filename);
	  import_temp_fix_filename="";
	}
	return MainObject::FileBad;
      }
      if(import_verbose) {
	printf("success.\n");
	fflush(stdout);
      }
      effective_filename=import_temp_fix_filename;
    }
    else {
      PrintLogDateTime(stderr);
      fprintf(stderr,
      " File \"%s\" is not readable or not a recognized format, skipping...\n",
      (const char *)RDGetBasePart(filename).utf8());
      fflush(stderr);
      delete wavefile;
      delete wavedata;
      delete effective_group;
      if(!import_run) {
	exit(0);
      }
      if(!import_temp_fix_filename.isEmpty()) {
	QFile::remove(import_temp_fix_filename);
	import_temp_fix_filename="";
      }
      return MainObject::FileBad;
    }
  }

  if(!import_metadata_pattern.isEmpty()) {
    QString groupname=effective_group->name();
    found_cart=RunPattern(import_metadata_pattern,RDGetBasePart(filename),
			  wavedata,&groupname);
    if(groupname!=effective_group->name()) {
      delete effective_group;
      effective_group=new RDGroup(groupname);
      if(!effective_group->exists()) {
	PrintLogDateTime(stderr);
	fprintf(stderr," Specified group \"%s\" from file \"%s\" does not exist, using default group...\n",
		(const char *)groupname,(const char *)filename.utf8());
	fflush(stderr);
	delete effective_group;
	effective_group=new RDGroup(import_group->name());
      }
    }
  }

  if(import_use_cartchunk_cutid||found_cart) {
    *cartnum=0;
    sscanf(wavedata->cutId(),"%u",cartnum);
    if((*cartnum==0)||(*cartnum>999999)||
       (effective_group->enforceCartRange()&&
	(!effective_group->cartNumberValid(*cartnum)))) {
      PrintLogDateTime(stderr);
      fprintf(stderr,
	      " File \"%s\" has an invalid or out of range Cart Number, skipping...\n",
	      (const char *)RDGetBasePart(filename).utf8());
      fflush(stderr);
      wavefile->closeWave();
      delete wavefile;
      delete wavedata;
      delete effective_group;
      return MainObject::FileBad;
    }
  }
  if(*cartnum==0) {
    *cartnum=effective_group->nextFreeCart();
  }
  if(*cartnum==0) {
    PrintLogDateTime(stderr);
    fprintf(stderr,"rdimport: no free carts available in specified group\n");
    fflush(stderr);
    wavefile->closeWave();
    delete wavefile;
    delete wavedata;
    delete effective_group;
    if(import_drop_box) {
      if(!import_run) {
	exit(0);
      }
      if(!import_temp_fix_filename.isEmpty()) {
	QFile::remove(import_temp_fix_filename);
	import_temp_fix_filename="";
      }
      return MainObject::NoCart;
    }
    exit(256);
  }
  if(import_delete_cuts) {
    DeleteCuts(import_cart_number);
  }
  RDCart *cart=new RDCart(*cartnum);
  if(!cart->exists()) {
    cart->create(effective_group->name(),RDCart::Audio);
    cart_created=true;
  }
  int cutnum=
    cart->addCut(import_format,import_samprate,import_bitrate,import_channels);
  RDCut *cut=new RDCut(QString().sprintf("%06u_%03d",*cartnum,cutnum));

  int format_in=0;
  switch(wavefile->getFormatTag()) {
      case WAVE_FORMAT_PCM:
	format_in=0;
	break;

      case WAVE_FORMAT_MPEG:
	format_in=wavefile->getHeadLayer();
	break;
  }
  QString tempname=tempnam(NULL,RDIMPORT_TEMP_BASENAME);
  QString tempwav_name=tempname+".wav";
  QString tempdat_name=tempname+".dat";
  double normal=pow(10.0,(double)import_normalization_level/2000.0);
  QString cmd;
  if(import_normalization_level==0) {
    cmd=QString().
      sprintf("rd_import_file 0 %d %d %s %d %d %d %d %s %s %s %d",
	      format_in,
	      wavefile->getSamplesPerSec(),
	      (const char *)RDEscapeString(effective_filename).utf8(),  
	      import_format,
	      import_channels,
	      import_samprate,
	      import_channels*import_bitrate/1000,
	      RDCut::pathName(cut->cutName()).ascii(), 
	      (const char *)tempdat_name,
	      (const char *)tempwav_name,
	      import_src_converter);
  }
  else {
    cmd=QString().
      sprintf("rd_import_file %6.4f %d %d %s %d %d %d %d %s %s %s %d",
	      normal,
	      format_in,
	      wavefile->getSamplesPerSec(),
	      (const char *)RDEscapeString(effective_filename).utf8(),  
	      import_format,
	      import_channels,
	      import_samprate,
	      import_channels*import_bitrate/1000,
	      RDCut::pathName(cut->cutName()).ascii(), 
	      (const char *)tempdat_name,
	      (const char *)tempwav_name,
	      import_src_converter);
  }
  if(import_verbose) {
    PrintLogDateTime();
    if(wavedata->title().length()==0 || ( (wavedata->title().length()>0) && (wavedata->title()[0] == '\0')) ) {
      printf(" Importing file \"%s\" to cart %06u ... ",
	     (const char *)RDGetBasePart(filename).utf8(),*cartnum);
    }
    else {
      printf(" Importing file \"%s\" [%s] to cart %06u ... ",
	     (const char *)RDGetBasePart(filename).utf8(),
	     (const char *)wavedata->title(),*cartnum);
    }
    fflush(stdout);
  }
  if(system((const char *)cmd.utf8())==0) {
    chown(RDCut::pathName(cut->cutName()),import_config->uid(),
	  import_config->gid());
    chmod(RDCut::pathName(cut->cutName()),
	  S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  }
  else {
    PrintLogDateTime(stderr);
    fprintf(stderr," Unable to import \"%s\", skipping...\n",
	    (const char *)filename.utf8());
    fflush(stderr);
    cart->remove();
    unlink(tempwav_name);
    unlink(tempdat_name);
    delete cut;
    delete cart;
    wavefile->closeWave();
    delete wavefile;
    delete wavedata;
    delete effective_group;
    if(!import_run) {
      exit(0);
    }
    if(!import_temp_fix_filename.isEmpty()) {
      QFile::remove(import_temp_fix_filename);
      import_temp_fix_filename="";
    }
    return MainObject::FileBad;
  }
  cut->reset();
  cut->setOriginName(import_config->stationName());
  if(wavedata->metadataFound()) {
    if(cart_created) {
      cart->setMetadata(wavedata);
    }
    cut->setMetadata(wavedata);
  }
  cut->autoTrim(RDCut::AudioBoth,import_autotrim_level);
  cut->autoSegue(import_segue_level,import_segue_length);
  if((wavedata->title().length()==0)||
     ((wavedata->title().length()>0)&&(wavedata->title()[0] == '\0'))) {
    QString title=effective_group->defaultTitle();
    QString basename=RDGetBasePart(filename);
    int ptr=basename.findRev(".");
    title.replace("%p",RDGetPathPart(filename));
    title.replace("%f",basename.left(ptr));
    title.replace("%e",basename.right(basename.length()-ptr-1));
    cut->setDescription(title.utf8());
    if(cart_created) {
      cart->setTitle(title.utf8());
    }
  }
  if(import_title_from_cartchunk_cutid) {    
    if((wavedata->cutId().length()>0)&&(wavedata->cutId()[0]!='\0')) {
      if(cut->description().isEmpty()) {
	cut->setDescription(wavedata->cutId());
      }
      cart->setTitle(wavedata->cutId());
    }
  }
  cart->updateLength();
  cart->resetRotation();
  cart->calculateAverageLength(&length_deviation);
  cart->setLengthDeviation(length_deviation);
  if(import_startdate_offset!=0) {
    dt=cut->startDatetime(&ok);
    if(ok) {
      cut->setStartDatetime(dt.addDays(import_startdate_offset),true);
    }
  }
  if(import_enddate_offset!=0) {
    dt=cut->endDatetime(&ok);
    if(ok) {
      cut->setEndDatetime(dt.addDays(import_enddate_offset),true);
    }
  }
  if(import_verbose) {
    printf("done.\n");
    fflush(stdout);
  }
  unlink(tempwav_name);
  unlink(tempdat_name);
  delete cut;
  delete cart;
  wavefile->closeWave();
  delete wavefile;
  delete wavedata;
  delete effective_group;

  if(import_delete_source) {
    unlink(filename.utf8());
    if(import_verbose) {
      PrintLogDateTime();
      printf(" Deleted file \"%s\"\n",(const char *)RDGetBasePart(filename).utf8());
      fflush(stdout);
    }
  }
  if(!import_run) {
    exit(0);
  }
  if(!import_temp_fix_filename.isEmpty()) {
    QFile::remove(import_temp_fix_filename);
    import_temp_fix_filename="";
  }

  return MainObject::Success;
}


void MainObject::VerifyFile(const QString &filename,unsigned *cartnum)
{
  bool found=false;
  QDateTime dt;

  for(std::list<struct DropboxList *>::const_iterator 
	ci=import_dropbox_list.begin();
      ci!=import_dropbox_list.end();ci++) {
    if((*ci)->filename==filename) {
      found=true;
      QFileInfo *file=new QFileInfo(filename);
      dt=GetCachedTimestamp(filename);
      if(dt.isNull()||(file->lastModified()>dt)) {
	if((file->size()==(*ci)->size)&&(!(*ci)->failed)) {
	  (*ci)->pass++;
	}
	else {
	  (*ci)->size=file->size();
	  (*ci)->pass=0;
	}
	if((*ci)->failed) {
	  (*ci)->checked=true;
	  if(file->size()!=(*ci)->size) {
	    (*ci)->failed=false;
	    (*ci)->size=file->size();
	    (*ci)->pass=0;
	  }
	}
	if((*ci)->pass>=RDIMPORT_DROPBOX_PASSES) {
	  switch(ImportFile(filename,cartnum)) {
	    case MainObject::Success:
	      WriteTimestampCache(filename,file->lastModified());
	      break;
	      
	    case MainObject::FileBad:
	      (*ci)->failed=true;
	      (*ci)->checked=true;
	      (*ci)->pass=0;
	      WriteTimestampCache(filename,file->lastModified());
	      break;
	      
	    case MainObject::NoCart:
	      (*ci)->pass=0;
	      (*ci)->checked=true;
	      break;
	  }
	}
	else {
	  (*ci)->checked=true;
	}
      }
      delete file;
    }
  }
  if(!found) {
    QFile *file=new QFile(filename);
    import_dropbox_list.push_back(new struct DropboxList());
    import_dropbox_list.back()->filename=filename;
    import_dropbox_list.back()->size=file->size();
    import_dropbox_list.back()->pass=0;
    import_dropbox_list.back()->checked=true;
    import_dropbox_list.back()->failed=false;
    delete file;
  }
}


RDWaveFile *MainObject::FixFile(const QString &filename,RDWaveData *wavedata)
{
  bool fix_needed=false;

  //
  // Determine Fixability
  //
  int fd=open(filename,O_RDONLY);
  if(fd<0) {
    return NULL;
  }
  if(!IsWav(fd)) {
    return NULL;
  }
  if(!FindChunk(fd,"fmt ",&fix_needed)) {
    return NULL;
  }
  if(!FindChunk(fd,"data",&fix_needed)) {
    return NULL;
  }
  if(!fix_needed) {  // This shouldn't ever happen!
    return NULL;
  }
  ::close(fd);

  //
  // Copy File
  //
  import_temp_fix_filename=
    QString(tempnam(RDTempDir(),"rdfix"))+QString(".wav");
  if(import_temp_fix_filename.isNull()) {
    return NULL;
  }
  if(!RDCopy(filename,import_temp_fix_filename)) {
    return NULL;
  }

  //
  // Apply Fix
  //
  if(!FixChunkSizes(import_temp_fix_filename)) {
    return NULL;
  }
  RDWaveFile *wf=new RDWaveFile();
  wf->nameWave(import_temp_fix_filename);
  if(!wf->openWave(wavedata)) {
    delete wf;
    return NULL;
  }
  return wf;
}


bool MainObject::IsWav(int fd)
{
  int i;
  char buffer[5];
  
  //
  // Is this a riff file? 
  //
  lseek(fd,0,SEEK_SET);
  i=read(fd,buffer,4);
  if(i==4) {
    buffer[4]=0;
    if(strcmp("RIFF",buffer)!=0) {
      return false;
    }
  }
  else {
    return false;
  }

  // 
  // Is this a WAVE file? 
  //
  if(lseek(fd,8,SEEK_SET)!=8) {
    return false;
  }
  i=read(fd,buffer,4);
  if(i==4) {
    buffer[4]=0;
    if(strcmp("WAVE",buffer)!=0) {
      return false;
    }
  }
  else {
    return false;
  }
  return true;
}


bool MainObject::FindChunk(int fd,char *chunk_name,bool *fix_needed)
{
  int i;
  char name[5]={0,0,0,0,0};
  unsigned char buffer[4];
  off_t offset;
  unsigned chunk_size;

  offset=lseek(fd,12,SEEK_SET);
  i=read(fd,name,4);
  i=read(fd,buffer,4);
  chunk_size=buffer[0]+(256*buffer[1])+(65536*buffer[2])+(16777216*buffer[3]);
  while(i==4) {
    if(strcasecmp(chunk_name,name)==0) {
      return true;
    }
    lseek(fd,chunk_size,SEEK_CUR);
    i=read(fd,name,4);
    if(name[0]==0) {  // Possible chunk size error
      if(isalpha(name[1])==0) {
	return false;
      }
      name[0]=name[1];
      name[1]=name[2];
      name[2]=name[3];
      if(read(fd,name+3,1)<1) {
	return false;
      }
      *fix_needed=true;
    }
    i=read(fd,buffer,4);
    chunk_size=
      buffer[0]+(256*buffer[1])+(65536*buffer[2])+(16777216*buffer[3]);
  }
  return false;
}


bool MainObject::FixChunkSizes(const QString &filename)
{
  int i;
  char name[5]={0,0,0,0,0};
  unsigned char buffer[4];
  off_t offset;
  unsigned chunk_size;
  int fd;

  //
  // Open File
  //
  if((fd=open(filename,O_RDWR))<0) {
    return false;
  }
  offset=lseek(fd,12,SEEK_SET);
  i=read(fd,name,4);
  i=read(fd,buffer,4);
  off_t last_offset=lseek(fd,0,SEEK_CUR);
  chunk_size=buffer[0]+(256*buffer[1])+(65536*buffer[2])+(16777216*buffer[3]);
  while(i==4) {
    lseek(fd,chunk_size,SEEK_CUR);
    i=read(fd,name,4);
    if(name[0]==0) {  // Possible chunk size error
      if(isalpha(name[1])==0) {
	return false;
      }
      //
      // Fix It Up
      //
      name[0]=name[1];
      name[1]=name[2];
      name[2]=name[3];
      if(read(fd,name+3,1)<1) {
	::close(fd);
	return false;
      }
      off_t pos=lseek(fd,0,SEEK_CUR);
      char buf[4];
      lseek(fd,last_offset,SEEK_SET);
      read(fd,buf,4);
      unsigned size=(0xff&buf[0])+(0xff&(256*buf[1]))+
	(0xff&(65536*buf[2]))+(0xff&(16777216*buf[3]))+1;
      buf[0]=size&0xff;
      buf[1]=(size>>8)&0xff;
      buf[2]=(size>>16)&0xff;
      buf[3]=(size>>24)&0xff;
      lseek(fd,last_offset,SEEK_SET);
      write(fd,buf,4);
      lseek(fd,pos,SEEK_SET);
    }
    last_offset=lseek(fd,0,SEEK_CUR);
    i=read(fd,buffer,4);
    chunk_size=
      buffer[0]+(256*buffer[1])+(65536*buffer[2])+(16777216*buffer[3]);
  }
  ::close(fd);
  return true;
}


bool MainObject::RunPattern(const QString &pattern,const QString &filename,
			    RDWaveData *wavedata,QString *groupname)
{
  bool macro_active=false;
  unsigned ptr=0;
  QChar field;
  QString value;
  QChar delimiter;
  bool found_cartnum=false;

  //
  // Initialize Pattern Parser
  //
  if((pattern.at(0)=='%')&&(pattern.at(1)!='%')) {
    field=pattern.at(1);
    value="";
    delimiter=pattern.at(2);
    ptr=3;
    macro_active=true;
  }
  else {
    delimiter=pattern.at(0);
    ptr=1;
  }

  for(unsigned i=0;i<=filename.length();i++) {
    if(macro_active) {
      if((filename.at(i)==delimiter)||(i==filename.length())) {
	switch(field) {
	  case 'a':
	    wavedata->setArtist(value);
	    wavedata->setMetadataFound(true);
	    break;

	  case 'b':
	    wavedata->setLabel(value);
	    wavedata->setMetadataFound(true);
	    break;

	  case 'c':
	    wavedata->setClient(value);
	    wavedata->setMetadataFound(true);
	    break;

	  case 'e':
	    wavedata->setAgency(value);
	    wavedata->setMetadataFound(true);
	    break;

	  case 'g':
	    *groupname=value;
	    break;

	  case 'l':
	    wavedata->setAlbum(value);
	    wavedata->setMetadataFound(true);
	    break;

	  case 'm':
	    wavedata->setComposer(value);
	    wavedata->setMetadataFound(true);
	    break;

	  case 'n':
	    wavedata->setCutId(value);
	    wavedata->setMetadataFound(true);
	    found_cartnum=true;
	    break;

	  case 'p':
	    wavedata->setPublisher(value);
	    wavedata->setMetadataFound(true);
	    break;

	  case 't':
	    wavedata->setTitle(value);
	    wavedata->setMetadataFound(true);
	    break;

	  case 'u':
	    wavedata->setUserDefined(value);
	    wavedata->setMetadataFound(true);
	    break;
	}
	value="";
	if((ptr>=pattern.length())||(i==filename.length())) {
	  return found_cartnum;
	}
	if((pattern.at(ptr)=='%')&&(pattern.at(ptr+1)!='%')) {
	  field=pattern.at(ptr+1);
	  delimiter=pattern.at(ptr+2);
	  ptr+=3;
	  macro_active=true;
	}
	else {
	  delimiter=pattern.at(ptr);
	  ptr++;
	  macro_active=false;
	}
      }
      else {
	value+=filename.at(i);
      }
    }
    else {
      if((ptr>=pattern.length())||(i==filename.length())) {
	return found_cartnum;
      }
      if(filename.at(i)!=delimiter) {
	return found_cartnum;
      }
      if((pattern.at(ptr)=='%')&&(pattern.at(ptr+1)!='%')) {
	field=pattern.at(ptr+1);
	delimiter=pattern.at(ptr+2);
	ptr+=3;
	macro_active=true;
      }
      else {
	delimiter=pattern.at(ptr);
	ptr++;
	macro_active=false;
      }
    }
  }
  return found_cartnum;
}


bool MainObject::VerifyPattern(const QString &pattern)
{
  bool macro_active=false;
  for(unsigned i=0;i<pattern.length();i++) {
    if(pattern.at(i)==QChar('%')) {
      if(macro_active) {
	return false;
      }
      macro_active=true;
      switch(pattern.at(++i)) {
	case 'a':
	case 'b':
	case 'c':
	case 'e':
	case 'g':
	case 'l':
	case 'm':
	case 'n':
	case 'p':
	case 't':
	case 'u':
	case '%':
	  break;

	default:
	  return false;
      }
    }
    else {
      macro_active=false;
    }
  }
  return true;
}


void MainObject::PrintLogDateTime(FILE *f)
{
  if(import_log_mode) {
    fprintf(f,"%s - %s : ",
	   (const char *)QDate::currentDate().toString("MM-dd-yyyy"),
	   (const char *)QTime::currentTime().toString("hh:mm:ss"));
  }
}


void MainObject::DeleteCuts(unsigned cartnum)
{
  if(import_verbose) {
    PrintLogDateTime();
    printf(" Deleting cuts from cart %06u\n",cartnum);
    fflush(stdout);
  }
  unsigned dev;
  RDCart *cart=new RDCart(cartnum);
  cart->removeAllCuts();
  cart->updateLength();
  cart->resetRotation();
  cart->calculateAverageLength(&dev);
  cart->setLengthDeviation(dev);
  delete cart;
}


QDateTime MainObject::GetCachedTimestamp(const QString &filename)
{
  QString sql;
  RDSqlQuery *q;
  QDateTime dt;
  if(import_persistent_dropbox_id<0) {
    return dt;
  }
  sql=QString().sprintf("select FILE_DATETIME from DROPBOX_PATHS \
                         where (DROPBOX_ID=%d)&&(FILE_PATH=\"%s\")",
			import_persistent_dropbox_id,
			(const char *)RDEscapeString(filename));
  q=new RDSqlQuery(sql);
  if(q->first()) {
    dt=q->value(0).toDateTime();
  }
  delete q;
  return dt;
}


void MainObject::WriteTimestampCache(const QString &filename,
				     const QDateTime &dt)
{
  QString sql;
  RDSqlQuery *q;
  if(import_persistent_dropbox_id<0) {
    return;
  }
  if(GetCachedTimestamp(filename).isNull()) {
    sql=QString().sprintf("insert into DROPBOX_PATHS set \
                           DROPBOX_ID=%d,\
                           FILE_PATH=\"%s\",\
                           FILE_DATETIME=\"%s\"",
			  import_persistent_dropbox_id,
			  (const char *)RDEscapeString(filename),
			  (const char *)dt.toString("yyyy-MM-dd hh:mm:ss"));
  }
  else {
    sql=QString().sprintf("update DROPBOX_PATHS set FILE_DATETIME=\"%s\" \
                           where (DROPBOX_ID=%d)&&(FILE_PATH=\"%s\")",
			  (const char *)dt.toString("yyyy-MM-dd hh:mm:ss"),
			  import_persistent_dropbox_id,
			  (const char *)RDEscapeString(filename));
  }
  q=new RDSqlQuery(sql);
  delete q;
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  new MainObject(NULL,"main");
  return a.exec();
}
