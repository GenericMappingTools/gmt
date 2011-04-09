/*--------------------------------------------------------------------
 *	$Id: gmt_fft.c,v 1.5 2011-04-09 19:20:52 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Here are various ways to perform 1-D and 2-D Fourier transforms.
 * Most of these were provided by other people, as indicated below.
 * The GMT configure script will select one approach that is the fastest
 * on the particular hardware we are compiling for.  The default FFT
 * is the one we have had for a long time: a C-translation of the old
 * and venerable Norman Brenner (MIT) Fortran code.
 * Configure is expected to set constant GMT_FFT to one of these values:
 * FFTPACK	: Link with the FFT Pack of Swarztrauber, supplied here
 * FFTW		: Link with the FFTW library (supplied externally)
 * SUN4		: Link with the Sun4 performance library (Sun only)
 * VECLIB	: Link with the OSX Accelerate Framework (OSX only)
 * 
 * If not set we default to GMTs Brenner algorithm.
 *
 * Author:	Paul Wessel
 * Date:	1-APR-2011
 * Version:	5
 * THIS CODE IS NOT SET UP FOR 64-BIT; IT HAS A LOT OF INTS ETC
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_internals.h"

#if GMT_FFT == GMT_FFTPACK

/*
fftpack.c : A set of FFT routines in C.
Algorithmically based on Fortran-77 FFTPACK by Paul N. Swarztrauber (Version 4, 1985).
*/

/* isign is +1 for backward and -1 for forward transforms */

/*
#define DOUBLE
*/

#ifdef DOUBLE
#define Treal double
#else
#define Treal float
#endif

#define ref(u,a) u[a]

#define MAXFAC 13    /* maximum number of factors in factorization of n */

/* ----------------------------------------------------------------------
   passf2, passf3, passf4, passf5, passf. Complex FFT passes fwd and bwd.
---------------------------------------------------------------------- */

static void passf2(int ido, int l1, const Treal cc[], Treal ch[], const Treal wa1[], int isign)
  /* isign==+1 for backward transform */
  {
    int i, k, ah, ac;
    Treal ti2, tr2;
    if (ido <= 2) {
      for (k=0; k<l1; k++) {
        ah = k*ido;
        ac = 2*k*ido;
        ch[ah]              = ref(cc,ac) + ref(cc,ac + ido);
        ch[ah + ido*l1]     = ref(cc,ac) - ref(cc,ac + ido);
        ch[ah+1]            = ref(cc,ac+1) + ref(cc,ac + ido + 1);
        ch[ah + ido*l1 + 1] = ref(cc,ac+1) - ref(cc,ac + ido + 1);
      }
    } else {
      for (k=0; k<l1; k++) {
        for (i=0; i<ido-1; i+=2) {
          ah = i + k*ido;
          ac = i + 2*k*ido;
          ch[ah]   = ref(cc,ac) + ref(cc,ac + ido);
          tr2      = ref(cc,ac) - ref(cc,ac + ido);
          ch[ah+1] = ref(cc,ac+1) + ref(cc,ac + 1 + ido);
          ti2      = ref(cc,ac+1) - ref(cc,ac + 1 + ido);
          ch[ah+l1*ido+1] = wa1[i]*ti2 + isign*wa1[i+1]*tr2;
          ch[ah+l1*ido]   = wa1[i]*tr2 - isign*wa1[i+1]*ti2;
        }
      }
    }
  } /* passf2 */

static void passf3(int ido, int l1, const Treal cc[], Treal ch[],
      const Treal wa1[], const Treal wa2[], int isign)
  /* isign==+1 for backward transform */
  {
    static const Treal taur = -0.5;
    static const Treal taui = 0.866025403784439;
    int i, k, ac, ah;
    Treal ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;
    if (ido == 2) {
      for (k=1; k<=l1; k++) {
        ac = (3*k - 2)*ido;
        tr2 = ref(cc,ac) + ref(cc,ac + ido);
        cr2 = ref(cc,ac - ido) + taur*tr2;
        ah = (k - 1)*ido;
        ch[ah] = ref(cc,ac - ido) + tr2;

        ti2 = ref(cc,ac + 1) + ref(cc,ac + ido + 1);
        ci2 = ref(cc,ac - ido + 1) + taur*ti2;
        ch[ah + 1] = ref(cc,ac - ido + 1) + ti2;

        cr3 = isign*taui*(ref(cc,ac) - ref(cc,ac + ido));
        ci3 = isign*taui*(ref(cc,ac + 1) - ref(cc,ac + ido + 1));
        ch[ah + l1*ido] = cr2 - ci3;
        ch[ah + 2*l1*ido] = cr2 + ci3;
        ch[ah + l1*ido + 1] = ci2 + cr3;
        ch[ah + 2*l1*ido + 1] = ci2 - cr3;
      }
    } else {
      for (k=1; k<=l1; k++) {
        for (i=0; i<ido-1; i+=2) {
          ac = i + (3*k - 2)*ido;
          tr2 = ref(cc,ac) + ref(cc,ac + ido);
          cr2 = ref(cc,ac - ido) + taur*tr2;
          ah = i + (k-1)*ido;
          ch[ah] = ref(cc,ac - ido) + tr2;
          ti2 = ref(cc,ac + 1) + ref(cc,ac + ido + 1);
          ci2 = ref(cc,ac - ido + 1) + taur*ti2;
          ch[ah + 1] = ref(cc,ac - ido + 1) + ti2;
          cr3 = isign*taui*(ref(cc,ac) - ref(cc,ac + ido));
          ci3 = isign*taui*(ref(cc,ac + 1) - ref(cc,ac + ido + 1));
          dr2 = cr2 - ci3;
          dr3 = cr2 + ci3;
          di2 = ci2 + cr3;
          di3 = ci2 - cr3;
          ch[ah + l1*ido + 1] = wa1[i]*di2 + isign*wa1[i+1]*dr2;
          ch[ah + l1*ido] = wa1[i]*dr2 - isign*wa1[i+1]*di2;
          ch[ah + 2*l1*ido + 1] = wa2[i]*di3 + isign*wa2[i+1]*dr3;
          ch[ah + 2*l1*ido] = wa2[i]*dr3 - isign*wa2[i+1]*di3;
        }
      }
    }
  } /* passf3 */


static void passf4(int ido, int l1, const Treal cc[], Treal ch[],
      const Treal wa1[], const Treal wa2[], const Treal wa3[], int isign)
  /* isign == -1 for forward transform and +1 for backward transform */
  {
    int i, k, ac, ah;
    Treal ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, tr3, tr4;
    if (ido == 2) {
      for (k=0; k<l1; k++) {
        ac = 4*k*ido + 1;
        ti1 = ref(cc,ac) - ref(cc,ac + 2*ido);
        ti2 = ref(cc,ac) + ref(cc,ac + 2*ido);
        tr4 = ref(cc,ac + 3*ido) - ref(cc,ac + ido);
        ti3 = ref(cc,ac + ido) + ref(cc,ac + 3*ido);
        tr1 = ref(cc,ac - 1) - ref(cc,ac + 2*ido - 1);
        tr2 = ref(cc,ac - 1) + ref(cc,ac + 2*ido - 1);
        ti4 = ref(cc,ac + ido - 1) - ref(cc,ac + 3*ido - 1);
        tr3 = ref(cc,ac + ido - 1) + ref(cc,ac + 3*ido - 1);
        ah = k*ido;
        ch[ah] = tr2 + tr3;
        ch[ah + 2*l1*ido] = tr2 - tr3;
        ch[ah + 1] = ti2 + ti3;
        ch[ah + 2*l1*ido + 1] = ti2 - ti3;
        ch[ah + l1*ido] = tr1 + isign*tr4;
        ch[ah + 3*l1*ido] = tr1 - isign*tr4;
        ch[ah + l1*ido + 1] = ti1 + isign*ti4;
        ch[ah + 3*l1*ido + 1] = ti1 - isign*ti4;
      }
    } else {
      for (k=0; k<l1; k++) {
        for (i=0; i<ido-1; i+=2) {
          ac = i + 1 + 4*k*ido;
          ti1 = ref(cc,ac) - ref(cc,ac + 2*ido);
          ti2 = ref(cc,ac) + ref(cc,ac + 2*ido);
          ti3 = ref(cc,ac + ido) + ref(cc,ac + 3*ido);
          tr4 = ref(cc,ac + 3*ido) - ref(cc,ac + ido);
          tr1 = ref(cc,ac - 1) - ref(cc,ac + 2*ido - 1);
          tr2 = ref(cc,ac - 1) + ref(cc,ac + 2*ido - 1);
          ti4 = ref(cc,ac + ido - 1) - ref(cc,ac + 3*ido - 1);
          tr3 = ref(cc,ac + ido - 1) + ref(cc,ac + 3*ido - 1);
          ah = i + k*ido;
          ch[ah] = tr2 + tr3;
          cr3 = tr2 - tr3;
          ch[ah + 1] = ti2 + ti3;
          ci3 = ti2 - ti3;
          cr2 = tr1 + isign*tr4;
          cr4 = tr1 - isign*tr4;
          ci2 = ti1 + isign*ti4;
          ci4 = ti1 - isign*ti4;
          ch[ah + l1*ido] = wa1[i]*cr2 - isign*wa1[i + 1]*ci2;
          ch[ah + l1*ido + 1] = wa1[i]*ci2 + isign*wa1[i + 1]*cr2;
          ch[ah + 2*l1*ido] = wa2[i]*cr3 - isign*wa2[i + 1]*ci3;
          ch[ah + 2*l1*ido + 1] = wa2[i]*ci3 + isign*wa2[i + 1]*cr3;
          ch[ah + 3*l1*ido] = wa3[i]*cr4 -isign*wa3[i + 1]*ci4;
          ch[ah + 3*l1*ido + 1] = wa3[i]*ci4 + isign*wa3[i + 1]*cr4;
        }
      }
    }
  } /* passf4 */


static void passf5(int ido, int l1, const Treal cc[], Treal ch[],
      const Treal wa1[], const Treal wa2[], const Treal wa3[], const Treal wa4[], int isign)
  /* isign == -1 for forward transform and +1 for backward transform */
  {
    static const Treal tr11 = 0.309016994374947;
    static const Treal ti11 = 0.951056516295154;
    static const Treal tr12 = -0.809016994374947;
    static const Treal ti12 = 0.587785252292473;
    int i, k, ac, ah;
    Treal ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, ti2, ti3,
        ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;
    if (ido == 2) {
      for (k = 1; k <= l1; ++k) {
        ac = (5*k - 4)*ido + 1;
        ti5 = ref(cc,ac) - ref(cc,ac + 3*ido);
        ti2 = ref(cc,ac) + ref(cc,ac + 3*ido);
        ti4 = ref(cc,ac + ido) - ref(cc,ac + 2*ido);
        ti3 = ref(cc,ac + ido) + ref(cc,ac + 2*ido);
        tr5 = ref(cc,ac - 1) - ref(cc,ac + 3*ido - 1);
        tr2 = ref(cc,ac - 1) + ref(cc,ac + 3*ido - 1);
        tr4 = ref(cc,ac + ido - 1) - ref(cc,ac + 2*ido - 1);
        tr3 = ref(cc,ac + ido - 1) + ref(cc,ac + 2*ido - 1);
        ah = (k - 1)*ido;
        ch[ah] = ref(cc,ac - ido - 1) + tr2 + tr3;
        ch[ah + 1] = ref(cc,ac - ido) + ti2 + ti3;
        cr2 = ref(cc,ac - ido - 1) + tr11*tr2 + tr12*tr3;
        ci2 = ref(cc,ac - ido) + tr11*ti2 + tr12*ti3;
        cr3 = ref(cc,ac - ido - 1) + tr12*tr2 + tr11*tr3;
        ci3 = ref(cc,ac - ido) + tr12*ti2 + tr11*ti3;
        cr5 = isign*(ti11*tr5 + ti12*tr4);
        ci5 = isign*(ti11*ti5 + ti12*ti4);
        cr4 = isign*(ti12*tr5 - ti11*tr4);
        ci4 = isign*(ti12*ti5 - ti11*ti4);
        ch[ah + l1*ido] = cr2 - ci5;
        ch[ah + 4*l1*ido] = cr2 + ci5;
        ch[ah + l1*ido + 1] = ci2 + cr5;
        ch[ah + 2*l1*ido + 1] = ci3 + cr4;
        ch[ah + 2*l1*ido] = cr3 - ci4;
        ch[ah + 3*l1*ido] = cr3 + ci4;
        ch[ah + 3*l1*ido + 1] = ci3 - cr4;
        ch[ah + 4*l1*ido + 1] = ci2 - cr5;
      }
    } else {
      for (k=1; k<=l1; k++) {
        for (i=0; i<ido-1; i+=2) {
          ac = i + 1 + (k*5 - 4)*ido;
          ti5 = ref(cc,ac) - ref(cc,ac + 3*ido);
          ti2 = ref(cc,ac) + ref(cc,ac + 3*ido);
          ti4 = ref(cc,ac + ido) - ref(cc,ac + 2*ido);
          ti3 = ref(cc,ac + ido) + ref(cc,ac + 2*ido);
          tr5 = ref(cc,ac - 1) - ref(cc,ac + 3*ido - 1);
          tr2 = ref(cc,ac - 1) + ref(cc,ac + 3*ido - 1);
          tr4 = ref(cc,ac + ido - 1) - ref(cc,ac + 2*ido - 1);
          tr3 = ref(cc,ac + ido - 1) + ref(cc,ac + 2*ido - 1);
          ah = i + (k - 1)*ido;
          ch[ah] = ref(cc,ac - ido - 1) + tr2 + tr3;
          ch[ah + 1] = ref(cc,ac - ido) + ti2 + ti3;
          cr2 = ref(cc,ac - ido - 1) + tr11*tr2 + tr12*tr3;

          ci2 = ref(cc,ac - ido) + tr11*ti2 + tr12*ti3;
          cr3 = ref(cc,ac - ido - 1) + tr12*tr2 + tr11*tr3;

          ci3 = ref(cc,ac - ido) + tr12*ti2 + tr11*ti3;
          cr5 = isign*(ti11*tr5 + ti12*tr4);
          ci5 = isign*(ti11*ti5 + ti12*ti4);
          cr4 = isign*(ti12*tr5 - ti11*tr4);
          ci4 = isign*(ti12*ti5 - ti11*ti4);
          dr3 = cr3 - ci4;
          dr4 = cr3 + ci4;
          di3 = ci3 + cr4;
          di4 = ci3 - cr4;
          dr5 = cr2 + ci5;
          dr2 = cr2 - ci5;
          di5 = ci2 - cr5;
          di2 = ci2 + cr5;
          ch[ah + l1*ido] = wa1[i]*dr2 - isign*wa1[i+1]*di2;
          ch[ah + l1*ido + 1] = wa1[i]*di2 + isign*wa1[i+1]*dr2;
          ch[ah + 2*l1*ido] = wa2[i]*dr3 - isign*wa2[i+1]*di3;
          ch[ah + 2*l1*ido + 1] = wa2[i]*di3 + isign*wa2[i+1]*dr3;
          ch[ah + 3*l1*ido] = wa3[i]*dr4 - isign*wa3[i+1]*di4;
          ch[ah + 3*l1*ido + 1] = wa3[i]*di4 + isign*wa3[i+1]*dr4;
          ch[ah + 4*l1*ido] = wa4[i]*dr5 - isign*wa4[i+1]*di5;
          ch[ah + 4*l1*ido + 1] = wa4[i]*di5 + isign*wa4[i+1]*dr5;
        }
      }
    }
  } /* passf5 */


