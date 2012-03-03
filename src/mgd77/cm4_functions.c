/*---------------------------------------------------------------------------
 *	$Id$
 *
 *
 *  File:	cm4_functions.c
 *
 *  Functions required to compute CM4 magnetic components
 *
 *  Authors:    J. Luis translated from original Fortran code
 *		P. Wessel further massaged it into this form
 *		
 *  Version:	1.0
 *  Revised:	1-MAY-2009
 * 
 * 
 *  NOTES:	The original Fortran code written by Terry Sabaka from
 *		- Planetary Geodynamics Lab at Goddard Space Flight Center -
 *		can be found at 
 *        	denali.gsfc.nasa.gov/cm/cm4field.f
 *        	This C version is a bit more limited (it doesn't allow computing the 
 *        	source coefficients - the GMDL array in original) and was striped of 
 *        	the long help/comments section. Many of those comments make no sense 
 *        	here since we changed the subroutine interface. With regard to this point, 
 *        	a substantial difference is that all is need is one single call to the 
 *        	function (with location arrays transmitted in input), and all selected 
 *        	field sources contribution add up to the final result.
 *
 *-------------------------------------------------------------------------*/

#include "mgd77.h"
#include "cm4_functions.h"

#define I_DIM(x, y) (((x) > (y)) ? (x) - (y) : 0)

void ymdtomjd(int yearad, int month, int dayofmonth, int *mjd, int *dayofyear);
void ydtomjdx(int yearad, int dayofyear, int * mjd, int *month, int *dayofmonth, int *daysinmonth);
double intdst(int mjdl, int mjdh, int mjdy, int msec, double *dstx, int *cerr);
double intf107(int iyrl, int imol, int iyrh, int imoh, int iyr, int imon, int idom, int *idim, 
	int msec, double *f107x, int *cerr);
double getmut2(double thenmp, double phinmp, int iyear, int iday, int msec);
void sun2(int iyr, int iday, double secs, double *gst, double *slong, double *srasn, double *sdec);
void rmerge_(double *rmrg, double *rmlt);
void tsearad(int full, int ks, int kr, int ns, int ng, double f, double *t, double *e, double *g);
void tseardr(int full, int ks, int kr, int ns, int ng, double f, double *t, double *e, double *g);
void mseason(int ks, int ns, int ng, double d, double *t, double *e, double *g);
void iseason(int ks, int ns, int ng, double f, double *t, double *e, double *g);
void mpotent(int nmax, int mmax, int nd, int nz, double cphi, double sphi, double *d, double *z);
void jtbelow(int pmin, int pmax, int nmax, int mmax, double r, double rref, int nz, double *z);
void jtabove(int pmin, int pmax, int nmax, int mmax, double r, double rref, int nz, double *z);
void jtbcont(int pmin, int pmax, int nmax, int mmax, double rold, double rnew, int nz, double *z);
void mstream(int nmax, int mmax, int nd, int nz, double cphi, double sphi, double faor, double *d, double *z);
void jpoloid(int pmin, int pmax, int nmax, int mmax, double r, double rm, int nd, int nz, double *t, double *d, double *z);
void blsgen(int nc, int nd, int ni, double *b, double *c, double *dldc);
void getgmf(int nder, int ns, double *ep, double *tm, double *b, double *c, double *g, int *h, int *o, double *p);
void dbspln_(int *l, double *t, int *n, int * d__, int *k, double *x, double *b, double *w);
void getgxf(int pmin, int pmax, int nmax, int mmax, int *ng, double *e, double *g, double *t);
void bfield(int rgen, int nmxi, int nmxe, int nmni, int nmne, int mmxi, int mmxe, int mmni,
	int mmne, int grad, int ctyp, int dtyp, int ityp, int etyp, double ep, double re, 
	double rp, double rm, double tm, double clat, double elon, double h, double dst, double dstt, 
	double *rse, int *nc, int *na, double *ro, double *theta, int *atyp, int *dsti, int *bori, int *bkni, 
	double *bkpi, int *tdgi, int *dste, int *bore, int *bkne, double *bkpe, int *tdge, double *a, 
	double *b, double *c, double *p, double *r, double *t, int *u, double *w, double *dsdc, 
	double *dldc, double *dlda, int *cerr);
void prebf_(int *rgen, int *ityp, int *etyp, int *dtyp, int *grad, int *nmni, int *nmxi, int *
	nmne, int *nmxe, int *mmni, int *mmxi, int *mmne, int *mmxe, int *nmax, int *mmin, int *mmax, int *
	ns, int *nsi, int *nse, int *nc, int *nci, int *nce, int *na, int *np, int *ii, int *ie, int *
	atyp, int *dsti, int *bori, int *bkni, int *tdgi, int *dste, int *bore, int *bkne, int *tdge, int *u, int *cerr);
void fdlds_(int *rgen, int *grad, int *ctyp, double *clat, double *phi, double *h, double *re, 
	double *rp, double *rm, double *ro, int *nsi, int *nc, int *nci, int *np, int *ii, int *ie, int *
	nmni, int *nmxi, int *nmne, int *nmxe, int *nmax, int *mmni, int *mmxi, int *mmne, int *mmxe, int *
	mmin, int *mmax, double *theta, double *p, double *r, double *t, int *u, double *w, double *dldc, int *cerr);
void geocen(int ctyp, double re, double rp, double rm, double h, double clat, double *r, double *theta, double *sinthe, double *costhe);
void schmit_(int *grad, int *rgen, int *nmax, int *mmin, int *mmax, double *sinthe, double *costhe, double *p, double *r);
void srecur_(int *grad, int *nmax, int *mmin, int *mmax, int *ksm2, int *ktm2, int *npall, int *
	nad1, int *nad2, int *nad3, int *nad4, int *nad5, int *nad6, int *nad7, int *nad8, double *r);
void trigmp(int mmax, double phi, double *t);
void tdc(int grad, int nc, double clat, double theta, double *dldc, double *r);
void fdsdc_(int *rgen, int *ityp, int *etyp, int *nsi, int *nse, int *nc, int *nci, double *ta,
	double *tb, double *dst, double *dstt, int *dsti, int *bori, int *bkni, double *bkpi, int *tdgi, 
	int *dste, int *bore, int *bkne, double *bkpe, int *tdge, int *u, double *w, double *dsdc, int *cerr);
void taylor(int nc, int ns, double ta, double tb, int *tdeg, int *u, double *dsdt, double *dsdc);
void bsplyn(int nc, int ns, double *ta, double *tb, int *bord, int *bkno, double *bkpo, int *u, double *dtdb, double *dsdc, int *cerr);
void sbspln_(double *ta, double *tb, int *n, int *k, double *bkpo, double *dtdb, double *dsdc, int *cerr);
void tbspln_(double *t, int *n, int *k, double *bkpo, double *dtdb, int *cerr);
void dstorm(int nc, int ns, double *dst, double *dstt, int *dstm, int *u, double *dsdc);
void fdldc(int grad, int nc, double *dsdc, double *dldc);
void blgen(int grad, int nc, double *b, double *c, double *dldc);
void bngen_(double *b);
void tec(int grad, int k, int nc, double *theta, double *phi, double *b, double *dldc, double *r);
void tse(int grad, int k, int nc, double *rse, double *b, double *dldc, double *r);
void tms(int grad, int k, int nc, int na, int ia, double *a, double *b, double *dldc, double *dlda, double *r);
void fdldeu_(int *k, int *na, int *ia, double *seulx, double *ceulx, double *seuly, double *ceuly, 
	double *seulz, double *ceulz, double *r, double *b, double *dlda);
void tnm_(int *grad, int *k, int *nc, int *na, int *ia, double *a, double *b, double *dldc, double *dlda, double *r);
void fdldno_(int *k, int *na, int *ia, double *schix, double *cchix, double *schiy, double *cchiy, 
	double *schiz, double *cchiz, double *r, double *b, double *dlda);
void fdldsl_(int *k, int *na, int *ia, double *b, double *dlda);
void tvn_(int *grad, int *k, int *nc, int *na, int *ia, double *a, double *b, double *dldc, double *dlda, double *r);
void tbi_(int *k, int *na, int *ia, double *a, double *b, double *dlda);
void fdldbi_(int *k, int *na, int *ia, double *dlda);
void ltrans(int n, int m, double *q, double *r, double *s);
void ltranv(int rfac, int n, int m, double *r, double *v);
int nshx(int nmax, int nmin, int mmax, int mmin);
int nlpx(int nmax, int mmax, int mmin);
int i8ssum(int abeg, int alen, int *a);
void i8vset(int abeg, int alen, int s, int *a);
void i8vadd(int abeg, int bbeg, int cbeg, int vlen, int *a, int *b, int *c);
void i8vadds(int abeg, int bbeg, int vlen, int s, int *a, int *b);
void i8vcum(int abas, int abeg, int alen, int *a);
void i8vdel(int abas, int abeg, int alen, int *a);
void r8vset(int abeg, int alen, double s, double *a);
double r8sdot(int abeg, int bbeg, int vlen, double *a, double *b);
double r8ssum_(int *abeg, int *alen, double *a);
void r8slt(int abeg, int alen, double s, double *a, int *j);
void r8vsub(int abeg, int bbeg, int cbeg, int vlen, double *a, double *b, double *c);
void r8vmul(int abeg, int bbeg, int cbeg, int vlen, double *a, double *b, double *c);
void r8vscale(int abeg, int alen, double s, double *a);
void r8vscats(int qbeg, int qlen, double s, int *q, double *a);
void r8vlinkt(int abeg, int bbeg, int vlen, double s, double *a, double *b);
void r8vlinkq(int abeg, int bbeg, int cbeg, int vlen, double s, double *a, double *b, double *c);
void r8vgathp(int abeg, int ainc, int bbeg, int blen, double *a, double *b);
double d_mod(double x, double y);
double pow_di(double ap, int bp);
int i_dnnt(double x);

