/*-------------------------------------------------------------------------
 *	$Id: mgd77.h,v 1.2 2004-08-19 21:29:22 pwessel Exp $
 * 
 *  File:	MGD77.h
 *
 *	Include file for programs that plan to read/write MGD77 files
 *
 *  Authors:    Michael Chandler, Master's Candidate, SOEST, U. of Hawaii
 *		Paul Wessel, Primary Investigator, SOEST, U. of Hawaii
 * 
 *  Date:	22-June-2004
 * 
 *-------------------------------------------------------------------------*/

#ifndef _MGD77_H
#define _MGD77_H

#include "gmt.h"

#define MGD77_RECORD_LENGTH	122
#define MGD77_HEADER_LENGTH	82
#define MGD77_N_HEADER_RECORDS	24
#define MGD77_METERS_PER_NM     1852		/* meters per nautical mile */
#define MGD77_METERS_PER_M      1609.344	/* meters per statute mile */
#define MGD77_N_DATA_FIELDS	27
#define MGD77_N_NUMBER_FIELDS	24
#define MGD77_TIME		(MGD77_N_DATA_FIELDS + 0)
#define MGD77_DISTANCE		(MGD77_N_DATA_FIELDS + 1)
#define MGD77_HEADING		(MGD77_N_DATA_FIELDS + 2)
#define MGD77_SPEED		(MGD77_N_DATA_FIELDS + 3)
#define MGD77_WEIGHT		(MGD77_N_DATA_FIELDS + 4)
#define MGD77_LATITUDE		7
#define MGD77_LONGITUDE		8
#define MGD77_ID		24
#define MGD77_SLN		25
#define MGD77_SSPN		26

/* We will use bit flags to keep track of which data column we are referring to.
 * field 0 is rightmost bit (1), field 1 is the next bit (2), field 2 is 4 and
 * so on for powers of 2.  We add these bit powers together to get the bit patterns
 * for a selection of columns.  E.g., field 0, 3, 5 has pattern 1 + 8 + 32 = 41.
 * Some combinations of columns have precalculated bit patterns given below */
#define MGD77_GEOPHYSICAL_BITS	5360640	/* 1010  0011  1001  1000  0000  0000*/
#define MGD77_CORRECTION_BITS	2883584	/*  101  1000  0000  0000  0000  0000*/
#define MGD77_FLOAT_BITS	(MGD77_GEOPHYSICAL_BITS + MGD77_CORRECTION_BITS)
#define MGD77_STRING_BITS	(16777216+33554432+67108864)

#define MGD77_EQ		1
#define MGD77_LT		2
#define MGD77_LE		3
#define MGD77_GT		4
#define MGD77_GE		5
#define MGD77_NEQ		8

typedef char byte;	/* Used to indicate 1-byte long integer */
typedef char* Text;	/* Used to indicate character strings */
 
/* The MGD77 File format contains a header section consisting a set of 24 records
 * of length 80 characters each.  This information can be read and stored internally
 * in the structure MGD77_HEADER_RECORD.  The two i/o functions MGD77_read_header and
 * MGD77_write_header will do exactly what they say.
 */

struct MGD77_HEADER_RECORD {		/* See MGD-77 Documentation from NGDC for details */
	char	record[MGD77_N_HEADER_RECORDS][MGD77_HEADER_LENGTH];	/* Keep all 24 header records in memory */
	/* Sequence No 01: */
	byte	Record_Type;
	Text	Cruise_Identifier;
	Text	Format_Acronym;
	int	Data_Center_File_Number;
	Text	Blank_1;
	byte	Paramaters_Surveyed_Code[5];
	short	File_Creation_Year;
	byte	File_Creation_Month;
	byte	File_Creation_Day;
	Text	Contributing_Institution;
	/* Sequence No 02: */
	Text	Country;
	Text	Platform_Name;
	byte	Platform_Type_Code;
	Text	Platform_Type;
	Text	Chief_Scientist;
	/* Sequence No 03: */
	Text	Project_Cruise_Leg;
	Text	Funding;
	/* Sequence No 04: */
	short	Survey_Departure_Year;
	byte	Survey_Departure_Month;
	byte	Survey_Departure_Day;
	Text	Port_of_Departure;
	short	Survey_Arrival_Year;
	byte	Survey_Arrival_Month;
	byte	Survey_Arrival_Day;
	Text	Port_of_Arrival;
	/* Sequence No 05: */
	Text	Navigation_Instrumentation;
	Text	Position_Determination_Method;
	/* Sequence No 06: */
	Text	Bathymetry_Instrumentation;
	Text	Bathymetry_Add_Forms_of_Data;
	/* Sequence No 07: */
	Text	Magnetics_Instrumentation;
	Text	Magnetics_Add_Forms_of_Data;
	/* Sequence No 08: */
	Text	Gravity_Instrumentation;
	Text	Gravity_Add_Forms_of_Data;
	/* Sequence No 09: */
	Text	Seismic_Instrumentation;
	Text	Seismic_Add_Forms_of_Data;
	/* Sequence No 10: */
	char	Format_Type;
	Text	Format_Description_1;
	Text	Blank_2;
	/* Sequence No 11: */
	Text	Format_Description_2;
	Text	Blank_3;
	short	Topmost_Latitude;
	short	Bottommost_Latitude;
	short	Leftmost_Longitude;
	short	Rightmost_Longitude;
	Text	Blank_4;
	/* Sequence No 12: */
	float	Bathymetry_Digitizing_Rate;
	Text	Bathymetry_Sampling_Rate;
	float	Bathymetry_Assumed_Sound_Velocity;
	byte	Bathymetry_Datum_Code;
	Text	Bathymetry_Interpolation_Scheme;
	/* Sequence No 13: */
	float	Magnetics_Digitizing_Rate;
	byte	Magnetics_Sampling_Rate;
	short	Magnetics_Sensor_Tow_Distance;
	float	Magnetics_Sensor_Depth;
	short	Magnetics_Sensor_Separation;
	byte	Magnetics_Ref_Field_Code;
	Text	Magnetics_Ref_Field;
	Text	Magnetics_Method_Applying_Res_Field;
	/* Sequence No 14: */
	float	Gravity_Digitizing_Rate;
	byte	Gravity_Sampling_Rate;
	byte	Gravity_Theoretical_Formula_Code;
	Text	Gravity_Theoretical_Formula;
	byte	Gravity_Reference_System_Code;
	Text	Gravity_Reference_System;
	Text	Gravity_Corrections_Applied;
	/* Sequence No 15: */
	float	Gravity_Departure_Base_Station;
	Text	Gravity_Departure_Base_Station_Name;
	float	Gravity_Arrival_Base_Station;
	Text	Gravity_Arrival_Base_Station_Name;
	/* Sequence No 16: */
	byte	Number_of_Ten_Degree_Identifiers;
	char	Blank_5;
	short	Ten_Degree_Identifier_1[15];
	/* Sequence No 17: */
	short	Ten_Degree_Identifier_2[15];
	Text	Blank_6;
	/* Sequences No 18-24: */
	Text	Additional_Documentation[7];
};