static void passf(int *nac, int ido, int ip, int l1, int idl1,
      Treal cc[], Treal ch[],
      const Treal wa[], int isign)
  /* isign is -1 for forward transform and +1 for backward transform */
  {
    int idij, idlj, idot, ipph, i, j, k, l, jc, lc, ik, nt, idj, idl, inc,idp;
    Treal wai, war;

    idot = ido / 2;
    nt = ip*idl1;
    ipph = (ip + 1) / 2;
    idp = ip*ido;
    if (ido >= l1) {
      for (j=1; j<ipph; j++) {
        jc = ip - j;
        for (k=0; k<l1; k++) {
          for (i=0; i<ido; i++) {
            ch[i + (k + j*l1)*ido] =
                ref(cc,i + (j + k*ip)*ido) + ref(cc,i + (jc + k*ip)*ido);
            ch[i + (k + jc*l1)*ido] =
                ref(cc,i + (j + k*ip)*ido) - ref(cc,i + (jc + k*ip)*ido);
          }
        }
      }
      for (k=0; k<l1; k++)
        for (i=0; i<ido; i++)
          ch[i + k*ido] = ref(cc,i + k*ip*ido);
    } else {
      for (j=1; j<ipph; j++) {
        jc = ip - j;
        for (i=0; i<ido; i++) {
          for (k=0; k<l1; k++) {
            ch[i + (k + j*l1)*ido] = ref(cc,i + (j + k*ip)*ido) + ref(cc,i + (jc + k*
                ip)*ido);
            ch[i + (k + jc*l1)*ido] = ref(cc,i + (j + k*ip)*ido) - ref(cc,i + (jc + k*
                ip)*ido);
          }
        }
      }
      for (i=0; i<ido; i++)
        for (k=0; k<l1; k++)
          ch[i + k*ido] = ref(cc,i + k*ip*ido);
    }

    idl = 2 - ido;
    inc = 0;
    for (l=1; l<ipph; l++) {
      lc = ip - l;
      idl += ido;
      for (ik=0; ik<idl1; ik++) {
        cc[ik + l*idl1] = ch[ik] + wa[idl - 2]*ch[ik + idl1];
        cc[ik + lc*idl1] = isign*wa[idl-1]*ch[ik + (ip-1)*idl1];
      }
      idlj = idl;
      inc += ido;
      for (j=2; j<ipph; j++) {
        jc = ip - j;
        idlj += inc;
        if (idlj > idp) idlj -= idp;
        war = wa[idlj - 2];
        wai = wa[idlj-1];
        for (ik=0; ik<idl1; ik++) {
          cc[ik + l*idl1] += war*ch[ik + j*idl1];
          cc[ik + lc*idl1] += isign*wai*ch[ik + jc*idl1];
        }
      }
    }
    for (j=1; j<ipph; j++)
      for (ik=0; ik<idl1; ik++)
        ch[ik] += ch[ik + j*idl1];
    for (j=1; j<ipph; j++) {
      jc = ip - j;
      for (ik=1; ik<idl1; ik+=2) {
        ch[ik - 1 + j*idl1] = cc[ik - 1 + j*idl1] - cc[ik + jc*idl1];
        ch[ik - 1 + jc*idl1] = cc[ik - 1 + j*idl1] + cc[ik + jc*idl1];
        ch[ik + j*idl1] = cc[ik + j*idl1] + cc[ik - 1 + jc*idl1];
        ch[ik + jc*idl1] = cc[ik + j*idl1] - cc[ik - 1 + jc*idl1];
      }
    }
    *nac = 1;
    if (ido == 2) return;
    *nac = 0;
    for (ik=0; ik<idl1; ik++)
      cc[ik] = ch[ik];
    for (j=1; j<ip; j++) {
      for (k=0; k<l1; k++) {
        cc[(k + j*l1)*ido + 0] = ch[(k + j*l1)*ido + 0];
        cc[(k + j*l1)*ido + 1] = ch[(k + j*l1)*ido + 1];
      }
    }
    if (idot <= l1) {
      idij = 0;
      for (j=1; j<ip; j++) {
        idij += 2;
        for (i=3; i<ido; i+=2) {
          idij += 2;
          for (k=0; k<l1; k++) {
            cc[i - 1 + (k + j*l1)*ido] =
                wa[idij - 2]*ch[i - 1 + (k + j*l1)*ido] -
                isign*wa[idij-1]*ch[i + (k + j*l1)*ido];
            cc[i + (k + j*l1)*ido] =
                wa[idij - 2]*ch[i + (k + j*l1)*ido] +
                isign*wa[idij-1]*ch[i - 1 + (k + j*l1)*ido];
          }
        }
      }
    } else {
      idj = 2 - ido;
      for (j=1; j<ip; j++) {
        idj += ido;
        for (k = 0; k < l1; k++) {
          idij = idj;
          for (i=3; i<ido; i+=2) {
            idij += 2;
            cc[i - 1 + (k + j*l1)*ido] =
                wa[idij - 2]*ch[i - 1 + (k + j*l1)*ido] -
                isign*wa[idij-1]*ch[i + (k + j*l1)*ido];
            cc[i + (k + j*l1)*ido] =
                wa[idij - 2]*ch[i + (k + j*l1)*ido] +
                isign*wa[idij-1]*ch[i - 1 + (k + j*l1)*ido];
          }
        }
      }
    }
  } /* passf */


  /* ----------------------------------------------------------------------
radf2,radb2, radf3,radb3, radf4,radb4, radf5,radb5, radfg,radbg.
Treal FFT passes fwd and bwd.
---------------------------------------------------------------------- */

static void radf2(int ido, int l1, const Treal cc[], Treal ch[], const Treal wa1[])
  {
    int i, k, ic;
    Treal ti2, tr2;
    for (k=0; k<l1; k++) {
      ch[2*k*ido] =
          ref(cc,k*ido) + ref(cc,(k + l1)*ido);
      ch[(2*k+1)*ido + ido-1] =
          ref(cc,k*ido) - ref(cc,(k + l1)*ido);
    }
    if (ido < 2) return;
    if (ido != 2) {
      for (k=0; k<l1; k++) {
        for (i=2; i<ido; i+=2) {
          ic = ido - i;
          tr2 = wa1[i - 2]*ref(cc, i-1 + (k + l1)*ido) + wa1[i - 1]*ref(cc, i + (k + l1)*ido);
          ti2 = wa1[i - 2]*ref(cc, i + (k + l1)*ido) - wa1[i - 1]*ref(cc, i-1 + (k + l1)*ido);
          ch[i + 2*k*ido] = ref(cc,i + k*ido) + ti2;
          ch[ic + (2*k+1)*ido] = ti2 - ref(cc,i + k*ido);
          ch[i - 1 + 2*k*ido] = ref(cc,i - 1 + k*ido) + tr2;
          ch[ic - 1 + (2*k+1)*ido] = ref(cc,i - 1 + k*ido) - tr2;
        }
      }
      if (ido % 2 == 1) return;
    }
    for (k=0; k<l1; k++) {
      ch[(2*k+1)*ido] = -ref(cc,ido-1 + (k + l1)*ido);
      ch[ido-1 + 2*k*ido] = ref(cc,ido-1 + k*ido);
    }
  } /* radf2 */


static void radb2(int ido, int l1, const Treal cc[], Treal ch[], const Treal wa1[])
  {
    int i, k, ic;
    Treal ti2, tr2;
    for (k=0; k<l1; k++) {
      ch[k*ido] =
          ref(cc,2*k*ido) + ref(cc,ido-1 + (2*k+1)*ido);
      ch[(k + l1)*ido] =
          ref(cc,2*k*ido) - ref(cc,ido-1 + (2*k+1)*ido);
    }
    if (ido < 2) return;
    if (ido != 2) {
      for (k = 0; k < l1; ++k) {
        for (i = 2; i < ido; i += 2) {
          ic = ido - i;
          ch[i-1 + k*ido] =
              ref(cc,i-1 + 2*k*ido) + ref(cc,ic-1 + (2*k+1)*ido);
          tr2 = ref(cc,i-1 + 2*k*ido) - ref(cc,ic-1 + (2*k+1)*ido);
          ch[i + k*ido] =
              ref(cc,i + 2*k*ido) - ref(cc,ic + (2*k+1)*ido);
          ti2 = ref(cc,i + (2*k)*ido) + ref(cc,ic + (2*k+1)*ido);
          ch[i-1 + (k + l1)*ido] =
              wa1[i - 2]*tr2 - wa1[i - 1]*ti2;
          ch[i + (k + l1)*ido] =
              wa1[i - 2]*ti2 + wa1[i - 1]*tr2;
        }
      }
      if (ido % 2 == 1) return;
    }
    for (k = 0; k < l1; k++) {
      ch[ido-1 + k*ido] = 2*ref(cc,ido-1 + 2*k*ido);
      ch[ido-1 + (k + l1)*ido] = -2*ref(cc,(2*k+1)*ido);
    }
  } /* radb2 */


static void radf3(int ido, int l1, const Treal cc[], Treal ch[],
      const Treal wa1[], const Treal wa2[])
  {
    static const Treal taur = -0.5;
    static const Treal taui = 0.866025403784439;
    int i, k, ic;
    Treal ci2, di2, di3, cr2, dr2, dr3, ti2, ti3, tr2, tr3;
    for (k=0; k<l1; k++) {
      cr2 = ref(cc,(k + l1)*ido) + ref(cc,(k + 2*l1)*ido);
      ch[3*k*ido] = ref(cc,k*ido) + cr2;
      ch[(3*k+2)*ido] = taui*(ref(cc,(k + l1*2)*ido) - ref(cc,(k + l1)*ido));
      ch[ido-1 + (3*k + 1)*ido] = ref(cc,k*ido) + taur*cr2;
    }
    if (ido == 1) return;
    for (k=0; k<l1; k++) {
      for (i=2; i<ido; i+=2) {
        ic = ido - i;
        dr2 = wa1[i - 2]*ref(cc,i - 1 + (k + l1)*ido) +
            wa1[i - 1]*ref(cc,i + (k + l1)*ido);
        di2 = wa1[i - 2]*ref(cc,i + (k + l1)*ido) - wa1[i - 1]*ref(cc,i - 1 + (k + l1)*ido);
        dr3 = wa2[i - 2]*ref(cc,i - 1 + (k + l1*2)*ido) + wa2[i - 1]*ref(cc,i + (k + l1*2)*ido);
        di3 = wa2[i - 2]*ref(cc,i + (k + l1*2)*ido) - wa2[i - 1]*ref(cc,i - 1 + (k + l1*2)*ido);
        cr2 = dr2 + dr3;
        ci2 = di2 + di3;
        ch[i - 1 + 3*k*ido] = ref(cc,i - 1 + k*ido) + cr2;
        ch[i + 3*k*ido] = ref(cc,i + k*ido) + ci2;
        tr2 = ref(cc,i - 1 + k*ido) + taur*cr2;
        ti2 = ref(cc,i + k*ido) + taur*ci2;
        tr3 = taui*(di2 - di3);
        ti3 = taui*(dr3 - dr2);
        ch[i - 1 + (3*k + 2)*ido] = tr2 + tr3;
        ch[ic - 1 + (3*k + 1)*ido] = tr2 - tr3;
        ch[i + (3*k + 2)*ido] = ti2 + ti3;
        ch[ic + (3*k + 1)*ido] = ti3 - ti2;
      }
    }
  } /* radf3 */


static void radb3(int ido, int l1, const Treal cc[], Treal ch[],
      const Treal wa1[], const Treal wa2[])
  {
    static const Treal taur = -0.5;
    static const Treal taui = 0.866025403784439;
    int i, k, ic;
    Treal ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;
    for (k=0; k<l1; k++) {
      tr2 = 2*ref(cc,ido-1 + (3*k + 1)*ido);
      cr2 = ref(cc,3*k*ido) + taur*tr2;
      ch[k*ido] = ref(cc,3*k*ido) + tr2;
      ci3 = 2*taui*ref(cc,(3*k + 2)*ido);
      ch[(k + l1)*ido] = cr2 - ci3;
      ch[(k + 2*l1)*ido] = cr2 + ci3;
    }
    if (ido == 1) return;
    for (k=0; k<l1; k++) {
      for (i=2; i<ido; i+=2) {
        ic = ido - i;
        tr2 = ref(cc,i - 1 + (3*k + 2)*ido) + ref(cc,ic - 1 + (3*k + 1)*ido);
        cr2 = ref(cc,i - 1 + 3*k*ido) + taur*tr2;
        ch[i - 1 + k*ido] = ref(cc,i - 1 + 3*k*ido) + tr2;
        ti2 = ref(cc,i + (3*k + 2)*ido) - ref(cc,ic + (3*k + 1)*ido);
        ci2 = ref(cc,i + 3*k*ido) + taur*ti2;
        ch[i + k*ido] = ref(cc,i + 3*k*ido) + ti2;
        cr3 = taui*(ref(cc,i - 1 + (3*k + 2)*ido) - ref(cc,ic - 1 + (3*k + 1)*ido));
        ci3 = taui*(ref(cc,i + (3*k + 2)*ido) + ref(cc,ic + (3*k + 1)*ido));
        dr2 = cr2 - ci3;
        dr3 = cr2 + ci3;
        di2 = ci2 + cr3;
        di3 = ci2 - cr3;
        ch[i - 1 + (k + l1)*ido] = wa1[i - 2]*dr2 - wa1[i - 1]*di2;
        ch[i + (k + l1)*ido] = wa1[i - 2]*di2 + wa1[i - 1]*dr2;
        ch[i - 1 + (k + 2*l1)*ido] = wa2[i - 2]*dr3 - wa2[i - 1]*di3;
        ch[i + (k + 2*l1)*ido] = wa2[i - 2]*di3 + wa2[i - 1]*dr3;
      }
    }
  } /* radb3 */


