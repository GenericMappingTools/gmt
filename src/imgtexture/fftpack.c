/********************************************************************
 *
 * File: fftpack.c
 * Function: Fast discrete Fourier and cosine transforms and inverses
 *
 * Original author: Paul N. Swarztrauber
 * Last modification date: 1985 Apr (public domain)
 *
 * Modifications by: Monty <xiphmont@mit.edu>
 * Last modification date: 1996 Jul 01 (public domain)
 *
 * Modifications by: Leland Brown
 * Last modification date: 2013 Nov 15
 *
 * Copyright (c) 2011-2013 Leland Brown.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************/

/*
 * These Fourier routines were originally based on the Fourier
 * routines of the same names from the NETLIB bihar and fftpack
 * Fortran libraries developed by Paul N. Swarztrauber at the
 * National Center for Atmospheric Research in Boulder, CO, USA.
 * They have been reimplemented in C and optimized in a few ways.
 */

#include "fftpack.h"

#include <math.h>

#define REAL FFTPACK_REAL

static INLINE void rfti1(int n, REAL *RESTRICT wa, int *RESTRICT ifac)
{
    static const int ntryh[4] = { 4,2,3,5 };
    static const REAL tpi = 6.2831853071795864769;
    //static const REAL tpi = 6.283185307179586476925286766559005768;   // long double
    REAL arg, argh, argld, fi;
    int ntry = 0;
    int i;
    int j = -1;
    int k1, l1, l2;
    int ld, ii, ip, is, nq, nr;
    int ido, ipm, nfm1;
    int nl = n;
    int nf = 0;

    do {

        do {
            j++;
            if (j < 4) {
                ntry = ntryh[j];
            } else {
                ntry += 2;
            }
            nq = nl / ntry;
            if (nq < ntry) {
                ntry = nl;
                nq = 1;
                break;
            }
            nr = nl - ntry * nq;
        } while (nr != 0);

        do {
            nf++;
            ifac[nf+1] = ntry;
            nl = nq;
            if (ntry == 2 && nf != 1) {
                ifac[nf+1] = ifac[2];
                ifac[2] = 2;
            }

            if (nl == 1) {
                break;
            }

            nq = nl / ntry;
            nr = nl - ntry * nq;
        } while (nr == 0);

    } while (nl != 1);

    ifac[nf+2] = 0;
    ifac[0] = n;
    ifac[1] = nf;
    argh = tpi / n;
    is = 0;
    nfm1 = nf - 1;
    l1 = 1;


    if (nfm1 == 0) {
        return;
    }

    for (k1=0; k1<nfm1; k1++) {
        ip = ifac[k1+2];
        ld = 0;
        l2 = l1 * ip;
        ido = n / l2;
        ipm = ip - 1;

        for (j=0; j<ipm; j++) {
            ld += l1;
            i = is;
            argld = (REAL)ld * argh;
            fi = 0.0;
            for (ii=2; ii<ido; ii+=2) {
                fi += 1.0;
                arg = fi * argld;
                wa[++i] = cos(arg);
                wa[++i] = sin(arg);
            }
            is += ido;
        }
        l1 = l2;
    }
}

void rffti(int n, REAL *RESTRICT wsave, int *RESTRICT ifac)
{

    if (n == 1) {
        return;
    }
    rfti1(n, wsave, ifac);
}

static INLINE void bluei1(
    int n, int m,
    REAL *RESTRICT wa1, REAL *RESTRICT wa2,
    REAL *RESTRICT wb1, REAL *RESTRICT wb2,
    REAL *RESTRICT wc,  REAL *RESTRICT wm,
    int  *RESTRICT mfac)
{
    static const REAL pi = 3.1415926535897932385;
    //static const REAL pi = 3.141592653589793238462643383279502884;    // long double
    int i, i2;
    int mh;
    long long k, n2;
    REAL dt, theta;
    REAL t, dm;
    
    rffti(m, wm, mfac);
    
    n2 = n + n;
    dt = pi / (REAL)n;
    for (k=1, i2=2; k<n; k++, i2+=2) {
        theta = dt * (REAL)( (k * k) % n2 );
        wc[i2]   = cos(theta);
        wc[i2+1] = sin(theta);
    }
    
    mh = m >> 1;
    dm = 1.0 / (REAL)m;
    wb1[0] = dm;
    wb2[0] = 0.0;
    for (i=1; i<n; i++) {
        i2 = i+i;
        wb1[i] = wb1[m-i] = dm * wc[i2];
        wb2[i] = wb2[m-i] = dm * wc[i2+1];
    }
    for (; i<=mh; i++) {
        wb1[i] = wb1[m-i] = 0.0;
        wb2[i] = wb2[m-i] = 0.0;
    }

    rfftf(m, wb1, wm, mfac);
    rfftf(m, wb2, wm, mfac);

    for (i=2; i<m; i+=2) {
        t         = wb1[i-1] + wb2[i];
        wb1[i-1] -= wb2[i];
        wb2[i]    = wb2[i-1] - wb1[i];
        wb2[i-1] += wb1[i];
        wb1[i]    = t;
    }
}

void cosqi(int n, REAL *RESTRICT wsave, int *RESTRICT ifac)
{
    static const REAL pih = 1.5707963267948966192;
    //static const REAL pih = 1.570796326794896619231321691639751442;   // long double
    int k;
    int nf2;
    int m, mf;
    int sum;
    int *mfac;
    REAL fk, dt;
    REAL *ww, *wa1, *wa2, *wb1, *wb2, *wc, *wm;
    
    // If the convolution in blue1() is replaced with a faster algorithm,
    // then reduce the following value proportionally:
    const int bluestein_threshold = 10;

    dt = pih / n;
    fk = 0.0;
    for (k=1; k<n; k++) {
        fk += 1.0;
        wsave[k] = cos(fk*dt);
    }

    rffti(n, wsave+n, ifac);

    nf2  = ifac[1]+2;
    mfac = ifac+nf2;
    
    if (n < 2) {
        return;
    }

    m  = 4;
    mf = 4;
    while (m < n) {
        m += m;
        mf++;
    }
    m  += m;
    mf /= 2;
    
    sum = 0;
    for (k=2; k<nf2; k++) {
        if (ifac[k] == 4) {
            sum += 5;
        } else {
            sum += 3 + ifac[k];
        }
    }
    if (sum * (long long)n < bluestein_threshold * mf * (long long)m) {
        return; // Bluestein's algorithm may be slower - don't use it
    }

    ww = wsave+n*2;    
    wa1 = ww, ww += m;
    wa2 = ww, ww += m;
    wb1 = ww, ww += m;
    wb2 = ww, ww += m;
    wc  = ww, ww += n+n;
    wm  = ww;//ww += m+m;
    
    bluei1( n, m, wa1, wa2, wb1, wb2, wc, wm, mfac );
}

static INLINE void blue1(
    int n, int m,
    REAL *RESTRICT xr,  REAL *RESTRICT xi,
    REAL *RESTRICT wa1, REAL *RESTRICT wa2,
    REAL *RESTRICT wb1, REAL *RESTRICT wb2,
    REAL *RESTRICT wc,  REAL *RESTRICT wm,
    int  *RESTRICT mfac)
{
    int i, i2;
    REAL t;
    
    if (n < 2) {
        return;
    }

    wa1[0] = xr[0];
    wa2[0] = xi[0];
    for (i=1; i<n; i++) {
        i2 = i+i;
        wa1[i] = xr[i] * wc[i2] + xi[i] * wc[i2+1];
        wa2[i] = xi[i] * wc[i2] - xr[i] * wc[i2+1];
    }
    for (; i<m; i++) {
        wa1[i] = 0.0;
        wa2[i] = 0.0;
    }

    rfftf(m, wa1, wm, mfac);
    rfftf(m, wa2, wm, mfac);

    for (i=0; i<m; i++) {
        t      = wa1[i] * wb1[i] - wa2[i] * wb2[i];
        wa2[i] = wa1[i] * wb2[i] + wa2[i] * wb1[i];
        wa1[i] = t;
    }
    
    rfftb(m, wa1, wm, mfac);
    rfftb(m, wa2, wm, mfac);

    xr[0] = wa1[0];
    xi[0] = wa2[0];
    for (i=1; i<n; i++) {
        i2 = i+i;
        xr[i] = wa1[i] * wc[i2] + wa2[i] * wc[i2+1];
        xi[i] = wa2[i] * wc[i2] - wa1[i] * wc[i2+1];
    }
}

static void bluestein(
    int n, int m,
    REAL *RESTRICT xr, REAL *RESTRICT xi,
    REAL *RESTRICT ww, int  *RESTRICT mfac)
{
    REAL *wa1, *wa2, *wb1, *wb2, *wc, *wm;

    wa1 = ww, ww += m;
    wa2 = ww, ww += m;
    wb1 = ww, ww += m;
    wb2 = ww, ww += m;
    wc  = ww, ww += n+n;
    wm  = ww;
    
    blue1(n, m, xr, xi, wa1, wa2, wb1, wb2, wc, wm, mfac);
}

