/*-------------------------------------------------------------------------
 *	$Id: mgd77.h,v 1.112 2009-07-02 01:54:53 guru Exp $
 * 
 *    Copyright (c) 2005-2009 by P. Wessel
 *    See README file for copying and redistribution conditions.
 *
 *  File:	mgd77.h
 *
 *  Include file for programs that plan to read/write MGD77[+] files
 *
 *  Authors:    Paul Wessel, Primary Investigator, SOEST, U. of Hawaii
 *				Michael Chandler, Ph.D. Student, SOEST, U. of Hawaii
 *		
 *  Version:	1.3
 *  Revised:	15-MAR-2006
 * 
 *-------------------------------------------------------------------------*/

#ifndef _MGD77_H
#define _MGD77_H

#include "gmt.h"

#define MGD77_VERSION		"1.3"		/* Current version of MGD77 supplement */
#define MGD77_CDF_VERSION	"2006.04.15"	/* Current version of MGD77+ files created */
#define MGD77_RECORD_LENGTH	120		/* Length of MGD77 ASCII data records */
#define MGD77_HEADER_LENGTH	80		/* Length of MGD77 ASCII header records */
#define MGD77_N_HEADER_RECORDS	24		/* Number of MGD77 header records */
#define MGD77_METERS_PER_NM     1852		/* meters per nautical mile */
#define MGD77_METERS_PER_M      1609.344	/* meters per statute mile */
#define MGD77_OLDEST_YY		39		/* Y2K says YY < 39 is 20YY */
#define MGD77_N_DATA_FIELDS	27		/* Original 27 data columns in MGD77 */
#define MGD77_N_DATA_EXTENDED	28		/* The 27 plus time */
#define MGD77_N_NUMBER_FIELDS	24		/* Original 24 numerical data columns in MGD77 */
#define MGD77_N_STRING_FIELDS	3		/* Original 3 text data columns in MGD77 */
#define MGD77_N_HEADER_ITEMS	72		/* Number of individual header items in the MGD77 header */
#define MGD77_N_MAG_RF		18		/* Number of different Mag ref fields so far in MGD77 docs */
/* The 28 MGD77 standard types (27 original + 1 conglomerate (time)) */
#define MGD77_RECTYPE		0
#define MGD77_TZ		1
#define MGD77_YEAR		2
#define MGD77_MONTH		3
#define MGD77_DAY		4
#define MGD77_HOUR		5
#define MGD77_MIN		6
#define MGD77_LATITUDE		7
#define MGD77_LONGITUDE		8
#define MGD77_PTC		9
#define MGD77_TWT		10
#define MGD77_DEPTH		11
#define MGD77_BCC		12
#define MGD77_BTC		13
#define MGD77_MTF1		14
#define MGD77_MTF2		15
#define MGD77_MAG		16
#define MGD77_MSENS		17
#define MGD77_DIUR		18
#define MGD77_MSD		19
#define MGD77_GOBS		20
#define MGD77_EOT		21
#define MGD77_FAA		22
#define MGD77_NQC		23
#define MGD77_ID		24
#define MGD77_SLN		25
#define MGD77_SSPN		26
#define MGD77_TIME		27

#define ALL_NINES               "9999999999"	/* Typical text value meaning no-data */
#define ALL_BLANKS "                      "	/* 32 blanks */
#define MGD77_NOT_SET		(-1)

#define MGD77_RESET_CONSTRAINT	1
#define MGD77_RESET_EXACT	2
#define MGD77_SET_ALLEXACT	4

#define MGD77_N_FORMATS		3
#define MGD77_FORMAT_M77	0
#define MGD77_FORMAT_CDF	1
#define MGD77_FORMAT_TBL	2
#define MGD77_FORMAT_ANY	3

#define MGD77_READ_MODE		0
#define MGD77_WRITE_MODE	1

#define MGD77_N_SETS		2
#define MGD77_M77_SET		0
#define MGD77_CDF_SET		1
#define MGD77_SET_COLS		32
#define MGD77_MAX_COLS		64

#define MGD77_FROM_HEADER	1
#define MGD77_TO_HEADER		2

#define MGD77_IGRF_F		0
#define MGD77_IGRF_H		1
#define MGD77_IGRF_X		2
#define MGD77_IGRF_Y		3
#define MGD77_IGRF_Z		4
#define MGD77_IGRF_D		5
#define MGD77_IGRF_I		6

