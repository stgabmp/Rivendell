/* rlm.h
 *
 * The Rivendell Loadable Module Interface
 *
 *   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * This interface can be used to create "Rivendell Loadable Modules" [RLMs]
 * that enable Rivendell's "Now & Next" capability to supply program associated
 * data [PAD] to external devices and systems.  Runtime module configuration
 * is accomplished in RDAdmin->ManageHosts->RDAirPlay->ConfigureNow&Next.
 *
 * Compiled plugins are dynamically loadable libraries (DLLs) and have names
 * of the form '<basename>.rlm'.  The following callbacks are provided:
 *
 *  void <basename>_RLMStart(void *ptr,const char *arg)
 *    Called once upon RDAirPlay startup.  The plugin should do any necessary
 *    startup tasks (opening i/o devices, allocating memory, etc) here.  The
 *    single argument 'arg' is a null-terminated string, consisting of the
 *    'Argument' parameter supplied in the specific runtime configuration
 *    from RDAdmin.  The 'ptr' argument is an opaque pointer that is
 *    used as the first argument to the utility functions.
 *
 *  void <basename>_RLMFree(void *ptr)
 *    Called once upon RDAirPlay shutdown.  Any system resources allocated
 *    by the plugin should be released here.  The 'ptr' argument is an opaque
 *    pointer that is be used as the first argument to any of the utility
 *    functions.
 *
 *
 *  void <basename>_RLMPadDataSent(void *ptr,const char *svcname,
 *                                 int onair,int lognum,
 *                                 const struct rlm_pad *now,
 *                                 const struct rlm_pad *next)
 *    Called each time RDAirPlay changes play state on a log.  The 'svcname' 
 *    parameter indicates the name of the service of the originating log.
 *    The 'lognum' parameter indicates the log machine, with 0=Main Log,
 *    1=Aux 1 Log and 2=Aux 2 Log.  The 'onair' parameter will be '0' if
 *    RDAirPlay's on-air flag is clear, and '1' if it is set.  The 'ptr'
 *    argument is an opaque pointer that is used as the first argument to
 *    the utility functions.
 *  
 *    WARNING: the structures provided in this callback are dynamically 
 *    allocated, their scope is valid only within the callback!
 *
 *  void <basename>_RLMTimerExpired(void *ptr,int timernum)
 *    Called each time the system RLM timer indicated by 'timernum' expires.
 *    See the 'RLMStartTimer()' and 'RLMStopTimer()' functions for info on
 *    using timers.  The 'ptr' argument is an opaque pointer that is
 *    used as the first argument to the utility functions.
 */

#ifndef RLM_H
#define RLM_H

#include <stdint.h>
#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * RLM Interface Version
 */
#define RLM_VERSION 1

/*
 * Available Timers
 */
#define RLM_MAX_TIMERS 32

/*
 * Timer Modes
 * (for use in the RLMStartTimer() function).
 */
#define RLM_TIMER_REPEATING 0
#define RLM_TIMER_ONESHOT 1

/*
 * Parity Modes
 * (for use in the RLMOpenSerial() function).
 */
#define RLM_PARITY_NONE 0
#define RLM_PARITY_EVEN 1
#define RLM_PARITY_ODD 2

/*
 * Metadata structure
 */
  struct rlm_pad {
    uint32_t rlm_cartnum;      /* Rivendell cart number */
    uint32_t rlm_len;          /* Event length, in milliseconds */
    char rlm_year[5];          /* Cart year */
    char rlm_group[11];        /* Rivendell Group Name */
    char rlm_title[256];       /* Cart 'title' field */
    char rlm_artist[256];      /* Cart 'artist' field */
    char rlm_label[65];        /* Cart 'label' field */
    char rlm_client[65];       /* Cart 'client' field */
    char rlm_agency[65];       /* Cart 'agency' field */
    char rlm_comp[65];         /* Cart 'composer' field */
    char rlm_pub[65];          /* Cart 'publisher' field */
    char rlm_userdef[256];     /* Cart 'user defined' field */
    char rlm_album[256];       /* Cart 'album' field */
    char reserved[673];        /* Reserved for future use */
  };
  
/*
 * Communications Functions
 *
 *
 * Send a UDP packet.  
 *
 * The <ipaddr> parameter is a null-terminated string consisting of the
 * IPv4 destination address in dotted-quad notation, and <port> is the
 * destination UDP port number.  All structures are in host (*not* network)
 * byte order.  The data to be sent, of length <len>, is pointed to by
 * <data>.
 */
  void RLMSendUdp(void *ptr,const char *ipaddr,uint16_t port,
		  const char *data,int len);