static void radf4(int ido, int l1, const Treal cc[], Treal ch[],
      const Treal wa1[], const Treal wa2[], const Treal wa3[])
  {
    static const Treal hsqt2 = 0.7071067811865475;
    int i, k, ic;
    Treal ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, tr3, tr4;
    for (k=0; k<l1; k++) {
      tr1 = ref(cc,(k + l1)*ido) + ref(cc,(k + 3*l1)*ido);
      tr2 = ref(cc,k*ido) + ref(cc,(k + 2*l1)*ido);
      ch[4*k*ido] = tr1 + tr2;
      ch[ido-1 + (4*k + 3)*ido] = tr2 - tr1;
      ch[ido-1 + (4*k + 1)*ido] = ref(cc,k*ido) - ref(cc,(k + 2*l1)*ido);
      ch[(4*k + 2)*ido] = ref(cc,(k + 3*l1)*ido) - ref(cc,(k + l1)*ido);
    }
    if (ido < 2) return;
    if (ido != 2) {
      for (k=0; k<l1; k++) {
        for (i=2; i<ido; i += 2) {
          ic = ido - i;
          cr2 = wa1[i - 2]*ref(cc,i - 1 + (k + l1)*ido) + wa1[i - 1]*ref(cc,i + (k + l1)*ido);
          ci2 = wa1[i - 2]*ref(cc,i + (k + l1)*ido) - wa1[i - 1]*ref(cc,i - 1 + (k + l1)*ido);
          cr3 = wa2[i - 2]*ref(cc,i - 1 + (k + 2*l1)*ido) + wa2[i - 1]*ref(cc,i + (k + 2*l1)*
              ido);
          ci3 = wa2[i - 2]*ref(cc,i + (k + 2*l1)*ido) - wa2[i - 1]*ref(cc,i - 1 + (k + 2*l1)*
              ido);
          cr4 = wa3[i - 2]*ref(cc,i - 1 + (k + 3*l1)*ido) + wa3[i - 1]*ref(cc,i + (k + 3*l1)*
              ido);
          ci4 = wa3[i - 2]*ref(cc,i + (k + 3*l1)*ido) - wa3[i - 1]*ref(cc,i - 1 + (k + 3*l1)*
              ido);
          tr1 = cr2 + cr4;
          tr4 = cr4 - cr2;
          ti1 = ci2 + ci4;
          ti4 = ci2 - ci4;
          ti2 = ref(cc,i + k*ido) + ci3;
          ti3 = ref(cc,i + k*ido) - ci3;
          tr2 = ref(cc,i - 1 + k*ido) + cr3;
          tr3 = ref(cc,i - 1 + k*ido) - cr3;
          ch[i - 1 + 4*k*ido] = tr1 + tr2;
          ch[ic - 1 + (4*k + 3)*ido] = tr2 - tr1;
          ch[i + 4*k*ido] = ti1 + ti2;
          ch[ic + (4*k + 3)*ido] = ti1 - ti2;
          ch[i - 1 + (4*k + 2)*ido] = ti4 + tr3;
          ch[ic - 1 + (4*k + 1)*ido] = tr3 - ti4;
          ch[i + (4*k + 2)*ido] = tr4 + ti3;
          ch[ic + (4*k + 1)*ido] = tr4 - ti3;
        }
      }
      if (ido % 2 == 1) return;
    }
    for (k=0; k<l1; k++) {
      ti1 = -hsqt2*(ref(cc,ido-1 + (k + l1)*ido) + ref(cc,ido-1 + (k + 3*l1)*ido));
      tr1 = hsqt2*(ref(cc,ido-1 + (k + l1)*ido) - ref(cc,ido-1 + (k + 3*l1)*ido));
      ch[ido-1 + 4*k*ido] = tr1 + ref(cc,ido-1 + k*ido);
      ch[ido-1 + (4*k + 2)*ido] = ref(cc,ido-1 + k*ido) - tr1;
      ch[(4*k + 1)*ido] = ti1 - ref(cc,ido-1 + (k + 2*l1)*ido);
      ch[(4*k + 3)*ido] = ti1 + ref(cc,ido-1 + (k + 2*l1)*ido);
    }
  } /* radf4 */


static void radb4(int ido, int l1, const Treal cc[], Treal ch[],
      const Treal wa1[], const Treal wa2[], const Treal wa3[])
  {
    static const Treal sqrt2 = 1.414213562373095;
    int i, k, ic;
    Treal ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, tr3, tr4;
    for (k = 0; k < l1; k++) {
      tr1 = ref(cc,4*k*ido) - ref(cc,ido-1 + (4*k + 3)*ido);
      tr2 = ref(cc,4*k*ido) + ref(cc,ido-1 + (4*k + 3)*ido);
      tr3 = ref(cc,ido-1 + (4*k + 1)*ido) + ref(cc,ido-1 + (4*k + 1)*ido);
      tr4 = ref(cc,(4*k + 2)*ido) + ref(cc,(4*k + 2)*ido);
      ch[k*ido] = tr2 + tr3;
      ch[(k + l1)*ido] = tr1 - tr4;
      ch[(k + 2*l1)*ido] = tr2 - tr3;
      ch[(k + 3*l1)*ido] = tr1 + tr4;
    }
    if (ido < 2) return;
    if (ido != 2) {
      for (k = 0; k < l1; ++k) {
        for (i = 2; i < ido; i += 2) {
          ic = ido - i;
          ti1 = ref(cc,i + 4*k*ido) + ref(cc,ic + (4*k + 3)*ido);
          ti2 = ref(cc,i + 4*k*ido) - ref(cc,ic + (4*k + 3)*ido);
          ti3 = ref(cc,i + (4*k + 2)*ido) - ref(cc,ic + (4*k + 1)*ido);
          tr4 = ref(cc,i + (4*k + 2)*ido) + ref(cc,ic + (4*k + 1)*ido);
          tr1 = ref(cc,i - 1 + 4*k*ido) - ref(cc,ic - 1 + (4*k + 3)*ido);
          tr2 = ref(cc,i - 1 + 4*k*ido) + ref(cc,ic - 1 + (4*k + 3)*ido);
          ti4 = ref(cc,i - 1 + (4*k + 2)*ido) - ref(cc,ic - 1 + (4*k + 1)*ido);
          tr3 = ref(cc,i - 1 + (4*k + 2)*ido) + ref(cc,ic - 1 + (4*k + 1)*ido);
          ch[i - 1 + k*ido] = tr2 + tr3;
          cr3 = tr2 - tr3;
          ch[i + k*ido] = ti2 + ti3;
          ci3 = ti2 - ti3;
          cr2 = tr1 - tr4;
          cr4 = tr1 + tr4;
          ci2 = ti1 + ti4;
          ci4 = ti1 - ti4;
          ch[i - 1 + (k + l1)*ido] = wa1[i - 2]*cr2 - wa1[i - 1]*ci2;
          ch[i + (k + l1)*ido] = wa1[i - 2]*ci2 + wa1[i - 1]*cr2;
          ch[i - 1 + (k + 2*l1)*ido] = wa2[i - 2]*cr3 - wa2[i - 1]*ci3;
          ch[i + (k + 2*l1)*ido] = wa2[i - 2]*ci3 + wa2[i - 1]*cr3;
          ch[i - 1 + (k + 3*l1)*ido] = wa3[i - 2]*cr4 - wa3[i - 1]*ci4;
          ch[i + (k + 3*l1)*ido] = wa3[i - 2]*ci4 + wa3[i - 1]*cr4;
        }
      }
      if (ido % 2 == 1) return;
    }
    for (k = 0; k < l1; k++) {
      ti1 = ref(cc,(4*k + 1)*ido) + ref(cc,(4*k + 3)*ido);
      ti2 = ref(cc,(4*k + 3)*ido) - ref(cc,(4*k + 1)*ido);
      tr1 = ref(cc,ido-1 + 4*k*ido) - ref(cc,ido-1 + (4*k + 2)*ido);
      tr2 = ref(cc,ido-1 + 4*k*ido) + ref(cc,ido-1 + (4*k + 2)*ido);
      ch[ido-1 + k*ido] = tr2 + tr2;
      ch[ido-1 + (k + l1)*ido] = sqrt2*(tr1 - ti1);
      ch[ido-1 + (k + 2*l1)*ido] = ti2 + ti2;
      ch[ido-1 + (k + 3*l1)*ido] = -sqrt2*(tr1 + ti1);
    }
  } /* radb4 */


static void radf5(int ido, int l1, const Treal cc[], Treal ch[],
      const Treal wa1[], const Treal wa2[], const Treal wa3[], const Treal wa4[])
  {
    static const Treal tr11 = 0.309016994374947;
    static const Treal ti11 = 0.951056516295154;
    static const Treal tr12 = -0.809016994374947;
    static const Treal ti12 = 0.587785252292473;
    int i, k, ic;
    Treal ci2, di2, ci4, ci5, di3, di4, di5, ci3, cr2, cr3, dr2, dr3, dr4, dr5,
        cr5, cr4, ti2, ti3, ti5, ti4, tr2, tr3, tr4, tr5;
    for (k = 0; k < l1; k++) {
      cr2 = ref(cc,(k + 4*l1)*ido) + ref(cc,(k + l1)*ido);
      ci5 = ref(cc,(k + 4*l1)*ido) - ref(cc,(k + l1)*ido);
      cr3 = ref(cc,(k + 3*l1)*ido) + ref(cc,(k + 2*l1)*ido);
      ci4 = ref(cc,(k + 3*l1)*ido) - ref(cc,(k + 2*l1)*ido);
      ch[5*k*ido] = ref(cc,k*ido) + cr2 + cr3;
      ch[ido-1 + (5*k + 1)*ido] = ref(cc,k*ido) + tr11*cr2 + tr12*cr3;
      ch[(5*k + 2)*ido] = ti11*ci5 + ti12*ci4;
      ch[ido-1 + (5*k + 3)*ido] = ref(cc,k*ido) + tr12*cr2 + tr11*cr3;
      ch[(5*k + 4)*ido] = ti12*ci5 - ti11*ci4;
    }
    if (ido == 1) return;
    for (k = 0; k < l1; ++k) {
      for (i = 2; i < ido; i += 2) {
        ic = ido - i;
        dr2 = wa1[i - 2]*ref(cc,i - 1 + (k + l1)*ido) + wa1[i - 1]*ref(cc,i + (k + l1)*ido);
        di2 = wa1[i - 2]*ref(cc,i + (k + l1)*ido) - wa1[i - 1]*ref(cc,i - 1 + (k + l1)*ido);
        dr3 = wa2[i - 2]*ref(cc,i - 1 + (k + 2*l1)*ido) + wa2[i - 1]*ref(cc,i + (k + 2*l1)*ido);
        di3 = wa2[i - 2]*ref(cc,i + (k + 2*l1)*ido) - wa2[i - 1]*ref(cc,i - 1 + (k + 2*l1)*ido);
        dr4 = wa3[i - 2]*ref(cc,i - 1 + (k + 3*l1)*ido) + wa3[i - 1]*ref(cc,i + (k + 3*l1)*ido);
        di4 = wa3[i - 2]*ref(cc,i + (k + 3*l1)*ido) - wa3[i - 1]*ref(cc,i - 1 + (k + 3*l1)*ido);
        dr5 = wa4[i - 2]*ref(cc,i - 1 + (k + 4*l1)*ido) + wa4[i - 1]*ref(cc,i + (k + 4*l1)*ido);
        di5 = wa4[i - 2]*ref(cc,i + (k + 4*l1)*ido) - wa4[i - 1]*ref(cc,i - 1 + (k + 4*l1)*ido);
        cr2 = dr2 + dr5;
        ci5 = dr5 - dr2;
        cr5 = di2 - di5;
        ci2 = di2 + di5;
        cr3 = dr3 + dr4;
        ci4 = dr4 - dr3;
        cr4 = di3 - di4;
        ci3 = di3 + di4;
        ch[i - 1 + 5*k*ido] = ref(cc,i - 1 + k*ido) + cr2 + cr3;
        ch[i + 5*k*ido] = ref(cc,i + k*ido) + ci2 + ci3;
        tr2 = ref(cc,i - 1 + k*ido) + tr11*cr2 + tr12*cr3;
        ti2 = ref(cc,i + k*ido) + tr11*ci2 + tr12*ci3;
        tr3 = ref(cc,i - 1 + k*ido) + tr12*cr2 + tr11*cr3;
        ti3 = ref(cc,i + k*ido) + tr12*ci2 + tr11*ci3;
        tr5 = ti11*cr5 + ti12*cr4;
        ti5 = ti11*ci5 + ti12*ci4;
        tr4 = ti12*cr5 - ti11*cr4;
        ti4 = ti12*ci5 - ti11*ci4;
        ch[i - 1 + (5*k + 2)*ido] = tr2 + tr5;
        ch[ic - 1 + (5*k + 1)*ido] = tr2 - tr5;
        ch[i + (5*k + 2)*ido] = ti2 + ti5;
        ch[ic + (5*k + 1)*ido] = ti5 - ti2;
        ch[i - 1 + (5*k + 4)*ido] = tr3 + tr4;
        ch[ic - 1 + (5*k + 3)*ido] = tr3 - tr4;
        ch[i + (5*k + 4)*ido] = ti3 + ti4;
        ch[ic + (5*k + 3)*ido] = ti4 - ti3;
      }
    }
  } /* radf5 */