#define MGD77_IGF_HEISKANEN	1
#define MGD77_IGF_1930		2
#define MGD77_IGF_1967		3
#define MGD77_IGF_1980		4

#define MGD77_IGRF_LAST_ID	20	/* IGRF fields IDs in MGD77 are 3,4,11-MGD77_IGRF_LAST_ID.  Update when more fields are added */

#define TWT_PDR_WRAP	10.0						/* The 10 second PDR wrap-around we see in SIO cruises */
#define TWT_PDR_WRAP_TRIGGER	0.5 * TWT_PDR_WRAP	/* Any jump in TWT that exceeds this triggers a wrap */

#define GMT_IMG_MINLAT -72.0059773539
#define GMT_IMG_MAXLAT +72.0059773539

#define MGD77_BAD_HEADER_RECNO		-1
#define MGD77_BAD_HEADER_ITEM		-2

#define MGD77_ORIG			0
#define MGD77_REVISED			1

/* Return error numbers */

#define MGD77_NO_ERROR			0
#define MGD77_FILE_NOT_FOUND		1
#define MGD77_ERROR_OPEN_FILE		2
#define MGD77_NO_HEADER_REC		3
#define MGD77_ERROR_READ_HEADER_ASC	4
#define MGD77_ERROR_WRITE_HEADER_ASC	5
#define MGD77_ERROR_READ_ASC_DATA	6
#define MGD77_ERROR_WRITE_ASC_DATA	7
#define MGD77_WRONG_HEADER_REC		8
#define MGD77_NO_DATA_REC		9
#define MGD77_WRONG_DATA_REC_LEN	10
#define MGD77_ERROR_CONV_DATA_REC	11
#define MGD77_ERROR_READ_HEADER_BIN	12
#define MGD77_ERROR_WRITE_HEADER_BIN	13
#define MGD77_ERROR_READ_BIN_DATA	14
#define MGD77_ERROR_WRITE_BIN_DATA	15
#define MGD77_ERROR_NOT_MGD77PLUS	16
#define MGD77_UNKNOWN_FORMAT		17
#define MGD77_UNKNOWN_MODE		18
#define MGD77_ERROR_NOSUCHCOLUMN	19
#define MGD77_BAD_ARG			20
#define MGD77_BAD_IGRFDATE		21

/* For mgd77+ files */

#define NCPOS_TIME	0
#define NCPOS_LAT	3
#define NCPOS_LON	4
#define NCPOS_TWT	6
#define NCPOS_MTF1	10
#define NCPOS_GOBS	16
#define NCPOS_EOT	21

/* We will use bit flags to keep track of which data column we are referring to.
 * field 0 is rightmost bit (1), field 1 is the next bit (2), field 2 is 4 and
 * so on for powers of 2.  We add these bit powers together to get the bit patterns
 * for a selection of columns.  E.g., field 0, 3, 5 has pattern 1 + 8 + 32 = 41.
 * Some combinations of columns have precalculated bit patterns given below.
 * Note: all this only applies to the MGD77_DATA_RECORD structure & number positions.
 */

#define MGD77_TWT_BIT		(1 << 10)
#define MGD77_DEPTH_BIT		(1 << 11)
#define MGD77_MTF1_BIT		(1 << 14)
#define MGD77_MTF2_BIT		(1 << 15)
#define MGD77_MAG_BIT		(1 << 16)
#define MGD77_GOBS_BIT		(1 << 20)
#define MGD77_FAA_BIT		(1 << 22)
#define MGD77_GEOPHYSICAL_BITS	5360640	/* 0101 0001 1100 1100 0000 0000 */
#define MGD77_CORRECTION_BITS	2883584	/* 0010 1100 0000 0000 0000 0000 */
#define MGD77_TIME_BITS		    124	    /* 0000 0000 0000 0000 0111 1100 */
#define MGD77_FLOAT_BITS	(MGD77_GEOPHYSICAL_BITS + MGD77_CORRECTION_BITS + MGD77_TIME_BITS)
#define MGD77_STRING_BITS	(16777216+33554432+67108864)

/* Codes for the logical tests */

#define MGD77_EQ		1
#define MGD77_LT		2
#define MGD77_LE		3
#define MGD77_GT		4
#define MGD77_GE		5
#define MGD77_BIT		6
#define MGD77_NEQ		8