static INLINE void radf2(
    int ido, int l1,
    REAL *RESTRICT cc, REAL *RESTRICT ch,
    REAL *RESTRICT wa1)
{
    int i, k;
    REAL ti2, tr2;
    int t0, t1, t2, t3, t4, t5, t6;
    
    t1 = 0;         // t1 = k*ido+0*l1*ido
    t2 = l1 * ido;  // t2 = k*ido+1*l1*ido
    t0 = t2;        // t0 = 1*l1*ido
    t3 = ido << 1;  // t3 = 2*ido
    for (k=0; k<l1; k++) {
        ch[t1<<1]        = cc[t1] + cc[t2]; // t1<<1 = 0*ido+k*2*ido
        ch[(t1<<1)+t3-1] = cc[t1] - cc[t2]; // (t1<<1)+t3-1 = ido-1+1*ido+k*2*ido
        t1 += ido;
        t2 += ido;
    }

    if (ido < 2) {
        return;
    }

    if (ido > 2) {
        t1 = 0;                     // t1 = k*ido
        t2 = t0;                    // t2 = k*ido+1*l1*ido
        for (k=0; k<l1; k++) {
            t3 = t2;                // t3 = i+k*ido+1*l1*ido
            t4 = (t1 + ido) << 1;   // t4 = ido-i+1*ido+k*2*ido
            t5 = t1;                // t5 = i+k*ido+0*l1*ido
            t6 = t1 + t1;           // t6 = i+k*2*ido
            for (i=2; i<ido; i+=2) {
                t3 += 2;
                t4 -= 2;
                t5 += 2;
                t6 += 2;
                tr2 = wa1[i-1] * cc[t3-1] + wa1[i] * cc[t3];
                ti2 = wa1[i-1] * cc[t3]   - wa1[i] * cc[t3-1];
                ch[t6]   = cc[t5] + ti2;
                ch[t4]   = ti2 - cc[t5];
                ch[t6-1] = cc[t5-1] + tr2;
                ch[t4-1] = cc[t5-1] - tr2;
            }
            t1 += ido;
            t2 += ido;
        }

        if (ido & 1) {
            return;
        }
    }

    t1 = ido;       // t1 = ido+k*2*ido
    t2 = t1 - 1;    // t2 = ido-1+k*ido+1*l1*ido
    t3 = t2;        // t3 = ido-1+k*ido+0*l1*ido
    t2 += t0;
    for (k=0; k<l1; k++) {
        ch[t1]   = -cc[t2];
        ch[t1-1] =  cc[t3];
        t1 += ido << 1;
        t2 += ido;
        t3 += ido;
    }
}

static INLINE void radf3(
    int ido, int l1,
    REAL *RESTRICT cc,  REAL *RESTRICT ch,
    REAL *RESTRICT wa1, REAL *RESTRICT wa2)
{
    static const REAL taur = -.5;
    static const REAL taui =  .8660254037844386468;
    //static const REAL taui =  .866025403784438646763723170752936183;  // long double
    int i, k, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    REAL ci2, di2, di3, cr2, dr2, dr3, ti2, ti3, tr2, tr3;
    
    t0 = l1 * ido;      // t0 = 1*l1*ido

    t1 = 0;                 // t1 = k*ido
    t2 = t0 << 1;           // t2 = 2*l1*ido
    t3 = ido << 1;          // t3 = 2*ido+k*3*ido
    t4 = ido + (ido<<1);    // t4 = 3*ido
    t5 = 0;                 // t5 = 0*ido+k*3*ido
    for (k=0; k<l1; k++) {
        cr2      = cc[t1+t0] + cc[t1+t2];           // t1+t0 = k*ido+1*l1*ido, t1+t2 = k*ido+2*l1*ido
        ch[t5]   = cc[t1]    + cr2;
        ch[t3]   = taui * (cc[t1+t2] - cc[t1+t0]);
        ch[t3-1] = cc[t1] + taur * cr2;
        t1 += ido;
        t3 += t4;
        t5 += t4;
    }

    if (ido == 1) {
        return;
    }

    t1 = 0;                 // t1 = k*ido
    t3 = ido << 1;          // t3 = 2*ido
    for (k=0; k<l1; k++) {
        t7  = t1 + (t1<<1); // t7  = i+0*ido+k*3*ido
        t5  = t7 + t3;      // t5  = i+2*ido+k*3*ido
        t6  = t5;           // t6  = ido-i+1*ido+k*3*ido
        t8  = t1;           // t8  = i+k*ido+0*l1*ido
        t9  = t1 + t0;      // t9  = i+k*ido+1*l1*ido
        t10 = t9 + t0;      // t10 = i+k*ido+2*l1*ido

        for (i=2; i<ido; i+=2) {
            t5 += 2;
            t6 -= 2;
            t7 += 2;
            t8 += 2;
            t9 += 2;
            t10 += 2;
            dr2 = wa1[i-1] * cc[t9-1]  + wa1[i] * cc[t9];
            di2 = wa1[i-1] * cc[t9]    - wa1[i] * cc[t9-1];
            dr3 = wa2[i-1] * cc[t10-1] + wa2[i] * cc[t10];
            di3 = wa2[i-1] * cc[t10]   - wa2[i] * cc[t10-1];
            cr2 = dr2 + dr3;
            ci2 = di2 + di3;
            ch[t7-1] = cc[t8-1] + cr2;
            ch[t7]   = cc[t8]   + ci2;
            tr2 = cc[t8-1] + taur * cr2;
            ti2 = cc[t8]   + taur * ci2;
            tr3 = taui * (di2 - di3);
            ti3 = taui * (dr3 - dr2);
            ch[t5-1] = tr2 + tr3;
            ch[t6-1] = tr2 - tr3;
            ch[t5]   = ti2 + ti3;
            ch[t6]   = ti3 - ti2;
        }
        t1 += ido;
    }
}

static INLINE void radf4(
    int ido, int l1,
    REAL *RESTRICT cc,  REAL *RESTRICT ch,
    REAL *RESTRICT wa1, REAL *RESTRICT wa2, REAL *RESTRICT wa3)
{
    static const REAL hsqt2 = .7071067811865475244;
    //static const REAL hsqt2 = .707106781186547524400844362104849039;  // long double
    int i, k, t0, t1, t2, t3, t4, t5, t6;
    REAL ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, tr3, tr4;
    
    t0 = l1 * ido;      // t0 = 1*l1*ido
    
    t1 = t0;            // t1 = k*ido+1*l1*ido
    t4 = t1 << 1;       // t4 = k*ido+2*l1*ido
    t2 = t1 + (t1<<1);  // t2 = k*ido+3*l1*ido
    t3 = 0;             // t3 = k*ido+0*l1*ido

    for (k=0; k<l1; k++) {
        tr1 = cc[t1] + cc[t2];
        tr2 = cc[t3] + cc[t4];
        t5 = t3 << 2;                   // t5 = k*4*ido
        ch[t5]            = tr1 + tr2;
        ch[(ido<<2)+t5-1] = tr2 - tr1;  // (ido<<2)+t5-1 = ido-1+3*ido+k*4*ido
        t5 += ido << 1;                 // t5 = 2*ido+k*4*ido
        ch[t5-1] = cc[t3] - cc[t4];
        ch[t5]   = cc[t2] - cc[t1];

        t1 += ido;
        t2 += ido;
        t3 += ido;
        t4 += ido;
    }

    if (ido < 2) {
        return;
    }
    
    if (ido > 2) {
        t1 = 0;                     // t1 = k*ido
        for (k=0; k<l1; k++) {
            t2 = t1;                // t2 = i+k*ido
            t4 = t1 << 2;           // t4 = i+0*ido+k*4*ido
            t6 = ido << 1;          // t6 = 2*ido
            t5 = t6 + t4;           // t5 = ido-i+1*ido+k*4*ido
            for (i=2; i<ido; i+=2) {
                t2 += 2;
                t3 = t2;            // t3 = i+k*ido+0*l1*ido, i+k*ido+1*l1*ido,
                                    //      i+k*ido+2*l1*ido, i+k*ido+3*l1*ido
                t4 += 2;
                t5 -= 2;

                t3 += t0;
                cr2 = wa1[i-1] * cc[t3-1] + wa1[i] * cc[t3];
                ci2 = wa1[i-1] * cc[t3]   - wa1[i] * cc[t3-1];
                t3 += t0;
                cr3 = wa2[i-1] * cc[t3-1] + wa2[i] * cc[t3];
                ci3 = wa2[i-1] * cc[t3]   - wa2[i] * cc[t3-1];
                t3 += t0;
                cr4 = wa3[i-1] * cc[t3-1] + wa3[i] * cc[t3];
                ci4 = wa3[i-1] * cc[t3]   - wa3[i] * cc[t3-1];

                tr1 = cr2 + cr4;
                tr4 = cr4 - cr2;
                ti1 = ci2 + ci4;
                ti4 = ci2 - ci4;
                ti2 = cc[t2]   + ci3;
                ti3 = cc[t2]   - ci3;
                tr2 = cc[t2-1] + cr3;
                tr3 = cc[t2-1] - cr3;

            
                ch[t4-1] = tr1 + tr2;
                ch[t4]   = ti1 + ti2;

                ch[t5-1] = tr3 - ti4;
                ch[t5]   = tr4 - ti3;

                ch[t4+t6-1] = ti4 + tr3;    // t4+t6 = i+2*ido+k*4*ido
                ch[t4+t6]   = tr4 + ti3;

                ch[t5+t6-1] = tr2 - tr1;    // t5+t6 = ido-i+3*ido+k*4*ido
                ch[t5+t6]   = ti1 - ti2;
            }
            t1 += ido;
        }
        if (ido & 1) {
            return;
        }
    }
    
    t1 = t0 + ido - 1;  // t1 = ido-1+k*ido+1*l1*ido
    t2 = t1 + (t0<<1);  // t2 = ido-1+k*ido+3*l1*ido
    t3 = ido << 2;      // t3 = 4*ido
    t4 = ido;           // t4 = 1*ido+k*4*ido
    t5 = ido << 1;      // t5 = 2*ido
    t6 = ido;           // t6-1 = ido-1+k*ido

    for (k=0; k<l1; k++) {
        ti1 = -hsqt2 * (cc[t1] + cc[t2]);
        tr1 =  hsqt2 * (cc[t1] - cc[t2]);
        ch[t4-1]    = tr1 + cc[t6-1];
        ch[t4+t5-1] = cc[t6-1] - tr1;   // t4+t5 = 3*ido+k*4*ido
        ch[t4]      = ti1 - cc[t1+t0];  // t1+t0 = ido-1+k*ido+2*l1*ido
        ch[t4+t5]   = ti1 + cc[t1+t0];
        t1 += ido;
        t2 += ido;
        t4 += t3;
        t6 += ido;
    }
}

