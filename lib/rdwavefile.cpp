//   rdwavefile.cpp
//
//   A class for handling Microsoft WAV files.
//
//   (C) Copyright 2002-2008 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: rdwavefile.cpp,v 1.13.2.6 2010/01/22 21:04:15 cvs Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <math.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <FLAC/metadata.h>

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>

#include <rd.h>
#include <rdwavefile.h>
#include <rdconf.h>

#include <id3/tag.h>
#include <id3/misc_support.h>

#include <mad.h> 
#include <qfile.h>
#include <map>


// Taglib for reading OGG Metadata
#include <taglib/taglib.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
// 

#ifndef _AFLIBMPGFILE_H
#define _AFLIBMPGFILE_H

#define INPUT_BUFFER_SIZE	(4*8192)
#define OUTPUT_BUFFER_SIZE	8192 /* Must be an integer multiple of 4. */
class Mpegtoraw;

class RMpegFile  {

   public:

      RMpegFile();

      ~RMpegFile();

      bool 
	 afopen(
	       QFile& wave_file 
	       );

      void 
	 afclose();

      // Read len shorts from mpeg file into p_data.
      // Return shorts read.
      int 
	 afread(
	       signed short * p_data,
	       int len
	       );

      int 
	 afread(
	       float * p_data,
	       int len
	       );

      int
	 afseek( int position, int whence);

      bool IsMpeg(const char *);
      // General
      int  getVersion(void)   const {return _version;};
      int  getLayer(void)     const {return _layer;};
      // Frequency and bitrate
      int  getFrequency(void) const {return _frequency;};
      int  getBitrate(void)   const {return _bitrate;};
      int getTotalSamples(void) const 
      //{return _total_frames*_samples_per_frame;};
      {return _total_samples;};
      int getDataLength(void) const { return _data_length;};
      int getDataStart(void) const { return _data_start;};
      int getChannels(void) const { return _channels;};
   private:

      signed short MadFixedToSshort(mad_fixed_t Fixed);
      float MadFixedToFloat(mad_fixed_t Fixed);
      unsigned long seekFrame(unsigned long frame);
      int MadDecodeFrame(signed short * buffer, unsigned long frame);
      bool MadDecodeHeader(unsigned long frames);
      size_t getOffset(unsigned long frame);

      std::map<unsigned long,size_t> _frame_offsets;

      struct mad_stream	Stream;
      struct mad_frame	Frame;
      struct mad_synth	Synth;
      mad_timer_t			Timer;
      int                 _version;
      int                 _layer;
      int                 _bitrate;
      int                 _frequency;
      int                 _length;
      long 		_samples_per_frame;
      long		_total_frames;
      long		_total_samples;
      int		_channels;
      long 			    _buflen;
      int 			    _bufpos;
      unsigned long 		_frame_count;

      unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD],
				*GuardPtr;
      int					Status;


      //        signed short *          _buffer;
      signed short		_buffer[OUTPUT_BUFFER_SIZE];
      QFile               _fd;
      size_t              _last_read_pos;


      bool id3v1_tag;
      bool id3v2_tag;
      unsigned id3v2_offset;
      int _data_length;
      int _data_start;
};


#endif

#if (MAD_VERSION_MAJOR>=1) || \
			((MAD_VERSION_MAJOR==0) && \
			 (((MAD_VERSION_MINOR==14) && \
			   (MAD_VERSION_PATCH>=2)) || \
			  (MAD_VERSION_MINOR>14)))
#define MadErrorString(x) mad_stream_errorstr(x)
#else
static const char *MadErrorString(const struct mad_stream *Stream)
{
   switch(Stream->error)
   {
      /* Generic unrecoverable errors. */
      case MAD_ERROR_BUFLEN:
	 return("input buffer too small (or EOF)");
      case MAD_ERROR_BUFPTR:
	 return("invalid (null) buffer pointer");
      case MAD_ERROR_NOMEM:
	 return("not enough memory");

	 /* Frame header related unrecoverable errors. */
      case MAD_ERROR_LOSTSYNC:
	 return("lost synchronization");
      case MAD_ERROR_BADLAYER:
	 return("reserved header layer value");
      case MAD_ERROR_BADBITRATE:
	 return("forbidden bitrate value");
      case MAD_ERROR_BADSAMPLERATE:
	 return("reserved sample frequency value");
      case MAD_ERROR_BADEMPHASIS:
	 return("reserved emphasis value");

	 /* Recoverable errors */
      case MAD_ERROR_BADCRC:
	 return("CRC check failed");
      case MAD_ERROR_BADBITALLOC:
	 return("forbidden bit allocation value");
      case MAD_ERROR_BADSCALEFACTOR:
	 return("bad scalefactor index");
      case MAD_ERROR_BADFRAMELEN:
	 return("bad frame length");
      case MAD_ERROR_BADBIGVALUES:
	 return("bad big_values count");
      case MAD_ERROR_BADBLOCKTYPE:
	 return("reserved block_type");
      case MAD_ERROR_BADSCFSI:
	 return("bad scalefactor selection info");
      case MAD_ERROR_BADDATAPTR:
	 return("bad main_data_begin pointer");
      case MAD_ERROR_BADPART3LEN:
	 return("bad audio data length");
      case MAD_ERROR_BADHUFFTABLE:
	 return("bad Huffman table select");
      case MAD_ERROR_BADHUFFDATA:
	 return("Huffman data overrun");
      case MAD_ERROR_BADSTEREO:
	 return("incompatible block_type for JS");

	 /* Unknown error. This switch may be out of sync with libmad's
	  * defined error codes.
	  */
      default:
	 return("Unknown error code");
   }
}
#endif

/****************************************************************************
 * Converts a sample from libmad's fixed point number format to a signed	*
 * short (16 bits).															*
 ****************************************************************************/
signed short RMpegFile::MadFixedToSshort(mad_fixed_t Fixed)
{
   /* A fixed point number is formed of the following bit pattern:
    *
    * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    * MSB                          LSB
    * S ==> Sign (0 is positive, 1 is negative)
    * W ==> Whole part bits
    * F ==> Fractional part bits
    *
    * This pattern contains MAD_F_FRACBITS fractional bits, one
    * should alway use this macro when working on the bits of a fixed
    * point number. It is not guaranteed to be constant over the
    * different platforms supported by libmad.
    *
    * The signed short value is formed, after clipping, by the least
    * significant whole part bit, followed by the 15 most significant
    * fractional part bits. Warning: this is a quick and dirty way to
    * compute the 16-bit number, madplay includes much better
    * algorithms.
    */

   /* Clipping */
   if(Fixed>=MAD_F_ONE)
      return(SHRT_MAX);
   if(Fixed<=-MAD_F_ONE)
      return(-SHRT_MAX);

   /* Conversion. */
   Fixed=Fixed>>(MAD_F_FRACBITS-15);
   return((signed short)Fixed);
}

/****************************************************************************
 * Converts a sample from libmad's fixed point number format to a signed	*
 * short (16 bits).															*
 ****************************************************************************/
float RMpegFile::MadFixedToFloat(mad_fixed_t Fixed)
{
   /* A fixed point number is formed of the following bit pattern:
    *
    * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    * MSB                          LSB
    * S ==> Sign (0 is positive, 1 is negative)
    * W ==> Whole part bits
    * F ==> Fractional part bits
    *
    * This pattern contains MAD_F_FRACBITS fractional bits, one
    * should alway use this macro when working on the bits of a fixed
    * point number. It is not guaranteed to be constant over the
    * different platforms supported by libmad.
    *
    * The signed short value is formed, after clipping, by the least
    * significant whole part bit, followed by the 15 most significant
    * fractional part bits. Warning: this is a quick and dirty way to
    * compute the 16-bit number, madplay includes much better
    * algorithms.
    */

   float f;
   if (Fixed & (1<<31)) {
      // negative
      Fixed = Fixed & ((1<<31)-1); // strip sign
      f = -(float)Fixed / (1<<MAD_F_FRACBITS);
   }
   else {
      f = (float)Fixed / (1<<MAD_F_FRACBITS);
   }
   return f;
}

size_t RMpegFile::getOffset(unsigned long frame){
   //unsigned long frame_no=0;
   size_t offset = 0;

   std::map<unsigned long,size_t>::const_iterator it; 
   /*
      for(it = _frame_offsets.begin();it != _frame_offsets.end() && frame_no != frame;it++){
      offset = it->second;
      frame_no = it->first;
      }
      */

   while(1){
      it  = _frame_offsets.find(frame);
      if(it == _frame_offsets.end())
      {
	 if(MadDecodeHeader(1)){
	    offset = _data_start;
	    break;
	 }
      }
      else
      {
	 offset = it->second;
	 break;
      }
   }
   return offset;

}

   unsigned long
RMpegFile::seekFrame(unsigned long frame)
{

   int offset = 0;

   //bool dump_one = true;
   unsigned long frame_no=0;
   if(frame >=5){
      frame_no = frame - 5;
   } else {
      frame_no = 0;      
   }

   offset = getOffset(frame_no);
   mad_synth_finish(&Synth);
   mad_frame_finish(&Frame);
   mad_stream_finish(&Stream);

   mad_stream_init(&Stream);
   mad_frame_init(&Frame);
   mad_synth_init(&Synth);
   mad_timer_reset(&Timer);

   _frame_count = frame_no;
   GuardPtr = NULL;
   if(_fd.at(offset)) _last_read_pos = offset;
   while(frame_no != frame){
      MadDecodeFrame(_buffer,1);
      frame_no++;
   }
   return frame;

}
static int PrintFrameInfo(FILE *fp, struct mad_header *Header)
{
   const char	*Layer,
	 *Mode,

	 *Emphasis;

   /* Convert the layer number to it's printed representation. */
   switch(Header->layer)
   {
      case MAD_LAYER_I:
	 Layer="I";
	 break;
      case MAD_LAYER_II:
	 Layer="II";
	 break;
      case MAD_LAYER_III:
	 Layer="III";
	 break;
      default:
	 Layer="(unexpected layer value)";
	 break;
   }

   /* Convert the audio mode to it's printed representation. */
   switch(Header->mode)
   {
      case MAD_MODE_SINGLE_CHANNEL:
	 Mode="single channel";
	 break;
      case MAD_MODE_DUAL_CHANNEL:
	 Mode="dual channel";
	 break;
      case MAD_MODE_JOINT_STEREO:
	 Mode="joint (MS/intensity) stereo";
	 break;
      case MAD_MODE_STEREO:
	 Mode="normal LR stereo";
	 break;
      default:
	 Mode="(unexpected mode value)";
	 break;
   }

   /* Convert the emphasis to it's printed representation. Note that
    * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
    * version 0.15.0b.
    */
   switch(Header->emphasis)
   {
      case MAD_EMPHASIS_NONE:
	 Emphasis="no";
	 break;
      case MAD_EMPHASIS_50_15_US:
	 Emphasis="50/15 us";
	 break;
      case MAD_EMPHASIS_CCITT_J_17:
	 Emphasis="CCITT J.17";
	 break;
#if (MAD_VERSION_MAJOR>=1) || \
	 ((MAD_VERSION_MAJOR==0) && (MAD_VERSION_MINOR>=15))
      case MAD_EMPHASIS_RESERVED:
	 Emphasis="reserved(!)";
	 break;
#endif
      default:
	 Emphasis="(unexpected emphasis value)";
	 break;
   }

/*   fprintf(fp,"%lu kb/s audio MPEG layer %s stream %s CRC, "
	 "%s with %s emphasis at %d Hz sample rate\n",
	 Header->bitrate,Layer,
	 Header->flags&MAD_FLAG_PROTECTION?"with":"without",
	 Mode,Emphasis,Header->samplerate);*/
   return(ferror(fp));
}

/****************************************************************************
 * Main decoding loop. This is where mad is used.							*
 ****************************************************************************/
bool RMpegFile::MadDecodeHeader(unsigned long frames)
{
   struct mad_header Header;
   mad_header_init(&Header);
   Status = 0;
   while(frames){
      /* The input bucket must be filled if it becomes empty or if
       * it's the first execution of the loop.
       */
      if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
      {
	 size_t			ReadSize,
				Remaining;
	 unsigned char	*ReadStart;

	 /* {2} libmad may not consume all bytes of the input
	  * buffer. If the last frame in the buffer is not wholly
	  * contained by it, then that frame's start is pointed by
	  * the next_frame member of the Stream structure. This
	  * common situation occurs when mad_frame_decode() fails,
	  * sets the stream error code to MAD_ERROR_BUFLEN, and
	  * sets the next_frame pointer to a non NULL value. (See
	  * also the comment marked {4} bellow.)
	  *
	  * When this occurs, the remaining unused bytes must be
	  * put back at the beginning of the buffer and taken in
	  * account before refilling the buffer. This means that
	  * the input buffer must be large enough to hold a whole
	  * frame at the highest observable bit-rate (currently 448
	  * kb/s). XXX=XXX Is 2016 bytes the size of the largest
	  * frame? (448000*(1152/32000))/8
	  */
	 _last_read_pos = _fd.at();
	 if(Stream.next_frame!=NULL)
	 {
	    Remaining=Stream.bufend-Stream.next_frame;
	    memmove(InputBuffer,Stream.next_frame,Remaining);
	    ReadStart=InputBuffer+Remaining;
	    ReadSize=INPUT_BUFFER_SIZE-Remaining;
	    _last_read_pos -= Remaining;
	 }
	 else
	    ReadSize=INPUT_BUFFER_SIZE,
	       ReadStart=InputBuffer,
	       Remaining=0;

	 /* Fill-in the buffer. If an error occurs print a message
	  * and leave the decoding loop. If the end of stream is
	  * reached we also leave the loop but the return status is
	  * left untouched.
	  */
	 ReadSize=_fd.readBlock((char*)ReadStart,ReadSize);
	 if(ReadSize<=0)
	 {
	    if(ReadSize < 0)
	       Status=1;
	    break;
	 }

	 /* {3} When decoding the last frame of a file, it must be
	  * followed by MAD_BUFFER_GUARD zero bytes if one wants to
	  * decode that last frame. When the end of file is
	  * detected we append that quantity of bytes at the end of
	  * the available data. Note that the buffer can't overflow
	  * as the guard size was allocated but not used the the
	  * buffer management code. (See also the comment marked
	  * {1}.)
	  *
	  * In a message to the mad-dev mailing list on May 29th,
	  * 2001, Rob Leslie explains the guard zone as follows:
	  *
	  *    "The reason for MAD_BUFFER_GUARD has to do with the
	  *    way decoding is performed. In Layer III, Huffman
	  *    decoding may inadvertently read a few bytes beyond
	  *    the end of the buffer in the case of certain invalid
	  *    input. This is not detected until after the fact. To
	  *    prevent this from causing problems, and also to
	  *    ensure the next frame's main_data_begin pointer is
	  *    always accessible, MAD requires MAD_BUFFER_GUARD
	  *    (currently 8) bytes to be present in the buffer past
	  *    the end of the current frame in order to decode the
	  *    frame."
	  */
	 if(_fd.atEnd())
	 {
	    GuardPtr=ReadStart+ReadSize;
	    memset(GuardPtr,0,MAD_BUFFER_GUARD);
	    ReadSize+=MAD_BUFFER_GUARD;
	 }

	 /* Pipe the new buffer content to libmad's stream decoder
	  * facility.
	  */
	 mad_stream_buffer(&Stream,(unsigned char*)InputBuffer,ReadSize+Remaining);
	 Stream.error=MAD_ERROR_NONE;
      }

      /* Decode the next MPEG frame. The streams is read from the
       * buffer, its constituents are break down and stored the the
       * Frame structure, ready for examination/alteration or PCM
       * synthesis. Decoding options are carried in the Frame
       * structure from the Stream structure.
       *
       * Error handling: mad_frame_decode() returns a non zero value
       * when an error occurs. The error condition can be checked in
       * the error member of the Stream structure. A mad error is
       * recoverable or fatal, the error status is checked with the
       * MAD_RECOVERABLE macro.
       *
       * {4} When a fatal error is encountered all decoding
       * activities shall be stopped, except when a MAD_ERROR_BUFLEN
       * is signaled. This condition means that the
       * mad_frame_decode() function needs more input to complete
       * its work. One shall refill the buffer and repeat the
       * mad_frame_decode() call. Some bytes may be left unused at
       * the end of the buffer if those bytes forms an incomplete
       * frame. Before refilling, the remaining bytes must be moved
       * to the beginning of the buffer and used for input for the
       * next mad_frame_decode() invocation. (See the comments
       * marked {2} earlier for more details.)
       *
       * Recoverable errors are caused by malformed bit-streams, in
       * this case one can call again mad_frame_decode() in order to
       * skip the faulty part and re-sync to the next frame.
       */
      if(mad_header_decode(&Header,&Stream))
      {
	 //fprintf(stderr,"%s\n",MadErrorString(&Stream));
	 if(MAD_RECOVERABLE(Stream.error))
	 {
	    /* Do not print a message if the error is a loss of
	     * synchronization and this loss is due to the end of
	     * stream guard bytes. (See the comments marked {3}
	     * supra for more informations about guard bytes.)
	     */
	    if(Stream.error!=MAD_ERROR_LOSTSYNC ||
		  Stream.this_frame!=(unsigned char*)GuardPtr)
	    {
	    }
	    continue;
	 }
	 else
	    if(Stream.error==MAD_ERROR_BUFLEN)
	       continue;
	    else
	    {
	       Status=1;
	       break;
	    }
      }
      frames--;

      std::map<unsigned long,size_t>::const_iterator it; 
      it  = _frame_offsets.find(_frame_count);
      if(it == _frame_offsets.end())
      {
	 size_t this_frame_at = _last_read_pos + Stream.this_frame - Stream.buffer;

	 std::pair<unsigned long,size_t> offset(_frame_count,this_frame_at);
	 _frame_offsets.insert(offset);
      }

      /* Accounting. The computed frame duration is in the frame
       * header structure. It is expressed as a fixed point number
       * whole data type is mad_timer_t. It is different from the
       * samples fixed point format and unlike it, it can't directly
       * be added or subtracted. The timer module provides several
       * functions to operate on such numbers. Be careful there, as
       * some functions of libmad's timer module receive some of
       * their mad_timer_t arguments by value!
       */
      _frame_count++;
   }
   return Status;
}

/****************************************************************************
 * Main decoding loop. This is where mad is used.							*
 ****************************************************************************/
