// rdimport.h
//
// A Batch Importer for Rivendell.
//
//   (C) Copyright 2002-2009 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdimport.h,v 1.11.2.3 2009/09/04 01:29:50 cvs Exp $
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


#ifndef RDIMPORT_H
#define RDIMPORT_H

#include <list>

#include <qobject.h>
#include <qsqldatabase.h>
#include <qfileinfo.h>
#include <qdatetime.h>

#include <rdwavedata.h>
#include <rdwavefile.h>
#include <rdconfig.h>
#include <rdcmd_switch.cpp>
#include <rdgroup.h>
#include <rdcart.h>
#include <rdcut.h>

#define RDIMPORT_TEMP_BASENAME "rdimp"
#define RDIMPORT_STDIN_BUFFER_LENGTH 1024
#define RDIMPORT_DROPBOX_SCAN_INTERVAL 5
#define RDIMPORT_DROPBOX_PASSES 3
#define RDIMPORT_USAGE "[options] <group> <filespec> [<filespec>]*\n\nImport one or more files into the specified group in the library.  By\ndefault, a new cart will be created for each file imported.  A <filespec> of \n'-' will cause RDImport to read the list of filespecs from standard input.\n\nThe following options are available:\n\n --verbose\n     Print progress messages during processing.\n\n --log-mode\n     Prepend date/time information to each line of printed status (implies\n     the '--verbose' option).\n\n --normalization-level=<level>\n     Specify the level to use for normalizing the audio, in dBFS.\n     Specifying '0' will turn off normalization.\n\n --autotrim-level=<level>\n     Specify the threshold level to use for autotrimming the audio, in\n     dBFS.  Specifying '0' will turn off autotrimming.\n\n --single-cart\n     If more than one file is imported, place them within multiple cuts\n     within a single cart, rather than creating separate carts for each\n     file.\n\n --segue-level=<level>\n     Specify the threshold level to use for setting the segue markers,\n     in dBFS.\n\n --segue-length=<length>\n     Length of the added segue in msecs.\n\n --to-cart=<cartnum>\n     Specify the cart to import the audio into, rather than using the next\n     available cart number for the group.  If the cart does not exist, it will\n     be created.  Each file will be imported into a separate new cut within the\n     cart.  Use of this option implies the '--single-cart' option as well,\n     and is mutually exclusive with the '--use-cartchunk-cutid' option.\n\n --use-cartchunk-cutid\n     Import the audio into the cart specified by the CartChunk CutID parameter\n     associated with the file.  If the cart does not exist, it will be\n     created.  Use of this option is mutually exclusive with the '--to-cart'\n     option.\n\n --title-from-cartchunk-cutid\n     Set the cart title from CartChunk CutID.\n\n --delete-source\n     Delete each source file after successful import.  Use with caution!\n\n --delete-cuts\n     Delete all cuts within the destination cart before importing.  Use\n     with caution!\n\n --drop-box\n     Operate in DropBox mode.  RDImport will run continuously, periodically\n     scanning for the specified files, importing and then deleting them when\n     found.  WARNING:  use of this option in command-line mode also implies\n     the '--delete-source' option!\n\n --metadata-pattern=<pattern>\n     Attempt to read metadata parameters from the source filename, using\n     the pattern <pattern>.  Patterns consist of a sequence of macros and\n     regular characters to indicate boundaries between metadata fields.\n     The available macros are:\n\n          %a - Artist\n          %b - Record Label\n          %c - Client\n          %e - Agency\n          %g - Rivendell Group\n          %l - Album\n          %m - Composer\n          %n - Rivendell Cart Number\n          %p - Publisher\n          %t - Title\n          %u - User Defined\n          %% - A literal '%'\n\n     Detection of either the Rivendell Group [%g] or Rivendell Cart [%n]\n     will cause RDImport to attempt to import the file to the specified Group\n     and/or Cart, overriding whatever values were specified elsewhere on the\n     command line.\n\n     Boundaries between metadata fields are indicated by placing regular\n     characters between macros.  For example, the pattern '%t_%a_%g_%n.',\n     when processing a filename of 'My Song_My Artist_TEMP_123456.mp3',\n     would extract 'My Song' as the title and 'My Artist' as the artist,\n     while importing it into cart 123456 in the TEMP group.\n\n --startdate-offset=<days>\n     If the imported file references a start date, offset the value by\n     <days> days.\n\n --enddate-offset=<days>\n     If the imported file references an end date, offset the value by\n     <days> days.\n\n --fix-broken-formats\n     Attempt to work around malformed audio input data.\n\nNOTES\nIt may be necessary to enclose individual <filespec> clauses in quotes in\norder to protect wildcard characters from expansion by the shell.  A typical\nindicator that this is necessary is the failure of RDImport to process newly\nadded files when running in DropBox mode.\n"
#define RDIMPORT_GLOB_SIZE 10

//
// Global Variables
//
RDConfig *lib_config;


class MainObject : public QObject
{
 public:
  MainObject(QObject *parent=0,const char *name=0);

 private:
  enum Result {Success=0,FileBad=1,NoCart=2};
  void RunDropBox();
  void ProcessFileList(const QString &flist);
  void ProcessFileEntry(const QString &entry);
  MainObject::Result ImportFile(const QString &filename,unsigned *cartnum);
  void VerifyFile(const QString &filename,unsigned *cartnum);
  RDWaveFile *FixFile(const QString &filename,RDWaveData *wavedata);
  bool IsWav(int fd);
  bool FindChunk(int fd,char *name,bool *fix_needed);
  bool FixChunkSizes(const QString &filename);
  bool RunPattern(const QString &pattern,const QString &filename,
		  RDWaveData *wavedata,QString *groupname);
  bool VerifyPattern(const QString &pattern);
  void PrintLogDateTime(FILE *f=stdout);
  void DeleteCuts(unsigned cartnum);
  QDateTime GetCachedTimestamp(const QString &filename);
  void WriteTimestampCache(const QString &filename,const QDateTime &dt);
  RDConfig *import_config;
  RDCmdSwitch *import_cmd;
  unsigned import_file_key;
  RDGroup *import_group;
  bool import_verbose;
  bool import_log_mode;
  bool import_use_cartchunk_cutid;
  bool import_single_cart;
  bool import_title_from_cartchunk_cutid;
  bool import_delete_source;
  bool import_delete_cuts;
  bool import_drop_box;
  bool import_stdin_specified;
  int import_startdate_offset;
  int import_enddate_offset;
  bool import_fix_broken_formats;
  int import_persistent_dropbox_id;
  unsigned import_format;
  unsigned import_samprate;
  unsigned import_bitrate;
  unsigned import_channels;
  int import_src_converter;
  int import_normalization_level;
  int import_autotrim_level;
  int import_segue_level;
  int import_segue_length;
  unsigned import_cart_number;
  QString import_metadata_pattern;
  struct DropboxList {
    QString filename;
    unsigned size;
    unsigned pass;
    bool checked;
    bool failed;
  };
  std::list<DropboxList *> import_dropbox_list;
  QString import_temp_fix_filename;
};


#endif  // RDIMPORT_H