typedef char byte;	/* Used to indicate 1-byte long integer */
typedef char* Text;	/* Used to indicate character strings */
 
/* The MGD77 File format contains a header section consisting a set of 24 records
 * of length 80 characters each.  This information can be read and stored internally
 * in the structure MGD77_HEADER.  The two i/o functions MGD77_read_header and
 * MGD77_write_header will do exactly what they say.
 */

#define MGD77_COL_ABBREV_LEN	16
#define MGD77_COL_NAME_LEN	64
#define MGD77_COL_UNIT_LEN	64
#define MGD77_COL_COMMENT_LEN	128

#define MGD77_COL_ADJ_TWT	1	/* Undo twt PDR wraps given PDR_wrap value */
#define MGD77_COL_ADJ_DEPTH	2	/* Compute Carter depth from twt */
#define MGD77_COL_ADJ_MAG	3	/* Compute mag from mtf1 - igrf */
#define MGD77_COL_ADJ_FAA	4	/* Compute faa from gobs - igf */
#define MGD77_COL_ADJ_FAA_EOT	5	/* Compute faa from gobs - igf + eot */

struct MGD77_COLINFO {
	char *abbrev;		/* Short name that identifies this column */
	char *name;		/* Longer, descriptive name for column */
	char *units;		/* Units of the data type in this column */
	char *comment;		/* Comment regarding this data column */
	double factor;		/* factor to multiply data immediately after reading from file */
	double offset;		/* offset to add after reading and multiplying by scale */
	double corr_factor;	/* Extra correction factor/offset to follow scale/offset; */
	double corr_offset;	/* this is used to correct wrong units, etc. */
	double limit[2];	/* Lower and upper limits on this data column */
	int pos;		/* Position in output record [0 - n_columns-1]*/
	nc_type type;		/* Type of representation of this data in the netCDF file (NC_SHORT, NC_INT, NC_BYTE, etc) */
	char text;		/* length if this is a text string, else 0 */
	int var_id;		/* netCDF variable ID */
	int adjust;		/* Column needs some sort of adjustment before data is returned [0 means as is] */
	BOOLEAN constant;	/* TRUE if column is constant and only 1 row is/should be stored */
	BOOLEAN present;	/* TRUE if column is present in the file (NaN or otherwise) */
};

struct MGD77_DATA_INFO {
	short n_col;					/* Number of active columns in this MGD77+ file */
	struct MGD77_COLINFO col[MGD77_SET_COLS];	/* List of info per extra column */
	unsigned int bit_pattern;			/* Up to 32 bit flags, one for each parameter desired */
};

struct MGD77_META {	/* Information about a cruise as derived from navigation data */
	BOOLEAN verified;	/* TRUE once MGD77_Verify_Prep has been called */
	int n_ten_box;		/* Number of 10x10 degree boxes visited by this cruise */
	int w, e, s, n;		/* Whole degree left/right/bottom/top coordinates */
	int Departure[3];	/* yyyy, mm, dd of departure */
	int Arrival[3];		/* yyyy, mm, dd of arrival */
	signed char ten_box[20][38];	/* Set to 1 for each box visited */
	double G1980_1930;	/* Average difference between 1980 and 1930 gravity reference field for this cruise */
};

struct MGD77_HEADER {	
	struct MGD77_HEADER_PARAMS *mgd77[2];		/* See MGD-77 Documentation from NGDC for details; [0] is ORIG, [1] is REVISED */
	struct MGD77_META meta;				/* Holds some meta-data derived directly from data records */
	char *author;					/* Name of author of last creation/modification */
	char *history;					/* History of creation/modifications */
	char *E77;					/* Statement of E77 information encoded */
	GMT_LONG n_records;					/* Number of MGD77 data records found */
	int n_fields;					/* Number of columns returned */
	int errors[3];					/* Number of total errors, (warnings, errors) found when reading this header */
	BOOLEAN no_time;				/* TRUE for those few cruises that have no time values */
	double PDR_wrap;				/* Non-zero if we must undo PDR wrapping */
	struct MGD77_DATA_INFO info[MGD77_N_SETS];	/* Info regarding [0] standard MGD77 columns and [1] any extra columns (max 32 each) */
};

