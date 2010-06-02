// bwf_wavefile.c
//
// Functions for generating a BWF-compliant wav file.
//
// See the Broadcast Wave File Specification (EBU Tech Document 3285,
// with supplements) for details on this file format.
//
// (C) Copyright 2002 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: bwf_wavefile.c,v 1.3 2007/02/14 21:59:12 fredg Exp $
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

#include "options.h"
#include "bwf_wavefile.h"

#include <time.h>
#include <stdlib.h>

//
// Local Variables
//
unsigned short *bwf_levl_buffer=NULL;
unsigned short bwf_levl_peak=0;
unsigned bwf_levl_size=0;
unsigned bwf_levl_peak_offset=0;
unsigned bwf_buffer_size=0;
long bwf_wave_offset;
long bwf_fact_offset;
long bwf_data_offset;
long bwf_frame_size;
int bwf_channels;
struct tm bwf_current_tm;

// **********************************************************************
//
// bwf_open
//
// PURPOSE:  Writes the header data, including the FMT,BEXT and MEXT
// chunks.  Also writes skeletons of the FACT and DATA chunks.
//
// **********************************************************************
void bwf_open(Bit_stream_struc *stream,frame_header *header)
{
  int i;
  time_t current_time;
  struct tm *current_tm;
  unsigned short flags=ACM_MPEG_ID_MPEG1;
  int brate=bitrate[header->version][header->bitrate_index]*1000;
  unsigned srate=
    (unsigned)(1000.0*s_freq[header->version][header->sampling_frequency]);
  bwf_frame_size=144*brate/srate;
  switch(header->mode) {                         // Channels 
      case MPG_MD_MONO:
	bwf_channels=1;
	break;
      default:
	bwf_channels=2;
	break;
  }

  //
  // RIFF Header
  //
  fprintf(stream->pt,"RIFF");
  bwf_wave_offset=ftell(stream->pt);
  bwf_write_dword(stream->pt,0);                 // File Size (placeholder) 
  fprintf(stream->pt,"WAVE");

  //
  // FMT Chunk
  //
  fprintf(stream->pt,"fmt ");
  bwf_write_dword(stream->pt,40);                // Chunk Size 
  bwf_write_sword(stream->pt,WAVE_FORMAT_MPEG);  // Format Tag 
  bwf_write_sword(stream->pt,bwf_channels);      // Channels
  bwf_write_dword(stream->pt,srate);             // Sample Rate 
  bwf_write_dword(stream->pt,brate/8);           // Bytes per Second 
  bwf_write_sword(stream->pt,bwf_frame_size);    // Frame Size 
  bwf_write_sword(stream->pt,0);                 // Bits per Sample 
  bwf_write_sword(stream->pt,40);                // Chunk Size 
  bwf_write_sword(stream->pt,2);                 // MPEG Layer 
  bwf_write_dword(stream->pt,brate);             // Bit Rate 
  switch(header->mode) {                         // Mode 
      case MPG_MD_STEREO:
	bwf_write_sword(stream->pt,ACM_MPEG_STEREO);
	break;
      case MPG_MD_JOINT_STEREO:
	bwf_write_sword(stream->pt,ACM_MPEG_JOINTSTEREO);
	break;
      case MPG_MD_DUAL_CHANNEL:
	bwf_write_sword(stream->pt,ACM_MPEG_DUALCHANNEL);
	break;
      case MPG_MD_MONO:
	bwf_write_sword(stream->pt,ACM_MPEG_SINGLECHANNEL);
	break;
  }

  bwf_write_sword(stream->pt,0);                 // Extended Mode 
  bwf_write_sword(stream->pt,0);                 // Preemphasis 
  flags=0;
  if(header->copyright) {                        // Flags 
    flags|=ACM_MPEG_COPYRIGHT;
  }
  if(header->original) {
    flags|=ACM_MPEG_ORIGINALHOME;
  }
  if(header->error_protection) {
    flags|=ACM_MPEG_PROTECTIONBIT;
  }
  bwf_write_sword(stream->pt,flags);
  bwf_write_dword(stream->pt,0);
  bwf_write_dword(stream->pt,0);

  //
  // FACT Chunk
  //
  fprintf(stream->pt,"fact");
  bwf_write_dword(stream->pt,4);              // Chunk Size 
  bwf_fact_offset=ftell(stream->pt);
  bwf_write_dword(stream->pt,0);              // Sample Count (placeholder) 

  //
  // BEXT Chunk
  //
  fprintf(stream->pt,"bext");
  bwf_write_dword(stream->pt,602);            // Chunk Size 
  for(i=0;i<256;i++) {                        // Description Field 
    fputc(0,stream->pt);
  }
  for(i=0;i<32;i++) {                         // Originator Field 
    fputc(0,stream->pt);
  }
  for(i=0;i<32;i++) {                         // Originator Ref Field 
    fputc(0,stream->pt);
  }
  time(&current_time);
  current_tm=localtime(&current_time);
  bwf_current_tm=*current_tm;
  fprintf(stream->pt,"%04d-%02d-%02d",        // Origination Date 
	  current_tm->tm_year+1900,
	  current_tm->tm_mon+1,
	  current_tm->tm_mday);
  fprintf(stream->pt,"%02d:%02d:%02d",        // Origination Time 
	  current_tm->tm_hour,
	  current_tm->tm_min,
	  current_tm->tm_sec);
  bwf_write_dword(stream->pt,0);              // Sample Time 
  bwf_write_dword(stream->pt,0);
  bwf_write_sword(stream->pt,BWF_VERSION);    // Format Version 
  for(i=0;i<64;i++) {                         // SMPTE UMID 
    fputc(0,stream->pt);
  }
  for(i=0;i<190;i++) {                        // Future
    fputc(0,stream->pt);
  }

  //
  // MEXT Chunk
  //
  fprintf(stream->pt,"mext");
  bwf_write_dword(stream->pt,12);               // Chunk Size 
  flags=0x01;                                   // Homogenous
  if(!glopts.usepadbit) {
    if((srate==44100)||(srate==22050)) {        // Padding Bit Not Active
      flags|=0x06;
    }
  }
  bwf_write_sword(stream->pt,flags);                
  bwf_write_sword(stream->pt,bwf_frame_size);   // MPEG Frame Size 
  bwf_write_sword(stream->pt,0);                // Ancillary Data Length 
  bwf_write_sword(stream->pt,0);                // No Energy Data 
  bwf_write_dword(stream->pt,0);                // Future 

  //
  // DATA Chunk
  //
  fprintf(stream->pt,"data");
  bwf_data_offset=ftell(stream->pt);
  bwf_write_dword(stream->pt,0);                // Chunk Size (placeholder) 

  //
  // Initialize LEVL Buffer
  //
  if((bwf_levl_buffer=malloc(BWF_LEVL_STEP*sizeof(unsigned short)))==NULL) {
    perror("toolame");
    exit(1);
  }
  bwf_buffer_size=BWF_LEVL_STEP;
}