int MGD77_cm4field (struct GMT_CTRL *GMT, struct MGD77_CM4 *Ctrl, double *p_lon, double *p_lat, double *p_alt, double *p_date) {

	int c__1356 = 1356, c__13680 = 13680;
	int i, j, k, l, n, p, nu, mz, nz, mu, js, jy, nt, mt, iyr = 0, jyr, jf107, cerr = 0;
	int lum1, lum2, lum3, lum4, lum5, lum6, lum7, nsm1, nsm2, lcmf, idim[12], omdl;
	int lsmf, lpos, lcmg, lsmg, lcsq, lssq, lcto, lsto, lrto, idoy, n_Dst_rows, i_unused = 0;
	int *msec, *mjdy, imon, idom, jaft, jmon, jdom, jmjd, jdoy, mjdl = 0, mjdh = 0, iyrl = 0, imol = 0, iyrh = 0, imoh = 0;
	int nout = 0, nygo = 0, nmax, nmin, nobo, nopo, nomn, nomx, noff, noga, nohq, nimf, nyto, nsto, ntay, mmdl;
	int us[4355], bord[4355], bkno[4355], pbto, peto, csys, jdst[24];
	double *mut, *dstx = NULL, dstt = 0., x, y, z, h, t, dumb, bmdl[21], jmdl[12], date, dst, mut_now, alt;
	double re, xd, yd, rm, xg, ro, rp, yg, zg, zd;
	double bc[29], wb[58], trig[132], ru, rt, rse[9], doy, fyr, cego, epch;
	double rlgm[15], rrgt[9], tsmg[6], tssq[6], tsto[6], tdmg[12], tdsq[10], tdto[10];
	double rtay_dw, rtay_or, sinp, fsrf, rtay, frto, frho, thetas, rtay_dk;
	double cnmp, enmp, omgs, omgd, hion, cpol, epol, ctmp, stmp, cemp, semp, rion, fdoy, clat, elon;
	double sthe, cthe, psiz, cpsi, spsi, ctgo, stgo, sego, cdip = 0, edip = 0, ctmo, stmo, cemo, semo, taus = 0, taud = 0, cosp;
	double *hq, *ht, *pleg, *rcur;			/* was hq[53040], ht[17680], pleg[4422], rcur[9104] */
	double *bkpo, *ws, *gamf, *epmg, *esmg;		/* was bkpo[12415], ws[4355], gamf[8840], epmg[1356], esmg[1356] */
	double *f107x;		/* was [100][12] */
	double *hymg;		/* was [1356][6] */
	double *gcto_or = NULL;	/* was [13680][5][2] */ 
	double *gcto_mg = NULL;	/* was [2736][3][2][2] */ 
	double *gpsq;		/* was [13680][5][2] */
	double *gssq;		/* was [13680][5] */
	double *gpmg;		/* was [1356][5][2] */ 
	double *gsmg;		/* was [1356][5][2] */  
	double *hysq;		/* was [1356][6] */  
	double *epsq;		/* was [13680] */
	double *essq;		/* was [13680] */
	double *ecto;		/* was [16416] */
	double *hyto;		/* was [49248] */
	char line[GMT_BUFSIZ], *c_unused = NULL;

	FILE *fp;

/* =====  FORTRAN SOUVENIRS ==============================================
   =======================================================================
   Main Field potential expansion parameters
      PARAMETER (NXMF=65,NYMF=8840,NXOR=4,NXKN=19,NXPO=12415)
   =======================================================================
   Magnetospheric and coupled Induction potential expansion parameters
      PARAMETER (PBMG=0,PEMG=5,PSMG=2,NXMG=11,MXMG=6,IXMG=226)
   =======================================================================
   Sq and coupled Induction potential expansion parameters
      PARAMETER (PBSQ=0,PESQ=4,PSSQ=2,NXSQ=60,MXSQ=12,IXSQ=2736)
   =======================================================================
   Toroidal scalar or stream function expansion parameters
      PARAMETER (PBTO_MG=0,PETO_MG=0,PBTO_OR=0,PETO_OR=4)
      PARAMETER (PSTO=2,NXTO=60,MXTO=12,IXTO=2736)
      PARAMETER (NTAY_MG=1,NTAY_OR=1)
 ======================================================================= */

	bkpo = calloc((size_t)(12415), sizeof(double));
	gamf = calloc((size_t)(8840), sizeof(double));
	f107x = calloc((size_t)(1200), sizeof(double));

	if ((fp = fopen(Ctrl->CM4_M.path, "r")) == NULL) {
		fprintf (stderr, "CM4: Could not open file %s\n", Ctrl->CM4_M.path);
		return 1;
	}

	c_unused = fgets(line, GMT_BUFSIZ, fp);
	sscanf (line, "%d %d %d", &lsmf, &lpos, &lcmf);
	c_unused = fgets(line, GMT_BUFSIZ, fp);
	sscanf (line, "%d", &lum1);
	c_unused = fgets(line, GMT_BUFSIZ, fp);
	sscanf (line, "%lf %lf %lf %lf", &epch, &re, &rp, &rm);
	for (j = 0; j < lsmf; ++j)
		i_unused = fscanf (fp, "%d", &bord[j]);
	for (j = 0; j < lsmf; ++j)
		i_unused = fscanf (fp, "%d", &bkno[j]);
	for (j = 0; j < lpos; ++j)
		i_unused = fscanf (fp, "%lf", &bkpo[j]);
	for (j = 0; j < lcmf; ++j)
		i_unused = fscanf (fp, "%lf", &gamf[j]);

	i_unused = fscanf (fp, "%d %d", &lcmg, &lsmg);
	i_unused = fscanf (fp, "%d %d %d %d %d %d", &lum1, &lum2, &lum3, &lum4, &lum5, &lum6);
	i_unused = fscanf (fp, "%lf %lf %lf %lf %lf %lf %lf", &cnmp, &enmp, &omgs, &omgd, &re, &rp, &rm);
	gpmg = calloc((size_t)(2 * lsmg * lcmg), sizeof(double));
	for (k = 0; k < 2; ++k)
		for (j = 0; j < lsmg; ++j) {
			n = (j + k * 5) * 1356;
			for (i = 0; i < lcmg; ++i)
				i_unused = fscanf (fp, "%lf", &gpmg[i + n]);
		}

	gsmg = calloc((size_t)(2 * lsmg * lcmg), sizeof(double));
	for (k = 0; k < 2; ++k)
		for (j = 0; j < lsmg; ++j) {
			n = (j + k * 5) * 1356;
			for (i = 0; i < lcmg; ++i)
				i_unused = fscanf (fp, "%lf", &gsmg[i + n]);
		}

	i_unused = fscanf (fp, "%d %d", &lcsq, &lssq);
	i_unused = fscanf (fp, "%d %d %d %d %d %d", &lum1, &lum2, &lum3, &lum4, &lum5, &lum6);
	i_unused = fscanf (fp, "%lf %lf %lf %lf %lf %lf %lf %lf", &cnmp, &enmp, &omgs, &omgd, &re, &rp, &rm, &hion);
	gpsq = calloc((size_t)(2 * lssq * lcsq), sizeof(double));
	for (k = 0; k < 2; ++k)
		for (j = 0; j < lssq; ++j) {
			n = (j + k * 5) * 13680;
			for (i = 0; i < lcsq; ++i)
				i_unused = fscanf (fp, "%lf", &gpsq[i + n]);
		}

	gssq = calloc((size_t)(lssq * lcsq), sizeof(double));
	for (j = 0; j < lssq; ++j) {
		n = j * 13680;
		for (i = 0; i < lcsq; ++i)
			i_unused = fscanf (fp, "%lf", &gssq[i + n]);
	}

	i_unused = fscanf (fp, "%d %d %d", &lcto, &lsto, &lrto);
	i_unused = fscanf (fp, "%d %d %d %d %d %d %d", &lum1, &lum2, &lum3, &lum4, &lum5, &lum6, &lum7);
	i_unused = fscanf (fp, "%lf %lf %lf %lf %lf %lf %lf %lf %lf", &cnmp, &enmp, &omgs, &omgd, &re, &rp, &rm, &rtay_dw, &rtay_dk);
	if (Ctrl->CM4_DATA.pred[3]) { 	/* In other cases the next coefficients are not used, so no waist time/memory with them */
		gcto_mg = calloc((size_t)(2 * lrto * lsto * lcto), sizeof(double));
		for (l = 0; l < 2; ++l)
			for (k = 0; k < lrto; ++k)
				for (j = 0; j < lsto; ++j) {
					n = (j + (k + (l << 1)) * 3) * 2736;
					for (i = 0; i < lcto; ++i)
						i_unused = fscanf (fp, "%lf", &gcto_mg[i + n]);
				}
	}
	else			/* Jump the unused coeffs */
		for (l = 0; l < 2 * lrto * lsto * lcto; ++l)
			i_unused = fscanf (fp, "%lf", &dumb);

	i_unused = fscanf (fp, "%d %d %d", &lcto, &lsto, &lrto);
	i_unused = fscanf (fp, "%d %d %d %d %d %d %d", &lum1, &lum2, &lum3, &lum4, &lum5, &lum6, &lum7);
	i_unused = fscanf (fp, "%lf %lf %lf %lf %lf %lf %lf %lf", &cnmp, &enmp, &omgs, &omgd, &re, &rp, &rm, &rtay_or);
	if (Ctrl->CM4_DATA.pred[3] && !Ctrl->CM4_DATA.pred[4]) { 	/* In other cases the next coefficients are not used, so no waist time/memory with them */
		gcto_or = calloc((size_t)(lrto * lsto * lcto), sizeof(double));
		for (k = 0; k < lrto; ++k)
			for (j = 0; j < lsto; ++j) {
				n = (j + k * 5) * 13680;
				for (i = 0; i < lcto; ++i)
					i_unused = fscanf (fp, "%lf", &gcto_or[i + n]);
			}
	}

	fclose(fp);
	cpol = cnmp * D2R;
	epol = enmp * D2R;
	sincos(cpol, &stmp, &ctmp);
	sincos(epol, &semp, &cemp);
	rion = rm + hion;

	mut = calloc((size_t)(Ctrl->CM4_DATA.n_times), sizeof(double));
	msec = calloc((size_t)(Ctrl->CM4_DATA.n_times), sizeof(int));
	mjdy = calloc((size_t)(Ctrl->CM4_DATA.n_times), sizeof(int));
	for (n = 0; n < Ctrl->CM4_DATA.n_times; ++n) {		/* If time is not constant compute the mut array */
		iyr = (int)(p_date[n]);
		fyr = p_date[n] - (double) iyr;
		doy = fyr * (double) (366 - MIN(1, iyr % 4));
		idoy = (int) doy;
		fdoy = doy - (double) idoy;
		++idoy;
		msec[n] = i_dnnt(fdoy * 8.64e7);
		ydtomjdx(iyr, idoy, &mjdy[n], &imon, &idom, idim);
		mut[n] = getmut2(cnmp, enmp, iyr, idoy, msec[n]);
	}

	csys = 1;
	if (Ctrl->CM4_G.geodetic) csys = 0;
	if (Ctrl->CM4_D.index) {
		if (Ctrl->CM4_D.load) {
			int k;
			if ((fp = fopen(Ctrl->CM4_D.path, "r")) == NULL) {
				fprintf (stderr, "CM4: Could not open file %s\n", Ctrl->CM4_D.path);
				return 1;
			}
			jaft = 0;
			n = 0;
			n_Dst_rows = 18262;	/* Current (13-05-2009) number of lines in Dst_all.wdc file */
			dstx = calloc((size_t)(n_Dst_rows * 24), sizeof(double));
			/* One improvment would be to compute year_min/year_max and retain only the needed data in dstx */

			while (fgets (line, GMT_BUFSIZ, fp)) {
				sscanf (&line[3], "%2d %2d", &jyr, &jmon);
				sscanf (&line[8], "%2d", &jdom);
				for (i = 0; i < 24; ++i)
					sscanf (&line[20+i*4],"%4d", &jdst[i]);
				jyr += 1900;
				if (jyr < 1957) jyr += 100;
				ymdtomjd(jyr, jmon, jdom, &jmjd, &jdoy);
				if (jaft == 0) {
					jaft = 1;
					mjdl = jmjd;
				}
				if (n > n_Dst_rows) {
					n_Dst_rows += 1000;
					dstx = realloc(dstx, (size_t)(n_Dst_rows * 24) * sizeof(double));
				}
				k = (jmjd - mjdl) * 24;
				for (j = 0; j < 24; ++j)
					dstx[k + j] = (double)jdst[j];
				n++;
			}
			fclose(fp);
			mjdh = jmjd;
    		}
		if (Ctrl->CM4_DATA.n_times > 1)	/* Need to re-allocate memory for all n_times in dst array */
			Ctrl->CM4_D.dst = realloc(Ctrl->CM4_D.dst, (size_t)(Ctrl->CM4_DATA.n_times) * sizeof(double));

		/* Get only one dst first so that we can test (and abort if needed) if date is out of bounds */
		Ctrl->CM4_D.dst[0] = intdst(mjdl, mjdh, mjdy[0], msec[0], dstx, &cerr);
		if (cerr > 49) {
			free( dstx);
			if (Ctrl->CM4_DATA.n_times > 1) free( Ctrl->CM4_D.dst);
			return 1;
		}


		for (n = 1; n < Ctrl->CM4_DATA.n_times; ++n)
			Ctrl->CM4_D.dst[n] = intdst(mjdl, mjdh, mjdy[n], msec[n], dstx, &cerr);

		free( dstx);
		if (cerr > 49) return 1;
	}
	if (Ctrl->CM4_I.index) {
		if (Ctrl->CM4_I.load) {
			if ((fp = fopen(Ctrl->CM4_I.path, "r")) == NULL) {
				fprintf (stderr, "CM4: Could not open file %s\n", Ctrl->CM4_I.path);
				return 1;
			}
			jaft = 0;
			while (fgets (line, GMT_BUFSIZ, fp)) {
				if (line[9] != '-') {
					sscanf (line, "%d %d %d", &jyr, &jmon, &jf107);
					if (jaft == 0) {
						jaft = 1;
						iyrl = jyr;
						imol = jmon;
					}
					f107x[(jyr - iyrl) * 12 + jmon-1] = (double)jf107 / 10.;
				}
			}
			fclose(fp);
			iyrh = jyr;
			imoh = jmon;
		}
		/* MUST INVESTIGATE IF IT WORTH HAVING AN ARRAY OF f107 LIKE IN THE DST CASE */
		Ctrl->CM4_I.F107 = intf107(iyrl, imol, iyrh, imoh, iyr, imon, idom, idim, msec[0], f107x, &cerr);
		if (cerr > 49) return 1;
	}
	free ( msec);
	free ( mjdy);

	/* On Windows, either this or declare them as "static", otherwise ... BOOM */
	hysq = calloc((size_t)(82080), sizeof(double));
	epsq = calloc((size_t)(13680), sizeof(double));
	essq = calloc((size_t)(13680), sizeof(double));
	ecto = calloc((size_t)(16416), sizeof(double));
	hyto = calloc((size_t)(49248), sizeof(double));
	hq = calloc((size_t)(53040), sizeof(double));
	ht = calloc((size_t)(17680), sizeof(double));
	ws = calloc((size_t)(4355), sizeof(double));
	epmg = calloc((size_t)(1356), sizeof(double));
	esmg = calloc((size_t)(1356), sizeof(double));
	hymg = calloc((size_t)(8136), sizeof(double));
	pleg = calloc((size_t)(4422), sizeof(double));
	rcur = calloc((size_t)(9104), sizeof(double));

	/* LOOP over number of input points (many computations below are useless repeated - room for improvment */
	for (n = 0; n < Ctrl->CM4_DATA.n_pts; ++n) {
		r8vset(1, 21, 0., &bmdl[0]);
		if (Ctrl->CM4_L.curr) r8vset(1, 12, 0., &jmdl[0]);
		clat = (90 - p_lat[n]) * D2R;
		elon = p_lon[n] * D2R;

		/* See if we are using a constant time or an array */
		if (Ctrl->CM4_DATA.n_times > 1) {
			date = p_date[n];
			dst = Ctrl->CM4_D.dst[n];
			mut_now = mut[n];
		}
		else {
			date = p_date[0];
			dst = Ctrl->CM4_D.dst[0];
			mut_now = mut[0];
		}

		/* See if we are using a constant altitude or an array */
		alt = (Ctrl->CM4_DATA.n_altitudes > 1) ? p_alt[n] : p_alt[0];

		if (Ctrl->CM4_DATA.coef) {
			nout = 1;	nygo = 0;
			if (Ctrl->CM4_DATA.pred[1]) nygo = MAX(nygo,113);
			if (Ctrl->CM4_DATA.pred[2]) nygo = MAX(nygo,1368);
			if (Ctrl->CM4_DATA.pred[3]) nygo = MAX(nygo,1368);
		}
		if (Ctrl->CM4_DATA.pred[0]) {
			nmax = MAX(Ctrl->CM4_S.nhmf[0], Ctrl->CM4_S.nhmf[1]);
			nmin = MIN(Ctrl->CM4_S.nlmf[0], Ctrl->CM4_S.nlmf[1]);
			nobo = nshx(nmin - 1, 1, nmin - 1, 0);
			nopo = i8ssum(1, nobo, bkno) + (nobo << 1);
			bfield(1, nmax, 0, nmin, 1, nmax, 0, 0, 0, 0, csys, 3, 2, 0, 
				epch, re, rp, rm, date, clat, elon, alt, dst, dstt, rse, &nz, 
				&mz, &ro, &thetas, us, us, &bord[nobo], &bkno[nobo], &bkpo[nopo], us, us, us, us, 
				ws, us, gamf, bc, gamf, pleg, rcur, trig, us, ws, ht, hq, hq, &cerr);
			if (cerr > 49) return 1;
			nomn = nshx(Ctrl->CM4_S.nlmf[0] - 1, 1, Ctrl->CM4_S.nlmf[0] - 1, 0);
			nomx = nshx(Ctrl->CM4_S.nhmf[0], 1, Ctrl->CM4_S.nhmf[0], 0);
			noff = nomn - nobo;
			nsm1 = I_DIM(nomx, nomn);
			noga = i8ssum(1, nomn, bord) + i8ssum(1, nomn, bkno) + nomn;
			nohq = i8ssum(nobo + 1, noff, bord) + i8ssum(nobo + 1, noff, bkno) + noff;
			nimf = i8ssum(nobo + 1, nsm1, bord) + i8ssum(nobo + 1, nsm1, bkno) + nsm1;
			blsgen(nimf, nz, 3, &bmdl[0], &gamf[noga], &hq[nohq]);
			if (Ctrl->CM4_DATA.coef) {
				nopo = i8ssum(1, nomn, bkno) + (nomn << 1);
				getgmf(4, nsm1, &epch, &date, wb, &gamf[noga], &Ctrl->CM4_DATA.gmdl[nout-1], 
					&bkno[nomn], &bord[nomn], &bkpo[nopo]);
				if (cerr > 49) return 1;
			}
			nomn = nshx(Ctrl->CM4_S.nlmf[1] - 1, 1, Ctrl->CM4_S.nlmf[1] - 1, 0);
			nomx = nshx(Ctrl->CM4_S.nhmf[1], 1, Ctrl->CM4_S.nhmf[1], 0);
			noff = nomn - nobo;
			nsm2 = I_DIM(nomx, nomn);
			noga = i8ssum(1, nomn, bord) + i8ssum(1, nomn, bkno) + nomn;
			nohq = i8ssum(nobo + 1, noff, bord) + i8ssum(nobo + 1, noff, bkno) + noff;
			nimf = i8ssum(nomn + 1, nsm2, bord) + i8ssum(nomn + 1, nsm2, bkno) + nsm2;
			blsgen(nimf, nz, 3, &bmdl[3], &gamf[noga], &hq[nohq]);
			if (Ctrl->CM4_DATA.coef) {
				nygo = MAX(nygo,nsm1 * 5);
				nygo = MAX(nygo,nsm2 * 5);
				nout += nygo * MIN(1,nsm1);
				nopo = i8ssum(1, nomn, bkno) + (nomn << 1);
				getgmf(4, nsm2, &epch, &date, wb, &gamf[noga], &Ctrl->CM4_DATA.gmdl[nout-1], 
					&bkno[nomn], &bord[nomn], &bkpo[nopo]);
				if (cerr > 49) return 1;
				nout += nygo * MIN(1,nsm2);
			}
		}
		if (Ctrl->CM4_DATA.pred[1] || Ctrl->CM4_DATA.pred[2] || Ctrl->CM4_DATA.pred[3]) {
			if (!Ctrl->CM4_DATA.pred[0])
				geocen(csys, re, rp, rm, alt, clat, &ro, &thetas, &sthe, &cthe);

			psiz = thetas - clat;
			sincos(psiz, &spsi, &cpsi);
			rlgm[0] = -cpsi;	rlgm[3] = 0.;	rlgm[6] = spsi;
			rlgm[1] = 0.;		rlgm[4] = 1.;	rlgm[7] = 0.;
			rlgm[2] = -spsi;	rlgm[5] = 0.;	rlgm[8] = -cpsi;
			sincos(thetas, &stgo, &ctgo);
			sincos(elon, &sego, &cego);
			rrgt[0] = ctgo * cego;	rrgt[3] = -sego;	rrgt[6] = stgo * cego;
			rrgt[1] = ctgo * sego;	rrgt[4] = cego;		rrgt[7] = stgo * sego;
			rrgt[2] = -stgo;	rrgt[5] = 0.;		rrgt[8] = ctgo;
			rmerge_(rlgm, rrgt);
			rrgt[0] = cemp * ctmp;	rrgt[3] = semp * ctmp;	rrgt[6] = -stmp;
			rrgt[1] = -semp;	rrgt[4] = cemp;		rrgt[7] = 0.;
			rrgt[2] = cemp * stmp;	rrgt[5] = semp * stmp;	rrgt[8] = ctmp;
			rmerge_(rlgm, rrgt);
			xg = stgo * cego;
			yg = stgo * sego;
			zg = ctgo;
			xd = rrgt[0] * xg + rrgt[3] * yg + rrgt[6] * zg;
			yd = rrgt[1] * xg + rrgt[4] * yg + rrgt[7] * zg;
			zd = rrgt[2] * xg + rrgt[5] * yg + rrgt[8] * zg;
			cdip = acos(zd);
			edip = atan2(yd, xd);
			ctmo = zd;
			stmo = sin(cdip);
			sincos(edip, &semo, &cemo);
			rrgt[0] = -cemo * ctmo;	rrgt[3] = -semo * ctmo;	rrgt[6] = stmo;
			rrgt[1] = -semo;	rrgt[4] = cemo;		rrgt[7] = 0.;
			rrgt[2] = -cemo * stmo; rrgt[5] = -semo * stmo; rrgt[8] = -ctmo;
			rmerge_(rlgm, rrgt);
			taus = omgs * date;
			taud = omgd * mut_now;
		}
		if (Ctrl->CM4_DATA.pred[1]) {
			bfield(1, 11, 11, 1, 1, 6, 6, 0, 0, 0, 1, 3, 0, 0, epch, re, rp, rm,
				date, cdip, edip, alt, dst, dstt, rse, &nu, &mu, 
				&ru, &thetas, us, us, us, us, ws, us, us, us, us, ws, us, gsmg, bc, gsmg, pleg, rcur, 
				trig, us, ws, ht, hq, hq, &cerr);
			if (cerr > 49) return 1;
			js = nu / 2;
			jy = 1;
			trigmp(2, taus, tsmg);
			trigmp(5, taud, tdmg);
			for (p = 0; p <= 5; ++p) {
				cosp = tdmg[p];
				sinp = tdmg[p + 6];
				mpotent(11, 6, nu, c__1356, cosp, sinp, hq, & hymg[jy - 1]);
				mpotent(11, 6, nu, c__1356, cosp, sinp, &hq[js], &hymg[jy + 4067]);
				jy += 226;
			}
			bc[0] = bc[1] = bc[2] = 0.;
			mseason(2, 5, c__1356, dst, tsmg, epmg, gpmg);
			blsgen(c__1356, c__1356, 3, bc, epmg, &hymg[4068]);
			ltrans(1, 1, bc, rlgm, &bmdl[6]);
			bc[0] = bc[1] = bc[2] = 0.;
			mseason(2, 5, c__1356, dst, tsmg, esmg, gsmg);
			blsgen(c__1356, c__1356, 3, bc, esmg, hymg);
			ltrans(1, 1, bc, rlgm, &bmdl[9]);
			if (Ctrl->CM4_L.curr) {
				bc[0] = bc[1] = bc[2] = 0.;
				jtbelow(0, 5, 11, 6, ro, rm, c__1356, hymg);
				blsgen(c__1356, c__1356, 3, bc, esmg, hymg);
				ltrans(1, 1, bc, rlgm, &jmdl[0]);
			}
			if (Ctrl->CM4_DATA.coef) {
				getgxf(0, 5, 11, 6, &js, epmg, &Ctrl->CM4_DATA.gmdl[nout-1], tdmg);
				nout += nygo;
				getgxf(0, 5, 11, 6, &js, esmg, &Ctrl->CM4_DATA.gmdl[nout-1], tdmg);
				nout += nygo;
			}
		}
		if (Ctrl->CM4_DATA.pred[2]) {
			fsrf = Ctrl->CM4_I.F107 * .01485 + 1.;
			if (ro < rion) {
				bfield(1, 60, 60, 1, 1, 12, 12, 0, 0, 0, 1, 3, 0, 0, epch, re, rp, rm,
					date, cdip, edip, alt, dst, dstt, rse, &nu,
					&mu, &ru, &thetas, us, us, us, us, ws, us, us, us, us, ws, us, gssq, 
					bc, gssq, pleg, rcur, trig, us, ws, ht, hq, hq, &cerr);
				if (cerr > 49) return 1;
				js = nu / 2;
				jy = 1;
				trigmp(2, taus, tssq);
				trigmp(4, taud, tdsq);
				for (p = 0; p <= 4; ++p) {
					cosp = tdsq[p];
					sinp = tdsq[p + 5];
					mpotent(60, 12, nu, c__13680, cosp, sinp, hq, &hysq[jy - 1]);
					mpotent(60, 12, nu, c__13680, cosp, sinp, &hq[js], &hysq[jy + 41039]);
					jy += 2736;
				}
				bc[0] = bc[1] = bc[2] = 0.;
				iseason(2, 5, c__13680, fsrf, tssq, epsq, gpsq);
				blsgen(c__13680, c__13680, 3, bc, epsq, &hysq[41040]);
				ltrans(1, 1, bc, rlgm, &bmdl[12]);
				bc[0] = bc[1] = bc[2] = 0.;
				iseason(2, 5, c__13680, fsrf, tssq, essq, gssq);
				blsgen(c__13680, c__13680, 3, bc, essq, hysq);
				ltrans(1, 1, bc, rlgm, &bmdl[15]);
				if (Ctrl->CM4_L.curr) {
					bc[0] = bc[1] = bc[2] = 0.;
					jtabove(0, 4, 60, 12, ro, rion, c__13680, &hysq[41040]);
					blsgen(c__13680, c__13680, 3, bc, epsq, &hysq[41040]);
					ltrans(1, 1, bc, rlgm, &jmdl[3]);
					bc[0] = bc[1] = bc[2] = 0.;
					jtbelow(0, 4, 60, 12, ro, rm, c__13680, hysq);
					blsgen(c__13680, c__13680, 3, bc, essq, hysq);
					ltrans(1, 1, bc, rlgm, &jmdl[6]);
				}
				if (Ctrl->CM4_DATA.coef) {
					getgxf(0, 4, 60, 12, &js, epsq, &Ctrl->CM4_DATA.gmdl[nout-1], tdsq);
					nout += nygo;
					getgxf(0, 4, 60, 12, &js, essq, &Ctrl->CM4_DATA.gmdl[nout-1], tdsq);
					nout += nygo;
				}
			}
			else {
				bfield(1, 60, 0, 1, 1, 12, 0, 0, 0, 0, 1, 3, 0, 0, epch, re, rp, rm,
					date, cdip, edip, alt, dst, dstt, 
					rse, &nu, &mu, &ru, &thetas, us, us, us, us, ws, us, us, us, us, ws, us, gssq, 
					bc, gssq, pleg, rcur, trig, us, ws, ht, hq, hq, &cerr);
				if (cerr > 49) return 1;
				jy = 1;
				trigmp(2, taus, tssq);
				trigmp(4, taud, tdsq);
				for (p = 0; p <= 4; ++p) {
					cosp = tdsq[p];
					sinp = tdsq[p + 5];
					mpotent(60, 12, nu, c__13680, cosp, sinp, hq, &hysq[jy - 1]);
					jy += 2736;
				}
				bc[0] = bc[1] = bc[2] = 0.;
				iseason(2, 5, c__13680, fsrf, tssq, epsq, &gpsq[68400]);
				blsgen(c__13680, c__13680, 3, bc, epsq, hysq);
				ltrans(1, 1, bc, rlgm, &bmdl[12]);
				bc[0] = bc[1] = bc[2] = 0.;
				iseason(2, 5, c__13680, fsrf, tssq, essq, gssq);
				blsgen(c__13680, c__13680, 3, bc, essq, hysq);
				ltrans(1, 1, bc, rlgm, &bmdl[15]);
				if (Ctrl->CM4_L.curr) {
					bc[0] = bc[1] = bc[2] = 0.;
					jtbelow(0, 4, 60, 12, ro, rion, c__13680, hysq);
					blsgen(c__13680, c__13680, 3, bc, epsq, hysq);
					ltrans(1, 1, bc, rlgm, &jmdl[3]);
					bc[0] = bc[1] = bc[2] = 0.;
					jtbcont(0, 4, 60, 12, rion, rm, c__13680, hysq);
					blsgen(c__13680, c__13680, 3, bc, essq, hysq);
					ltrans(1, 1, bc, rlgm, &jmdl[6]);
				}
				if (Ctrl->CM4_DATA.coef) {
					getgxf(0, 4, 60, 12, &nu, epsq, &Ctrl->CM4_DATA.gmdl[nout-1], tdsq);
					nout += nygo;
					getgxf(0, 4, 60, 12, &nu, essq, &Ctrl->CM4_DATA.gmdl[nout-1], tdsq);
					nout += nygo;
				}
			}
		}
		if (Ctrl->CM4_DATA.pred[3]) {
			if (Ctrl->CM4_DATA.pred[4]) {
				if (Ctrl->CM4_DATA.pred[5]) {
					pbto = peto = 0;
					nyto = 2736;
					nsto = 3;
					ntay = 1;
					rtay = rtay_dw;
					omdl = FALSE;
					mmdl = 1;
				} else {
					pbto = peto = 0;
					nyto = 2736;
					nsto = 3;
					ntay = 1;
					rtay = rtay_dk;
					omdl = FALSE;
					mmdl = 2;
				}
			} else {
				pbto = 0;
				peto = 4;
				nyto = 13680;
				nsto = 5;
				ntay = 1;
				rtay = rtay_or;
				omdl = TRUE;
				mmdl = 1;
			}
			bfield(1, 60, 0, 1, 1, 12, 0, 0, 0, 0, 1, 3, 0, 0, epch, re, rp, rm,
				date, cdip, edip, 0., dst, dstt, rse, &nt, &mt,
				&rt, &thetas, us, us, us, us, ws, us, us, us, us, ws, us, gcto_mg, bc, gcto_mg, 
				pleg, rcur, trig, us, ws, ht, hq, hq, &cerr);
			if (cerr > 49) return 1;
			frto = rm / ro;
			frho = (ro - rtay) / rm;
			jy = 1;
			trigmp(2, taus, tsto);
			trigmp(peto, taud, tdto);
			for (p = pbto; p <= peto; ++p) {
				cosp = tdto[p];
				sinp = tdto[p + 1 + peto];
				mstream(60, 12, nt, nyto, cosp, sinp, frto, hq, &hyto[jy - 1]);
				jy += 2736;
			}
			bc[0] = bc[1] = bc[2] = 0.;
			if (omdl)
				tsearad(omdl, 2, ntay, nsto, nyto, frho, tsto, ecto, gcto_or);
			else
				tsearad(omdl, 2, ntay, nsto, nyto, frho, tsto, ecto, &gcto_mg[(((mmdl << 1) + 1) * 3 + 1) * 2736 - 27360]);

			blsgen(nyto, nyto, 2, bc, ecto, hyto);
			ltrans(1, 1, bc, rlgm, &bmdl[18]);
			if (Ctrl->CM4_DATA.coef)
				getgxf(pbto, peto, 60, 12, &nt, ecto, &Ctrl->CM4_DATA.gmdl[nout-1], tdto);

			if (Ctrl->CM4_L.curr) {
	        		bc[0] = bc[1] = bc[2] = 0.;
				jpoloid(pbto, peto, 60, 12, ro, rm, nt, nyto, tdto, hq, hyto);
				blsgen(nyto, nyto, 1, &bc[2], ecto, &hyto[nyto * 2]);
				if (omdl)
					tseardr(omdl, 2, ntay, nsto, nyto, frho, tsto, ecto, gcto_or);
				else
					tseardr(omdl, 2, ntay, nsto, nyto, frho, tsto, ecto, &gcto_mg[(((mmdl << 1) + 1) * 3 + 1) * 2736 - 27360]);

				blsgen(nyto, nyto, 2, bc, ecto, hyto);
				ltrans(1, 1, bc, rlgm, &jmdl[9]);
			}
		}

		x = y = z = 0.0;
		if (!Ctrl->CM4_L.curr) {		/* Magnetic field */
			for (k = 0; k < Ctrl->CM4_F.n_field_sources; k++) {		/* Sum all field sources */
				x += bmdl[Ctrl->CM4_F.field_sources[k]*3]; 
				y += bmdl[Ctrl->CM4_F.field_sources[k]*3+1]; 
				z += bmdl[Ctrl->CM4_F.field_sources[k]*3+2]; 
			}
			for (j = 0; j < Ctrl->CM4_F.n_field_components; j++) {	/* Loop over vector field components */
				h = 0.;
				if (Ctrl->CM4_F.field_components[j] == 0) {
					t = sqrt(x*x + y*y + z*z); 
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_F.n_field_components+j] = t;
				}
				else if (Ctrl->CM4_F.field_components[j] == 1) {
					h = sqrt(x*x + y*y); 
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_F.n_field_components+j] = h;
				}
				else if (Ctrl->CM4_F.field_components[j] == 2)
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_F.n_field_components+j] = x; 
				else if (Ctrl->CM4_F.field_components[j] == 3)
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_F.n_field_components+j] = y; 
				else if (Ctrl->CM4_F.field_components[j] == 4)
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_F.n_field_components+j] = z; 
				else if (Ctrl->CM4_F.field_components[j] == 5)
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_F.n_field_components+j] = atan2(y,x) * R2D; 
				else if (Ctrl->CM4_F.field_components[j] == 6) {
					if (!h) h = sqrt(x*x + y*y);
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_F.n_field_components+j] = atan2(z,h) * R2D; 
				}
			}
		}
		else {				/* Current density field (J) */
			for (k = 0; k < Ctrl->CM4_L.n_curr_sources; k++) {		/* Sum all current sources */
				x += jmdl[Ctrl->CM4_L.curr_sources[k]*3]; 
				y += jmdl[Ctrl->CM4_L.curr_sources[k]*3+1]; 
				z += jmdl[Ctrl->CM4_L.curr_sources[k]*3+2]; 
			}
			for (j = 0; j < Ctrl->CM4_L.n_curr_components; j++) {	/* Loop over current components */
				if (Ctrl->CM4_L.curr_components[j] == 0) {
					t = sqrt(x*x + y*y + z*z); 
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_L.n_curr_components+j] = t;
				}
				else if (Ctrl->CM4_L.curr_components[j] == 1)
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_L.n_curr_components+j] = x; 
				else if (Ctrl->CM4_L.curr_components[j] == 2)
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_L.n_curr_components+j] = y; 
				else if (Ctrl->CM4_L.curr_components[j] == 3)
					Ctrl->CM4_DATA.out_field[n*Ctrl->CM4_L.n_curr_components+j] = z; 
			}
		}
	}

	free ( mut);
	free ( gpsq);	free ( gssq);	free ( gpmg);
	free ( gsmg);	free ( hysq);	free ( epsq);
	free ( essq);	free ( ecto);	free ( hyto);
	free ( hq);	free ( ht);	free ( bkpo);
	free ( ws);	free ( gamf);	free ( epmg);
	free ( esmg);	free ( hymg);	free ( f107x);
	free ( pleg);	free ( rcur);
	if (gcto_or) free( gcto_or);
	if (gcto_mg) free( gcto_mg);
	return 0;
}