int RMpegFile::MadDecodeFrame(signed short * buffer, unsigned long frames)
{
   // unsigned char * c_buffer = buffer;
   signed short * head = buffer;

   while(frames){
      /* The input bucket must be filled if it becomes empty or if
       * it's the first execution of the loop.
       */
      if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
      {
	 size_t			ReadSize,
				Remaining;
	 unsigned char	*ReadStart;

	 /* {2} libmad may not consume all bytes of the input
	  * buffer. If the last frame in the buffer is not wholly
	  * contained by it, then that frame's start is pointed by
	  * the next_frame member of the Stream structure. This
	  * common situation occurs when mad_frame_decode() fails,
	  * sets the stream error code to MAD_ERROR_BUFLEN, and
	  * sets the next_frame pointer to a non NULL value. (See
	  * also the comment marked {4} bellow.)
	  *
	  * When this occurs, the remaining unused bytes must be
	  * PUT back at the beginning of the buffer and taken in
	  * account before refilling the buffer. This means that
	  * the input buffer must be large enough to hold a whole
	  * frame at the highest observable bit-rate (currently 448
	  * kb/s). XXX=XXX Is 2016 bytes the size of the largest
	  * frame? (448000*(1152/32000))/8
	  */
	 _last_read_pos = _fd.at();
	 if(Stream.next_frame!=NULL)
	 {
	    Remaining=Stream.bufend-Stream.next_frame;
	    memmove(InputBuffer,Stream.next_frame,Remaining);
	    ReadStart=InputBuffer+Remaining;
	    ReadSize=INPUT_BUFFER_SIZE-Remaining;
	    _last_read_pos -= Remaining;
	 }
	 else
	    ReadSize=INPUT_BUFFER_SIZE,
	       ReadStart=InputBuffer,
	       Remaining=0;

	 /* Fill-in the buffer. If an error occurs print a message
	  * and leave the decoding loop. If the end of stream is
	  * reached we also leave the loop but the return status is
	  * left untouched.
	  */
	 ReadSize=_fd.readBlock((char*)ReadStart,ReadSize);
	 if(ReadSize<=0)
	 {
	    if(ReadSize < 0)
	       Status=1;
	    break;
	 }

	 /* {3} When decoding the last frame of a file, it must be
	  * followed by MAD_BUFFER_GUARD zero bytes if one wants to
	  * decode that last frame. When the end of file is
	  * detected we append that quantity of bytes at the end of
	  * the available data. Note that the buffer can't overflow
	  * as the guard size was allocated but not used the the
	  * buffer management code. (See also the comment marked
	  * {1}.)
	  *
	  * In a message to the mad-dev mailing list on May 29th,
	  * 2001, Rob Leslie explains the guard zone as follows:
	  *
	  *    "The reason for MAD_BUFFER_GUARD has to do with the
	  *    way decoding is performed. In Layer III, Huffman
	  *    decoding may inadvertently read a few bytes beyond
	  *    the end of the buffer in the case of certain invalid
	  *    input. This is not detected until after the fact. To
	  *    prevent this from causing problems, and also to
	  *    ensure the next frame's main_data_begin pointer is
	  *    always accessible, MAD requires MAD_BUFFER_GUARD
	  *    (currently 8) bytes to be present in the buffer past
	  *    the end of the current frame in order to decode the
	  *    frame."
	  */
	 if(_fd.atEnd())
	 {
	    GuardPtr=ReadStart+ReadSize;
	    memset(GuardPtr,0,MAD_BUFFER_GUARD);
	    ReadSize+=MAD_BUFFER_GUARD;
	 }

	 /* Pipe the new buffer content to libmad's stream decoder
	  * facility.
	  */
	 mad_stream_buffer(&Stream,(unsigned char*)InputBuffer,ReadSize+Remaining);
	 Stream.error=MAD_ERROR_NONE;
      }

      /* Decode the next MPEG frame. The streams is read from the
       * buffer, its constituents are break down and stored the the
       * Frame structure, ready for examination/alteration or PCM
       * synthesis. Decoding options are carried in the Frame
       * structure from the Stream structure.
       *
       * Error handling: mad_frame_decode() returns a non zero value
       * when an error occurs. The error condition can be checked in
       * the error member of the Stream structure. A mad error is
       * recoverable or fatal, the error status is checked with the
       * MAD_RECOVERABLE macro.
       *
       * {4} When a fatal error is encountered all decoding
       * activities shall be stopped, except when a MAD_ERROR_BUFLEN
       * is signaled. This condition means that the
       * mad_frame_decode() function needs more input to complete
       * its work. One shall refill the buffer and repeat the
       * mad_frame_decode() call. Some bytes may be left unused at
       * the end of the buffer if those bytes forms an incomplete
       * frame. Before refilling, the remaining bytes must be moved
       * to the beginning of the buffer and used for input for the
       * next mad_frame_decode() invocation. (See the comments
       * marked {2} earlier for more details.)
       *
       * Recoverable errors are caused by malformed bit-streams, in
       * this case one can call again mad_frame_decode() in order to
       * skip the faulty part and re-sync to the next frame.
       */
      if(mad_frame_decode(&Frame,&Stream))
      {
	 //fprintf(stderr,"%s\n",MadErrorString(&Stream));
	 if(MAD_RECOVERABLE(Stream.error))
	 {
	    /* Do not print a message if the error is a loss of
	     * synchronization and this loss is due to the end of
	     * stream guard bytes. (See the comments marked {3}
	     * supra for more informations about guard bytes.)
	     */
	    if(Stream.error!=MAD_ERROR_LOSTSYNC ||
		  Stream.this_frame!=(unsigned char*)GuardPtr)
	    {
	    }
	    continue;
	 }
	 else
	    if(Stream.error==MAD_ERROR_BUFLEN)
	       continue;
	    else
	    {
	       Status=1;
	       break;
	    }
      }
      frames--;

      std::map<unsigned long,size_t>::const_iterator it; 
      it  = _frame_offsets.find(_frame_count);
      if(it == _frame_offsets.end())
      {
	 size_t this_frame_at = _last_read_pos + Stream.this_frame - Stream.buffer;

	 std::pair<unsigned long,size_t> offset(_frame_count,this_frame_at);
	 _frame_offsets.insert(offset);
      }

      /* The characteristics of the stream's first frame is printed
       * on stderr. The first frame is representative of the entire
       * stream.
       */
      if(_frame_count==0)
	 if(PrintFrameInfo(stderr,&Frame.header))
	 {
	    Status=1;
	    break;
	 }

      /* Accounting. The computed frame duration is in the frame
       * header structure. It is expressed as a fixed point number
       * whole data type is mad_timer_t. It is different from the
       * samples fixed point format and unlike it, it can't directly
       * be added or subtracted. The timer module provides several
       * functions to operate on such numbers. Be careful there, as
       * some functions of libmad's timer module receive some of
       * their mad_timer_t arguments by value!
       */
      _frame_count++;
      mad_timer_add(&Timer,Frame.header.duration);

      /* Once decoded the frame is synthesized to PCM samples. No errors
       * are reported by mad_synth_frame();
       */
      mad_synth_frame(&Synth,&Frame);

      /* Synthesized samples must be converted from libmad's fixed
       * point number to the consumer format. Here we use unsigned
       * 16 bit big endian integers on two channels. Integer samples
       * are temporarily stored in a buffer that is flushed when
       * full.
       */
      for(int i=0;i<Synth.pcm.length;i++)
      {
	 signed short	Sample;

	 /* Left channel */
	 Sample=MadFixedToSshort(Synth.pcm.samples[0][i]);
	 *(buffer++)=Sample;
	 /*
	  *(c_buffer++)=Sample>>8;
	  *(c_buffer++)=Sample&0xff;
	  */

	 /* Right channel. If the decoded stream is monophonic then
	  * the right output channel is the same as the left one.
	  */
	 if(MAD_NCHANNELS(&Frame.header)==2)
	 {
	    Sample=MadFixedToSshort(Synth.pcm.samples[1][i]);
	 }
	 *(buffer++)=Sample;
	    /*
	     *(c_buffer++)=Sample>>8;
	     *(c_buffer++)=Sample&0xff;
	     */


      }
   }
   return buffer - head;
}

//#define RMPEGFILE_SHOW_SLOTS 1
RMpegFile::RMpegFile() 
{
   /* First the structures used by libmad must be initialized. */
   mad_stream_init(&Stream);
   mad_frame_init(&Frame);
   mad_synth_init(&Synth);
   mad_timer_reset(&Timer);
}

RMpegFile::~RMpegFile()
{
   mad_synth_finish(&Synth);
   mad_frame_finish(&Frame);
   mad_stream_finish(&Stream);
   _fd.close();
}

void 
RMpegFile::afclose()
{
   _fd.close();
}

bool
RMpegFile::afopen(
      QFile& wave_file
      )
{
   id3v1_tag = false;
   id3v2_tag = false;
   id3v2_offset = 0;
   _bufpos = _buflen = _frame_count = 0;
   // This function needs to be redone to find the header info correctly
   // This function will open an existing MP3 file.
   ID3_Tag id3_tag(QCString().sprintf("%s",(const char *)wave_file.name().utf8()));
   const Mp3_Headerinfo* mp3info;
   mp3info = id3_tag.GetMp3HeaderInfo();
        
   if(mp3info){
      switch(mp3info->version){
	 case MPEGVERSION_1:
	    _version = 0;
	    _samples_per_frame = 1152; 
	    break;
	 case MPEGVERSION_2:
	    _version = 1;
	    _samples_per_frame = 576; 
	    break;
	 case MPEGVERSION_2_5:
	    _version = 2;
	    _samples_per_frame = 576; 
	    break;
	 default:
	    return false;
	    break;
      }
      switch (mp3info->layer)
      {
         case MPEGLAYER_III:
            _layer = 3;
            break;
         case MPEGLAYER_II:
            _layer = 2;
            break;
         case MPEGLAYER_I:
            _layer = 1;
            break;
         default:
  	    return false;
            break;
      }
      switch (mp3info->channelmode)
      {
         case MP3CHANNELMODE_STEREO:
         case MP3CHANNELMODE_JOINT_STEREO:
         case MP3CHANNELMODE_DUAL_CHANNEL:
         case MP3CHANNELMODE_SINGLE_CHANNEL:
         case MP3CHANNELMODE_FALSE:
            break;
      }
      _channels = 2;

      _bitrate = mp3info->bitrate;
      _frequency = mp3info->frequency;
      _total_samples = ((mp3info->time)+1)*_frequency;
      _total_frames = _total_samples / _samples_per_frame;
//      _total_frames = mp3info->frames;
//      _total_samples = _total_frames * _samples_per_frame;
   } else { return false; }
   
   _fd.setName(wave_file.name());
   if(!_fd.open(IO_ReadOnly)) {
	   return false;
   }
   _fd.at(0);


   _data_length=wave_file.size();
   if(id3v1_tag) {
      _data_length-=128;
   }
   if(id3v2_tag) {
      _data_length-=id3v2_offset;
   }
   _data_start=id3v2_offset;

   short buf;
   if(!afread(&buf,1)){
      _fd.close();
      return false;
   }
   afseek(0, SEEK_SET);

   return (true);
}

int
RMpegFile::afread(
      signed short * p_data,
      int len
      )
{
   if(_fd.handle() == -1) return 0;
   long   new_length = 0;

   while(new_length < len){
      if (_bufpos == _buflen){
	 _buflen = MadDecodeFrame(_buffer,1);
	 _bufpos = 0;
	 if(_buflen == 0 )
	    break;
      }
      *(p_data + new_length++) = _buffer[_bufpos++];
   }

   if (new_length != len)
      _total_frames = _frame_count;

#ifdef RMPEGFILE_SHOW_SLOTS
   fprintf(stderr,"length requested = %d returned %d\n",len, _current_position);
#endif

   return(new_length);
}

int
RMpegFile::afread(
      float * p_data,
      int len
      )
{
   if(_fd.handle() == -1) return 0;
   long   new_length = 0;

   while(new_length < len){
      if (_bufpos == _buflen){
	 _buflen = MadDecodeFrame(_buffer,1);
	 _bufpos = 0;
	 if(_buflen == 0 )
	    break;
      }
      *(p_data + new_length++) = MadFixedToFloat(_buffer[_bufpos++]);
   }

   if (new_length != len)
      _total_frames = _frame_count;

#ifdef RMPEGFILE_SHOW_SLOTS
   fprintf(stderr,"length requested = %d returned %d\n",len, _current_position);
#endif

   return(new_length);
}

int
RMpegFile::afseek( int position, int whence)
{
   if(_fd.handle() == -1) return 0;
   position /= 4;
   switch(whence) {
      case SEEK_SET:
	 break;
      case SEEK_CUR:
	 position += _frame_count*_samples_per_frame + _bufpos;
	 break;
      case SEEK_END:
	 position = _total_frames * _samples_per_frame - position;
	 if(position < 0)
	    position = 0;
	 break;
   }
   // get remainder
   _bufpos = (position) % _samples_per_frame;
   int frame_no = (position - _bufpos)/_samples_per_frame;

   // IF we are beyond last frame then we are done
   if (frame_no > _total_frames)
   {
      return (_frame_count*_samples_per_frame + _bufpos) * 2;
   }
   else if(_frame_count != frame_no)
   {
      _frame_count = seekFrame(frame_no);
      _buflen = MadDecodeFrame(_buffer,1); 
   }
   if(_buflen < _bufpos){
      _bufpos = _buflen;
      position = _frame_count*_samples_per_frame + _bufpos;
   }
#ifdef RMPEGFILE_SHOW_SLOTS
   fprintf(stderr,"requested seek to: %d\n _frame_count: %d of %d total frames.\n _last_read_pos: %d\n _buflen: %d _bufpos: %d\n",position, _frame_count, _total_frames, _last_read_pos,_buflen,_bufpos);
#endif
   return position*4;
}

bool RMpegFile::IsMpeg(const char * file_name)
{
   QFile file(file_name);
   if(!afopen(file))
      return false;
   _fd.close();
   return true;
  // using namespace TagLib;

}


RDWaveFile::RDWaveFile(QString file_name)
{
  // 
  // Initialize Class Structures
  //
  wave_file.setName(file_name);
  wave_data=NULL;
  mpeg_file=NULL;
  recordable=false;
  format_chunk=false;
  format_tag=0;
  energy_tag=0;
  normalize_level=1.0;
  channels=0;
  samples_per_sec=0;
  avg_bytes_per_sec=0;
  block_align=0;
  bits_per_sample=0;
  cb_size=0;
  head_layer=0;
  head_bit_rate=0;
  head_mode=0;
  head_mode_ext=0;
  head_emphasis=1;
  head_flags=0;
  pts=0;
  mpeg_id=RDWaveFile::NonMpeg;
  mpeg_frame_size=0;
  id3v1_tag=false;
  id3v2_tag[0]=false;
  id3v2_tag[1]=false;
  id3v2_offset[0]=0;
  id3v2_offset[1]=0;
  has_energy=false;
  energy_loaded=false;
  energy_ptr=0;
  for(int i=0;i<FMT_CHUNK_SIZE;i++) {
    fmt_chunk_data[i]=0;
  }
  fmt_size=0;
  fact_chunk=false;
  sample_length=0;
  for(int i=0;i<FACT_CHUNK_SIZE;i++) {
    fact_chunk_data[i]=0;
  }
  data_chunk=false;
  data_length=0;
  cart_chunk=false;
  cart_version=0;
  cart_title="";
  cart_title="";
  cart_artist="";
  cart_cut_id="";
  cart_client_id="";
  cart_category="";
  cart_classification="";
  cart_out_cue="";
  cart_start_date=QDate::currentDate();
  cart_start_time=QTime::currentTime();
  cart_end_date=QDate(CART_DEFAULT_END_YEAR,
		      CART_DEFAULT_END_MONTH,
		      CART_DEFAULT_END_DAY);
  cart_end_time=QTime(CART_DEFAULT_END_HOUR,
		      CART_DEFAULT_END_MINUTE,
		      CART_DEFAULT_END_SECOND);
  cart_producer_app_id="";
  cart_producer_app_ver="";
  cart_user_def="";
  cart_url="";
  cart_tag_text="";
  cart_level_ref=CART_DEFAULT_LEVEL_REF;
  for(int i=0;i<MAX_TIMERS;i++) {
    cart_timer_label[i]=QString("");
    cart_timer_sample[i]=0;
  }
  for(int i=0;i<CART_CHUNK_SIZE;i++) {
    cart_chunk_data[i]=0;
  }
  bext_chunk=false;
  bext_description="";
  bext_originator="";
  bext_originator_ref="";
  bext_origination_date=QDate::currentDate();
  bext_origination_time=QTime::currentTime();
  bext_time_reference_low=0;
  bext_time_reference_high=0;
  bext_version=0;
  for(int i=0;i<64;i++) {
    bext_umid[i]=0;
  }
  bext_coding_history="";
  for(int i=0;i<BEXT_CHUNK_SIZE;i++) {
    bext_chunk_data[i]=0;
  }
  bext_coding_data=NULL;
  bext_coding_size=0;
  mext_chunk=false;
  mext_homogenous=true;
  mext_padding_used=false;
  mext_rate_hacked=false;
  mext_free_format=false;
  mext_frame_size=0;
  mext_anc_length=0;
  mext_left_energy=false;
  mext_ancillary_private=false;
  mext_right_energy=false;
  for(int i=0;i<MEXT_CHUNK_SIZE;i++) {
    mext_chunk_data[i]=0;
  }
  levl_chunk=false;
  levl_format=DEFAULT_LEVL_FORMAT; 
  levl_points=DEFAULT_LEVL_POINTS;
  levl_block_size=DEFAULT_LEVL_BLOCK_SIZE;
  cook_buffer=NULL;
  cook_buffer_size=0;
  cook_encoding=RDWaveFile::Raw;
  wave_type=RDWaveFile::Unknown;
  encode_quality=5.0f;
  serial_number=-1;
  atx_offset=0;
  scot_chunk=false;
  av10_chunk=false;
}


RDWaveFile::~RDWaveFile()
{
  if(bext_coding_data!=NULL) {
    free(bext_coding_data);
  }
  if(cook_buffer!=NULL) {
    free(cook_buffer);
  }
  if(mpeg_file!=NULL) {
    delete mpeg_file;
  }
  mpeg_file = NULL;
}


RDWaveFile::Type RDWaveFile::type() const
{
  return wave_type;
}


void RDWaveFile::nameWave(QString file_name)
{
  if(wave_file.isOpen()) {
    return;
  }
  wave_file.setName(file_name);
}



