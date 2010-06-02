//   rdhpisoundcard.cpp
//
//   The audio card subsystem for the HPI Library.
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: rdhpisoundcard.cpp,v 1.2.2.1 2008/11/13 14:53:35 fredg Exp $
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
//


#include <qtimer.h>
#include <rdhpisoundcard.h>

#include <unistd.h>

HPI_HSUBSYS *hpi_subsys=NULL;


RDHPISoundCard::RDHPISoundCard(QObject *parent,const char *name)
  : QObject(parent,name)
{
  hpi_subsys=NULL;
  card_quantity=0;
  fade_type=RDHPISoundCard::Log;
  for(int i=0;i<HPI_MAX_ADAPTERS;i++) {
    card_input_streams[i]=0;
    card_output_streams[i]=0;
    card_input_ports[i]=0;
    card_output_ports[i]=0;
    input_mux_type[i]=false;
    timescale_support[i]=false;
    for(int j=0;j<HPI_MAX_NODES;j++) {
      input_port_level[i][j]=false;
      output_port_level[i][j]=false;
      input_port_meter[i][j]=false;
      output_port_meter[i][j]=false;
      input_port_mux[i][j]=false;
      input_port_mux_type[i][j][0]=false;
      input_port_mux_type[i][j][1]=false;
      input_mux_index[i][j][0]=0;
      input_mux_index[i][j][1]=0;
      input_port_aesebu[i][j]=false;
      input_port_aesebu_error[i][j]=false;
      for(int k=0;k<HPI_MAX_STREAMS;k++) {
	input_stream_volume[i][k][j]=false;
	output_stream_volume[i][k][j]=false;
      }
      for(int k=0;k<HPI_MAX_NODES;k++) {
	passthrough_port_volume[i][j][k]=false;
      }
    }
    for(int j=0;j<HPI_MAX_STREAMS;j++) {
      input_stream_meter[i][j]=false;
      output_stream_meter[i][j]=false;
      input_stream_mode[i][j]=false;
      output_stream_mode[i][j]=false;
      input_stream_vox[i][j]=false;
      input_stream_mux[i][j]=false;
    }
  }
  if((hpi_subsys=HPI_SubSysCreate())==NULL) {
    return;
  }
  HPIProbe();
}


RDHPISoundCard::~RDHPISoundCard()
{
  if(hpi_subsys!=NULL) {
    HPI_SubSysFree(hpi_subsys);
  }
}


RDHPISoundCard::Driver RDHPISoundCard::driver() const
{
  return RDHPISoundCard::Hpi;
}


int RDHPISoundCard::getCardQuantity() const
{
  return card_quantity;
}


int RDHPISoundCard::getCardInputStreams(int card) const
{
  return card_input_streams[card];
}


int RDHPISoundCard::getCardOutputStreams(int card) const
{
  return card_output_streams[card];
}


int RDHPISoundCard::getCardInputPorts(int card) const
{
  return card_input_ports[card];
}


int RDHPISoundCard::getCardOutputPorts(int card) const
{
  return card_output_ports[card];
}


const void *RDHPISoundCard::getCardInfo(int card) const
{
  return &hpi_info[card];
}


QString RDHPISoundCard::getCardDescription(int card) const
{
  return card_description[card];
}


QString RDHPISoundCard::getInputStreamDescription(int card,int stream) const
{
  return input_stream_description[card][stream];
}


QString RDHPISoundCard::getOutputStreamDescription(int card,int stream) const
{
  return output_stream_description[card][stream];
}


QString RDHPISoundCard::getInputPortDescription(int card,int port) const
{
  return input_port_description[card][port];
}


QString RDHPISoundCard::getOutputPortDescription(int card,int port) const
{
  return output_port_description[card][port];
}


void RDHPISoundCard::setClockSource(int card,RDHPISoundCard::ClockSource src)
{
  switch(src) {
      case RDHPISoundCard::Internal:
	HPI_SampleClock_SetSource(hpi_subsys,clock_source_control[card],
				  HPI_SAMPLECLOCK_SOURCE_ADAPTER);
	break;
      case RDHPISoundCard::AesEbu:
      case RDHPISoundCard::SpDiff:
	HPI_SampleClock_SetSource(hpi_subsys,clock_source_control[card],
				  HPI_SAMPLECLOCK_SOURCE_AESEBU_SYNC);
	break;
      case RDHPISoundCard::WordClock:
	HPI_SampleClock_SetSource(hpi_subsys,clock_source_control[card],
				  HPI_SAMPLECLOCK_SOURCE_WORD);
	break;
  }
}