static void radb5(int ido, int l1, const Treal cc[], Treal ch[],
      const Treal wa1[], const Treal wa2[], const Treal wa3[], const Treal wa4[])
  {
    static const Treal tr11 = 0.309016994374947;
    static const Treal ti11 = 0.951056516295154;
    static const Treal tr12 = -0.809016994374947;
    static const Treal ti12 = 0.587785252292473;
    int i, k, ic;
    Treal ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, ti2, ti3,
        ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;
    for (k = 0; k < l1; k++) {
      ti5 = 2*ref(cc,(5*k + 2)*ido);
      ti4 = 2*ref(cc,(5*k + 4)*ido);
      tr2 = 2*ref(cc,ido-1 + (5*k + 1)*ido);
      tr3 = 2*ref(cc,ido-1 + (5*k + 3)*ido);
      ch[k*ido] = ref(cc,5*k*ido) + tr2 + tr3;
      cr2 = ref(cc,5*k*ido) + tr11*tr2 + tr12*tr3;
      cr3 = ref(cc,5*k*ido) + tr12*tr2 + tr11*tr3;
      ci5 = ti11*ti5 + ti12*ti4;
      ci4 = ti12*ti5 - ti11*ti4;
      ch[(k + l1)*ido] = cr2 - ci5;
      ch[(k + 2*l1)*ido] = cr3 - ci4;
      ch[(k + 3*l1)*ido] = cr3 + ci4;
      ch[(k + 4*l1)*ido] = cr2 + ci5;
    }
    if (ido == 1) return;
    for (k = 0; k < l1; ++k) {
      for (i = 2; i < ido; i += 2) {
        ic = ido - i;
        ti5 = ref(cc,i + (5*k + 2)*ido) + ref(cc,ic + (5*k + 1)*ido);
        ti2 = ref(cc,i + (5*k + 2)*ido) - ref(cc,ic + (5*k + 1)*ido);
        ti4 = ref(cc,i + (5*k + 4)*ido) + ref(cc,ic + (5*k + 3)*ido);
        ti3 = ref(cc,i + (5*k + 4)*ido) - ref(cc,ic + (5*k + 3)*ido);
        tr5 = ref(cc,i - 1 + (5*k + 2)*ido) - ref(cc,ic - 1 + (5*k + 1)*ido);
        tr2 = ref(cc,i - 1 + (5*k + 2)*ido) + ref(cc,ic - 1 + (5*k + 1)*ido);
        tr4 = ref(cc,i - 1 + (5*k + 4)*ido) - ref(cc,ic - 1 + (5*k + 3)*ido);
        tr3 = ref(cc,i - 1 + (5*k + 4)*ido) + ref(cc,ic - 1 + (5*k + 3)*ido);
        ch[i - 1 + k*ido] = ref(cc,i - 1 + 5*k*ido) + tr2 + tr3;
        ch[i + k*ido] = ref(cc,i + 5*k*ido) + ti2 + ti3;
        cr2 = ref(cc,i - 1 + 5*k*ido) + tr11*tr2 + tr12*tr3;

        ci2 = ref(cc,i + 5*k*ido) + tr11*ti2 + tr12*ti3;
        cr3 = ref(cc,i - 1 + 5*k*ido) + tr12*tr2 + tr11*tr3;

        ci3 = ref(cc,i + 5*k*ido) + tr12*ti2 + tr11*ti3;
        cr5 = ti11*tr5 + ti12*tr4;
        ci5 = ti11*ti5 + ti12*ti4;
        cr4 = ti12*tr5 - ti11*tr4;
        ci4 = ti12*ti5 - ti11*ti4;
        dr3 = cr3 - ci4;
        dr4 = cr3 + ci4;
        di3 = ci3 + cr4;
        di4 = ci3 - cr4;
        dr5 = cr2 + ci5;
        dr2 = cr2 - ci5;
        di5 = ci2 - cr5;
        di2 = ci2 + cr5;
        ch[i - 1 + (k + l1)*ido] = wa1[i - 2]*dr2 - wa1[i - 1]*di2;
        ch[i + (k + l1)*ido] = wa1[i - 2]*di2 + wa1[i - 1]*dr2;
        ch[i - 1 + (k + 2*l1)*ido] = wa2[i - 2]*dr3 - wa2[i - 1]*di3;
        ch[i + (k + 2*l1)*ido] = wa2[i - 2]*di3 + wa2[i - 1]*dr3;
        ch[i - 1 + (k + 3*l1)*ido] = wa3[i - 2]*dr4 - wa3[i - 1]*di4;
        ch[i + (k + 3*l1)*ido] = wa3[i - 2]*di4 + wa3[i - 1]*dr4;
        ch[i - 1 + (k + 4*l1)*ido] = wa4[i - 2]*dr5 - wa4[i - 1]*di5;
        ch[i + (k + 4*l1)*ido] = wa4[i - 2]*di5 + wa4[i - 1]*dr5;
      }
    }
  } /* radb5 */


static void radfg(int ido, int ip, int l1, int idl1,
      Treal cc[], Treal ch[], const Treal wa[])
  {
    static const Treal twopi = 6.28318530717959;
    int idij, ipph, i, j, k, l, j2, ic, jc, lc, ik, is, nbd;
    Treal dc2, ai1, ai2, ar1, ar2, ds2, dcp, arg, dsp, ar1h, ar2h;
    arg = twopi / ip;
    dcp = cos(arg);
    dsp = sin(arg);
    ipph = (ip + 1) / 2;
    nbd = (ido - 1) / 2;
    if (ido != 1) {
      for (ik=0; ik<idl1; ik++) ch[ik] = cc[ik];
      for (j=1; j<ip; j++)
        for (k=0; k<l1; k++)
          ch[(k + j*l1)*ido] = cc[(k + j*l1)*ido];
      if (nbd <= l1) {
        is = -ido;
        for (j=1; j<ip; j++) {
          is += ido;
          idij = is-1;
          for (i=2; i<ido; i+=2) {
            idij += 2;
            for (k=0; k<l1; k++) {
              ch[i - 1 + (k + j*l1)*ido] =
                  wa[idij - 1]*cc[i - 1 + (k + j*l1)*ido] + wa[idij]*cc[i + (k + j*l1)*ido];
              ch[i + (k + j*l1)*ido] =
                  wa[idij - 1]*cc[i + (k + j*l1)*ido] - wa[idij]*cc[i - 1 + (k + j*l1)*ido];
            }
          }
        }
      } else {
        is = -ido;
        for (j=1; j<ip; j++) {
          is += ido;
          for (k=0; k<l1; k++) {
            idij = is-1;
            for (i=2; i<ido; i+=2) {
              idij += 2;
              ch[i - 1 + (k + j*l1)*ido] =
                  wa[idij - 1]*cc[i - 1 + (k + j*l1)*ido] + wa[idij]*cc[i + (k + j*l1)*ido];
              ch[i + (k + j*l1)*ido] =
                  wa[idij - 1]*cc[i + (k + j*l1)*ido] - wa[idij]*cc[i - 1 + (k + j*l1)*ido];
            }
          }
        }
      }
      if (nbd >= l1) {
        for (j=1; j<ipph; j++) {
          jc = ip - j;
          for (k=0; k<l1; k++) {
            for (i=2; i<ido; i+=2) {
              cc[i - 1 + (k + j*l1)*ido] = ch[i - 1 + (k + j*l1)*ido] + ch[i - 1 + (k + jc*l1)*ido];
              cc[i - 1 + (k + jc*l1)*ido] = ch[i + (k + j*l1)*ido] - ch[i + (k + jc*l1)*ido];
              cc[i + (k + j*l1)*ido] = ch[i + (k + j*l1)*ido] + ch[i + (k + jc*l1)*ido];
              cc[i + (k + jc*l1)*ido] = ch[i - 1 + (k + jc*l1)*ido] - ch[i - 1 + (k + j*l1)*ido];
            }
          }
        }
      } else {
        for (j=1; j<ipph; j++) {
          jc = ip - j;
          for (i=2; i<ido; i+=2) {
            for (k=0; k<l1; k++) {
              cc[i - 1 + (k + j*l1)*ido] =
                  ch[i - 1 + (k + j*l1)*ido] + ch[i - 1 + (k + jc*l1)*ido];
              cc[i - 1 + (k + jc*l1)*ido] = ch[i + (k + j*l1)*ido] - ch[i + (k + jc*l1)*ido];
              cc[i + (k + j*l1)*ido] = ch[i + (k + j*l1)*ido] + ch[i + (k + jc*l1)*ido];
              cc[i + (k + jc*l1)*ido] = ch[i - 1 + (k + jc*l1)*ido] - ch[i - 1 + (k + j*l1)*ido];
            }
          }
        }
      }
    } else {  /* now ido == 1 */
      for (ik=0; ik<idl1; ik++) cc[ik] = ch[ik];
    }
    for (j=1; j<ipph; j++) {
      jc = ip - j;
      for (k=0; k<l1; k++) {
        cc[(k + j*l1)*ido] = ch[(k + j*l1)*ido] + ch[(k + jc*l1)*ido];
        cc[(k + jc*l1)*ido] = ch[(k + jc*l1)*ido] - ch[(k + j*l1)*ido];
      }
    }

    ar1 = 1;
    ai1 = 0;
    for (l=1; l<ipph; l++) {
      lc = ip - l;
      ar1h = dcp*ar1 - dsp*ai1;
      ai1 = dcp*ai1 + dsp*ar1;
      ar1 = ar1h;
      for (ik=0; ik<idl1; ik++) {
        ch[ik + l*idl1] = cc[ik] + ar1*cc[ik + idl1];
        ch[ik + lc*idl1] = ai1*cc[ik + (ip-1)*idl1];
      }
      dc2 = ar1;
      ds2 = ai1;
      ar2 = ar1;
      ai2 = ai1;
      for (j=2; j<ipph; j++) {
        jc = ip - j;
        ar2h = dc2*ar2 - ds2*ai2;
        ai2 = dc2*ai2 + ds2*ar2;
        ar2 = ar2h;
        for (ik=0; ik<idl1; ik++) {
          ch[ik + l*idl1] += ar2*cc[ik + j*idl1];
          ch[ik + lc*idl1] += ai2*cc[ik + jc*idl1];
        }
      }
    }
    for (j=1; j<ipph; j++)
      for (ik=0; ik<idl1; ik++)
        ch[ik] += cc[ik + j*idl1];

    if (ido >= l1) {
      for (k=0; k<l1; k++) {
        for (i=0; i<ido; i++) {
          ref(cc,i + k*ip*ido) = ch[i + k*ido];
        }
      }
    } else {
      for (i=0; i<ido; i++) {
        for (k=0; k<l1; k++) {
          ref(cc,i + k*ip*ido) = ch[i + k*ido];
        }
      }
    }
    for (j=1; j<ipph; j++) {
      jc = ip - j;
      j2 = 2*j;
      for (k=0; k<l1; k++) {
        ref(cc,ido-1 + (j2 - 1 + k*ip)*ido) =
            ch[(k + j*l1)*ido];
        ref(cc,(j2 + k*ip)*ido) =
            ch[(k + jc*l1)*ido];
      }
    }
    if (ido == 1) return;
    if (nbd >= l1) {
      for (j=1; j<ipph; j++) {
        jc = ip - j;
        j2 = 2*j;
        for (k=0; k<l1; k++) {
          for (i=2; i<ido; i+=2) {
            ic = ido - i;
            ref(cc,i - 1 + (j2 + k*ip)*ido) = ch[i - 1 + (k + j*l1)*ido] + ch[i - 1 + (k + jc*l1)*ido];
            ref(cc,ic - 1 + (j2 - 1 + k*ip)*ido) = ch[i - 1 + (k + j*l1)*ido] - ch[i - 1 + (k + jc*l1)*ido];
            ref(cc,i + (j2 + k*ip)*ido) = ch[i + (k + j*l1)*ido] + ch[i + (k + jc*l1)*ido];
            ref(cc,ic + (j2 - 1 + k*ip)*ido) = ch[i + (k + jc*l1)*ido] - ch[i + (k + j*l1)*ido];
          }
        }
      }
    } else {
      for (j=1; j<ipph; j++) {
        jc = ip - j;
        j2 = 2*j;
        for (i=2; i<ido; i+=2) {
          ic = ido - i;
          for (k=0; k<l1; k++) {
            ref(cc,i - 1 + (j2 + k*ip)*ido) = ch[i - 1 + (k + j*l1)*ido] + ch[i - 1 + (k + jc*l1)*ido];
            ref(cc,ic - 1 + (j2 - 1 + k*ip)*ido) = ch[i - 1 + (k + j*l1)*ido] - ch[i - 1 + (k + jc*l1)*ido];
            ref(cc,i + (j2 + k*ip)*ido) = ch[i + (k + j*l1)*ido] + ch[i + (k + jc*l1)*ido];
            ref(cc,ic + (j2 - 1 + k*ip)*ido) = ch[i + (k + jc*l1)*ido] - ch[i + (k + j*l1)*ido];
          }
        }
      }
    }
  } /* radfg */