static INLINE void radf5(
    int ido, int l1,
    REAL *RESTRICT cc,  REAL *RESTRICT ch,
    REAL *RESTRICT wa1, REAL *RESTRICT wa2, REAL *RESTRICT wa3, REAL *RESTRICT wa4)
{
    static const REAL tr11 =  .3090169943749474241;
    static const REAL ti11 =  .9510565162951535721;
    static const REAL tr12 = -.8090169943749474241;
    static const REAL ti12 =  .5877852522924731292;
    //static const REAL tr11 =  .309016994374947424102293417182819059;  // long double
    //static const REAL ti11 =  .951056516295153572116439333379382143;  // long double
    //static const REAL tr12 = -.809016994374947424102293417182819059;  // long double
    //static const REAL ti12 =  .587785252292473129168705954639072769;  // long double
    int i, k;
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16;
    REAL ci2, ci3, ci4, ci5, di2, di3, di4, di5;
    REAL cr2, cr3, cr4, cr5, dr2, dr3, dr4, dr5;
    REAL ti2, ti3, ti4, ti5, tr2, tr3, tr4, tr5;

    t0 = l1 * ido;          // t0 = 1*l1*ido

    t1 = 0;                 // t1 = k*ido
    t2 = t0 << 1;           // t2 = 2*l1*ido
    t3 = t0 + t2;           // t3 = 3*l1*ido
    t4 = t2 << 1;           // t4 = 4*l1*ido
    t5 = ido << 1;          // t5 = 2*ido+k*5*ido
    t6 = ido << 2;          // t6 = 4*ido+k*5*ido
    t7 = ido + (ido<<2);    // t7 = 5*ido
    t8 = 0;                 // t8 = 0*ido+k*5*ido
    for (k=0; k<l1; k++) {
        cr2 = cc[t1+t4] + cc[t1+t0];    // t1+t4 = k*ido+4*l1*ido, t1+t0 = k*ido+1*l1*ido
        ci5 = cc[t1+t4] - cc[t1+t0];
        cr3 = cc[t1+t3] + cc[t1+t2];    // t1+t3 = k*ido+3*l1*ido, t1+t2 = k*ido+2*l1*ido
        ci4 = cc[t1+t3] - cc[t1+t2];
        ch[t8]   = cc[t1] + cr2 + cr3;
        ch[t5-1] = cc[t1] + tr11 * cr2 + tr12 * cr3;
        ch[t5]   =          ti11 * ci5 + ti12 * ci4;
        ch[t6-1] = cc[t1] + tr12 * cr2 + tr11 * cr3;
        ch[t6]   =          ti12 * ci5 - ti11 * ci4;
        t1 += ido;
        t5 += t7;
        t6 += t7;
        t8 += t7;
    }

    if (ido == 1) {
        return;
    }

    t1 = 0;                 // t1 = k*ido
    t5 = ido << 1;          // t5 = 2*ido
    for (k=0; k<l1; k++) {
        t6  = t1 + (t1<<2); // t6  = i+0*ido+k*5*ido
        t8  = t6 + t5;      // t8  = i+2*ido+k*5*ido
        t9  = t8;           // t9  = ido-i+1*ido+k*5*ido
        t10 = t8 + t5;      // t10 = i+4*ido+k*5*ido
        t11 = t10;          // t11 = ido-i+3*ido+k*5*ido
        t12 = t1;           // t12 = i+k*ido+0*l1*ido
        t13 = t1  + t0;     // t13 = i+k*ido+1*l1*ido
        t14 = t13 + t0;     // t14 = i+k*ido+2*l1*ido
        t15 = t14 + t0;     // t15 = i+k*ido+3*l1*ido
        t16 = t15 + t0;     // t16 = i+k*ido+4*l1*ido

        for (i=2; i<ido; i+=2) {
            t6  += 2;
            t8  += 2;
            t9  -= 2;
            t10 += 2;
            t11 -= 2;
            t12 += 2;
            t13 += 2;
            t14 += 2;
            t15 += 2;
            t16 += 2;
            dr2 = wa1[i-1] * cc[t13-1] + wa1[i] * cc[t13];
            di2 = wa1[i-1] * cc[t13]   - wa1[i] * cc[t13-1];
            dr3 = wa2[i-1] * cc[t14-1] + wa2[i] * cc[t14];
            di3 = wa2[i-1] * cc[t14]   - wa2[i] * cc[t14-1];
            dr4 = wa3[i-1] * cc[t15-1] + wa3[i] * cc[t15];
            di4 = wa3[i-1] * cc[t15]   - wa3[i] * cc[t15-1];
            dr5 = wa4[i-1] * cc[t16-1] + wa4[i] * cc[t16];
            di5 = wa4[i-1] * cc[t16]   - wa4[i] * cc[t16-1];
            cr2 = dr2 + dr5;
            ci5 = dr5 - dr2;
            cr5 = di2 - di5;
            ci2 = di2 + di5;
            cr3 = dr3 + dr4;
            ci4 = dr4 - dr3;
            cr4 = di3 - di4;
            ci3 = di3 + di4;
            ch[t6-1] = cc[t12-1] + cr2 + cr3;
            ch[t6]   = cc[t12]   + ci2 + ci3;
            tr2 = cc[t12-1] + tr11 * cr2 + tr12 * cr3;
            ti2 = cc[t12]   + tr11 * ci2 + tr12 * ci3;
            tr3 = cc[t12-1] + tr12 * cr2 + tr11 * cr3;
            ti3 = cc[t12]   + tr12 * ci2 + tr11 * ci3;
            tr5 =             ti11 * cr5 + ti12 * cr4;
            ti5 =             ti11 * ci5 + ti12 * ci4;
            tr4 =             ti12 * cr5 - ti11 * cr4;
            ti4 =             ti12 * ci5 - ti11 * ci4;
            ch[t8-1]  = tr2 + tr5;
            ch[t9-1]  = tr2 - tr5;
            ch[t8]    = ti2 + ti5;
            ch[t9]    = ti5 - ti2;
            ch[t10-1] = tr3 + tr4;
            ch[t11-1] = tr3 - tr4;
            ch[t10]   = ti3 + ti4;
            ch[t11]   = ti4 - ti3;
        }
        t1 += ido;
    }
}