// **********************************************************************
//
// bwf_close
//
// PURPOSE:  Finalizes the FACT and DATA chunks, and adds the LEVL chunk.
//
// **********************************************************************
void bwf_close(Bit_stream_struc *stream)
{
  int i;
  long end=ftell(stream->pt);  

  //
  // LEVL Chunk
  //
  fprintf(stream->pt,"levl");
  bwf_write_dword(stream->pt,128+bwf_levl_size);     // Chunk Size
  bwf_write_dword(stream->pt,0);                     // Chunk Version
  bwf_write_dword(stream->pt,2);                     // Format (unsigned short)
  bwf_write_dword(stream->pt,1);                     // Points per Value
  bwf_write_dword(stream->pt,1152);                  // Frames per value
  bwf_write_dword(stream->pt,bwf_channels);          // Channels
  bwf_write_dword(stream->pt,bwf_levl_size/bwf_channels);  // Peak Frames
  bwf_write_dword(stream->pt,bwf_levl_peak_offset);  // Peak of peaks pointer
  bwf_write_dword(stream->pt,128);                   // Offset to Peak Data
  fprintf(stream->pt,                                // Timestamp
	  "%04d:%02d:%02d:%02d:%02d:%02d:000",
	  bwf_current_tm.tm_year+1900,
	  bwf_current_tm.tm_mon+1,
	  bwf_current_tm.tm_mday,
	  bwf_current_tm.tm_hour,
	  bwf_current_tm.tm_min,
	  bwf_current_tm.tm_sec);
  for(i=0;i<5;i++) {                                 // Timestamp Zero Padding
    fputc(0,stream->pt);
  }
  for(i=0;i<60;i++) {                                // Future
    fputc(0,stream->pt);
  }
  for(i=0;i<bwf_levl_size;i++) {
    bwf_write_sword(stream->pt,bwf_levl_buffer[i]);
  }
  free(bwf_levl_buffer);
  bwf_levl_buffer=NULL;

  //
  // File Size
  //
  fseek(stream->pt,bwf_wave_offset,SEEK_SET);
  bwf_write_dword(stream->pt,end-12);

  //
  // Samples Written
  //
  fseek(stream->pt,bwf_fact_offset,SEEK_SET);
  bwf_write_dword(stream->pt,1152*(bwf_levl_size/bwf_channels));  // Peak Frames
//  bwf_write_dword(stream->pt,1152*((end-bwf_data_offset)/bwf_frame_size));

  //
  // DATA Chunk Size
  //
  fseek(stream->pt,bwf_data_offset,SEEK_SET);
  bwf_write_dword(stream->pt,end-bwf_data_offset-4);
}