static void radbg(int ido, int ip, int l1, int idl1,
      Treal cc[], Treal ch[], const Treal wa[])
  {
    static const Treal twopi = 6.28318530717959;
    int idij, ipph, i, j, k, l, j2, ic, jc, lc, ik, is;
    Treal dc2, ai1, ai2, ar1, ar2, ds2;
    int nbd;
    Treal dcp, arg, dsp, ar1h, ar2h;
    arg = twopi / ip;
    dcp = cos(arg);
    dsp = sin(arg);
    nbd = (ido - 1) / 2;
    ipph = (ip + 1) / 2;
    if (ido >= l1) {
      for (k=0; k<l1; k++) {
        for (i=0; i<ido; i++) {
          ch[i + k*ido] = ref(cc,i + k*ip*ido);
        }
      }
    } else {
      for (i=0; i<ido; i++) {
        for (k=0; k<l1; k++) {
          ch[i + k*ido] = ref(cc,i + k*ip*ido);
        }
      }
    }
    for (j=1; j<ipph; j++) {
      jc = ip - j;
      j2 = 2*j;
      for (k=0; k<l1; k++) {
        ch[(k + j*l1)*ido] = ref(cc,ido-1 + (j2 - 1 + k*ip)*ido) + ref(cc,ido-1 + (j2 - 1 + k*ip)*
            ido);
        ch[(k + jc*l1)*ido] = ref(cc,(j2 + k*ip)*ido) + ref(cc,(j2 + k*ip)*ido);
      }
    }

    if (ido != 1) {
      if (nbd >= l1) {
        for (j=1; j<ipph; j++) {
          jc = ip - j;
          for (k=0; k<l1; k++) {
            for (i=2; i<ido; i+=2) {
              ic = ido - i;
              ch[i - 1 + (k + j*l1)*ido] = ref(cc,i - 1 + (2*j + k*ip)*ido) + ref(cc,
                  ic - 1 + (2*j - 1 + k*ip)*ido);
              ch[i - 1 + (k + jc*l1)*ido] = ref(cc,i - 1 + (2*j + k*ip)*ido) -
                  ref(cc,ic - 1 + (2*j - 1 + k*ip)*ido);
              ch[i + (k + j*l1)*ido] = ref(cc,i + (2*j + k*ip)*ido) - ref(cc,ic
                  + (2*j - 1 + k*ip)*ido);
              ch[i + (k + jc*l1)*ido] = ref(cc,i + (2*j + k*ip)*ido) + ref(cc,ic
                  + (2*j - 1 + k*ip)*ido);
            }
          }
        }
      } else {
        for (j=1; j<ipph; j++) {
          jc = ip - j;
          for (i=2; i<ido; i+=2) {
            ic = ido - i;
            for (k=0; k<l1; k++) {
              ch[i - 1 + (k + j*l1)*ido] = ref(cc,i - 1 + (2*j + k*ip)*ido) + ref(cc,
                  ic - 1 + (2*j - 1 + k*ip)*ido);
              ch[i - 1 + (k + jc*l1)*ido] = ref(cc,i - 1 + (2*j + k*ip)*ido) -
                  ref(cc,ic - 1 + (2*j - 1 + k*ip)*ido);
              ch[i + (k + j*l1)*ido] = ref(cc,i + (2*j + k*ip)*ido) - ref(cc,ic
                  + (2*j - 1 + k*ip)*ido);
              ch[i + (k + jc*l1)*ido] = ref(cc,i + (2*j + k*ip)*ido) + ref(cc,ic
                  + (2*j - 1 + k*ip)*ido);
            }
          }
        }
      }
    }

    ar1 = 1;
    ai1 = 0;
    for (l=1; l<ipph; l++) {
      lc = ip - l;
      ar1h = dcp*ar1 - dsp*ai1;
      ai1 = dcp*ai1 + dsp*ar1;
      ar1 = ar1h;
      for (ik=0; ik<idl1; ik++) {
        cc[ik + l*idl1] = ch[ik] + ar1*ch[ik + idl1];
        cc[ik + lc*idl1] = ai1*ch[ik + (ip-1)*idl1];
      }
      dc2 = ar1;
      ds2 = ai1;
      ar2 = ar1;
      ai2 = ai1;
      for (j=2; j<ipph; j++) {
        jc = ip - j;
        ar2h = dc2*ar2 - ds2*ai2;
        ai2 = dc2*ai2 + ds2*ar2;
        ar2 = ar2h;
        for (ik=0; ik<idl1; ik++) {
          cc[ik + l*idl1] += ar2*ch[ik + j*idl1];
          cc[ik + lc*idl1] += ai2*ch[ik + jc*idl1];
        }
      }
    }
    for (j=1; j<ipph; j++) {
      for (ik=0; ik<idl1; ik++) {
        ch[ik] += ch[ik + j*idl1];
      }
    }
    for (j=1; j<ipph; j++) {
      jc = ip - j;
      for (k=0; k<l1; k++) {
        ch[(k + j*l1)*ido] = cc[(k + j*l1)*ido] - cc[(k + jc*l1)*ido];
        ch[(k + jc*l1)*ido] = cc[(k + j*l1)*ido] + cc[(k + jc*l1)*ido];
      }
    }

    if (ido == 1) return;
    if (nbd >= l1) {
      for (j=1; j<ipph; j++) {
        jc = ip - j;
        for (k=0; k<l1; k++) {
          for (i=2; i<ido; i+=2) {
            ch[i - 1 + (k + j*l1)*ido] = cc[i - 1 + (k + j*l1)*ido] - cc[i + (k + jc*l1)*ido];
            ch[i - 1 + (k + jc*l1)*ido] = cc[i - 1 + (k + j*l1)*ido] + cc[i + (k + jc*l1)*ido];
            ch[i + (k + j*l1)*ido] = cc[i + (k + j*l1)*ido] + cc[i - 1 + (k + jc*l1)*ido];
            ch[i + (k + jc*l1)*ido] = cc[i + (k + j*l1)*ido] - cc[i - 1 + (k + jc*l1)*ido];
          }
        }
      }
    } else {
      for (j=1; j<ipph; j++) {
        jc = ip - j;
        for (i=2; i<ido; i+=2) {
          for (k=0; k<l1; k++) {
            ch[i - 1 + (k + j*l1)*ido] = cc[i - 1 + (k + j*l1)*ido] - cc[i + (k + jc*l1)*ido];
            ch[i - 1 + (k + jc*l1)*ido] = cc[i - 1 + (k + j *l1)*ido] + cc[i + (k + jc*l1)*ido];
            ch[i + (k + j*l1)*ido] = cc[i + (k + j*l1)*ido] + cc[i - 1 + (k + jc*l1)*ido];
            ch[i + (k + jc*l1)*ido] = cc[i + (k + j*l1)*ido] - cc[i - 1 + (k + jc*l1)*ido];
          }
        }
      }
    }
    for (ik=0; ik<idl1; ik++) cc[ik] = ch[ik];
    for (j=1; j<ip; j++)
      for (k=0; k<l1; k++)
        cc[(k + j*l1)*ido] = ch[(k + j*l1)*ido];
    if (nbd <= l1) {
      is = -ido;
      for (j=1; j<ip; j++) {
        is += ido;
        idij = is-1;
        for (i=2; i<ido; i+=2) {
          idij += 2;
          for (k=0; k<l1; k++) {
            cc[i - 1 + (k + j*l1)*ido] = wa[idij - 1]*ch[i - 1 + (k + j*l1)*ido] - wa[idij]*
                ch[i + (k + j*l1)*ido];
            cc[i + (k + j*l1)*ido] = wa[idij - 1]*ch[i + (k + j*l1)*ido] + wa[idij]*ch[i - 1 + (k + j*l1)*ido];
          }
        }
      }
    } else {
      is = -ido;
      for (j=1; j<ip; j++) {
        is += ido;
        for (k=0; k<l1; k++) {
          idij = is;
          for (i=2; i<ido; i+=2) {
            idij += 2;
            cc[i - 1 + (k + j*l1)*ido] = wa[idij-1]*ch[i - 1 + (k + j*l1)*ido] - wa[idij]*
                ch[i + (k + j*l1)*ido];
            cc[i + (k + j*l1)*ido] = wa[idij-1]*ch[i + (k + j*l1)*ido] + wa[idij]*ch[i - 1 + (k + j*l1)*ido];
          }
        }
      }
    }
  } /* radbg */

  /* ----------------------------------------------------------------------
cfftf1, cfftf, cfftb, cffti1, cffti. Complex FFTs.
---------------------------------------------------------------------- */

static void cfftf1(int n, Treal c[], Treal ch[], const Treal wa[], const int ifac[MAXFAC+2], int isign)
  {
    int idot, i;
    int k1, l1, l2;
    int na, nf, ip, iw, ix2, ix3, ix4, nac, ido, idl1;
    Treal *cinput, *coutput;
    nf = ifac[1];
    na = 0;
    l1 = 1;
    iw = 0;
    for (k1=2; k1<=nf+1; k1++) {
      ip = ifac[k1];
      l2 = ip*l1;
      ido = n / l2;
      idot = ido + ido;
      idl1 = idot*l1;
      if (na) {
        cinput = ch;
        coutput = c;
      } else {
        cinput = c;
        coutput = ch;
      }
      switch (ip) {
      case 4:
        ix2 = iw + idot;
        ix3 = ix2 + idot;
        passf4(idot, l1, cinput, coutput, &wa[iw], &wa[ix2], &wa[ix3], isign);
        na = !na;
        break;
      case 2:
        passf2(idot, l1, cinput, coutput, &wa[iw], isign);
        na = !na;
        break;
      case 3:
        ix2 = iw + idot;
        passf3(idot, l1, cinput, coutput, &wa[iw], &wa[ix2], isign);
        na = !na;
        break;
      case 5:
        ix2 = iw + idot;
        ix3 = ix2 + idot;
        ix4 = ix3 + idot;
        passf5(idot, l1, cinput, coutput, &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4], isign);
        na = !na;
        break;
      default:
        passf(&nac, idot, ip, l1, idl1, cinput, coutput, &wa[iw], isign);
        if (nac != 0) na = !na;
      }
      l1 = l2;
      iw += (ip - 1)*idot;
    }
    if (na == 0) return;
    for (i=0; i<2*n; i++) c[i] = ch[i];
  } /* cfftf1 */


void cfftf(int n, Treal c[], Treal wsave[])
  {
    int iw1, iw2;
    if (n == 1) return;
    iw1 = 2*n;
    iw2 = iw1 + 2*n;
    cfftf1(n, c, wsave, wsave+iw1, (int*)(wsave+iw2), -1);
  } /* cfftf */


void cfftb(int n, Treal c[], Treal wsave[])
  {
    int iw1, iw2;
    if (n == 1) return;
    iw1 = 2*n;
    iw2 = iw1 + 2*n;
    cfftf1(n, c, wsave, wsave+iw1, (int*)(wsave+iw2), +1);
  } /* cfftb */


static void factorize(int n, int ifac[MAXFAC+2])
  /* Factorize n in factors of 2,3,4,5 and rest. On exit,
ifac[0] contains n and ifac[1] contains number of factors,
the factors start from ifac[2]. */
  {
    static const int ntryh[4] = {
      3,4,2,5    };
    int ntry=3, i, j=0, ib, nf=0, nl=n, nq, nr;
startloop:
    if (j < 4)
      ntry = ntryh[j];
    else
      ntry+= 2;
    j++;
    do {
      nq = nl / ntry;
      nr = nl - ntry*nq;
      if (nr != 0) goto startloop;
      nf++;
      ifac[nf + 1] = ntry;
      nl = nq;
      if (ntry == 2 && nf != 1) {
        for (i=2; i<=nf; i++) {
          ib = nf - i + 2;
          ifac[ib + 1] = ifac[ib];
        }
        ifac[2] = 2;
      }
    } while (nl != 1);
    ifac[0] = n;
    ifac[1] = nf;
  }


static void cffti1(int n, Treal wa[], int ifac[MAXFAC+2])
  {
    static const Treal twopi = 6.28318530717959;
    Treal arg, argh, argld, fi;
    int idot, i, j;
    int i1, k1, l1, l2;
    int ld, ii, nf, ip;
    int ido, ipm;

    factorize(n,ifac);
    nf = ifac[1];
    argh = twopi/(Treal)n;
    i = 1;
    l1 = 1;
    for (k1=1; k1<=nf; k1++) {
      ip = ifac[k1+1];
      ld = 0;
      l2 = l1*ip;
      ido = n / l2;
      idot = ido + ido + 2;
      ipm = ip - 1;
      for (j=1; j<=ipm; j++) {
        i1 = i;
        wa[i-1] = 1;
        wa[i] = 0;
        ld += l1;
        fi = 0;
        argld = ld*argh;
        for (ii=4; ii<=idot; ii+=2) {
          i+= 2;
          fi+= 1;
          arg = fi*argld;
          wa[i-1] = cos(arg);
          wa[i] = sin(arg);
        }
        if (ip > 5) {
          wa[i1-1] = wa[i-1];
          wa[i1] = wa[i];
        }
      }
      l1 = l2;
    }
  } /* cffti1 */


void cffti(int n, Treal wsave[])
 {
    int iw1, iw2;
    if (n == 1) return;
    iw1 = 2*n;
    iw2 = iw1 + 2*n;
    cffti1(n, wsave+iw1, (int*)(wsave+iw2));
  } /* cffti */

  /* ----------------------------------------------------------------------
rfftf1, rfftb1, rfftf, rfftb, rffti1, rffti. Treal FFTs.
---------------------------------------------------------------------- */

static void rfftf1(int n, Treal c[], Treal ch[], const Treal wa[], const int ifac[MAXFAC+2])
  {
    int i;
    int k1, l1, l2, na, kh, nf, ip, iw, ix2, ix3, ix4, ido, idl1;
    Treal *cinput, *coutput;
    nf = ifac[1];
    na = 1;
    l2 = n;
    iw = n-1;
    for (k1 = 1; k1 <= nf; ++k1) {
      kh = nf - k1;
      ip = ifac[kh + 2];
      l1 = l2 / ip;
      ido = n / l2;
      idl1 = ido*l1;
      iw -= (ip - 1)*ido;
      na = !na;
      if (na) {
        cinput = ch;
        coutput = c;
      } else {
        cinput = c;
        coutput = ch;
      }
      switch (ip) {
      case 4:
        ix2 = iw + ido;
        ix3 = ix2 + ido;
        radf4(ido, l1, cinput, coutput, &wa[iw], &wa[ix2], &wa[ix3]);
        break;
      case 2:
        radf2(ido, l1, cinput, coutput, &wa[iw]);
        break;
      case 3:
        ix2 = iw + ido;
        radf3(ido, l1, cinput, coutput, &wa[iw], &wa[ix2]);
        break;
      case 5:
        ix2 = iw + ido;
        ix3 = ix2 + ido;
        ix4 = ix3 + ido;
        radf5(ido, l1, cinput, coutput, &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]);
        break;
      default:
        if (ido == 1)
          na = !na;
        if (na == 0) {
          radfg(ido, ip, l1, idl1, c, ch, &wa[iw]);
          na = 1;
        } else {
          radfg(ido, ip, l1, idl1, ch, c, &wa[iw]);
          na = 0;
        }
      }
      l2 = l1;
    }
    if (na == 1) return;
    for (i = 0; i < n; i++) c[i] = ch[i];
  } /* rfftf1 */


void rfftb1(int n, Treal c[], Treal ch[], const Treal wa[], const int ifac[MAXFAC+2])
  {
    int i;
    int k1, l1, l2, na, nf, ip, iw, ix2, ix3, ix4, ido, idl1;
    Treal *cinput, *coutput;
    nf = ifac[1];
    na = 0;
    l1 = 1;
    iw = 0;
    for (k1=1; k1<=nf; k1++) {
      ip = ifac[k1 + 1];
      l2 = ip*l1;
      ido = n / l2;
      idl1 = ido*l1;
      if (na) {
        cinput = ch;
        coutput = c;
      } else {
        cinput = c;
        coutput = ch;
      }
      switch (ip) {
      case 4:
        ix2 = iw + ido;
        ix3 = ix2 + ido;
        radb4(ido, l1, cinput, coutput, &wa[iw], &wa[ix2], &wa[ix3]);
        na = !na;
        break;
      case 2:
        radb2(ido, l1, cinput, coutput, &wa[iw]);
        na = !na;
        break;
      case 3:
        ix2 = iw + ido;
        radb3(ido, l1, cinput, coutput, &wa[iw], &wa[ix2]);
        na = !na;
        break;
      case 5:
        ix2 = iw + ido;
        ix3 = ix2 + ido;
        ix4 = ix3 + ido;
        radb5(ido, l1, cinput, coutput, &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]);
        na = !na;
        break;
      default:
        radbg(ido, ip, l1, idl1, cinput, coutput, &wa[iw]);
        if (ido == 1) na = !na;
      }
      l1 = l2;
      iw += (ip - 1)*ido;
    }
    if (na == 0) return;
    for (i=0; i<n; i++) c[i] = ch[i];
  } /* rfftb1 */


