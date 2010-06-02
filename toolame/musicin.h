/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
 *
 * $Id: musicin.h,v 1.2 2007/02/14 21:59:12 fredg Exp $
 *
 * $Log: musicin.h,v $
 * Revision 1.2  2007/02/14 21:59:12  fredg
 * 2007-02-14 Fred Gleason <fredg@paravelsystems.com>
 * 	* Changed the contact e-mail address in the copyright notices to
 * 	'fredg@paravelsystems.com'.
 *
 * Revision 1.1  2003/12/05 21:36:52  fredg
 * 2003-12-05 Fred Gleason <fredg@paravelsystems.com>
 * 	* Removed 'config.guess'
 * 	* Removed 'config.status'
 * 	* Removed 'config.sub'
 * 	* Removed 'install-sh'
 * 	* Removed 'ltmain.sh'
 * 	* Removed 'missing'
 * 	* Removed 'mkinstalldirs'
 * 	* Removed 'depcomp'
 * 	* Added 'autogen.sh'.
 * 	* Added the 'toolame' MPEG L2 encoder in 'toolame/'.
 *
 * Revision 1.1  1996/02/14 04:04:23  rowlands
 * Initial revision
 *
 * Received from Mike Coleman
 **********************************************************************/

#ifndef LOOP_DOT_H
#define LOOP_DOT_H
#include "common.h"

/**********************************************************************
 *   date   programmers                comment                        *
 * 25. 6.92  Toshiyuki Ishino          Ver 1.0                        *
 * 29.10.92  Masahiro Iwadare          Ver 2.0                        *
 * 17. 4.93  Masahiro Iwadare          Updated for IS Modification    *
 *                                                                    *
 *********************************************************************/

extern int cont_flag;

#define e              2.71828182845

#define CBLIMIT       21

#define SFB_LMAX 22
#define SFB_SMAX 13

extern int pretab[];

struct scalefac_struct
{
  int l[23];
  int s[14];
};

extern struct scalefac_struct sfBandIndex[];	/* Table B.8 -- in loop.c */

int nint (double in);

#define maximum(A,B) ( (A) > (B) ? (A) : (B) )
#define minimum(A,B) ( (A) < (B) ? (A) : (B) )
#define signum( A ) ( (A) > 0 ? 1 : -1 )

/* GLOBALE VARIABLE */

extern int bit_buffer[50000];

#endif