void ymdtomjd(int yearad, int month, int dayofmonth, int *mjd, int *dayofyear) {
    static int daysuptomonth[12] = { 0,31,59,90,120,151,181,212,243,273, 304,334 };
    int remyear;

    remyear = yearad - 1900;
    if (remyear > 0) {
	--remyear;
	*mjd = remyear / 4 * 1461 + 15384;
	remyear %= 4;
	*dayofyear = daysuptomonth[month - 1] + dayofmonth;
	if (month > 2) {
	    *dayofyear += I_DIM(remyear, 2);
	}
	*mjd = *mjd + remyear * 365 + *dayofyear;
    } else {
	*dayofyear = daysuptomonth[month - 1] + dayofmonth;
	*mjd = *dayofyear + 15019;
    }
}

void ydtomjdx(int yearad, int dayofyear, int *mjd, int *month, int *dayofmonth, int *daysinmonth) {
    static int daysuptomonth[12] = { 0,31,59,90,120,151,181,212,243,273,304,334 };
    int j, leapday, leapcum, remyear;

    remyear = yearad - 1900;
    if (remyear > 0) {
	--remyear;
	*mjd = remyear / 4 * 1461 + 15384;
	remyear %= 4;
	*mjd = *mjd + remyear * 365 + dayofyear;
	leapday = I_DIM(remyear, 2);
    } else {
	*mjd = dayofyear + 15019;
	leapday = 0;
    }
    for (j = 12; j >= 1; --j) {
	leapcum = MIN(1,I_DIM(j, 2)) * leapday;
	if (daysuptomonth[j - 1] + leapcum < dayofyear) {
	    *month = j;
	    *dayofmonth = dayofyear - daysuptomonth[j - 1] - leapcum;
	    break;
	}
    }

    daysinmonth[0] = 31;
    daysinmonth[1] = leapday + 28;
    daysinmonth[2] = 31;
    daysinmonth[3] = 30;
    daysinmonth[4] = 31;
    daysinmonth[5] = 30;
    daysinmonth[6] = 31;
    daysinmonth[7] = 31;
    daysinmonth[8] = 30;
    daysinmonth[9] = 31;
    daysinmonth[10] = 30;
    daysinmonth[11] = 31;
}

double intdst(int mjdl, int mjdh, int mjdy, int msec, double *dstx, int *cerr) {
    int hbot, mjdt, jbot, mobs, htop, jtop, hour;
    double ttop, dst;

    hour = msec / 3600000;
    mjdt = mjdy + hour / 24;
    hour = hour % 24 + 1;
    mobs = msec % 3600000;
    if (mobs <= 1800000) {
	ttop = (double) (mobs + 1800000) / 3.6e6;
	jtop = mjdt;
	if (hour > 1) {
	    jbot = mjdt;
	    htop = hour;
	    hbot = hour - 1;
	} else {
	    jbot = mjdt - 1;
	    htop = 1;
	    hbot = 24;
	}
    } else {
	ttop = (double) (mobs - 1800000) / 3.6e6;
	if (hour < 24) {
	    jtop = jbot = mjdt;
	    htop = hour + 1;
	    hbot = hour;
	} else {
	    jtop = mjdt + 1;
	    jbot = mjdt;
	    htop = 1;
	    hbot = 24;
	}
    }
	if (jbot < mjdl || jtop > mjdh) {
		*cerr = 50;
		fprintf (stderr, "INTDST -- Error: T (%d; %d) LIES OUTSIDE OF DST TABLE TIME SPAN [%d; %d] -- ABORT\n", jbot, jtop, mjdl, mjdh);
		dst = -1e12;
	}
	else
		dst = ttop * dstx[(jtop - mjdl)*24 + (htop-1)] + (1. - ttop) * dstx[(jbot - mjdl)*24 + (hbot-1)];

	return (dst);
}

double intf107(int iyrl, int imol, int iyrh, int imoh, int iyr, int imon, int idom, int *idim, int msec, double *f107x, int *cerr) {

    int mbot, ybot, mtop, ytop;
    double tadd, thlf, tobs, tint, ttop, f107;

    /* Parameter adjustments */
    --idim;

    /* Function Body */
    thlf = (double) idim[imon] * .5;
    tobs = (double) (idom - 1) + (double) (msec) / 8.64e7;
    if (tobs <= thlf) {
	if (imon > 1) {
	    tadd = (double) idim[imon - 1] * .5;
	    tobs += tadd;
	    tint = thlf + tadd;
	    ttop = tobs / tint;
	    ytop = iyr;
	    ybot = iyr;
	    mtop = imon;
	    mbot = imon - 1;
	} else {
	    tobs += 15.5;
	    tint = thlf + 15.5;
	    ttop = tobs / tint;
	    ytop = iyr;
	    ybot = iyr - 1;
	    mtop = 1;
	    mbot = 12;
	}
    } else {
	if (imon < 12) {
	    tadd = (double) idim[imon + 1] * .5;
	    tobs -= thlf;
	    tint = thlf + tadd;
	    ttop = tobs / tint;
	    ytop = iyr;
	    ybot = iyr;
	    mtop = imon + 1;
	    mbot = imon;
	} else {
	    tobs += -15.5;
	    tint = thlf + 15.5;
	    ttop = tobs / tint;
	    ytop = iyr + 1;
	    ybot = iyr;
	    mtop = 1;
	    mbot = 12;
	}
    }
	if (ybot < iyrl || ytop > iyrh || (ybot == iyrl && mbot < imol) || (ytop == iyrh && mtop > imoh)) {
		fprintf (stderr, "SUBROUTINE INTF107 -- ERROR CODE 50 -- T LIES OUTSIDE OF F10.7 TABLE TIME SPAN -- ABORT\n");
		f107 = -1.;
		*cerr = 50;
	}
	else
		f107 = ttop * f107x[(ytop - iyrl)*12 + mtop-1] + (1. - ttop) * f107x[(ybot - iyrl)*12 + mbot-1];

	return (f107);
}

double getmut2(double thenmp, double phinmp, int iyear, int iday, int msec) {
	double gst, cth0, the0, phi0, sth0, eadj, sdec, cphd, sphd, cths, thes, phis, eopp, gmts, sths, slong, srasn;

	the0 = thenmp * D2R;
	phi0 = phinmp * D2R;
	sincos(the0, &sth0, &cth0);
	gmts = (double) (msec) * 1e-3;
	sun2(iyear, iday, gmts, &gst, &slong, &srasn, &sdec);
	thes = (90. - sdec) * D2R;
	sincos(thes, &sths, &cths);
	phis = (srasn - gst) * D2R;
	sincos(phis - phi0, &sphd, &cphd);
	eopp = sths * sphd;
	eadj = cth0 * sths * cphd - sth0 * cths;
	return (12. - atan2(eopp, eadj) * R2D / 15.);
}

void sun2(int iyr, int iday, double secs, double *gst, double *slong, double *srasn, double *sdec) {
    double d__1, g, t, dj, vl, slp, fday, sined, obliq, cosined;

    if (iyr < 1901 || iyr > 2099) {
	*gst = 0.;
	*slong = 0.;
	*srasn = 0.;
	*sdec = 0.;
    } else {
	fday = secs / 86400.;
	dj = (double) (iyr - 1900) * 365. + (double) ((iyr - 1901) / 4) + (double) (iday) + fday - .5;
	t = dj / 36525.;
	d__1 = dj * .9856473354 + 279.696678;
	vl = d_mod(d__1, 360);
	d__1 = dj * .9856473354 + 279.690983 + fday * 360. + 180.;
	*gst = d_mod(d__1, 360);
	d__1 = dj * .985600267 + 358.475845;
	g = d_mod(d__1, 360) * D2R;
	*slong = vl + (1.91946 - t * .004789) * sin(g) + sin(g * 2.) * .020094;
	obliq = (23.45229 - t * .0130125) * D2R;
	slp = (*slong - .005686) * D2R;
	sined = sin(obliq) * sin(slp);
	cosined = sqrt(1. - sined * sined);
	*sdec = R2D * atan(sined / cosined);
	*srasn = 180. - R2D * atan2(1. / tan(obliq) * sined / cosined, -cos(slp) / cosined);
    }
}