#ifdef USE_CM4
typedef int logical;
struct MGD77_CM4 {	/* For use with cm4field.c and initialized by MGD77_CM4_init () */
	char *path[3];		/* Paths to the three coefficient files */
	int unit[3];		/* IDs of logical units */
	logical load[3];	/* TRUE if the file has been read into memory */
	logical index[2];	/* Index acquisition flags/ TRUE from file, FALSE from argument */
	logical gmut;		/* Magnetic dipole universal time (MUT) acquisition flag: TRUE, compute from UT, FALSE, from arg */
	logical cord;		/* TRUE for geodetic, FALSE for geocentric */
	logical pred[6];	/* TRUE to compute, FALSE not: 1 = main field, 2 magnetispheric, 3 ionospheric, etc */
	logical curr;		/* Model J current field prediction flag: TRUE, compute, FALSE not */
	logical coef;		/* Model coefficient generation flag: TRUE, compute, FALSE not */
	int nhmf[2];		/* Maximum main field spherical harmonic degree (0 for main field 1, 1 for main field 2) */
	int nlmf[2];		/* Minimum main field spherical harmonic degree (0 for main field 1, 1 for main field 2) */
	double mut;		/* Magnetic dipole universal time (hours 0-24), computed from UT and returned as gmut = TRUE */
	double alt;		/* Altitude [0] */
	double dst;		/* Linearly interpolated hourly Dst magnetic index, returned from file as index[0] = TRUE */
	double f107;		/* Linearly interpolated 3-monthly means of absolute F10.7 solar radiation flux value, returned */
	double bmdl[3][7];	/* Array storing computed B field vectors from various sources (nT) */
	double jmdl[3][4];	/* Array storing computed J field vectors from certain external sources */
	double *gmdl;		/* Array storing coefficients from various sources */
	int perr;		/* Error message print flag: 0 do not print */
	int oerr;		/* Unit to print to */
	int cerr;		/* Error return code (0 = normal, 1-49 warning, > 50 fatal */
};
#endif

/* We may want to output columns that themselves are not stored in the MGD77[+] files but
 * rather are computed based on data that are stored in the file.  We consider such information
 * as AUXILLARY columns and insert them between the observed columns when needed.  The following
 * structures are used to facilitate this process. */

#ifdef USE_CM4
#define N_MGD77_AUX	16		/* Number of auxilliary derived columns for MGD77 data, including optional CM4 */
#else
#define N_MGD77_AUX	15		/* Number of auxilliary derived columns for MGD77 data */
#endif
#define N_GENERIC_AUX	3		/* Number of auxilliary derived columns for general files (dist, azim, vel) */

#define MGD77_AUX_DS	0
#define MGD77_AUX_AZ	1
#define MGD77_AUX_SP	2
#define MGD77_AUX_YR	3
#define MGD77_AUX_MO	4
#define MGD77_AUX_DY	5
#define MGD77_AUX_HR	6
#define MGD77_AUX_MI	7
#define MGD77_AUX_SC	8
#define MGD77_AUX_WT	9
#define MGD77_AUX_RT	10
#define MGD77_AUX_MG	11
#define MGD77_AUX_CT	12
#define MGD77_AUX_GR	13
#define MGD77_AUX_ID	14
#ifdef USE_CM4
#define MGD77_AUX_CM	15
#endif

struct MGD77_AUXLIST {
	char name[MGD77_COL_ABBREV_LEN];
	GMT_LONG type;
	GMT_LONG text;
	GMT_LONG requested;
	char header[GMT_TEXT_LEN];
};

struct MGD77_AUX_INFO {
	GMT_LONG type;
	GMT_LONG text;
	GMT_LONG pos;
};

/* The data records in the MGD77 file consist of records that are 120 characters.
 * This information can be read and stored internally in the structure MGD77_DATA_RECORD.
 * The two i/o functions MGD77_read_record and MGD77_write_record will do exactly what they say.
 */

struct MGD77_DATA_RECORD {	/* See MGD77 Documentation from NGDC for details */
	/* This is the classic MGD77 portion of the data record */
	double number[MGD77_N_NUMBER_FIELDS];	/* 24 fields that express numerical values */
	double time;				/* Time using current GMT absolute time conventions (J2000 UTC) */
	char word[MGD77_N_STRING_FIELDS][10];	/* The 3 text strings in MGD77 records */
	unsigned int bit_pattern;		/* Bit pattern indicating which of the 27 fields are present in current record */
	BOOLEAN keep_nav;				/* Set to false when navigation is bad */
};

