// rdmatrix.h
//
// Abstract a Rivendell Switcher Matrix
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdmatrix.h,v 1.24.2.2.2.2 2010/05/08 23:01:57 cvs Exp $
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

#include <qsqldatabase.h>
#include <qdatetime.h>
#include <qhostaddress.h>

#ifndef RDMATRIX_H
#define RDMATRIX_H


class RDMatrix
{
 public:
  enum Role {Primary=0,Backup=2};
  enum PortType {TtyPort=0,TcpPort=1,NoPort=2};
  enum Type {LocalGpio=0,GenericGpo=1,GenericSerial=2,Sas32000=3,Sas64000=4,
	     Unity4000=5,BtSs82=6,Bt10x1=7,Sas64000Gpi=8,Bt16x1=9,Bt8x2=10,
	     BtAcs82=11,SasUsi=12,Bt16x2=13,BtSs124=14,LocalAudioAdapter=15,
	     LogitekVguest=16,BtSs164=17,StarGuideIII=18,BtSs42=19,
	     LiveWire=20,Quartz1=21,BtSs44=22,BtSrc8III=23,BtSrc16=24,
	     None=255};
  enum Endpoint {Input=0,Output=1};
  enum Mode {Stereo=0,Left=1,Right=2};
  enum VguestAttribute {VguestEngine=0,VguestDevice=1,VguestSurface=2,
			VguestRelay=3,VguestBuss=4};
  enum VguestType {VguestTypeRelay=0,VguestTypeDisplay=2};
  enum GpioType {GpioInput=0,GpioOutput=1};
  RDMatrix(const QString &station,int matrix);
  QString station() const;
  int matrix() const;
  bool exists() const;
  RDMatrix::Type type() const;
  void setType(RDMatrix::Type type) const;
  int layer() const;
  void setLayer(int layer);
  QString typeString() const;
  QString name() const;
  void setName(const QString &name) const;
  PortType portType(RDMatrix::Role role) const;
  void setPortType(RDMatrix::Role role,PortType type) const;
  int card() const;
  void setCard(int card) const;
  QHostAddress ipAddress(RDMatrix::Role role) const;
  void setIpAddress(RDMatrix::Role role,QHostAddress addr) const;
  int ipPort(RDMatrix::Role role) const;
  void setIpPort(RDMatrix::Role role,int port) const;
  QString username(RDMatrix::Role role) const;
  void setUsername(RDMatrix::Role role,const QString &name) const;
  QString password(RDMatrix::Role role) const;
  void setPassword(RDMatrix::Role role,const QString &passwd) const;
  unsigned startCart(RDMatrix::Role role) const;
  void setStartCart(RDMatrix::Role role,unsigned cartnum) const;
  unsigned stopCart(RDMatrix::Role role) const;
  void setStopCart(RDMatrix::Role role,unsigned cartnum) const;
  int port(RDMatrix::Role role) const;
  void setPort(RDMatrix::Role role,int port) const;
  int inputs() const;
  void setInputs(int inputs) const;
  QString inputName(int input) const;
  RDMatrix::Mode inputMode(int input) const;
  int outputs() const;
  void setOutputs(int outputs) const;
  QString outputName(int output) const;
  int gpis() const;
  void setGpis(int gpis) const;
  int gpos() const;
  void setGpos(int gpos) const;
  QString gpioDevice() const;
  void setGpioDevice(const QString &gpos) const;
  int faders() const;
  void setFaders(int quan) const;
  int displays() const;
  void setDisplays(int quan) const;

 private:
  QString GetEndpointName(int pointnum,const QString &table) const;
  QVariant GetRow(const QString &param) const;
  void SetRow(const QString &param,const QString &value) const;
  void SetRow(const QString &param,int value) const;
  void SetRow(const QString &param,unsigned value) const;
  QString mx_station;
  int mx_number;
};


#endif 