bool RDHPISoundCard::haveTimescaling(int card) const
{
  return timescale_support[card];
}


bool RDHPISoundCard::haveInputVolume(int card,int stream,int port) const
{
  return input_stream_volume[card][stream][port];
}


bool RDHPISoundCard::haveOutputVolume(int card,int stream,int port) const
{
  return output_stream_volume[card][stream][port];
}


bool RDHPISoundCard::haveInputLevel(int card,int port) const
{
  return input_port_level[card][port];
}


bool RDHPISoundCard::haveOutputLevel(int card,int port) const
{
  return output_port_level[card][port];
}


bool RDHPISoundCard::haveInputStreamVOX(int card,int stream) const
{
  return input_stream_vox[card][stream];
}


RDHPISoundCard::SourceNode RDHPISoundCard::getInputPortMux(int card,int port)
{
  HW16 type;
  HW16 index;

  HPI_Multiplexer_GetSource(hpi_subsys,input_mux_control[card][port],
			    &type,&index);
  return (RDHPISoundCard::SourceNode)type;
}


bool RDHPISoundCard::setInputPortMux(int card,int port,RDHPISoundCard::SourceNode node)
{
  HW16 hpi_error;

  switch(node) {
      case RDHPISoundCard::LineIn:
	if(HPI_Multiplexer_SetSource(hpi_subsys,
				     input_mux_control[card][port],
				     node,0)!=0) {
	  return false;
	}
	break;
      case RDHPISoundCard::AesEbuIn:
	if((hpi_error=
	    HPI_Multiplexer_SetSource(hpi_subsys,
				      input_mux_control[card][port],node,
				    input_mux_index[card][port][1]))!=0) {
	  return false;
	}
	break;
      default:
	return false;
	break;
  }
  return true;
}


unsigned short RDHPISoundCard::getInputPortError(int card,int port)
{
  HW16 error_word=0;

  if(input_port_aesebu[card][port]) {
    HPI_AESEBU_Receiver_GetErrorStatus(hpi_subsys,
				       input_port_aesebu_control[card][port],
				       &error_word);
  }
  return error_word;
}


RDHPISoundCard::FadeProfile RDHPISoundCard::getFadeProfile() const
{
  return fade_type;
}


void RDHPISoundCard::setFadeProfile(RDHPISoundCard::FadeProfile profile)
{
  fade_type=profile;
  switch(fade_type) {
      case RDHPISoundCard::Linear:
	hpi_fade_type=HPI_VOLUME_AUTOFADE_LINEAR;
	break;
      case RDHPISoundCard::Log:
	hpi_fade_type=HPI_VOLUME_AUTOFADE_LOG;
	break;
  }
}


bool RDHPISoundCard::haveInputStreamMeter(int card,int stream) const
{
  return input_stream_meter[card][stream];
}


bool RDHPISoundCard::haveInputPortMeter(int card,int port) const
{
  return input_stream_meter[card][port];
}


bool RDHPISoundCard::haveOutputStreamMeter(int card,int stream) const
{
  return output_stream_meter[card][stream];
}


bool RDHPISoundCard::haveOutputPortMeter(int card,int port) const
{
  return output_port_meter[card][port];
}


bool RDHPISoundCard::haveTuner(int card,int port) const
{
  return false;
}


void RDHPISoundCard::setTunerBand(int card,int port,
				 RDHPISoundCard::TunerBand band)
{
}


int RDHPISoundCard::tunerFrequency(int card,int port)
{
  return 0;
}


void RDHPISoundCard::setTunerFrequency(int card,int port,int freq)
{
}


bool RDHPISoundCard::tunerSubcarrier(int card,int port,
				    RDHPISoundCard::Subcarrier sub)
{
  return false;
}


int RDHPISoundCard::tunerLowFrequency(int card,int port,
				     RDHPISoundCard::TunerBand band)
{
  return 0;
}


int RDHPISoundCard::tunerHighFrequency(int card,int port,
				      RDHPISoundCard::TunerBand band)
{
  return 0;
}


bool RDHPISoundCard::inputStreamMeter(int card,int stream,short *level)
{
  if(card>=card_quantity) {
    return false;
  }
  if(stream>=card_input_streams[card]) {
    return false;
  }
  HPI_MeterGetPeak(hpi_subsys,input_stream_meter_control[card][stream],level);
  return true;
}