struct MGD77_DATASET {	/* Info for an entire MGD77+ data set */
	int n_fields;				/* Number of active columns in the values table */
	int errors;				/* Number of errors encountered when writing this data */
	struct MGD77_HEADER H;			/* The file's header information */
	void *values[MGD77_MAX_COLS];		/* 2-D table of necessary number of columns and rows (mix of double and char pointers) */
	unsigned int *flags[MGD77_N_SETS];	/* Optional arrays of custom error bit flags for each set */
};

struct MGD77_RECORD_DEFAULTS {
	char *fieldID;     /* variable names for the different MGD77 data fields */
	char *abbrev ;     /* acronyms for the 27 MGD77 data fields */
	int start;         /* beginning character number for each data field */
	int length;	   /* number of characters for each data field */
	char *fortranCode; /* data type specified in NGDC's MGD-77 Documentation */
	double factor;	   /* implied decimal factor specified by NGDC */
	char *readMGD77;   /* sscanf conversions for MGD-77 input */
	int order;	   /* MGD-77 specified data record order */
	char *printMGD77;  /* printf conversions for MGD-77 output  */
	char *printVALS;   /* printf conversions for printing converted values */
	char *not_given;   /* MGD77 representation of "no value given" */
};

struct MGD77_CONSTRAINT {
	char name[MGD77_COL_ABBREV_LEN];	/* Name of data col that is constrained */
	int col;				/* Number of data col that is constrained */
	int code;				/* Which test this is */
	BOOLEAN exact;				/* If TRUE we MUST pass this test */
	double d_constraint;			/* Value for testing */
	char c_constraint[GMT_TEXT_LEN];	/* String value for testing */
	PFB double_test;			/* Pointer to function performing the chosen limit test on a double */
	PFB string_test;			/* Pointer to function performing the chosen limit test on a string */
};

struct MGD77_PAIR {
	char name[MGD77_COL_ABBREV_LEN];	/* Name of data col that is to match exactly */
	int col;				/* Number of data col that is constrained */
	int match;				/* 1 if we want the bit to be 1 or 0 if we want it to be 0 to indicate a match*/
	int set, item;				/* Entries into corresponding info structure column */
};

struct MGD77_ORDER {	/* Info on a single desired output column */
	int set;	/* 0 for standard MGD77 data set, 1 for extra CDF columns */
	int item;	/* Position in the H.info[set] column array */
};

struct MGD77_CONTROL {
	/* Programs that need to write out MGD77 data columns in a certain order will need
	 * to declare this structure and use the MGD77_Init function to get going
	 */
	
	/* File path information */
	char *MGD77_HOME;				/* Directory where paths are stored [$GMT_SHAREDIR/mgd77] */
	char **MGD77_datadir;				/* Directories where MGD77 data may live */
	int n_MGD77_paths;				/* Number of such directories */
	char user[MGD77_COL_ABBREV_LEN];		/* Current user id */
	char NGDC_id[MGD77_COL_ABBREV_LEN];		/* Current NGDC file tag id */
	char path[BUFSIZ];				/* Full path to current file */
	FILE *fp;					/* File pointer to current open file (not used by MGD77+) */
	int verbose_level;				/* 0 = none, 1 = warnings, 2 = errors (combined 3 for both) */
	int verbose_dest;				/* 1 = to stdout, 2 = to stderr */
	int nc_id;					/* netCDF ID for current open file (MGD77+ only) */
	int nc_recid;					/* netCDF ID for dimension of records (time) */
	GMT_LONG rec_no;					/* Current record to read/write for record-based i/o */
	int format;					/* 0 if any file format, 1 if MGD77, and 2 if netCDF, 3 if ascii table */
	/* Format-related issues */
	int time_format;				/* Either GMT_IS_ABSTIME or GMT_IS_RELTIME */
	struct GMT_TIME_SYSTEM unix;			/* All the information about the Unix time system */
	BOOLEAN adjust_time;				/* TRUE if GMT time-system is NOT unix */
	/* Data use information */
	BOOLEAN original;				/* TRUE means we want original not revised header attributes */
	BOOLEAN Want_Header_Item[MGD77_N_HEADER_ITEMS];	/* TRUE means print this header item if dump is selected */
	BOOLEAN use_flags[MGD77_N_SETS];		/* TRUE means programs will use error bitflags (if present) when returning data */
	BOOLEAN use_corrections[MGD77_N_SETS];		/* TRUE means we will apply correction factors (if present) when reading data */
	struct MGD77_ORDER order[MGD77_MAX_COLS];	/* Gives the output order (set, item) of each desired column */
	unsigned int bit_pattern[2];			/* 64 bit flags, one for each parameter desired */
	int n_constraints;				/* Number of constraints specified */
	int n_exact;					/* Number of exact columns to match */
	int n_bit_tests;				/* Number of bit tests to match */
	int no_checking;				/* TRUE if there are no constraints, exact-tests, or bit-tests to pass */
	struct MGD77_CONSTRAINT Constraint[MGD77_MAX_COLS];		/* List of constraints, if any */
	char desired_column[MGD77_MAX_COLS][MGD77_COL_ABBREV_LEN];	/* List of desired column names in final output order */
	struct MGD77_PAIR Exact[MGD77_MAX_COLS];	/* List of column names whose values must be !NaN to be output, if any */
	struct MGD77_PAIR Bit_test[MGD77_MAX_COLS];	/* List of bit-tests, if any */
	int n_out_columns;				/* Number of output columns requested */
};