static void radfg(
    int ido, int ip, int l1, int idl1,
    REAL *RESTRICT cc, REAL *RESTRICT ch,
    REAL *RESTRICT wa)
{
    static const REAL tpi = 6.2831853071795864769;
    //static const REAL tpi = 6.283185307179586476925286766559005768;   // long double
    int idij, ipph, i, j, k, l, ic, ik, is;
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    REAL dc2, ai1, ai2, ar1, ar2, ds2;
    int nbd;
    REAL dcp, arg, dsp, ar1h, ar2h;
    int idp2, ipp2;
    
    arg = tpi / (REAL)ip;
    dcp = cos(arg);
    dsp = sin(arg);
    ipph = (ip+1) >> 1;
    ipp2 = ip;
    idp2 = ido;
    nbd = (ido-1) >> 1;
    t0 = l1*ido;
    t10 = ip*ido;

    if (ido == 1) {
        for (ik=0; ik<idl1; ik++) {
            cc[ik] = ch[ik];
        }
    } else {
        for (ik=0; ik<idl1; ik++) {
            ch[ik] = cc[ik];
        }

        t1 = 0;
        for (j=1; j<ip; j++) {
            t1 += t0;
            t2 = t1;
            for (k=0; k<l1; k++) {
                ch[t2] = cc[t2];
                t2 += ido;
            }
        }

        is=-ido;
        t1 = 0;
        if (nbd > l1) {
            for (j=1; j<ip; j++) {
                t1 += t0;
                is += ido;
                t2 = -ido+t1;
                for (k=0; k<l1; k++) {
                    idij = is - 1;
                    t2 += ido;
                    t3 = t2;
                    for (i=2; i<ido; i+=2) {
                        idij += 2;
                        t3 += 2;
                        ch[t3-1] = wa[idij] * cc[t3-1] + wa[idij+1] * cc[t3];
                        ch[t3]   = wa[idij] * cc[t3]   - wa[idij+1] * cc[t3-1];
                    }
                }
            }
        } else {

            for (j=1; j<ip; j++) {
                is += ido;
                idij = is-1;
                t1 += t0;
                t2 = t1;
                for (i=2; i<ido; i+=2) {
                    idij += 2;
                    t2 += 2;
                    t3 = t2;
                    for (k=0; k<l1; k++) {
                        ch[t3-1] = wa[idij] * cc[t3-1] + wa[idij+1] * cc[t3];
                        ch[t3]   = wa[idij] * cc[t3]   - wa[idij+1] * cc[t3-1];
                        t3 += ido;
                    }
                }
            }
        }

        t1 = 0;
        t2 = ipp2 * t0;
        if (nbd < l1) {
            for (j=1; j<ipph; j++) {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                for (i=2; i<ido; i+=2) {
                    t3 += 2;
                    t4 += 2;
                    t5 = t3 - ido;
                    t6 = t4 - ido;
                    for (k=0; k<l1; k++) {
                        t5 += ido;
                        t6 += ido;
                        cc[t5-1] = ch[t5-1] + ch[t6-1];
                        cc[t6-1] = ch[t5]   - ch[t6];
                        cc[t5]   = ch[t5]   + ch[t6];
                        cc[t6]   = ch[t6-1] - ch[t5-1];
                    }
                }
            }
        } else {
            for (j=1; j<ipph; j++) {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                for (k=0; k<l1; k++) {
                    t5 = t3;
                    t6 = t4;
                    for (i=2; i<ido; i+=2) {
                        t5 += 2;
                        t6 += 2;
                        cc[t5-1] = ch[t5-1] + ch[t6-1];
                        cc[t6-1] = ch[t5]   - ch[t6];
                        cc[t5]   = ch[t5]   + ch[t6];
                        cc[t6]   = ch[t6-1] - ch[t5-1];
                    }
                    t3 += ido;
                    t4 += ido;
                }
            }
        }
    }

    t1 = 0;
    t2 = ipp2 * idl1;
    for (j=1; j<ipph; j++) {
        t1 += t0;
        t2 -= t0;
        t3 = t1 - ido;
        t4 = t2 - ido;
        for (k=0; k<l1; k++) {
            t3 += ido;
            t4 += ido;
            cc[t3] = ch[t3] + ch[t4];
            cc[t4] = ch[t4] - ch[t3];
        }
    }

    ar1 = 1.0;
    ai1 = 0.0;
    t1 = 0;
    t2 = ipp2 * idl1;
    t3 = (ip-1) * idl1;
    for (l=1; l<ipph; l++) {
        t1 += idl1;
        t2 -= idl1;
        ar1h = dcp * ar1 - dsp * ai1;
        ai1  = dcp * ai1 + dsp * ar1;
        ar1  = ar1h;
        t4 = t1;
        t5 = t2;
        t6 = t3;
        t7 = idl1;

        for (ik=0; ik<idl1; ik++) {
            ch[t4++] = cc[ik] + ar1 * cc[t7++];
            ch[t5++] = ai1 * cc[t6++];
        }

        dc2 = ar1;
        ds2 = ai1;
        ar2 = ar1;
        ai2 = ai1;

        t4 = idl1;
        t5 = (ipp2-1) * idl1;
        for (j=2; j<ipph; j++) {
            t4 += idl1;
            t5 -= idl1;

            ar2h = dc2 * ar2 - ds2 * ai2;
            ai2  = dc2 * ai2 + ds2 * ar2;
            ar2  = ar2h;

            t6 = t1;
            t7 = t2;
            t8 = t4;
            t9 = t5;
            for (ik=0; ik<idl1; ik++) {
                ch[t6++] += ar2 * cc[t8++];
                ch[t7++] += ai2 * cc[t9++];
            }
        }
    }

    t1 = 0;
    for (j=1; j<ipph; j++) {
        t1 += idl1;
        t2 = t1;
        for (ik=0; ik<idl1; ik++) {
            ch[ik] += cc[t2++];
        }
    }

    if (ido >= l1) {
    t1 = 0;
    t2 = 0;
    for (k=0; k<l1; k++) {
        t3 = t1;
        t4 = t2;
        for (i=0; i<ido; i++) {
            cc[t4++] = ch[t3++];
        }
        t1 += ido;
        t2 += t10;
    }
    } else {
    for (i=0; i<ido; i++) {
        t1 = i;
        t2 = i;
        for (k=0; k<l1; k++) {
            cc[t2] = ch[t1];
            t1 += ido;
            t2 += t10;
        }
    }
    }

    t1 = 0;
    t2 = ido << 1;
    t3 = 0;
    t4 = ipp2 * t0;
    for (j=1; j<ipph; j++) {

        t1 += t2;
        t3 += t0;
        t4 -= t0;

        t5 = t1;
        t6 = t3;
        t7 = t4;

        for (k=0; k<l1; k++) {
            cc[t5-1] = ch[t6];
            cc[t5]   = ch[t7];
            t5 += t10;
            t6 += ido;
            t7 += ido;
        }
    }

    if (ido == 1) {
        return;
    }

    if (nbd >= l1) {
        t1 = -ido;
        t3 = 0;
        t4 = 0;
        t5 = ipp2 * t0;
        for (j=1; j<ipph; j++) {
            t1 += t2;
            t3 += t2;
            t4 += t0;
            t5 -= t0;
            t6 = t1;
            t7 = t3;
            t8 = t4;
            t9 = t5;
            for (k=0; k<l1; k++) {
                for (i=2; i<ido; i+=2) {
                    ic = idp2 - i;
                    cc[i+t7-1]  = ch[i+t8-1] + ch[i+t9-1];
                    cc[ic+t6-1] = ch[i+t8-1] - ch[i+t9-1];
                    cc[i+t7]    = ch[i+t8]   + ch[i+t9];
                    cc[ic+t6]   = ch[i+t9]   - ch[i+t8];
                }
                t6 += t10;
                t7 += t10;
                t8 += ido;
                t9 += ido;
            }
        }
        return;
    }

    t1 = -ido;
    t3 = 0;
    t4 = 0;
    t5 = ipp2 * t0;
    for (j=1; j<ipph; j++) {
        t1 += t2;
        t3 += t2;
        t4 += t0;
        t5 -= t0;
        for (i=2; i<ido; i+=2) {
            t6 = idp2 + t1 - i;
            t7 = i + t3;
            t8 = i + t4;
            t9 = i + t5;
            for (k=0; k<l1; k++) {
                cc[t7-1] = ch[t8-1] + ch[t9-1];
                cc[t6-1] = ch[t8-1] - ch[t9-1];
                cc[t7]   = ch[t8]   + ch[t9];
                cc[t6]   = ch[t9]   - ch[t8];
                t6 += t10;
                t7 += t10;
                t8 += ido;
                t9 += ido;
            }
        }
    }
}

static INLINE void rftf1(
    int n, REAL *RESTRICT c, REAL *RESTRICT ch, REAL *RESTRICT wa, int *RESTRICT ifac)
{
    int i, k1, l1, l2;
    int na, kh, nf;
    int ip, iw, ido, idl1, ix2, ix3, ix4;

    nf = ifac[1];
    na = 1;
    l2 = n;
    iw = n-1;

    for (k1=0; k1<nf; k1++) {
        REAL *RESTRICT ca, *RESTRICT cb, *RESTRICT cc;

        kh = nf - k1;
        ip = ifac[kh+1];
        l1 = l2 / ip;
        ido = n / l2;
        idl1 = ido * l1;
        iw -= (ip-1) * ido;
        na = 1 - na;

        if (na != 0) {
            ca = ch;
            cb = c;
        } else {
            ca = c;
            cb = ch;
        }
    
        switch (ip) {

        case 4:
            ix2 = iw  + ido;
            ix3 = ix2 + ido;
            radf4(ido, l1, ca, cb, wa+iw, wa+ix2, wa+ix3);
            break;

        case 2:
            radf2(ido, l1, ca, cb, wa+iw);
            break;

        case 3:
            ix2 = iw + ido;
            radf3(ido, l1, ca, cb, wa+iw, wa+ix2);
            break;

        case 5:

            ix2 = iw  + ido;
            ix3 = ix2 + ido;
            ix4 = ix3 + ido;
            radf5(ido, l1, ca, cb, wa+iw, wa+ix2, wa+ix3, wa+ix4);
            break;

        default:
            if (ido == 1) {
                cc = ca;
                ca = cb;
                cb = cc;
            } else {
                na = 1 - na;
            }
            radfg(ido, ip, l1, idl1, ca, cb, wa+iw);

        }

        l2 = l1;
    }

    if (na == 1) {
        return;
    }

    for (i=0; i<n; i++) {
        c[i] = ch[i];
    }
}

void rfftf(int n, REAL *RESTRICT r, REAL *RESTRICT wsave, int *RESTRICT ifac)
{
    if (n == 1) {
        return;
    }
    rftf1(n, r, wsave+n, wsave, ifac);
}

static INLINE void csqf1(
    int n, REAL *RESTRICT x, REAL *RESTRICT w, REAL *RESTRICT xh, int *RESTRICT ifac)
{
    int modn, i, k, kc;
    int ns2;
    REAL xim1;

    ns2 = (n+1) >> 1;

    kc = n;
    for (k=1; k<ns2; k++) {
        kc--;
        xh[k]  = x[k] + x[kc];
        xh[kc] = x[k] - x[kc];
    }

    modn = n & 1;
    if (modn == 0) {
        xh[ns2] = x[ns2] + x[ns2];
    }

    for (k=1; k<ns2; k++) {
        kc = n - k;
        x[k]  = w[k] * xh[kc] + w[kc] * xh[k];
        x[kc] = w[k] * xh[k]  - w[kc] * xh[kc];
    }

    if (modn == 0) {
        x[ns2] = w[ns2] * xh[ns2];
    }

    rfftf(n, x, w+n, ifac);

    for (i=2; i<n; i+=2) {
        xim1   = x[i-1] - x[i];
        x[i]  += x[i-1];
        x[i-1] = xim1;
    }
}

