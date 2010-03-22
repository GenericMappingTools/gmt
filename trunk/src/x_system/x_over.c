/*	$Id: x_over.c,v 1.15 2010-03-22 18:55:47 guru Exp $
 *
 * X_OVER will compute cross-overs between 2 legs (or internal cross-overs
 * if both legs are the same) and write out time,lat,lon,cross-over values,
 * heading_track_1, and heading_track_2 for each crossover. The input file
 * is assumed to be an unformatted, fixed length record file:
 *
 * Rec#1:
 *	YEAR	- (int)		Year the cruise begun (first data point)
 *	NPOINTS	- (int)		Total number of records following rec #1
 *	INFO	- (char)	10 character info field.
 * Rec#2-(NPOINTS+1):
 * 	GMT_TIME	- (int)  	Stored as seconds from 00:00:00 on Jan. 1, YEAR.
 *	LAT	- (float)	Stored as degrees * 1.0E6 in the interval <-90.,+90.>
 *	LON	- (float)	Stored as degrees * 1.0E6 in the interval <0.,360.>
 *	GRAV	- (int)		Gravity in mGal * 10.
 *	MAG	- (int)		Magnetic anomaly, in gamma
 *	BATH	- (int)		Bathymetry, in meters
 *
 * The user may choose from three interpolation schemes to calculate the value
 * at the crossover. The default type is linear interpolation, but by specifying
 * -a or -c at the command line, akima's spline or natural cubic spline will be
 * used, respectively. To avoid computing crossovers where the time separation
 * between two consecutive points are large (i.e. at data gaps), the time "
 * must not exceed a certain value. The default value is 15 minutes, but can be
 * changed by using the option -g<minutes>. To find all crossovers regardless of
 * data gaps, specify a large timegap, e.g. -g99999.
 *
 * Programmer:	Paul Wessel
 * Date:	15-JAN-1987
 * Last rev:	18-FEB-1989
 * Last rev:	3-AUG-1989:		Swaps leg1 and leg2 if leg2 is lexically less than leg1
 * Last rev:	6-MAR-2000:		POSIX
 *
 * Restrictions:
 *	Max 100000 points (sum of legs or leg1 if internal)
 */

#include "gmt.h"
#include "gmt_mgg.h"

#include "x_system.h"

#define NPPS		8
#define NSPB		64
#define NPPB		512
#define MAX_FLOAT	1.0e38
#define MAX_DOUBLE	1.0e100
#define MAXPOINTS	524288	/* 2 * 512 ^ 2 */
#define MAXSECT		100
#define MAXBLOCK	100

#define SWAP(f1,f2) {float tmp; tmp = *f1; *f1 = *f2; *f2 = tmp;}

typedef int HANDLE;

GMT_LONG over_lap (float xx[2][2], float yy[2][2]);
GMT_LONG find_cross (double *xc, double *yc, double *tc, double *dc, float *hc, double *xvalues, GMT_LONG *pt, float xx[2][2], float yy[2][2]);

static char *oformat = "%9.5f %9.5f %10.1f %10.1f %9.2f %9.2f %9.2f %8.1f %8.1f %8.1f %5.1f %5.1f\n";
static int ttime[MAXPOINTS];		/* time (in seconds) at each point */
static int maxgap = 900;		/* Largest time separation between two points */
static int npoints[2];			/* no of points pr leg */
static int offset[2];			/* array index where first point if found */
static short int gmt[MAXPOINTS][3];	/* grav/mag/bath value at each point */
static int int_mode = 0;		/* Type of interpolation (default = linear) */
static int max_window = 1;		/* No of points on each side of crossover to include
  				   	   when interpolating the cross-over value */
static int leg_no[2];			/* leg_no[1] = 0 if internal */
static float lat[MAXPOINTS];		/* latitude for each point */
static float lon[MAXPOINTS];		/* longitude for each point */
				/* Min/max coordinates for pieces of the legs */
static float grid_section[4][MAXSECT+1][MAXBLOCK][2];
GMT_LONG gmt_flag[3] = {FALSE, FALSE, FALSE};		/* TRUE (for each data type) if it exists */