bool RDWaveFile::openWave(RDWaveData *data)
{
#ifdef HAVE_VORBIS
  vorbis_info *vorbis_info;
#endif  // HAVE_VORBIS
  unsigned char tmc_buffer[4];

  wave_data=data;
  if(!wave_file.open(IO_ReadOnly)) {
    return false;
  }
  switch(GetType(wave_file.handle())) {
      case RDWaveFile::Wave:
	if(GetFmt(wave_file.handle())) {
	  wave_type=RDWaveFile::Wave;
	}
	else {
	  wave_type=RDWaveFile::Ambos;
	  format_tag=WAVE_FORMAT_MPEG;
	}
	if(!GetChunk(wave_file.handle(),"data",&data_length,NULL,0)) { // Set fileptr to start of data block
	  return false;
	}
	data_chunk=true;
	data_start=lseek(wave_file.handle(),0,SEEK_CUR);
	if(format_tag==WAVE_FORMAT_MPEG) {
              mpeg_file = new RMpegFile();
              if(!mpeg_file->afopen(wave_file)) {
                 delete mpeg_file;
                 mpeg_file=NULL;
                 return false;
              }
  
              data_length=mpeg_file->getDataLength();
              data_start=mpeg_file->getDataStart();
   
              bits_per_sample=16;
              sample_length=mpeg_file->getTotalSamples();
              ext_time_length=
                 (unsigned)(1000.0*(double)sample_length/(double)samples_per_sec);
              time_length=ext_time_length/1000;
              data_chunk=true;
              lseek(wave_file.handle(),data_start,SEEK_SET);
              format_chunk=true;
   	      ReadNormalizeLevel(wave_file.name());
              wave_type=RDWaveFile::Mpeg;
              block_align=2*channels;
              ReadId3Metadata();
        }
	if((!GetFact(wave_file.handle()))||(sample_length==0)) {
	  if((format_tag!=WAVE_FORMAT_PCM) && format_chunk) {
#ifdef MPEG_FACT_FUDGE
	    // Guesstimate the overall sample size
	    time_length=data_length/avg_bytes_per_sec;
	    sample_length=time_length*samples_per_sec;
	    ext_time_length=(unsigned)(1000.0*(double)time_length/
				       (double)samples_per_sec);
#else
	    time_length=0;
	    sample_length=0;
	    ext_time_length=0;
#endif
	  }
	  else {
	    if(format_chunk) {
	      ext_time_length=(unsigned)(1000.0*(double)data_length/
					 (double)(block_align*samples_per_sec));
	      time_length=ext_time_length/1000;
	      sample_length=data_length/block_align;
	    }
	    else {
	      if(!GetMpegHeader(wave_file.handle(),data_start)) {
		wave_file.close();
		return false;
	      }
	      data_length=wave_file.size()-data_start;
	      sample_length=1152*(data_length/mpeg_frame_size);
	      ext_time_length=(unsigned)(1000.0*(double)sample_length/
					 (double)samples_per_sec);
	      time_length=ext_time_length/1000;
	      lseek(wave_file.handle(),data_start,SEEK_SET);
	      format_chunk=true;
	    }
	  }
	}
	else {
	  if(format_chunk) {
	    time_length=sample_length/samples_per_sec;
	    ext_time_length=(unsigned)(1000.0*(double)sample_length/
				       (double)samples_per_sec);
	  }
	  else {
	    time_length=0;
	    ext_time_length=0;
	  }
	}
	GetCart(wave_file.handle());
	GetBext(wave_file.handle());
	GetMext(wave_file.handle());
//	GetLevl(wave_file.handle());
	GetList(wave_file.handle());
	GetScot(wave_file.handle());
	GetAv10(wave_file.handle());
	break;

      case RDWaveFile::Mpeg:
	format_tag=WAVE_FORMAT_MPEG;
	if(!GetMpegHeader(wave_file.handle(),id3v2_offset[0])) {
	  wave_file.close();
	  return false;
	}
        mpeg_file = new RMpegFile();
        if(mpeg_file->afopen(wave_file) == false) {
            delete mpeg_file;
            mpeg_file=NULL;
            return false;
        } 
    
        data_length=mpeg_file->getDataLength();
        data_start=mpeg_file->getDataStart();
   
        bits_per_sample=16;
        sample_length=mpeg_file->getTotalSamples();
     	ext_time_length=
   	       (unsigned)(1000.0*(double)sample_length/(double)samples_per_sec);
   	time_length=ext_time_length/1000;
   	data_chunk=true;
   	lseek(wave_file.handle(),data_start,SEEK_SET);
   	format_chunk=true;
      	ReadNormalizeLevel(wave_file.name());
   	wave_type=RDWaveFile::Mpeg;
        block_align=2*channels;
   	ReadId3Metadata();
    	break;

      case RDWaveFile::Ogg:
#ifdef HAVE_VORBIS
	format_tag=WAVE_FORMAT_VORBIS;
	avg_bytes_per_sec=ov_bitrate(&vorbis_file,-1)/8;
	vorbis_info=ov_info(&vorbis_file,-1);
	channels=vorbis_info->channels;
	block_align=2*channels;
	samples_per_sec=vorbis_info->rate;
	avg_bytes_per_sec=block_align*samples_per_sec;
	bits_per_sample=16;
	data_start=0;
	sample_length=ov_pcm_total(&vorbis_file,-1);
	data_length=sample_length*2*channels;
	ext_time_length=(unsigned)(ov_time_total(&vorbis_file,-1)*1000.0);
	time_length=(unsigned)ov_time_total(&vorbis_file,-1);
	data_chunk=true;
	format_chunk=true;
        ReadNormalizeLevel(wave_file.name());
	wave_type=RDWaveFile::Ogg;
	ReadOggMetadata();
	return true;
#else
	return false;
#endif  // HAVE_VORBIS
	break;

      case RDWaveFile::Atx:
	format_tag=WAVE_FORMAT_MPEG;
	atx_offset=GetAtxOffset(wave_file.handle());
	if(!GetMpegHeader(wave_file.handle(),atx_offset)) {
	  wave_file.close();
	  return false;
	}
	data_length=wave_file.size()-atx_offset;
	data_start=atx_offset;
	sample_length=1152*(data_length/mpeg_frame_size);
	ext_time_length=
	  (unsigned)(1000.0*(double)sample_length/(double)samples_per_sec);
	time_length=ext_time_length/1000;
	data_chunk=true;
	lseek(wave_file.handle(),data_start,SEEK_SET);
	format_chunk=true;
	wave_type=RDWaveFile::Atx;	
	break;

      case RDWaveFile::Tmc:
	format_tag=WAVE_FORMAT_MPEG;
	atx_offset=4;
	if(!GetMpegHeader(wave_file.handle(),atx_offset)) {
	  wave_file.close();
	  return false;
	}
	lseek(wave_file.handle(),0,SEEK_SET);
	read(wave_file.handle(),tmc_buffer,4);
	data_length=(0xFF&tmc_buffer[0])+(0xFF&tmc_buffer[1])*256+
	  (0xFF&tmc_buffer[2])*65536+(0xFF&tmc_buffer[3])*16777216;
	data_start=atx_offset;
	sample_length=1152*(data_length/mpeg_frame_size);
	ext_time_length=
	  (unsigned)(1000.0*(double)sample_length/(double)samples_per_sec);
	time_length=ext_time_length/1000;
	data_chunk=true;
	lseek(wave_file.handle(),data_start,SEEK_SET);
	format_chunk=true;
	wave_type=RDWaveFile::Tmc;
	ReadTmcMetadata(wave_file.handle());
	break;

      case RDWaveFile::Flac:
	format_tag=WAVE_FORMAT_FLAC;
	if(!GetFlacStreamInfo()) {
	  wave_file.close();
	  return false;
	}
	wave_type=RDWaveFile::Flac;
	format_chunk=true;
	ReadId3Metadata();
	ReadFlacMetadata();
	break;

      default:
	close(wave_file.handle());
	return false;
	break;
  }
  lseek(wave_file.handle(),data_start,SEEK_SET);
  ValidateMetadata();

  return true;
}


bool RDWaveFile::createWave(RDWaveData *data)
{
  QFile energy_file;
  QString str;
  mode_t prev_mask;
  bool rc;
  wave_data=data;
  unlink((wave_file.name()+".energy").ascii());
  if(wave_data!=NULL) {
    cart_title=wave_data->title();
    cart_artist=wave_data->artist();
    cart_cut_id=wave_data->cutId();
    cart_client_id=wave_data->client();
    cart_category=wave_data->category();
    cart_classification=wave_data->classification();
    cart_out_cue=wave_data->outCue();
    cart_start_date=wave_data->startDate();
    cart_start_time=wave_data->startTime();
    cart_end_date=wave_data->endDate();
    cart_end_time=wave_data->endTime();
    cart_user_def=wave_data->userDefined();
    cart_url=wave_data->url();
    cart_tag_text=wave_data->tagText();
    bext_description=wave_data->description();
    bext_originator=wave_data->originator();
    bext_originator_ref=wave_data->originatorReference();
    bext_coding_history=wave_data->codingHistory();
  }

  switch(format_tag) {
      case WAVE_FORMAT_PCM:
      case WAVE_FORMAT_MPEG:
	levl_block_ptr=0;
	levl_istate=0;
	levl_accum=0;
	energy_data.clear();
	for(int i=0;i<channels;i++) {
	  energy_data.push_back(0);
	}
	if(!MakeFmt()) {
	  return false;
	}
        prev_mask = umask(0113);      // Set umask so files are user and group writable.
        rc=wave_file.open(IO_ReadWrite|IO_Truncate);
        umask(prev_mask);
	if(rc==false) {
	  return false;
	}
	recordable=true;
	write(wave_file.handle(),"RIFF\0\0\0\0WAVE",12);
	WriteChunk(wave_file.handle(),"fmt ",fmt_chunk_data,fmt_size);
	if(format_tag==WAVE_FORMAT_MPEG) {
	  write(wave_file.handle(),"fact\4\0\0\0\0\0\0\0",12);
	}
	if(cart_chunk) {
	  MakeCart();
	  WriteChunk(wave_file.handle(),"cart",cart_chunk_data,
		     CART_CHUNK_SIZE);
	}
	if(bext_chunk) {
	  MakeBext();
	  WriteChunk(wave_file.handle(),"bext",bext_coding_data,
		     bext_coding_size);
	}
	if(mext_chunk) {
	  MakeMext();
	  WriteChunk(wave_file.handle(),"mext",mext_chunk_data,
		     MEXT_CHUNK_SIZE);
	}
	wave_type=RDWaveFile::Wave;
	write(wave_file.handle(),"data\0\0\0\0",8);
	data_start=lseek(wave_file.handle(),0,SEEK_CUR);
	break;

      case WAVE_FORMAT_VORBIS:
#ifdef HAVE_VORBIS
	energy_data.clear();
	for(int i=0;i<channels;i++) {
	  energy_data.push_back(0);
	}
	avg_bytes_per_sec=2*channels*samples_per_sec;
	vorbis_info_init(&vorbis_inf);
	if(vorbis_encode_init_vbr(&vorbis_inf,channels,samples_per_sec,
				  encode_quality)<0) {
	  vorbis_info_clear(&vorbis_inf);
	  return false;
	}
	vorbis_encode_ctl(&vorbis_inf,OV_ECTL_RATEMANAGE_SET,NULL);

        prev_mask = umask(0113);      // Set umask so files are user and group writable.
	    rc=wave_file.open(IO_ReadWrite|IO_Truncate);
        umask(prev_mask);
	if(rc==false) {
	  vorbis_info_clear(&vorbis_inf);
	  return false;
	}
	wave_type=RDWaveFile::Ogg;
	recordable=true;
	vorbis_encode_setup_init(&vorbis_inf);
	vorbis_analysis_init(&vorbis_dsp,&vorbis_inf);
	vorbis_block_init(&vorbis_dsp,&vorbis_blk);
	if(serial_number<0) {  // Generate random serial number
	  srand(time(NULL));
	  serial_number=abs(rand());
	}
	ogg_stream_init(&ogg_stream,serial_number);
	{
	  ogg_packet header_main;
	  ogg_packet header_comments;
	  ogg_packet header_codebooks;
	  vorbis_comment vorbis_comm;
	  vorbis_comment_init(&vorbis_comm);
	  
	  vorbis_analysis_headerout(&vorbis_dsp,&vorbis_comm,&header_main,
				    &header_comments,&header_codebooks);
	  ogg_stream_packetin(&ogg_stream,&header_main);
	  ogg_stream_packetin(&ogg_stream,&header_comments);
	  ogg_stream_packetin(&ogg_stream,&header_codebooks);
	  while(ogg_stream_flush(&ogg_stream,&ogg_pg)) {
	    WriteOggPage(&ogg_pg);
	  }
	}
	return true;
#endif  // HAVE_VORBIS
	return false;
	break;
  }
  levl_timestamp=QDateTime(QDate::currentDate(),QTime::currentTime());
  data_length=0;
  if(energy_tag==1)
    {
     energy_file.setName(wave_file.name()+".energy");
     prev_mask = umask(0113);      // Set umask so files are user and group writable.
     rc=energy_file.open(IO_ReadWrite|IO_Truncate);
     umask(prev_mask);
     energy_file.close();
    }
  return true;
}

bool RDWaveFile::recreateEnergy()
{
has_energy=false;
energy_loaded=false;
unlink(wave_file.name()+".energy");
GetEnergy();
return has_energy;
}

void RDWaveFile::closeWave(int samples)
{
  unsigned char size_buf[4];
  unsigned csize;
  unsigned lsize=0;
  unsigned cptr;
  QFile energy_file;
  QString str;
  mode_t prev_mask;
  bool rc;

  if(energy_tag==1) {
      str=str.sprintf("%f\n",normalize_level);
      energy_file.setName(wave_file.name()+".energy");
      prev_mask = umask(0113);      // Set umask so files are user and group writable.
      rc=energy_file.open(IO_ReadWrite|IO_Truncate);
      umask(prev_mask);
      if(rc) {
         write(energy_file.handle(),(const char*)str,strlen(str));
         write(energy_file.handle(),&(energy_data[0]),2*energy_data.size());
         energy_file.close();
      }
   }

  if(recordable) {
    switch(wave_type) {
	case RDWaveFile::Wave:
	  //
	  // Write levl chunk
	  //
	  if(energy_tag==0&&levl_chunk&&((format_tag==WAVE_FORMAT_PCM)||
			  ((format_tag==WAVE_FORMAT_MPEG)&&(head_layer==2)))) {
	    levl_version=0;
	    levl_format=2;
	    levl_points=1;
	    levl_block_size=1152;
	    levl_channels=channels;
	    levl_frames=energy_data.size()/channels;
	    levl_peak_offset=0xFFFFFFFF;
	    levl_peak_value=0;
	    MakeLevl();
	    lseek(wave_file.handle(),0,SEEK_END);
	    write(wave_file.handle(),"levl",4);
	    lsize=LEVL_CHUNK_SIZE+energy_data.size()*2;
	    size_buf[0]=lsize&0xff;
	    size_buf[1]=(lsize>>8)&0xff;
	    size_buf[2]=(lsize>>16)&0xff;
	    size_buf[3]=(lsize>>24)&0xff;
	    write(wave_file.handle(),size_buf,4);
	    write(wave_file.handle(),levl_chunk_data,LEVL_CHUNK_SIZE-4);
	    write(wave_file.handle(),&(energy_data[0]),2*energy_data.size());
	    ftruncate(wave_file.handle(),lseek(wave_file.handle(),0,SEEK_CUR));
	  }

	  //
	  // Update file size
	  //
	  cptr=lseek(wave_file.handle(),0,SEEK_END)-12;
	  size_buf[0]=cptr&0xff;
	  size_buf[1]=(cptr>>8)&0xff;
	  size_buf[2]=(cptr>>16)&0xff;
	  size_buf[3]=(cptr>>24)&0xff;
	  lseek(wave_file.handle(),4,SEEK_SET);
	  write(wave_file.handle(),size_buf,4);
	  
	  //
	  // Update data chunk size
	  //
	  size_buf[0]=data_length&0xff;
	  size_buf[1]=(data_length>>8)&0xff;
	  size_buf[2]=(data_length>>16)&0xff;
	  size_buf[3]=(data_length>>24)&0xff;
	  lseek(wave_file.handle(),
		FindChunk(wave_file.handle(),"data",&csize)-4,SEEK_SET);
	  write(wave_file.handle(),size_buf,4);
	  
	  //
	  // Update fact chunk
	  //
	  if(FindChunk(wave_file.handle(),"fact",&csize)>0) {
	    if(samples<0) {
	      if(format_tag==WAVE_FORMAT_PCM) {
		samples=data_length/block_align;
	      }
	      if(format_tag==WAVE_FORMAT_MPEG) {
		samples=(unsigned)(1152.0*((double)data_length/
					   (144.0*(double)head_bit_rate/
					    (double)samples_per_sec)));
	      }
	    }
	    size_buf[0]=samples&0xff;
	    size_buf[1]=(samples>>8)&0xff;
	    size_buf[2]=(samples>>16)&0xff;
	    size_buf[3]=(samples>>24)&0xff;
	    WriteChunk(wave_file.handle(),"fact",size_buf,4);
	  }
	  
	  //
	  // Update Cart Chunk
	  //
	  if(cart_chunk) {
	    MakeCart();
	    WriteChunk(wave_file.handle(),"cart",cart_chunk_data,CART_CHUNK_SIZE);
	  }
	  
	  //
	  // Update Bext Chunk
	  //
	  if(bext_chunk) {
	    MakeBext();
	    WriteChunk(wave_file.handle(),"bext",bext_coding_data,bext_coding_size);
	  }
	  
	  //
	  // Update Mext Chunk
	  //
	  if(mext_chunk) {
	    MakeMext();
	    WriteChunk(wave_file.handle(),"mext",mext_chunk_data,MEXT_CHUNK_SIZE);
	  }
	  
	  //
	  // Truncate
	  //
	  if(!levl_chunk) {
	    ftruncate(wave_file.handle(),FindChunk(wave_file.handle(),
						   "data",&csize)+data_length);
	  }
	  else {
	    if((format_tag==WAVE_FORMAT_MPEG)&&(head_layer!=2)) {
	      ftruncate(wave_file.handle(),
			FindChunk(wave_file.handle(),
				  "data",&csize)+data_length);
	    }
	  }
	  break;

	case RDWaveFile::Ogg:
#ifdef HAVE_VORBIS
	  WriteOggBuffer(NULL,0);
	  ogg_stream_clear(&ogg_stream);
	  vorbis_block_clear(&vorbis_blk);
	  vorbis_dsp_clear(&vorbis_dsp);
	  vorbis_info_clear(&vorbis_inf);
	  wave_file.close();
#endif  // HAVE_VORBIS
	  break;

	default:
	  break;
    }
  }
  if(wave_type==RDWaveFile::Ogg) {
#ifdef HAVE_VORBIS
    if(!recordable) {
      ov_clear(&vorbis_file);
    }
#endif  // HAVE_VORBIS
  }
  wave_file.close();
  recordable=false;
  time_length=0;
  format_chunk=false;
  format_tag=0;
  energy_tag=0;
  normalize_level=1.0;
  channels=0;
  samples_per_sec=0;
  avg_bytes_per_sec=0;
  block_align=0;
  bits_per_sample=0;
  cb_size=0;
  head_layer=0;
  head_bit_rate=0;
  head_mode=0;
  head_mode_ext=0;
  head_emphasis=1;
  head_flags=0;
  pts=0;
  mpeg_id=RDWaveFile::NonMpeg;
  mpeg_frame_size=0;
  id3v1_tag=false;
  id3v2_tag[0]=false;
  id3v2_tag[1]=false;
  id3v2_offset[0]=0;
  id3v2_offset[1]=0;
  for(int i=0;i<FMT_CHUNK_SIZE;i++) {
    fmt_chunk_data[i]=0;
  }
  fmt_size=0;
  fact_chunk=false;
  for(int i=0;i<FACT_CHUNK_SIZE;i++) {
    fact_chunk_data[i]=0;
  }
  sample_length=0;
  data_chunk=false;
  data_length=0;
  cart_chunk=false;
  cart_version=0;
  cart_title="";
  cart_title="";
  cart_artist="";
  cart_cut_id="";
  cart_client_id="";
  cart_category="";
  cart_classification="";
  cart_out_cue="";
  cart_start_date=QDate::currentDate();
  cart_start_time=QTime::currentTime();
  cart_end_date=QDate(CART_DEFAULT_END_YEAR,
		      CART_DEFAULT_END_MONTH,
		      CART_DEFAULT_END_DAY);
  cart_end_time=QTime(CART_DEFAULT_END_HOUR,
		      CART_DEFAULT_END_MINUTE,
		      CART_DEFAULT_END_SECOND);
  cart_producer_app_id="";
  cart_producer_app_ver="";
  cart_user_def="";
  cart_url="";
  cart_tag_text=""; 
  cart_level_ref=CART_DEFAULT_LEVEL_REF;
  for(int i=0;i<CART_CHUNK_SIZE;i++) {
    cart_chunk_data[i]=0;
  }
  bext_chunk=false;
  bext_description="";
  bext_originator="";
  bext_originator_ref="";
  bext_origination_date=QDate::currentDate();
  bext_origination_time=QTime::currentTime();
  bext_time_reference_low=0;
  bext_time_reference_high=0;
  bext_version=0;
  for(int i=0;i<64;i++) {
    bext_umid[i]=0;
  }
  bext_coding_history="";
  for(int i=0;i<BEXT_CHUNK_SIZE;i++) {
    bext_chunk_data[i]=0;
  }
  free(bext_coding_data);
  bext_coding_data=NULL;
  bext_coding_size=0;
  mext_chunk=false;
  mext_homogenous=true;
  mext_padding_used=false;
  mext_rate_hacked=false;
  mext_free_format=false;
  mext_frame_size=0;
  mext_anc_length=0;
  mext_left_energy=false;
  mext_ancillary_private=false;
  mext_right_energy=false;
  for(int i=0;i<MEXT_CHUNK_SIZE;i++) {
    mext_chunk_data[i]=0;
  }
  levl_chunk=false;
  levl_format=DEFAULT_LEVL_FORMAT; 
  levl_points=DEFAULT_LEVL_POINTS;
  levl_block_size=DEFAULT_LEVL_BLOCK_SIZE;
  energy_loaded=false;
  energy_data.clear();
  free(cook_buffer);
  cook_buffer=NULL;
  cook_buffer_size=0;
  cook_encoding=RDWaveFile::Raw;
  encode_quality=5.0f;
  serial_number=-1;
  atx_offset=0;
  if(mpeg_file!=NULL) {
    delete mpeg_file;
  }
  mpeg_file = NULL;
  av10_chunk=false;
}


