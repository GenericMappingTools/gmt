/*-----------------------------------------------------------------
 *	x_systemid = "@(#)x_system.h	2.8  03/07/00"
 *
 * xsystem.h contains the declaration for the LEG and XOVER structures
 * used in the XSYSTEM programs
 *
 * Author:	Paul Wessel
 * Date:	18-FEB-1989
 * Modified:	4-FEB-1991  Padded XOVER structure to 40 bytes (38 caused problems
 *				on SUN-4s
 * Version:	1.3
 *
 */
 
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

struct LEG {	/* Structure with info about one leg */
	char name[10];			/* Name of leg */
	char agency[10];		/* Collecting agency */
	int year;			/* Year the leg started */
	int n_x_int;			/* Total number of internal cross-over points */
	int n_x_ext;			/* Total number of external cross-over points */
	int n_gmtint[3];		/* Number of internal gravity/magnetisc/topography crossovers */
	int n_gmtext[3];		/* Number of external gravity/magnetisc/topography crossovers */
	double mean_gmtint[3];		/* Mean gravity/magnetics/topography internal xover value */
	double mean_gmtext[3];		/* Mean gravity/magnetics/topography esternal xover value */
	double st_dev_gmtint[3];	/* St. Dev. of the internal gravity/magnetics/topography crossovers */
	double st_dev_gmtext[3];	/* Same for external xovers */
	double dc_shift_gmt[3];		/* Best fitting d.c.-shift for gravity/magnetics/topography */
	double drift_rate_gmt[3];	/* Best fitting drift rate for gravity/magnetics/topography */
	struct LEG *next_leg;		/* Pointer to next leg in list */
};

struct XOVERS {	/* Structure with info on one cross-over error */
	int lat;			/* Latitude * 1.0E6 */
	int lon;			/* Longitude * 1.0E6 */
	int xtime[2];			/* Time at cross-over along track 1 and 2 in 1/10 seconds */
	float x_val[3];			/* Gravity/Magnetics/Topography cross-over values */
	short int gmt[3];		/* Average gravity/magnetics/topography value at crossover */
	short xhead[2];			/* Heading at cross-over along track 1 and 2*/
	short id_no;			/* Currently unused */
};

struct CORR {	/* Structure with the corrections for each leg */
	char name[10];			/* Name of leg */
	short int year;			/* Year the leg started */
	float dc_shift_gmt[3];		/* Best fitting d.c.-shift for gravity, magnetics, and topo */
	float drift_rate_gmt[3];	/* Best fitting drift-rate for gravity, magnetics, and topo */
};

#define REC_SIZE 40	/* Rec size for xx_base.b file xover-records and struct XOVERS */
#define NODATA (-32000)