#define N_CARTER_BINS 64800             /* Number of 1x1 degree bins */
#define N_CARTER_ZONES 85               /* Number of Carter zones */
#define N_CARTER_OFFSETS 86             /* Number of Carter offsets */
#define N_CARTER_CORRECTIONS 5812       /* Number of Carter corrections */

struct MGD77_CARTER {
	int initialized;
	short int carter_zone[N_CARTER_BINS];
	short int carter_offset[N_CARTER_OFFSETS];
	short int carter_correction[N_CARTER_CORRECTIONS];
};

/* Structures for ephemeral corrections */

struct MGD77_CORRECTION {	/* Holds parameters for one term of a correction for one kind of observation */
	int id;			/* The id - entry to give us the data column to use*/
	double factor;		/* Amplitude to multiply the basis function [1] */
	double origin;		/* Local origin to subtract from argument [0] */
	double scale;		/* Scale to apply to (value - origin) */
	double power;		/* Power we should raise the argument to [1] */
	PFD modifier;		/* Pointer to function that will modify argument */
	struct MGD77_CORRECTION *next;
};

struct MGD77_CORRTABLE {
	struct MGD77_CORRECTION *term;
};

/* Primary user functions */

extern void MGD77_Init (struct MGD77_CONTROL *F);						/* Initialize the MGD77 machinery */
extern void MGD77_Reset (struct MGD77_CONTROL *F);									/* Reset after finishing a file */
extern void MGD77_end (struct MGD77_CONTROL *F);				/* Free up MGD77-related variables */
extern int MGD77_Path_Expand (struct MGD77_CONTROL *F, char **argv, int argc, char ***list);				/* Returns the full list of IDs */
extern void MGD77_Path_Free (int n, char **list);	/* Free the list of IDs */
extern void MGD77_Cruise_Explain (void);										/* Explains how to specify IDs */
extern int MGD77_Get_Path (char *track_path, char *track, struct MGD77_CONTROL *F);					/* Returns full path to cruise */
extern int MGD77_Open_File (char *leg, struct MGD77_CONTROL *F, int rw);						/* Opens a MGD77[+] file */
extern int MGD77_Close_File (struct MGD77_CONTROL *F);									/* Closes a MGD77[+] file */
extern int MGD77_Read_File (char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET *S);				/* Allocate & Read entire file (selected columns only) */
extern int MGD77_Write_File (char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET *S);				/* Write entire file (all columns) */
extern int MGD77_Read_Header_Record (char *file, struct MGD77_CONTROL *F, struct MGD77_HEADER *H);			/* Read the header record */
extern int MGD77_Write_Header_Record (char *file, struct MGD77_CONTROL *F, struct MGD77_HEADER *H);			/* Write the header record */
extern int MGD77_Free_Header_Record (struct MGD77_CONTROL *F, struct MGD77_HEADER *H);					/* Frees up header memory */
extern int MGD77_Read_Data (char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET *S);				/* Allocate & Read all data (selected columns only); Header already read */
extern int MGD77_Write_Data (char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET *S);				/* Write all data (all columns); Header already written */
extern int MGD77_Read_Data_Record (struct MGD77_CONTROL *F, struct MGD77_HEADER *H, double dvals[], char *tvals[]);	/* Read a single data record (selected columns only) */
extern int MGD77_Write_Data_Record (struct MGD77_CONTROL *F, struct MGD77_HEADER *H, double dvals[], char *tvals[]);	/* Write a single data record (selected columns only) */
extern void MGD77_Free (struct MGD77_DATASET *S);									/* Free memory allocated by MGD77_Read_File/MGD77_Read_Data */
extern void MGD77_Select_Columns (char *string, struct MGD77_CONTROL *F, int option);					/* Decode the -F option specifying the desired columns */
extern int MGD77_Get_Column (char *word, struct MGD77_CONTROL *F);							/* Get column number from column name (or -1 if not present) */
extern int MGD77_Info_from_Abbrev (char *name, struct MGD77_HEADER *H, int *set, int *item);
extern void MGD77_List_Header_Items (struct MGD77_CONTROL *F);
extern int MGD77_Select_Header_Item (struct MGD77_CONTROL *F, char *item);
extern int MGD77_Get_Set (char *abbrev);										/* Returns 0 if abbrev is in the MGD77 set, else 1 */
extern void MGD77_Fatal_Error (int error);										/* Print message for this error and exit */
extern BOOLEAN MGD77_Pass_Record (struct MGD77_CONTROL *F, struct MGD77_DATASET *S, GMT_LONG rec);				/* Tests if a record passes all specified logical & exact tests */
extern void MGD77_Apply_Bitflags (struct MGD77_CONTROL *F, struct MGD77_DATASET *S, GMT_LONG rec, BOOLEAN apply_bits[]);	/* Replaces values whose flags are ON with NaNs */
extern void MGD77_Set_Unit (char *dist, double *scale, int way);							/* Convert appended distance unit to a numerical scale to give meters */
extern void MGD77_nc_status (int status);										/* Checks for netCDF errors and aborts with error message */
extern void MGD77_Process_Ignore (char code, char *format);								/* Process the ignre-format option */
extern void MGD77_Ignore_Format (int format);										/* Dissallow some formats for consideration */
extern struct MGD77_DATASET *MGD77_Create_Dataset ();									/* Create an empty data set structure */
extern void MGD77_Prep_Header_cdf (struct MGD77_CONTROL *F, struct MGD77_DATASET *S);					/* Prepare header before we write */
extern void MGD77_Dump_Header_Params (struct MGD77_CONTROL *F, struct MGD77_HEADER_PARAMS *P);				/* Dump of header items, one per line */
extern void MGD77_Verify_Header (struct MGD77_CONTROL *F, struct MGD77_HEADER *H, FILE *ufp);				/* Verify content of header per MGD77 docs */
extern void MGD77_Verify_Prep (struct MGD77_CONTROL *F, struct MGD77_DATASET *D);
extern void MGD77_Verify_Prep_m77 (struct MGD77_CONTROL *F, struct MGD77_META *C, struct MGD77_DATA_RECORD *D, GMT_LONG nrec);
extern int MGD77_Remove_E77 (struct MGD77_CONTROL *F);

