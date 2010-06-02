#include "common.h"
#include "encoder.h"

void IDCT32 (double *, double *, int);

/***********************************************************************
 An implementation of a modified window subband as seen in Kumar & Zubair's
"A high performance software implentation of mpeg audio encoder"
I think from IEEE ASCAP 1996 proceedings
 
input: shift in 32*12 (384) new samples into a 864 point buffer.
ch - which channel we're looking at.
 
This routine basically does 12 calls to window subband all in one go.
Not yet called in code. here for testing only.
************************************************************************/
#define NEWWS
void window_subband12 (short **buffer, int ch)
{
  static double x[2][864];	/* 2 channels, 864 buffer for each */
  double *xk;
  double t[12];			/* a temp buffer for summing values */
  double y[12][64];		/* 12 output arrays of 64 values */
  int i, j, k, m;
  static double c[512];		/* enwindow array */
  static int init = 0;
  double c0;

  if (!init) {
    read_ana_window (c);
    printf ("done init\n");
    init++;
  }

  xk = x[ch];			/* an easier way of referencing the array */

  /* shift 384 new samples into the buffer */
  for (i = 863; i >= 384; i--)
    xk[i] = xk[i - 384];
  for (i = 383; i >= 0; i--)
    xk[i] = (double) *(*buffer)++ / SCALE;

  for (j = 0; j < 64; j++) {
    for (k = 0; k < 12; k++)
      t[k] = 0;
    for (i = 0; i < 8; i++) {
      m = i * 64 + j;
      c0 = c[m];
      t[0] += c0 * xk[m + 352];
      t[1] += c0 * xk[m + 320];
      t[2] += c0 * xk[m + 288];
      t[3] += c0 * xk[m + 256];
      t[4] += c0 * xk[m + 224];
      t[5] += c0 * xk[m + 192];
      t[6] += c0 * xk[m + 160];
      t[7] += c0 * xk[m + 128];
      t[8] += c0 * xk[m + 96];
      t[9] += c0 * xk[m + 64];
      t[10] += c0 * xk[m + 32];
      t[11] += c0 * xk[m];
    }
    for (i = 0; i < 12; i++) {
      y[i][j] = t[i];
    }
  }
#define DB1x
#ifdef DB1
  for (i = 0; i < 12; i++) {
    printf ("--%i--\n", i);
    for (j = 0; j < 64; j++) {
      printf ("%f\t", y[i][j]);
      if ((j + 1) % 4 == 0)
	printf ("\n");
    }
  }
  exit (0);
#endif
}


/************************************************************************/
/*
/* read_ana_window()
/*
/* PURPOSE:  Reads encoder window file "enwindow" into array #ana_win#
/*
/************************************************************************/

void read_ana_window (ana_win)
     double ana_win[HAN_SIZE];
{
  int i, j[4];
  FILE *fp;
  double f[4];
  char t[150];

  if (!(fp = OpenTableFile ("enwindow"))) {
    printf ("Please check analysis window table 'enwindow'\n");
    exit (1);
  }
  for (i = 0; i < 512; i += 4) {
    fgets (t, 150, fp);
    sscanf (t, "C[%d] = %lf C[%d] = %lf C[%d] = %lf C[%d] = %lf\n", j, f,
	    j + 1, f + 1, j + 2, f + 2, j + 3, f + 3);
    if (i == j[0]) {
      ana_win[i] = f[0];
      ana_win[i + 1] = f[1];
      ana_win[i + 2] = f[2];
      ana_win[i + 3] = f[3];
    } else {
      printf ("Check index in analysis window table\n");
      exit (1);
    }
    fgets (t, 150, fp);
  }
  fclose (fp);
}