bool RDHPISoundCard::outputStreamMeter(int card,int stream,short *level)
{
  if(card>=card_quantity) {
    return false;
  }
  if(stream>=card_output_streams[card]) {
    return false;
  }
  HPI_MeterGetPeak(hpi_subsys,output_stream_meter_control[card][stream],level);
  return true;
}


bool RDHPISoundCard::inputPortMeter(int card,int port,short *level)
{
  if(card>=card_quantity) {
    return false;
  }
  if(port>=card_input_ports[card]) {
    return false;
  }
  HPI_MeterGetPeak(hpi_subsys,input_port_meter_control[card][port],level);
  return true;
}


bool RDHPISoundCard::outputPortMeter(int card,int port,short *level)
{
  if(card>=card_quantity) {
    return false;
  }
  if(port>=card_output_ports[card]) {
    return false;
  }
  HPI_MeterGetPeak(hpi_subsys,output_port_meter_control[card][port],level);
  return true;
}


bool RDHPISoundCard::haveInputMode(int card,int stream) const
{
  return false;
}


bool RDHPISoundCard::haveOutputMode(int card,int stream) const
{
  return false;
}


bool RDHPISoundCard::haveInputPortMux(int card,int port) const
{
  return input_port_mux[card][port];
}


bool RDHPISoundCard::queryInputPortMux(int card,int port,SourceNode node) const
{
  switch(node) {
      case RDHPISoundCard::LineIn:
	return input_port_mux_type[card][port][0];
	break;
      case RDHPISoundCard::AesEbuIn:
	return input_port_mux_type[card][port][1];
	break;
      default:
	return false;
	break;
  }
}


bool RDHPISoundCard::haveInputStreamMux(int card,int stream) const
{
  return input_stream_mux[card][stream];
}


int RDHPISoundCard::getInputVolume(int card,int stream,int port)
{
  short gain[2];

  HPI_VolumeGetGain(hpi_subsys,
		    input_stream_volume_control[card][stream][port],gain);
  return gain[0];
}


int RDHPISoundCard::getOutputVolume(int card,int stream,int port)
{
  short gain[2];

  HPI_VolumeGetGain(hpi_subsys,
		    output_stream_volume_control[card][stream][port],gain);
  return gain[0];
}


int RDHPISoundCard::getInputLevel(int card,int port)
{
  short gain[2];

  HPI_VolumeGetGain(hpi_subsys,
		    input_port_level_control[card][port],gain);
  return gain[0];
}


int RDHPISoundCard::getOutputLevel(int card,int port)
{
  short gain[2];

  HPI_VolumeGetGain(hpi_subsys,
		    output_port_level_control[card][port],gain);
  return gain[0];
}



void RDHPISoundCard::setInputVolume(int card,int stream,int level)
{
  if(!haveInputVolume(card,stream,0)) {
    return;
  }
  short gain[2];
  gain[0]=level;
  gain[1]=level;
  HPI_VolumeSetGain(hpi_subsys,
		    input_stream_volume_control[card][stream][0],gain);
}


void RDHPISoundCard::setOutputVolume(int card,int stream,int port,int level)
{
  if(!haveOutputVolume(card,stream,port)) {
    return;
  }
  short gain[2];
  gain[0]=level;
  gain[1]=level;
  HPI_VolumeSetGain(hpi_subsys,
		    output_stream_volume_control[card][stream][port],gain);
}



void RDHPISoundCard::fadeOutputVolume(int card,int stream,int port,
				     int level,int length)
{
  if(!haveOutputVolume(card,stream,port)) {
    return;
  }
  short gain[2];

  gain[0]=level;
  gain[1]=level;
  HPI_VolumeAutoFadeProfile(hpi_subsys,
			    output_stream_volume_control[card][stream][port],
			    gain,length,hpi_fade_type);
}


void RDHPISoundCard::setInputLevel(int card,int port,int level)
{
  if(!haveInputLevel(card,port)) {
    return;
  }
  short gain[2];
  gain[0]=level;
  gain[1]=level;
  HPI_VolumeSetGain(hpi_subsys,
		    input_port_level_control[card][port],gain);
}


