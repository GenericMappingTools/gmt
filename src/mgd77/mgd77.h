/*-------------------------------------------------------------------------
 *	$Id: mgd77.h,v 1.43 2005-10-17 08:16:28 pwessel Exp $
 * 
 *    Copyright (c) 2005 by P. Wessel
 *    See README file for copying and redistribution conditions.
 *
 *  File:	mgd77.h
 *
 *  Include file for programs that plan to read/write MGD77[+] files
 *
 *  Authors:    Paul Wessel, Primary Investigator, SOEST, U. of Hawaii
 *		Michael Chandler, Master's Candidate, SOEST, U. of Hawaii
 *		
 *  Version:	1.1
 *  Revised:	10-OCT-2005
 * 
 *-------------------------------------------------------------------------*/

#ifndef _MGD77_H
#define _MGD77_H

#include "gmt.h"

#define MGD77_CDF_VERSION	"2005.11.1"	/* Current version of MGD77+ files created */
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
#define MGD77_TIME_BITS		    124	/* 0000 0000 0000 0000 0111 1100 */
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

struct MGD77_HEADER_PARAMS {		/* See MGD-77 Documentation from NGDC for details */
/* START OF MGD77_HEADER_PARAMS */
	/* Sequence No 01: */
	char	Record_Type;
	char	Survey_Identifier[9];
	char	Format_Acronym[6];
	char	Data_Center_File_Number[9];
	char	Paramaters_Surveyed_Code[5];
	char	File_Creation_Year[5];
	char	File_Creation_Month[3];
	char	File_Creation_Day[3];
	char	Source_Institution[40];
	/* Sequence No 02: */
	char	Country[19];
	char	Platform_Name[22];
	char	Platform_Type_Code;
	char	Platform_Type[7];
	char	Chief_Scientist[33];
	/* Sequence No 03: */
	char	Project_Cruise_Leg[59];
	char	Funding[21];
	/* Sequence No 04: */
	char	Survey_Departure_Year[5];
	char	Survey_Departure_Month[3];
	char	Survey_Departure_Day[3];
	char	Port_of_Departure[33];
	char	Survey_Arrival_Year[5];
	char	Survey_Arrival_Month[3];
	char	Survey_Arrival_Day[3];
	char	Port_of_Arrival[31];
	/* Sequence No 05: */
	char	Navigation_Instrumentation[41];
	char	Geodetic_Datum_Position_Determination_Method[39];
	/* Sequence No 06: */
	char	Bathymetry_Instrumentation[41];
	char	Bathymetry_Add_Forms_of_Data[39];
	/* Sequence No 07: */
	char	Magnetics_Instrumentation[41];
	char	Magnetics_Add_Forms_of_Data[39];
	/* Sequence No 08: */
	char	Gravity_Instrumentation[41];
	char	Gravity_Add_Forms_of_Data[39];
	/* Sequence No 09: */
	char	Seismic_Instrumentation[41];
	char	Seismic_Add_Forms_of_Data[39];
	/* Sequence No 10: (Format_Description is split across 10 and 11 */
	char	Format_Type;
	char	Format_Description[95];
	/* Sequence No 11: */
	char	Topmost_Latitude[4];
	char	Bottommost_Latitude[4];
	char	Leftmost_Longitude[5];
	char	Rightmost_Longitude[5];
	/* Sequence No 12: */
	char	Bathymetry_Digitizing_Rate[4];
	char	Bathymetry_Sampling_Rate[13];
	char	Bathymetry_Assumed_Sound_Velocity[6];
	char	Bathymetry_Datum_Code[3];
	char	Bathymetry_Interpolation_Scheme[57];
	/* Sequence No 13: */
	char	Magnetics_Digitizing_Rate[4];
	char	Magnetics_Sampling_Rate[3];
	char	Magnetics_Sensor_Tow_Distance[5];
	char	Magnetics_Sensor_Depth[6];
	char	Magnetics_Sensor_Separation[4];
	char	Magnetics_Ref_Field_Code[3];
	char	Magnetics_Ref_Field[13];
	char	Magnetics_Method_Applying_Res_Field[48];
	/* Sequence No 14: */
	char	Gravity_Digitizing_Rate[4];
	char	Gravity_Sampling_Rate[3];
	char	Gravity_Theoretical_Formula_Code;
	char	Gravity_Theoretical_Formula[18];
	char	Gravity_Reference_System_Code;
	char	Gravity_Reference_System[17];
	char	Gravity_Corrections_Applied[39];
	/* Sequence No 15: */
	char	Gravity_Departure_Base_Station[8];
	char	Gravity_Departure_Base_Station_Name[34];
	char	Gravity_Arrival_Base_Station[8];
	char	Gravity_Arrival_Base_Station_Name[32];
	/* Sequence No 16+17: */
	char	Number_of_Ten_Degree_Identifiers[3];
	char	Ten_Degree_Identifier[151];
	/* Sequence No 18-24: */
	char	Additional_Documentation[7][79];
/* END OF MGD77_HEADER_PARAMS */
};