/* Secondary user functions */

extern int MGD77_Read_Header_Record_asc (char *file, struct MGD77_CONTROL *F, struct MGD77_HEADER *H);			/* Hardwired read of ascii/MGD77 header */
extern int MGD77_Read_Data_Record_m77 (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *MGD77Record);			/* Hardwired read of ascii/MGD77 data record */
extern int MGD77_Write_Header_Record_m77 (char *file, struct MGD77_CONTROL *F, struct MGD77_HEADER *H);			/* Hardwired write of ascii/MGD77 header */
extern int MGD77_Write_Data_Record_m77 (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *MGD77Record);		/* Hardwired write of ascii/MGD77 data record */

/* These are only for developers */

extern BOOLEAN MGD77_dbl_are_constant (double x[], GMT_LONG n, double limits[]);
extern BOOLEAN MGD77_txt_are_constant (char *txt, GMT_LONG n, int width);
extern int MGD77_do_scale_offset_before_write (double new[], const double x[], GMT_LONG n, double scale, double offset, int type);
extern void MGD77_select_high_resolution ();
extern void MGD77_free_plain_mgd77 (struct MGD77_HEADER *H);
extern int MGD77_Match_List (char *word, int n_fields, char **list);

/* User functions for direct use of Carter corrections */

extern int MGD77_carter_depth_from_twt (int zone, double twt_in_msec, struct MGD77_CARTER *C, double *depth_in_corr_m);
extern int MGD77_carter_twt_from_depth (int zone, double depth_in_corr_m, struct MGD77_CARTER *C, double *twt_in_msec);
extern int MGD77_carter_depth_from_xytwt (double lon, double lat, double twt_in_msec, struct MGD77_CARTER *C, double *depth_in_corr_m);
extern int MGD77_carter_twt_from_xydepth (double lon, double lat, double depth_in_corr_m, struct MGD77_CARTER *C, double *twt_in_msec);
extern double MGD77_carter_correction (double lon, double lat, double twt_in_msec, struct MGD77_CARTER *C);