/************************************************************************/
/*
/* window_subband()
/*
/* PURPOSE:  Overlapping window on PCM samples
/*
/* SEMANTICS:
/* 32 16-bit pcm samples are scaled to fractional 2's complement and
/* concatenated to the end of the window buffer #x#. The updated window
/* buffer #x# is then windowed by the analysis window #c# to produce the
/* windowed sample #z#
/*
/************************************************************************/
#ifdef COMBWS
void window_subband (short **buffer, double s[SBLIMIT], int k, int sblimit)
{
  typedef double XX[2][HAN_SIZE];
  static XX *x;
  double *xk;
  int i;
  static int off[2] = { 0, 0 };
  static char init = 0;
  double t;
  static double enwindow[512];
  double *ep0, *ep1, *ep2, *ep3, *ep4, *ep5, *ep6, *ep7;
  double z[64];
  double yprime[32];
  if (!init) {
    read_ana_window (enwindow);
    x = (XX *) mem_alloc (sizeof (XX), "x");
    memset (x, 0, 2 * HAN_SIZE * sizeof (double));
    init = 1;
  }
  xk = (*x)[k];

  /* replace 32 oldest samples with 32 new samples */
  for (i = 0; i < 32; i++)
    xk[31 - i + off[k]] = (double) *(*buffer)++ / SCALE;

  ep0 = &enwindow[0];
  ep1 = &enwindow[64];
  ep2 = &enwindow[128];
  ep3 = &enwindow[192];
  ep4 = &enwindow[256];
  ep5 = &enwindow[320];
  ep6 = &enwindow[384];
  ep7 = &enwindow[448];

  /* shift samples into proper window positions */
  for (i = 0; i < 64; i++) {
    t = xk[(i + off[k]) & (512 - 1)] * *ep0++;
    t += xk[(i + 64 + off[k]) & (512 - 1)] * *ep1++;
    t += xk[(i + 128 + off[k]) & (512 - 1)] * *ep2++;
    t += xk[(i + 192 + off[k]) & (512 - 1)] * *ep3++;
    t += xk[(i + 256 + off[k]) & (512 - 1)] * *ep4++;
    t += xk[(i + 320 + off[k]) & (512 - 1)] * *ep5++;
    t += xk[(i + 384 + off[k]) & (512 - 1)] * *ep6++;
    t += xk[(i + 448 + off[k]) & (512 - 1)] * *ep7++;
    z[i] = t;
  }

  off[k] += 480;		/*offset is modulo (HAN_SIZE-1) */
  off[k] &= HAN_SIZE - 1;

  yprime[0] = z[16];
  for (i = 1; i <= 16; i++)
    yprime[i] = z[i + 16] + z[16 - i];
  for (i = 17; i <= 31; i++)
    yprime[i] = z[i + 16] - z[80 - i];
  IDCT32 (yprime, s, sblimit);
  /*    filter_subband (z,s);  */
}
#else
void window_subband (short **buffer, double z[64], int k)
{
  typedef double XX[2][HAN_SIZE];
  static XX *x;
  double *xk;
  int i;
  static int off[2] = { 0, 0 };
  static char init = 0;
  double t;
  static double enwindow[512];
  double *ep0, *ep1, *ep2, *ep3, *ep4, *ep5, *ep6, *ep7;
  if (!init) {
    read_ana_window (enwindow);
    x = (XX *) mem_alloc (sizeof (XX), "x");
    memset (x, 0, 2 * HAN_SIZE * sizeof (double));
    init = 1;
  }
  xk = (*x)[k];

  /* replace 32 oldest samples with 32 new samples */
  /* PDS old code:   */
  /* for (i=0;i<32;i++)
     xk[31-i+off[k]] = (double) *(*buffer)++/SCALE;
   */
  {
    register double *xk_t = xk + off[k];
    for (i = 32; i--;)
      xk_t[i] = (double) *(*buffer)++;
  }

  ep0 = &enwindow[0];
  ep1 = &enwindow[64];
  ep2 = &enwindow[128];
  ep3 = &enwindow[192];
  ep4 = &enwindow[256];
  ep5 = &enwindow[320];
  ep6 = &enwindow[384];
  ep7 = &enwindow[448];

  /* shift samples into proper window positions */
  for (i = 0; i < 64; i++) {
    t = xk[(i + off[k]) & (512 - 1)] * *ep0++;
    t += xk[(i + 64 + off[k]) & (512 - 1)] * *ep1++;
    t += xk[(i + 128 + off[k]) & (512 - 1)] * *ep2++;
    t += xk[(i + 192 + off[k]) & (512 - 1)] * *ep3++;
    t += xk[(i + 256 + off[k]) & (512 - 1)] * *ep4++;
    t += xk[(i + 320 + off[k]) & (512 - 1)] * *ep5++;
    t += xk[(i + 384 + off[k]) & (512 - 1)] * *ep6++;
    t += xk[(i + 448 + off[k]) & (512 - 1)] * *ep7++;
    z[i] = t;
  }

  off[k] += 480;		/*offset is modulo (HAN_SIZE-1) */
  off[k] &= HAN_SIZE - 1;

}
#endif
/************************************************************************/
/*
/* create_ana_filter()
/*
/* PURPOSE:  Calculates the analysis filter bank coefficients
/*
/* SEMANTICS:
/* Calculates the analysis filterbank coefficients and rounds to the
/* 9th decimal place accuracy of the filterbank tables in the ISO
/* document.  The coefficients are stored in #filter#
/*
/************************************************************************/

