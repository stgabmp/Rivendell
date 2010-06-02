//   rdwavedata.h
//
//   A Container Class for Audio Meta Data.
//
//   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: rdwavedata.h,v 1.2 2008/07/21 17:58:15 fredg Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
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
//

#ifndef RDWAVEDATA_H
#define RDWAVEDATA_H

#include <qstring.h>
#include <qdatetime.h>

class RDWaveData
{
 public:
  enum EndType {UnknownEnd='N',ColdEnd='C',FadeEnd='F'};
  RDWaveData();
  bool metadataFound() const;
  void setMetadataFound(bool state);
  QString title() const;
  void setTitle(const QString &str);
  QString artist() const;
  void setArtist(const QString &str);
  QString album() const;
  void setAlbum(const QString &str);
  QString label() const;
  void setLabel(const QString &str);
  int releaseYear() const;
  void setReleaseYear(int year);
  QString client() const;
  void setClient(const QString &str);
  QString agency() const;
  void setAgency(const QString &str);
  QString composer() const;
  void setComposer(const QString &str);
  QString publisher() const;
  void setPublisher(const QString &str);
  QString licensingOrganization() const;
  void setLicensingOrganization(const QString &str);
  QString copyrightNotice() const;
  void setCopyrightNotice(const QString &str);
  QString cutId() const;
  void setCutId(const QString &str);
  QString classification() const;
  void setClassification(const QString &str);
  QString category() const;
  void setCategory(const QString &str);
  QString url() const;
  void setUrl(const QString &str);
  QString tagText() const;
  void setTagText(const QString &str);
  QString description() const;
  void setDescription(const QString &str);
  QString originator() const;
  void setOriginator(const QString &str);
  QString originatorReference() const;
  void setOriginatorReference(const QString &str);
  QString codingHistory() const;
  void setCodingHistory(const QString &str);
  QString userDefined() const;
  void setUserDefined(const QString &str);
  QString isrc() const;
  void setIsrc(const QString &str);
  QString mcn() const;
  void setMcn(const QString &str);
  QString outCue() const;
  void setOutCue(const QString &str);
  RDWaveData::EndType endType() const;
  void setEndType(RDWaveData::EndType type);
  int introStartPos() const;
  void setIntroStartPos(int msec);
  int introEndPos() const;
  void setIntroEndPos(int msec);
  int segueStartPos() const;
  void setSegueStartPos(int msec);
  int segueEndPos() const;
  void setSegueEndPos(int msec);
  int startPos() const;
  void setStartPos(int msec);
  int endPos() const;
  void setEndPos(int msec);
  int beatsPerMinute() const;
  void setBeatsPerMinute(int bpm);
  QString tmciSongId() const;
  void setTmciSongId(const QString &str);
  QDate originationDate() const;
  void setOriginationDate(const QDate &date);
  QTime originationTime() const;
  void setOriginationTime(const QTime &time);
  QDate startDate() const;
  void setStartDate(const QDate &date);
  QTime startTime() const;
  void setStartTime(const QTime &time);
  QDate endDate() const;
  void setEndDate(const QDate &date);
  QTime endTime() const;
  void setEndTime(const QTime &time);
  void clear();

 private:
  bool data_metadata_found;
  QString data_title;
  QString data_artist;
  QString data_album;
  QString data_label;
  int data_release_year;
  QString data_client;
  QString data_agency;
  QString data_composer;
  QString data_publisher;
  QString data_licensing_organization;
  QString data_copyright_notice;
  QString data_user_defined;
  QString data_cut_id;
  QString data_classification;
  QString data_category;
  QString data_url;
  QString data_tag_text;
  QString data_description;
  QString data_originator;
  QString data_originator_reference;
  QString data_coding_history;
  QString data_isrc;
  QString data_mcn;
  QString data_out_cue;
  RDWaveData::EndType data_end_type;
  int data_intro_start_pos;
  int data_intro_end_pos;
  int data_segue_start_pos;
  int data_segue_end_pos;
  int data_start_pos;
  int data_end_pos;
/*
  int data_intro_length;
  int data_segue_length;
  int data_full_length;
*/
  int data_beats_per_minute;
  QString data_tmci_song_id;
  QDate data_origination_date;
  QTime data_origination_time;
  QDate data_start_date;
  QTime data_start_time;
  QDate data_end_date;
  QTime data_end_time;
};


#endif  // RDWAVEDATA_H