void rmerge_(double *rmrg, double *rmlt) {
    double r1, r2, r3;

    r1 = rmrg[0];
    r2 = rmrg[1];
    r3 = rmrg[2];
    rmrg[0] = r1 * rmlt[0] + r2 * rmlt[3] + r3 * rmlt[6];
    rmrg[1] = r1 * rmlt[1] + r2 * rmlt[4] + r3 * rmlt[7];
    rmrg[2] = r1 * rmlt[2] + r2 * rmlt[5] + r3 * rmlt[8];
    r1 = rmrg[3];
    r2 = rmrg[4];
    r3 = rmrg[5];
    rmrg[3] = r1 * rmlt[0] + r2 * rmlt[3] + r3 * rmlt[6];
    rmrg[4] = r1 * rmlt[1] + r2 * rmlt[4] + r3 * rmlt[7];
    rmrg[5] = r1 * rmlt[2] + r2 * rmlt[5] + r3 * rmlt[8];
    r1 = rmrg[6];
    r2 = rmrg[7];
    r3 = rmrg[8];
    rmrg[6] = r1 * rmlt[0] + r2 * rmlt[3] + r3 * rmlt[6];
    rmrg[7] = r1 * rmlt[1] + r2 * rmlt[4] + r3 * rmlt[7];
    rmrg[8] = r1 * rmlt[2] + r2 * rmlt[5] + r3 * rmlt[8];
}

void tsearad(int full, int ks, int kr, int ns, int ng, double f, double *t, double *e, double *g) {
    int i, j, k;
    double s, z;

    /* Parameter adjustments */
    g -= (1 + ng * (1 + ns));
    --t;

    /* Function Body */
    r8vset(1, ng, 0., &e[0]);
    j = 1;
    s = 1.;
    r8vlinkt(1, 1, ng, s, &g[(j + ns) * ng + 1], &e[0]);
    for (i = 1; i <= ks; ++i) {
	++j;
	s = t[i + 1];
	r8vlinkt(1, 1, ng, s, &g[(j + ns) * ng + 1], &e[0]);
	if (full) {
	    ++j;
	    s = t[i + ks + 2];
	    r8vlinkt(1, 1, ng, s, &g[(j + ns) * ng + 1], &e[0]);
	}
    }
    z = 1.;
    for (k = 1; k <= kr; ++k) {
	j = 1;
	z = z * f / (double) k;
	r8vlinkt(1, 1, ng, z, &g[(j + (k + 1) * ns) * ng + 1], &e[0]);
	for (i = 1; i <= ks; ++i) {
	    ++j;
	    s = t[i + 1] * z;
	    r8vlinkt(1, 1, ng, s, &g[(j + (k + 1) * ns) * ng + 1], &e[0]);
	    if (full) {
		++j;
		s = t[i + ks + 2] * z;
		r8vlinkt(1, 1, ng, s, &g[(j + (k + 1) * ns) * ng + 1], &e[0]);
	    }
	}
    }
}

void tseardr(int full, int ks, int kr, int ns, int ng, double f, double *t, double *e, double *g) {
    int i, j, k;
    double s, z;

    /* Parameter adjustments */
    g -= (1 + ng * (1 + ns));
    --t;

    /* Function Body */
    r8vset(1, ng, 0., &e[0]);
    z = 1.;
    for (k = 1; k <= kr; ++k) {
	j = 1;
	r8vlinkt(1, 1, ng, z, &g[(j + (k + 1) * ns) * ng + 1], &e[0]);
	for (i = 1; i <= ks; ++i) {
	    ++j;
	    s = t[i + 1] * z;
	    r8vlinkt(1, 1, ng, s, &g[(j + (k + 1) * ns) * ng + 1], &e[0]);
	    if (full) {
		++j;
		s = t[i + ks + 2] * z;
		r8vlinkt(1, 1, ng, s, &g[(j + (k + 1) * ns) * ng + 1], &e[0]);
	    }
	}
	z = z * f / (double) k;
    }
}

void mseason(int ks, int ns, int ng, double d, double *t, double *e, double *g) {
    int i, j;
    double s;

    /* Parameter adjustments */
    g -= (1 + ng * (1 + ns));

    /* Function Body */
    r8vset(1, ng, 0., &e[0]);
    j = 1;
    s = 1.;
    r8vlinkt(1, 1, ng, s, &g[(j + ns) * ng + 1], &e[0]);
    r8vlinkt(1, 1, ng, d, &g[(j + (ns << 1)) * ng + 1], &e[0]);
    for (i = 1; i <= ks; ++i) {
	++j;
	s = t[i];
	r8vlinkt(1, 1, ng, s, &g[(j + ns) * ng + 1], &e[0]);
	s *= d;
	r8vlinkt(1, 1, ng, s, &g[(j + (ns << 1)) * ng + 1], &e[0]);
	++j;
	s = t[i + ks + 1];
	r8vlinkt(1, 1, ng, s, &g[(j + ns) * ng + 1], &e[0]);
	s *= d;
	r8vlinkt(1, 1, ng, s, &g[(j + (ns << 1)) * ng + 1], &e[0]);
    }
}

void iseason(int ks, int ns, int ng, double f, double *t, double *e, double *g) {
	int i, j;
	double s;

	r8vset(1, ng, 0., &e[0]);
	j = 0;
	r8vlinkt(1, 1, ng, f, &g[j * ng], &e[0]);
	for (i = 1; i <= ks; ++i) {
		++j;
		s = f * t[i];
		r8vlinkt(1, 1, ng, s, &g[j * ng], &e[0]);
		++j;
		s = f * t[i + ks + 1];
		r8vlinkt(1, 1, ng, s, &g[j * ng], &e[0]);
	}
}

void mpotent(int nmax, int mmax, int nd, int nz, double cphi, double sphi, double *d, double *z) {
    int m, n, id, iz, nd2;

    /* Parameter adjustments */
    z -= (1 + nz);
    --d;

    /* Function Body */
    nd2 = nd + nd;
    id = iz = 0;
    for (n = 1; n <= nmax; ++n) {
	++id;
	++iz;
	z[iz + nz] = d[id] * cphi;
	z[iz + (nz << 1)] = d[nd + id] * cphi;
	z[iz + nz * 3] = d[nd2 + id] * cphi;
	++iz;
	z[iz + nz] = d[id] * sphi;
	z[iz + (nz << 1)] = d[nd + id] * sphi;
	z[iz + nz * 3] = d[nd2 + id] * sphi;
	for (m = 1; m <= MIN(n,mmax); ++m) {
	    id += 2;
	    ++iz;
	    z[iz + nz] = d[id - 1] * cphi + d[id] * sphi;
	    z[iz + (nz << 1)] = d[nd + id - 1] * cphi + d[nd + id] * sphi;
	    z[iz + nz * 3] = d[nd2 + id - 1] * cphi + d[nd2 + id] * sphi;
	    ++iz;
	    z[iz + nz] = d[id] * cphi - d[id - 1] * sphi;
	    z[iz + (nz << 1)] = d[nd + id] * cphi - d[nd + id - 1] * sphi;
	    z[iz + nz * 3] = d[nd2 + id] * cphi - d[nd2 + id - 1] * sphi;
	    ++iz;
	    z[iz + nz] = d[id - 1] * cphi - d[id] * sphi;
	    z[iz + (nz << 1)] = d[nd + id - 1] * cphi - d[nd + id] * sphi;
	    z[iz + nz * 3] = d[nd2 + id - 1] * cphi - d[nd2 + id] * sphi;
	    ++iz;
	    z[iz + nz] = d[id] * cphi + d[id - 1] * sphi;
	    z[iz + (nz << 1)] = d[nd + id] * cphi + d[nd + id - 1] * sphi;
	    z[iz + nz * 3] = d[nd2 + id] * cphi + d[nd2 + id - 1] * sphi;
	}
    }
}

void jtabove(int pmin, int pmax, int nmax, int mmax, double r, double rref, int nz, double *z) {
    /* Initialized data */

    static double fgeo = 7.95774715459478e-4;

    /* Local variables */
    int m, n, p, iz;
    double ffac, fcur, fpsi, frmu, frpw, ztemp;

    /* Parameter adjustments */
    z -= (1 + nz);

    /* Function Body */
    frmu = rref / r;
    iz = 0;
    for (p = pmin; p <= pmax; ++p) {
	frpw = fgeo;
	for (n = 1; n <= nmax; ++n) {
	    ffac = frpw * (double) (n + n + 1);
	    fcur = ffac / (double) (n + 1);
	    fpsi = -(rref) * ffac / (double) (n * (n + 1));
	    ++iz;
	    ztemp = z[iz + nz];
	    z[iz + nz] = -fcur * z[iz + (nz << 1)];
	    z[iz + (nz << 1)] = fcur * ztemp;
	    z[iz + nz * 3] = fpsi * z[iz + nz * 3];
	    ++iz;
	    ztemp = z[iz + nz];
	    z[iz + nz] = -fcur * z[iz + (nz << 1)];
	    z[iz + (nz << 1)] = fcur * ztemp;
	    z[iz + nz * 3] = fpsi * z[iz + nz * 3];
	    for (m = 1; m <= MIN(n,mmax); ++m) {
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = -fcur * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = fcur * ztemp;
		z[iz + nz * 3] = fpsi * z[iz + nz * 3];
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = -fcur * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = fcur * ztemp;
		z[iz + nz * 3] = fpsi * z[iz + nz * 3];
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = -fcur * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = fcur * ztemp;
		z[iz + nz * 3] = fpsi * z[iz + nz * 3];
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = -fcur * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = fcur * ztemp;
		z[iz + nz * 3] = fpsi * z[iz + nz * 3];
	    }
	    frpw *= frmu;
	}
    }
}

void jtbelow(int pmin, int pmax, int nmax, int mmax, double r, double rref, int nz, double *z) {
    /* Initialized data */

    static double fgeo = 7.95774715459478e-4;

    /* Local variables */
    int m, n, p, iz;
    double ffac, frbg, fcur, fpsi, frmu, frpw, ztemp;

    /* Parameter adjustments */
    z -= (1 + nz);

    /* Function Body */
    frmu = r / rref;
    frbg = fgeo * (frmu * frmu * frmu);
    iz = 0;
    for (p = pmin; p <= pmax; ++p) {
	frpw = frbg;
	for (n = 1; n <= nmax; ++n) {
	    ffac = frpw * (double) (n + n + 1);
	    fcur = ffac / (double) n;
	    fpsi = -(rref) * ffac / (double) (n * (n + 1));
	    ++iz;
	    ztemp = z[iz + nz];
	    z[iz + nz] = fcur * z[iz + (nz << 1)];
	    z[iz + (nz << 1)] = -fcur * ztemp;
	    z[iz + nz * 3] = fpsi * z[iz + nz * 3];
	    ++iz;
	    ztemp = z[iz + nz];
	    z[iz + nz] = fcur * z[iz + (nz << 1)];
	    z[iz + (nz << 1)] = -fcur * ztemp;
	    z[iz + nz * 3] = fpsi * z[iz + nz * 3];
	    for (m = 1; m <= MIN(n,mmax); ++m) {
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = fcur * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = -fcur * ztemp;
		z[iz + nz * 3] = fpsi * z[iz + nz * 3];
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = fcur * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = -fcur * ztemp;
		z[iz + nz * 3] = fpsi * z[iz + nz * 3];
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = fcur * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = -fcur * ztemp;
		z[iz + nz * 3] = fpsi * z[iz + nz * 3];
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = fcur * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = -fcur * ztemp;
		z[iz + nz * 3] = fpsi * z[iz + nz * 3];
	    }
	    frpw *= frmu;
	}
    }
}

void jtbcont(int pmin, int pmax, int nmax, int mmax, double rold, double rnew, int nz, double *z) {
    int m, n, p, iz, ii;
    double frbg, fcur, fpsi, frmu;

    /* Parameter adjustments */
    z -= (1 + nz);

    /* Function Body */
    frmu = rold / rnew;
    frbg = frmu * frmu;
    iz = 0;
    for (p = pmin; p <= pmax; ++p) {
	fpsi = frbg;
	for (n = 1; n <= nmax; ++n) {
	    fcur = fpsi * frmu;
	    ++iz;
		ii = iz + nz;		z[ii] = fcur * z[ii];
		ii = iz + (nz << 1);	z[ii] = fcur * z[ii];
		ii = iz + nz * 3;	z[ii] = fpsi * z[ii];
	    ++iz;
		ii = iz + nz;		z[ii] = fcur * z[ii];
		ii = iz + (nz << 1);	z[ii] = fcur * z[ii];
		ii = iz + nz * 3;	z[ii] = fpsi * z[ii];
	    for (m = 1; m <= MIN(n,mmax); ++m) {
		++iz;
			ii = iz + nz;		z[ii] = fcur * z[ii];
			ii = iz + (nz << 1);	z[ii] = fcur * z[ii];
			ii = iz + nz * 3;	z[ii] = fpsi * z[ii];
		++iz;
			ii = iz + nz;		z[ii] = fcur * z[ii];
			ii = iz + (nz << 1);	z[ii] = fcur * z[ii];
			ii = iz + nz * 3;	z[ii] = fpsi * z[ii];
		++iz;
			ii = iz + nz;		z[ii] = fcur * z[ii];
			ii = iz + (nz << 1);	z[ii] = fcur * z[ii];
			ii = iz + nz * 3;	z[ii] = fpsi * z[ii];
		++iz;
			ii = iz + nz;		z[ii] = fcur * z[ii];
			ii = iz + (nz << 1);	z[ii] = fcur * z[ii];
			ii = iz + nz * 3;	z[ii] = fpsi * z[ii];
	    }
	    fpsi *= frmu;
	}
    }
}

void mstream(int nmax, int mmax, int nd, int nz, double cphi, double sphi, double faor, double *d, double *z) {
    int m, n, id, iz;

    /* Parameter adjustments */
    z -= (1 + nz);
    --d;

    /* Function Body */
    id = iz = 0;
    for (n = 1; n <= nmax; ++n) {
	++id;
	++iz;
	z[iz + nz] = faor * d[nd + id] * cphi;
	z[iz + (nz << 1)] = -(faor) * d[id] * cphi;
	++iz;
	z[iz + nz] = faor * d[nd + id] * sphi;
	z[iz + (nz << 1)] = -(faor) * d[id] * sphi;
	for (m = 1; m <= MIN(n,mmax); ++m) {
	    id += 2;
	    ++iz;
	    z[iz + nz] = faor * (d[nd + id - 1] * cphi + d[nd + id] * sphi);
	    z[iz + (nz << 1)] = -(faor) * (d[id - 1] * cphi + d[id] * sphi);
	    ++iz;
	    z[iz + nz] = faor * (d[nd + id] * cphi - d[nd + id - 1] * sphi);
	    z[iz + (nz << 1)] = -(faor) * (d[id] * cphi - d[id - 1] * sphi);
	    ++iz;
	    z[iz + nz] = faor * (d[nd + id - 1] * cphi - d[nd + id] * sphi);
	    z[iz + (nz << 1)] = -(faor) * (d[id - 1] * cphi - d[id] * sphi);
	    ++iz;
	    z[iz + nz] = faor * (d[nd + id] * cphi + d[nd + id - 1] * sphi);
	    z[iz + (nz << 1)] = -(faor) * (d[id] * cphi + d[id - 1] * sphi);
	}
    }
}

void jpoloid(int pmin, int pmax, int nmax, int mmax, double r, double rm, int nd, int nz, double *t, double *d, double *z) {
    /* Initialized data */

    static double u0 = .0012566370614359158;

    /* Local variables */
    int m, n, p, id, iz, nd2;
    double fdeg, fhtj, frtj, cosp, sinp, ztemp;

    /* Parameter adjustments */
    z -= (1 + nz);
    --d;

    /* Function Body */
    nd2 = nd + nd;
    fhtj = 1. / rm / u0;
    frtj = rm / (r * r) / u0;
    iz = 0;
    for (p = pmin; p <= pmax; ++p) {
	id = 0;
	cosp = t[p];
	sinp = t[p + 1 + pmax];
	for (n = 1; n <= nmax; ++n) {
	    fdeg = frtj * (double) n;
	    ++id;
	    ++iz;
	    ztemp = z[iz + nz];
	    z[iz + nz] = fhtj * z[iz + (nz << 1)];
	    z[iz + (nz << 1)] = -fhtj * ztemp;
	    z[iz + nz * 3] = fdeg * d[nd2 + id] * cosp;
	    ++iz;
	    ztemp = z[iz + nz];
	    z[iz + nz] = fhtj * z[iz + (nz << 1)];
	    z[iz + (nz << 1)] = -fhtj * ztemp;
	    z[iz + nz * 3] = fdeg * d[nd2 + id] * sinp;
	    for (m = 1; m <= MIN(n,mmax); ++m) {
		id += 2;
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = fhtj * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = -fhtj * ztemp;
		z[iz + nz * 3] = fdeg * (d[nd2 + id - 1] * cosp + d[nd2 + id] * sinp);
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = fhtj * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = -fhtj * ztemp;
		z[iz + nz * 3] = fdeg * (d[nd2 + id] * cosp - d[nd2 + id - 1] * sinp);
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = fhtj * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = -fhtj * ztemp;
		z[iz + nz * 3] = fdeg * (d[nd2 + id - 1] * cosp - d[nd2 + id] * sinp);
		++iz;
		ztemp = z[iz + nz];
		z[iz + nz] = fhtj * z[iz + (nz << 1)];
		z[iz + (nz << 1)] = -fhtj * ztemp;
		z[iz + nz * 3] = fdeg * (d[nd2 + id] * cosp + d[nd2 + id - 1] * sinp);
	    }
	}
    }
}

void blsgen(int nc, int nd, int ni, double *b, double *c, double *dldc) {
    int j;

    /* Parameter adjustments */
    dldc -= (1 + nd);
    --b;

    for (j = 1; j <= ni; ++j)
	b[j] += r8sdot(1, 1, nc, &dldc[j * nd + 1], &c[0]);
}