int main (int argc, char *argv[])
{
  char legname[2][10];		/* Names of the legs */
  char legfile[100];		/* Filename for leg */
  char info[11];		/* Info-header from input-file */
  char tmp[10];
  
  GMT_LONG n_pts_pr_blk[2];		/* points pr block for each leg */
  GMT_LONG last_pt[2];		/* no of points in last section */
  GMT_LONG pnt_begin[2];		/* first point to check in current section */
  GMT_LONG pnt_end[2];		/* last point to check in current section */
  GMT_LONG rec, ind, delta, rec_no;	/* misc. counters */
  GMT_LONG pnt[2];

  GMT_LONG nxovers;			/* no of crossovers found */
  GMT_LONG nlegs;			/* 2 if external, 1 if internal */
  int year[2];			/* year of leg */
  GMT_LONG n_pts_pr_sect[2];		/* points pr section for each leg */
  GMT_LONG n_sect_pr_blk[2];		/* sections pr block for each leg */
  GMT_LONG last_blk[2];		/* no of blocks - 1 for each leg */
  GMT_LONG last_sect[2];		/* no of sections - 1 for each leg */
  GMT_LONG blk_begin[2];		/* first block to check for each leg */
  GMT_LONG blk_end[2];		/* last block to check for each leg */
  GMT_LONG sect_begin[2];		/* first sect to check in current block */
  GMT_LONG sect_end[2];		/* last sect to check in current block */
  GMT_LONG latitude, longitude;	/* lat/lon * 1.0E6 from .gmt-file */
  GMT_LONG leg, sct[2], blk[2], sect, ntot = 0;
  GMT_LONG arg, block, mode, i;		/* Misc. counters */

  GMT_LONG shift_lon = FALSE;	/* TRUE if areas cross datumline */
  GMT_LONG internal = FALSE;	/* TRUE if leg1 = leg2 */
  GMT_LONG x_flag;		/* TRUE if crossover is found */
  GMT_LONG error = FALSE;	/* TRUE for invalid arguments */
  GMT_LONG verbose = FALSE;	/* Work in silence */
  GMT_LONG ok;			/* misc. booleans */
  GMT_LONG first = TRUE;		/* Used to print out header before first xover */

  double x_cross, y_cross;	/* Coordinates of crossover */
  double t_old = -1.0e38;	/* Time of previous crossover */
  double t_cross[2];		/* time at crossover along each leg */
  double d_cross[3];		/* Gravity/Magnetic/Bathymetry crossover errors */
  double x_gmb[3];			/* Gravity/Magnetic/Bathymetry mean value at crossover point */

  float xm[2][2];		/* Min/max longitude for area in question */
  float ym[2][2];			/* Same for latitude */
  float x_leg_max[2], x_leg_min[2];	/* Min/max longitudes for leg */
  float y_leg_max[2], y_leg_min[2];	/* Same for latitude */
  float x_blk_max, x_blk_min;		/* Min/max for block */
  float y_blk_max, y_blk_min;
  float x_sect_max, x_sect_min;		/* Min/max for section */
  float y_sect_max, y_sect_min;
  float x_old, y_old;			/* Position of previous crossover */
  float h_cross[2];			/* Heading at crossover along each leg */
  float d_factor[3];			/* Scale factors if applicable */
  float last_lon;			/* Used to check if we cross Greenwich */
  size_t not_used = 0;

  FILE *fp;				/* File pointer */

  d_factor[0] = (float)0.1;			/* Scale mGal*10 to mGal */
  d_factor[1] = d_factor[2] = 1.0;	/* Leave mag/bath untouched */
  leg = 0;
  for (arg = 1; arg < argc; arg++) {
    if (argv[arg][0] == '-') {
      switch (argv[arg][1]) {
        case 'L':	/* Use linear interpolation */
          int_mode = 0;
          break;
        case 'A':	/* Use akima's interpolation */
          int_mode = 1;
          max_window = 3;
          break;
        case 'C':	/* Use natural cubic spline */
          int_mode = 1; /* Not yet implemented, use Akima */
          max_window = 3;
          break;
        case 'N':	/* No of points to use in interpolation */
          max_window = atoi (&argv[arg][2]);
          break;
        case 'W':	/* Get new timegap */
          maxgap = atoi(&argv[arg][2]) * 60;
          if (maxgap == 0) maxgap = 900;	/* 15 minutes default */
          break;
        case 'G':
          d_factor[0] = (float)atof(&argv[arg][2]);
          if (d_factor[0] == (float)0.0) d_factor[0] = (float)0.1;
          break;
        case 'M':
          d_factor[1] = (float)atof(&argv[arg][2]);
          if (d_factor[1] == (float)0.0) d_factor[1] = (float)1.0;
          break;
        case 'T':
          d_factor[2] = (float)atof(&argv[arg][2]);
          if (d_factor[2] == (float)0.0) d_factor[2] = (float)1.0;
          break;
        case 'V':
          verbose = TRUE;
          break;
        default:
          error = TRUE;
          break;
      }
    }
    else {
      strcpy(legname[leg++],argv[arg]);
      if (leg > 2) error = TRUE;
    }
  }
  if (leg == 1) {
    strcpy(legname[1],legname[0]);
    internal = TRUE;
  }
  else if (leg == 2)
    internal = (!strcmp(legname[0],legname[1])) ? TRUE : FALSE;
  else
    error = TRUE;
  
  if (argc == 1 || error) {
    fprintf(stderr,"x_over - find crossovers between gmt-files\n\n");
    fprintf(stderr,"Usage : x_over leg1 [leg2] [options]\n");
    fprintf(stderr,"options: -L           linear interpolation at crossover [Default]\n");
    fprintf(stderr,"         -A           Quasi hermite (Akima) spline interpolation\n");
    fprintf(stderr,"         -C           Natural cubic spline interpolation\n");
    fprintf(stderr,"	     -N<np>       No of points to use when interpolating. [Default = 6]\n");
    fprintf(stderr,"         -W[minutes]   maximum timegap (in min) allowed at crossover [Default=15]\n");
    fprintf(stderr,"         -G[factor]   multiply gravity values by factor [Default = 0.1]\n");
    fprintf(stderr,"         -M[factor]   multiply magnetic values by factor [Default = 1]\n");
    fprintf(stderr,"         -T[factor]   multiply bathymetric values by factor [Default = 1]\n");
    fprintf(stderr,"         -V           Verbose, report no of crossovers found.\n");
    exit (EXIT_FAILURE);
  }

  leg_no[0] = 0;
  if (internal) {	/* Same leg -> Internal crossovers */
    leg_no[1] = 0;
    nlegs = 1;
  }
  else {
    leg_no[1] = 1;
    nlegs = 2;
    if (strcmp (legname[0], legname[1]) > 0) {	/* leg2 is lexically less than leg1, swap */
    	strcpy (tmp, legname[0]);
    	strcpy (legname[0], legname[1]);
    	strcpy (legname[1], tmp);
    }
    
  }

  gmtmggpath_init(GMT_SHAREDIR);

  /* Read data for each leg (only 1 for internal) */

  nxovers = offset[0] = offset[1] = 0;
  for (leg = 0; leg < nlegs; leg++) {
    if (leg == 1) offset[1] = npoints[0];
    if (gmtmggpath_func(legfile,legname[leg])) {
      fprintf(stderr,"x_over : No path for leg %s\n", legname[leg]);
      exit (EXIT_FAILURE);
    }
    if ((fp = fopen(legfile,"rb")) == NULL) {
      fprintf(stderr,"x_over : Could not find/open %s\n",legfile);
      exit (EXIT_FAILURE);
    }
    if (fread((void *)(&year[leg]), (size_t)4, (size_t)1, fp) != (size_t)1) {
      fprintf(stderr,"x_over: Read error 1. record(year)\n");
      exit (EXIT_FAILURE);
    }
    if (fread((void *)(&npoints[leg]), (size_t)4, (size_t)1, fp) != (size_t)1) {
      fprintf(stderr,"x_over: Read error 1. record(npoints)\n");
      exit (EXIT_FAILURE);
    }
    if (fread(info, (size_t)10, (size_t)1, fp) != (size_t)1) {
      fprintf(stderr,"x_over: Read error 1. record(info)\n");
      exit (EXIT_FAILURE);
    }
    ntot += npoints[leg];
    if (ntot > MAXPOINTS) {
      fprintf(stderr,"x_over : ntot = %ld, must allocate more memory\n",ntot);
      exit (EXIT_FAILURE);
    }
    n_pts_pr_sect[leg] = NPPS;
    n_sect_pr_blk[leg] = NSPB;
    n_pts_pr_blk[leg]  = NPPB;
    if (npoints[leg]/n_pts_pr_blk[leg] < 1) {
      n_pts_pr_blk[leg] /= 10;
      n_sect_pr_blk[leg] /= 10;
    }
    last_blk[leg] = npoints[leg]/n_pts_pr_blk[leg];
    last_sect[leg] = (npoints[leg]%n_pts_pr_blk[leg])/n_pts_pr_sect[leg];
    last_pt[leg] = npoints[leg] - last_blk[leg]*n_pts_pr_blk[leg]
    		   - last_sect[leg]*n_pts_pr_sect[leg];
    not_used = fread((void *)(&ttime[offset[leg]]), (size_t)4, (size_t)1, fp);
    not_used = fread((void *)(&latitude), (size_t)4, (size_t)1, fp);
    not_used = fread((void *)(&longitude), (size_t)4, (size_t)1, fp);
    lat[offset[leg]] = (float)(latitude *0.000001);
    lon[offset[leg]] = (float)(longitude *0.000001);
    /* Make sure geodetic longitudes are used */
    if (lon[offset[leg]] < 0.) lon[offset[leg]] += 360.0;
    not_used = fread((void *)gmt[offset[leg]], (size_t)2, (size_t)3, fp);

    last_lon = lon[offset[leg]];
    for (rec_no = offset[leg] + 1; rec_no < (offset[leg] + npoints[leg]); rec_no++) {
      not_used = fread((void *)(&ttime[rec_no]), (size_t)4, (size_t)1, fp);
      not_used = fread((void *)(&latitude), (size_t)4, (size_t)1, fp);
      not_used = fread((void *)(&longitude), (size_t)4, (size_t)1, fp);
      lat[rec_no] = (float)(latitude *0.000001);
      lon[rec_no] = (float)(longitude *0.000001);
      if (lon[rec_no] < 0.) lon[rec_no] += 360.0;
      not_used = fread((void *)gmt[rec_no], (size_t)2, (size_t)3, fp);
      for (i = 0; i < 3; i++)
        if (!gmt_flag[i] && gmt[rec_no][i] != NODATA) gmt_flag[i] = TRUE;
      if (fabs(lon[rec_no] - last_lon) > 180.0 ) shift_lon = TRUE;
      last_lon = lon[rec_no];
    }
    fclose(fp);
  }

  /* Divide data into blocks and sections and find min/max coordinates for each piece */

  rec = 0;
  for (leg = 0; leg < nlegs; leg++) {
    x_leg_max[leg] = x_sect_max = y_leg_max[leg] = y_sect_max = -100000.0;
    x_leg_min[leg] = x_sect_min = y_leg_min[leg] = y_sect_min =  100000.0;
    rec_no = 0;
    for (block = 0; block <= last_blk[leg]; block++) {
      x_blk_max = y_blk_max = -100000.0;
      x_blk_min = y_blk_min =  100000.0;
      sect = 0;
      while (rec_no < npoints[leg] && sect < n_sect_pr_blk[leg]) {
        if (lon[rec] > 180.0 && shift_lon) lon[rec] -= 360.0;
        if (lat[rec] > y_sect_max) y_sect_max = lat[rec];
        if (lat[rec] < y_sect_min) y_sect_min = lat[rec];
        if (lon[rec] > x_sect_max) x_sect_max = lon[rec];
        if (lon[rec] < x_sect_min) x_sect_min = lon[rec];
        if ((rec_no && !(rec_no%n_pts_pr_sect[leg])) || rec_no == (npoints[leg]-1)) {
          if (x_sect_max > x_blk_max) x_blk_max = x_sect_max;
          if (x_sect_min < x_blk_min) x_blk_min = x_sect_min;
          if (y_sect_max > y_blk_max) y_blk_max = y_sect_max;
          if (y_sect_min < y_blk_min) y_blk_min = y_sect_min;
          /* Now, get the min/max coordinates within this section
           * of the ships track */
          grid_section[0][sect][block][leg] = x_sect_min;
          grid_section[1][sect][block][leg] = x_sect_max;
          grid_section[2][sect][block][leg] = y_sect_min;
          grid_section[3][sect][block][leg] = y_sect_max;
          x_sect_min = x_sect_max = lon[rec];
          y_sect_min = y_sect_max = lat[rec];
          sect++;
        }
        rec_no++;
        rec++;
      } /* end of section */
      /* store min/max coordinates for this block */
      grid_section[0][n_sect_pr_blk[leg]][block][leg] = x_blk_min;
      grid_section[1][n_sect_pr_blk[leg]][block][leg] = x_blk_max;
      grid_section[2][n_sect_pr_blk[leg]][block][leg] = y_blk_min;
      grid_section[3][n_sect_pr_blk[leg]][block][leg] = y_blk_max;
      if (x_blk_min < x_leg_min[leg]) x_leg_min[leg] = x_blk_min;
      if (x_blk_max > x_leg_max[leg]) x_leg_max[leg] = x_blk_max;
      if (y_blk_min < y_leg_min[leg]) y_leg_min[leg] = y_blk_min;
      if (y_blk_max > y_leg_max[leg]) y_leg_max[leg] = y_blk_max;
    } /* end of block */
  } /* end of leg */

  /* Here, all data is read and stored in arrays. The min/max coordinates
   * for the legs, blocks, and sections have been found and stored in
   * grid_section[....]. Let's find some cross-overs. First, see if the
   * two legs (if external) have some area in common.
   */
  x_old = lon[0];
  y_old = lat[0];
  for (leg = 0; leg < 2; leg++) {
    blk_begin[leg] = 0;
    blk_end[leg] = last_blk[leg_no[leg]];
    xm[0][leg] = x_leg_min[leg_no[leg]];
    xm[1][leg] = x_leg_max[leg_no[leg]];
    ym[0][leg] = y_leg_min[leg_no[leg]];
    ym[1][leg] = y_leg_max[leg_no[leg]];
    if (xm[0][leg] > xm[1][leg]) xm[0][leg] = xm[1][leg];
    if (ym[0][leg] > ym[1][leg]) ym[0][leg] = ym[1][leg];
  }
  if (internal || over_lap(xm,ym)) {		/* Yes, the legs do overlap, check further */
    xm[0][1] = MAX(xm[0][0], xm[0][1]);
    xm[1][1] = MIN(xm[1][0], xm[1][1]);
    ym[0][1] = MAX(ym[0][0], ym[0][1]);
    ym[1][1] = MIN(ym[1][0], ym[1][1]);
    if ((xm[1][1]-xm[0][1]) > 180.0) {	/* Area crosses both Greenwich and datumline */
      fprintf(stderr,"x_over: Legs %s and %s have more than 180 degrees longitude in common! Aborts\n",
        legname[0], legname[leg_no[1]]);
      exit (EXIT_FAILURE);
    }
    for (leg = 0; !internal && leg < 2; leg++) {
      for (mode = 0; mode < 2; mode++) {
        ind = (mode) ? blk_begin[leg] : blk_end[leg];
        ok = TRUE;
        while (ok) {
          xm[0][0] = grid_section[0][n_sect_pr_blk[leg]][ind][leg];
          xm[1][0] = grid_section[1][n_sect_pr_blk[leg]][ind][leg];
          ym[0][0] = grid_section[2][n_sect_pr_blk[leg]][ind][leg];
          ym[1][0] = grid_section[3][n_sect_pr_blk[leg]][ind][leg];
          if (over_lap(xm,ym))
            ok = FALSE;
          else if (mode && ind < blk_end[leg])
            ind++;
          else if (!mode && ind > blk_begin[leg])
            ind--;
          else
            ok = FALSE;
        }
        if (mode)
          blk_begin[leg] = ind;
        else
          blk_end[leg] = ind;
      }
    }

    /* - - - - - - - - - - - - - - - -
     *   L E V E L 1 :  B L O C K S
     * - - - - - - - - - - - - - - - -
     * Start comparing two blocks, one from each leg
     */

    for (blk[0] = blk_begin[0]; blk[0] <= blk_end[0]; blk[0]++) {
      for (blk[1] = (internal) ? blk[0] : blk_begin[1]; blk[1] <= blk_end[1]; blk[1]++) {
        /* Now, check if these two blocks overlap */
        for (leg = 0; leg < 2; leg++) {
          xm[0][leg] = grid_section[0][n_sect_pr_blk[leg_no[leg]]][blk[leg]][leg_no[leg]];
          xm[1][leg] = grid_section[1][n_sect_pr_blk[leg_no[leg]]][blk[leg]][leg_no[leg]];
          ym[0][leg] = grid_section[2][n_sect_pr_blk[leg_no[leg]]][blk[leg]][leg_no[leg]];
          ym[1][leg] = grid_section[3][n_sect_pr_blk[leg_no[leg]]][blk[leg]][leg_no[leg]];
          if (xm[0][leg] > xm[1][leg]) xm[0][leg] = xm[1][leg];
          if (ym[0][leg] > ym[1][leg]) ym[0][leg] = ym[1][leg];
        }
        if (over_lap(xm,ym)) {
          /* - - - - - - - - - - - - - - - - - -
           *   L E V E L 2 :  S E C T I O N S
           * - - - - - - - - - - - - - - - - - -
           * Since the blocks have area in common, the next step is to look for
           * overlap between sections within those blocks. For internal crossovers
           * this will always be TRUE, obviously.
           */
          for (leg = 0; leg < 2; leg++) {
            sect_begin[leg] = 0;
            sect_end[leg] = (blk[leg] == last_blk[leg_no[leg]]) ? last_sect[leg_no[leg]] : n_sect_pr_blk[leg_no[leg]] - 1;
          }
          /* Find the area these two blocks have in common, unless
           * we're looking for internal crossovers, where blk1 = blk2
           */
          xm[0][1] = MAX(xm[0][0], xm[0][1]);
          xm[1][1] = MIN(xm[1][0], xm[1][1]);
          ym[0][1] = MAX(ym[0][0], ym[0][1]);
          ym[1][1] = MIN(ym[1][0], ym[1][1]);
          if (!internal) {
            for (leg = 0; leg < 2; leg++) {
              for (mode = 0; mode < 2; mode++) {
        	ind = (mode) ? sect_begin[leg] : sect_end[leg];
        	ok = TRUE;
        	while (ok) {
          	  xm[0][0] = grid_section[0][ind][blk[leg]][leg];
          	  xm[1][0] = grid_section[1][ind][blk[leg]][leg];
          	  ym[0][0] = grid_section[2][ind][blk[leg]][leg];
          	  ym[1][0] = grid_section[3][ind][blk[leg]][leg];
          	  if (over_lap(xm,ym))
            	    ok = FALSE;
          	  else if (mode && ind < sect_end[leg])
              	    ind++;
       	          else if (!mode && ind > sect_begin[leg])
              	    ind--;
              	  else
              	    ok = FALSE;
        	}
        	if (mode)
          	  sect_begin[leg] = ind;
        	else
          	  sect_end[leg] = ind;
     	      }
    	    }
    	  }
    	  /* Start comparing the necessary sections */
    	  for (sct[0] = sect_begin[0]; sct[0] <= sect_end[0]; sct[0]++) {
   	    if (internal && blk[0] == blk[1]) sect_begin[1] = sct[0];
    	    for (sct[1] = sect_begin[1]; sct[1] <= sect_end[1]; sct[1]++) {
    	      for (leg = 0; leg < 2; leg++) {
    	        xm[0][leg] = grid_section[0][sct[leg]][blk[leg]][leg_no[leg]];
    	        xm[1][leg] = grid_section[1][sct[leg]][blk[leg]][leg_no[leg]];
    	        ym[0][leg] = grid_section[2][sct[leg]][blk[leg]][leg_no[leg]];
    	        ym[1][leg] = grid_section[3][sct[leg]][blk[leg]][leg_no[leg]];
          	if (xm[0][leg] > xm[1][leg]) xm[0][leg] = xm[1][leg];
          	if (ym[0][leg] > ym[1][leg]) ym[0][leg] = ym[1][leg];
              }
              if (over_lap(xm,ym)) {
		/* - - - - - - - - - - - - - -
		 *  L E V E L 3 :  P O I N T S
		 * - - - - - - - - - - - - - -
          	 * Here we have overlap between two sections of the ships
          	 * track. We must now look at pairs of adjacent points to
          	 * see if we indeed have a crossover. Note that if internal
          	 * this test will always be TRUE.
          	 */
	        for (leg = 0; leg < 2; leg++) {
		  if (blk[leg] == last_blk[leg_no[leg]] && sct[leg] == last_sect[leg_no[leg]])
		    pnt_end[leg] = npoints[leg_no[leg]] + offset[leg] - 1;
		  else
		    pnt_end[leg] = blk[leg]*n_pts_pr_blk[leg_no[leg]]
		      + sct[leg]*n_pts_pr_sect[leg_no[leg]] + n_pts_pr_sect[leg_no[leg]]
		      + offset[leg] - 1;
		  pnt_begin[leg] = blk[leg]*n_pts_pr_blk[leg_no[leg]]
		    + sct[leg]*n_pts_pr_sect[leg_no[leg]] + offset[leg];
		}
		xm[0][1] = MAX(xm[0][0],xm[0][1]);
		xm[1][1] = MIN(xm[1][0],xm[1][1]);
		ym[0][1] = MAX(ym[0][0],ym[0][1]);
		ym[1][1] = MIN(ym[1][0],ym[1][1]);
		if (!(blk[0] == blk[1] && sct[0] == sct[1] && internal)) {
		  /* Find the first/last point/sect/block of overlap */
		  for (leg = 0; leg < 2; leg++) {
		    for (mode = 0; mode < 2; mode++) {
		      ind = (mode) ? pnt_begin[leg] : pnt_end[leg];
		      ok = TRUE;
		      while (ok) {
		        xm[0][0] = lon[ind];
		        xm[1][0] = lon[ind+1];
		        ym[0][0] = lat[ind];
		        ym[1][0] = lat[ind+1];
		        if (xm[0][0] > xm[1][0]) SWAP(&xm[0][0],&xm[1][0]);
		        if (ym[0][0] > ym[1][0]) SWAP(&ym[0][0],&ym[1][0]);
		        if (over_lap(xm,ym))
		          ok = FALSE;
		        else if (mode && ind < (pnt_end[leg]-1))
		          ind++;
		        else if (!mode && ind > pnt_begin[leg])
		          ind--;
		        else
		          ok = FALSE;
		      }
		      if (mode)
		        pnt_begin[leg] = ind;
		      else
		        pnt_end[leg] = ind;
		    }
		  }
		}
		/* Start comparing pairs of points from each section */
		for (pnt[0] = pnt_begin[0]; pnt[0] <= pnt_end[0]; pnt[0]++) {
		  if (blk[0] == blk[1] && sct[0] == sct[1] && internal)
		    pnt_begin[1] = pnt[0] + 1;
		  xm[0][0] = lon[pnt[0]];
		  xm[1][0] = lon[pnt[0]+1];
		  ym[0][0] = lat[pnt[0]];
		  ym[1][0] = lat[pnt[0]+1];
		  if (xm[0][0] > xm[1][0]) SWAP(&xm[0][0],&xm[1][0]);
		  if (ym[0][0] > ym[1][0]) SWAP(&ym[0][0],&ym[1][0]);
		  for (pnt[1] = pnt_begin[1]; pnt[1] <= pnt_end[1]; pnt[1]++) {
		    xm[0][1] = lon[pnt[1]];
		    xm[1][1] = lon[pnt[1]+1];
		    ym[0][1] = lat[pnt[1]];
		    ym[1][1] = lat[pnt[1]+1];
		    if (xm[0][1] > xm[1][1]) SWAP(&xm[0][1],&xm[1][1]);
		    if (ym[0][1] > ym[1][1]) SWAP(&ym[0][1],&ym[1][1]);
		    delta = GMT_abs(pnt[0] - pnt[1]);
		    if (over_lap(xm,ym) && delta > 1) {
		      /* Here we have found a possible crossover. We interpolate to
		       * find the crossover values for position and time along each
		       * track, and check if the crossover position is inside the
		       * area defined by the 4 points. If so we have a crossover.
		       */
		      x_flag = find_cross(&x_cross,&y_cross,t_cross,d_cross,h_cross,x_gmb,pnt,xm,ym);
		      if (x_flag && !((float)x_cross == x_old && (float)y_cross == y_old) && t_cross[0] != t_old) {
		        /* We have found a genuine crossover */
		        nxovers++;
		        x_old = (float)x_cross;
		        y_old = (float)y_cross;
		        t_old = t_cross[0];
		        if (x_cross < 0.0) x_cross += 360.0;
		        for (i = 0; i < 3; i++) {
		          if (d_cross[i] != NODATA) d_cross[i] *= d_factor[i];
		          if (x_gmb[i] != NODATA) x_gmb[i] *= d_factor[i];
		        }
		        if (first) {
			  /* Print out legnames and start-years */
			  printf("%s %d %s %d\n", legname[0], year[0], legname[leg_no[1]], year[leg_no[1]]);
			  first = FALSE;
			}
		        printf(oformat,y_cross,x_cross,t_cross[0],t_cross[1],d_cross[0],d_cross[1],d_cross[2],
		          x_gmb[0],x_gmb[1],x_gmb[2],h_cross[0],h_cross[1]);
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
  if (verbose)
    fprintf(stderr,"x_over: Found %5ld cross-overs between %s and %s\n",nxovers,legname[0],legname[1]);
  exit (EXIT_SUCCESS);
}

GMT_LONG find_cross (double *xc, double *yc, double *tc, double *dc, float *hc, double *xvalues, GMT_LONG *pt, float xx[2][2], float yy[2][2])
{
  double delx[2], dely[2], grad[2], xc0, t_next;
  double delt[2], dx[2][3];
  double deg_pr_rad = 57.29577951308232087680;
  double ti[6], di[6];
  GMT_LONG leg, i, j, no_cross, n_int, dtype, i1, i2, n_left, n_right, first, error = 0;
  for (leg = 0; leg < 2; leg++) {	/* Copy pertinent info */
    if (pt[leg] == (npoints[leg_no[leg]] + offset[leg] - 1)) return (FALSE);
    delx[leg] = (double)lon[pt[leg]+1] - (double)lon[pt[leg]];
    dely[leg] = (double)lat[pt[leg]+1] - (double)lat[pt[leg]];
    delt[leg] = (double)ttime[pt[leg]+1] - (double)ttime[pt[leg]];
    grad[leg] = (delx[leg] == 0.0) ? grad[leg] = MAX_DOUBLE : dely[leg]/delx[leg];
  }
  /* grad = MAX_DOUBLE means that line due N-S */
  if (delt[0] > maxgap || delt[1] > maxgap) return (FALSE);	/* Data gap */

  /* Find xcross and ycross */

  if (grad[0] == grad[1])
    *xc = MAX_DOUBLE;
  else if (grad[0] == MAX_DOUBLE)
    *xc = (double) lon[pt[0]];
  else if (grad[1] == MAX_DOUBLE)
    *xc = (double) lon[pt[1]];
  else {
    xc0 = (double)lat[pt[1]] - (double)lat[pt[0]] + grad[0]*(double)lon[pt[0]]
          - grad[1]*(double)lon[pt[1]];
    *xc = xc0/(grad[0]-grad[1]);
  }
  if (grad[0] == grad[1])
    *yc = MAX_DOUBLE;
  else if (grad[0] == MAX_DOUBLE)
    *yc = (double)lat[pt[1]] + grad[1]*(*xc - (double)lon[pt[1]]);
  else
    *yc = (double)lat[pt[0]] + grad[0]*(*xc - (double)lon[pt[0]]);

  /* Check if xcross,ycross is inside the xm,ym area */

  if ((*xc) < xx[0][0] || (*xc) > xx[1][0] || (*xc) < xx[0][1] || (*xc) > xx[1][1]) return (FALSE); /* Outside box */
  if ((*yc) < yy[0][0] || (*yc) > yy[1][0] || (*yc) < yy[0][1] || (*yc) > yy[1][1]) return (FALSE);

  /* if the crossover coincides with one of the input points, we might end up computing this
   * crossover twice. To avoid duplicates of this kind, allow such crossovers to occur only
   * at the second point (timewise)
   */

  if (((*xc) == (double)lon[pt[0]+1] && (*yc) == (double)lat[pt[0]+1]) ||
    ((*xc) == (double)lon[pt[1]+1] && (*yc) == (double)lat[pt[1]+1])) return (FALSE);

  /* Compute time, heading + crossover value */

  for (leg = 0; leg < 2; leg++) {

    /* First, let us compute the crossover time along this leg */

    if (delx[leg] != 0.0)
      tc[leg] = (double)ttime[pt[leg]] + (delt[leg]/delx[leg])*(*xc-(double)lon[pt[leg]]);
    else if (dely[leg] != 0.0)
      tc[leg] = (double)ttime[pt[leg]] + (delt[leg]/dely[leg])*(*yc-(double)lat[pt[leg]]);
    else
      tc[leg] = (double)ttime[pt[leg]];

    /* Now, lets find the data value at the crossover point on this leg */

    /* Here we try to find up to 3 points on each side of the crossover, so
     * that a maximum of 6 points will be used for the computation of spline 
     * coefficients. This must be done for all the 3 data-types.
     */
    dx[leg][0] = dx[leg][1] = dx[leg][2] = NODATA;
    for (dtype = 0; dtype < 3; dtype++) {
      if (!gmt_flag[dtype]) continue;	/* Skip if no such data */
      /* First we look to the 'left' */
      i1 = pt[leg];
      t_next = tc[leg];
      n_left = 0;
      first = -1;
      while (i1 >= offset[leg] && n_left < max_window && (t_next - ttime[i1]) <= maxgap) {
        if (gmt[i1][dtype] != NODATA) {
          t_next = ttime[i1];
          if (first < 0) first = i1;
          n_left++;
        }
        i1--;
      }
      if (n_left == 0) continue;
      /* Then we look to the 'right' */
      i2 = pt[leg] + 1;
      n_right = 0;
      t_next = ttime[first];
      while (i2 < (npoints[leg_no[leg]]+offset[leg]) && n_right < max_window && (ttime[i2] - t_next) <= maxgap) {
        if (gmt[i2][dtype] != NODATA) {
          t_next = ttime[i2];
          n_right++;
        }
        i2++;
      }
      if (n_right == 0) continue;
      n_int = n_right + n_left;
      for (i = i1+1, j = 0; j < n_int; i++) {
        if (gmt[i][dtype] != NODATA) {
          ti[j] = ttime[i];
          di[j] = gmt[i][dtype];
          j++;
        }
      }

      error = GMT_intpol (ti, di, n_int, (GMT_LONG)1, &tc[leg], &(dx[leg][dtype]), int_mode);	/* Do the interpolation */

      if (error != 0) {	/* Oh shit, what could this mean... */
        fprintf(stderr,"x_over : Error = %ld returned from intpol\n",error);
        fprintf(stderr,"(pnt[0] = %ld, pnt[1] = %ld\n", pt[0],pt[1]);
        return (FALSE);
      }
    }
    
    /* Finally, lets evaluate the heading at the crossover-point along this leg */

    delx[leg] *= cos(0.5*(lat[pt[leg]+1] + lat[pt[leg]])/deg_pr_rad);
    if (delx[leg] == 0.0)
      hc[leg] = (float)((dely[leg] > 0.0) ? 0.0 : 180.0);
    else {
      hc[leg] = (float)(90.0 - atan2(dely[leg],delx[leg])*deg_pr_rad);
      if (hc[leg] < 0.0) hc[leg] += 360.0;
    }
  }
    
  /* Then compute the cross-over values */
    
  no_cross = 0;
  for (dtype = 0; dtype < 3; dtype++) {
    if (dx[0][dtype] == NODATA || dx[1][dtype] == NODATA) {
      no_cross++;
      dc[dtype] = NODATA;
      xvalues[dtype] = NODATA;
    }
    else {
      xvalues[dtype] = 0.5*(dx[0][dtype] + dx[1][dtype]);
      dc[dtype] = dx[0][dtype] - dx[1][dtype];
    }
  }
  if (no_cross == 3)
    return(FALSE);
  else
    return (TRUE);
}

GMT_LONG over_lap (float xx[2][2], float yy[2][2])		/* Checks if the two areas overlap */
{
  if (xx[1][0] < xx[0][1] || xx[0][0] > xx[1][1]) return (FALSE);
  if (yy[1][0] < yy[0][1] || yy[0][0] > yy[1][1]) return (FALSE);
  return (TRUE);
}