void create_ana_filter (filter)
     double filter[SBLIMIT][64];
{
  register int i, k;

  for (i = 0; i < 32; i++)
    for (k = 0; k < 64; k++) {
      if ((filter[i][k] =
	   1e9 * cos ((double) ((2 * i + 1) * (16 - k) * PI64))) >= 0)
	modf (filter[i][k] + 0.5, &filter[i][k]);
      else
	modf (filter[i][k] - 0.5, &filter[i][k]);
      filter[i][k] *= 1e-9;
    }
}

/************************************************************************
*
* filter_subband()
*
* PURPOSE:  Calculates the analysis filter bank coefficients
*
* SEMANTICS:
*      The windowed samples #z# is filtered by the digital filter matrix #m#
* to produce the subband samples #s#. This done by first selectively
* picking out values from the windowed samples, and then multiplying
* them by the filter matrix, producing 32 subband samples.
*
************************************************************************/
void create_dct_matrix (filter)
     double filter[16][32];
{
  register int i, k;

  for (i = 0; i < 16; i++)
    for (k = 0; k < 32; k++) {
      if ((filter[i][k] = 1e9 * cos ((double) ((2 * i + 1) * k * PI64))) >= 0)
	modf (filter[i][k] + 0.5, &filter[i][k]);
      else
	modf (filter[i][k] - 0.5, &filter[i][k]);
      filter[i][k] *= 1e-9;
      filter[i][k] /= (double) SCALE;	/* PDS */
    }
  /* PDS this code could/should be replaced; use simple cos  */
  /* and don't do additional rounding ??? See LAME(e.g.3.34) */
}

void IDCT32 (xin, xout, sblimit)
     double *xin, *xout;
     int sblimit;
{
  int i, j;
  double s0, s1;
  typedef double MM[16][32];
  static MM *m = 0;
  if (m == 0) {
    m = (MM *) mem_alloc (sizeof (MM), "filter");
    create_dct_matrix (*m);
  }
  /* Only compute subband filter info for frequency ranges which */
  /* will be really needed/encoded later. Code is "general", but */
  /* only produces speed up in low qual/bps coding situations.   */
  /* Added/adapted by PDS Oct 1999.                              */
  for (i = ((sblimit > 16) ? SBLIMIT - sblimit : sblimit); i--;) {
    s0 = 0.0;
    for (j = 0; j < 32; j++) {
      s0 += (*m)[i][j] * xin[j + 0];
    }
    xout[i] = s0;
  }
  for (i = SBLIMIT - sblimit; i < 16; i++) {
    s0 = s1 = 0.0;
    for (j = 0; j < 32; j += 2) {
      s0 += (*m)[i][j] * xin[j];
      s1 += (*m)[i][j + 1] * xin[j + 1];
    }
    xout[i] = s0 + s1;
    xout[31 - i] = s0 - s1;
  }
  /* PDS TODO: use pointers instead of arrays ???  */
  /* PDS TODO: is '--' j loop faster then original */
  /* code:   ' for( j=0; j<32; j+=2 )  { ... } ' ? */
}

void filter_subband (z, s, sblimit)
     double z[HAN_SIZE], s[SBLIMIT];
     int sblimit;
{
  double yprime[32];
  int i, j;

  {
    yprime[0] = z[16];
    for (i = 1; i <= 16; i++)
      yprime[i] = z[i + 16] + z[16 - i];
    for (i = 17; i <= 31; i++)
      yprime[i] = z[i + 16] - z[80 - i];
    IDCT32 (yprime, s, sblimit);
  }
}