static INLINE void csqf2(
    int n, int m, REAL *RESTRICT x1, REAL *RESTRICT x2, REAL *RESTRICT w, REAL *RESTRICT xh, int *RESTRICT mfac)
{
    int modn, i, k, k2, kc;
    int ns2;
    REAL t1, t2;

    ns2 = (n+1) >> 1;
    modn = n & 1;

    kc = n;
    for (k=1; k<ns2; k++) {
        kc--;

        t1 = x1[k] + x1[kc];
        t2 = x1[k] - x1[kc];
        x1[k]  = w[k] * t2 + w[kc] * t1;
        x1[kc] = w[k] * t1 - w[kc] * t2;

        t1 = x2[k] + x2[kc];
        t2 = x2[k] - x2[kc];
        x2[k]  = w[k] * t2 + w[kc] * t1;
        x2[kc] = w[k] * t1 - w[kc] * t2;
    }
    if (modn == 0) {
        x1[k] = w[k] * (x1[k] + x1[k]);
        x2[k] = w[k] * (x2[k] + x2[k]);
    }

    bluestein(n, m, x1, x2, xh, mfac);
    
    k2 = 1;
    kc = n;
    for (k=1; k<ns2; k++) {
        kc--;
        xh[k2++] = (x1[kc] + x1[k]) * 0.5;
        xh[k2++] = (x1[kc] - x1[k]) * 0.5;
    }
    if (modn == 0) {
        x1[k2] = x1[k];
    }

    k2 = 1;
    kc = n;
    for (k=1; k<ns2; k++) {
        kc--;
        x1[k2++] = (x2[k] + x2[kc]) * 0.5;
        x1[k2++] = (x2[k] - x2[kc]) * 0.5;
    }
    if (modn == 0) {
        x2[k2] = x2[k];
    }

    for (i=2; i<n; i+=2) {
        x2[i-1] = x1[i-1] - xh[i];
        x2[i]   = x1[i-1] + xh[i];
        x1[i-1] = xh[i-1] - x1[i];
        x1[i]  += xh[i-1];
    }
}

void cosqf(int n, REAL *RESTRICT x, REAL *RESTRICT wsave, int *RESTRICT ifac)
{
    static const REAL sqrt2 = 1.4142135623730950488;
    //static const REAL sqrt2 = 1.414213562373095048801688724209698079; // long double
    REAL tsqx;

    if (n < 2) {
        return;
    }
    if (n == 2) {
        tsqx = sqrt2 * x[1];
        x[1] = x[0] - tsqx;
        x[0] += tsqx;
        return;
    }

    csqf1(n, x, wsave, wsave+n*2, ifac);
}

void cosqf2(int n, REAL *RESTRICT x1, REAL *RESTRICT x2, REAL *RESTRICT wsave, int *RESTRICT ifac)
{
    static const REAL sqrt2 = 1.4142135623730950488;
    //static const REAL sqrt2 = 1.414213562373095048801688724209698079; // long double
    int m;
    int *mfac;
    REAL tsqx;
    REAL *xh;

    if (n < 2) {
        return;
    }
    if (n == 2) {
        tsqx = sqrt2 * x1[1];
        x1[1] = x1[0] - tsqx;
        x1[0] += tsqx;
        tsqx = sqrt2 * x2[1];
        x2[1] = x2[0] - tsqx;
        x2[0] += tsqx;
        return;
    }

    xh = wsave+n*2;
    
    mfac = ifac+ifac[1]+2;
    m    = mfac[0];

    if (m) {
        csqf2(n, m, x1, x2, wsave, xh, mfac);
    } else {
        csqf1(n, x1, wsave, xh, ifac);
        csqf1(n, x2, wsave, xh, ifac);
    }
}

static INLINE void radb2(
    int ido, int l1,
    REAL *RESTRICT cc, REAL *RESTRICT ch,
    REAL *RESTRICT wa1)
{
    int i, k, t0, t1, t2, t3, t4, t5, t6;
    REAL ti2, tr2;

    t0 = l1 * ido;      // t0 = 1*l1*ido
    
    t1 = 0;             // t1 = k*ido
    t2 = 0;             // t2 = k*2*ido
    t3 = (ido<<1) - 1;  // t3 = ido-1+1*ido
    for (k=0; k<l1; k++) {
        ch[t1]    = cc[t2] + cc[t3+t2]; // t3+t2 = ido-1+1*ido+k*2*ido
        ch[t1+t0] = cc[t2] - cc[t3+t2]; // t1+t0 = k*ido+1*l1*ido
        t1 += ido;
        t2 = t1 << 1;
    }

    if (ido < 2) {
        return;
    }

    if (ido > 2) {
        t1 = 0;                 // t1 = k*ido
        t2 = 0;                 // t2 = k*2*ido
        for (k=0; k<l1; k++) {
            t3 = t1;            // t3 = i+k*ido
            t4 = t2;            // t4 = i+0*ido+k*2*ido
            t5 = t4 + (ido<<1); // t5 = ido-i+1*ido+k*2*ido
            t6 = t0 + t1;       // t6 = i+k*ido+1*l1*ido
            for (i=2; i<ido; i+=2) {
                t3 += 2;
                t4 += 2;
                t5 -= 2;
                t6 += 2;
                ch[t3-1] = cc[t4-1] + cc[t5-1];
                tr2      = cc[t4-1] - cc[t5-1];
                ch[t3]   = cc[t4]   - cc[t5];
                ti2      = cc[t4]   + cc[t5];
                ch[t6-1] = wa1[i-1] * tr2 - wa1[i] * ti2;
                ch[t6]   = wa1[i-1] * ti2 + wa1[i] * tr2;
            }
            t1 += ido;
            t2 = t1 << 1;
        }

        if (ido & 1) {
            return;
        }
    }

    t1 = ido - 1;   // t1 = ido-1+k*ido
    t2 = ido - 1;   // t2 = ido-1+k*2*ido
    for (k=0; k<l1; k++) {
        ch[t1]    =   cc[t2]   + cc[t2];
        ch[t1+t0] = -(cc[t2+1] + cc[t2+1]); // t1+t0 = ido-1+k*ido+1*l1*ido
        t1 += ido;
        t2 += ido << 1;
    }
}

static INLINE void radb3(
    int ido, int l1,
    REAL *RESTRICT cc,  REAL *RESTRICT ch,
    REAL *RESTRICT wa1, REAL *RESTRICT wa2)
{
    static const REAL taur = -.5;
    static const REAL taui =  .8660254037844386468;
    //static const REAL taui =  .866025403784438646763723170752936183;  // long double
    int i, k, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    REAL ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;
    
    t0 = l1 * ido;          // t0 = 1*l1*ido

    t1 = 0;                 // t1 = k*ido
    t2 = t0 << 1;           // t2 = 2*l1*ido
    t3 = ido << 1;          // t3 = 2*ido+k*3*ido
    t4 = ido + (ido<<1);    // t4 = 3*ido
    t5 = 0;                 // t5 = 0*ido+k*3*ido
    for (k=0; k<l1; k++) {
        tr2 = cc[t3-1] + cc[t3-1];
        cr2 = cc[t5] + taur * tr2;
        ch[t1] = cc[t5] + tr2;
        ci3 = taui * (cc[t3] + cc[t3]);
        ch[t1+t0] = cr2 - ci3;  // t1+t0 = k*ido+1*l1*ido
        ch[t1+t2] = cr2 + ci3;  // t1+t2 = k*ido+2*l1*ido
        t1 += ido;
        t3 += t4;
        t5 += t4;
    }

    if (ido == 1) {
        return;
    }

    t1 = 0;                 // t1 = k*ido
    t3 = ido << 1;          // t3 = 2*ido
    for (k=0; k<l1; k++) {
        t7  = t1 + (t1<<1); // t7  = i+0*ido+k*3*ido
        t5  = t7 + t3;      // t5  = i+2*ido+k*3*ido
        t6  = t5;           // t6  = ido-i+1*ido+k*3*ido
        t8  = t1;           // t8  = i+k*ido+0*l1*ido
        t9  = t1 + t0;      // t9  = i+k*ido+1*l1*ido
        t10 = t9 + t0;      // t10 = i+k*ido+2*l1*ido

        for (i=2; i<ido; i+=2) {
            t5 += 2;
            t6 -= 2;
            t7 += 2;
            t8 += 2;
            t9 += 2;
            t10 += 2;
            tr2 = cc[t5-1] + cc[t6-1];
            cr2 = cc[t7-1] + taur * tr2;
            ch[t8-1] = cc[t7-1] + tr2;
            ti2 = cc[t5] - cc[t6];
            ci2 = cc[t7] + taur * ti2;
            ch[t8] = cc[t7] + ti2;
            cr3 = taui * (cc[t5-1] - cc[t6-1]);
            ci3 = taui * (cc[t5]   + cc[t6]);
            dr2 = cr2 - ci3;
            dr3 = cr2 + ci3;
            di2 = ci2 + cr3;
            di3 = ci2 - cr3;
            ch[t9-1]  = wa1[i-1] * dr2 - wa1[i] * di2;
            ch[t9]    = wa1[i-1] * di2 + wa1[i] * dr2;
            ch[t10-1] = wa2[i-1] * dr3 - wa2[i] * di3;
            ch[t10]   = wa2[i-1] * di3 + wa2[i] * dr3;
        }
        t1 += ido;
    }
}