void RDWaveFile::resetWave()
{
  if(wave_type!=RDWaveFile::Ogg) {
    lseek(wave_file.handle(),data_start,SEEK_SET);
    ftruncate(wave_file.handle(),data_start);
  }
  if(mpeg_file!=NULL) {
    delete mpeg_file;
  }
  mpeg_file = NULL;
}


QString RDWaveFile::getName() const
{
  return wave_file.name();
}


bool RDWaveFile::getFormatChunk() const
{
  return format_chunk;
}



bool RDWaveFile::getFactChunk() const
{
  return fact_chunk;
}



unsigned RDWaveFile::getSampleLength() const
{
  return sample_length;
}



unsigned RDWaveFile::getTimeLength() const
{
  return time_length;
}


unsigned RDWaveFile::getExtTimeLength() const
{
  return ext_time_length;
}


bool RDWaveFile::getDataChunk() const
{
  return data_chunk;
}



unsigned RDWaveFile::getDataLength() const
{
  return data_length;
}


RDWaveFile::Encoding RDWaveFile::encoding() const
{
  return cook_encoding;
}


void RDWaveFile::setEncoding(RDWaveFile::Encoding code)
{
  cook_encoding=code;
}


int RDWaveFile::readWave(void *buf,int count)
{
  int stream;
  int n=0;
  unsigned int pos;
  int16_t *sample;

  switch(wave_type) {
      case RDWaveFile::Ogg:
#ifdef HAVE_VORBIS
	      n = 0;
	      while(n!=count){
		    int ret = ov_read(&vorbis_file,(char *)buf+n,count-n,0,2,1,&stream);
		    if (!ret) break;
		    n+=ret;
	      }
           if(normalize_level != 1.0f){
	      for (int i=0;i<n/2;i++) {
             sample=(int16_t *)buf+i;
            *sample=(int16_t)(normalize_level*(double)*sample);
	      }
           }
	return n;
#endif  // HAVE_VORBIS
	return 0;

      case RDWaveFile::Wave:
        switch(format_tag){
           case WAVE_FORMAT_PCM:
	pos = lseek(wave_file.handle(),0,SEEK_CUR);
	//
	// FIXME: how fix comparing singed (data_start, count) vs. 
	// unsigned (pos, data_length) ... WAVE standard is 32 bit, 
	// but not sure if signed or not... 
	// grauf@rfa.org Tue, 04 Apr 2006 21:02:51 -0400
	//
        if (((pos+count)>(data_start+data_length))&&(data_length>0)) {
          count=count - ( (pos+count) - (data_start+data_length) );
        }
	if(normalize_level != 1.0f) {
           n=read(wave_file.handle(),buf,count); 
           for (int i=0;i<n/2;i++) {
              sample=(int16_t *)buf+i;
              *sample=(int16_t)(normalize_level*(double)*sample);
  	      }
	  return n;
          }
        else {
	  return read(wave_file.handle(),buf,count);
          }
               break;
            case WAVE_FORMAT_MPEG:
               {
                  int16_t *sample;
                  n = mpeg_file->afread((short*)buf,count/2);
                  if(normalize_level != 1.0f){
                     for (int i=0;i<n;i++) {
                        sample=(int16_t *)buf+i;
                        *sample=(int16_t)(normalize_level*(double)*sample);
                     }
                  }
               }
               return n*2;
	break;
  }
  
       case RDWaveFile::Mpeg:
         {
            n = mpeg_file->afread((short*)buf,count/2);
            if(normalize_level != 1.0f){
               for (int i=0;i<n;i++) {
                  sample=(int16_t *)buf+i;
                  *sample=(int16_t)(normalize_level*(double)*sample);
               }
    }
  }
         return n*2;

      default:
	return read(wave_file.handle(),buf,count);
  }
  return 0;
}


int RDWaveFile::writeWave(void *buf,int count)
{
  if(!recordable) {
    return -1;
  }
  switch(format_tag) {
      case WAVE_FORMAT_PCM:
	if(levl_chunk) {
	  for(int i=0;i<count;i++) {
	    switch(levl_istate) {
		case 0:   // Left Channel, LSB
		  levl_accum=((char *)buf)[i]&0xff;
		  levl_istate=1;
		  break;

		case 1:   // Left Channel, MSB
		  levl_accum|=((((char *)buf)[i]&0xff)<<8);
		  switch(channels) {
		      case 1:
			if(levl_accum>energy_data[energy_data.size()-1]) {
			  energy_data[energy_data.size()-1]=levl_accum;
			}
			if(++levl_block_ptr==1152) {
			  energy_data.push_back(0);
			  levl_block_ptr=0;
			}
			levl_istate=0;
			break;

		      case 2:
			if(levl_accum>energy_data[energy_data.size()-2]) {
			  energy_data[energy_data.size()-2]=levl_accum;
			}
			levl_istate=2;
			break;
		  }
		  break;

		case 2:   // Right Channel, LSB
		  levl_accum=((char *)buf)[i]&0xff;
		  levl_istate=3;
		  break;

		case 3:   // Right Channel, MSB
		  levl_accum|=((((char *)buf)[i]&0xff)<<8);
		  if(levl_accum>energy_data[energy_data.size()-1]) {
		    energy_data[energy_data.size()-1]=levl_accum;
		  }
		  if(++levl_block_ptr==1152) {
		    energy_data.push_back(0);
		    energy_data.push_back(0);
		    levl_block_ptr=0;
		  }
		  levl_istate=0;
		  break;
	    }
	  }
	}
	lseek(wave_file.handle(),0,SEEK_END);
	data_length+=count;
	return write(wave_file.handle(),buf,count);

      case WAVE_FORMAT_MPEG:
	if(levl_chunk&&(head_layer==2)) {
	  for(int i=0;i<count;i++) {
	    if(levl_block_ptr==(block_align-5)) {  // Right Channel, MSB
	      if(channels==2) {
		levl_accum=((((char *)buf)[i]&0xff)<<8);
	      }
	      levl_block_ptr++;
	    }
	    else {
	      if(levl_block_ptr==(block_align-4)) {  // Right Channel, LSB
		if(channels==2) {
		  levl_accum|=((char *)buf)[i]&0xff;
		  energy_data[energy_data.size()-1]=levl_accum;
		}
		levl_block_ptr++;
	      }
	      else {
		if(levl_block_ptr==(block_align-2)) { // Left Channel, MSB
		  levl_accum=((((char *)buf)[i]&0xff)<<8);
		  levl_block_ptr++;
		}
		else {
		  if(levl_block_ptr==(block_align-1)) {  // Left Channel, LSB
		    levl_accum|=((char *)buf)[i]&0xff;
		    energy_data[energy_data.size()-channels]=levl_accum;
		    for(unsigned j=0;j<channels;j++) {
		      energy_data.push_back(0);
		    }
		    levl_block_ptr=0;
		  }
		  else {
		    levl_block_ptr++;
		  }
		}
	      }
	    }
	  }
	}
	lseek(wave_file.handle(),0,SEEK_END);
	data_length+=count;
	return write(wave_file.handle(),buf,count);

      case WAVE_FORMAT_VORBIS:
	  for(int i=0;i<count;i++) {
	    switch(levl_istate) {
		case 0:   // Left Channel, LSB
		  levl_accum=((char *)buf)[i]&0xff;
		  levl_istate=1;
		  break;

		case 1:   // Left Channel, MSB
		  levl_accum|=((((char *)buf)[i]&0xff)<<8);
		  switch(channels) {
		      case 1:
			if(levl_accum>energy_data[energy_data.size()-1]) {
			  energy_data[energy_data.size()-1]=levl_accum;
			}
			if(++levl_block_ptr==1152) {
			  energy_data.push_back(0);
			  levl_block_ptr=0;
			}
			levl_istate=0;
			break;

		      case 2:
			if(levl_accum>energy_data[energy_data.size()-2]) {
			  energy_data[energy_data.size()-2]=levl_accum;
			}
			levl_istate=2;
			break;
		  }
		  break;

		case 2:   // Right Channel, LSB
		  levl_accum=((char *)buf)[i]&0xff;
		  levl_istate=3;
		  break;

		case 3:   // Right Channel, MSB
		  levl_accum|=((((char *)buf)[i]&0xff)<<8);
		  if(levl_accum>energy_data[energy_data.size()-1]) {
		    energy_data[energy_data.size()-1]=levl_accum;
		  }
		  if(++levl_block_ptr==1152) {
		    energy_data.push_back(0);
		    energy_data.push_back(0);
		    levl_block_ptr=0;
		  }
		  levl_istate=0;
		  break;
	    }
	  }
	WriteOggBuffer((char *)buf,count);
	break;
  }
  return 0;
}


bool RDWaveFile::hasEnergy()
{
  GetEnergy();
  return has_energy;
}


unsigned RDWaveFile::energySize()
{
  GetEnergy();
  if(!has_energy) {
    return 0;
  }
  return energy_data.size();
}


unsigned short RDWaveFile::energy(unsigned frame)
{
  GetEnergy();
  if(!has_energy) {
    return 0;
  }
  return energy_data[frame];
}


int RDWaveFile::readEnergy(unsigned short buf[],int count)
{
  GetEnergy();
  if(!has_energy) {
    return 0;
  }
  for(int i=0;i<count;i++) {
    if((i+energy_ptr)<energy_data.size()) {
      buf[i]=energy_data[i+energy_ptr];
    }
    else {
      energy_ptr+=i;
      return i;
    }
  }
  return 0;
}


int RDWaveFile::startTrim(int level)
{
  double ratio=pow(10,-(double)level/2000.0)*32768.0;
  GetEnergy();
  for(unsigned i=0;i<energy_data.size();i++) {
    if((double)energy_data[i]>=ratio) {
      return i*1152/getChannels();
    }
  }
  return -1;
}


int RDWaveFile::endTrim(int level)
{
  double ratio=pow(10,-(double)level/2000.0)*32768.0;
  GetEnergy();
  for(int i=energy_data.size()-1;i>=0;i--) {
    if((double)energy_data[i]>=ratio) {
      return i*1152/getChannels();
    }
  }
  return -1;
}


void RDWaveFile::normalize(double level)
{
  GetEnergy();
        //
        // The way sox does this:
        // - Find the sample with the highest value.  
        // Should be looking at minimum values too and take the absolute value.
        double peak_level = 0;
        for(unsigned i=0;i<energy_data.size();i++) {
           if(energy_data[i] > peak_level) {
               peak_level = energy_data[i]; 
           }
        }
        peak_level=peak_level/normalize_level;
        // - Then we need a ratio to multiple all our samples by.  i.e.
        // vol_scale * sample will never be greater then SIGNED_SHORT_MAX.
        double vol_scale = 32768.0f/peak_level; 
        // - Then we want to allow for adjusting the volume so it's not so full on loud.  
        // Like set it 13db below the max.  This is where the user selectable level comes in.
        double scale_level=(level*vol_scale)/normalize_level;
        normalize_level = level*vol_scale;
        if( normalize_level==0){
           normalize_level=1.0f;
           scale_level=1.0f;
        }else{
           for(unsigned i=0;i<energy_data.size();i++) {
              energy_data[i] *= scale_level;
           }
        }

        // I Propose the normalize_level here is the same as sent to rd_import_file.
        // i.e. normal=pow(10.0,(double)(library_conf->ripperLevel())/20.0);
        // failing that change it to a int and set the normalize_level to ripperLevel() and 
        // keep the gory details of converting that to double within this library.
        energy_tag=1;
}

int RDWaveFile::seekWave(int offset,int whence)
{
  int pos;

  switch(wave_type) {
      case RDWaveFile::Ogg:
#ifdef HAVE_VORBIS
	switch(whence) {
	    case SEEK_SET:
	      if(ov_pcm_seek(&vorbis_file,offset/(2*channels))==0) {
	      //printf("RDWaveFile::seekWave() = %d\n",offset);
		return offset;
	      }
	      //printf("RDWaveFile::seekWave() = -1\n");
	      return -1;
	      break;

	    case SEEK_CUR:
	      pos=ov_pcm_tell(&vorbis_file)*2*channels;
	      if(offset==0) {
		return pos;
	      }
	      return seekWave(pos+offset,SEEK_SET);
	      break;

	    case SEEK_END:
	      break;
	}
#endif  // HAVE_VORBIS
	return -1;
	break;
      case RDWaveFile::Mpeg:
         if(mpeg_file) 
            return mpeg_file->afseek(offset,whence);
         return -1;
         break;

      case RDWaveFile::Wave:// FIXME: how fix comparing singed (data_start, offset, pos) vs. unsigned (data_length) ... WAVE standard is 32 bit, but not sure if signed or not... grauf@rfa.org Tue, 04 Apr 2006 21:02:51 -0400
        switch(whence) {
            case SEEK_SET:
              if (offset<0) {
                offset=0;
              }
              if (offset>data_length) {
                offset=data_length;
              }
              return lseek(wave_file.handle(),
                           offset+data_start,SEEK_SET)-data_start;
              break;
            case SEEK_CUR:
              pos = lseek(wave_file.handle(),0,SEEK_CUR);
              if ((pos+offset)<data_start) {
                offset=offset + (data_start - (pos+offset));
              }
              if ((pos+offset)>(data_start+data_length)) { 
                offset=offset - ((pos+offset) - (data_start+data_length));
              }
              return lseek(wave_file.handle(),offset,SEEK_CUR)-data_start;
              break;
            case SEEK_END:
              pos = lseek(wave_file.handle(),0,SEEK_END);
              if ((pos+offset)<data_start) {
                offset=offset + (data_start - (pos+offset));
              }
              if ((pos+offset)>(data_start+data_length)) {
                offset=offset - ((pos+offset) - (data_start+data_length));
              }
              return lseek(wave_file.handle(),offset,SEEK_END)-data_start;
              break;
        }
        break;

      default:
	switch(whence) {
	    case SEEK_SET:
	      return lseek(wave_file.handle(),
			   offset+data_start,SEEK_SET)-data_start;
	      break;
	    case SEEK_CUR:
	      return lseek(wave_file.handle(),offset,SEEK_CUR)-data_start;
	      break;
	    case SEEK_END:
	      return lseek(wave_file.handle(),offset,SEEK_END)-data_start;
	      break;
	}
  }
  return 0;
} 



unsigned short RDWaveFile::getFormatTag() const
{
  return format_tag;
}


void RDWaveFile::setFormatTag(unsigned short format)
{
  if(!recordable) {
    format_tag=format;
  }
}


void RDWaveFile::setEnergyTag(int format)
{
  energy_tag=format;
}


int RDWaveFile::getEnergyTag() const
{
  return energy_tag;
}


double RDWaveFile::getNormalizeLevel() const
{
  return normalize_level;
}


void RDWaveFile::setNormalizeLevel(double level)
{
  normalize_level=level;
}


unsigned short RDWaveFile::getChannels() const
{
  return channels;
}


void RDWaveFile::setChannels(unsigned short chan)
{
  if(!recordable) {
    channels=chan;
  }
}


unsigned RDWaveFile::getSamplesPerSec() const
{
  return samples_per_sec;
}


void RDWaveFile::setSamplesPerSec(unsigned rate)
{
  if(!recordable) {
    samples_per_sec=rate;
  }
}


unsigned RDWaveFile::getAvgBytesPerSec() const
{
  return avg_bytes_per_sec;
}



unsigned short RDWaveFile::getBlockAlign() const
{
  return block_align;
}


unsigned short RDWaveFile::getBitsPerSample() const
{
  return bits_per_sample;
}