/* User functions for direct use of IGRF corrections, theoretical gravity */

extern int MGD77_igrf10syn (int isv, double date, int itype, double alt, double lon, double lat, double *out);
extern double MGD77_Theoretical_Gravity (double lon, double lat, int version);
extern void MGD77_IGF_text (FILE *fp, int version);
extern double MGD77_Recalc_Mag_Anomaly_IGRF (double time, double lon, double lat, double obs, BOOLEAN calc_date);
extern double MGD77_time_to_fyear (double time);
extern double MGD77_cal_to_fyear (struct GMT_gcal *cal);
extern BOOLEAN MGD77_fake_times (struct MGD77_CONTROL *F, struct MGD77_HEADER *H, double *lon, double *lat, double *times, GMT_LONG nrec);
#ifdef USE_CM4 
extern double MGD77_Calc_CM4 (double time, double lon, double lat, BOOLEAN calc_date, struct MGD77_CM4 *CM4);
extern double MGD77_Recalc_Mag_Anomaly_CM4 (double time, double lon, double lat, double obs, BOOLEAN calc_date, struct MGD77_CM4 *CM4);
extern void MGD77_CM4_end (struct MGD77_CM4 *CM4);
#endif

/* These are called indirectly but remain accessible for specialist programs */

extern int MGD77_carter_init (struct MGD77_CARTER *C);
extern int MGD77_carter_get_bin (double lon, double lat, int *bin);
extern int MGD77_carter_get_zone (int bin, struct MGD77_CARTER *C, int *zone);
extern double *MGD77_Distances (double x[], double y[], GMT_LONG n, int dist_flag);

/* Global variables used by MGD77 programs */

extern struct MGD77_RECORD_DEFAULTS mgd77defs[MGD77_N_DATA_FIELDS];
extern double MGD77_NaN_val[7], MGD77_Low_val[7], MGD77_High_val[7];
extern char *MGD77_suffix[MGD77_N_FORMATS];
extern BOOLEAN MGD77_format_allowed[MGD77_N_FORMATS];	/* By default we allow opening of files in any format.  See MGD77_Ignore_Format() */
extern double MGD77_Epoch_zero;
extern int MGD77_pos[MGD77_N_DATA_EXTENDED];

int MGD77_Scan_Corrtable (char *tablefile, char **cruises, int n_cruises, int n_fields, char **field_names, char ***item_names, int mode);
void MGD77_Parse_Corrtable (char *tablefile, char **cruises, int n_cruises, int n_fields, char **field_names, int mode, struct MGD77_CORRTABLE ***CORR);
void MGD77_Init_Correction (struct MGD77_CORRTABLE *CORR, double **value);
double MGD77_Correction (struct MGD77_CORRECTION *C, double **value, double *aux, GMT_LONG rec);
double MGD77_Correction_Rec (struct MGD77_CORRECTION *C, double *value, double *aux);
void MGD77_Free_Correction (struct MGD77_CORRTABLE **CORR, int n);

#include "mgd77_functions.h"	/* These were created by mgd77netcdfhelper.sh */
#include "cm4_functions.h"
extern void MGD77_CM4_init (struct MGD77_CONTROL *F, struct MGD77_CM4 *CM4);

#endif	/* _MGD77_H */