static INLINE void radb4(
    int ido, int l1,
    REAL *RESTRICT cc,  REAL *RESTRICT ch,
    REAL *RESTRICT wa1, REAL *RESTRICT wa2, REAL *RESTRICT wa3)
{
    static const REAL sqrt2 = 1.4142135623730950488;
    //static const REAL sqrt2 = 1.414213562373095048801688724209698079; // long double
    int i, k, t0, t1, t2, t3, t4, t5, t6, t7, t8;
    REAL ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, tr3, tr4;
    
    t0 = l1 * ido;      // t0 = 1*l1*ido
    
    t1 = 0;             // t1 = k*ido
    t2 = ido << 2;      // t2 = 4*ido
    t3 = 0;             // t3 = k*4*ido
    t6 = ido << 1;      // t6 = 2*ido
    for (k=0; k<l1; k++) {
        t4 = t3 + t6;   // t4 = 2*ido+k*4*ido, 4*ido+k*4*ido
        t5 = t1;        // t5 = k*ido, k*ido+1*l1*ido, k*ido+2*l1*ido, k*ido+3*l1*ido
        tr3 = cc[t4-1] + cc[t4-1];
        tr4 = cc[t4]   + cc[t4]; 
        tr1 = cc[t3]   - cc[(t4+=t6)-1];
        tr2 = cc[t3]   + cc[t4-1];
        ch[t5]     = tr2 + tr3;
        ch[t5+=t0] = tr1 - tr4;
        ch[t5+=t0] = tr2 - tr3;
        ch[t5+t0]  = tr1 + tr4;
        t1 += ido;
        t3 += t2;
    }

    if (ido < 2) {
        return;
    }

    if (ido > 2) {

        t1 = 0;             // t1 = k*ido
        for (k=0; k<l1; k++) {
            t2 = t1 << 2;   // t2 = i+0*ido+k*4*ido
            t3 = t2 + t6;   // t3 = i+2*ido+k*4*ido
            t4 = t3;        // t4 = ido-i+1*ido+k*4*ido
            t5 = t4 + t6;   // t5 = ido-i+3*ido+k*4*ido
            t7 = t1;        // t7 = i+k*ido+0*l1*ido,
                            // t8 = i+k*ido+1*l1*ido, i+k*ido+2*l1*ido, i+k*ido+3*l1*ido
            for (i=2; i<ido; i+=2) {
                t2 += 2;
                t3 += 2;
                t4 -= 2;
                t5 -= 2;
                t7 += 2;
                ti1 = cc[t2]   + cc[t5];
                ti2 = cc[t2]   - cc[t5];
                ti3 = cc[t3]   - cc[t4];
                tr4 = cc[t3]   + cc[t4];
                tr1 = cc[t2-1] - cc[t5-1];
                tr2 = cc[t2-1] + cc[t5-1];
                ti4 = cc[t3-1] - cc[t4-1];
                tr3 = cc[t3-1] + cc[t4-1];
                ch[t7-1] = tr2 + tr3;
                cr3      = tr2 - tr3;
                ch[t7]   = ti2 + ti3;
                ci3      = ti2 - ti3;
                cr2      = tr1 - tr4;
                cr4      = tr1 + tr4;
                ci2      = ti1 + ti4;
                ci4      = ti1 - ti4;

                t8 = t7 + t0;
                ch[t8-1] = wa1[i-1] * cr2 - wa1[i] * ci2;
                ch[t8]   = wa1[i-1] * ci2 + wa1[i] * cr2;
                t8 += t0;
                ch[t8-1] = wa2[i-1] * cr3 - wa2[i] * ci3;
                ch[t8]   = wa2[i-1] * ci3 + wa2[i] * cr3;
                t8 += t0;
                ch[t8-1] = wa3[i-1] * cr4 - wa3[i] * ci4;
                ch[t8]   = wa3[i-1] * ci4 + wa3[i] * cr4;
            }
            t1 += ido;
        }

        if (ido & 1) {
            return;
        }

    }

    t1 = ido;               // t1 = 1*ido+k*4*ido
    t2 = ido << 2;          // t2 = 4*ido
    t3 = ido - 1;           // t3 = ido-1+k*ido
    t4 = ido + (ido<<1);    // t4 = 3*ido+k*4*ido
    for (k=0; k<l1; k++) {
        t5 = t3;
        ti1 = cc[t1]   + cc[t4];
        ti2 = cc[t4]   - cc[t1];
        tr1 = cc[t1-1] - cc[t4-1];
        tr2 = cc[t1-1] + cc[t4-1];
        ch[t5]     = tr2 + tr2;
        ch[t5+=t0] =  sqrt2 * (tr1 - ti1);
        ch[t5+=t0] = ti2 + ti2;
        ch[t5+t0]  = -sqrt2 * (tr1 + ti1);

        t3 += ido;
        t1 += t2;
        t4 += t2;
    }
}

static INLINE void radb5(
    int ido, int l1,
    REAL *RESTRICT cc,  REAL *RESTRICT ch,
    REAL *RESTRICT wa1, REAL *RESTRICT wa2, REAL *RESTRICT wa3, REAL *RESTRICT wa4)
{
    static const REAL tr11 =  .3090169943749474241;
    static const REAL ti11 =  .9510565162951535721;
    static const REAL tr12 = -.8090169943749474241;
    static const REAL ti12 =  .5877852522924731292;
    //static const REAL tr11 =  .309016994374947424102293417182819059;  // long double
    //static const REAL ti11 =  .951056516295153572116439333379382143;  // long double
    //static const REAL tr12 = -.809016994374947424102293417182819059;  // long double
    //static const REAL ti12 =  .587785252292473129168705954639072769;  // long double
    int i, k;
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16;
    REAL ci2, ci3, ci4, ci5, di2, di3, di4, di5;
    REAL cr2, cr3, cr4, cr5, dr2, dr3, dr4, dr5;
    REAL ti2, ti3, ti4, ti5, tr2, tr3, tr4, tr5;
    
    t0 = l1 * ido;          // t0 = 1*l1*ido

    t1 = 0;                 // t1 = k*ido
    t2 = t0 << 1;           // t2 = 2*l1*ido
    t3 = t0 + t2;           // t3 = 3*l1*ido
    t4 = t2 << 1;           // t4 = 4*l1*ido
    t5 = ido << 1;          // t5 = 2*ido+k*5*ido
    t6 = ido << 2;          // t6 = 4*ido+k*5*ido
    t7 = ido + (ido<<2);    // t7 = 5*ido
    t8 = 0;                 // t8 = 0*ido+k*5*ido
    for (k=0; k<l1; k++) {
        ti5 = cc[t5]   + cc[t5];
        ti4 = cc[t6]   + cc[t6];
        tr2 = cc[t5-1] + cc[t5-1];
        tr3 = cc[t6-1] + cc[t6-1];
        ch[t1] = cc[t8] + tr2 + tr3;
        cr2 = cc[t8] + tr11 * tr2 + tr12 * tr3;
        cr3 = cc[t8] + tr12 * tr2 + tr11 * tr3;
        ci5 =          ti11 * ti5 + ti12 * ti4;
        ci4 =          ti12 * ti5 - ti11 * ti4;
        ch[t1+t0] = cr2 - ci5;  // t1+t0 = k*ido+1*l1*ido
        ch[t1+t2] = cr3 - ci4;  // t1+t2 = k*ido+2*l1*ido
        ch[t1+t3] = cr3 + ci4;  // t1+t3 = k*ido+3*l1*ido
        ch[t1+t4] = cr2 + ci5;  // t1+t4 = k*ido+4*l1*ido
        t1 += ido;
        t5 += t7;
        t6 += t7;
        t8 += t7;
    }

    if (ido == 1) {
        return;
    }

    t1 = 0;                 // t1 = k*ido
    t5 = ido << 1;          // t5 = 2*ido
    for (k=0; k<l1; k++) {
        t6  = t1 + (t1<<2); // t6  = i+0*ido+k*5*ido
        t8  = t6 + t5;      // t8  = i+2*ido+k*5*ido
        t9  = t8;           // t9  = ido-i+1*ido+k*5*ido
        t10 = t8 + t5;      // t10 = i+4*ido+k*5*ido
        t11 = t10;          // t11 = ido-i+3*ido+k*5*ido
        t12 = t1;           // t12 = i+k*ido+0*l1*ido
        t13 = t1  + t0;     // t13 = i+k*ido+1*l1*ido
        t14 = t13 + t0;     // t14 = i+k*ido+2*l1*ido
        t15 = t14 + t0;     // t15 = i+k*ido+3*l1*ido
        t16 = t15 + t0;     // t16 = i+k*ido+4*l1*ido

        for (i=2; i<ido; i+=2) {
            t6  += 2;
            t8  += 2;
            t9  -= 2;
            t10 += 2;
            t11 -= 2;
            t12 += 2;
            t13 += 2;
            t14 += 2;
            t15 += 2;
            t16 += 2;
            ti5 = cc[t8]    + cc[t9];
            ti2 = cc[t8]    - cc[t9];
            ti4 = cc[t10]   + cc[t11];
            ti3 = cc[t10]   - cc[t11];
            tr5 = cc[t8-1]  - cc[t9-1];
            tr2 = cc[t8-1]  + cc[t9-1];
            tr4 = cc[t10-1] - cc[t11-1];
            tr3 = cc[t10-1] + cc[t11-1];
            ch[t12-1] = cc[t6-1] + tr2 + tr3;
            ch[t12]   = cc[t6]   + ti2 + ti3;
            cr2 = cc[t6-1] + tr11 * tr2 + tr12 * tr3;
            ci2 = cc[t6]   + tr11 * ti2 + tr12 * ti3;
            cr3 = cc[t6-1] + tr12 * tr2 + tr11 * tr3;
            ci3 = cc[t6]   + tr12 * ti2 + tr11 * ti3;
            cr5 =            ti11 * tr5 + ti12 * tr4;
            ci5 =            ti11 * ti5 + ti12 * ti4;
            cr4 =            ti12 * tr5 - ti11 * tr4;
            ci4 =            ti12 * ti5 - ti11 * ti4;
            dr3 = cr3 - ci4;
            dr4 = cr3 + ci4;
            di3 = ci3 + cr4;
            di4 = ci3 - cr4;
            dr5 = cr2 + ci5;
            dr2 = cr2 - ci5;
            di5 = ci2 - cr5;
            di2 = ci2 + cr5;
            ch[t13-1] = wa1[i-1] * dr2 - wa1[i] * di2;
            ch[t13]   = wa1[i-1] * di2 + wa1[i] * dr2;
            ch[t14-1] = wa2[i-1] * dr3 - wa2[i] * di3;
            ch[t14]   = wa2[i-1] * di3 + wa2[i] * dr3;
            ch[t15-1] = wa3[i-1] * dr4 - wa3[i] * di4;
            ch[t15]   = wa3[i-1] * di4 + wa3[i] * dr4;
            ch[t16-1] = wa4[i-1] * dr5 - wa4[i] * di5;
            ch[t16]   = wa4[i-1] * di5 + wa4[i] * dr5;
        }
        t1 += ido;
    }
}