void RDWaveFile::setBitsPerSample(unsigned short bits)
{
  if(!recordable) {
    bits_per_sample=bits;
  }
}


unsigned short RDWaveFile::getHeadLayer() const
{
  return head_layer;
}


void RDWaveFile::setHeadLayer(unsigned short layer)
{
  if(!recordable) {
    head_layer=layer;
  }
}


unsigned RDWaveFile::getHeadBitRate() const
{
  return head_bit_rate;
}


void RDWaveFile::setHeadBitRate(unsigned rate)
{
  if(!recordable) {
    head_bit_rate=rate;
  }
}


unsigned short RDWaveFile::getHeadMode() const
{
  return head_mode;
}


void RDWaveFile::setHeadMode(unsigned short mode)
{
  if(!recordable) {
    head_mode=mode;
  }
}


void RDWaveFile::setHeadFlags(unsigned short flags)
{
  if(!recordable) {
    head_flags=flags;
  }
}


unsigned RDWaveFile::getHeadModeExt() const
{
  return head_mode_ext;
}



unsigned RDWaveFile::getHeadEmphasis() const
{
  return head_emphasis;
}



unsigned short RDWaveFile::getHeadFlags() const
{
  return head_flags;
}



unsigned long RDWaveFile::getPTS() const
{
  return pts;
}


bool RDWaveFile::getCartChunk() const
{
  return cart_chunk;
}


void RDWaveFile::setCartChunk(bool state)
{
  if(!recordable) {
    cart_chunk=state;
  }
}


unsigned RDWaveFile::getCartVersion() const
{
  return cart_version;
}


QString RDWaveFile::getCartTitle() const
{
  return cart_title;
}


void RDWaveFile::setCartTitle(QString string)
{
  cart_title=string;
}


QString RDWaveFile::getCartArtist() const
{
  return cart_artist;
}


void RDWaveFile::setCartArtist(QString string)
{
  cart_artist=string;
}


QString RDWaveFile::getCartCutID() const
{
  return cart_cut_id;
}


void RDWaveFile::setCartCutID(QString string)
{
  cart_cut_id=string;
}


QString RDWaveFile::getCartClientID() const
{
  return cart_client_id;
}


void RDWaveFile::setCartClientID(QString string)
{
  cart_client_id=string;
}


QString RDWaveFile::getCartCategory() const
{
  return cart_category;
}


void RDWaveFile::setCartCategory(QString string)
{
  cart_category=string;
}


QString RDWaveFile::getCartClassification() const
{
  return cart_classification;
}


void RDWaveFile::setCartClassification(QString string)
{
  cart_classification=string;
}


QString RDWaveFile::getCartOutCue() const
{
  return cart_out_cue;
}


void RDWaveFile::setCartOutCue(QString string)
{
  cart_out_cue=string;
}


QDate RDWaveFile::getCartStartDate() const
{
  return cart_start_date;
}


void RDWaveFile::setCartStartDate(QDate date)
{
  cart_start_date=date;
}


QTime RDWaveFile::getCartStartTime() const
{
  return cart_start_time;
}


void RDWaveFile::setCartStartTime(QTime time)
{
  cart_start_time=time;
}


QDate RDWaveFile::getCartEndDate() const
{
  return cart_end_date;
}


void RDWaveFile::setCartEndDate(QDate date)
{
  cart_end_date=date;
}


QTime RDWaveFile::getCartEndTime() const
{
  return cart_end_time;
}


void RDWaveFile::setCartEndTime(QTime time)
{
  cart_end_time=time;
}


QString RDWaveFile::getCartProducerAppID() const
{
  return cart_producer_app_id;
}


QString RDWaveFile::getCartProducerAppVer() const
{
  return cart_producer_app_ver;
}


QString RDWaveFile::getCartUserDef() const
{
  return cart_user_def;
}


void RDWaveFile::setCartUserDef(QString string)
{
  cart_user_def=string;
}


unsigned RDWaveFile::getCartLevelRef() const
{
  return cart_level_ref;
}


void RDWaveFile::setCartLevelRef(unsigned level)
{
  cart_level_ref=level;
}


QString RDWaveFile::getCartTimerLabel(int index) const
{
  if(index<MAX_TIMERS) {
    return cart_timer_label[index];
  }
  return QString("");
}


void RDWaveFile::setCartTimerLabel(int index,QString label)
{
  if(index<MAX_TIMERS) {
    cart_timer_label[index]=label;
  }
}


unsigned RDWaveFile::getCartTimerSample(int index) const
{
  if(index<MAX_TIMERS) {
    return cart_timer_sample[index];
  }
  return 0;
}


void RDWaveFile::setCartTimerSample(int index,unsigned sample)
{
  if(index<MAX_TIMERS) {
    cart_timer_sample[index]=sample;
  }
}


QString RDWaveFile::getCartURL() const
{
  return cart_url;
}


void RDWaveFile::setCartURL(QString string)
{
  cart_url=string;
}


QString RDWaveFile::getCartTagText() const
{
  return cart_tag_text;
}


bool RDWaveFile::getBextChunk() const
{
  return bext_chunk;
}


void RDWaveFile::setBextChunk(bool state) 
{
  if(!recordable) {
    bext_chunk=state;
  }
}


QString RDWaveFile::getBextDescription() const
{
  return bext_description;
}


void RDWaveFile::setBextDescription(QString string)
{
  bext_description=string;
}


QString RDWaveFile::getBextOriginator() const
{
  return bext_originator;
}


void RDWaveFile::setBextOriginator(QString string)
{
  bext_originator=string;
}


QString RDWaveFile::getBextOriginatorRef() const
{
  return bext_originator_ref;
}


void RDWaveFile::setBextOriginatorRef(QString string) 
{
  bext_originator_ref=string;
}


QDate RDWaveFile::getBextOriginationDate() const
{
  return bext_origination_date;
}


void RDWaveFile::setBextOriginationDate(QDate date)
{
  bext_origination_date=date;
}


QTime RDWaveFile::getBextOriginationTime() const
{
  return bext_origination_time;
}


void RDWaveFile::setBextOriginationTime(QTime time)
{
  bext_origination_time=time;
}


unsigned RDWaveFile::getBextTimeReferenceLow() const
{
  return bext_time_reference_low;
}


void RDWaveFile::setBextTimeReferenceLow(unsigned sample)
{
  bext_time_reference_low=sample;
}


unsigned RDWaveFile::getBextTimeReferenceHigh() const
{
  return bext_time_reference_low;
}


void RDWaveFile::setBextTimeReferenceHigh(unsigned sample)
{
  bext_time_reference_high=sample;
}


unsigned short RDWaveFile::getBextVersion() const
{
  return bext_version;
}


void RDWaveFile::getBextUMD(unsigned char *buf) const
{
  for(int i=0;i<64;i++) {
    buf[i]=bext_umid[i];
  }
}


void RDWaveFile::setBextUMD(unsigned char *buf)
{
  for(int i=0;i<64;i++) {
    bext_umid[i]=buf[i];
  }
}


QString RDWaveFile::getBextCodingHistory() const
{
  return bext_coding_history;
}


void RDWaveFile::setBextCodingHistory(QString string)
{
  if(!recordable) {
    bext_coding_history=string;
  }
}


bool RDWaveFile::getMextChunk() const
{
  return mext_chunk;
}


void RDWaveFile::setMextChunk(bool state)
{
  if(!recordable) {
    mext_chunk=state;
  }
}


bool RDWaveFile::getMextHomogenous() const
{
  return mext_homogenous;
}


void RDWaveFile::setMextHomogenous(bool state)
{
  mext_homogenous=state;
}


bool RDWaveFile::getMextPaddingUsed() const
{
  return mext_padding_used;
}


void RDWaveFile::setMextPaddingUsed(bool state)
{
  mext_padding_used=state;
}


bool RDWaveFile::getMextHackedBitRate() const
{
  return mext_rate_hacked;
}


void RDWaveFile::setMextHackedBitRate(bool state)
{
  mext_rate_hacked=state;
}


bool RDWaveFile::getMextFreeFormat() const
{
  return mext_free_format;
}


void RDWaveFile::setMextFreeFormat(bool state)
{
  mext_free_format=state;
}


int RDWaveFile::getMextFrameSize() const
{
  return mext_frame_size;
}


void RDWaveFile::setMextFrameSize(int size)
{
  mext_frame_size=size;
}


int RDWaveFile::getMextAncillaryLength() const
{
  return mext_anc_length;
}


void RDWaveFile::setMextAncillaryLength(int length)
{
  mext_anc_length=length;
}


bool RDWaveFile::getMextLeftEnergyPresent() const
{
  return mext_left_energy;
}


void RDWaveFile::setMextLeftEnergyPresent(bool state)
{
  mext_left_energy=state;
}


bool RDWaveFile::getMextPrivateDataPresent() const
{
  return mext_ancillary_private;
}


void RDWaveFile::setMextPrivateDataPresent(bool state)
{
  mext_ancillary_private=state;
}


bool RDWaveFile::getMextRightEnergyPresent() const
{
  return mext_right_energy;
}


void RDWaveFile::setMextRightEnergyPresent(bool state)
{
  mext_right_energy=state;
}


bool RDWaveFile::getLevlChunk() const
{
  return levl_chunk;
}


void RDWaveFile::setLevlChunk(bool state)
{
  levl_chunk=state;
}


int RDWaveFile::getLevlVersion() const
{
  return levl_version;
}


void RDWaveFile::setLevlVersion(unsigned ver)
{
  levl_version=ver;
}


int RDWaveFile::getLevlBlockSize() const
{
  return levl_block_size;
}


void RDWaveFile::setLevlBlockSize(unsigned size)
{
  levl_block_size=size;
}


int RDWaveFile::getLevlChannels() const
{
  return levl_channels;
}


unsigned short RDWaveFile::getLevlPeak() const
{
  return levl_peak_value;
}


QDateTime RDWaveFile::getLevlTimestamp() const
{
  return levl_timestamp;
}


void RDWaveFile::setEncodeQuality(float qual)
{
  encode_quality=qual;
}


int RDWaveFile::getSerialNumber() const
{
  return serial_number;
}


void RDWaveFile::setSerialNumber(int serial)
{
  serial_number=serial;
}


bool RDWaveFile::getScotChunk() const
{
  return scot_chunk;
}



RDWaveFile::Type RDWaveFile::GetType(int fd)
{
  if(IsWav(fd)) {
    return RDWaveFile::Wave;
  }
  if(IsFlac(fd)) {
    return RDWaveFile::Flac;
  }
  if(IsAtx(fd)) {
    return RDWaveFile::Atx;
  }
  if(IsTmc(fd)) {
    return RDWaveFile::Tmc;
  }
  if(IsOgg(fd)) {
    return RDWaveFile::Ogg;
  }
  if(IsMpeg(fd)) {
    return RDWaveFile::Mpeg;
  }
  return RDWaveFile::Unknown;
}


bool RDWaveFile::IsWav(int fd)
{
  int i;
  char buffer[5];
  
  /* 
   * Is this a riff file? 
   */
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

  /* 
   * Is this a WAVE file? 
   */
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


bool RDWaveFile::IsMpeg(int fd)
{
  RMpegFile mpegfile;
  if(mpegfile.afopen(wave_file)) {
    mpegfile.afclose();
    return true;
  }
  return false;
}


bool RDWaveFile::IsOgg(int fd)
{
#ifdef HAVE_VORBIS
  lseek(fd,0,SEEK_SET);
  if(ov_open(fdopen(fd,"r"),&vorbis_file,NULL,0)==0) {
    return true;
  }
#endif  // HAVE_VORBIS
  return false;
}


bool RDWaveFile::IsAtx(int fd)
{
  char buffer[6];

  lseek(fd,0,SEEK_SET);
  if(read(fd,buffer,5)!=5) {
    return false;
  }
  buffer[5]=0;
  if(strcmp("FILE:",buffer)) {
    return false;
  }

  return true;
}


bool RDWaveFile::IsTmc(int fd)
{
  unsigned char buffer[7];

  lseek(fd,0,SEEK_SET);
  if(read(fd,buffer,6)!=6) {
    return false;
  }
  buffer[6]=0;
  if((buffer[4]!=0xFF)||((buffer[5]&0xF0)!=0xF0)) {
    return false;
  }
  return true;
}


bool RDWaveFile::IsFlac(int fd)
{
  char buffer[5];

  ID3_Tag id3_tag(QCString().sprintf("%s",(const char *)wave_file.name().utf8()));
  lseek(fd,id3_tag.GetPrependedBytes(),SEEK_SET);
  if(read(fd,buffer,4)!=4) {
    return false;
  }
  buffer[4]=0;
  if(strcmp(buffer,"fLaC")) {
    return false;
  }
  return true;
}


off_t RDWaveFile::FindChunk(int fd,char *chunk_name,unsigned *chunk_size)
{
  int i;
//  off_t oOffset;
  char name[5]={0,0,0,0,0};
  unsigned char buffer[4];
  off_t offset;

  offset=lseek(fd,12,SEEK_SET);
  i=read(fd,name,4);
  i=read(fd,buffer,4);
  *chunk_size=buffer[0]+(256*buffer[1])+(65536*buffer[2])+(16777216*buffer[3]);
  while(i==4) {
    if(strcasecmp(chunk_name,name)==0) {
      return lseek(fd,0,SEEK_CUR);
    }
    lseek(fd,*chunk_size,SEEK_CUR);
    i=read(fd,name,4);
    i=read(fd,buffer,4);
    *chunk_size=buffer[0]+(256*buffer[1])+(65536*buffer[2])+
      (16777216*buffer[3]);
  }
  return -1;
}


bool RDWaveFile::GetChunk(int fd,char *chunk_name,unsigned *chunk_size,
			 unsigned char *chunk,size_t size)
{
  if(FindChunk(fd,chunk_name,chunk_size)<0) {
    return false;
  }
  read(fd,chunk,size);
  return true;
}


void RDWaveFile::WriteChunk(int fd,char *cname,unsigned char *buf,unsigned size)
{
  unsigned char size_buf[4];
  unsigned csize;

  if(FindChunk(fd,cname,&csize)<0) {
    size_buf[0]=size&0xff;
    size_buf[1]=(size>>8)&0xff;
    size_buf[2]=(size>>16)&0xff;
    size_buf[3]=(size>>24)&0xff;
    
    lseek(fd,0,SEEK_END);
    write(fd,cname,4);
    write(fd,size_buf,4);
    write(fd,buf,size);

    return;
  }
  if(csize==size) {
    write(fd,buf,size);
    return;
  }
  printf("WARNING: Updated chunk size mismatch!  Update not written.\n");
}


bool RDWaveFile::GetFmt(int fd)
{
  unsigned chunk_size;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"fmt ",&chunk_size,fmt_chunk_data,FMT_CHUNK_SIZE)) {
    format_chunk=false;
    return false;
  }
  format_chunk=true;

  /*
   * Generic data -- present in ALL valid WAV files
   */
  format_tag=fmt_chunk_data[0]+256*fmt_chunk_data[1];
  channels=fmt_chunk_data[2]+256*fmt_chunk_data[3];
  samples_per_sec=fmt_chunk_data[4]+256*fmt_chunk_data[5]+65536*
    fmt_chunk_data[6]+16777216*fmt_chunk_data[7];
  avg_bytes_per_sec=fmt_chunk_data[8]+256*fmt_chunk_data[9]+65536*
    fmt_chunk_data[10]+16777216*fmt_chunk_data[11];
  block_align=fmt_chunk_data[12]+256*fmt_chunk_data[13];
  
  /*
   * PCM Linear specific fields
   */
  if(format_tag==WAVE_FORMAT_PCM) {
    bits_per_sample=fmt_chunk_data[14]+256*fmt_chunk_data[15];
  }

  /*
   * MPEG-1 specific fields
   */
  if(format_tag==WAVE_FORMAT_MPEG) {
    head_layer=fmt_chunk_data[18]+256*fmt_chunk_data[19];
    head_bit_rate=fmt_chunk_data[20]+256*fmt_chunk_data[21]+65536*
      fmt_chunk_data[22]+16777216*fmt_chunk_data[23];
    head_mode=fmt_chunk_data[24]+256*fmt_chunk_data[25];
    head_mode_ext=fmt_chunk_data[26]+256*fmt_chunk_data[27];
    head_emphasis=fmt_chunk_data[28]+256*fmt_chunk_data[29];
    head_flags=fmt_chunk_data[30]+256*fmt_chunk_data[31];
    pts=fmt_chunk_data[32]+256*fmt_chunk_data[33]+65536*
      fmt_chunk_data[34]+16777216*fmt_chunk_data[35];
  }

  return true;
}



bool RDWaveFile::GetFact(int fd)
{
  unsigned chunk_size;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"fact",&chunk_size,fact_chunk_data,FACT_CHUNK_SIZE)) {
    return false;
  }
  fact_chunk=true;

  sample_length=fact_chunk_data[0]+256*fact_chunk_data[1]+
    65536*fact_chunk_data[2]+16777216*fact_chunk_data[3];
  return true;
}



