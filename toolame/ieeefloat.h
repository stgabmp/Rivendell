#ifndef IEEE_FLOAT_H__
#define IEEE_FLOAT_H__
/* Copyright (C) 1988-1991 Apple Computer, Inc.
 * All Rights Reserved.
 *
 * Warranty Information
 * Even though Apple has reviewed this software, Apple makes no warranty
 * or representation, either express or implied, with respect to this
 * software, its quality, accuracy, merchantability, or fitness for a 
 * particular purpose.  As a result, this software is provided "as is,"
 * and you, its user, are assuming the entire risk as to its quality
 * and accuracy.
 *
 * This code may be used and freely distributed as long as it includes
 * this copyright notice and the warranty information.
 *
 * Machine-independent I/O routines for IEEE FLOATing-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *	Apple Macintosh, MPW 3.1 C compiler
 *	Apple Macintosh, THINK C compiler
 *	Silicon Graphics IRIS, MIPS compiler
 *	Cray X/MP and Y/MP
 *	Digital Equipment VAX
 *	Sequent Balance (Multiprocesor 386)
 *	NeXT
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * FLOATing-point format, and conversions to and from IEEE single-
 * precision FLOATing-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 *
 * $Id: ieeefloat.h,v 1.2 2007/02/14 21:59:12 fredg Exp $
 *
 * $Log: ieeefloat.h,v $
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
 * Revision 1.1  1993/06/11  17:45:46  malcolm
 * Initial revision
 *
 */

#include	<math.h>

typedef float Single;

#define	kFloatLength	4
#define	kDoubleLength	8
#define	kExtendedLength	10

double ConvertFromIeeeExtended (char *bytes);
#endif