static void radbg(
    int ido, int ip, int l1, int idl1,
    REAL *RESTRICT cc, REAL *RESTRICT ch,
    REAL *RESTRICT wa)
{
    static const REAL tpi = 6.2831853071795864769;
    //static const REAL tpi = 6.283185307179586476925286766559005768;   // long double
    int idij, ipph, i, j, k, l, ik, is;
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12;
    REAL dc2, ai1, ai2, ar1, ar2, ds2;
    int nbd;
    REAL dcp, arg, dsp, ar1h, ar2h;
    int ipp2;

    t10 = ip * ido;
    t0  = l1 * ido;
    arg = tpi / (REAL)ip;
    dcp = cos(arg);
    dsp = sin(arg);
    nbd = (ido-1) >> 1;
    ipp2 = ip;
    ipph = (ip+1) >> 1;
    
    if (ido >= l1) {

        t1 = 0;
        t2 = 0;
        for (k=0; k<l1; k++) {
            t3 = t1;
            t4 = t2;
            for (i=0; i<ido; i++) {
                ch[t3] = cc[t4];
                t3++;
                t4++;
            }
            t1 += ido;
            t2 += t10;
        }

    } else {

        t1 = 0;
        for (i=0; i<ido; i++) {
            t2 = t1;
            t3 = t1;
            for (k=0; k<l1; k++) {
                ch[t2] = cc[t3];
                t2 += ido;
                t3 += t10;
            }
            t1++;
        }

    }

    t1 = 0;
    t2 = ipp2 * t0;
    t7 = (t5 = ido<<1);
    for (j=1; j<ipph; j++) {
        t1 += t0;
        t2 -= t0;
        t3 = t1;
        t4 = t2;
        t6 = t5;
        for (k=0; k<l1; k++) {
            ch[t3] = cc[t6-1] + cc[t6-1];
            ch[t4] = cc[t6]   + cc[t6];
            t3 += ido;
            t4 += ido;
            t6 += t10;
        }
        t5 += t7;
    }

    if (ido != 1) {

        if (nbd >= l1) {

            t1 = 0;
            t2 = ipp2 * t0;
            t7 = 0;
            for (j=1; j<ipph; j++) {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;

                t7 += (ido<<1);
                t8 = t7;
                for (k=0; k<l1; k++) {
                    t5  = t3;
                    t6  = t4;
                    t9  = t8;
                    t11 = t8;
                    for (i=2; i<ido; i+=2) {
                        t5  += 2;
                        t6  += 2;
                        t9  += 2;
                        t11 -= 2;
                        ch[t5-1] = cc[t9-1] + cc[t11-1];
                        ch[t6-1] = cc[t9-1] - cc[t11-1];
                        ch[t5]   = cc[t9]   - cc[t11];
                        ch[t6]   = cc[t9]   + cc[t11];
                    }
                    t3 += ido;
                    t4 += ido;
                    t8 += t10;
                }
            }

        } else {

            t1 = 0;
            t2 = ipp2 * t0;
            t7 = 0;
            for (j=1; j<ipph; j++) {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                t7 += (ido<<1);
                t8 = t7;
                t9 = t7;
                for (i=2; i<ido; i+=2) {
                    t3 += 2;
                    t4 += 2;
                    t8 += 2;
                    t9 -= 2;
                    t5  = t3;
                    t6  = t4;
                    t11 = t8;
                    t12 = t9;
                    for (k=0; k<l1; k++) {
                        ch[t5-1] = cc[t11-1] + cc[t12-1];
                        ch[t6-1] = cc[t11-1] - cc[t12-1];
                        ch[t5]   = cc[t11]   - cc[t12];
                        ch[t6]   = cc[t11]   + cc[t12];
                        t5  += ido;
                        t6  += ido;
                        t11 += t10;
                        t12 += t10;
                    }
                }
            }

        }

    }

    ar1 = 1.0;
    ai1 = 0.0;
    t1 = 0;
    t9 = (t2 = ipp2 * idl1);
    t3 = (ip-1) * idl1;
    for (l=1; l<ipph; l++) {
        t1 += idl1;
        t2 -= idl1;

        ar1h = dcp * ar1 - dsp * ai1;
        ai1  = dcp * ai1 + dsp * ar1;
        ar1  = ar1h;
        t4 = t1;
        t5 = t2;
        t6 = 0;
        t7 = idl1;
        t8 = t3;
        for (ik=0; ik<idl1; ik++) {
            cc[t4++] = ch[t6++] + ar1 * ch[t7++];
            cc[t5++] = ai1 * ch[t8++];
        }
        dc2 = ar1;
        ds2 = ai1;
        ar2 = ar1;
        ai2 = ai1;

        t6 = idl1;
        t7 = t9 - idl1;
        for (j=2; j<ipph; j++) {
            t6 += idl1;
            t7 -= idl1;
            ar2h = dc2 * ar2 - ds2 * ai2;
            ai2  = dc2 * ai2 + ds2 * ar2;
            ar2  = ar2h;
            t4  = t1;
            t5  = t2;
            t11 = t6;
            t12 = t7;
            for (ik=0; ik<idl1; ik++) {
                cc[t4++] += ar2 * ch[t11++];
                cc[t5++] += ai2 * ch[t12++];
            }
        }
    }

    t1 = 0;
    for (j=1; j<ipph; j++) {
        t1 += idl1;
        t2 = t1;
        for (ik=0; ik<idl1; ik++) {
            ch[ik] += ch[t2++];
        }
    }

    t1 = 0;
    t2 = ipp2 * t0;
    for (j=1; j<ipph; j++) {
        t1 += t0;
        t2 -= t0;
        t3 = t1;
        t4 = t2;
        for (k=0; k<l1; k++) {
            ch[t3] = cc[t3] - cc[t4];
            ch[t4] = cc[t3] + cc[t4];
            t3 += ido;
            t4 += ido;
        }
    }

    if (ido != 1) {

        if (nbd >= l1) {

            t1 = 0;
            t2 = ipp2 * t0;
            for (j=1; j<ipph; j++) {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                for (k=0; k<l1; k++) {
                    t5 = t3;
                    t6 = t4;
                    for (i=2; i<ido; i+=2) {
                        t5 += 2;
                        t6 += 2;
                        ch[t5-1] = cc[t5-1] - cc[t6];
                        ch[t6-1] = cc[t5-1] + cc[t6];
                        ch[t5]   = cc[t5]   + cc[t6-1];
                        ch[t6]   = cc[t5]   - cc[t6-1];
                    }
                    t3 += ido;
                    t4 += ido;
                }
            }

        } else {

            t1 = 0;
            t2 = ipp2 * t0;
            for (j=1; j<ipph; j++) {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                for (i=2; i<ido; i+=2) {
                    t3 += 2;
                    t4 += 2;
                    t5 = t3;
                    t6 = t4;
                    for (k=0; k<l1; k++) {
                        ch[t5-1] = cc[t5-1] - cc[t6];
                        ch[t6-1] = cc[t5-1] + cc[t6];
                        ch[t5]   = cc[t5]   + cc[t6-1];
                        ch[t6]   = cc[t5]   - cc[t6-1];
                        t5 += ido;
                        t6 += ido;
                    }
                }
            }

        }

    }

    if (ido == 1) {
        return;
    }

    for (ik=0; ik<idl1; ik++) {
        cc[ik] = ch[ik];
    }

    t1 = 0;
    for (j=1; j<ip; j++) {
        t1 += t0;
        t2 = t1;
        for (k=0; k<l1; k++) {
            cc[t2] = ch[t2];
            t2 += ido;
        }
    }

    if (nbd <= l1) {

        is= -ido - 1;
        t1 = 0;
        for (j=1; j<ip; j++) {
            is += ido;
            t1 += t0;
            idij = is;
            t2 = t1;
            for (i=2; i<ido; i+=2) {
                t2 += 2;
                idij += 2;
                t3 = t2;
                for (k=0; k<l1; k++) {
                    cc[t3-1] = wa[idij] * ch[t3-1] - wa[idij+1] * ch[t3];
                    cc[t3]   = wa[idij] * ch[t3]   + wa[idij+1] * ch[t3-1];
                    t3 += ido;
                }
            }
        }

    } else {

        is= -ido - 1;
        t1 = 0;
        for (j=1; j<ip; j++) {
            is += ido;
            t1 += t0;
            t2 = t1;
            for (k=0; k<l1; k++) {
                idij = is;
                t3 = t2;
                for (i=2; i<ido; i+=2) {
                    idij += 2;
                    t3 += 2;
                    cc[t3-1] = wa[idij] * ch[t3-1] - wa[idij+1] * ch[t3];
                    cc[t3]   = wa[idij] * ch[t3]   + wa[idij+1] * ch[t3-1];
                }
                t2 += ido;
            }
        }

    }
}