void RDHPISoundCard::setOutputLevel(int card,int port,int level)
{
  if(!haveOutputLevel(card,port)) {
    return;
  }
  short gain[2];

  gain[0]=level;
  gain[1]=level;
  HPI_VolumeSetGain(hpi_subsys,
		    output_port_level_control[card][port],gain);
}


void RDHPISoundCard::setInputMode(int card,int stream,
				 RDHPISoundCard::ChannelMode mode)
{

}


void RDHPISoundCard::setOutputMode(int card,int stream,
				  RDHPISoundCard::ChannelMode mode)
{

}


void RDHPISoundCard::setInputStreamVOX(int card,int stream,short gain)
{
  HPI_VoxSetThreshold(hpi_subsys,input_stream_vox_control[card][stream],gain);
}


bool RDHPISoundCard::havePassthroughVolume(int card,int in_port,int out_port)
{
  return passthrough_port_volume[card][in_port][out_port];
}


bool RDHPISoundCard::setPassthroughVolume(int card,int in_port,int out_port,
					 int level)
{
  if(!passthrough_port_volume[card][in_port][out_port]) {
    return false;
  }
  short gain[2];
  gain[0]=level;
  gain[1]=level;
  HPI_VolumeSetGain(hpi_subsys,
		    passthrough_port_volume_control[card][in_port][out_port],
		    gain);
  return true;
}


void RDHPISoundCard::clock()
{
  HW16 error_word;

  for(int i=0;i<card_quantity;i++) {
    for(int j=0;j<HPI_MAX_NODES;j++) {
      if(input_port_aesebu[i][j]) {
	error_word=getInputPortError(i,j);
	if(error_word!=input_port_aesebu_error[i][j]) {
	  input_port_aesebu_error[i][j]=error_word;
	  emit inputPortError(i,j);
	}
      }
    }
  }
}