#define MGD77_COL_ABBREV_LEN	16
#define MGD77_COL_NAME_LEN	64
#define MGD77_COL_UNIT_LEN	64
#define MGD77_COL_COMMENT_LEN	128

struct MGD77_COLINFO {
	char *abbrev;		/* Short name that identifies this column */
	char *name;		/* Longer, descriptive name for column */
	char *units;		/* Units of the data type in this column */
	char *comment;		/* Comment regarding this data column */
	double scale;		/* factor to multiply data immediately after reading from file */
	double offset;		/* offset to add after reading and multiplying by scale */
	double limit[2];	/* Lower and upper limits on this data column */
	int pos;		/* Position in output record [0 - n_columns-1]*/
	nc_type type;		/* Type of representation of this data in the netCDF file (NC_SHORT, NC_INT, NC_BYTE, etc) */
	char text;		/* length if this is a text string, else 0 */
	int var_id;		/* netCDF variable ID */
	BOOLEAN constant;	/* TRUE if column is constant and only 1 row is/should be stored */
	BOOLEAN present;	/* TRUE if column is present in the file (NaN or otherwise) */
};

struct MGD77_DATA_INFO {
	short n_col;					/* Number of active columns in this MGD77+ file */
	struct MGD77_COLINFO col[MGD77_SET_COLS];	/* List of info per extra column */
	unsigned int bit_pattern;			/* Up to 32 bit flags, one for each parameter desired */
};

struct MGD77_HEADER {	
	struct MGD77_HEADER_PARAMS *mgd77;		/* See MGD-77 Documentation from NGDC for details */
	char *author;					/* Name of author of last creation/modification */
	char *history;					/* History of creation/modifications */
	int n_records;					/* Number of MGD77 data records found */
	int n_fields;					/* Number of columns returned */
	BOOLEAN no_time;				/* TRUE for those few cruises that have no time values */
	struct MGD77_DATA_INFO info[MGD77_N_SETS];	/* Info regarding [0] standard MGD77 columns and [1] any extra columns (max 32 each) */
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
};