static INLINE void rftb1(
    int n, REAL *RESTRICT c, REAL *RESTRICT ch, REAL *RESTRICT wa, int *RESTRICT ifac)
{
    int i, k1, l1, l2;
    int na;
    int nf, ip, iw, ix2, ix3, ix4, ido, idl1;

    nf = ifac[1];
    na = 0;
    l1 = 1;
    iw = 0;

    for (k1=0; k1<nf; k1++) {
        REAL *RESTRICT ca, *RESTRICT cb;

        ip = ifac[k1+2];
        l2 = ip * l1;
        ido = n / l2;
        idl1 = ido * l1;
    
        if (na != 0) {
            ca = ch;
            cb = c;
        } else {
            ca = c;
            cb = ch;
        }

        switch (ip) {

        case 4:
            ix2 = iw  + ido;
            ix3 = ix2 + ido;
            radb4(ido, l1, ca, cb, wa+iw, wa+ix2, wa+ix3);
            na = 1 - na;
            break;

        case 2:
            radb2(ido, l1, ca, cb, wa+iw);
            na = 1 - na;
            break;

        case 3:
            ix2 = iw+ido;
            radb3(ido, l1, ca, cb, wa+iw, wa+ix2);
            na = 1 - na;
            break;

        case 5:
            ix2 = iw  + ido;
            ix3 = ix2 + ido;
            ix4 = ix3 + ido;
            radb5(ido, l1, ca, cb, wa+iw, wa+ix2, wa+ix3, wa+ix4);
            na = 1 - na;
            break;

        default:
            radbg(ido, ip, l1, idl1, ca, cb, wa+iw);
            if (ido == 1) {
                na = 1 - na;
            }

        }

        l1 = l2;
        iw += (ip-1) * ido;
    }

    if (na == 0) {
        return;
    }

    for (i=0; i<n; i++) {
        c[i] = ch[i];
    }
}

void rfftb(int n, REAL *RESTRICT r, REAL *RESTRICT wsave, int *RESTRICT ifac)
{
    if (n == 1) {
        return;
    }
    rftb1(n, r, wsave+n, wsave, ifac);
}

static INLINE void csqb1(
    int n, REAL *RESTRICT x, REAL *RESTRICT w, REAL *RESTRICT xh, int *RESTRICT ifac)
{
    int modn, i, k, kc;
    int ns2;
    REAL xim1;

    ns2 = (n+1) >> 1;

    for (i=2; i<n; i+=2) {
        xim1   = x[i-1] + x[i];
        x[i]  -= x[i-1];
        x[i-1] = xim1;
    }

    x[0] += x[0];
    modn = n & 1;
    if (modn == 0) {
        x[n-1] += x[n-1];
    }

    rfftb(n, x, w+n, ifac);

    kc = n;
    for (k=1; k<ns2; k++) {
        kc--;
        xh[k]  = w[k] * x[kc] + w[kc] * x[k];
        xh[kc] = w[k] * x[k]  - w[kc] * x[kc];
    }

    if (modn == 0) {
        x[ns2] = w[ns2] * (x[ns2] + x[ns2]);
    }

    kc = n;
    for (k=1; k<ns2; k++) {
        kc--;
        x[k]  = xh[k] + xh[kc];
        x[kc] = xh[k] - xh[kc];
    }
    x[0] += x[0];
}

static INLINE void csqb2(
    int n, int m, REAL *RESTRICT x1, REAL *RESTRICT x2, REAL *RESTRICT w, REAL *RESTRICT xh, int *RESTRICT mfac)
{
    int modn, i, i2, k, kc;
    int ns2;
    REAL t1, t2;

    ns2 = (n+1) >> 1;
    modn = n & 1;

    for (i=2; i<n; i+=2) {
        xh[i]   = x1[i] - x1[i-1];
        xh[i-1] = x2[i] + x2[i-1];
        x2[i]  -=         x2[i-1];
        x2[i-1] = x1[i] + x1[i-1];
    }

    x1[0] += x1[0];
    x2[0] += x2[0];
    
    if (modn == 0) {
        x1[n-1] += x1[n-1];
        x1[ns2]  = x1[n-1];
    }
    for (i=1, i2=2; i<ns2; i++, i2+=2) {
        x1[i]   = x2[i2-1] - x2[i2];
        x1[n-i] = x2[i2-1] + x2[i2];
    }
    
    if (modn == 0) {
        x2[n-1] += x2[n-1];
        x2[ns2]  = x2[n-1];
    }
    for (i=1, i2=2; i<ns2; i++, i2+=2) {
        x2[i]   = xh[i2-1] + xh[i2];
        x2[n-i] = xh[i2-1] - xh[i2];
    }

    bluestein(n, m, x2, x1, xh, mfac);

    x1[0] += x1[0];
    x2[0] += x2[0];

    kc = n;
    for (k=1; k<ns2; k++) {
        kc--;
        
        t1 = w[k] * x1[kc] + w[kc] * x1[k];
        t2 = w[k] * x1[k]  - w[kc] * x1[kc];
        x1[k]  = t1 + t2;
        x1[kc] = t1 - t2;

        t1 = w[k] * x2[kc] + w[kc] * x2[k];
        t2 = w[k] * x2[k]  - w[kc] * x2[kc];
        x2[k]  = t1 + t2;
        x2[kc] = t1 - t2;
    }
    if (modn == 0) {
        x1[k] = w[k] * (x1[k] + x1[k]);
        x2[k] = w[k] * (x2[k] + x2[k]);
    }

}

void cosqb(int n, REAL *RESTRICT x, REAL *RESTRICT wsave, int *RESTRICT ifac)
{
    static const REAL tsqrt2 = 2.8284271247461900976;
    //static const REAL tsqrt2 = 2.828427124746190097603377448419396157;    // long double
    REAL x1;

    if (n < 2) {
        x[0] *= 4.0;
        return;
    }
    if (n == 2) {
        x1   = (x[0] + x[1]) * 4.0;
        x[1] = (x[0] - x[1]) * tsqrt2;
        x[0] = x1;
        return;
    }
    
    csqb1(n, x, wsave, wsave+n*2, ifac);
}

void cosqb2(int n, REAL *RESTRICT x1, REAL *RESTRICT x2, REAL *RESTRICT wsave, int *RESTRICT ifac)
{
    static const REAL tsqrt2 = 2.8284271247461900976;
    //static const REAL tsqrt2 = 2.828427124746190097603377448419396157;    // long double
    int m;
    int *mfac;
    REAL t;
    REAL *xh;

    if (n < 2) {
        x1[0] *= 4.0;
        x2[0] *= 4.0;
        return;
    }
    if (n == 2) {
        t     = (x1[0] + x1[1]) * 4.0;
        x1[1] = (x1[0] - x1[1]) * tsqrt2;
        x1[0] = t;
        t     = (x2[0] + x2[1]) * 4.0;
        x2[1] = (x2[0] - x2[1]) * tsqrt2;
        x2[0] = t;
        return;
    }
    
    xh = wsave+n*2;
    
    mfac = ifac+ifac[1]+2;
    m    = mfac[0];

    if (m) {
        csqb2(n, m, x1, x2, wsave, xh, mfac);
    } else {
        csqb1(n, x1, wsave, xh, ifac);
        csqb1(n, x2, wsave, xh, ifac);
    }
}

//void costi(int n, REAL *RESTRICT wsave, int *RESTRICT ifac)
//*******************************************************************************
//
//  costi initializes wsave and ifac, used in cost().
//
//  Description:
//
//    The prime factorization of n together with a tabulation of the
//    trigonometric functions are computed and stored in ifac and wsave.
//
//  Parameters:
//
//    Input, int n, the length of the sequence to be transformed.  The
//    method is more efficient when n-1 is the product of small primes.
//
//    Output, REAL wsave[3*n], contains data, depending on n, and
//    required by the cost() algorithm.
//
//    Output, int ifac[].
//    ifac[0] = n, the number that was factored.
//    ifac[1] = nf, the number of factors.
//    ifac[2..1+nf], the factors.
//    ifac[2+nf] = m, the smallest power of two >= 2*n,
//                    or 0 if Bluestein's algorithm is not warranted for this n.
//    ifac[3+nf] = mf, the number of factors (2's and 4's) of m.
//    ifac[4+nf..3+nf+mf], factors of m (2's and 4's).
//    ifac[4+nf+mf] = 0.
//    Note: For a given value max_n, max_mf < 2 + 0.8 * max_nf,
//    where max_nf and max_mf are the max values of nf and mf for n<=max_n.
//
//*******************************************************************************
//{
//}

//void cost(int n, REAL *RESTRICT x, REAL *RESTRICT wsave, int *RESTRICT ifac)
//*******************************************************************************
//
//  cost computes the discrete Fourier cosine transform of an even sequence.
//
//  Description:
//
//    This routine is the unnormalized inverse of itself.  Two successive
//    calls will multiply the input sequence x by 2*(n-1).
//
//    The arrays wsave and ifac must be initialized by calling costi.
//
//    The transform is defined by:
//
//      x_out[i] = x_in[0] + (-1)^i * x_in[n-1] + sum ( 1 <= k <= n-2 )
//
//        2 * x_in[k] * cos ( k * i * pi / ( n - 1 ) )
//
//  Parameters:
//
//    Input, int n, the length of the sequence to be transformed.  The
//    method is more efficient when n-1 is the product of small primes.
//
//    Input/output, REAL x[n].
//    On input, the sequence to be transformed.
//    On output, the transformed sequence.
//
//    Input, REAL wsave[3*n].
//    The wsave array must be initialized by calling costi.  A different
//    array must be used for each different value of n.
//
//    Input, int ifac[].  The ifac array must be initialized by calling costi.
//    ifac[0] = n, the number that was factored.
//    ifac[1] = nf, the number of factors.
//    ifac[2..1+nf], the factors.
//    ifac[2+nf] = m, the smallest power of two >= 2*n,
//                    or 0 if Bluestein's algorithm is not warranted for this n.
//    ifac[3+nf] = mf, the number of factors (2's and 4's) of m.
//    ifac[4+nf..3+nf+mf], factors of m (2's and 4's).
//    ifac[4+nf+mf] = 0.
//
//*******************************************************************************
//{
//}