void RDHPISoundCard::HPIProbe()
{
  HW16 hpi_adapter_list[HPI_MAX_ADAPTERS];
  HW32 dummy_serial;
  HW32 dummy_hpi;
  HW16 dummy_version;
  HW16 dummy_type;
  HW16 l;
  HW16 type;
  HW16 index;
  QString str;

  hpi_fade_type=HPI_VOLUME_AUTOFADE_LOG;
  HPI_SubSysGetVersion(hpi_subsys,&dummy_hpi);
  HPI_SubSysFindAdapters(hpi_subsys,(HW16 *)&card_quantity,hpi_adapter_list,
			 HPI_MAX_ADAPTERS);  
  for(int i=0;i<card_quantity;i++) {
    if((hpi_adapter_list[i]&0xF000)==0x6000) { 
      timescale_support[i]=true;
    }
    else {
      timescale_support[i]=false;
    }
    switch(hpi_adapter_list[i]) {
	case 0x5111:
	  input_mux_type[i]=true;
	  break;

	default:
	  input_mux_type[i]=false;
	  break;
    }
    card_input_ports[i]=0;
    card_output_ports[i]=0;
    card_description[i]=QString().sprintf("AudioScience %04X [%d]",
					  hpi_adapter_list[i],i+1);
    HPI_AdapterOpen(hpi_subsys,i);
    HPI_AdapterGetInfo(hpi_subsys,i,
		       &card_output_streams[i],
		       &card_input_streams[i],
		       &dummy_version,(HW32 *)&dummy_serial,
		       &dummy_type);
    hpi_info[i].setSerialNumber(dummy_serial);
    hpi_info[i].setHpiMajorVersion(dummy_hpi>>8);
    hpi_info[i].setHpiMinorVersion(dummy_hpi&255);
    hpi_info[i].setDspMajorVersion((dummy_version>>13)&7);
    hpi_info[i].setDspMinorVersion((dummy_version>>7)&63);
    hpi_info[i].setPcbVersion((char)(((dummy_version>>3)&7)+'A'));
    hpi_info[i].setAssemblyVersion(dummy_version&7);
    HPI_AdapterClose(hpi_subsys,i);
    str=QString(tr("Input Stream"));
    for(int j=0;j<card_input_streams[i];j++) {
      input_stream_description[i][j]=
	QString().sprintf("%s - %s %d",(const char *)card_description[i],
			  (const char *)str,j+1);
    }
    str=QString(tr("Output Stream"));
    for(int j=0;j<card_output_streams[i];j++) {
      output_stream_description[i][j]=
	QString().sprintf("%s - %s %d",(const char *)card_description[i],
			  (const char *)str,j+1);
    }
  }

  //
  // Mixer Initialization
  //
  for(int i=0;i<card_quantity;i++) {
    HPI_MixerOpen(hpi_subsys,i,&hpi_mixer[i]);

    //
    // Get Input Ports
    //
    str=QString(tr("Input Port"));
    for(int k=0;k<HPI_MAX_NODES;k++) {    
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],
			     0,0,
			     HPI_DESTNODE_ISTREAM,k,
			     HPI_CONTROL_MULTIPLEXER,
			     &input_stream_volume_control[i][0][k])==0) {
	card_input_ports[i]++;
	input_port_description[i][k]=
	  QString().sprintf("%s - %s %d",(const char *)card_description[i],
			    (const char *)str,
			    card_input_ports[i]);
      }
    }

    //
    // Get Output Ports
    //
    str=QString(tr("Output Port"));
    for(int k=0;k<HPI_MAX_NODES;k++) {
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],
			     HPI_SOURCENODE_OSTREAM,0,
			     HPI_DESTNODE_LINEOUT,k,
			     HPI_CONTROL_VOLUME,
			     &output_stream_volume_control[i][0][k])==0) {
	output_stream_volume[i][0][k]=true;
	card_output_ports[i]++;
	output_port_description[i][k]=
	  QString().sprintf("%s - %s %d",(const char *)card_description[i],
			    (const char *)str,
			    card_output_ports[i]);
      }
    }
    HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],
			HPI_SOURCENODE_CLOCK_SOURCE,0,
			0,0,
			HPI_CONTROL_SAMPLECLOCK,
			&clock_source_control[i]);
    for(int j=0;j<card_input_streams[i];j++) {
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],  // VOX Controls
			     0,0,
			     HPI_DESTNODE_ISTREAM,j,
			     HPI_CONTROL_VOX,
			     &input_stream_vox_control[i][j])==0) {
	input_stream_vox[i][j]=true;
      }
      else {
	input_stream_vox[i][j]=false;
      }


      if(input_mux_type[i]) {
	if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],  // MUX Controls
			       0,0,
			       HPI_DESTNODE_ISTREAM,j,
			       HPI_CONTROL_MULTIPLEXER,
			       &input_mux_control[i][j])==0) {
	  input_stream_mux[i][j]=true;
	  l=0;
	  input_port_mux_type[i][j][0]=false;
	  input_port_mux_type[i][j][1]=false;
	  while(HPI_Multiplexer_QuerySource(hpi_subsys,
					    input_mux_control[i][j],
					    l++,&type,&index)==0) {
	    switch(type) {
		case HPI_SOURCENODE_LINEIN:
		  input_port_mux_type[i][j][0]=true;
		  input_mux_index[i][j][0]=index;
		  break;
		case HPI_SOURCENODE_AESEBU_IN:
		  input_port_mux_type[i][j][1]=true;
		  input_mux_index[i][j][1]=index;
		  break;
	    }
	  }
	}
	else {
	  input_stream_mux[i][j]=false;
	}
      }
    }
    for(int j=0;j<card_output_streams[i];j++) {
      for(int k=0;k<HPI_MAX_NODES;k++) {
	if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],
			       HPI_SOURCENODE_LINEIN,j,
			       HPI_DESTNODE_ISTREAM,k,
			       HPI_CONTROL_VOLUME,
			       &input_stream_volume_control[i][j][k])==0) {
	  input_stream_volume[i][j][k]=true;
	}
	else {
	  input_stream_volume[i][j][k]=false;
	}
	if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],
			       HPI_SOURCENODE_OSTREAM,j,
			       HPI_DESTNODE_LINEOUT,k,
			       HPI_CONTROL_VOLUME,
			       &output_stream_volume_control[i][j][k])==0) {
	  output_stream_volume[i][j][k]=true;
	}
	else {
	  output_stream_volume[i][j][k]=false;
	}
      }
      //
      // A hack to make the ASI4215 Work with the summed output port
      //
      if(hpi_adapter_list[i]==0x4215) {
	output_stream_volume_control[i][0][4]=
	  output_stream_volume_control[i][0][0];
	output_stream_volume[i][0][4]=true;
	output_stream_volume_control[i][1][4]=
	  output_stream_volume_control[i][1][1];
	output_stream_volume[i][1][4]=true;
	output_stream_volume_control[i][2][4]=
	  output_stream_volume_control[i][2][2];
	output_stream_volume[i][2][4]=true;
	output_stream_volume_control[i][3][4]=
	  output_stream_volume_control[i][3][3];
	output_stream_volume[i][3][4]=true;
      }
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],
			     0,0,
			     HPI_DESTNODE_ISTREAM,j,
			     HPI_CONTROL_METER,
			     &input_stream_meter_control[i][j])==0) {
	input_stream_meter[i][j]=true;
      }
      else {
	input_stream_meter[i][j]=false;
      }
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],
			     HPI_SOURCENODE_OSTREAM,j,
			     0,0,
			     HPI_CONTROL_METER,
			     &output_stream_meter_control[i][j])==0) {
	output_stream_meter[i][j]=true;
      }
      else {
	output_stream_meter[i][j]=false;
      }
    }
    for(int j=0;j<HPI_MAX_NODES;j++) {
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],  // Input Level Controls
			     HPI_SOURCENODE_LINEIN,j,
			     0,0,
			     HPI_CONTROL_LEVEL,
			     &input_port_level_control[i][j])==0) {
	input_port_level[i][j]=true;
      }
      else {
	input_port_level[i][j]=false;
      }
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],  // Output Level Controls
			     0,0,
			     HPI_DESTNODE_LINEOUT,j,
			     HPI_CONTROL_LEVEL,
			     &output_port_level_control[i][j])==0) {
	output_port_level[i][j]=true;
      }
      else {
	output_port_level[i][j]=false;
      }
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],  // Input Port Meter
			     HPI_SOURCENODE_LINEIN,j,
			     0,0,
			     HPI_CONTROL_METER,
			     &input_port_meter_control[i][j])==0) {
	input_port_meter[i][j]=true;
      }
      else {
	input_port_meter[i][j]=false;
      }
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],  // Output Port Meter
			     0,0,
			     HPI_DESTNODE_LINEOUT,j,
			     HPI_CONTROL_METER,
			     &output_port_meter_control[i][j])==0) {
	output_port_meter[i][j]=true;
      }
      else {
	output_port_meter[i][j]=false;
      }
      if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],  // Input Port AES/EBU
			     HPI_SOURCENODE_AESEBU_IN,j,
			     0,0,
			     HPI_CONTROL_AESEBU_RECEIVER,
			     &input_port_aesebu_control[i][j])==0) {
	input_port_aesebu[i][j]=true;
      }
      else {
	input_port_aesebu[i][j]=false;
      }
      if(!input_mux_type[i]) {
	if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],  // Input Port Mux
			       HPI_SOURCENODE_LINEIN,j,
			       0,0,
			       HPI_CONTROL_MULTIPLEXER,
			       &input_mux_control[i][j])==0) {
	  input_port_mux[i][j]=true;
	  l=0;
	  input_port_mux_type[i][j][0]=false;
	  input_port_mux_type[i][j][1]=false;
	  while(HPI_Multiplexer_QuerySource(hpi_subsys,
					    input_mux_control[i][j],
					    l++,&type,&index)==0) {
	    switch(type) {
		case HPI_SOURCENODE_LINEIN:
		  input_port_mux_type[i][j][0]=true;
		  input_mux_index[i][j][0]=index;
		  break;
		case HPI_SOURCENODE_AESEBU_IN:
		  input_port_mux_type[i][j][1]=true;
		  input_mux_index[i][j][1]=index;
		  break;
	    }
	  }
	}
	else {
	  input_port_mux[i][j]=false;
	}
      }
    }

    //
    // Get The Passthroughs
    //
    for(int j=0;j<HPI_MAX_NODES;j++) {
      for(int k=0;k<HPI_MAX_NODES;k++) {
	if(HPI_MixerGetControl(hpi_subsys,hpi_mixer[i],
			       HPI_SOURCENODE_LINEIN,j,
			       HPI_DESTNODE_LINEOUT,k,
			       HPI_CONTROL_VOLUME,
			       &passthrough_port_volume_control[i][j][k])==0) {
	  passthrough_port_volume[i][j][k]=true;
	}
	else {
	  passthrough_port_volume[i][j][k]=false;
	}
      }
    }
  }
  clock_timer=new QTimer(this,"clock_timer");
  connect(clock_timer,SIGNAL(timeout()),this,SLOT(clock()));
  clock_timer->start(METER_INTERVAL);
}