/* The data records in the MGD77 file consist of records that are 120 characters.
 * This information can be read and stored internally in the structure MGD77_DATA_RECORD.
 * The two i/o functions MGD77_read_record and MGD77_write_record will do exactly what they say.
 */

struct MGD77_DATA_RECORD {	/* See MGD-77 Documentation from NGDC for details */
	double number[24];
	double time;		/* Time using current GMT absolute time conventions */
	char word[3][10];
	unsigned int bit_pattern;
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
	int min;	   /* minimum accepted value for MGD77 data fields */
	int max;	   /* maximum accepted value for MGD77 data fields */
	double maxSlope;   /* maximum +/- change in values through distance or time */
};

struct MGD77_CONSTRAINT {
	int col;		/* Number of data col that is constrained */
	BOOLEAN exact;		/* If TRUE we MUST pass this test */
	double d_constraint;	/* Value for testing */
	char *c_constraint;	/* String value for testing */
	PFB double_test;	/* Pointer to function performing the chosen limit test on a double */
	PFB string_test;	/* Pointer to function performing the chosen limit test on a string */
};

struct MGD77_CONTROL {
	/* Programs that need to write out MGD77 data columns in a certain order will need
	 * to declare this structure and use the MGD77_Init function to get going
	 */
	 
	char *MGD77_HOME;				/* Directory where paths are stored */
	char **MGD77_datadir;				/* Directories where MGD77 data may live */
	int n_MGD77_paths;				/* Number of these directories */

	int n_out_columns;				/* Number of output columns requested */
	int order[32];					/* Gives the output order of each column */
	BOOLEAN use_column[32];				/* TRUE for columns we are interested in outputting */
	unsigned int bit_pattern;			/* 27 bit flags, one for each parameter desired */
	int n_constraints;				/* Number of constraints selected */
	int n_exact;					/* Number of exact columns to match */
	int *exact;					/* List of columns to match exactly */
	BOOLEAN no_checking;				/* TRUE if there are no complicated checking to do */
	struct MGD77_CONSTRAINT *Constraint;		/* List of constraints, if any */
};

EXTERN_MSC void MGD77_Init (struct MGD77_CONTROL *F, BOOLEAN remove_blanks);		/* Initialize the MGD77 machinery */
EXTERN_MSC int  MGD77_Read_Header_Record (FILE *fp, struct MGD77_HEADER_RECORD *H);	/* Will read the entire 24-section header structure */
EXTERN_MSC int  MGD77_Write_Header_Record_New (FILE *fp, struct MGD77_HEADER_RECORD *H);	/* Will write the entire 24-section header structure based on variables */
EXTERN_MSC int  MGD77_Write_Header_Record_Orig (FILE *fp, struct MGD77_HEADER_RECORD *H);	/* Will write the entire 24-section header structure by echoing text records */
EXTERN_MSC int  MGD77_Read_Data_Record (FILE *fp, struct MGD77_DATA_RECORD *H);		/* Will read a single data record */
EXTERN_MSC int  MGD77_Write_Data_Record_ew (FILE *fp, struct MGD77_DATA_RECORD *H);	/* Will write a single data record */
EXTERN_MSC int  MGD77_View_Line (FILE *fp, char *line);					/* View a single MGD77 string */
EXTERN_MSC int  MGD77_Convert_To_Old_Format(char *newFormatLine, char *oldFormatLine);	/* Will convert a single record from new to old MGD77 format */
EXTERN_MSC int  MGD77_Convert_To_New_Format(char *oldFormatLine, char *newFormatLine);	/* Will convert a single record from old to new MGD77 format */
EXTERN_MSC int  MGD77_Get_Path (char *track_path, char *track, struct MGD77_CONTROL *F);	/* Returns full path to cruise */
EXTERN_MSC void MGD77_Select_Columns (char *string, struct MGD77_CONTROL *F);		/* Decode the -F option */
EXTERN_MSC BOOLEAN MGD77_pass_record (struct MGD77_DATA_RECORD *H, struct MGD77_CONTROL *F);	/* Compare record to specified constraints */
EXTERN_MSC int MGD77_set_unit (char *dist, double *scale);

EXTERN_MSC struct MGD77_RECORD_DEFAULTS mgd77defs[MGD77_N_DATA_FIELDS];


#endif	/* _MGD77_H */