bool RDWaveFile::GetCart(int fd)
{
  unsigned chunk_size;
  char *tag_buffer=NULL;
  int i,j;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"cart",&chunk_size,cart_chunk_data,CART_CHUNK_SIZE)) {
    return false;
  }
  cart_chunk=true;

  cart_version=cart_chunk_data[0]+256*cart_chunk_data[1]+
    65536*cart_chunk_data[2]+16777216*cart_chunk_data[3];
  cart_title=cutString((char *)cart_chunk_data,4,64);
  cart_artist=cutString((char *)cart_chunk_data,68,64);
  cart_cut_id=cutString((char *)cart_chunk_data,132,64);
  cart_client_id=cutString((char *)cart_chunk_data,196,64);
  cart_category=cutString((char *)cart_chunk_data,260,64);
  cart_classification=cutString((char *)cart_chunk_data,324,64);
  cart_out_cue=cutString((char *)cart_chunk_data,388,64);
  cart_start_date=cutDate((char *)cart_chunk_data,452);
  cart_start_time=cutTime((char *)cart_chunk_data,462);
  cart_end_date=cutDate((char *)cart_chunk_data,470);
  cart_end_time=cutTime((char *)cart_chunk_data,480);
  cart_producer_app_id=cutString((char *)cart_chunk_data,488,64);
  cart_producer_app_ver=cutString((char *)cart_chunk_data,552,64);
  cart_user_def=cutString((char *)cart_chunk_data,616,64);
  cart_level_ref=cart_chunk_data[680]+256*cart_chunk_data[681]+
    65536*cart_chunk_data[682]+16777216*cart_chunk_data[683];

  j=0;
  for(i=0;i<MAX_TIMERS;i++) {
    if(cart_chunk_data[684+i*8]!=0) {
      cart_timer_label[j]=cutString((char *)cart_chunk_data,684+i*8,4);
      cart_timer_sample[j]=cart_chunk_data[688+i*8]+
	256*cart_chunk_data[689+i*8]+
	65536*cart_chunk_data[690+i*8]+
	16777216*cart_chunk_data[691+i*8];
      j++;
    }
  }
  if(wave_type==RDWaveFile::Wave) {
    cart_url=cutString((char *)cart_chunk_data,1024,1024);

    //
    // Get the Tag Text
    //
    if(chunk_size>2048) {
      tag_buffer=(char *)malloc(chunk_size-2048+1);
      read(wave_file.handle(),tag_buffer,chunk_size-2048);
      tag_buffer[chunk_size-2048]=0;
      cart_tag_text=tag_buffer;
      free(tag_buffer);
      tag_buffer=NULL;
    }
  }
  else {
    cart_url="";
    cart_tag_text="";
  }


  if(wave_data!=NULL) {
    wave_data->setMetadataFound(true);
    wave_data->setTitle(cart_title);
    wave_data->setArtist(cart_artist);
    wave_data->setCutId(cart_cut_id);
    wave_data->setClient(cart_client_id);
    wave_data->setCategory(cart_category);
    wave_data->setClassification(cart_classification);
    wave_data->setOutCue(cart_out_cue);
    wave_data->setStartDate(cart_start_date);
    wave_data->setStartTime(cart_start_time);
    wave_data->setEndDate(cart_end_date);
    wave_data->setEndTime(cart_end_time);
    wave_data->setUserDefined(cart_user_def);
    wave_data->setUrl(cart_url);
    wave_data->setTagText(cart_tag_text);
    for(int i=0;i<MAX_TIMERS;i++) {
      if(cart_timer_label[i]=="SEGs") {
	wave_data->setSegueStartPos((int)(1000.0*((double)cart_timer_sample[i])/
					  ((double)getSamplesPerSec())));
      }
      if(cart_timer_label[i]=="SEGe") {
	wave_data->setSegueEndPos((int)(1000.0*((double)cart_timer_sample[i])/
					((double)getSamplesPerSec())));
      }
      if(cart_timer_label[i]=="INTs") {
	wave_data->setIntroStartPos((int)(1000.0*((double)cart_timer_sample[i])/
					  ((double)getSamplesPerSec())));
      }
      if(cart_timer_label[i]=="INTe") {
	wave_data->setIntroEndPos((int)(1000.0*((double)cart_timer_sample[i])/
					((double)getSamplesPerSec())));
      }
      if(cart_timer_label[i]=="AUDs") {
	wave_data->setStartPos((int)(1000.0*((double)cart_timer_sample[i])/
					  ((double)getSamplesPerSec())));
      }
      if(cart_timer_label[i]=="AUDe") {
	wave_data->setEndPos((int)(1000.0*((double)cart_timer_sample[i])/
					((double)getSamplesPerSec())));
      }
    }
  }
  return true;
}


bool RDWaveFile::GetBext(int fd)
{
  unsigned chunk_size;
  char *tag_buffer=NULL;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"bext",&chunk_size,bext_chunk_data,BEXT_CHUNK_SIZE)) {
    return false;
  }
  bext_chunk=true;

  bext_description=cutString((char *)bext_chunk_data,0,256);
  bext_originator=cutString((char *)bext_chunk_data,256,32);
  bext_originator_ref=cutString((char *)bext_chunk_data,288,32);
  bext_origination_date=cutDate((char *)bext_chunk_data,320);
  bext_origination_time=cutTime((char *)bext_chunk_data,330);
  bext_time_reference_low=bext_chunk_data[338]+256*bext_chunk_data[339]+
    65536*bext_chunk_data[340]+16777216*bext_chunk_data[341];
  bext_time_reference_high=bext_chunk_data[342]+256*bext_chunk_data[343]+
    65536*bext_chunk_data[344]+16777216*bext_chunk_data[345];
  bext_version=bext_chunk_data[346]+256*bext_chunk_data[347];
  for(int i=0;i<64;i++) {
    bext_umid[i]=bext_chunk_data[348+i];
  }

  //
  // Get the Coding History
  //
  if(chunk_size>602) {
    tag_buffer=(char *)malloc(chunk_size-602+1);
    read(wave_file.handle(),tag_buffer,chunk_size-602);
    tag_buffer[chunk_size-602]=0;
    bext_coding_history=tag_buffer;
    free(tag_buffer);
    tag_buffer=NULL;
  }

  if(wave_data!=NULL) {
    wave_data->setMetadataFound(true);
    wave_data->setDescription(bext_description);
    wave_data->setOriginator(bext_originator);
    wave_data->setOriginatorReference(bext_originator_ref);
    wave_data->setOriginationDate(bext_origination_date);
    wave_data->setOriginationTime(bext_origination_time);
    wave_data->setCodingHistory(bext_coding_history);
  }
  return true;
}


bool RDWaveFile::GetMext(int fd)
{
  unsigned chunk_size;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"mext",&chunk_size,mext_chunk_data,MEXT_CHUNK_SIZE)) {
    return false;
  }
  mext_chunk=true;

  if((mext_chunk_data[0]&1)!=0) {
    mext_homogenous=true;
  }
  if((mext_chunk_data[0]&2)==0) {
    mext_padding_used=true;
  }
  if((mext_chunk_data[0]&4)!=0) {
    mext_rate_hacked=true;
  }
  if((mext_chunk_data[0]&8)!=0) {
    mext_free_format=true;
  }
  mext_frame_size=mext_chunk_data[2]+256*mext_chunk_data[3];
  mext_anc_length=mext_chunk_data[4]+256*mext_chunk_data[5];
  if((mext_chunk_data[6]&1)!=0) {
    mext_left_energy=true;
  }
  if((mext_chunk_data[6]&2)!=0) {
    mext_ancillary_private=true;
  }
  if((mext_chunk_data[6]&4)!=0) {
    mext_right_energy=true;
  }

  return true;
}



bool RDWaveFile::ReadEnergyFile(QString wave_file_name)
{
  if(has_energy && energy_loaded) return true;

  QFile energy_file;
  QString str;
  unsigned char frame[50];

  energy_file.setName(wave_file_name+".energy");
  if(!energy_file.open(IO_ReadOnly)) 
    return false;
  if(energy_file.readLine(str,20) <= 0)
     return false;
  normalize_level=str.toDouble();
  energy_file.close();
  if(!energy_file.open(IO_ReadOnly)) 
    return false;
  read(energy_file.handle(),frame,str.length());
  int i=0;
  while(read(energy_file.handle(),frame,2)>0) {
      energy_data.push_back(frame[0]+256*frame[1]);
      i++;
  }
  ext_time_length=
   	       (unsigned)(1000.0*(((double)i*1152)/getChannels())/(double)samples_per_sec);  
  energy_file.close();
  energy_loaded=true;
  has_energy=true;
  return true;
}


bool RDWaveFile::ReadNormalizeLevel(QString wave_file_name)
{
  QFile energy_file;
  QString str;

  energy_file.setName(wave_file_name+".energy");
  if(!energy_file.open(IO_ReadOnly)) 
    return false;
  if(energy_file.readLine(str,20) <= 0)
     return false;
  normalize_level=str.toDouble();
  energy_file.close();
  return true;
}


bool RDWaveFile::GetLevl(int fd)
{
  unsigned size=LEVL_CHUNK_SIZE;
  unsigned chunk_size;
  unsigned char frame[2];
  QDate date;
  QTime time;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"levl",&chunk_size,levl_chunk_data,LEVL_CHUNK_SIZE)) {
  //  if(ReadEnergyFile(wave_file.name())) {
   //   energy_loaded=true;
   //   has_energy=true;
   //   return true;
   //   }
  //  else {
     return false;
  //  }
  }
  levl_chunk=true;

  levl_version=ReadDword(levl_chunk_data,0);
  levl_format=ReadDword(levl_chunk_data,4);
  levl_points=ReadDword(levl_chunk_data,8);
  levl_block_size=ReadDword(levl_chunk_data,12);
  levl_channels=ReadDword(levl_chunk_data,16);
  levl_frames=ReadDword(levl_chunk_data,20);
  levl_peak_offset=ReadDword(levl_chunk_data,24);
  levl_block_offset=ReadDword(levl_chunk_data,28);
  levl_timestamp=QDateTime(cutDate((char *)levl_chunk_data,32),
			   cutTime((char *)levl_chunk_data,43));
  if(levl_block_size!=1152) {
    return true;
  }
  lseek(wave_file.handle(),FindChunk(wave_file.handle(),"levl",&size)+
	levl_block_offset-8,SEEK_SET);
  for(unsigned i=1;i<levl_frames;i++) {
    for(int j=0;j<levl_channels;j++) {
      read(wave_file.handle(),frame,2);
      energy_data.push_back(frame[0]+256*frame[1]);
    }
  }
  if(levl_peak_offset==0xFFFFFFFF) {
    levl_peak_value=0;
  }
  else {
    levl_peak_value=energy_data[levl_peak_offset];
  }
  energy_loaded=true;
  has_energy=true;
  return true;
}


bool RDWaveFile::GetScot(int fd)
{
  unsigned chunk_size;
  int start_day;
  int start_month;
  int start_year;
  int start_hour;
  int end_day;
  int end_month;
  int end_year;
  int end_hour;


  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"scot",&chunk_size,scot_chunk_data,SCOT_CHUNK_SIZE)) {
    return false;
  }
  scot_chunk=true;

  scot_title=cutString((char *)scot_chunk_data,4,42);
  scot_artist=cutString((char *)scot_chunk_data,267,33);
  scot_etc=cutString((char *)scot_chunk_data,301,33);
  scot_year=cutString((char *)scot_chunk_data,338,4).toInt();
  scot_intro_length=cutString((char *)scot_chunk_data,335,2).toInt()*1000;
  start_year=cutString((char *)scot_chunk_data,69,2).toInt()+2000;
  start_month=cutString((char *)scot_chunk_data,65,2).toInt();
  start_day=cutString((char *)scot_chunk_data,67,2).toInt();
  if((start_month>0)&&(start_month<13)&&(start_month>0)&&(start_day<32)) {
    scot_start_date=QDate(start_day,start_month,start_day);
  }
  start_hour=cutString((char *)scot_chunk_data,77,2).toInt();
  if((start_hour>=129)&&(start_hour<=151)) {
    scot_start_time=QTime(start_hour-128,0,0);
  }
  end_year=cutString((char *)scot_chunk_data,75,2).toInt()+2000;
  end_month=cutString((char *)scot_chunk_data,71,2).toInt();
  end_day=cutString((char *)scot_chunk_data,73,2).toInt();
  if((end_month>0)&&(end_month<13)&&(end_day>0)&&(end_day<32)&&
    scot_start_date.isValid()) {
    scot_end_date=QDate(end_day,end_month,end_day);
  }
  else {
    scot_start_date=QDate();
    scot_end_date=QDate();
  }
  end_hour=cutString((char *)scot_chunk_data,78,2).toInt();
  if((end_hour>=129)&&(end_hour<=151)) {
    scot_end_time=QTime(end_hour-128,0,0);
  }
  if(wave_data!=NULL) {
    wave_data->setMetadataFound(true);
    wave_data->setTitle(scot_title.stripWhiteSpace());
    wave_data->setArtist(scot_artist.stripWhiteSpace());
    wave_data->setUserDefined(scot_etc.stripWhiteSpace());
    wave_data->setReleaseYear(scot_year);
    wave_data->setIntroStartPos(0);
    wave_data->setIntroEndPos(scot_intro_length);
    if(scot_start_date.isValid()) {
      wave_data->setStartDate(scot_start_date);
    }
    if(scot_start_time.isValid()) {
      wave_data->setStartTime(scot_start_time);
    }
    if(scot_end_date.isValid()) {
      wave_data->setEndDate(scot_end_date);
    }
    if(scot_end_time.isValid()) {
      wave_data->setEndTime(scot_end_time);
    }
  }
  return true;
}


bool RDWaveFile::GetAv10(int fd)
{
  //
  // The 'av10' chunk is used by BE AudioVault systems for metadata storage
  //
  unsigned chunk_size;
  char temp[512];
  unsigned ptr=1;
  QString str;
  int n;
  int pos;
  int offset;
  bool ok=false;
  int istate=0;
  QString label;
  QString arg;
  QString userdef;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"av10",&chunk_size,av10_chunk_data,AV10_CHUNK_SIZE)) {
    return false;
  }
  av10_chunk=true;

  //
  // Walk through the fields
  //
  for(int i=2;i<chunk_size;i++) {
    switch(istate) {
      case 0:  // Label
	if(av10_chunk_data[i]==0) {
	  i++;
	  istate=1;
	}
	else {
	  label+=av10_chunk_data[i];
	}
	break;

      case 1:  // Argument
	if(av10_chunk_data[i]==0) {  // Found a label/value pair, see if we can
                                     // use it
	  if(label=="1") {  // Start/end markers
	    if((n=arg.find(","))>0) {
	      pos=arg.left(n).toInt(&ok);
	      if(ok) {
		offset=arg.right(arg.length()-n-1).toInt(&ok);
		if(ok) {
		  if(wave_data!=NULL) {
		    wave_data->setStartPos(pos);
		    wave_data->setEndPos(pos+offset);
		    wave_data->setMetadataFound(true);
		  }
		}
	      }
	    }
	  }
	  if(label=="2") {  // Segue markers
	    if((n=arg.find(","))>0) {
	      pos=arg.left(n).toInt(&ok);
	      if(ok) {
		offset=arg.right(arg.length()-n-1).toInt(&ok);
		if(ok) {
		  if(wave_data!=NULL) {
		    wave_data->setSegueStartPos(pos);
		    wave_data->setSegueEndPos(pos+offset);
		    wave_data->setMetadataFound(true);
		  }
		}
	      }
	    }
	  }
	  if(label=="C") {  // AV Category
	    userdef+=("av_category="+arg+", ");
	  }
	  if(label=="CI") {  // AV Artist/Client
	    if(wave_data!=NULL) {
	      wave_data->setArtist(arg);
	    }
	  }
	  if(label=="CL") {  // AV Class
	    userdef+=("av_class="+arg+", ");
	  }
	  if(label=="CO") {  // AV Codes
	    userdef+=("av_codes="+arg+", ");
	  }
	  if(label=="D") {   // AV Description
	    if(wave_data!=NULL) {
	      wave_data->setTitle(arg);
	      wave_data->setMetadataFound(true);
	    }
	  }
	  if(label=="G") {   // AV ???
	  }
	  if(label=="IN") {  // AV Intro
	    if(ok) {
	      if(wave_data!=NULL) {
		wave_data->setIntroStartPos(wave_data->startPos());
		wave_data->setIntroEndPos(1000*arg.toInt());
		wave_data->setMetadataFound(true);
	      }
	    }
	  }
	  if(label=="K") {   // AV Kill Date
	  }
	  if(label=="N") {   // AV Name
	  }
	  if(label=="Q") {   // AV Outcue
	    if(wave_data!=NULL) {
	      wave_data->setOutCue(arg);
	      wave_data->setMetadataFound(true);
	    }
	  }
	  if(label=="S") {   // AV Start Date
	  }
	  if(label=="SR") {  // AV ???
	  }
	  // printf("Label: %s  Arg: %s\n",(const char *)label,(const char *)arg);
	  label="";
	  arg="";
	  istate=0;
	}
	else {
	  arg+=av10_chunk_data[i];
	}
	break;	
    }
    if(!userdef.isEmpty()) {
      if(wave_data!=NULL) {
	wave_data->setUserDefined(userdef.left(userdef.length()-2));
      }      
    }
  }

  return true;
}


bool RDWaveFile::GetList(int fd)
{
  unsigned chunk_size=0;
  if((wave_data==NULL)||(FindChunk(fd,"list",&chunk_size)<0)) {
    return false;
  }
  unsigned char *chunk_data=new unsigned char[chunk_size];
  read(fd,chunk_data,chunk_size);
/*
  if(strncmp("INFO",chunk_data,4)) {
    delete chunk_data;
    return false;
  }
*/
  unsigned offset=4;
  while(ReadListElement(chunk_data,&offset,chunk_size));
  if((wave_data->segueStartPos()>=0)&&(wave_data->segueEndPos()<0)) {
    wave_data->setSegueEndPos(ext_time_length);
  }
  return true;
}