// **********************************************************************
//
// bwf_calc_peak
//
// PURPOSE:  Calculate the peak levels in an audio frame.
//
// **********************************************************************
void bwf_calc_peak(short buffer[2][1152],int chans,int len)
{
  int i;
  int j;
  unsigned short peak;
  static unsigned sample=0;

  for(i=0;i<chans;i++) {
    peak=0;
    for(j=0;j<len;j++) {
      if(abs(buffer[i][j])>peak) {
	peak=abs(buffer[i][j]);
      }
    }
    if(peak>bwf_levl_peak) {
      bwf_levl_peak=peak;
      bwf_levl_peak_offset=sample;
    }
    sample++;
    bwf_levl_push(peak);
  }
}


// **********************************************************************
//
// bwf_levl_push
//
// PURPOSE:  Push a peak value into the LEVL buffer.
//
// **********************************************************************
void bwf_levl_push(unsigned short level)
{
  if(bwf_levl_size==bwf_buffer_size) {
    bwf_buffer_size+=BWF_LEVL_STEP;
    if((bwf_levl_buffer=realloc(bwf_levl_buffer,
		bwf_buffer_size*sizeof(unsigned short)))==NULL) {
      perror("toolame");
      exit(1);
    }
  }
  bwf_levl_buffer[bwf_levl_size++]=level;
}


// **********************************************************************
//
// bwf_write_dword
//
// PURPOSE:  Write 32 bit unsigned ints in little endian order to a 
// file handle.
//
// **********************************************************************
void bwf_write_dword(FILE *ptr,unsigned value)
{
  fputc(value&0xff,ptr);
  fputc((value>>8)&0xff,ptr);
  fputc((value>>16)&0xff,ptr);
  fputc((value>>24)&0xff,ptr);
}


// **********************************************************************
//
// bwf_write_dword
//
// PURPOSE:  Write 16 bit unsigned ints in little endian order to a 
// file handle.
//
// **********************************************************************
void bwf_write_sword(FILE *ptr,unsigned short value)
{
  fputc(value&0xff,ptr);
  fputc((value>>8)&0xff,ptr);
}