void rfftf(int n, Treal r[], Treal wsave[])
  {
    if (n == 1) return;
    rfftf1(n, r, wsave, wsave+n, (int*)(wsave+2*n));
  } /* rfftf */


void rfftb(int n, Treal r[], Treal wsave[])
  {
    if (n == 1) return;
    rfftb1(n, r, wsave, wsave+n, (int*)(wsave+2*n));
  } /* rfftb */


static void rffti1(int n, Treal wa[], int ifac[MAXFAC+2])
  {
    static const Treal twopi = 6.28318530717959;
    Treal arg, argh, argld, fi;
    int i, j;
    int k1, l1, l2;
    int ld, ii, nf, ip, is;
    int ido, ipm, nfm1;
    factorize(n,ifac);
    nf = ifac[1];
    argh = twopi / n;
    is = 0;
    nfm1 = nf - 1;
    l1 = 1;
    if (nfm1 == 0) return;
    for (k1 = 1; k1 <= nfm1; k1++) {
      ip = ifac[k1 + 1];
      ld = 0;
      l2 = l1*ip;
      ido = n / l2;
      ipm = ip - 1;
      for (j = 1; j <= ipm; ++j) {
        ld += l1;
        i = is;
        argld = (Treal) ld*argh;
        fi = 0;
        for (ii = 3; ii <= ido; ii += 2) {
          i += 2;
          fi += 1;
          arg = fi*argld;
          wa[i - 2] = cos(arg);
          wa[i - 1] = sin(arg);
        }
        is += ido;
      }
      l1 = l2;
    }
  } /* rffti1 */


void rffti(int n, Treal wsave[])
  {
    if (n == 1) return;
    rffti1(n, wsave+n, (int*)(wsave+2*n));
  } /* rffti */

/************************************************************************
* cfft1d is a subroutine used to call and initialize FFT routines from  *
* fftpack.c   The calls are almost identical to the old Sun perflib     *
************************************************************************/
/************************************************************************
* Creator: David T. Sandwell	(Scripps Institution of Oceanography    *
* Date   : 12/27/96                                                     *
* Date   : 09/15/07 re-worked by Rob Mellors                            *
* Date   : 10/16/07 re-worked by David Sandwells  to use pointers       *
************************************************************************/

typedef struct FCOMPLEX {float r,i;} fcomplex;

/*----------------------------------------------------------------------------*/
void cfft1d_(int *np, fcomplex *c, int *dir)
{

	static float *work;
	static int nold = 0;
	int i, n;

/* Initialize work array with sines and cosines to save CPU time later 
   This is done when the length of the FFT has changed or when *dir == 0. */
	
	n = *np;

	if ((n != nold) || (*dir == 0)) {
		if (nold != 0) free((char *) work);
		if ((work = (float *) malloc((4*n+30)*sizeof(float))) == NULL) {
			fprintf (stderr, "Cannot allocate work memory\n");
			GMT_exit (EXIT_FAILURE);
		}

		cffti (n, work);

		nold = n;
	}

/* Do forward transform with NO normalization.  Forward is exp(+i*k*x) */

	if (*dir == -1) cfftf (n, (Treal *)c, work); 

/* Do inverse transform with normalization.  Inverse is exp(-i*k*x) */

	if (*dir == 1) {
	  	cfftb (n, (Treal *)c, work);
          	for (i=0; i<n; i++) {
			c[i].i = c[i].i/(1.0*n);
			c[i].r = c[i].r/(1.0*n);
		}
	}
}

/*------------------------------------------------------------------------*/
/*	calculates 2D fft by doing 1D over rows, transposing, and then 	  */
/*	columns								  */
/*------------------------------------------------------------------------*/

void transpose_complex_NM (struct FCOMPLEX *in, int n, int m)
{
	int i, j;
	struct FCOMPLEX	*tmp;

	tmp = (struct FCOMPLEX *) malloc(n * m * sizeof(struct FCOMPLEX));

	for (i=0; i<n; i++) {
		for (j=0; j<m; j++) {
			tmp[j*n+i] = in[i*m+j];
			}
		}

	for (i=0; i<(n*m); i++) in[i] = tmp[i];

	free((char *) tmp);
}

int cfft2d (int *N, int *M, struct FCOMPLEX *cin, int *dir)
{
	int	i, j;
	static	int flag = 1;

	if (flag == 0) {
		fprintf(stderr,"using fftpack \n");
		flag = 1;
	}

	if (debug) print_complex (cin, *N, *M, 1);

	/* forward 2D */
	if (*dir == -1) {
		/* forward rows */
		for (i=0; i<(*N); i++) cfft1d_(M, &cin[i*(*M)], dir); 

		/* transpose */
		transpose_complex_NM(cin, *N, *M);

		/* forward columns */
		for (i=0; i<(*M); i++) cfft1d_(N, &cin[i*(*N)], dir); 

		/* transpose */
		transpose_complex_NM(cin, *M, *N);
	}

	/* inverse 2D */
	if (*dir == 1) {
		/* forward rows */
		for (i=0; i<(*N); i++) cfft1d_(M, &cin[i*(*M)], dir); 

		/* transpose */
		transpose_complex_NM(cin, *N, *M);

		/* forward columns */
		for (i=0; i<(*M); i++) cfft1d_(N, &cin[i*(*N)], dir); 

		/* transpose */
		transpose_complex_NM(cin, *M, *N);
		}

	if (debug) print_complex(cin, *N, *M, 1);

return 0;

}

#elif GMT_FFT == GMT_FFTW
/************************************************************************
* cfft1d is a subroutine to do a 1-D fft using FFTW routines            *
************************************************************************/
/************************************************************************
* Creator: David T. Sandwell    (Scripps Institution of Oceanography    *
* Date   : 12/27/96                                                     *
************************************************************************/
/************************************************************************
* Modification history:                                                 *
*  04/01/98  - changed to have arguments be pointers (Fotran callable)  *
*  10/16/03  - changed to call fftw instead of the sun perflib          *
************************************************************************/

#include "fftw3.h"

cfft1d_(int *np, fftwf_complex *c, int *dir)
{
        static int nold = 0;
        static fftwf_plan pf, pi;
        int i, n;

/* Make the plans for FFTW and destroy the old ones if they exist.
   This is done when the length of the FFT has changed or when *dir == 0. */

        n = *np;
        if ((n != nold) || (*dir == 0)) {
		if (nold != 0) {
			fftwf_destroy_plan (pf);
			fftwf_destroy_plan (pi);
		}
		pf = fftwf_plan_dft_1d (n,c,c,-1,FFTW_MEASURE);
		pi = fftwf_plan_dft_1d (n,c,c, 1,FFTW_MEASURE);
		printf (" reset plan \n");
		nold = n;
	}

/* Do forward transform with NO normalization. */

	if (*dir == -1) {
		fftwf_execute(pf);
	}

/* Do inverse transform with normalization. */

	if (*dir == 1) {
		fftwf_execute (pi);
		for (i=0;i<n;i++){
			c[i][0] = c[i][0]/((float) n);
			c[i][1] = c[i][1]/((float) n);
		}
	}
}

#elif GMT_FFT == GMT_SUN4

/************************************************************************
* cfft1d is a subroutine used to call and initialize perflib Fortran FFT *
* routines in a Sun computer.                                           *
************************************************************************/
/************************************************************************
* Creator: David T. Sandwell	(Scripps Institution of Oceanography    *
* Date   : 12/27/96                                                     *
************************************************************************/
/************************************************************************
* Modification history:                                                 *
*  04/01/98  - changed to have arguments be pointers (Fotran callable)  *
*                                                                       *
* DATE                                                                  *
************************************************************************/
 
#include "../include/soi.h"

cfft1d_(int *np,fcomplex *c,int *dir)
{

	static float *work;
	static int nold = 0;
	int i,n;

/* Initialize work array with sines and cosines to save CPU time later 
   This is done when the length of the FFT has changed or when *dir == 0. */

	n = *np;
	if ((n != nold) || (*dir == 0)) {
		if(nold != 0) free ((char *) work);
		if((work = (float *) malloc((4*n+30)*sizeof(float))) == NULL){
			fprintf(stderr,"Sorry, can't allocate mem.\n");
			return(-1);
		}
		cffti_ (np,work);
		nold = n;
	}

/* Do forward transform with NO normalization.  Forward is exp(+i*k*x) */

	if (*dir == -1) {
		cfftf_ (np,c,work);
	}

/* Do inverse transform with normalization.  Inverse is exp(-i*k*x) */

	if (*dir == 1) {
		cfftb_ (np,c,work);
		for (i=0;i<n;i++) {
			c[i].r = c[i].r/((float) n);
			c[i].i = c[i].i/((float) n);
		}
	}
}

#elif GMT_FFT == GMT_VECLIB

/************************************************************************
* cfft1d is a subroutine used to call and initialize veclib FFT         *
* in a Mac OS X computer.                                               *
************************************************************************/
/************************************************************************
* Creator: Robert Kern          (Scripps Institution of Oceanography    *
* Date   : 12/2005                                                      *
************************************************************************/
#include <vecLib/vecLib.h>
#include <stdio.h>
#include <stdlib.h>

int cfft1d_(int* np, DSPComplex* c, int* dir);
void cfft1d_cleanup_();

static int n = 0; 
static int log2n;
static FFTSetup setup;
static DSPSplitComplex d;
static float scale;
static int inited = 0;

int cfft1d_ (int* np, DSPComplex* c, int* dir)
{
	if (*dir == 0) return;
	if (n != *np) {
		cfft1d_cleanup_ ();
		n = *np;
		for (log2n=1; (1<<log2n)<*np; log2n++) {}

		d.realp = (float*)malloc(n*sizeof(float));
		d.imagp = (float*)malloc(n*sizeof(float));

		setup = vDSP_create_fftsetup (log2n, 0);
		scale = 1.0/n;
		inited = 1;
	}
    
	vDSP_ctoz (c, 2, &d, 1, n);

	vDSP_fft_zip (setup, &d, 1, log2n, (*dir==-1 ? FFT_FORWARD : FFT_INVERSE));

	vDSP_ztoc (&d, 1, c, 2, n);

	if (*dir == 1) {
		vDSP_vsmul ((float*)c, 1, &scale, (float*)c, 1, 2*n);
	}
}

void cfft1d_cleanup_()
{
	if (inited) {
		free(d.realp);
		free(d.imagp);
		vDSP_destroy_fftsetup(setup);
		inited = 0;
	}
}

#endif

/* Default GMT FFT which we always compile in */

/*--------------------------------------------------------------------
 *	Translation of old fourt.f FORTRAN code to C using the automatic
 *	translator f2c written by S.I. Feldman, David M. Gay, Mark W. Maimone,
 *	and N.L. Schryer.  Translated version provided by Andrew MacRae
 *	at the University of Calgary.  I've cleaned up the resulting code
 *	since much of the f2c.h include stuff was unnecessary for this
 *	function.
 *
 *	P. Wessel, last century sometime
 *--------------------------------------------------------------------*/

