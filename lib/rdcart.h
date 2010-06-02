// rdcart.h
//
// Abstract a Rivendell Cart
//
//   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcart.h,v 1.34.2.2 2009/11/19 17:56:44 cvs Exp $
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

#include <qdatetime.h>

#include <rdwavedata.h>

#include <rdcut.h>
#include <rddb.h>

#ifndef RDCART_H
#define RDCART_H

#define MAX_SERVICES 16

class RDCart
{
 public:
  enum Type {All=0,Audio=1,Macro=2};
  enum PlayOrder {Sequence=0,Random=1};
  enum UsageCode {UsageFeature=0,UsageOpen=1,UsageClose=2,UsageTheme=3,
		  UsageBackground=4,UsagePromo=5,UsageLast=6};
  enum Validity {NeverValid=0,ConditionallyValid=1,AlwaysValid=2,
		 EvergreenValid=3};
  RDCart(unsigned number);
  bool exists() const;
  bool selectCut(QString *cut) const;
  bool selectCut(QString *cut,const QTime &time) const;
  RDCart::Type type() const;
  void setType(RDCart::Type type) const;
  unsigned number() const;
  QString groupName() const;
  void setGroupName(const QString &name) const;
  QString title() const;
  void setTitle(const QString &title) const;
  QString artist() const;
  void setArtist(const QString &name) const;
  QString album() const;
  void setAlbum(const QString &album) const;
  QDate year() const;
  void setYear(const QDate &date) const;
  void setYear() const;
  QString schedCodes() const;
  void setSchedCodes(const QString &sched_codes) const;
  void updateSchedCodes(const QString &add_codes,const QString &remove_codes) const;
  QString label() const;
  void setLabel(const QString &label) const;
  QString client() const;
  void setClient(const QString &client) const;
  QString agency() const;
  void setAgency(const QString &agency) const;
  QString publisher() const;
  void setPublisher(const QString &publisher) const;
  QString composer() const;
  void setComposer(const QString &composer) const;
  QString userDefined() const;
  void setUserDefined(const QString &string) const;
  RDCart::UsageCode usageCode() const;
  void setUsageCode(RDCart::UsageCode code);
  QString notes() const;
  void setNotes(const QString &notes) const;
  unsigned forcedLength() const;
  void setForcedLength(unsigned length) const;
  unsigned lengthDeviation() const;
  void setLengthDeviation(unsigned length) const;
  unsigned calculateAverageLength(unsigned *max_dev=0) const;
  unsigned averageLength() const;
  void setAverageLength(unsigned length) const;
  unsigned averageSegueLength() const;
  void setAverageSegueLength(unsigned length) const;
  unsigned averageHookLength() const;
  void setAverageHookLength(unsigned length) const;
  unsigned cutQuantity() const;
  void setCutQuantity(unsigned quan) const;
  unsigned lastCutPlayed() const;
  void setLastCutPlayed(unsigned cut) const;
  RDCart::PlayOrder playOrder() const;
  void setPlayOrder(RDCart::PlayOrder order) const;
  RDCart::Validity validity() const;
  void setValidity(RDCart::Validity state);
  QDateTime startDateTime() const;
  void setStartDateTime(const QDateTime &time) const;
  void setStartDateTime() const;
  QDateTime endDateTime() const;
  void setEndDateTime(const QDateTime &time) const;
  void setEndDateTime() const;
  bool enforceLength() const;
  void setEnforceLength(bool state) const;
  bool preservePitch() const;
  void setPreservePitch(bool state) const;
  bool asyncronous() const;
  void setAsyncronous(bool state) const;
  QString owner() const;
  void setOwner(const QString &owner) const;
  QString macros() const;
  void setMacros(const QString &cmds) const;
  void getMetadata(RDWaveData *data);
  void setMetadata(RDWaveData *data);
  void updateLength() const;
  void updateLength(bool enforce_length,unsigned length) const;
  void resetRotation() const;
  int addCut(unsigned format,unsigned samprate,
	     unsigned bitrate,unsigned chans) const;
  void removeAllCuts();
  void removeCut(const QString &cutname);
  bool create(const QString &groupname,RDCart::Type type) const;
  void remove() const;
  static bool exists(unsigned cartnum);
  static QString playOrderText(RDCart::PlayOrder order);
  static QString usageText(RDCart::UsageCode usage);
  
 private:
  QString GetNextCut(RDSqlQuery *q) const;
  int GetNextFreeCut() const;
  RDCut::Validity ValidateCut(RDSqlQuery *q,bool enforce_length,
			      unsigned length,bool *time_ok) const;
  QString VerifyTitle(const QString &title) const;
  void SetRow(const QString &param,const QString &value) const;
  void SetRow(const QString &param,unsigned value) const;
  void SetRow(const QString &param,const QDateTime &value) const;
  void SetRow(const QString &param,const QDate &value) const;
  void SetRow(const QString &param) const;
  unsigned cart_number;
};


#endif  // RDCART_H