/*
 * Open a tty device (serial port) for output.
 *
 * The <devname> parameter is a null-terminated string consisting of the 
 * name of the tty device to open (e.g. "/dev/ttyS0").  The <speed>, 
 * <parity> and <word_length> parameters define the communications 
 * parameters to be used.
 *
 * RETURNS: if successful, a non-negative integer that should be used 
 * as the <handle> argument for the RLMSendSerial() and RLMCloseSerial()
 * functions.  If unsuccessful, a negative integer will be returned.
 */
  int RLMOpenSerial(void *ptr,const char *devname,int speed,
		    int parity,int word_length);

/*
 * Output data on a tty device.
 *
 * Output data of length <len> pointed to by <data> on the tty device
 * indicated by the <handle> value returned by the RLMOpenSerial() function.
 */
  void RLMSendSerial(void *ptr,int handle,const char *data,int len);

/*
 * Close a tty device.
 *
 * Close the tty device indicated by the <handle> value returned by the
 * RLMOpenSerial() function.
 */
  void RLMCloseSerial(void *ptr,int handle);

/*
 * Convienence Functions
 */

/*
 * Get a string indicating the system time.
 *
 * Returns a pointer to a null-terminated string indicating the system time,
 * formatted as per the <format> argument.  The following wildcards are 
 * supported:
 *      d 	the day as number without a leading zero (1-31)
 *     dd 	the day as number with a leading zero (01-31)
 *    ddd 	the abbreviated localized day name (e.g. 'Mon'..'Sun').
 *   dddd 	the long localized day name (e.g. 'Monday'..'Sunday').
 *      M 	the month as number without a leading zero (1-12)
 *     MM 	the month as number with a leading zero (01-12)
 *    MMM 	the abbreviated localized month name (e.g. 'Jan'..'Dec').
 *   MMMM 	the long localized month name (e.g. 'January'..'December').
 *     yy 	the year as two digit number (00-99)
 *   yyyy 	the year as four digit number (1752-8000)
 *      h 	the hour without a leading zero (0..23 or 1..12 if AM/PM 
 *              display)
 *     hh 	the hour with a leading zero (00..23 or 01..12 if AM/PM display)
 *      m 	the minute without a leading zero (0..59)
 *     mm 	the minute with a leading zero (00..59)
 *      s 	the second whithout a leading zero (0..59)
 *     ss 	the second whith a leading zero (00..59)
 *      z 	the milliseconds without leading zeroes (0..999)
 *    zzz 	the milliseconds with leading zeroes (000..999)
 *     AP 	use AM/PM display. AP will be replaced by either "AM" or "PM".
 *     ap 	use am/pm display. ap will be replaced by either "am" or "pm".
 *
 * RETURNS: A pointer to a null terminated string.  This string is statically
 * allocated, and may be reused in subsequent calls to the utility functions.
 */
  const char *RLMDateTime(void *ptr,int offset_msecs,const char *format);

/*
 * Resolve standard Rivendell Now & Next wildcards.
 *
 * Returns a pointer to a null-terminated string resulting from resolving
 * the 'standard' Rivendell Now & Next wildcards in accordance with the
 * data values in the <now> and <next> parameters.  The following wildcards
 * are supported:
 *
 *  Now  Next  Field
 *  ----------------------------------------------
 *   %n   %N   The Rivendell cart number
 *   %h   %H   Event length (in milliseconds)
 *   %g   %G   The Rivendell group name
 *   %t   %T   Title
 *   %a   %A   Artist
 *   %l   %L   Album
 *   %y   %Y   Year
 *   %b   %B   Record Label
 *   %c   %C   Client
 *   %e   %E   Agency
 *   %m   %M   Composer
 *   %p   %P   Publisher
 *   %u   %U   User Definied
 *
 * RETURNS: A pointer to a null terminated string.  This string is statically
 * allocated, and may be reused in subsequent calls to the utility functions.
 */
  const char *RLMResolveNowNext(void *ptr,const struct rlm_pad *now,
				const struct rlm_pad *next,const char *format);
  void RLMLog(void *ptr,int prio,const char *msg);
  void RLMStartTimer(void *ptr,int timernum,int msecs,int mode);
  void RLMStopTimer(void *ptr,int timernum);
  int RLMGetIntegerValue(void *ptr,const char *filename,const char *section,
			 const char *label,int default_value);
  int RLMGetHexValue(void *ptr,const char *filename,
		     const char *section,const char *label,int default_value);
  int RLMGetBooleanValue(void *ptr,const char *filename,
			 const char *section,const char *label,
			 int default_value);
  const char *RLMGetStringValue(void *ptr,const char *filename,
				const char *section,const char *label,
				const char *default_value);


#ifdef __cplusplus
}
#endif

#endif  /* RLM_H */