GMT_LONG BRENNER_fourt_ (float *data, GMT_LONG *nn, GMT_LONG *ndim, GMT_LONG *ksign, GMT_LONG *iform, float *work)
{

	/* System generated locals */
	GMT_LONG i__1, i__2, i__3, i__4, i__5, i__6, i__7, i__8, i__9, i__10, i__11, i__12;

	/* Local variables */

	static GMT_LONG j1rg2, idiv, irem, ipar, kmin, imin, jmin, lmax, mmax, imax, jmax;
	static GMT_LONG ntwo, j1cnj, np1hf, np2hf, j1min, i1max, i1rng, j1rng, j2min, j3max;
	static GMT_LONG j1max, j2max, i2max, non2t, j2stp, i, j, k, l, m, n, icase, ifact[32];
	static GMT_LONG nhalf, krang, kconj, kdif, idim, ntot, kstep, k2, k3, k4, nprev, iquot;
	static GMT_LONG i2, i1, i3, j3, k1, j2, j1, if_, np0, np1, np2, ifp1, ifp2, non2;

	static float theta, oldsi, tempi, oldsr, sinth, difi, difr, sumi, sumr, tempr, twopi;
	static float wstpi, wstpr, twowr, wi, wr, u1i, w2i, w3i, u2i, u3i, u4i, t2i, u1r;
	static float u2r, u3r, w2r, w3r, u4r, t2r, t3r, t3i, t4r, t4i;
	static double wrd, wid;

/*---------------------------------------------------------------------------
	ARGUMENTS :
	DATA - COMPLEX ARRAY, LENGTH NN
	NN - ARRAY OF NUMBER OF POINTS IN EACH DIMENSION
	NDIM - NUMBER OF DIMENSIONS (FOR OUR PURPOSES, NDIM=1)
	KSIGN - +1 FOR INVERSE TRANSFORM (FREQ TO GMT_TIME DOMAIN)
		-1 FOR FORWARD TRANSFORM (GMT_TIME TO FREQ DOMAIN)
	IFORM - 0 REAL DATA
		+1 COMPLEX DATA
	WORK - 0 IF ALL DIMENSIONS ARE RADIX 2
	COMPLEX ARRAY, LARGE AS LARGEST NON-RADIX 2 DIMENSI0N

	PROGRAM BY NORMAN BRENNER FROM THE BASIC PROGRAM BY CHARLES
	RADER.  RALPH ALTER SUGGESTED THE IDEA FOR THE DIGIT REVERSAL.
	MIT LINCOLN LABORATORY, AUGUST 1967.

---------------------------------------------------------------------------*/

	/* Parameter adjustments */
	--work;
	--nn;
	--data;

	/* Function Body */
	wr = wi = wstpr = wstpi = (float)0.0;
	twopi = (float)6.283185307;
	if (*ndim - 1 >= 0) {
		goto L1;
	} else {
		goto L920;
	}
L1:
	ntot = 2;
	i__1 = *ndim;
	for (idim = 1; idim <= i__1; ++idim) {
	if (nn[idim] <= 0) {
		goto L920;
	} else {
		goto L2;
	}
L2:
	ntot *= nn[idim];
	}
	np1 = 2;
	i__1 = *ndim;
	for (idim = 1; idim <= i__1; ++idim) {
	n = nn[idim];
	np2 = np1 * n;
	if ((i__2 = n - 1) < 0) {
		goto L920;
	} else if (i__2 == 0) {
		goto L900;
	} else {
		goto L5;
	}
L5:
	m = n;
	ntwo = np1;
	if_ = 1;
	idiv = 2;
L10:
	iquot = m / idiv;
	irem = m - idiv * iquot;
	if (iquot - idiv >= 0) {
		goto L11;
	} else {
		goto L50;
	}
L11:
	if (irem != 0) {
		goto L20;
	} else {
		goto L12;
	}
L12:
	ntwo += ntwo;
	m = iquot;
	goto L10;
L20:
	idiv = 3;
L30:
	iquot = m / idiv;
	irem = m - idiv * iquot;
	if (iquot - idiv >= 0) {
		goto L31;
	} else {
		goto L60;
	}
L31:
	if (irem != 0) {
		goto L40;
	} else {
		goto L32;
	}
L32:
	ifact[if_ - 1] = idiv;
	++if_;
	m = iquot;
	goto L30;
L40:
	idiv += 2;
	goto L30;
L50:
	if (irem != 0) {
		goto L60;
	} else {
		goto L51;
	}
L51:
	ntwo += ntwo;
	goto L70;
L60:
	ifact[if_ - 1] = m;
L70:
	non2 = np1 * (np2 / ntwo);
	icase = 1;
	if (idim - 4 >= 0) {
		goto L90;
	} else {
		goto L71;
	}
L71:
	if (*iform <= 0) {
		goto L72;
	} else {
		goto L90;
	}
L72:
	icase = 2;
	if (idim - 1 <= 0) {
		goto L73;
	} else {
		goto L90;
	}
L73:
	icase = 3;
	if (ntwo - np1 <= 0) {
		goto L90;
	} else {
		goto L74;
	}
L74:
	icase = 4;
	ntwo /= 2;
	n /= 2;
	np2 /= 2;
	ntot /= 2;
	i = 3;
	i__2 = ntot;
	for (j = 2; j <= i__2; ++j) {
		data[j] = data[i];
		i += 2;
	}
L90:
	i1rng = np1;
	if (icase - 2 != 0) {
		goto L100;
	} else {
		goto L95;
	}
L95:
	i1rng = np0 * (nprev / 2 + 1);
L100:
	if (ntwo - np1 <= 0) {
		goto L600;
	} else {
		goto L110;
	}
L110:
	np2hf = np2 / 2;
	j = 1;
	i__2 = np2;
	i__3 = non2;
	for (i2 = 1; i__3 < 0 ? i2 >= i__2 : i2 <= i__2; i2 += i__3) {
		if (j - i2 >= 0) {
		goto L130;
		} else {
		goto L120;
		}
L120:
		i1max = i2 + non2 - 2;
		i__4 = i1max;
		for (i1 = i2; i1 <= i__4; i1 += 2) {
		i__5 = ntot;
		i__6 = np2;
		for (i3 = i1; i__6 < 0 ? i3 >= i__5 : i3 <= i__5; i3 += i__6) {
			j3 = j + i3 - i2;
			tempr = data[i3];
			tempi = data[i3 + 1];
			data[i3] = data[j3];
			data[i3 + 1] = data[j3 + 1];
			data[j3] = tempr;
			data[j3 + 1] = tempi;
		}
		}
L130:
		m = np2hf;
L140:
		if (j - m <= 0) {
		goto L150;
		} else {
		goto L145;
		}
L145:
		j -= m;
		m /= 2;
		if (m - non2 >= 0) {
		goto L140;
		} else {
		goto L150;
		}
L150:
		j += m;
	}
	non2t = non2 + non2;
	ipar = ntwo / np1;
L310:
	if ((i__3 = ipar - 2) < 0) {
		goto L350;
	} else if (i__3 == 0) {
		goto L330;
	} else {
		goto L320;
	}
L320:
	ipar /= 4;
	goto L310;
L330:
	i__3 = i1rng;
	for (i1 = 1; i1 <= i__3; i1 += 2) {
		i__2 = non2;
		i__6 = np1;
		for (j3 = i1; i__6 < 0 ? j3 >= i__2 : j3 <= i__2; j3 +=  i__6) {
		i__5 = ntot;
		i__4 = non2t;
		for (k1 = j3; i__4 < 0 ? k1 >= i__5 : k1 <= i__5; k1 += i__4) {
			k2 = k1 + non2;
			tempr = data[k2];
			tempi = data[k2 + 1];
			data[k2] = data[k1] - tempr;
			data[k2 + 1] = data[k1 + 1] - tempi;
			data[k1] += tempr;
			data[k1 + 1] += tempi;
		}
		}
	}
L350:
	mmax = non2;
L360:
	if (mmax - np2hf >= 0) {
		goto L600;
	} else {
		goto L370;
	}
L370:
/* Computing MAX */
	i__4 = non2t, i__5 = mmax / 2;
	lmax = MAX(i__4,i__5);
	if (mmax - non2 <= 0) {
		goto L405;
	} else {
		goto L380;
	}
L380:
	theta = -twopi * (float) non2 / (float) (mmax <<  2);
	if (*ksign >= 0) {
		goto L390;
	} else {
		goto L400;
	}
L390:
	theta = -theta;
L400:
	sincos ((double)theta, &wid, &wrd);
	wr = (float)wrd;
	wi = (float)wid;
	wstpr = (float)-2.0 * wi * wi;
	wstpi = (float)2.0 * wr * wi;
L405:
	i__4 = lmax;
	i__5 = non2t;
	for (l = non2; i__5 < 0 ? l >= i__4 : l <= i__4; l += i__5) {
		m = l;
		if (mmax - non2 <= 0) {
		goto L420;
		} else {
		goto L410;
		}
L410:
		w2r = wr * wr - wi * wi;
		w2i = (float)(wr * 2.0 * wi);
		w3r = w2r * wr - w2i * wi;
		w3i = w2r * wi + w2i * wr;
L420:
		i__6 = i1rng;
		for (i1 = 1; i1 <= i__6; i1 += 2) {
		i__2 = non2;
		i__3 = np1;
		for (j3 = i1; i__3 < 0 ? j3 >= i__2 : j3 <= i__2; j3  += i__3) {
			kmin = j3 + ipar * m;
			if (mmax - non2 <= 0) {
			goto L430;
			} else {
			goto L440;
			}
L430:
			kmin = j3;
L440:
			kdif = ipar * mmax;
L450:
			kstep = kdif << 2;
			i__7 = ntot;
			i__8 = kstep;
			for (k1 = kmin; i__8 < 0 ? k1 >= i__7 : k1 <=  i__7; k1 += i__8) {
			k2 = k1 + kdif;
			k3 = k2 + kdif;
			k4 = k3 + kdif;
			if (mmax - non2 <= 0) {
				goto L460;
			} else {
				goto L480;
			}
L460:
			u1r = data[k1] + data[k2];
			u1i = data[k1 + 1] + data[k2 + 1];
			u2r = data[k3] + data[k4];
			u2i = data[k3 + 1] + data[k4 + 1];
			u3r = data[k1] - data[k2];
			u3i = data[k1 + 1] - data[k2 + 1];
			if (*ksign >= 0) {
				goto L475;
			} else {
				goto L470;
			}
L470:
			u4r = data[k3 + 1] - data[k4 + 1];
			u4i = data[k4] - data[k3];
			goto L510;
L475:
			u4r = data[k4 + 1] - data[k3 + 1];
			u4i = data[k3] - data[k4];
			goto L510;
L480:
			t2r = w2r * data[k2] - w2i * data[k2 + 1];
			t2i = w2r * data[k2 + 1] + w2i * data[k2];
			t3r = wr * data[k3] - wi * data[k3 + 1];
			t3i = wr * data[k3 + 1] + wi * data[k3];
			t4r = w3r * data[k4] - w3i * data[k4 + 1];
			t4i = w3r * data[k4 + 1] + w3i * data[k4];
			u1r = data[k1] + t2r;
			u1i = data[k1 + 1] + t2i;
			u2r = t3r + t4r;
			u2i = t3i + t4i;
			u3r = data[k1] - t2r;
			u3i = data[k1 + 1] - t2i;
			if (*ksign >= 0) {
				goto L500;
			} else {
				goto L490;
			}
L490:
			u4r = t3i - t4i;
			u4i = t4r - t3r;
			goto L510;
L500:
			u4r = t4i - t3i;
			u4i = t3r - t4r;
L510:
			data[k1] = u1r + u2r;
			data[k1 + 1] = u1i + u2i;
			data[k2] = u3r + u4r;
			data[k2 + 1] = u3i + u4i;
			data[k3] = u1r - u2r;
			data[k3 + 1] = u1i - u2i;
			data[k4] = u3r - u4r;
			data[k4 + 1] = u3i - u4i;
			}
			kmin = ((kmin - j3) << 2) + j3;
			kdif = kstep;
			if (kdif - np2 >= 0) {
			goto L530;
			} else {
			goto L450;
			}
L530:
			;
		}
		}
		m = mmax - m;
		if (*ksign >= 0) {
		goto L550;
		} else {
		goto L540;
		}
L540:
		tempr = wr;
		wr = -wi;
		wi = -tempr;
		goto L560;
L550:
		tempr = wr;
		wr = wi;
		wi = tempr;
L560:
		if (m - lmax <= 0) {
		goto L565;
		} else {
		goto L410;
		}
L565:
		tempr = wr;
		wr = wr * wstpr - wi * wstpi + wr;
		wi = wi * wstpr + tempr * wstpi + wi;
	}
	ipar = 3 - ipar;
	mmax += mmax;
	goto L360;
L600:
	if (ntwo - np2 >= 0) {
		goto L700;
	} else {
		goto L605;
	}
L605:
	ifp1 = non2;
	if_ = 1;
	np1hf = np1 / 2;
L610:
	ifp2 = ifp1 / ifact[if_ - 1];
	j1rng = np2;
	if (icase - 3 != 0) {
		goto L612;
	} else {
		goto L611;
	}
L611:
	j1rng = (np2 + ifp1) / 2;
	j2stp = np2 / ifact[if_ - 1];
	j1rg2 = (j2stp + ifp2) / 2;
L612:
	j2min = ifp2 + 1;
	if (ifp1 - np2 >= 0) {
		goto L640;
	} else {
		goto L615;
	}
L615:
	i__5 = ifp1;
	i__4 = ifp2;
	for (j2 = j2min; i__4 < 0 ? j2 >= i__5 : j2 <= i__5; j2 +=  i__4) {
		theta = -twopi * (float) (j2 - 1) / (float)  np2;
		if (*ksign >= 0) {
		goto L620;
		} else {
		goto L625;
		}
L620:
		theta = -theta;
L625:
		sinth = (float)sin((double)(0.5 * theta));
		wstpr = sinth * (float)(-2. * sinth);
		wstpi = (float)sin((double)theta);
		wr = wstpr + (float)1.0;
		wi = wstpi;
		j1min = j2 + ifp1;
		i__3 = j1rng;
		i__2 = ifp1;
		for (j1 = j1min; i__2 < 0 ? j1 >= i__3 : j1 <= i__3; j1 += i__2) {

		i1max = j1 + i1rng - 2;
		i__6 = i1max;
		for (i1 = j1; i1 <= i__6; i1 += 2) {
			i__8 = ntot;
			i__7 = np2;
			for (i3 = i1; i__7 < 0 ? i3 >= i__8 : i3 <= i__8; i3 += i__7) {
			j3max = i3 + ifp2 - np1;
			i__9 = j3max;
			i__10 = np1;
			for (j3 = i3; i__10 < 0 ? j3 >= i__9 : j3 <= i__9; j3 += i__10) {
				tempr = data[j3];
				data[j3] = data[j3] * wr - data[j3 + 1] *  wi;
				data[j3 + 1] = tempr * wi + data[j3 + 1]  * wr;
			}
			}
		}
		tempr = wr;
		wr = wr * wstpr - wi * wstpi + wr;
		wi = tempr * wstpi + wi * wstpr + wi;
		}
	}
L640:
	theta = -twopi / (float) ifact[if_ - 1];
	if (*ksign >= 0) {
		goto L645;
	} else {
		goto L650;
	}
L645:
	theta = -theta;
L650:
	sinth = (float)sin((double)(0.5 * theta));
	wstpr = sinth * (float)(-2. * sinth);
	wstpi = (float)sin((double)theta);
	kstep = (n << 1) / ifact[if_ - 1];
	krang = kstep * (ifact[if_ - 1] / 2) + 1;
	i__2 = i1rng;
	for (i1 = 1; i1 <= i__2; i1 += 2) {
		i__3 = ntot;
		i__4 = np2;
		for (i3 = i1; i__4 < 0 ? i3 >= i__3 : i3 <= i__3; i3 += i__4) {
		i__5 = krang;
		i__10 = kstep;
		for (kmin = 1; i__10 < 0 ? kmin >= i__5 : kmin <= i__5; kmin += i__10) {
			j1max = i3 + j1rng - ifp1;
			i__9 = j1max;
			i__7 = ifp1;
			for (j1 = i3; i__7 < 0 ? j1 >= i__9 : j1 <= i__9; j1 += i__7) {
			j3max = j1 + ifp2 - np1;
			i__8 = j3max;
			i__6 = np1;
			for (j3 = j1; i__6 < 0 ? j3 >= i__8 : j3 <= i__8; j3 += i__6) {
				j2max = j3 + ifp1 - ifp2;
				k = kmin + (j3 - j1 + (j1 - i3) / ifact[if_ - 1]) / np1hf;
				if (kmin - 1 <= 0) {
				goto L655;
				} else {
				goto L665;
				}
L655:
				sumr = (float)0.0;
				sumi = (float)0.0;
				i__11 = j2max;
				i__12 = ifp2;
				for (j2 = j3; i__12 < 0 ? j2 >= i__11 : j2 <= i__11; j2 += i__12) {
				sumr += data[j2];
				sumi += data[j2 + 1];
				}
				work[k] = sumr;
				work[k + 1] = sumi;
				goto L680;
L665:
				kconj = k + ((n - kmin + 1) << 1);
				j2 = j2max;
				sumr = data[j2];
				sumi = data[j2 + 1];
				oldsr = (float)0.0;
				oldsi = (float)0.0;
				j2 -= ifp2;
L670:
				tempr = sumr;
				tempi = sumi;
				sumr = twowr * sumr - oldsr + data[j2];
				sumi = twowr * sumi - oldsi + data[j2 + 1];
				oldsr = tempr;
				oldsi = tempi;
				j2 -= ifp2;
				if (j2 - j3 <= 0) {
				goto L675;
				} else {
				goto L670;
				}
L675:
				tempr = wr * sumr - oldsr + data[j2];
				tempi = wi * sumi;
				work[k] = tempr - tempi;
				work[kconj] = tempr + tempi;
				tempr = wr * sumi - oldsi + data[j2 + 1];
				tempi = wi * sumr;
				work[k + 1] = tempr + tempi;
				work[kconj + 1] = tempr - tempi;
L680:
				;
			}
			}
			if (kmin - 1 <= 0) {
			goto L685;
			} else {
			goto L686;
			}
L685:
			wr = wstpr + (float)1.0;
			wi = wstpi;
			goto L690;
L686:
			tempr = wr;
			wr = wr * wstpr - wi * wstpi + wr;
			wi = tempr * wstpi + wi * wstpr + wi;
L690:
			twowr = wr + wr;
		}
		if (icase - 3 != 0) {
			goto L692;
		} else {
			goto L691;
		}
L691:
		if (ifp1 - np2 >= 0) {
			goto L692;
		} else {
			goto L695;
		}
L692:
		k = 1;
		i2max = i3 + np2 - np1;
		i__10 = i2max;
		i__5 = np1;
		for (i2 = i3; i__5 < 0 ? i2 >= i__10 : i2 <= i__10; i2 += i__5) {
			data[i2] = work[k];
			data[i2 + 1] = work[k + 1];
			k += 2;
		}
		goto L698;
L695:
		j3max = i3 + ifp2 - np1;
		i__5 = j3max;
		i__10 = np1;
		for (j3 = i3; i__10 < 0 ? j3 >= i__5 : j3 <= i__5; j3 += i__10) {
			j2max = j3 + np2 - j2stp;
			i__6 = j2max;
			i__8 = j2stp;
			for (j2 = j3; i__8 < 0 ? j2 >= i__6 : j2 <= i__6; j2 += i__8) {
			j1max = j2 + j1rg2 - ifp2;
			j1cnj = j3 + j2max + j2stp - j2;
			i__7 = j1max;
			i__9 = ifp2;
			for (j1 = j2; i__9 < 0 ? j1 >= i__7 : j1 <= i__7; j1 += i__9) {
				k = j1 + 1 - i3;
				data[j1] = work[k];
				data[j1 + 1] = work[k + 1];
				if (j1 - j2 <= 0) {
				goto L697;
				} else {
				goto L696;
				}
L696:
				data[j1cnj] = work[k];
				data[j1cnj + 1] = -work[k + 1];
L697:
				j1cnj -= ifp2;
			}
			}
		}
L698:
		;
		}
	}
	++if_;
	ifp1 = ifp2;
	if (ifp1 - np1 <= 0) {
		goto L700;
	} else {
		goto L610;
	}
L700:
	switch ((int)icase) {
		case 1:  goto L900;
		case 2:  goto L800;
		case 3:  goto L900;
		case 4:  goto L701;
	}
L701:
	nhalf = n;
	n += n;
	theta = -twopi / (float) n;
	if (*ksign >= 0) {
		goto L702;
	} else {
		goto L703;
	}
L702:
	theta = -theta;
L703:
	sinth = (float)sin((double)(0.5 * theta));
	wstpr = sinth * (float)(-2. * sinth);
	wstpi = (float)sin((double)theta);
	wr = wstpr + (float)1.0;
	wi = wstpi;
	imin = 3;
	jmin = (nhalf << 1) - 1;
	goto L725;
L710:
	j = jmin;
	i__4 = ntot;
	i__3 = np2;
	for (i = imin; i__3 < 0 ? i >= i__4 : i <= i__4; i += i__3) {
		sumr = (float)0.5 * (data[i] + data[j]);
		sumi = (float)0.5 * (data[i + 1] + data[j + 1]);
		difr = (float)0.5 * (data[i] - data[j]);
		difi = (float)0.5 * (data[i + 1] - data[j + 1]);
		tempr = wr * sumi + wi * difr;
		tempi = wi * sumi - wr * difr;
		data[i] = sumr + tempr;
		data[i + 1] = difi + tempi;
		data[j] = sumr - tempr;
		data[j + 1] = -difi + tempi;
		j += np2;
	}
	imin += 2;
	jmin += -2;
	tempr = wr;
	wr = wr * wstpr - wi * wstpi + wr;
	wi = tempr * wstpi + wi * wstpr + wi;
L725:
	if ((i__3 = imin - jmin) < 0) {
		goto L710;
	} else if (i__3 == 0) {
		goto L730;
	} else {
		goto L740;
	}
L730:
	if (*ksign >= 0) {
		goto L740;
	} else {
		goto L731;
	}
L731:
	i__3 = ntot;
	i__4 = np2;
	for (i = imin; i__4 < 0 ? i >= i__3 : i <= i__3; i += i__4) {
		data[i + 1] = -data[i + 1];
	}
L740:
	np2 += np2;
	ntot += ntot;
	j = ntot + 1;
	imax = ntot / 2 + 1;
L745:
	imin = imax - (nhalf << 1);
	i = imin;
	goto L755;
L750:
	data[j] = data[i];
	data[j + 1] = -data[i + 1];
L755:
	i += 2;
	j += -2;
	if (i - imax >= 0) {
		goto L760;
	} else {
		goto L750;
	}
L760:
	data[j] = data[imin] - data[imin + 1];
	data[j + 1] = (float)0.0;
	if (i - j >= 0) {
		goto L780;
	} else {
		goto L770;
	}
L765:
	data[j] = data[i];
	data[j + 1] = data[i + 1];
L770:
	i += -2;
	j += -2;
	if (i - imin <= 0) {
		goto L775;
	} else {
		goto L765;
	}
L775:
	data[j] = data[imin] + data[imin + 1];
	data[j + 1] = (float)0.0;
	imax = imin;
	goto L745;
L780:
	data[1] += data[2];
	data[2] = (float)0.0;
	goto L900;
L800:
	if (i1rng - np1 >= 0) {
		goto L900;
	} else {
		goto L805;
	}
L805:
	i__4 = ntot;
	i__3 = np2;
	for (i3 = 1; i__3 < 0 ? i3 >= i__4 : i3 <= i__4; i3 += i__3) {
		i2max = i3 + np2 - np1;
		i__2 = i2max;
		i__9 = np1;
		for (i2 = i3; i__9 < 0 ? i2 >= i__2 : i2 <= i__2; i2 += i__9) {
		imin = i2 + i1rng;
		imax = i2 + np1 - 2;
		jmax = (i3 << 1) + np1 - imin;
		if (i2 - i3 <= 0) {
			goto L820;
		} else {
			goto L810;
		}
L810:
		jmax += np2;
L820:
		if (idim - 2 <= 0) {
			goto L850;
		} else {
			goto L830;
		}
L830:
		j = jmax + np0;
		i__7 = imax;
		for (i = imin; i <= i__7; i += 2) {
			data[i] = data[j];
			data[i + 1] = -data[j + 1];
			j += -2;
		}
L850:
		j = jmax;
		i__7 = imax;
		i__8 = np0;
		for (i = imin; i__8 < 0 ? i >= i__7 : i <= i__7; i += i__8) {
			data[i] = data[j];
			data[i + 1] = -data[j + 1];
			j -= np0;
		}
		}
	}
L900:
	np0 = np1;
	np1 = np2;
	nprev = n;
	}
L920:
	return 0;
} /* fourt_ */