void getgmf(int nder, int ns, double *ep, double *tm, double *b, double *c, double *g, int *h, int *o, double *p) {
    /* Initialized data */

    static int null = 0;

    /* Local variables */
    int d, i, j, k, m, n, y, ic, la, lb, jc, ig, nb, ik, ni, im, nk, jm, no, js, nw;
    double gd, gm, bsum;

    /* Parameter adjustments */
    --p;
    --o;
    --h;
    --g;
    --c;
    --b;

    /* Function Body */
    ic = 1;
    ik = 1;
    for (i = 1; i <= ns; ++i) {
	g[i] = c[ic];
	ig = i;
	for (j = 1; j <= nder; ++j) {
	    ig += ns;
	    g[ig] = 0.;
	}
	n = o[i];
	if (n > 0) {
	    k = h[i];
	    if (*tm < p[ik] || *tm > p[ik + k + 1]) {
		fprintf(stderr, "SUBROUTINE GETGMF -- ERROR CODE 56 -- T LIES OUTSIDE OF KNOT DOMAIN -- ABORT\n");
		return;
	    }
	    m = n + 1;
	    nk = k + 2;
	    nb = n + k;
	    ni = nb + 1;
	    no = ni + 1;
	    nw = no + no;
	    r8slt(1, nk, *ep, &p[ik], &y);
	    la = MIN(nk,y+1);
	    r8slt(1, nk, *tm, &p[ik], &y);
	    lb = MIN(nk,y+1);
	    r8vset(1, nw, 0., &b[1]);
	    dbspln_(&la, ep, &m, &null, &k, &p[ik], &b[la - 1], &b[nw + 1]);
	    dbspln_(&lb, tm, &m, &null, &k, &p[ik], &b[no + lb - 1], &b[nw + 1]);
	    r8vsub(1, no+1, 1, no, &b[1], &b[1], &b[1]);
	    bsum = 0.;
	    gm = 0.;
	    jc = ic + nb;
	    for (j = ni; j >= 2; --j) {
		bsum += b[j];
		im = MIN(j,nk);
		jm = MAX(j-n,1);
		gm += (p[ik + im - 1] - p[ik + jm - 1]) * bsum * c[jc];
		--jc;
	    }
	    g[i] += gm / (double) n;
	    js = ic + lb - 1;
	    ig = i;
	    for (d = 0; d <= nder - 1; ++d) {
		ig += ns;
		dbspln_(&lb, tm, &n, &d, &k, &p[ik], &b[1], &b[nw + 1]);
		gd = 0.;
		jc = js;
		for (j = 1; j <= n; ++j) {
		    gd += b[j] * c[jc];
		    ++jc;
		}
		g[ig] = gd;
	    }
	    ic += nb;
	    ik += nk;
	}
	++ic;
    }
}

void dbspln_(int *l, double *t, int *n, int * d__, int *k, double *x, double *b, double *w) {
    /* Local variables */
    int i__, j, m, q, ik, jk, iw, lenb, posb, posw, posx;
    double fn, difi, difj, delx, temp;

    /* Parameter adjustments */
    --w;
    --b;
    --x;

    /* Function Body */
    q = *n - *d__;
    if (q == 1)
	b[1] = 1.;
    else {
	m = *l;
	ik = MIN(m,*k + 2);
	jk = MAX(m-1,1);
	difi = x[ik] - *t;
	delx = x[ik] - x[jk];
	if (delx == 0.)
	    b[q] = 0.;
	else
	    b[q] = 1. / delx;

	posb = q - 1;
	for (j = 2; j <= q; ++j) {
	    jk = MAX(m-j,1);
	    temp = difi * b[posb + 1];
	    delx = x[ik] - x[jk];
	    if (delx == 0.)
		temp = 0.;
	    else if (j < *n) 
		    temp /= delx;

	    b[posb] = temp;
	    --posb;
	}
	b[q + 1] = 0.;
	++m;
	for (i__ = 2; i__ <= q; ++i__) {
	    ik = MIN(m,*k + 2);
	    difi = x[ik] - *t;
	    posb = q;
	    for (j = i__; j <= q; ++j) {
		jk = MAX(m-j,1);
		difj = *t - x[jk];
		temp = difj * b[posb] + difi * b[posb + 1];
		delx = x[ik] - x[jk];
		if (delx == 0.)
		    temp = 0.;
		else if (j < *n)
			temp /= delx;

		b[posb] = temp;
		--posb;
	    }
	    ++m;
	}
    }
    posx = *l + *n - 1;
    posw = *d__ + *n;
    for (i__ = 1; i__ <= *n; ++i__) {
	lenb = MIN(q, posw - *d__);
	r8vset(1, posw, 0., &w[1]);
	r8vgathp(1, 1, *d__ + 1, lenb, &b[1], &w[1]);
	for (j = 1; j <= *d__; ++j) {
	    ik = posx;
	    jk = posx - q - j;
	    iw = posw;
	    fn = (double) (q + j - 1);
	    for (m = j; m <= *d__; ++m) {
		if (j < *d__) {
		    delx = x[MAX(1,MIN(ik,*k + 2))] - x[MAX(jk,1)];
		    if (delx == 0.)
			w[iw] = 0.;
		    else
			w[iw] = fn * (w[iw - 1] - w[iw]) / delx;

		} else
		    w[iw] = fn * (w[iw - 1] - w[iw]);

		--ik;
		--jk;
		--iw;
	    }
	}
	--posx;
	--posw;
    }
    r8vgathp(*d__ + 1, 1, 1, *n, &w[1], &b[1]);
}

void getgxf(int pmin, int pmax, int nmax, int mmax, int *ng, double *e, double *g, double *t) {
    int m, n, p, ie, ig;
    double cosp, sinp;

    /* Function Body */
    r8vset(1, *ng, 0., &g[0]);
    ie = 0;
    for (p = pmin; p <= pmax; ++p) {
	ig = 0;
	cosp = t[p];
	sinp = t[p + 1 + pmax];
	for (n = 1; n <= nmax; ++n) {
	    g[ig++] += (e[ie] * cosp + e[ie + 1] * sinp);
	    ie += 2;
	    for (m = 1; m <= MIN(n,mmax); ++m) {
		g[ig++] += ((e[ie] + e[ie + 2]) * cosp + (e[ie + 3] - e[ie + 1]) * sinp);
		g[ig++] += ((e[ie + 3] + e[ie + 1]) * cosp + (e[ie] - e[ie + 2]) * sinp);
		ie += 4;
	    }
	}
    }
}

void bfield(int rgen, int nmxi, int nmxe, int nmni, int nmne, int mmxi, int mmxe, int mmni,
	int mmne, int grad, int ctyp, int dtyp, int ityp, int etyp, double ep, double re, 
	double rp, double rm, double tm, double clat, double elon, double h, double dst, double dstt, 
	double *rse, int *nc, int *na, double *ro, double *theta, int *atyp, int *dsti, int *bori, int *bkni, 
	double *bkpi, int *tdgi, int *dste, int *bore, int *bkne, double *bkpe, int *tdge, double *a, 
	double *b, double *c, double *p, double *r, double *t, int *u, double *w, double *dsdc, 
	double *dldc, double *dlda, int *cerr) {

    int ia, ie, ii, np, ns, nce, nci, nse, nsi, nmax, mmin, mmax;
    double phi;

    if (*cerr <= 49) {
	phi = elon;
	prebf_(&rgen, &ityp, &etyp, &dtyp, &grad, &nmni, &nmxi, &nmne, &nmxe, &mmni, &mmxi, &mmne, &mmxe, &nmax, &mmin, &mmax, &ns, &nsi, 
		&nse, nc, &nci, &nce, na, &np, &ii, &ie, &atyp[0], &dsti[0], &bori[0], &bkni[0], &tdgi[0], &dste[0], &bore[0], 
		&bkne[0], &tdge[0], &u[0], cerr);
	if (*cerr >= 50) return;
	if (*nc > 0) {
	    fdsdc_(&rgen, &ityp, &etyp, &nsi, &nse, nc, &nci, &ep, &tm, &dst, &dstt, &dsti[0], &bori[0], &bkni[0], &bkpi[0], &tdgi[0], 
		&dste[0], &bore[0], &bkne[0], &bkpe[0], &tdge[0], &u[0], &w[0], &dsdc[0], cerr);
	    if (*cerr >= 50) return;
	    fdlds_(&rgen, &grad, &ctyp, &clat, &phi, &h, &re, &rp, &rm, ro, &nsi, nc, &nci, &np, &ii, &ie, &nmni, &nmxi, &nmne, &nmxe, &nmax, 
		    &mmni, &mmxi, &mmne, &mmxe, &mmin, &mmax, theta, &p[0], &r[0], &t[0], &u[0], &w[0], &dldc[0], cerr);
	    if (*cerr >= 50) return;
	    if (rgen > 0) {
		rgen = 0;
		r8vset(1, grad * 36 + 28, 0., &b[0]);
		fdldc(grad, *nc, &dsdc[0], &dldc[0]);
		blgen(grad, *nc, &b[0], &c[0], &dldc[0]);
		bngen_(&b[0]);
	    }
	    if (dtyp == 2) {
		tec(grad, atyp[0], *nc, theta, &phi, &b[0], &dldc[0], &w[0]);
		tse(grad, atyp[1], *nc, &rse[0], &b[0], &dldc[0], &w[0]);
	    }
	}
	r8vgathp(1, 1, 15, 14, &b[0], &b[0]);
	if (grad == 1)
	    r8vgathp(29, 1, 47, 18, &b[0], &b[0]);

	ia = 0;
	if (*na > 0) {
	    r8vset(1, *na * 6, 0., &dlda[0]);
	    if (dtyp == 1)
		tbi_(&atyp[0], na, &ia, &a[0], &b[0], &dlda[0]);
	    else if (dtyp == 2) {
		tms(grad, atyp[2], *nc, *na, ia, &a[0], &b[0], &dldc[0], &dlda[0], &w[0]);
		tnm_(&grad, &atyp[3], nc, na, &ia, &a[0], &b[0], &dldc[0], &dlda[0], &w[0]);
		tvn_(&grad, &atyp[4], nc, na, &ia, &a[0], &b[0], &dldc[0], &dlda[0], &w[0]);
		tbi_(&atyp[5], na, &ia, &a[0], &b[0], &dlda[0]);
	    }
	}
    }
}

void prebf_(int *rgen, int *ityp, int *etyp, int *dtyp, int *grad, int *nmni, int *nmxi, int *
	nmne, int *nmxe, int *mmni, int *mmxi, int *mmne, int *mmxe, int *nmax, int *mmin, int *mmax, int *
	ns, int *nsi, int *nse, int *nc, int *nci, int *nce, int *na, int *np, int *ii, int *ie, int *
	atyp, int *dsti, int *bori, int *bkni, int *tdgi, int *dste, int *bore, int *bkne, int *tdge, int *u, int *cerr) {

    static int nx = 0;
    int i__1, edst, esvr, idst, isvr;

    /* Parameter adjustments */
    --u;
    --tdge;
    --bkne;
    --bore;
    --dste;
    --tdgi;
    --bkni;
    --bori;
    --dsti;
    --atyp;

    /* Function Body */
    if (*rgen == 1) {
	i__1 = MIN(MIN(*nmni,*nmxi), *nmne);
	if (MIN(i__1,*nmxe) < 0) {
		fprintf(stderr, "SUBROUTINE BFIELD -- ERROR CODE 50 -- NMNI, NMXI, NMNE, OR NMXE < 0 -- ABORT\n");
		return;
	}
	i__1 = MIN(MIN(*mmni,*mmxi), *mmne);
	if (MIN(i__1,*mmxe) < 0) {
		fprintf(stderr, "SUBROUTINE BFIELD -- ERROR CODE 51 -- MMNI, MMXI, MMNE, OR MMXE < 0 -- ABORT\n");
		return;
	}
	if (*mmni > *mmxi || *mmne > *mmxe) {
		fprintf(stderr, "SUBROUTINE BFIELD -- ERROR CODE 52 -- EITHER MMNI > MMXI OR MMNE > MMXE -- ABORT\n");
		return;
	}
	if (*mmxi > *nmxi || *mmxe > *nmxe) {
		fprintf(stderr, "SUBROUTINE BFIELD -- ERROR CODE 53 -- EITHER MMXI > NMXI OR MMXE > NMXE -- ABORT\n");
		return;
	}
	isvr = *ityp % 3;
	idst = *ityp / 3;
	esvr = *etyp % 3;
	edst = *etyp / 3;
	*nmax = MAX(*nmxi,*nmxe);
	*mmin = MIN(*mmni,*mmne);
	*mmax = MAX(*mmxi,*mmxe);
	*nsi = nshx(*nmxi, *nmni, *mmxi, *mmni);
	*nse = nshx(*nmxe, *nmne, *mmxe, *mmne);
	*ns = *nsi + *nse;
	*np = nlpx(*nmax, *mmax, *mmin);
	*ii = nlpx(*nmni - 1, *mmax, *mmin);
	*ie = nlpx(*nmne - 1, *mmax, *mmin);
	*nci = 0;
	if (*nsi > 0) {
	    i8vset(1, *nsi, 1, &u[1]);
	    if (isvr == 1)
		i8vadd(1, 1, 1, *nsi, &tdgi[1], &u[1], &u[1]);
	    else if (isvr == 2) {
		i8vadd(1, 1, 1, *nsi, &bori[1], &u[1], &u[1]);
		i8vadd(1, 1, 1, *nsi, &bkni[1], &u[1], &u[1]);
	    }
	    if (idst == 1)
		i8vadd(1, 1, 1, *nsi, &dsti[1], &u[1], &u[1]);

	    *nci = i8ssum(1, *nsi, &u[1]);
	}
	*nce = 0;
	if (*nse > 0) {
	    i__1 = *nsi + 1;
	    i8vset(i__1, *nse, 1, &u[1]);
	    if (esvr == 1)
		i8vadd(1, i__1, i__1, *nse, &tdge[1], &u[1], &u[1]);
	    else if (esvr == 2) {
		i8vadd(1, i__1, i__1, *nse, &bore[1], &u[1], &u[1]);
		i8vadd(1, i__1, i__1, *nse, &bkne[1], &u[1], &u[1]);
	    }
	    if (edst == 1)
		i8vadd(1, i__1, i__1, *nse, &dste[1], &u[1], &u[1]);

	    *nce = i8ssum(i__1, *nse, &u[1]);
	}
	*nc = *nci + *nce;
	*rgen = 7;
    }
    *rgen += nx;
    *na = 0;
    nx = 0;
    if (*dtyp == 1)
	*na += MIN(1,atyp[1]) * 3;
    else if (*dtyp == 2) {
	nx += atyp[1];
	nx += atyp[2];
	*na += MIN(1,atyp[3]) * 3;
	*na += MIN(1,atyp[4]) * 3;
	*na += MIN(1,atyp[5]) * 3;
	nx += *na;
	*na += MIN(1,atyp[6]) * 3;
    }
    nx = MIN(1,nx);
}