bool RDWaveFile::ReadListElement(unsigned char *buffer,unsigned *offset,
				unsigned size)
{
  if(*offset>=size) {
    return false;
  }
  char tag[5];
  for(int i=0;i<4;i++) {
    tag[i]=buffer[*offset+i];
  }
  tag[4]=0;
  *offset+=4;
  unsigned len=ReadDword(buffer,*offset);
  *offset+=4;
  if(!strcmp(tag,"tref")) {
    wave_data->setTmciSongId(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tttl")) {
    wave_data->setTitle(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tart")) {
    wave_data->setArtist(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }

  if(!strcmp(tag,"tcom")) {
    wave_data->setComposer(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tpub")) {
    wave_data->setPublisher(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tlic")) {
    wave_data->setLicensingOrganization(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tlab")) {
    wave_data->setLabel(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tint")) {
    wave_data->setIntroStartPos(0);
    wave_data->setIntroEndPos(RDSetTimeLength(((char *)buffer)+(*offset)));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"ttim")) {
    wave_data->setStartPos(0);
    wave_data->setEndPos(RDSetTimeLength(((char *)buffer)+(*offset)));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tend")) {
    wave_data->setEndType((RDWaveData::EndType)(((char *)buffer)+(*offset))[0]);
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tyr ")) {
    wave_data->setReleaseYear(QString(((char *)buffer)+(*offset)).toInt());
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"taux")) {
    wave_data->setSegueStartPos(RDSetTimeLength(((char *)buffer)+(*offset)));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tbpm")) {
    wave_data->setBeatsPerMinute(QString(((char *)buffer)+(*offset)).toInt());
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"talb")) {
    wave_data->setAlbum(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tpli")) {
    wave_data->setCopyrightNotice(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tisr")) {
    wave_data->setIsrc(QString(((char *)buffer)+(*offset)).remove(" "));
    wave_data->setMetadataFound(true);
  }

  *offset+=len;
  while(((*offset)<size) && (buffer[*offset]==0)) {
    (*offset)++;
  }
  return true;
}


bool RDWaveFile::ReadTmcMetadata(int fd)
{
  char buffer[256];
  QString current_tag;

  lseek(fd,data_length+4,SEEK_SET);
  while(GetLine(fd,buffer,255)) {
    if(buffer[0]=='#') {
      current_tag=QString(buffer+1);
    }
    else {
      if(!current_tag.isEmpty()) {
	ReadTmcTag(current_tag,QString(buffer));
      }
    }
  }
  return false;
}


void RDWaveFile::ReadTmcTag(const QString tag,const QString value)
{
  if(tag=="TITLE") {
    wave_data->setTitle(value.stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
  if(tag=="ARTIST") {
    wave_data->setArtist(value.stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
  if(tag=="COMPOSER") {
    wave_data->setComposer(value.stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
  if(tag=="PUBLISHER") {
    wave_data->setPublisher(value.stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
  if(tag=="LICENSE") {
    wave_data->setLicensingOrganization(value.stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
  if(tag=="LABEL") {
    wave_data->setLabel(value.stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
  if(tag=="ALBUM") {
    wave_data->setAlbum(value.stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
  if(tag=="YEAR") {
    wave_data->setReleaseYear(value.stripWhiteSpace().toInt());
    wave_data->setMetadataFound(true);
  }
  if(tag=="INTRO") {
    wave_data->setIntroStartPos(0);
    wave_data->setIntroEndPos(RDSetTimeLength(value.stripWhiteSpace()));
    wave_data->setMetadataFound(true);
  }
  if(tag=="AUX") {
    wave_data->setSegueStartPos(RDSetTimeLength(value.stripWhiteSpace()));
    wave_data->setMetadataFound(true);
  }
  if(tag=="END") {
    wave_data->setEndType((RDWaveData::EndType)((char)value[0]));
    wave_data->setMetadataFound(true);
  }
  if(tag=="TMCIREF") {
    wave_data->setTmciSongId(value.stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
  if(tag=="BPM") {
    wave_data->setBeatsPerMinute(value.toInt());
    wave_data->setMetadataFound(true);
  }
  if(tag=="ISRC") {
    QString str=value;
    wave_data->setIsrc(str.remove(" ").stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
  if(tag=="PLINE") {
    wave_data->setCopyrightNotice(value.stripWhiteSpace());
    wave_data->setMetadataFound(true);
  }
}


bool RDWaveFile::GetLine(int fd,char *buffer,int max_len)
{
  for(int i=0;i<max_len;i++) {
    if(read(fd,buffer+i,1)==0) {
      return false;
    }
    if(buffer[i]==10) {
      if(buffer[i-1]==13) {
	buffer[i-1]=0;
      }
      else {
	buffer[i]=0;
      }
      return true;
    }
  }
  buffer[max_len-1]=0;
  return true;
}


void RDWaveFile::ReadOggMetadata()
{
  if(wave_data==NULL) {
    return;
  }
  TagLib::FileRef f(QCString().sprintf("%s",(const char *)wave_file.name().utf8()));
  if(!f.isNull() && f.tag()){
    TagLib::Tag *tag=f.tag();
    wave_data->setTitle(TStringToQString(tag->title()));
    wave_data->setArtist(TStringToQString(tag->artist()));
    wave_data->setAlbum(TStringToQString(tag->album()));
    wave_data->setReleaseYear((int)tag->year());
    wave_data->setMetadataFound(true);
  } 
}


void RDWaveFile::ReadId3Metadata()
{
  if(wave_data==NULL) {
    return;
  }
  ID3_Frame *frame=NULL;
  ID3_Tag id3_tag(QCString().sprintf("%s",(const char *)wave_file.name().utf8()));
  if((frame=id3_tag.Find(ID3FID_TITLE))!=NULL) {
    wave_data->setTitle(ID3_GetString(frame,ID3FN_TEXT));
    wave_data->setMetadataFound(true);
  }
  if((frame=id3_tag.Find(ID3FID_BPM))!=NULL) {
    wave_data->
      setBeatsPerMinute(QString(ID3_GetString(frame,ID3FN_TEXT)).toInt());
    wave_data->setMetadataFound(true);
  }
  if((frame=id3_tag.Find(ID3FID_ALBUM))!=NULL) {
    wave_data->setAlbum(ID3_GetString(frame,ID3FN_TEXT));
    wave_data->setMetadataFound(true);
  }
  if((frame=id3_tag.Find(ID3FID_COMPOSER))!=NULL) {
    wave_data->setComposer(ID3_GetString(frame,ID3FN_TEXT));
    wave_data->setMetadataFound(true);
  }
  if((frame=id3_tag.Find(ID3FID_COPYRIGHT))!=NULL) {
    wave_data->setCopyrightNotice(ID3_GetString(frame,ID3FN_TEXT));
    wave_data->setMetadataFound(true);
  }
  if((frame=id3_tag.Find(ID3FID_ORIGARTIST))!=NULL) {
    wave_data->setArtist(ID3_GetString(frame,ID3FN_TEXT));
    wave_data->setMetadataFound(true);
  }
  if((frame=id3_tag.Find(ID3FID_LEADARTIST))!=NULL) {
    wave_data->setArtist(ID3_GetString(frame,ID3FN_TEXT));
    wave_data->setMetadataFound(true);
  }
  if((frame=id3_tag.Find(ID3FID_PUBLISHER))!=NULL) {
    wave_data->setPublisher(ID3_GetString(frame,ID3FN_TEXT));
    wave_data->setMetadataFound(true);
  }
  if((frame=id3_tag.Find(ID3FID_ISRC))!=NULL) {
    wave_data->setIsrc(ID3_GetString(frame,ID3FN_TEXT));
    wave_data->setMetadataFound(true);
  }
  if((frame=id3_tag.Find(ID3FID_YEAR))!=NULL) {
    wave_data->
      setReleaseYear(QString(ID3_GetString(frame,ID3FN_TEXT)).toInt());
    wave_data->setMetadataFound(true);
  }
}


bool RDWaveFile::GetMpegHeader(int fd,int offset)
{
   if (format_tag == WAVE_FORMAT_MPEG){
   RMpegFile mpegfile;
   if(!mpegfile.afopen(wave_file)) return false;

   switch(mpegfile.getVersion())
   {
      case 0:
    mpeg_id=RDWaveFile::Mpeg1;
	break;
		  case 1:
	 mpeg_id=RDWaveFile::Mpeg2;
		    break;
		  case 2:
	 //mpeg_id=RDWaveFile::Mpeg25;
		    break;
		  default:
	 mpeg_id=RDWaveFile::NonMpeg;
	break;

  }

  //
   // Layer
  //
   head_layer = mpegfile.getLayer();

  //
   // Bitrate
  //
   head_bit_rate = mpegfile.getBitrate();

   //
   // Sample Rate
   //
   samples_per_sec = mpegfile.getFrequency();

  //
   // Channels 
  //
   channels = mpegfile.getChannels();

  //
  // Frame Size (without padding)
  //
  mpeg_frame_size=144*head_bit_rate/samples_per_sec;
  mpegfile.afclose();
  return true;
}

  unsigned char buffer[4];
  int n;
  
  lseek(fd,offset,SEEK_SET);
  if((n=read(fd,buffer,4))!=4) {
    return false;
  }

  //
  // Sync
  //
  if((buffer[0]!=0xFF)||((buffer[1]&0xE0)!=0xE0)) {
    return false;
  }

  //
  // MPEG Id
  //
  if((buffer[1]&0x08)==0) {
    mpeg_id=RDWaveFile::Mpeg2;
  }
  else {
    mpeg_id=RDWaveFile::Mpeg1;
  }

  //
  // Layer
  //
  switch((buffer[1]&0x06)>>1) {
      case 1:
	head_layer=3;
	break;

      case 2:
	head_layer=2;
	break;

      case 3:
	head_layer=1;
	break;

      default:
	return false;
  }

  //
  // Bitrate
  //
  switch(mpeg_id) {
      case RDWaveFile::Mpeg1:
	switch(head_layer) {
	    case 1:
	      switch(buffer[2]>>4) {
		  case 1:
		    head_bit_rate=32000;
		    break;
	
		  case 2:
		    head_bit_rate=64000;
		    break;
	
		  case 3:
		    head_bit_rate=96000;
		    break;
	
		  case 4:
		    head_bit_rate=128000;
		    break;
	
		  case 5:
		    head_bit_rate=160000;
		    break;
	
		  case 6:
		    head_bit_rate=192000;
		    break;
	
		  case 7:
		    head_bit_rate=224000;
		    break;
	
		  case 8:
		    head_bit_rate=256000;
		    break;
	
		  case 9:
		    head_bit_rate=288000;
		    break;
	
		  case 10:
		    head_bit_rate=320000;
		    break;
	
		  case 11:
		    head_bit_rate=352000;
		    break;
	
		  case 12:
		    head_bit_rate=384000;
		    break;
	
		  case 13:
		    head_bit_rate=416000;
		    break;
	
		  case 14:
		    head_bit_rate=448000;
		    break;

		  default:
		    return false;
	      }
	      break;

	    case 2:
	      switch(buffer[2]>>4) {
		  case 1:
		    head_bit_rate=32000;
		    break;
	
		  case 2:
		    head_bit_rate=48000;
		    break;
	
		  case 3:
		    head_bit_rate=56000;
		    break;
	
		  case 4:
		    head_bit_rate=64000;
		    break;
	
		  case 5:
		    head_bit_rate=80000;
		    break;
	
		  case 6:
		    head_bit_rate=96000;
		    break;
	
		  case 7:
		    head_bit_rate=112000;
		    break;
	
		  case 8:
		    head_bit_rate=128000;
		    break;
	
		  case 9:
		    head_bit_rate=160000;
		    break;
	
		  case 10:
		    head_bit_rate=192000;
		    break;
	
		  case 11:
		    head_bit_rate=224000;
		    break;
	
		  case 12:
		    head_bit_rate=256000;
		    break;
	
		  case 13:
		    head_bit_rate=320000;
		    break;
	
		  case 14:
		    head_bit_rate=384000;
		    break;

		  default:
		    return false;
		    break;
	      }
	      break;

	    case 3:
	      switch(buffer[2]>>4) {
		  case 1:
		    head_bit_rate=32000;
		    break;
	
		  case 2:
		    head_bit_rate=40000;
		    break;
	
		  case 3:
		    head_bit_rate=48000;
		    break;
	
		  case 4:
		    head_bit_rate=56000;
		    break;
	
		  case 5:
		    head_bit_rate=64000;
		    break;
	
		  case 6:
		    head_bit_rate=80000;
		    break;
	
		  case 7:
		    head_bit_rate=96000;
		    break;
	
		  case 8:
		    head_bit_rate=112000;
		    break;
	
		  case 9:
		    head_bit_rate=128000;
		    break;
	
		  case 10:
		    head_bit_rate=160000;
		    break;
	
		  case 11:
		    head_bit_rate=192000;
		    break;
	
		  case 12:
		    head_bit_rate=224000;
		    break;
	
		  case 13:
		    head_bit_rate=256000;
		    break;
	
		  case 14:
		    head_bit_rate=320000;
		    break;

		  default:
		    return false;
		    break;
	      }
	      break;
	}
	break;

      case RDWaveFile::Mpeg2:
	switch(head_layer) {
	    case 1:
	      switch(buffer[2]>>4) {
		  case 1:
		    head_bit_rate=32000;
		    break;
	
		  case 2:
		    head_bit_rate=48000;
		    break;
	
		  case 3:
		    head_bit_rate=56000;
		    break;
	
		  case 4:
		    head_bit_rate=64000;
		    break;
	
		  case 5:
		    head_bit_rate=80000;
		    break;
	
		  case 6:
		    head_bit_rate=96000;
		    break;
	
		  case 7:
		    head_bit_rate=112000;
		    break;
	
		  case 8:
		    head_bit_rate=128000;
		    break;
	
		  case 9:
		    head_bit_rate=144000;
		    break;
	
		  case 10:
		    head_bit_rate=160000;
		    break;
	
		  case 11:
		    head_bit_rate=176000;
		    break;
	
		  case 12:
		    head_bit_rate=192000;
		    break;
	
		  case 13:
		    head_bit_rate=224000;
		    break;
	
		  case 14:
		    head_bit_rate=256000;
		    break;

		  default:
		    return false;
	      }
	      break;

	    case 2:
	      switch(buffer[2]>>4) {
		  case 1:
		    head_bit_rate=8000;
		    break;
	
		  case 2:
		    head_bit_rate=16000;
		    break;
	
		  case 3:
		    head_bit_rate=24000;
		    break;
	
		  case 4:
		    head_bit_rate=32000;
		    break;
	
		  case 5:
		    head_bit_rate=40000;
		    break;
	
		  case 6:
		    head_bit_rate=48000;
		    break;
	
		  case 7:
		    head_bit_rate=56000;
		    break;
	
		  case 8:
		    head_bit_rate=64000;
		    break;
	
		  case 9:
		    head_bit_rate=80000;
		    break;
	
		  case 10:
		    head_bit_rate=96000;
		    break;
	
		  case 11:
		    head_bit_rate=112000;
		    break;
	
		  case 12:
		    head_bit_rate=128000;
		    break;
	
		  case 13:
		    head_bit_rate=144000;
		    break;
	
		  case 14:
		    head_bit_rate=160000;
		    break;

		  default:
		    return false;
		    break;
	      }
	      break;

	    case 3:
	      switch(buffer[2]>>4) {
		  case 1:
		    head_bit_rate=8000;
		    break;
	
		  case 2:
		    head_bit_rate=16000;
		    break;
	
		  case 3:
		    head_bit_rate=24000;
		    break;
	
		  case 4:
		    head_bit_rate=32000;
		    break;
	
		  case 5:
		    head_bit_rate=40000;
		    break;
	
		  case 6:
		    head_bit_rate=48000;
		    break;
	
		  case 7:
		    head_bit_rate=56000;
		    break;
	
		  case 8:
		    head_bit_rate=64000;
		    break;
	
		  case 9:
		    head_bit_rate=80000;
		    break;
	
		  case 10:
		    head_bit_rate=96000;
		    break;
	
		  case 11:
		    head_bit_rate=112000;
		    break;
	
		  case 12:
		    head_bit_rate=128000;
		    break;
	
		  case 13:
		    head_bit_rate=144000;
		    break;
	
		  case 14:
		    head_bit_rate=160000;
		    break;

		  default:
		    return false;
		    break;
	      }
	      break;
	}
	break;

      default:
	return false;
  }

  //
  // Sample Rate
  //
  switch((buffer[2]>>2)&0x03) {
      case 0:
	switch(mpeg_id) {
	    case RDWaveFile::Mpeg1:
	      samples_per_sec=44100;
	      break;

	    case RDWaveFile::Mpeg2:
	      samples_per_sec=22050;
	      break;

	    default:
	      break;
	}
	break;

      case 1:
	switch(mpeg_id) {
	    case RDWaveFile::Mpeg1:
	      samples_per_sec=48000;
	      break;

	    case RDWaveFile::Mpeg2:
	      samples_per_sec=24000;
	      break;

	    default:
	      break;
	}
	break;

      case 2:
	switch(mpeg_id) {
	    case RDWaveFile::Mpeg1:
	      samples_per_sec=32000;
	      break;

	    case RDWaveFile::Mpeg2:
	      samples_per_sec=16000;
	      break;

	    default:
	      break;
	}
	break;

      default:
	return false;
	break;
  }

  //
  // Mode
  //
  switch(buffer[3]>>6) {
      case 0:
	head_mode=ACM_MPEG_STEREO;
	channels=2;
	break;

      case 1:
	head_mode=ACM_MPEG_JOINTSTEREO;
	channels=2;
	break;

      case 2:
	head_mode=ACM_MPEG_DUALCHANNEL;
	channels=2;
	break;

      case 3:
	head_mode=ACM_MPEG_SINGLECHANNEL;
	channels=1;
	break;
  }

  //
  // Flags
  //
  if((buffer[2]&0x01)!=0) {
    head_flags|=ACM_MPEG_PRIVATEBIT;
  }
  if((buffer[3]&0x08)!=0) {
    head_flags|=ACM_MPEG_COPYRIGHT;
  }
  if((buffer[3]&0x04)!=0) {
    head_flags|=ACM_MPEG_ORIGINALHOME;
  }
  if((buffer[1]&0x01)!=0) {
    head_flags|=ACM_MPEG_PROTECTIONBIT;
  }
  if(mpeg_id==RDWaveFile::Mpeg1) {
    head_flags|=ACM_MPEG_ID_MPEG1;
  }

  //
  // Frame Size (without padding)
  //
  mpeg_frame_size=144*head_bit_rate/samples_per_sec;
  return true;
}


int RDWaveFile::GetAtxOffset(int fd)
{
  unsigned char buffer[MAX_ATX_HEADER_SIZE];
  lseek(fd,0,SEEK_SET);

  int n=read(fd,buffer,MAX_ATX_HEADER_SIZE-1);
  for(int i=0;i<n;i++) {
    if(buffer[i]==0xFF) {
      return i;
    }
  }
  return -1;
}


bool RDWaveFile::GetFlacStreamInfo()
{
  FLAC__StreamMetadata sinfo;
  if(!FLAC__metadata_get_streaminfo(QCString().sprintf("%s",(const char *)wave_file.name().utf8()),&sinfo)) {
    return false;
  }
  samples_per_sec=sinfo.data.stream_info.sample_rate;
  bits_per_sample=sinfo.data.stream_info.bits_per_sample;
  sample_length=sinfo.data.stream_info.total_samples;
  channels=sinfo.data.stream_info.channels;
  return true;
}


void RDWaveFile::ReadFlacMetadata()
{
#ifdef HAVE_FLAC_METADATA
  QString artist;
  QString composer;
  FLAC__StreamMetadata* tags;
  if(!FLAC__metadata_get_tags(QCString().
	    sprintf("%s",(const char *)wave_file.name().utf8()),&tags)) {
    return;
  }
  for(int iCommentIndex=0;iCommentIndex<tags->data.vorbis_comment.num_comments;
      ++iCommentIndex)
  {
    FLAC__StreamMetadata_VorbisComment_Entry&
      comment(tags->data.vorbis_comment.comments[iCommentIndex]);
	   
    // comment.entry is a UTF-8 string of comment.length octets 
    // (http://www.xiph.org/vorbis/doc/v-comment.html)
    QString entry=
      QString::fromUtf8((const char *)comment.entry, comment.length);
	   
    // every entry is a name=value pair. Name is not allowed to contain a '='
    // so we just scan to the first instance of it.
    int nameLength = entry.find('=');
    if(nameLength < 0) continue; // malformed comment, it would seem
    QString name = entry.left(nameLength), value = entry.mid(nameLength + 1);
	   
    if(name=="TITLE") { 
      wave_data->setTitle(value); 
      wave_data->setMetadataFound(true); 
      continue;
    }
    if(name=="ARTIST") { 
      composer=value;
      wave_data->setMetadataFound(true);
      continue;
    }
    if(name=="PERFORMER") { 
      artist=value;
      wave_data->setMetadataFound(true);
    }
    if(name=="ALBUM") { 
      wave_data->setAlbum(value);
      wave_data->setMetadataFound(true);
      continue;
    }
    if(name=="ORGANIZATION") {
      wave_data->setLabel(value);
      wave_data->setMetadataFound(true);
      continue;
    }
    // TODO: Parse the date field to get the year out
    //if(name == "DATE") { wave_data->setReleaseYear(value);
    //wave_data->setMetadataFound(true); continue; }
    if(name=="ISRC") { 
      wave_data->setIsrc(value); 
      wave_data->setMetadataFound(true); 
      continue; 
    }
  }
  if((!artist.isEmpty())&&(!composer.isEmpty())) {
    wave_data->setArtist(artist);
    wave_data->setComposer(composer);
  }
  else {
    if(!artist.isEmpty()) {
      wave_data->setArtist(artist);
    }
    else {
      wave_data->setArtist(composer);
    }
  }
  FLAC__metadata_object_delete(tags);
#endif  // HAVE_FLAC_METADATA
}


bool RDWaveFile::MakeFmt()
{
  if((format_tag!=WAVE_FORMAT_PCM)&&(format_tag!=WAVE_FORMAT_MPEG)) {
    return false;
  }
  if((channels!=1)&&(channels!=2)) {
    return false;
  }
  if(samples_per_sec==0) {
    return false;
  }
  if(format_tag==WAVE_FORMAT_PCM) {
    switch(bits_per_sample) {
	case 8:
	  block_align=channels;
	  break;
	case 16:
	  block_align=2*channels;
	  break;
	case 24:
	  block_align=3*channels;
	  break;
	case 32:
	  block_align=4*channels;
	  break;
	default:
	  return false;
	  break;
    }
    avg_bytes_per_sec=block_align*samples_per_sec;
    cb_size=0;
    WriteSword(fmt_chunk_data,0,format_tag);
    WriteSword(fmt_chunk_data,2,channels);
    WriteDword(fmt_chunk_data,4,samples_per_sec);
    WriteDword(fmt_chunk_data,8,avg_bytes_per_sec);
    WriteSword(fmt_chunk_data,12,block_align);
    WriteSword(fmt_chunk_data,14,bits_per_sample);
    WriteSword(fmt_chunk_data,16,cb_size);
    fmt_size=18;
    return true;
  }
  if(format_tag==WAVE_FORMAT_MPEG) {
    bits_per_sample=0;
    block_align=144*head_bit_rate/samples_per_sec;
    cb_size=40;
    if(head_layer==0) {
      return false;
    }
    if(head_bit_rate==0) {
      return false;
    }
    if((!mext_padding_used)&&((samples_per_sec==11025)||
			      (samples_per_sec==22050)||
			      (samples_per_sec==44100))) {
      avg_bytes_per_sec=samples_per_sec*block_align/1152;
    }
    else {
      avg_bytes_per_sec=head_bit_rate/8;
    }
    if(head_mode==0) {
      return false;
    }
    head_flags|=ACM_MPEG_ID_MPEG1;

    WriteSword(fmt_chunk_data,0,format_tag);
    WriteSword(fmt_chunk_data,2,channels);
    WriteDword(fmt_chunk_data,4,samples_per_sec);
    WriteDword(fmt_chunk_data,8,avg_bytes_per_sec);
    WriteSword(fmt_chunk_data,12,block_align);
    WriteSword(fmt_chunk_data,14,bits_per_sample);
    WriteSword(fmt_chunk_data,16,cb_size);
    WriteSword(fmt_chunk_data,18,head_layer);
    WriteDword(fmt_chunk_data,20,head_bit_rate);
    WriteSword(fmt_chunk_data,24,head_mode);
    WriteSword(fmt_chunk_data,26,head_mode_ext);
    WriteSword(fmt_chunk_data,28,head_emphasis);
    WriteSword(fmt_chunk_data,30,head_flags);
    WriteDword(fmt_chunk_data,32,0);
    WriteDword(fmt_chunk_data,36,0);
    fmt_size=40;
    return true;
  }    
  return false;
}


bool RDWaveFile::MakeCart()
{
  for(int i=0;i<CART_CHUNK_SIZE;i++) {
    cart_chunk_data[i]=0;
  }
  sprintf((char *)cart_chunk_data,"%4s",
	  CART_VERSION);
  if(!cart_title.isEmpty()) {
    sprintf((char *)cart_chunk_data+4,"%s",
	    (const char *)cart_title.left(64));
  }
  if(!cart_artist.isEmpty()) {
    sprintf((char *)cart_chunk_data+68,"%s",
	    (const char *)cart_artist.left(64));
  }
  if(!cart_cut_id.isEmpty()) {
    sprintf((char *)cart_chunk_data+132,"%s",
	    (const char *)cart_cut_id.left(64));
  }
  if(!cart_client_id.isEmpty()) {
    sprintf((char *)cart_chunk_data+196,"%s",
	    (const char *)cart_client_id.left(64));
  }
  if(!cart_category.isEmpty()) {
    sprintf((char *)cart_chunk_data+260,"%s",
	    (const char *)cart_category.left(64));
  }
  if(!cart_classification.isEmpty()) {
    sprintf((char *)cart_chunk_data+324,"%s",
	    (const char *)cart_classification.left(64));
  }
  if(!cart_out_cue.isEmpty()) {
    sprintf((char *)cart_chunk_data+388,"%s",
	    (const char *)cart_out_cue.left(64));
  }
  sprintf((char *)cart_chunk_data+452,"%04d/%02d/%02d",
	  cart_start_date.year(),
	  cart_start_date.month(),
	  cart_start_date.day());
  sprintf((char *)cart_chunk_data+462,"%02d:%02d:%02d",
	  cart_start_time.hour(),
	  cart_start_time.minute(),
	  cart_start_time.second());
  sprintf((char *)cart_chunk_data+470,"%04d/%02d/%02d",
	  cart_end_date.year(),
	  cart_end_date.month(),
	  cart_end_date.day());
  sprintf((char *)cart_chunk_data+480,"%02d:%02d:%02d",
	  cart_end_time.hour(),
	  cart_end_time.minute(),
	  cart_end_time.second());
  sprintf((char *)cart_chunk_data+488,"%s",
	  (const char *)PACKAGE);
  sprintf((char *)cart_chunk_data+552,"%s",
	  (const char *)VERSION);
  if(!cart_user_def.isEmpty()) {
    sprintf((char *)cart_chunk_data+616,"%s",
	    (const char *)cart_user_def.left(64));
  }
  WriteDword(cart_chunk_data,680,cart_level_ref);
  for(int i=0;i<MAX_TIMERS;i++) {
    if(!cart_timer_label[i].isEmpty()) {
      sprintf((char *)cart_chunk_data+684+i*MAX_TIMERS,"%4s",
	      (const char *)cart_timer_label[i].left(4));
      WriteDword(cart_chunk_data,688+i*MAX_TIMERS,cart_timer_sample[i]);
    }
  }
  if(!cart_url.isEmpty()) {
    sprintf((char *)cart_chunk_data+1020,"%s",
	    (const char *)cart_url.left(1024));
  }
  return true;
}


bool RDWaveFile::MakeBext()
{
  bext_coding_size=bext_coding_history.length()+BEXT_CHUNK_SIZE;
  bext_coding_data=(unsigned char *)realloc(bext_coding_data,bext_coding_size);

  for(int i=0;i<BEXT_CHUNK_SIZE;i++) {
    bext_coding_data[i]=0;
  }
  if(!bext_description.isEmpty()) {
    sprintf((char *)bext_coding_data,"%s",
	    (const char *)bext_description.left(256));
  }
  if(!bext_originator.isEmpty()) {
    sprintf((char *)bext_coding_data+256,"%s",
	    (const char *)bext_originator.left(32));
  }
  if(!bext_originator_ref.isEmpty()) {
    sprintf((char *)bext_coding_data+288,"%s",
	    (const char *)bext_originator_ref.left(32));
  }
  sprintf((char *)bext_coding_data+320,"%04d-%02d-%02d",
	  bext_origination_date.year(),
	  bext_origination_date.month(),
	  bext_origination_date.day());
  sprintf((char *)bext_coding_data+330,"%02d:%02d:%02d",
	  bext_origination_time.hour(),
	  bext_origination_time.minute(),
	  bext_origination_time.second());
  WriteDword(bext_coding_data,338,bext_time_reference_low);
  WriteDword(bext_coding_data,342,bext_time_reference_high);
  WriteSword(bext_coding_data,346,BWF_VERSION);
  for(int i=0;i<64;i++) {
    bext_coding_data[i+348]=bext_umid[i];
  }
  if(!bext_coding_history.isEmpty()) {
    sprintf((char *)bext_coding_data+602,"%s",
	    (const char *)bext_coding_history);
  }
  return true;
}


bool RDWaveFile::MakeMext()
{
  for(int i=0;i<MEXT_CHUNK_SIZE;i++) {
    mext_chunk_data[i]=0;
  }
  if(mext_homogenous) {
    mext_chunk_data[0]|=1;
  }
  if(!mext_padding_used) {
    mext_chunk_data[0]|=2;
  }
  if(mext_rate_hacked) {
    mext_chunk_data[0]|=4;
  }
  if(mext_free_format) {
    mext_chunk_data[0]|=8;
  }
  if(mext_homogenous) {
    WriteSword(mext_chunk_data,2,mext_frame_size);
  }
  WriteSword(mext_chunk_data,4,mext_anc_length);
  if(mext_left_energy) {
    mext_chunk_data[6]|=1;
  }
  if(mext_ancillary_private) {
    mext_chunk_data[6]|=2;
  }
  if(mext_right_energy) {
    mext_chunk_data[6]|=4;
  }
  return true;
}


bool RDWaveFile::MakeLevl()
{
  for(int i=0;i<LEVL_CHUNK_SIZE;i++) {
    levl_chunk_data[i]=0;
  }
  WriteDword(levl_chunk_data,0,levl_version);
  WriteDword(levl_chunk_data,4,levl_format);       // Format
  WriteDword(levl_chunk_data,8,levl_points);       // Points per Value
  WriteDword(levl_chunk_data,12,levl_block_size);  // Blocksize
  WriteDword(levl_chunk_data,16,levl_channels);    // Channels
  WriteDword(levl_chunk_data,20,levl_frames);      // Total Peak Values
  WriteDword(levl_chunk_data,24,levl_peak_offset); // Offset to Peak-of-Peaks
  WriteDword(levl_chunk_data,28,128);              // Offset to Peak Data
  sprintf((char *)levl_chunk_data+32,"%s",
	  (const char *)levl_timestamp.toString("yyyy:MM:dd:hh:mm:ss:000"));

  return true;
}


QString RDWaveFile::cutString(char *buffer,unsigned start_point,unsigned size)
{
  QString string;

  for(unsigned i=start_point;i<start_point+size;i++) {
    string=string.append(buffer[i]);
  }
  
  return string;
}
  



QDate RDWaveFile::cutDate(char *buffer,unsigned start_point)
{
  int i;
  char date_buf[11];
  int day=0,mon=0,year=0;

  for(i=0;i<4;i++) {
    date_buf[i]=buffer[start_point+i];
  }
  date_buf[4]=0;
  sscanf(date_buf,"%d",&year);
  if(year>RD_MAX_YEAR) {
    year=RD_MAX_YEAR;
  }
  for(i=0;i<2;i++) {
    date_buf[i]=buffer[start_point+i+5];
  }
  date_buf[2]=0;
  sscanf(date_buf,"%d",&mon);
  for(i=0;i<4;i++) {
    date_buf[i]=buffer[start_point+i+8];
  }
  date_buf[2]=0;
  sscanf(date_buf,"%d",&day);

  if((mon>0)&&(mon<13)&&(day>0)&&(day<32)) {
    return QDate(year,mon,day);
  }
  return QDate();
}




QTime RDWaveFile::cutTime(char *buffer,unsigned start_point)
{
  int i;
  char time_buf[9];
  int hour=0,min=0,sec=0;

  for(i=0;i<8;i++) {
    time_buf[i]=buffer[start_point+i];
  }
  time_buf[8]=0;
  sscanf(time_buf,"%d:%d:%d",&hour,&min,&sec);

  return QTime(hour,min,sec);
}


void RDWaveFile::WriteDword(unsigned char *buf,unsigned ptr,unsigned value)
{
  buf[ptr]=value&0xff;
  buf[ptr+1]=(value>>8)&0xff;
  buf[ptr+2]=(value>>16)&0xff;
  buf[ptr+3]=(value>>24)&0xff;
}


void RDWaveFile::WriteSword(unsigned char *buf,unsigned ptr,
			   unsigned short value)
{
  buf[ptr]=value&0xff;
  buf[ptr+1]=(value>>8)&0xff;
}


unsigned RDWaveFile::ReadDword(unsigned char *buffer,unsigned offset)
{
  return buffer[offset]+256*buffer[offset+1]+
    65536*buffer[offset+2]+16777216*buffer[offset+3];
}


unsigned short RDWaveFile::ReadSword(unsigned char *buffer,unsigned offset)
{
  return buffer[offset]+256*buffer[offset+1];
}


void RDWaveFile::GetEnergy()
{
  int file_ptr;

  if(energy_loaded) {
    return;
  }
  if(ReadEnergyFile(wave_file.name())){
    return;
  }
  if(!levl_chunk) {
    GetLevl(wave_file.handle());
  }
  file_ptr=lseek(wave_file.handle(),0,SEEK_CUR);
  lseek(wave_file.handle(),0,SEEK_SET);
  unsigned energy_size=LoadEnergy();
  ext_time_length=
   	       (unsigned)(1000.0*(((double)energy_size*1152)/getChannels())/(double)samples_per_sec);  
  energy_loaded=true;
  lseek(wave_file.handle(),file_ptr,SEEK_SET);
}


unsigned RDWaveFile::LoadEnergy()
{
  unsigned i=0;
  unsigned char block[5];
  char pcm[4608];
  int block_size;
  int offset;
  unsigned energy_size;

  short max=0;

  energy_data.clear();
  energy_size=getSampleLength()*getChannels()/1152;
  seekWave(0,SEEK_SET);
  switch(format_tag) {
/*      case WAVE_FORMAT_MPEG:
	if((head_layer==2)&&(mext_left_energy||mext_right_energy)) {
	  while(i<energy_size) {
	    lseek(wave_file.handle(),block_align-5,SEEK_CUR);
	    if(read(wave_file.handle(),block,5)<5) {
	      has_energy=true;
	      return i;
	    }
	    if(mext_left_energy) {
	      energy_data.push_back(block[4]+256*block[3]);
	      i++;
	    }
	    if(mext_right_energy) {
	      energy_data.push_back(block[1]+256*block[0]);
	      i++;
	    }
	  }
	  has_energy=true;
	  return i;
	}
	else {
	  has_energy=false;
	  return 0;
	}
	break;
*/
      case WAVE_FORMAT_PCM:
	block_size=2304*channels;
	while(i<energy_size) {
	  if(read(wave_file.handle(),pcm,block_size)!=block_size) {
	    has_energy=true;
	    return i;
	  }
	  for(int j=0;j<channels;j++) {
	    max=0;
	    energy_data.push_back(0);
	    for(int k=0;k<1152;k++) {
	      offset=2*k*channels+2*j;
	      if((pcm[offset]+256*pcm[offset+1])>energy_data[i]) {
		energy_data[i]=pcm[offset]+256*pcm[offset+1];
	      }
	    }
	    i++;
	  }
	}
	has_energy=true;
	return i;
	break;

      case WAVE_FORMAT_MPEG:
      case WAVE_FORMAT_VORBIS:
	block_size=2304*channels;
	while(i<energy_size) {
	  if(readWave(pcm,block_size)!=block_size) {
	    has_energy=true;
	    return i;
	  }
	  for(int j=0;j<channels;j++) {
	    max=0;
	    energy_data.push_back(0);
	    for(int k=0;k<1152;k++) {
	      offset=2*k*channels+2*j;
	      if((pcm[offset]+256*pcm[offset+1])>energy_data[i]) {
		energy_data[i]=pcm[offset]+256*pcm[offset+1];
	      }
	    }
	    i++;
	  }
	}
	has_energy=true;
	return i;
	break;

      default:
	has_energy=false;
	return 0;
	break;
  }
}


void RDWaveFile::GrowAlloc(size_t size)
{
  if(size>(size_t)cook_buffer_size) {
    cook_buffer=(unsigned char *)realloc(cook_buffer,size);
    cook_buffer_size=size;
  }
}


void RDWaveFile::ValidateMetadata()
{
  if(wave_data==NULL) {
    return;
  }

  if(!wave_data->metadataFound()) {
    return;
  }
  if(wave_data->startPos()<0) {
    wave_data->setStartPos(0);
  }
  if(wave_data->endPos()<0) {
    wave_data->setEndPos(ext_time_length);
  }
  if((wave_data->segueStartPos()>=0)&&(wave_data->segueEndPos()<0)) {
    wave_data->setSegueEndPos(wave_data->endPos());
  }
}


#ifdef HAVE_VORBIS
int RDWaveFile::WriteOggPage(ogg_page *page)
{
  int n;

  n=write(wave_file.handle(),page->header,page->header_len);
  n+=write(wave_file.handle(),page->body,page->body_len);
  return n;
}
#endif  // HAVE_VORBIS


int RDWaveFile::WriteOggBuffer(char *buf,int size)
{
#ifdef HAVE_VORBIS
  float **buffer=vorbis_analysis_buffer(&vorbis_dsp,size/channels);
  for(int i=0;i<(size/(2*channels));i++) {
    for(int j=0;j<channels;j++) {
/*
      buffer[j][i]=
	(float)((buf[2*channels*i+2*j]&0xff)|
		((buf[2*channels*i+2*j+1]&0xff)<<8))/32768.0f;
*/
/*
      if(buffer[j][i]>high) {
	high=buffer[j][i];
      }
*/
	    buffer[j][i] = ((buf[i*2*channels + 2*j + 1]<<8) |
			    (buf[i*2*channels + 2*j] & 0xff))/32768.0f;
            /* My compilier doesn't like these casts.
      buffer[j][i]=
	((float)(buf[2*channels*i+2*j]&0xff)+
	 (256.0f*(float)(buf[2*channels*i+2*j+1]&0xff)))/32768.0f;
         */
    }
//    printf("HIGH: %5.3f\n",high);
  }
  vorbis_analysis_wrote(&vorbis_dsp,size/(2*channels));
  while(vorbis_analysis_blockout(&vorbis_dsp,&vorbis_blk)==1) {
    vorbis_analysis(&vorbis_blk,NULL);
    vorbis_bitrate_addblock(&vorbis_blk);
    while(vorbis_bitrate_flushpacket(&vorbis_dsp,&ogg_pack)) {
      ogg_stream_packetin(&ogg_stream,&ogg_pack);
      while(ogg_stream_pageout(&ogg_stream,&ogg_pg)) {
	WriteOggPage(&ogg_pg);
      }
    }
  }
#endif  // HAVE_VORBIS
  return 0;
}
