// bwf_wavefile.h
//
// Functions for generating a BWF-compliant wav file.
//
// See the Broadcast Wave File Specification (EBU Tech Document 3285,
// with supplements) for details on this file format.
//
// by Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: bwf_wavefile.h,v 1.3 2007/02/14 21:59:12 fredg Exp $
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

#ifndef BWF_WAVEFILE_H
#define BWF_WAVEFILE_H

#include <stdio.h>
#include "common.h"

/*
 * Function Prototypes
 */
void bwf_open(Bit_stream_struc *,frame_header *);
void bwf_close(Bit_stream_struc *);
void bwf_calc_peak(short[2][1152],int,int);
void bwf_levl_push(unsigned short);
void bwf_write_dword(FILE *,unsigned);
void bwf_write_sword(FILE *,unsigned short);

//
// Memory Allocation Step Size
//
#define BWF_LEVL_STEP 1024

/*
 * BWF Stuff
 */
#define BWF_VERSION 1

/* 
 * WAVE Format Categories 
 */
#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_MPEG 0x0050

/*
 * Proprietary Format Categories
 * (Not supported)
 */
#define MS_FORMAT_ADPCM 0x0002
#define ITU_FORMAT_G711_ALAW 0x0006
#define ITU_FORMAT_G711_MLAW 0x0007
#define IMA_FORMAT_ADPCM 0x0011
#define ITU_FORMAT_G723_ADPCM 0x0016
#define GSM_FORMAT_610 0x0031
#define ITU_FORMAT_G721_ADPCM 0x0040
#define IBM_FORMAT_MULAW 0x0101
#define IBM_FORMAT_ALAW 0x0102
#define IBM_FORMAT_ADPCM 0x0103

/*
 * MPEG Defines
 *
 * fwHeadLayer Flags
 */
#define ACM_MPEG_LAYER1 0x0001
#define ACM_MPEG_LAYER2 0x0002
#define ACM_MPEG_LAYER3 0x0004

/*
 * fwHeadMode Flags
 */
#define ACM_MPEG_STEREO 0x0001
#define ACM_MPEG_JOINTSTEREO 0x0002
#define ACM_MPEG_DUALCHANNEL 0x0004
#define ACM_MPEG_SINGLECHANNEL 0x0008

/*
 * fwHeadFlags Flags
 */
#define ACM_MPEG_PRIVATEBIT 0x0001
#define ACM_MPEG_COPYRIGHT 0x0002
#define ACM_MPEG_ORIGINALHOME 0x0004
#define ACM_MPEG_PROTECTIONBIT 0x0008
#define ACM_MPEG_ID_MPEG1 0x0010



#endif  // BWF_WAVEFILE_H