void fdlds_(int *rgen, int *grad, int *ctyp, double *clat, double *phi, double *h__, double *re, 
	double *rp, double *rm, double *ro, int *nsi, int *nc, int *nci, int *np, int *ii, int *ie, int *
	nmni, int *nmxi, int *nmne, int *nmxe, int *nmax, int *mmni, int *mmxi, int *mmne, int *mmxe, int *
	mmin, int *mmax, double *theta, double *p, double *r__, double *t, int *u, double *w, double *dldc, int *cerr) {
    /* Initialized data */

    static double roo = 0.;
    static double phio = 0.;
    static double thetao = 0.;
    static double clato = 0.;

    /* Local variables */
    int m, n, ic, id, ip, lend, pgen, tgen;
    double fa, fc, fd, ar, fm, ra, fp, fr, fs, fn, fnm1, fnp1, fnp2, fnfp, fprr, fdrr;
    double pbppp = 0, pbppr = 0, pbrpp = 0, pbtpp = 0, pbppt = 0, pbtpr = 0, pbrpt = 0, pbtpt = 0, pbrpr = 0, cscth2;
    double cscthe, costhe, cotthe, sinthe, fmfpst;

    /* Parameter adjustments */
    --dldc;
    --w;
    --u;
    --t;
    --r__;
    --p;

    /* Function Body */
    geocen(*ctyp, *re, *rp, *rm, *h__, *clat, ro, theta, &sinthe, &costhe);
    pgen = *rgen - 6;
    tgen = pgen;
    if (phio != *phi) {
	pgen = 1;
	++(*rgen);
	phio = *phi;
    }
    if (thetao != *theta) {
	tgen = 1;
	++(*rgen);
	thetao = *theta;
    }
    if (clato != *clat) {
	++(*rgen);
	clato = *clat;
    }
    if (roo != *ro) {
	++(*rgen);
	roo = *ro;
    }
    if (*rgen > 0) {
	if (sinthe == 0.) {
	    if (*grad == 0)
		fprintf(stderr, "SUBROUTINE BFIELD -- ERROR CODE 1 -- GEOGRAPHIC POLAR POSITION DETECTED, B-PHI INDETERMINABLE -- WARNING\n");
	    else
		fprintf(stderr, "SUBROUTINE BFIELD -- ERROR CODE 2 -- GEOGRAPHIC POLAR POSITION DETECTED, B-PHI AND\n\t\t\t-- PHI-DERIVATIVE GRADIENT COMPONENTS INDETERMINABLE -- WARNING\n");

	    *phi = 0.;
	    cscthe = 0.;
	    cscth2 = 0.;
	    cotthe = 0.;
	} else {
	    cscthe = 1. / sinthe;
	    cscth2 = cscthe * cscthe;
	    cotthe = costhe * cscthe;
	}
	if (tgen > 0)
	    schmit_(grad, rgen, nmax, mmin, mmax, &sinthe, &costhe, &p[1], &r__[1]);

	if (pgen > 0)
	    trigmp(*mmax, *phi, &t[1]);

	r8vset(1, (*grad * 18 + 6) * *nc, 0., &dldc[1]);
	ic = 0;
	id = 1;
	ip = *ii;
	ar = *rm / *ro;
	fa = pow_di(ar, *nmni + 1);
	for (n = *nmni; n <= *nmxi; ++n) {
	    fnp1 = (double) (n + 1);
	    fnp2 = (double) (n + 2);
	    fa *= ar;
	    for (m = *mmin; m <= MIN(n,*mmax); ++m) {
		++ip;
		if (m >= *mmni && m <= *mmxi) {
		    ++ic;
		    fm = (double) m;
		    fp = fa * p[ip];
		    fd = -fa * p[*np + ip];
		    fc = t[m + 1];
		    fs = t[*mmax + m + 2];
		    fnfp = fnp1 * fp;
		    fmfpst = fm * fp * cscthe;
		    lend = u[ic];
		    r8vset(id, u[ic], fd * fc, &dldc[1]);
		    r8vset(*nc + id, u[ic], fmfpst * fs, &dldc[1]);
		    r8vset((*nc << 1) + id, u[ic], fnfp * fc, &dldc[1]);
		    if (*grad == 1) {
			fprr = fp / *ro;
			fdrr = fd / *ro;
			pbtpt = -fa * p[(*np << 1) + ip] / *ro;
			pbtpp = -fm * fdrr * cscthe;
			pbtpr = -fnp2 * fdrr;
			pbppt = -fm * (fdrr + fprr * cotthe) * cscthe;
			pbppp = fm * fm * fprr * cscth2;
			pbppr = -fnp2 * fm * fprr * cscthe;
			pbrpt = -fnp1 * fdrr;
			pbrpp = -fnp1 * fm * fprr * cscthe;
			pbrpr = -fnp1 * fnp2 * fprr;
			r8vset(*nc * 6 + id, lend, pbtpt * fc, &dldc[1]);
			r8vset(*nc * 7 + id, lend, pbtpp * fs, &dldc[1]);
			r8vset((*nc << 3) + id, lend, pbtpr * fc, &dldc[1]);
			r8vset(*nc * 9 + id, lend, pbppt * fs, &dldc[1]);
			r8vset(*nc * 10 + id, lend, pbppp * fc, &dldc[1]);
			r8vset(*nc * 11 + id, lend, pbppr * fs, &dldc[1]);
			r8vset(*nc * 12 + id, lend, pbrpt * fc, &dldc[1]);
			r8vset(*nc * 13 + id, lend, pbrpp * fs, &dldc[1]);
			r8vset(*nc * 14 + id, lend, pbrpr * fc, &dldc[1]);
		    }
		    id += lend;
		    if (m > 0) {
			++ic;
			lend = u[ic];
			r8vset(id, lend, fd * fs, &dldc[1]);
			r8vset(*nc + id, lend, -fmfpst * fc, &dldc[1]);
			r8vset((*nc << 1) + id, lend, fnfp * fs, &dldc[1]);
			if (*grad == 1) {
			    r8vset(*nc * 6 + id, lend, pbtpt * fs, &dldc[1]);
			    r8vset(*nc * 7 + id, lend, -pbtpp * fc, &dldc[1]);
			    r8vset((*nc << 3) + id, lend, pbtpr * fs, &dldc[1]);
			    r8vset(*nc * 9 + id, lend, -pbppt * fc, &dldc[1]);
			    r8vset(*nc * 10 + id, lend, pbppp * fs, &dldc[1]);
			    r8vset(*nc * 11 + id, lend, -pbppr * fc, &dldc[1]);
			    r8vset(*nc * 12 + id, lend, pbrpt * fs, &dldc[1]);
			    r8vset(*nc * 13 + id, lend, -pbrpp * fc, &dldc[1]);
			    r8vset(*nc * 14 + id, lend, pbrpr * fs, &dldc[1]);
			}
			id += lend;
		    }
		}
	    }
	}
	ip = *ie;
	ra = *ro / *rm;
	fr = pow_di(ra, *nmne - 2);
	for (n = *nmne; n <= *nmxe; ++n) {
	    fnm1 = (double) (n - 1);
	    fn = (double) n;
	    fr *= ra;
	    for (m = *mmin; m <= MIN(n,*mmax); ++m) {
		++ip;
		if (m >= *mmne && m <= *mmxe) {
		    ++ic;
		    fm = (double) m;
		    fp = fr * p[ip];
		    fd = -fr * p[*np + ip];
		    fc = t[m + 1];
		    fs = t[*mmax + m + 2];
		    fnfp = -fn * fp;
		    fmfpst = fm * fp * cscthe;
		    lend = u[ic];
		    r8vset(id, lend, fd * fc, &dldc[1]);
		    r8vset(*nc + id, lend, fmfpst * fs, &dldc[1]);
		    r8vset((*nc << 1) + id, lend, fnfp * fc, &dldc[1]);
		    if (*grad == 1) {
			fprr = fp / *ro;
			fdrr = fd / *ro;
			pbtpt = -fr * p[(*np << 1) + ip] / *ro;
			pbtpp = -fm * fdrr * cscthe;
			pbtpr = fnm1 * fdrr;
			pbppt = -fm * (fdrr + fprr * cotthe) * cscthe;
			pbppp = fm * fm * fprr * cscth2;
			pbppr = fnm1 * fm * fprr * cscthe;
			pbrpt = fn * fdrr;
			pbrpp = fn * fm * fprr * cscthe;
			pbrpr = -fn * fnm1 * fprr;
			r8vset(*nc * 6 + id, lend, pbtpt * fc, &dldc[1]);
			r8vset(*nc * 7 + id, lend, pbtpp * fs, &dldc[1]);
			r8vset((*nc << 3) + id, lend, pbtpr * fc, &dldc[1]);
			r8vset(*nc * 9 + id, lend, pbppt * fs, &dldc[1]);
			r8vset(*nc * 10 + id, lend, pbppp * fc, &dldc[1]);
			r8vset(*nc * 11 + id, lend, pbppr * fs, &dldc[1]);
			r8vset(*nc * 12 + id, lend, pbrpt * fc, &dldc[1]);
			r8vset(*nc * 13 + id, lend, pbrpp * fs, &dldc[1]);
			r8vset(*nc * 14 + id, lend, pbrpr * fc, &dldc[1]);
		    }
		    id += lend;
		    if (m > 0) {
			++ic;
			lend = u[ic];
			r8vset(id, lend, fd * fs, &dldc[1]);
			r8vset(*nc + id, lend, -fmfpst * fc, &dldc[1]);
			r8vset((*nc << 1) + id, lend, fnfp * fs, &dldc[1]);
			if (*grad == 1) {
			    r8vset(*nc * 6 + id, lend, pbtpt * fs, &dldc[1]);
			    r8vset(*nc * 7 + id, lend, -pbtpp * fc, &dldc[1]);
			    r8vset((*nc << 3) + id, lend, pbtpr * fs, &dldc[1]);
			    r8vset(*nc * 9 + id, lend, -pbppt * fc, &dldc[1]);
			    r8vset(*nc * 10 + id, lend, pbppp * fs, &dldc[1]);
			    r8vset(*nc * 11 + id, lend, -pbppr * fc, &dldc[1]);
			    r8vset(*nc * 12 + id, lend, pbrpt * fs, &dldc[1]);
			    r8vset(*nc * 13 + id, lend, -pbrpp * fc, &dldc[1]);
			    r8vset(*nc * 14 + id, lend, pbrpr * fs, &dldc[1]);
			}
			id += lend;
		    }
		}
	    }
	}
	tdc(*grad, *nc, *clat, *theta, &dldc[1], &w[1]);
    }
}

void geocen(int ctyp, double re, double rp, double rm, double h, double clat, double *r, double *theta, double *sinthe, double *costhe) {
    int ifirst = 1;
    double re2 = 0, rp2 = 0, kappa, rhoew, rhons, costh2, sinth2;

    *theta = clat;
    *r = rm + h;
    /* *sinthe = sin(*theta);
    *costhe = cos(*theta);*/
	sincos(*theta, sinthe, costhe);
    if (ctyp == 0) {
	if (ifirst == 1) {
	    ifirst = 0;
	    re2 = re * re;
	    rp2 = rp * rp;
	}
	sinth2 = *sinthe * *sinthe;
	costh2 = *costhe * *costhe;
	kappa = 1. / sqrt(re2 * sinth2 + rp2 * costh2);
	rhoew = re2 * kappa + h;
	rhons = rp2 * kappa + h;
	*theta = atan2(rhoew * *sinthe, rhons * *costhe);
	*r = sqrt(rhoew * rhoew * sinth2 + rhons * rhons * costh2);
	/* *sinthe = sin(*theta);
	*costhe = cos(*theta); */
	sincos(*theta, sinthe, costhe);
    }
}

void schmit_(int *grad, int *rgen, int *nmax, int *mmin, int *mmax, double *sinthe, double *costhe, double *p, double *r) {

    int l, n, np, kd0, kd1, kd2, kp0, kp1, kp2, kq2, kq1, kq0, ksm2, ktm2, ksec, ktes, knnp, nd0sec;
    int np1sec, nd0tes, nd1tes, np1tes, np2tes, nq0nnp, nq0msq;
    double cscth2, costh2, sinth2, cscthe, cotthe, cthsth;
    double root3 = 1.732050807568877;

    /* Parameter adjustments */
    --r;
    --p;

    /* Function Body */
    if (*rgen > 6)
	srecur_(grad, nmax, mmin, mmax, &ksm2, &ktm2, &np, &np1sec, &nd0sec, &np1tes, &np2tes, &nd0tes, &nd1tes, &nq0nnp, &nq0msq, &r[1]);
 
    if (*nmax >= 1) {
	kp0 = 1;
	kp2 = kp0;
	if (*mmin <= 0) {
	    p[kp0] = *costhe;
	    p[np + kp0] = -(*sinthe);
	    if (*grad == 1)
		p[(np << 1) + kp0] = -(*costhe);

	    ++kp0;
	}
	if (*mmax >= 1) {
	    p[kp0] = *sinthe;
	    p[np + kp0] = *costhe;
	    if (*grad == 1)
		p[(np << 1) + kp0] = -(*sinthe);

	    ++kp0;
	}
	if (*nmax >= 2) {
	    kp1 = kp0;
	    sinth2 = *sinthe * *sinthe;
	    costh2 = *costhe * *costhe;
	    cthsth = *costhe * *sinthe;
	    if (*mmin <= 0) {
		p[kp0] = (costh2 * 3. - 1.) / 2.;
		p[np + kp0] = cthsth * -3.;
		if (*grad == 1) {
		    p[(np << 1) + kp0] = (costh2 * 2. - 1.) * -3.;
		}
		++kp0;
	    }
	    if (*mmin <= 1 && *mmax >= 1) {
		p[kp0] = root3 * cthsth;
		p[np + kp0] = root3 * (costh2 * 2. - 1.);
		if (*grad == 1) {
		    p[(np << 1) + kp0] = root3 * -4. * cthsth;
		}
		++kp0;
	    }
	    if (*mmax >= 2) {
		p[kp0] = root3 * sinth2 / 2.;
		p[np + kp0] = root3 * cthsth;
		if (*grad == 1) {
		    p[(np << 1) + kp0] = root3 * (costh2 * 2. - 1.);
		}
		++kp0;
	    }
	    kd2 = np + kp2;
	    kd1 = np + kp1;
	    kd0 = np + kp0;
	    kq2 = np + kd2;
	    kq1 = np + kd1;
	    kq0 = np + kd0;
	    ksec = 1;
	    ktes = 1;
	    knnp = 1;
	    if (*sinthe == 0.) {
		for (n = 3; n <= *nmax; ++n) {
		    l = MAX(0, MIN(n-1,*mmax) - *mmin + 1);
		    if (l > 0) {
			r8vset(kp0, l, 0., &p[1]);
			r8vlinkq(kp1, np1tes + ktes, kp0, l, *costhe, &p[1], &r[1], &p[1]);
			r8vlinkq(kp2, np2tes + ktes, kp0, l, -1., &p[1], &r[1], &p[1]);
			r8vset(kd0, l, 0., &p[1]);
			r8vlinkq(kd1, np1tes + ktes, kd0, l, *costhe, &p[1], &r[1], &p[1]);
			r8vlinkq(kd2, np2tes + ktes, kd0, l, -1., &p[1], &r[1], &p[1]);
			if (*grad == 1) {
			    r8vset(kq0, l, 0., &p[1]);
			    r8vlinkq(kq1, np1tes + ktes, kq0, l, *costhe, &p[1], &r[1], &p[1]);
			    r8vlinkq(kp1, np1tes + ktes, kq0, l, -(*costhe), &p[1], &r[1], &p[1]);
			    r8vlinkq(kq2, np2tes + ktes, kq0, l, -1., &p[1], &r[1], &p[1]);
			}
			ktes += l;
		    }
		    if (n <= *mmax) {
			p[kp0 + l] = 0.;
			p[kd0 + l] = 0.;
			if (*grad == 1)
			    p[kq0 + l] = 0.;
			++l;
		    }
		    kp2 = kp1;
		    kp1 = kp0;
		    kp0 += l;
		    kd2 = kd1;
		    kd1 = kd0;
		    kd0 += l;
		    kq2 = kq1;
		    kq1 = kq0;
		    kq0 += l;
		}
	    } else {
		cotthe = *costhe / *sinthe;
		cscthe = 1. / *sinthe;
		cscth2 = cscthe * cscthe;
		for (n = 3; n <= *nmax; ++n) {
		    l = MAX(0, MIN(n-1,*mmax) - *mmin + 1);
		    if (l > 0) {
			r8vset(kp0, l, 0., &p[1]);
			r8vlinkq(kp1, np1tes + ktes, kp0, l, *costhe, &p[1], &r[1], &p[1]);
			r8vlinkq(kp2, np2tes + ktes, kp0, l, -1., &p[1], &r[1], &p[1]);
			r8vset(kd0, l, 0., &p[1]);
			r8vlinkq(kp0, nd0tes + ktes, kd0, l, cotthe, &p[1], &r[1], &p[1]);
			r8vlinkq(kp1, nd1tes + ktes, kd0, l, -cscthe, &p[1], &r[1], &p[1]);
			if (*grad == 1) {
			    r8vset(kq0, l, 0., &p[1]);
			    r8vlinkq(kp0, nq0msq + ktm2, kq0, l, cscth2, &p[1], &r[1], &p[1]);
			    r8vlinkt(kp0, kq0, l, -r[nq0nnp + knnp], &p[1], &p[1]);
			    r8vlinkt(kd0, kq0, l, -cotthe, &p[1], &p[1]);
			}
			ktes += l;
		    }
		    if (n <= *mmax) {
			p[kp0 + l] = r[np1sec + ksec] * *sinthe * p[kp0 - 1];
			p[kd0 + l] = r[nd0sec + ksec] * cotthe * p[kp0 + l];
			if (*grad == 1)
			    p[kq0 + l] = (r[nq0msq - ksm2 + n + 1] * cscth2 -r[nq0nnp + knnp]) * p[kp0 + l] - cotthe * p[kd0 + l];

			++ksec;
			++l;
		    }
		    ++knnp;
		    kp2 = kp1;
		    kp1 = kp0;
		    kp0 += l;
		    kd0 += l;
		    kq0 += l;
		}
	    }
	}
    }
}

void srecur_(int *grad, int *nmax, int *mmin, int *mmax, int *ksm2, int *ktm2, int *npall, int *
	nad1, int *nad2, int *nad3, int *nad4, int *nad5, int *nad6, int *nad7, int *nad8, double *r) {
    int m, n, ksec, kmin, lmax, kmax, ktes, knnp, kmsq, nnnp1, nsect, ntess;
    double fn, f2n, fnl1, fnp1, f2nl1, fnsq, fmsq, fnl1sq, fdsqrt;

    /* Parameter adjustments */
    --r;

    /* Function Body */
    lmax = MIN(*nmax,2);
    kmax = MIN(*mmax,2);
    kmin = MIN(*mmin,2);
    *ksm2 = MIN(*mmin,3);
    *ktm2 = I_DIM(*mmin, 3) + 1;
    *npall = nlpx(*nmax, *mmax, *mmin);
    ntess = *npall - nlpx(lmax, kmax, kmin) + kmax - *mmax;
    nsect = MAX(0, *mmax - 2);
    nnnp1 = MAX(0, *nmax - 2);
    *nad1 = 0;
    *nad2 = nsect;
    *nad3 = nsect + *nad2;
    *nad4 = ntess + *nad3;
    *nad5 = ntess + *nad4;
    *nad6 = ntess + *nad5;
    *nad7 = ntess + *nad6;
    *nad8 = nnnp1 + *nad7;
    ksec = ktes = knnp = 0;
    for (n = 3; n <= *nmax; ++n) {
	fnp1 = (double) (n + 1);
	fn = (double) n;
	fnl1 = (double) (n - 1);
	f2n = fn * 2.;
	f2nl1 = f2n - 1.;
	fnsq = fn * fn;
	fnl1sq = fnl1 * fnl1;
	if (n <= *mmax) {
	    ++ksec;
	    r[*nad1 + ksec] = sqrt(f2nl1 / f2n);
	    r[*nad2 + ksec] = fn;
	}
	if (*grad == 1) {
	    ++knnp;
	    r[*nad7 + knnp] = fn * fnp1;
	}
	for (m = *mmin; m <= MIN(n-1,*mmax); ++m) {
	    ++ktes;
	    fmsq = (double)(m * m);
	    fdsqrt = sqrt(fnsq - fmsq);
	    r[*nad3 + ktes] = f2nl1 / fdsqrt;
	    r[*nad4 + ktes] = sqrt(fnl1sq - fmsq) / fdsqrt;
	    r[*nad5 + ktes] = fn;
	    r[*nad6 + ktes] = fdsqrt;
	}
    }
    if (*grad == 1) {
	kmsq = 0;
	for (m = *ksm2; m <= *mmax; ++m) {
	    ++kmsq;
	    r[*nad8 + kmsq] = (double)(m * m);
	}
    }
}

void trigmp(int mmax, double phi, double *t) {
    int m;

    /* Parameter adjustments */
    --t;

    t[1] = 1.;
    t[mmax + 2] = 0.;
    if (mmax > 0) {
	t[2] = cos(phi);
	t[mmax + 3] = sin(phi);
	for (m = 2; m <= mmax; ++m) {
	    t[m + 1] = t[2] * 2. * t[m] - t[m - 1];
	    t[mmax + m + 2] = t[2] * 2. * t[mmax + m + 1] - t[mmax + m];
	}
    }
}

void tdc(int grad, int nc, double clat, double theta, double *dldc, double *r) {
    double cpsi, spsi;

	sincos(theta - clat, &spsi, &cpsi);
    r[0] = -cpsi;
    r[1] = 0.;
    r[2] = -spsi;
    r[3] = 0.;
    r[4] = 1.;
    r[5] = 0.;
    r[6] = spsi;
    r[7] = 0.;
    r[8] = -cpsi;
    ltranv(1, nc, nc, &r[0], &dldc[0]);
    if (grad == 1) {
	ltranv(0, nc * 3, nc * 3, &r[0], &dldc[nc * 6]);
	ltranv(0, nc, nc, &r[0], &dldc[nc * 6]);
	ltranv(0, nc, nc, &r[0], &dldc[nc * 9]);
	ltranv(0, nc, nc, &r[0], &dldc[nc * 12]);
    }
}