struct MGD77_DATASET {	/* Info for an entire MGD77+ data set */
	int n_fields;				/* Number of active colums in the values table */
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
	char *MGD77_HOME;				/* Directory where paths are stored [$GMTHOME/share/mgd77] */
	char **MGD77_datadir;				/* Directories where MGD77 data may live */
	int n_MGD77_paths;				/* Number of such directories */
	char user[MGD77_COL_ABBREV_LEN];		/* Current user id */
	char NGDC_id[MGD77_COL_ABBREV_LEN];		/* Current NGDC file tag id */
	char path[BUFSIZ];				/* Full path to current file */
	FILE *fp;					/* File pointer to current open file (not used by MGD77+) */
	FILE *fp_err;					/* File pointer error stream [stdout or stderr, usually) */
	int nc_id;					/* netCDF ID for current open file (MGD77+ only) */
	int nc_recid;					/* netCDF ID for dimension of records (time) */
	int rec_no;					/* Current record to read/write for record-based i/o */
	int format;					/* 0 if any file format, 1 if MGD77, and 2 if netCDF, 3 if ascii table */
	/* Format-related issues */
	int time_format;				/* Either GMT_IS_ABSTIME or GMT_IS_RELTIME */
	BOOLEAN flat_earth;				/* TRUE if we want quick distance calcuations */
	/* Data use information */
	BOOLEAN use_flags[MGD77_N_SETS];		/* TRUE means programs will use error bitflags (if present) when returning data */
	BOOLEAN use_corrections[MGD77_N_SETS];		/* TRUE means we will apply correction factors (if present) when reading data */
	struct MGD77_ORDER order[MGD77_MAX_COLS];	/* Gives the output order (set, item) of each desired column */
	unsigned int bit_pattern[2];			/* 64 bit flags, one for each parameter desired */
	int n_constraints;				/* Number of constraints specified */
	int n_exact;					/* Number of exact columns to match */
	int n_bit_tests;				/* Number of bit tests to match */
	BOOLEAN header_verify_level;			/* 0 = none, 1 = to stdout, 2 = to stderr (reports of errors in MGD77 header */
	int no_checking;				/* TRUE if there are no constraints, extact-tests, or bit-tests to pass */
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

/* Primary user functions */

EXTERN_MSC void MGD77_Init (struct MGD77_CONTROL *F, BOOLEAN remove_blanks);							/* Initialize the MGD77 machinery */
EXTERN_MSC void MGD77_Reset (struct MGD77_CONTROL *F);										/* Reset after finishing a file */
EXTERN_MSC int MGD77_Get_Path (char *track_path, char *track, struct MGD77_CONTROL *F);						/* Returns full path to cruise */
EXTERN_MSC int MGD77_Open_File (char *leg, struct MGD77_CONTROL *F, int rw);							/* Opens a MGD77[+] file */
EXTERN_MSC int MGD77_Close_File (struct MGD77_CONTROL *F);									/* Closes a MGD77[+] file */
EXTERN_MSC int MGD77_Read_File (char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET *S);					/* Allocate & Read entire file (selected columns only) */
EXTERN_MSC int MGD77_Write_File (char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET *S);					/* Write entire file (all columns) */
EXTERN_MSC int MGD77_Read_Header_Record (char *file, struct MGD77_CONTROL *F, struct MGD77_HEADER *H);				/* Read the header record */
EXTERN_MSC int MGD77_Write_Header_Record (char *file, struct MGD77_CONTROL *F, struct MGD77_HEADER *H);				/* Write the header record */
EXTERN_MSC int MGD77_Read_Data (char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET *S);					/* Allocate & Read all data (selected columns only); Header already read */
EXTERN_MSC int MGD77_Write_Data (char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET *S);					/* Write all data (all columns); Header already written */
EXTERN_MSC int MGD77_Read_Data_Record (struct MGD77_CONTROL *F, struct MGD77_HEADER *H, double dvals[], char *tvals[]);		/* Read a single data record (selected columns only) */
EXTERN_MSC int MGD77_Write_Data_Record (struct MGD77_CONTROL *F, struct MGD77_HEADER *H, double dvals[], char *tvals[]);	/* Write a single data record (selected columns only) */
EXTERN_MSC void MGD77_Free (struct MGD77_DATASET *S);										/* Free memory allocated by MGD77_Read_File/MGD77_Read_Data */
EXTERN_MSC void MGD77_Select_Columns (char *string, struct MGD77_CONTROL *F, int option);					/* Decode the -F option specifying the desired columns */
EXTERN_MSC int MGD77_Get_Column (char *word, struct MGD77_CONTROL *F);								/* Get column number from column name (or -1 if not present) */
EXTERN_MSC int MGD77_Get_Set (char *abbrev);											/* Returns 0 if abbrev is in the MGD77 set, else 1 */
EXTERN_MSC void MGD77_Fatal_Error (int error);											/* Print message for this error and exit */
EXTERN_MSC BOOLEAN MGD77_Pass_Record (struct MGD77_CONTROL *F, struct MGD77_DATASET *S, int rec);				/* Tests if a record passes all specified logical & exact tests */
EXTERN_MSC void MGD77_Set_Unit (char *dist, double *scale);									/* Convert appended distance unit to a numerical scale to give meters */
EXTERN_MSC void MGD77_nc_status (int status);											/* Checks for netCDF errors and aborts with error message */
EXTERN_MSC void MGD77_Ignore_Format (int format);										/* Dissallow some formats for consideration */
EXTERN_MSC struct MGD77_DATASET *MGD77_Create_Dataset ();									/* Create an empty data set structure */
EXTERN_MSC void MGD77_Prep_Header_cdf (struct MGD77_CONTROL *F, struct MGD77_DATASET *S);

/* Secondary user functions */

EXTERN_MSC int MGD77_Read_Header_Record_asc (char *file, struct MGD77_CONTROL *F, struct MGD77_HEADER *H);			/* Hardwired read of ascii/MGD77 header */
EXTERN_MSC int MGD77_Read_Data_Record_m77 (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *MGD77Record);			/* Hardwired read of ascii/MGD77 data record */
EXTERN_MSC int MGD77_Write_Header_Record_m77 (char *file, struct MGD77_CONTROL *F, struct MGD77_HEADER *H);			/* Hardwired write of ascii/MGD77 header */
EXTERN_MSC int MGD77_Write_Data_Record_m77 (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *MGD77Record);			/* Hardwired write of ascii/MGD77 data record */

/* User functions for direct use of Carter corrections */

EXTERN_MSC int MGD77_carter_depth_from_twt (int zone, double twt_in_msec, struct MGD77_CARTER *C, double *depth_in_corr_m);
EXTERN_MSC int MGD77_carter_twt_from_depth (int zone, double depth_in_corr_m, struct MGD77_CARTER *C, double *twt_in_msec);
EXTERN_MSC int MGD77_carter_depth_from_xytwt (double lon, double lat, double twt_in_msec, struct MGD77_CARTER *C, double *depth_in_corr_m);
EXTERN_MSC int MGD77_carter_twt_from_xydepth (double lon, double lat, double depth_in_corr_m, struct MGD77_CARTER *C, double *twt_in_msec);

/* These are called indirectly but remain accessible for specialist programs */

EXTERN_MSC int MGD77_carter_init (struct MGD77_CARTER *C);
EXTERN_MSC int MGD77_carter_get_bin (double lon, double lat, int *bin);
EXTERN_MSC int MGD77_carter_get_zone (int bin, struct MGD77_CARTER *C, int *zone);
EXTERN_MSC double *MGD77_Distances (double x[], double y[], int n, int dist_flag);

/* Global variables used by MGD77 programs */

EXTERN_MSC struct MGD77_RECORD_DEFAULTS mgd77defs[MGD77_N_DATA_FIELDS];
EXTERN_MSC double MGD77_NaN_val[7];
EXTERN_MSC char *MGD77_suffix[MGD77_N_FORMATS];
EXTERN_MSC BOOLEAN MGD77_format_allowed[MGD77_N_FORMATS];	/* By default we allow opening of files in any format.  See MGD77_Ignore_Format() */
EXTERN_MSC double MGD77_Epoch_zero;

#endif	/* _MGD77_H */