GMT_LONG gmt_get_non_symmetric_f (GMT_LONG *f, GMT_LONG n)
{
	/* Return the product of the non-symmetric factors in f[]  */
	GMT_LONG i = 0, j = 1, retval = 1;

	if (n == 1) return (f[0]);

	while (i < n) {
		while (j < n && f[j] == f[i]) j++;
		if ((j-i)%2) retval *= f[i];
		i = j;
		j = i + 1;
	}
	if (retval == 1) retval = 0;	/* There are no non-sym factors  */
	return (retval);
}

GMT_LONG brenner_worksize (GMT_LONG nx, GMT_LONG ny)
{
	/* Find the size of the workspace that will be needed by the transform.
	 * To use this routine for a 1-D transform, set ny = 1.
	 * 
	 * This is all based on the comments in Norman Brenner's code
	 * FOURT, from which our C codes are translated.
	 * 
	 * Let m = largest prime factor in the list of factors.
	 * Let p = product of all primes which appear an odd number of
	 * times in the list of prime factors.  Then the worksize needed
	 * s = max(m,p).  However, we know that if n is radix 2, then no
	 * work is required; yet this formula would say we need a worksize
	 * of at least 2.  So I will return s = 0 when max(m,p) = 2.
	 *
	 * W. H. F. Smith, 26 February 1992.
	 *  */

	GMT_LONG f[32], n_factors, nonsymx, nonsymy, nonsym, storage, ntotal;
	EXTERN_MSC GMT_LONG GMT_get_prime_factors (GMT_LONG n, GMT_LONG *f);

	/* Find workspace needed.  First find non_symmetric factors in nx, ny  */
	n_factors = GMT_get_prime_factors (nx, f);
	nonsymx = gmt_get_non_symmetric_f (f, n_factors);
	n_factors = GMT_get_prime_factors (ny, f);
	nonsymy = gmt_get_non_symmetric_f (f, n_factors);
	nonsym = MAX (nonsymx, nonsymy);

	/* Now get factors of ntotal  */
	ntotal = GMT_get_nm (nx,ny);
        n_factors = GMT_get_prime_factors (ntotal, f);
	storage = MAX (nonsym, f[n_factors-1]);
	return (2 * ((storage == 2) ? 0 : storage));
} 

/* C-callable wrapper for BRENNER_fourt_ */

#define GMT_radix2(n) GMT_IS_ZERO(log2 ((double)n)-floor(log2 ((double)n)))

GMT_LONG GMT_fft_1d_general (struct GMT_CTRL *C, float *data, GMT_LONG n, GMT_LONG direction, GMT_LONG mode)
{
	/* void GMT_fourt (struct GMT_CTRL *C, float *data, GMT_LONG *nn, GMT_LONG ndim, GMT_LONG ksign, GMT_LONG iform, float *work) */
	/* Data array */
	/* Dimension array */
	/* Number of dimensions */
	/* Forward(-1) or Inverse(+1) */
	/* Real(0) or complex(1) data */
	/* Work array */
	GMT_LONG ksign, ndim = 1, work_size = 0;
	float *work = NULL;
	ksign = (direction == GMT_FFT_INV) ? +1 : -1;
	if ((work_size = brenner_worksize (n, 1))) work = GMT_memory (C, NULL, work_size, float);
	(void) BRENNER_fourt_ (data, &n, &ndim, &ksign, &mode, work);
	if (work_size) GMT_free (C, work);
	return (GMT_OK);
}
GMT_LONG GMT_fft_1d_radix2 (struct GMT_CTRL *C, float *data, GMT_LONG n, GMT_LONG direction, GMT_LONG mode)
{
	return (GMT_fft_1d_general (C, data, n, direction, mode));
}

GMT_LONG GMT_fft_2d_general (struct GMT_CTRL *C, float *data, GMT_LONG nx, GMT_LONG ny, GMT_LONG direction, GMT_LONG mode)
{
	/* Data array */
	/* Dimension array */
	/* Number of dimensions */
	/* Forward(-1) or Inverse(+1) */
	/* Real(0) or complex(1) data */
	/* Work array */
	GMT_LONG ksign, ndim = 2, nn[2] = {nx, ny}, work_size = 0;
	float *work = NULL;
	ksign = (direction == GMT_FFT_INV) ? +1 : -1;
	if ((work_size = brenner_worksize (nx, ny))) work = GMT_memory (C, NULL, work_size, float);
	(void) BRENNER_fourt_ (data, nn, &ndim, &ksign, &mode, work);
	if (work_size) GMT_free (C, work);
	return (GMT_OK);
}
GMT_LONG GMT_fft_2d_radix2 (struct GMT_CTRL *C, float *data, GMT_LONG nx, GMT_LONG ny, GMT_LONG direction, GMT_LONG mode)
{
	return (GMT_fft_2d_general (C, data, nx, ny, direction, mode));
	
}

/* C-callable wrapper for BRENNER_fourt_ */

void GMT_fourt (struct GMT_CTRL *C, float *data, GMT_LONG *nn, GMT_LONG ndim, GMT_LONG ksign, GMT_LONG iform, float *work)
{
	/* Data array */
	/* Dimension array */
	/* Number of dimensions */
	/* Forward(-1) or Inverse(+1) */
	/* Real(0) or complex(1) data */
	/* Work array */
	(void) BRENNER_fourt_ (data, nn, &ndim, &ksign, &iform, work);
}

GMT_LONG GMT_fft_1d (struct GMT_CTRL *C, float *data, GMT_LONG n, GMT_LONG direction, GMT_LONG mode)
{
	/* data is an array of length n (or 2*n for complex) data points
	 * n is the number of data points
	 * direction is either 0 (forward) or 1(inverse)
	 * mode is either 0(real) or 1(compex)
	 */
	GMT_LONG status;
	if (GMT_radix2 (n))
		status = GMT_fft_1d_radix2 (C, data, n, direction, mode);
	else
		status = GMT_fft_1d_general (C, data, n, direction, mode);
	return (status);
}

GMT_LONG GMT_fft_2d (struct GMT_CTRL *C, float *data, GMT_LONG nx, GMT_LONG ny, GMT_LONG direction, GMT_LONG mode)
{
	/* data is an array of length nx*ny (or 2*nx*ny for complex) data points
	 * nx, ny is the number of data nodes
	 * direction is either 0 (forward) or 1(inverse)
	 * mode is either 0(real) or 1(compex)
	 */
	GMT_LONG status;
	if (GMT_radix2 (nx) && GMT_radix2 (ny))
		status = GMT_fft_2d_radix2 (C, data, nx, ny, direction, mode);
	else
		status = GMT_fft_2d_general (C, data, nx, ny, direction, mode);
	return (status);
}