void fdsdc_(int *rgen, int *ityp, int *etyp, int *nsi, int *nse, int *nc, int *nci, double *ta,
	double *tb, double *dst, double *dstt, int *dsti, int *bori, int *bkni, double *bkpi, int *tdgi, 
	int *dste, int *bore, int *bkne, double *bkpe, int *tdge, int *u, double *w, double *dsdc, int *cerr) {

    static double tbo = 0.;

    /* System generated locals */
    int i__1;

    /* Local variables */
    int tgen, edst, idst, esvr, isvr;

    /* Parameter adjustments */
    --dsdc;
    --w;
    --u;
    --tdge;
    --bkpe;
    --bkne;
    --bore;
    --dste;
    --tdgi;
    --bkpi;
    --bkni;
    --bori;
    --dsti;

    /* Function Body */
    tgen = MAX(0,*rgen - 6);
    if (tbo != *tb) {
	tgen = MIN(1,tgen + *ityp + *etyp);
	*rgen += tgen;
	tbo = *tb;
    }
    if (tgen > 0) {
	r8vset(1, *nc << 1, 0., &dsdc[1]);
	if (*nsi > 0) {
	    isvr = *ityp % 3;
	    idst = *ityp / 3;
	    i8vcum(1, 1, *nsi, &u[1]);
	    r8vscats(1, *nsi, 1., &u[1], &dsdc[1]);
	    r8vscats(1, *nsi, 0., &u[1], &dsdc[*nc + 1]);
	    i8vadds(1, 1, *nsi, 1, &u[1], &u[1]);
	    if (isvr == 1)
		taylor(*nc, *nsi, *ta, *tb, &tdgi[1], &u[1], &w[1], &dsdc[1]);
	    else if (isvr == 2) {
		bsplyn(*nc, *nsi, ta, tb, &bori[1], &bkni[1], &bkpi[1], &u[1], &w[1], &dsdc[1], cerr);
		if (*cerr >= 50) goto L10;
	    }
	    if (idst == 1)
		dstorm(*nc, *nsi, dst, dstt, &dsti[1], &u[1], &dsdc[1]);

L10:
	    i8vdel(1, 1, *nsi, &u[1]);
	}
	if (*nse > 0) {
	    esvr = *etyp % 3;
	    edst = *etyp / 3;
	    i__1 = *nsi + 1;
	    i8vcum(1, i__1, *nse, &u[1]);
	    r8vscats(i__1, *nse, 1., &u[1], &dsdc[*nci + 1]);
	    r8vscats(i__1, *nse, 0., &u[1], &dsdc[*nc + *nci + 1]);
	    i8vadds(i__1, i__1, *nse, 1, &u[1], &u[1]);
	    if (esvr == 1)
		taylor(*nc, *nse, *ta, *tb, &tdge[1], &u[*nsi + 1], &w[1], &dsdc[*nci + 1]);
	    else if (esvr == 2) {
		bsplyn(*nc, *nse, ta, tb, &bore[1], &bkne[1], &bkpe[1], &u[*nsi + 1], &w[1], &dsdc[*nci + 1], cerr);
		if (*cerr >= 50) goto L20;
	    }
	    if (edst == 1)
		dstorm(*nc, *nse, dst, dstt, &dste[1], &u[*nsi + 1], &dsdc[*nci + 1]);

L20:
	    i8vdel(1, *nsi + 1, *nse, &u[1]);
	}
    }
}

void taylor(int nc, int ns, double ta, double tb, int *tdeg, int *u, double *dsdt, double *dsdc) {
    int i, j, n, iu;
    double dt;

    /* Parameter adjustments */
    --dsdt;
    --u;
    --tdeg;

    /* Function Body */
    dt = tb - ta;
    for (i = 1; i <= ns; ++i) {
	n = tdeg[i];
	if (n > 0) {
	    iu = u[i];
	    dsdt[1] = 1.;
	    for (j = 1; j <= n; ++j) {
		dsdt[j + 1] = dsdt[j] * dt / (double) j;
	    }
	    r8vgathp(2, 1, iu, n, &dsdt[1], &dsdc[0]);
	    r8vgathp(1, 1, nc + iu, n, &dsdt[1], &dsdc[0]);
	    u[i] += n;
	}
    }
}

void bsplyn(int nc, int ns, double *ta, double *tb, int *bord, int *bkno, double *bkpo, int *u, double *dtdb, double *dsdc, int *cerr) {
    int i, k, l, m, n, ik, iu;

    /* Parameter adjustments */
    --dsdc;
    --dtdb;
    --u;
    --bkpo;
    --bkno;
    --bord;

    /* Function Body */
    ik = 1;
    for (i = 1; i <= ns; ++i) {
	n = bord[i];
	k = bkno[i];
	l = n + k;
	iu = u[i];
	if (n > 0) {
	    m = n + 1;
	    sbspln_(ta, tb, &m, &k, &bkpo[ik], &dtdb[1], &dsdc[iu], cerr);
	    if (*cerr >= 50) return;
	    r8vset(1, l + 1, 0., &dtdb[1]);
	    tbspln_(tb, &n, &k, &bkpo[ik], &dtdb[1], cerr);
	    if (*cerr >= 50) return;
	    r8vgathp(1, 1, nc + iu, l, &dtdb[1], &dsdc[1]);
	} else if (k > 0) {
	    r8vset(iu, l, 0., &dsdc[1]);
	    r8vset(nc + iu, l, 0., &dsdc[1]);
	}
	u[i] += l;
	ik = ik + k + 2;
    }
}

void sbspln_(double *ta, double *tb, int *n, int *k, double *bkpo, double *dtdb, double *dsdc, int *cerr) {
    int i, ik, jk, ip, nl, np, ns;
    double rn;

    /* Parameter adjustments */
    --dsdc;
    --dtdb;
    --bkpo;

    /* Function Body */
    if (*n > 1) {
	nl = *n + *k + 1;
	r8vset(1, nl << 1, 0., &dtdb[1]);
	tbspln_(tb, n, k, &bkpo[1], &dtdb[1], cerr);
	if (*cerr >= 50) return;
	tbspln_(ta, n, k, &bkpo[1], &dtdb[nl + 1], cerr);
	if (*cerr >= 50) return;
	r8vsub(nl+1, 1, 1, nl, &dtdb[1], &dtdb[1], &dtdb[1]);
	rn = 1. / (double) (*n - 1);
	np = *n + *k - 1;
	ns = np;
	for (i = 1; i <= np; ++i) {
	    ip = i + 1;
	    ik = MIN(ip,*k + 2);
	    jk = MAX(ip - *n + 1,1);
	    dsdc[i] = (bkpo[ik] - bkpo[jk]) * r8ssum_(&ip, &ns, &dtdb[1]);
	    --ns;
	}
	if (np > 0)
	    r8vscale(1, np, rn, &dsdc[1]);
    }
}

void tbspln_(double *t, int *n, int *k, double *bkpo, double *dtdb, int *cerr) {
    int i, j, l, m, p, y, ik, jk, posb;
    double difi, difj, delk, temp;

    /* Parameter adjustments */
    --dtdb;
    --bkpo;

    /* Function Body */
    if (*t >= bkpo[1] && *t <= bkpo[*k + 2]) {
	r8slt(1, *k + 2, *t, &bkpo[1], &y);
	l = MIN(*k+2,y+1);
	p = l + *n - 2;
	if (*n == 1)
	    dtdb[p] = 1.;
	else if (*n > 1) {
	    m = l;
	    ik = MIN(m,*k+2);
	    jk = MAX(m-1,1);
	    difi = bkpo[ik] - *t;
	    delk = bkpo[ik] - bkpo[jk];
	    if (delk == 0.)
		dtdb[p] = 0.;
	    else
		dtdb[p] = 1. / delk;

	    posb = p - 1;
	    for (j = 2; j <= *n; ++j) {
		jk = MAX(m-j,1);
		temp = difi * dtdb[posb + 1];
		delk = bkpo[ik] - bkpo[jk];
		if (delk == 0.)
		    temp = 0.;
		else if (j < *n)
			temp /= delk;

		dtdb[posb] = temp;
		--posb;
	    }
	    dtdb[p + 1] = 0.;
	    ++m;
	    for (i = 2; i <= *n; ++i) {
		ik = MIN(m,*k+2);
		difi = bkpo[ik] - *t;
		posb = p;
		for (j = i; j <= *n; ++j) {
		    jk = MAX(m-j,1);
		    difj = *t - bkpo[jk];
		    temp = difj * dtdb[posb] + difi * dtdb[posb + 1];
		    delk = bkpo[ik] - bkpo[jk];
		    if (delk == 0.)
			temp = 0.;
		    else if (j < *n)
			temp /= delk;

		    dtdb[posb] = temp;
		    --posb;
		}
		++m;
	    }
	}
    } else {
	fprintf (stderr, "TBSPLN -- Error: T (%f) LIES OUTSIDE OF KNOT DOMAIN [%f; %f] -- ABORT\n", *t, bkpo[1], bkpo[*k + 2]);
	*cerr = 50;
    }
}

void dstorm(int nc, int ns, double *dst, double *dstt, int *dstm, int *u, double *dsdc) {
    int i, n, iu;

    /* Parameter adjustments */
    --u;
    --dstm;

    for (i = 1; i <= ns; ++i) {
	n = dstm[i];
	if (n > 0) {
	    iu = u[i];
	    r8vset(iu, n, *dst, &dsdc[0]);
	    r8vset(nc + iu, n, *dstt, &dsdc[0]);
	    u[i] += n;
	}
    }
}

void fdldc(int grad, int nc, double *dsdc, double *dldc) {
    int i__1, i, j;

    i = 1;
    for (j = 0; j < 3; ++j) {
	r8vmul(nc+1, i, nc * 3 + i, nc, &dsdc[0], &dldc[0], &dldc[0]);
	i += nc;
    }
    i = 1;
    for (j = 0; j < 3; ++j) {
	r8vmul(1, i, i, nc, &dsdc[0], &dldc[0], &dldc[0]);
	i += nc;
    }
    if (grad == 1) {
	i = 1;
	for (j = 0; j < 9; ++j) {
	    r8vmul(nc+1, nc * 6 + i, nc * 15 + i, nc, &dsdc[0], &dldc[0], &dldc[0]);
	    i += nc;
	}
	i = 1;
	for (j = 0; j < 9; ++j) {
	    i__1 = nc * 6 + i;
	    r8vmul(1, i__1, i__1, nc, &dsdc[0], &dldc[0], &dldc[0]);
	    i += nc;
	}
    }
}

void blgen(int grad, int nc, double *b, double *c, double *dldc) {
    int i, j;

    i = 1;
    for (j = 0; j < 6; ++j) {
	b[j] += r8sdot(i, 1, nc, &dldc[0], &c[0]);
	i += nc;
    }
    if (grad == 1) {
	i = 1;
	for (j = 28; j < 46; ++j) {
	    b[j] += r8sdot(nc * 6 + i, 1, nc, &dldc[0], &c[0]);
	    i += nc;
	}
    }
}

void bngen_(double *b) {
    double cd, cf, ch, ci, cx, cy, cz, cdt, cft, cht, cit, cxt, cyt, czt;

    cx = b[0];
    cy = b[1];
    cz = b[2];
    cxt = b[3];
    cyt = b[4];
    czt = b[5];
    ch = sqrt(cx * cx + cy * cy);
    cf = sqrt(ch * ch + cz * cz);
    if (ch != 0.) {
	cd = atan(cy / (ch + cx)) * 2.;
	cdt = (cx * cyt - cy * cxt) / ch / ch;
	cht = (cx * cxt + cy * cyt) / ch;
    } else {
	cd = cdt = cht = 0.;
    }
    if (cf != 0.) {
	ci = atan(cz / (cf + ch)) * 2.;
	cit = (ch * czt - cz * cht) / cf / cf;
	cft = (ch * cht + cz * czt) / cf;
    } else {
	ci = cit = cft = 0.;
    }
    b[6] = cd;
    b[7] = ci;
    b[8] = ch;
    b[9] = cf;
    b[10] = cdt;
    b[11] = cit;
    b[12] = cht;
    b[13] = cft;
}

void tec(int grad, int k, int nc, double *theta, double *phi, double *b, double *dldc, double *r) {
    int i__1;

    /* Local variables */
    double cphi, sphi, costhe, sinthe;

    /* Parameter adjustments */
    --b;

    if (k >= 1) {
	sinthe = sin(*theta);
	costhe = cos(*theta);
	sincos(*theta, &sinthe, &costhe);
	sincos(*phi, &sphi, &cphi);
	r[0] = -costhe * cphi;
	r[1] = -sphi;
	r[2] = -sinthe * cphi;
	r[3] = -costhe * sphi;
	r[4] = cphi;
	r[5] = -sinthe * sphi;
	r[6] = sinthe;
	r[7] = 0.;
	r[8] = -costhe;
	ltrans(1, 1, &b[1], &r[0], &b[1]);
	ltrans(1, 1, &b[4], &r[0], &b[4]);
	ltranv(1, nc, nc, &r[0], &dldc[0]);
	ltranv(0, nc, nc, &r[0], &dldc[nc * 3]);
	bngen_(&b[1]);
	if (grad == 1) {
	    ltranv(0, 3, 3, &r[0], &b[29]);
	    ltranv(0, 3, 3, &r[0], &b[38]);
	    ltrans(1, 1, &b[29], &r[0], &b[29]);
	    ltrans(1, 1, &b[32], &r[0], &b[32]);
	    ltrans(1, 1, &b[35], &r[0], &b[35]);
	    ltrans(1, 1, &b[38], &r[0], &b[38]);
	    ltrans(1, 1, &b[41], &r[0], &b[41]);
	    ltrans(1, 1, &b[44], &r[0], &b[44]);
	    i__1 = nc * 3;
	    ltranv(0, i__1, i__1, &r[0], &dldc[nc * 6]);
	    ltranv(0, i__1, i__1, &r[0], &dldc[nc * 15]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 6]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 9]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 12]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 15]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 18]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 21]);
	}
    }
}

void tse(int grad, int k, int nc, double *rse, double *b, double *dldc, double *r) {
    int i__1;

    if (k >= 1) {
	r8vgathp(1, 1, 1, 9, &rse[0], &r[0]);
	ltrans(1, 1, &b[0], &r[0], &b[0]);
	ltrans(1, 1, &b[3], &r[0], &b[3]);
	ltranv(1, nc, nc, &r[0], &dldc[0]);
	ltranv(0, nc, nc, &r[0], &dldc[nc * 3]);
	bngen_(&b[0]);
	if (grad == 1) {
	    ltranv(0, 3, 3, &r[0], &b[28]);
	    ltranv(0, 3, 3, &r[0], &b[37]);
	    ltrans(1, 1, &b[28], &r[0], &b[28]);
	    ltrans(1, 1, &b[31], &r[0], &b[31]);
	    ltrans(1, 1, &b[34], &r[0], &b[34]);
	    ltrans(1, 1, &b[37], &r[0], &b[37]);
	    ltrans(1, 1, &b[40], &r[0], &b[40]);
	    ltrans(1, 1, &b[43], &r[0], &b[43]);
	    i__1 = nc * 3;
	    ltranv(0, i__1, i__1, &r[0], &dldc[nc * 6]);
	    ltranv(0, i__1, i__1, &r[0], &dldc[nc * 15]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 6]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 9]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 12]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 15]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 18]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 21]);
	}
    }
}

void tms(int grad, int k, int nc, int na, int ia, double *a, double *b, double *dldc, double *dlda, double *r) {
    int i__1;
    double eulx, euly, eulz, ceulx, ceuly, ceulz, seulx, seuly, seulz;

    if (k >= 1) {
	eulx = a[ia];
	euly = a[ia + 1];
	eulz = a[ia + 2];
	sincos(eulx, &seulx, &ceulx);
	sincos(euly, &seuly, &ceuly);
	sincos(eulz, &seulz, &ceulz);
	fdldeu_(&k, &na, &ia, &seulx, &ceulx, &seuly, &ceuly, &seulz, &ceulz, &r[0], &b[1], &dlda[0]);
	r[0] = ceuly * ceulz;
	r[1] = -seulz;
	r[2] = -seuly * ceulz;
	r[3] = ceuly * ceulx * seulz + seuly * seulx;
	r[4] = ceulx * ceulz;
	r[5] = ceuly * seulx - seuly * seulz * ceulx;
	r[6] = -ceuly * seulx * seulz + seuly * ceulx;
	r[7] = -seulx * ceulz;
	r[8] = ceuly * ceulx + seuly * seulx * seulz;
	ltrans(1, 1, &b[0], &r[0], &b[0]);
	ltrans(1, 1, &b[3], &r[0], &b[3]);
	ltranv(1, nc, nc, &r[0], &dldc[0]);
	ltranv(0, nc, nc, &r[0], &dldc[nc * 3]);
	ltranv(0, na, ia, &r[0], &dlda[0]);
	ltranv(0, na, ia, &r[0], &dlda[na * 3]);
	bngen_(&b[1]);
	if (grad == 1) {
	    ltranv(0, 3, 3, &r[0], &b[28]);
	    ltranv(0, 3, 3, &r[0], &b[37]);
	    ltrans(1, 1, &b[28], &r[0], &b[28]);
	    ltrans(1, 1, &b[31], &r[0], &b[31]);
	    ltrans(1, 1, &b[34], &r[0], &b[34]);
	    ltrans(1, 1, &b[37], &r[0], &b[37]);
	    ltrans(1, 1, &b[40], &r[0], &b[40]);
	    ltrans(1, 1, &b[43], &r[0], &b[43]);
	    i__1 = nc * 3;
	    ltranv(0, i__1, i__1, &r[0], &dldc[nc * 6]);
	    ltranv(0, i__1, i__1, &r[0], &dldc[nc * 15]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 6]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 9]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 12]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 15]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 18]);
	    ltranv(0, nc, nc, &r[0], &dldc[nc * 21]);
	}
	ia += 3;
    }
}

void fdldeu_(int *k, int *na, int *ia, double *seulx, double *ceulx, double *seuly, double *ceuly, 
	double *seulz, double *ceulz, double *r, double *b, double *dlda) {

    int i, j;

    if (*k == 1) {
	i = *ia;
	for (j = 1; j <= 6; ++j) {
	    dlda[i] = 0.;
	    dlda[i + 1] = 0.;
	    dlda[i + 2] = 0.;
	    i += *na;
	}
    } else {
	r[0] = 0.;
	r[1] = 0.;
	r[2] = 0.;
	r[3] = -(*ceuly) * *seulx * *seulz + *seuly * *ceulx;
	r[4] = -(*seulx) * *ceulz;
	r[5] = *ceuly * *ceulx + *seuly * *seulz * *seulx;
	r[6] = -(*ceuly) * *ceulx * *seulz - *seuly * *seulx;
	r[7] = -(*ceulx) * *ceulz;
	r[8] = -(*ceuly) * *seulx + *seuly * *ceulx * *seulz;
	ltrans(*na, 1, &b[0], &r[0], &dlda[*ia]);
	ltrans(*na, 1, &b[3], &r[0], &dlda[*na * 3 + *ia]);
	r[0] = -(*seuly) * *ceulz;
	r[1] = 0.;
	r[2] = -(*ceuly) * *ceulz;
	r[3] = -(*seuly) * *ceulx * *seulz + *ceuly * *seulx;
	r[4] = 0.;
	r[5] = -(*seuly) * *seulx - *ceuly * *seulz * *ceulx;
	r[6] = *seuly * *seulx * *seulz + *ceuly * *ceulx;
	r[7] = 0.;
	r[8] = -(*seuly) * *ceulx + *ceuly * *seulx * *seulz;
	ltrans(*na, 1, &b[0], &r[0], &dlda[*ia + 1]);
	ltrans(*na, 1, &b[3], &r[0], &dlda[*na * 3 + *ia + 1]);
	r[0] = -(*ceuly) * *seulz;
	r[1] = -(*ceulz);
	r[2] = *seuly * *seulz;
	r[3] = *ceuly * *ceulx * *ceulz;
	r[4] = -(*ceulx) * *seulz;
	r[5] = -(*seuly) * *ceulz * *ceulx;
	r[6] = -(*ceuly) * *seulx * *ceulz;
	r[7] = *seulx * *seulz;
	r[8] = *seuly * *seulx * *ceulz;
	ltrans(*na, 1, &b[0], &r[0], &dlda[*ia + 2]);
	ltrans(*na, 1, &b[3], &r[0], &dlda[*na * 3 + *ia + 2]);
    }
}

void tnm_(int *grad, int *k, int *nc, int *na, int *ia, double *a, double *b, double *dldc, double *dlda, double *r) {
    int i__1;
    double chix, chiy, chiz, cchix, cchiy, cchiz, schix, schiy, schiz;

    if (*k >= 1) {
	chix = a[*ia];
	chiy = a[*ia + 1];
	chiz = a[*ia + 2];
	sincos(chix, &schix, &cchix);
	sincos(chiy, &schiy, &cchiy);
	sincos(chiz, &schiz, &cchiz);
	fdldno_(k, na, ia, &schix, &cchix, &schiy, &cchiy, &schiz, &cchiz, &r[0], &b[0], &dlda[0]);
	r[0] = 1.;
	r[1] = 0.;
	r[2] = 0.;
	r[3] = schix;
	r[4] = cchix;
	r[5] = 0.;
	r[6] = schiy * cchiz;
	r[7] = schiy * schiz;
	r[8] = cchiy;
	ltrans(1, 1, &b[0], &r[0], &b[0]);
	ltrans(1, 1, &b[3], &r[0], &b[3]);
	ltranv(1, *nc, *nc, &r[0], &dldc[0]);
	ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 3]);
	ltranv(0, *na, *ia, &r[0], &dlda[0]);
	ltranv(0, *na, *ia, &r[0], &dlda[*na * 3]);
	bngen_(&b[0]);
	if (*grad == 1) {
	    ltranv(0, 3, 3, &r[0], &b[28]);
	    ltranv(0, 3, 3, &r[0], &b[37]);
	    ltrans(1, 1, &b[28], &r[0], &b[28]);
	    ltrans(1, 1, &b[31], &r[0], &b[31]);
	    ltrans(1, 1, &b[34], &r[0], &b[34]);
	    ltrans(1, 1, &b[37], &r[0], &b[37]);
	    ltrans(1, 1, &b[40], &r[0], &b[40]);
	    ltrans(1, 1, &b[43], &r[0], &b[43]);
	    i__1 = *nc * 3;
	    ltranv(0, i__1, i__1, &r[0], &dldc[*nc * 6]);
	    ltranv(0, i__1, i__1, &r[0], &dldc[*nc * 15]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 6]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 9]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 12]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 15]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 18]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 21]);
	}
	*ia += 3;
    }
}

void fdldno_(int *k, int *na, int *ia, double *schix, double *cchix, double *schiy, double *cchiy, 
	double *schiz, double *cchiz, double *r, double *b, double *dlda) {

    int i, j;

    if (*k == 1) {
	i = *ia;
	for (j = 1; j <= 6; ++j) {
	    dlda[i] = 0.;
	    dlda[i + 1] = 0.;
	    dlda[i + 2] = 0.;
	    i += *na;
	}
    } else {
	r[0] = r[1] = r[2] = 0.;
	r[3] = *cchix;
	r[4] = -(*schix);
	r[5] = r[6] = r[7] = r[8] = 0.;
	ltrans(*na, 1, &b[0], &r[0], &dlda[*ia]);
	ltrans(*na, 1, &b[3], &r[0], &dlda[*na * 3 + *ia]);
	r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = 0.;
	r[6] = *cchiy * *cchiz;
	r[7] = *cchiy * *schiz;
	r[8] = -(*schiy);
	ltrans(*na, 1, &b[0], &r[0], &dlda[*ia + 1]);
	ltrans(*na, 1, &b[3], &r[0], &dlda[*na * 3 + *ia + 1]);
	r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = 0.;
	r[6] = -(*schiy) * *schiz;
	r[7] = *schiy * *cchiz;
	r[8] = 0.;
	ltrans(*na, 1, &b[0], &r[0], &dlda[*ia + 2]);
	ltrans(*na, 1, &b[3], &r[0], &dlda[*na * 3 + *ia + 2]);
    }
}

void tvn_(int *grad, int *k, int *nc, int *na, int *ia, double *a, double *b, double *dldc, double *dlda, double *r) {
    int i__1;

    /* Local variables */
    double slpx, slpy, slpz;

    if (*k >= 1) {
	slpx = a[*ia];
	slpy = a[*ia + 1];
	slpz = a[*ia + 2];
	fdldsl_(k, na, ia, &b[0], &dlda[0]);
	r[0] = slpx;
	r[1] = 0.;
	r[2] = 0.;
	r[3] = 0.;
	r[4] = slpy;
	r[5] = 0.;
	r[6] = 0.;
	r[7] = 0.;
	r[8] = slpz;
	ltrans(1, 1, &b[0], &r[0], &b[0]);
	ltrans(1, 1, &b[3], &r[0], &b[3]);
	ltranv(1, *nc, *nc, &r[0], &dldc[0]);
	ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 3]);
	ltranv(0, *na, *ia, &r[0], &dlda[0]);
	ltranv(0, *na, *ia, &r[0], &dlda[*na * 3]);
	bngen_(&b[0]);
	if (*grad == 1) {
	    ltranv(0, 3, 3, &r[0], &b[28]);
	    ltranv(0, 3, 3, &r[0], &b[37]);
	    ltrans(1, 1, &b[28], &r[0], &b[28]);
	    ltrans(1, 1, &b[31], &r[0], &b[31]);
	    ltrans(1, 1, &b[34], &r[0], &b[34]);
	    ltrans(1, 1, &b[37], &r[0], &b[37]);
	    ltrans(1, 1, &b[40], &r[0], &b[40]);
	    ltrans(1, 1, &b[43], &r[0], &b[43]);
	    i__1 = *nc * 3;
	    ltranv(0, i__1, i__1, &r[0], &dldc[*nc * 6]);
	    ltranv(0, i__1, i__1, &r[0], &dldc[*nc * 15]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 6]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 9]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 12]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 15]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 18]);
	    ltranv(0, *nc, *nc, &r[0], &dldc[*nc * 21]);
	}
	*ia += 3;
    }
}

void fdldsl_(int *k, int *na, int *ia, double *b, double *dlda) {
    int i, j;

    i = *ia;
    for (j = 0; j < 6; ++j) {
	dlda[i] = 0.;
	dlda[i + 1] = 0.;
	dlda[i + 2] = 0.;
	i += *na;
    }
    if (*k > 1) {
	i = *ia;
	dlda[i] = b[0];
	i += *na;
	dlda[i + 1] = b[1];
	i += *na;
	dlda[i + 2] = b[2];
	i += *na;
	dlda[i] = b[3];
	i += *na;
	dlda[i + 1] = b[4];
	i += *na;
	dlda[i + 2] = b[5];
    }
}

void tbi_(int *k, int *na, int *ia, double *a, double *b, double *dlda) {
    double biax, biay, biaz;

    if (*k >= 1) {
	biax = a[*ia];
	biay = a[*ia + 1];
	biaz = a[*ia + 2];
	fdldbi_(k, na, ia, &dlda[0]);
	b[0] += biax;
	b[1] += biay;
	b[2] += biaz;
	bngen_(&b[0]);
	*ia += 3;
    }
}

void fdldbi_(int *k, int *na, int *ia, double *dlda) {
    int i, j;

    i = *ia;
    for (j = 0; j < 6; ++j) {
	dlda[i] = 0.;
	dlda[i + 1] = 0.;
	dlda[i + 2] = 0.;
	i += *na;
    }
    if (*k > 1) {
	i = *ia;
	dlda[i] = 1.;
	i += *na;
	dlda[i + 1] = 1.;
	i += *na;
	dlda[i + 2] = 1.;
    }
}

void ltrans(int n, int m, double *q, double *r, double *s) {
    double q3;

    q3 = q[m + m];
    s[0] = q[0] * r[0] + q[m] * r[1] + q3 * r[2];
    s[n] = q[0] * r[3] + q[m] * r[4] + q3 * r[5];
    s[n + n] = q[0] * r[6] + q[m] * r[7] + q3 * r[8];
}

void ltranv(int rfac, int n, int m, double *r, double *v) {
    /* System generated locals */
    int i__1;

    /* Parameter adjustments */
    --r;

    /* Function Body */
    if (m > 0) {
	if (rfac == 1) {
	    r[10] = r[4] / r[1];
	    r[11] = r[5] - r[2] * r[10];
	    r[12] = r[6] - r[3] * r[10];
	    r[13] = r[7] / r[1];
	    r[14] = (r[8] - r[2] * r[13]) / r[11];
	    r[15] = r[9] - r[3] * r[13] - r[12] * r[14];
	    r[13] -= r[10] * r[14];
	}
	r8vscale(1, m, r[1], &v[0]);
	r8vlinkt(n+1, 1, m, r[2], &v[0], &v[0]);
	r8vlinkt(n + n + 1, 1, m, r[3], &v[0], &v[0]);
	i__1 = n + 1;
	r8vscale(i__1, m, r[11], &v[0]);
	r8vlinkt(1, i__1, m, r[10], &v[0], &v[0]);
	i__1 = n + n + 1;
	r8vlinkt(i__1, n+1, m, r[12], &v[0], &v[0]);
	r8vscale(i__1, m, r[15], &v[0]);
	r8vlinkt(1, i__1, m, r[13], &v[0], &v[0]);
	r8vlinkt(n+1, i__1, m, r[14], &v[0], &v[0]);
    }
}

int nshx(int nmax, int nmin, int mmax, int mmin) {
	int ret_val, i__2, i__5, i__6, kmax;

	kmax = mmax + 1;
	i__5 = MIN(nmin,mmin);
	i__6 = MIN(nmin,kmax);
	i__2 = kmax * kmax - mmin * mmin + i__5 * i__5 - i__6 * i__6 + (nmax - mmax - I_DIM(nmin, kmax)) * 
		((mmax << 1) + 1) + (I_DIM(nmin, mmin) - nmax + mmin - 1) * MAX(0, (mmin << 1) - 1);
	ret_val = MAX(0,i__2);
	return ret_val;
}

int nlpx(int nmax, int mmax, int mmin) {
    int mdif;

    mdif = MAX(0, MIN(nmax,mmax) - mmin + 1);
    return(mdif * (mdif + 1) / 2 + mdif * I_DIM(nmax, mmax) + mmin - 1);
}

int i8ssum(int abeg, int alen, int *a) {
    int i, aadr, ret_val;

    /* Parameter adjustments */
    --a;

    ret_val = 0;
    aadr = abeg;
    for (i = 0; i < alen; ++i)
	ret_val += a[aadr++];

    return ret_val;
}

void i8vset(int abeg, int alen, int s, int *a) {
    int i, aadr;

    /* Parameter adjustments */
    --a;

    /* Function Body */
    aadr = abeg;
    for (i = 0; i < alen; ++i)
	a[aadr++] = s;
}

void i8vadd(int abeg, int bbeg, int cbeg, int vlen, int *a, int *b, int *c) {
    int i, aadr, badr, cadr;

    /* Parameter adjustments */
    --c;
    --b;
    --a;

    /* Function Body */
    aadr = abeg;
    badr = bbeg;
    cadr = cbeg;
    for (i = 0; i < vlen; ++i)
	c[cadr++] = b[badr++] + a[aadr++];
}

void i8vadds(int abeg, int bbeg, int vlen, int s, int *a, int *b) {
    int i, aadr, badr;

    /* Parameter adjustments */
    --b;
    --a;

    /* Function Body */
    aadr = abeg;
    badr = bbeg;
    for (i = 0; i < vlen; ++i)
	b[badr++] = a[aadr++] + s;
}

void i8vcum(int abas, int abeg, int alen, int *a) {
    int i, aadr, acur, aprv;

    /* Parameter adjustments */
    --a;

    aprv = a[abeg];
    a[abeg] = abas;
    aadr = abeg + 1;
    for (i = 0; i < alen - 1; ++i) {
	acur = a[aadr];
	a[aadr] = a[aadr - 1] + aprv;
	aprv = acur;
	++aadr;
    }
}

void i8vdel(int abas, int abeg, int alen, int *a) {
    int i, aadr, acur, aprv;

    /* Parameter adjustments */
    --a;

    aprv = abas;
    aadr = abeg;
    for (i = 0; i < alen; ++i) {
	acur = a[aadr];
	a[aadr] = acur - aprv;
	aprv = acur;
	++aadr;
    }
}

void r8vset(int abeg, int alen, double s, double *a) {
    int i, aadr;

    /* Parameter adjustments */
    --a;

    aadr = abeg;
    for (i = 0; i < alen; ++i)
	a[aadr++] = s;
}

double r8sdot(int abeg, int bbeg, int vlen, double *a, double *b) {
    double ret_val;
    int i, aadr, badr;

    /* Parameter adjustments */
    --b;
    --a;

    ret_val = 0.;
    aadr = abeg;
    badr = bbeg;
    for (i = 0; i < vlen; ++i)
	ret_val += a[aadr++] * b[badr++];
 
    return ret_val;
}

double r8ssum_(int *abeg, int *alen, double *a) {
    double ret_val;
    int i, aadr;

    /* Parameter adjustments */
    --a;

    /* Function Body */
    ret_val = 0.;
    aadr = *abeg;
    for (i = 0; i < *alen; ++i) {
	ret_val += a[aadr];
	++aadr;
    }
    return ret_val;
}

void r8slt(int abeg, int alen, double s, double *a, int *j) {
    int i, aadr;

    /* Parameter adjustments */
    --a;

    /* Function Body */
    aadr = abeg;
    for (i = 1; i <= alen; ++i) {
	if (s < a[aadr]) {
	    *j = i - 1;
	    return;
	}
	++aadr;
    }
    *j = alen;
}

void r8vsub(int abeg, int bbeg, int cbeg, int vlen, double *a, double *b, double *c) {
    int i, aadr, badr, cadr;

    /* Parameter adjustments */
    --c;
    --b;
    --a;

    /* Function Body */
    aadr = abeg;
    badr = bbeg;
    cadr = cbeg;
    for (i = 0; i < vlen; ++i)
	c[cadr++] = b[badr++] - a[aadr++];
}

void r8vmul(int abeg, int bbeg, int cbeg, int vlen, double *a, double *b, double *c) {
    int i, aadr, badr, cadr;

    /* Parameter adjustments */
    --c;
    --b;
    --a;

    aadr = abeg;
    badr = bbeg;
    cadr = cbeg;
    for (i = 0; i < vlen; ++i)
	c[cadr++] = b[badr++] * a[aadr++];
}

void r8vscale(int abeg, int alen, double s, double *a) {
    int i, aadr;

    /* Parameter adjustments */
    --a;

    aadr = abeg;
    for (i = 0; i < alen; ++i) {
	a[aadr] = s * a[aadr];
	++aadr;
    }
}

void r8vscats(int qbeg, int qlen, double s, int *q, double *a) {
    int i, qadr;

    /* Parameter adjustments */
    --a;
    --q;

    qadr = qbeg;
    for (i = 0; i < qlen; ++i)
	a[q[qadr++]] = s;

}

void r8vlinkt(int abeg, int bbeg, int vlen, double s, double *a, double *b) {
    int i, aadr, badr;

    /* Parameter adjustments */
    --b;
    --a;

    aadr = abeg;
    badr = bbeg;
    for (i = 0; i < vlen; ++i)
	b[badr++] += s * a[aadr++];
}

void r8vlinkq(int abeg, int bbeg, int cbeg, int vlen, double s, double *a, double *b, double *c) {
    int i, aadr, badr, cadr;

    /* Parameter adjustments */
    --c;
    --b;
    --a;

    aadr = abeg;
    badr = bbeg;
    cadr = cbeg;
    for (i = 0; i < vlen; ++i)
	c[cadr++] += s * a[aadr++] * b[badr++];
}

void r8vgathp(int abeg, int ainc, int bbeg, int blen, double *a, double *b) {
    int i, aadr, badr;

    /* Parameter adjustments */
    --b;	--a;

    aadr = abeg;
    badr = bbeg;
    for (i = 0; i < blen; ++i) {
	b[badr++] = a[aadr];
	aadr += ainc;
    }
}

double d_mod(double x, double y) {
	double quotient;
	if( (quotient = x / y) >= 0)
		quotient = floor(quotient);
	else
		quotient = -floor(-quotient);
	return(x - y * quotient );
}

double pow_di(double ap, int bp) {
	double pow, x;
	int n;
	unsigned long u;

	pow = 1;
	x = ap;
	n = bp;

	if(n != 0) {
		if(n < 0) {
			n = -n;
			x = 1/x;
		}
		for(u = n; ; ) {
			if(u & 01)
				pow *= x;
			if(u >>= 1)
				x *= x;
			else
				break;
		}
	}
	return(pow);
}

int i_dnnt(double x) {
	return (int)(x >= 0. ? floor(x + .5) : -floor(.5 - x));
}
