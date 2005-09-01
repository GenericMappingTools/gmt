/*---------------------------------------------------------------------------
 *	$Id: mgd77.c,v 1.21 2005-09-01 23:49:02 pwessel Exp $
 *
 *  File:	MGD77.c
 * 
 *  Authors:    Paul Wessel, Primary Investigator, SOEST, U. of Hawaii
 *              Michael Chandler, Master's Candidate, SOEST, U. of Hawaii
 * 
 *  Date:	21-June-2004
 * 
 *-------------------------------------------------------------------------*/

#include "mgd77.h"

#define MGD77_OLDEST_YY		39
#define ALL_NINES		"9999999999"
#define ALL_BLANKS "                      "	/* 32 blanks */

void MGD77_Set_Home (struct MGD77_CONTROL *F);
void MGD77_Init_Columns (struct MGD77_CONTROL *F);
void MGD77_Path_Init (struct MGD77_CONTROL *F);
BOOLEAN MGD77_lt_test (double value, double limit);
BOOLEAN MGD77_le_test (double value, double limit);
BOOLEAN MGD77_eq_test (double value, double limit);
BOOLEAN MGD77_neq_test (double value, double limit);
BOOLEAN MGD77_gt_test (double value, double limit);
BOOLEAN MGD77_ge_test (double value, double limit);
BOOLEAN MGD77_clt_test (char *value, char *match);
BOOLEAN MGD77_cle_test (char *value, char *match);
BOOLEAN MGD77_ceq_test (char *value, char *match);
BOOLEAN MGD77_cneq_test (char *value, char *match);
BOOLEAN MGD77_cgt_test (char *value, char *match);
BOOLEAN MGD77_cge_test (char *value, char *match);
int MGD77_Read_Header_Record_ASCII (struct MGD77_CONTROL *F, struct MGD77_HEADER_RECORD *H);
int MGD77_Read_Header_Record_Binary (struct MGD77_CONTROL *F, struct MGD77_HEADER_RECORD *H);
int MGD77_Read_Data_Record_ASCII (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *H);
int MGD77_Read_Data_Record_Binary (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *H);
int MGD77_Write_Data_Record_ASCII (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *H);
int MGD77_Write_Data_Record_Binary (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *H);

struct MGD77_DATA_RECORD *MGD77Record;
 
struct MGD77_RECORD_DEFAULTS mgd77defs[MGD77_N_DATA_FIELDS] = {
#include "mgd77defaults.h"
};

char *MGD77_fmt[2][11] = {
	{
	"%0.0d",
	"%1.1d",
	"%2.2d",
	"%3.3d",
	"%4.4d",
	"%5.5d",
	"%6.6d",
	"%7.7d",
	"%8.8d",
	"%9.9d",
	"%10.10d",
},
{
	"%0d",
	"%1d",
	"%2d",
	"%3d",
	"%4d",
	"%5d",
	"%6d",
	"%7d",
	"%8d",
	"%9d",
	"%10d",
}
};

int MGD77_out_order[27] = { 0, 24, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 25, 26, 23 };
int MGD77_rec_no = 0;
int MGD77_fmt_no = 1;	/* 0 for %x.xd, 1 for %xd */
BOOLEAN MGD77_Strip_Blanks = FALSE;
double MGD77_NaN;
PFB MGD77_column_test_double[9];
PFB MGD77_column_test_string[9];
unsigned int MGD77_this_bit[32];

float MGD77_Get_float (char *record, int pos, int length, double scale);
short MGD77_Get_short (char *record, int pos, int length, double scale);
int MGD77_Get_int (char *record, int pos, int length, double scale);
byte MGD77_Get_byte (char *record, int pos, int length, double scale);
char MGD77_Get_char (char *record, int pos, int length, double scale);
Text MGD77_Get_Text (char *record, int pos, int length, double scale);
void MGD77_Put_blanks (FILE *fp, int length);
void MGD77_Put_float (FILE *fp, float f, int length, double scale, int sign);
void MGD77_Put_int (FILE *fp, int i, int length, double scale, int sign);
void MGD77_Put_short (FILE *fp, short s, int length, double scale, int sign);
void MGD77_Put_byte (FILE *fp, byte b, int length, double scale, int sign);
void MGD77_Put_char (FILE *fp, char c, int length, double scale, int sign);
void MGD77_Put_Text (FILE *fp, Text t, int length, double scale, int sign);
int MGD77_Read_Header_Sequence (FILE *fp, char *record, int seq, int format);
int MGD77_Read_Data_Sequence (FILE *fp, char *record);
void MGD77_Write_Sequence (FILE *fp, int seq, int format);
int MGD77_fwrite_char (double value, double scale, FILE *fp);
int MGD77_fwrite_short (double value, double scale, FILE *fp);
int MGD77_fwrite_int (double value, double scale, FILE *fp);
int MGD77_fread_char (double *value, double scale, FILE *fp);
int MGD77_fread_short (double *value, double scale, FILE *fp);
int MGD77_fread_int (double *value, double scale, FILE *fp);

int MGD77_Open_File (char *leg, struct MGD77_CONTROL *F, int rw)  /* Opens a MGD77[+] file */
{
	int start, stop;
	char path[BUFSIZ], mode[3];
	
	mode[2] = '\0';
	
	if (rw == 0) {	/* Reading a file */
		mode[0] = 'r';
		if (MGD77_Get_Path (path, leg, F)) {
   			fprintf (stderr, "%s : Cannot find leg %s\n", GMT_program, leg);
     			return (MGD77_FILE_NOT_FOUND);
  		}
	}
	else {		/* Writing to a new file; leg is assumed to be complete name */
		mode[0] = 'w';
		strcpy (path, leg);
		F->binary = (path[strlen(path)-1] == '+');
	}
	mode[1] = (F->binary) ? 'b' : '\0';
	if ((F->fp = GMT_fopen (path, mode)) == NULL) {
		fprintf (stderr, "%s: Could not open %s\n", GMT_program, path);
		return (MGD77_ERROR_OPEN_FILE);
	}

	start = stop = -1;
	for (start = strlen (path) - 1; stop == -1 && start >= 0; start--) if (path[start] == '.') stop = start;
	while (start >= 0 && path[start] != '/') start--;
	start++;
	strncpy (F->NGDC_id, &path[start], stop - start);
	
	return (0);
}

int MGD77_Close_File (struct MGD77_CONTROL *F)  /* Closes a MGD77[+] file */
{
	return (GMT_fclose (F->fp));
}

int MGD77_Read_Header_Record (struct MGD77_CONTROL *F, struct MGD77_HEADER_RECORD *H)  /* Will read the entire 24-section header structure */
{
	int error;
	
	error = (F->binary) ? MGD77_Read_Header_Record_Binary (F, H) : MGD77_Read_Header_Record_ASCII (F, H);	/* Will read the ascii or binary MGD77 header record block */
	return (error);
}

int MGD77_Read_Data_Record (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *MGD77Record)	  /* Will read a single MGD77 record */
{
	int error;

	error = (F->binary) ? MGD77_Read_Data_Record_Binary (F, MGD77Record) : MGD77_Read_Data_Record_ASCII (F, MGD77Record);	/* Will read a single ascii or binary MGD77 record */
	return (error);
}

int MGD77_Read_Header_Record_ASCII (struct MGD77_CONTROL *F, struct MGD77_HEADER_RECORD *H)  /* Will read the entire 24-section header structure */
{
	char record[MGD77_HEADER_LENGTH+1];
	int i, sequence = 0, fmt, err;

	for (i = 0; i < MGD77_N_HEADER_RECORDS; i++) memset ((void *)H->record[i], '\0', MGD77_HEADER_LENGTH + 1);
	
	fmt = (F->binary) ? MGD77_FORMAT_BIN : MGD77_FORMAT_ASC;
	
	/* Process Sequence No 01: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[0], record, MGD77_HEADER_LENGTH);

	H->Record_Type                         = MGD77_Get_byte (record, 1, 1, 1);
	H->Cruise_Identifier                   = MGD77_Get_Text (record, 2, 8, 1);
	H->Format_Acronym                      = MGD77_Get_Text (record, 10, 5, 1);
	H->Data_Center_File_Number             = MGD77_Get_int (record, 15, 8, 1);
	H->Blank_1                             = MGD77_Get_Text (record, 23, 4, 1);
	for (i = 0; i < 5; i++) {
		H->Paramaters_Surveyed_Code[i] = MGD77_Get_byte (record, 27 + i, 1, 1);
	}
	H->File_Creation_Year                  = MGD77_Get_short (record, 32, 4, 1);
	H->File_Creation_Month                 = MGD77_Get_byte (record, 36, 2, 1);
	H->File_Creation_Day                   = MGD77_Get_byte (record, 38, 2, 1);
	H->Contributing_Institution            = MGD77_Get_Text (record, 40, 39, 1);

	/* Process Sequence No 02: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[1], record, MGD77_HEADER_LENGTH);

	H->Country                             = MGD77_Get_Text (record, 1, 18, 1);
	H->Platform_Name                       = MGD77_Get_Text (record, 19, 21, 1);
	H->Platform_Type_Code                  = MGD77_Get_byte (record, 40, 1, 1);
	H->Platform_Type                       = MGD77_Get_Text (record, 41, 6, 1);
	H->Chief_Scientist                     = MGD77_Get_Text (record, 47, 32, 1);

	/* Process Sequence No 03: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[2], record, MGD77_HEADER_LENGTH);

	H->Project_Cruise_Leg                  = MGD77_Get_Text (record, 1, 58, 1);
	H->Funding                             = MGD77_Get_Text (record, 59, 20, 1);

	/* Process Sequence No 04: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[3], record, MGD77_HEADER_LENGTH);

	H->Survey_Departure_Year               = MGD77_Get_short (record, 1, 4, 1);
	H->Survey_Departure_Month              = MGD77_Get_byte (record, 5, 2, 1);
	H->Survey_Departure_Day                = MGD77_Get_byte (record, 7, 2, 1);
	H->Port_of_Departure                   = MGD77_Get_Text (record, 9, 32, 1);
	H->Survey_Arrival_Year                 = MGD77_Get_short (record, 41, 4, 1);
	H->Survey_Arrival_Month                = MGD77_Get_byte (record, 45, 2, 1);
	H->Survey_Arrival_Day                  = MGD77_Get_byte (record, 47, 2, 1);
	H->Port_of_Arrival                     = MGD77_Get_Text (record, 49, 30, 1);

	/* Process Sequence No 05: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[4], record, MGD77_HEADER_LENGTH);

	H->Navigation_Instrumentation          = MGD77_Get_Text (record, 1, 40, 1);
	H->Position_Determination_Method       = MGD77_Get_Text (record, 41, 38, 1);

	/* Process Sequence No 06: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[5], record, MGD77_HEADER_LENGTH);

	H->Bathymetry_Instrumentation          = MGD77_Get_Text (record, 1, 40, 1);
	H->Bathymetry_Add_Forms_of_Data        = MGD77_Get_Text (record, 41, 38, 1);

	/* Process Sequence No 07: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[6], record, MGD77_HEADER_LENGTH);

	H->Magnetics_Instrumentation           = MGD77_Get_Text (record, 1, 40, 1);
	H->Magnetics_Add_Forms_of_Data         = MGD77_Get_Text (record, 41, 38, 1);

	/* Process Sequence No 08: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[7], record, MGD77_HEADER_LENGTH);

	H->Gravity_Instrumentation             = MGD77_Get_Text (record, 1, 40, 1);
	H->Gravity_Add_Forms_of_Data           = MGD77_Get_Text (record, 41, 38, 1);

	/* Process Sequence No 09: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[8], record, MGD77_HEADER_LENGTH);

	H->Seismic_Instrumentation             = MGD77_Get_Text (record, 1, 40, 1);
	H->Seismic_Add_Forms_of_Data           = MGD77_Get_Text (record, 41, 38, 1);

	/* Process Sequence No 10: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[9], record, MGD77_HEADER_LENGTH);

	H->Format_Type                         = MGD77_Get_char (record, 1, 1, 1);
	H->Format_Description_1                = MGD77_Get_Text (record, 2, 74, 1);
	H->Blank_2                             = MGD77_Get_Text (record, 76, 3, 1);

	/* Process Sequence No 11: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[10], record, MGD77_HEADER_LENGTH);

	H->Format_Description_2                = MGD77_Get_Text (record, 1, 17, 1);
	H->Blank_3                             = MGD77_Get_Text (record, 18, 23, 1);
	H->Topmost_Latitude                    = MGD77_Get_short (record, 41, 3, 1);
	H->Bottommost_Latitude                 = MGD77_Get_short (record, 44, 3, 1);
	H->Leftmost_Longitude                  = MGD77_Get_short (record, 47, 4, 1);
	H->Rightmost_Longitude                 = MGD77_Get_short (record, 51, 4, 1);
	H->Blank_4                             = MGD77_Get_Text (record, 55, 24, 1);

	/* Process Sequence No 12: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[11], record, MGD77_HEADER_LENGTH);

	H->Bathymetry_Digitizing_Rate          = MGD77_Get_float (record, 1, 3, 0.1);
	H->Bathymetry_Sampling_Rate            = MGD77_Get_Text (record, 4, 12, 1);
	H->Bathymetry_Assumed_Sound_Velocity   = MGD77_Get_float (record, 16, 5, 0.1);
	H->Bathymetry_Datum_Code               = MGD77_Get_byte (record, 21, 2, 1);
	H->Bathymetry_Interpolation_Scheme     = MGD77_Get_Text (record, 23, 56, 1);

	/* Process Sequence No 13: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[12], record, MGD77_HEADER_LENGTH);

	H->Magnetics_Digitizing_Rate           = MGD77_Get_float (record, 1, 3, 0.1);
	H->Magnetics_Sampling_Rate             = MGD77_Get_byte (record, 4, 2, 1);
	H->Magnetics_Sensor_Tow_Distance       = MGD77_Get_short (record, 6, 4, 1);
	H->Magnetics_Sensor_Depth              = MGD77_Get_float (record, 10, 5, 0.1);
	H->Magnetics_Sensor_Separation         = MGD77_Get_short (record, 15, 3, 1);
	H->Magnetics_Ref_Field_Code            = MGD77_Get_byte (record, 18, 2, 1);
	H->Magnetics_Ref_Field                 = MGD77_Get_Text (record, 20, 12, 1);
	H->Magnetics_Method_Applying_Res_Field = MGD77_Get_Text (record, 32, 47, 1);

	/* Process Sequence No 14: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[13], record, MGD77_HEADER_LENGTH);

	H->Gravity_Digitizing_Rate             = MGD77_Get_float (record, 1, 3, 0.1);
	H->Gravity_Sampling_Rate               = MGD77_Get_byte (record, 4, 2, 1);
	H->Gravity_Theoretical_Formula_Code    = MGD77_Get_byte (record, 6, 1, 1);
	H->Gravity_Theoretical_Formula         = MGD77_Get_Text (record, 7, 17, 1);
	H->Gravity_Reference_System_Code       = MGD77_Get_byte (record, 24, 1, 1);
	H->Gravity_Reference_System            = MGD77_Get_Text (record, 25, 16, 1);
	H->Gravity_Corrections_Applied         = MGD77_Get_Text (record, 41, 38, 1);

	/* Process Sequence No 15: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[14], record, MGD77_HEADER_LENGTH);

	H->Gravity_Departure_Base_Station      = MGD77_Get_float (record, 1, 7, 0.1);
	H->Gravity_Departure_Base_Station_Name = MGD77_Get_Text (record, 8, 33, 1);
	H->Gravity_Arrival_Base_Station        = MGD77_Get_float (record, 41, 7, 0.1);
	H->Gravity_Arrival_Base_Station_Name   = MGD77_Get_Text (record, 48, 31, 1);

	/* Process Sequence No 16: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[15], record, MGD77_HEADER_LENGTH);

	H->Number_of_Ten_Degree_Identifiers    = MGD77_Get_byte (record, 1, 2, 1);
	H->Blank_5                             = MGD77_Get_char (record, 3, 1, 1);
	for (i = 0; i < 15; i++) {
		H->Ten_Degree_Identifier_1[i] = MGD77_Get_short (record, 4 + (i * 5), 5, 1);
	}

	/* Process Sequence No 17: */

	if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
	strncpy (H->record[16], record, MGD77_HEADER_LENGTH);

	for (i = 0; i < 15; i++) {
		H->Ten_Degree_Identifier_2[i] = MGD77_Get_short (record, 1 + (i * 5), 5, 1);
	}
	H->Blank_6                             = MGD77_Get_Text (record, 76, 3, 1);

	/* Process Sequence No 18-24: */

	for (i = 0; i < 7; i++) {
		if ((err = MGD77_Read_Header_Sequence (F->fp, record, ++sequence, fmt))) return (err);
		strncpy (H->record[17+i], record, MGD77_HEADER_LENGTH);
		H->Additional_Documentation[i] = MGD77_Get_Text (record, 1, 78, 1);
	}
	return (0);	/* Success, it seems */
}

int MGD77_Read_Header_Record_Binary (struct MGD77_CONTROL *F, struct MGD77_HEADER_RECORD *H)  /* Will read the entire 24-section header structure */
{
	int i;
	for (i = 0; i < MGD77_N_HEADER_RECORDS; i++) {
		memset ((void *)H->record[i], '\0', MGD77_HEADER_LENGTH+1);
		if (fread (H->record[i], sizeof (char), MGD77_HEADER_LENGTH, F->fp) != MGD77_HEADER_LENGTH) return (MGD77_ERROR_READ_HEADER_BIN);
	}
	return (0);	/* Success, it seems */
}

int MGD77_Write_Header_Record (struct MGD77_CONTROL *F, struct MGD77_HEADER_RECORD *H)  /* Will echo the original 24 records */
{	/* Writes records using original text records directly */
	int i, err;
	
	if (F->binary) {
		for (i = 0; i < MGD77_N_HEADER_RECORDS; i++) if (fwrite (H->record[i], sizeof (char), MGD77_HEADER_LENGTH, F->fp) != MGD77_HEADER_LENGTH) return (MGD77_ERROR_WRITE_HEADER_BIN);
	}
	else {
		for (i = 0; i < MGD77_N_HEADER_RECORDS; i++) if (fprintf (F->fp, "%80s\n", H->record[i]) < 0) return (MGD77_ERROR_WRITE_HEADER_ASC);
	}
	return (0);	/* Success */
}

int MGD77_Write_Header_Record_New (FILE *fp, struct MGD77_HEADER_RECORD *H, int fmt)  /* Will write the entire 24-section header structure */
{	/* Writes records from content of structure members */
	int i, sequence = 0;

	/* Write Sequence No 01: */

	MGD77_fmt_no = 0;
	MGD77_Put_byte (fp, H->Record_Type, 1, 1, 0);
	MGD77_Put_Text (fp, H->Cruise_Identifier, 8, 1, 0);
	MGD77_Put_Text (fp, H->Format_Acronym, 5, 1, 0);
	MGD77_Put_int (fp, H->Data_Center_File_Number, 8, 1, 0);
	MGD77_Put_Text (fp, H->Blank_1, 4, 1, 0);
	for (i = 0; i < 5; i++) {
		MGD77_Put_byte (fp, H->Paramaters_Surveyed_Code[i], 1, 1, 0);
	}
	MGD77_Put_short (fp, H->File_Creation_Year, 4, 1, 0);
	MGD77_Put_byte (fp, H->File_Creation_Month, 2, 1, 0);
	MGD77_Put_byte (fp, H->File_Creation_Day, 2, 1, 0);
	MGD77_Put_Text (fp, H->Contributing_Institution, 39, 1, 0);

	/* Write Sequence No 02: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_Text (fp, H->Country, 18, 1, 0);
	MGD77_Put_Text (fp, H->Platform_Name, 21, 1, 0);
	MGD77_Put_byte (fp, H->Platform_Type_Code, 1, 1, 0);
	MGD77_Put_Text (fp, H->Platform_Type, 6, 1, 0);
	MGD77_Put_Text (fp, H->Chief_Scientist, 32, 1, 0);

	/* Write Sequence No 03: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_Text (fp, H->Project_Cruise_Leg, 58, 1, 0);
	MGD77_Put_Text (fp, H->Funding, 20, 1, 0);

	/* Write Sequence No 04: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_short (fp, H->Survey_Departure_Year, 4, 1, 0);
	MGD77_Put_byte (fp, H->Survey_Departure_Month, 2, 1, 0);
	MGD77_Put_byte (fp, H->Survey_Departure_Day, 2, 1, 0);
	MGD77_Put_Text (fp, H->Port_of_Departure, 32, 1, 0);
	MGD77_Put_short (fp, H->Survey_Arrival_Year, 4, 1, 0);
	MGD77_Put_byte (fp, H->Survey_Arrival_Month, 2, 1, 0);
	MGD77_Put_byte (fp, H->Survey_Arrival_Day, 2, 1, 0);
	MGD77_Put_Text (fp, H->Port_of_Arrival, 30, 1, 0);

	/* Write Sequence No 05: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_Text (fp, H->Navigation_Instrumentation, 40, 1, 0);
	MGD77_Put_Text (fp, H->Position_Determination_Method, 38, 1, 0);

	/* Write Sequence No 06: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_Text (fp, H->Bathymetry_Instrumentation, 40, 1, 0);
	MGD77_Put_Text (fp, H->Bathymetry_Add_Forms_of_Data, 38, 1, 0);

	/* Write Sequence No 07: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_Text (fp, H->Magnetics_Instrumentation, 40, 1, 0);
	MGD77_Put_Text (fp, H->Magnetics_Add_Forms_of_Data, 38, 1, 0);

	/* Write Sequence No 08: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_Text (fp, H->Gravity_Instrumentation, 40, 1, 0);
	MGD77_Put_Text (fp, H->Gravity_Add_Forms_of_Data, 38, 1, 0);

	/* Write Sequence No 09: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_Text (fp, H->Seismic_Instrumentation, 40, 1, 0);
	MGD77_Put_Text (fp, H->Seismic_Add_Forms_of_Data, 38, 1, 0);

	/* Write Sequence No 10: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_char (fp, H->Format_Type, 1, 1, 0);
	MGD77_Put_Text (fp, H->Format_Description_1, 74, 1, 0);
	MGD77_Put_Text (fp, H->Blank_2, 3, 1, 0);

	/* Write Sequence No 11: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_Text (fp, H->Format_Description_2, 17, 1, 0);
	MGD77_Put_Text (fp, H->Blank_3, 23, 1, 0);
	MGD77_Put_short (fp, H->Topmost_Latitude, 3, 1, 1);
	MGD77_Put_short (fp, H->Bottommost_Latitude, 3, 1, 1);
	MGD77_Put_short (fp, H->Leftmost_Longitude, 4, 1, 1);
	MGD77_Put_short (fp, H->Rightmost_Longitude, 4, 1, 1);
	MGD77_Put_Text (fp, H->Blank_4, 24, 1, 0);

	/* Write Sequence No 12: */

	MGD77_fmt_no = 1;
	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_float (fp, H->Bathymetry_Digitizing_Rate, 3, 0.1, 0);
	MGD77_Put_Text (fp, H->Bathymetry_Sampling_Rate, 12, 1, 0);
	MGD77_Put_float (fp, H->Bathymetry_Assumed_Sound_Velocity, 5, 0.1, 0);
	MGD77_Put_byte (fp, H->Bathymetry_Datum_Code, 2, 1, 0);
	MGD77_Put_Text (fp, H->Bathymetry_Interpolation_Scheme, 56, 1, 0);

	/* Write Sequence No 13: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_float (fp, H->Magnetics_Digitizing_Rate, 3, 0.1, 0);
	MGD77_Put_byte (fp, H->Magnetics_Sampling_Rate, 2, 1, 0);
	MGD77_Put_short (fp, H->Magnetics_Sensor_Tow_Distance, 4, 1, 0);
	MGD77_Put_float (fp, H->Magnetics_Sensor_Depth, 5, 0.1, 0);
	MGD77_Put_short (fp, H->Magnetics_Sensor_Separation, 3, 1, 0);
	MGD77_Put_byte (fp, H->Magnetics_Ref_Field_Code, 2, 1, 0);
	MGD77_Put_Text (fp, H->Magnetics_Ref_Field, 12, 1, 0);
	MGD77_Put_Text (fp, H->Magnetics_Method_Applying_Res_Field, 47, 1, 0);

	/* Write Sequence No 14: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_float (fp, H->Gravity_Digitizing_Rate, 3, 0.1, 0);
	MGD77_Put_byte (fp, H->Gravity_Sampling_Rate, 2, 1, 0);
	MGD77_Put_byte (fp, H->Gravity_Theoretical_Formula_Code, 1, 1, 0);
	MGD77_Put_Text (fp, H->Gravity_Theoretical_Formula, 17, 1, 0);
	MGD77_Put_byte (fp, H->Gravity_Reference_System_Code, 1, 1, 0);
	MGD77_Put_Text (fp, H->Gravity_Reference_System, 16, 1, 0);
	MGD77_Put_Text (fp, H->Gravity_Corrections_Applied, 38, 1, 0);

	/* Write Sequence No 15: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_float (fp, H->Gravity_Departure_Base_Station, 7, 0.1, 0);
	MGD77_Put_Text (fp, H->Gravity_Departure_Base_Station_Name, 33, 1, 0);
	MGD77_Put_float (fp, H->Gravity_Arrival_Base_Station, 7, 0.1, 0);
	MGD77_Put_Text (fp, H->Gravity_Arrival_Base_Station_Name, 31, 1, 0);

	/* Write Sequence No 16: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	MGD77_Put_byte (fp, H->Number_of_Ten_Degree_Identifiers, 2, 1, 0);
	MGD77_Put_char (fp, H->Blank_5, 1, 1, 0);
	for (i = 0; i < 15; i++) {
		MGD77_Put_short (fp, H->Ten_Degree_Identifier_1[i], 5, 1, 2);
	}

	/* Write Sequence No 17: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	for (i = 0; i < 15; i++) {
		MGD77_Put_short (fp, H->Ten_Degree_Identifier_2[i], 5, 1, 2);
	}
	MGD77_Put_Text (fp, H->Blank_6, 3, 1, 0);

	/* Process Sequence No 18-24: */

	MGD77_Write_Sequence (fp, ++sequence, fmt);
	for (i = 0; i < 7; i++) {
		MGD77_Put_Text (fp, H->Additional_Documentation[i], 78, 1, 0);
		MGD77_Write_Sequence (fp, ++sequence, fmt);
	}
	MGD77_fmt_no = 0;
	
	return (0);	/* Success is assured */
}

/* MGD77_Read_Record_ASCII decodes the MGD77 data record, storing values in a structure of type
 * MGD77_DATA_RECORD (see MGD77.h for structure definition).
 */
int MGD77_Read_Data_Record_ASCII (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *MGD77Record)	  /* Will read a single MGD77 record */
{
	int len, i, k, nwords, value, rata_die, yyyy, mm, dd, nconv;
	char line[BUFSIZ], currentField[10];
	BOOLEAN may_convert;
	double secs, tz;

	if (!(fgets (line, BUFSIZ, F->fp))) return (MGD77_ERROR_READ_ASC_DATA);			/* Try to read one line from the file */

	if (!(line[0] == '3' || line[0] == '5')) return (MGD77_NO_DATA_REC);			/* Only process data records */

	GMT_chop (line);	/* Get rid of CR or LF */
	
	if ((len = (int)strlen(line)) != MGD77_RECORD_LENGTH) {
		fprintf (stderr, "Incorrect record length (%d), skipped\n",len);
		return (MGD77_WRONG_DATA_REC_LEN);
	}
	
	/* Convert old format to new if necessary */
	if (line[0] == '3') MGD77_Convert_To_New_Format (line);

	MGD77Record->bit_pattern = 0;

	/* DECODE the 27 data fields (24 numerical and 3 strings) and store in MGD77_DATA_RECORD */
	
	for (i = 0; i < MGD77_N_NUMBER_FIELDS; i++) {	/* Do the numerical fields first */
	
		strncpy (currentField, &line[mgd77defs[i].start-1], mgd77defs[i].length);
		currentField[mgd77defs[i].length] = '\0';
		
		may_convert = !(MGD77_this_bit[i] & MGD77_FLOAT_BITS) || strcmp (currentField, mgd77defs[i].not_given);
		if (may_convert) {	/* OK, we need to decode the value and scale it according to factor */
			MGD77Record->bit_pattern |= (1 << i);	/* Turn on this bit */
			if ((nconv = sscanf (currentField, mgd77defs[i].readMGD77, &value)) != 1) return (MGD77_ERROR_CONV_DATA_REC);
			MGD77Record->number[i] = ((double) value) / mgd77defs[i].factor;
		}
		else 	/* Geophysical observation absent, assign NaN (assign NaN to unspecified time values??) */
			MGD77Record->number[i] = MGD77_NaN;
	}

	for (i = MGD77_N_NUMBER_FIELDS, nwords = 0; i < MGD77_N_DATA_FIELDS; i++, nwords++) {	/* Do the last 3 string fields */
	
		strncpy (currentField,&line[mgd77defs[i].start-1],mgd77defs[i].length);
		currentField[mgd77defs[i].length] = '\0';

		may_convert = (strncmp(currentField, ALL_NINES, mgd77defs[i].length));
		if (may_convert) {		/* Turn on this data bit */
			MGD77Record->bit_pattern |= (1 << i);
		}
		if (MGD77_Strip_Blanks) {	/* Remove leading and trailing blanks - may lead to empty string */
			k = strlen (currentField) - 1;
			while (k >= 0 && currentField[k] == ' ') k--;
			currentField[++k] = '\0';	/* No longer any trailing blanks */
			k = 0;
			while (currentField[k] && currentField[k] == ' ') k++;	/* Wind past any leading blanks */
			strcpy (MGD77Record->word[nwords], &currentField[k]);	/* Just copy text without changing it at all */
		}
		else
			strcpy (MGD77Record->word[nwords], currentField);	/* Just copy text without changing it at all */
	}

	/* Get absolute time, if all the pieces are there */
	
	if ((MGD77Record->bit_pattern & MGD77_TIME_BITS) == MGD77_TIME_BITS) {	/* Got all the time items */
		yyyy = irint (MGD77Record->number[2]);
		mm = irint (MGD77Record->number[3]);
		dd = irint (MGD77Record->number[4]);
		rata_die = GMT_rd_from_gymd (yyyy, mm, dd);
		tz = (GMT_is_dnan (MGD77Record->number[1])) ? 0.0 : MGD77Record->number[1];
		secs = GMT_HR2SEC_I * (MGD77Record->number[5] + tz) + GMT_MIN2SEC_I * MGD77Record->number[6];
		MGD77Record->time = GMT_rdc2dt (rata_die, secs);
	}
	else	/* Not present or incomplete, assign NaN */
		MGD77Record->time = MGD77_NaN;
	
	return (0);
}

int MGD77_Write_Data_Record (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *MGD77Record)	/* Will write a single MGD77 record */
{
	int error;
	
	error = (F->binary) ? MGD77_Write_Data_Record_Binary (F, MGD77Record) : MGD77_Write_Data_Record_ASCII (F, MGD77Record);
	return (error);
}

/* MGD77_Write_Data_Record writes the MGD77_DATA_RECORD structure, printing stored values in original MGD77 format.
 */
int MGD77_Write_Data_Record_ASCII (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *MGD77Record)	/* Will write a single ASCII MGD77 record */
{
	int nwords = 0, nvalues = 0, i;

	for (i = 0; i < MGD77_N_DATA_FIELDS; i++) {
		if (i == 1) fprintf (F->fp, mgd77defs[24].printMGD77, MGD77Record->word[nwords++]);
		else if (i == 24 || i == 25) fprintf (F->fp, mgd77defs[i+1].printMGD77, MGD77Record->word[nwords++]);
		else {
			if (GMT_is_dnan (MGD77Record->number[nvalues]))	fprintf (F->fp, "%s", mgd77defs[nvalues].not_given);
			else fprintf (F->fp, mgd77defs[nvalues].printMGD77, irint (MGD77Record->number[nvalues]*mgd77defs[nvalues].factor));
			nvalues++;
		}
	}
	fprintf (F->fp, "\n");
	return (0);
}

int MGD77_View_Line (FILE *fp, char *MGD77line)	/* View a single MGD77 string */
{
/*	char line[MGD77_RECORD_LENGTH];
	strcpy (MGD77line,line); */
	if (!(fgets (MGD77line, BUFSIZ, fp))) return FALSE;	/* Read one line from the file */
	if (!(fputs (MGD77line, fp))) return FALSE;		/* Put the line back on the stream */
	return TRUE;
}

int MGD77_Convert_To_New_Format(char *line)
{
	int yy, nconv;

	if (line[0] != '3') return FALSE;

	/* Fix DRT and Time Zone Corrector */
	line[0] = '5';
	line[10] = line[12]; 
	line[11] = line[13];

	/* Fix year - Y2K Kludge Fix */
	if ((nconv = sscanf (&line[14], "%2d", &yy)) != 1)	return FALSE;
	if (yy == 99 && !strncmp(&line[16],"99999999999",11)) {
		line[12] = '9';
		line[13] = '9';
	} else {
		if (yy < MGD77_OLDEST_YY) {
			line[12] = '2';
			line[13] = '0';
		} else {
			line[12] = '1';
			line[13] = '9';
		}
	}
	return TRUE;
}

int MGD77_Convert_To_Old_Format(char *newFormatLine, char *oldFormatLine)
{
	int tz; char legid[9], s_tz[6], s_year[5];
	
	if (newFormatLine[0] != '5') return FALSE;
	strncpy (legid, &oldFormatLine[mgd77defs[1].start-1], mgd77defs[1].length);
	tz = atoi (strncpy(s_tz, &newFormatLine[mgd77defs[2].start-1], mgd77defs[2].length));
	strncpy(s_year, &newFormatLine[mgd77defs[3].start-1], mgd77defs[3].length);
	if (tz == 99) tz = 9999;  /* Handle the empty case */
	else tz *= 100;
	sprintf (oldFormatLine,"3%s%+05d%2d%s", legid, tz, *(s_year + 2), (newFormatLine + mgd77defs[4].start-1));
	return TRUE;
}

/* Internal decoding functions.  Note the position refers to the original FORTRAN positions starting at 1 (not 0) */

float MGD77_Get_float (char *record, int pos, int length, double scale)
{
	float value;
	char keep;
	pos--;	/* Adjust for C array start */
	keep = record[pos+length];
	record[pos+length] = 0;
	if (!strncmp (&record[pos], ALL_BLANKS, length))	/* Found just a blank string, set to NaN or equivalent */
		value = (float) MGD77_NaN;
	else
		value = (float) (atof (&record[pos]) * scale);
	record[pos+length] = keep;
	return (value);
}

short MGD77_Get_short (char *record, int pos, int length, double scale)
{
	short value;
	char keep;
	pos--;	/* Adjust for C array start */
	keep = record[pos+length];
	record[pos+length] = 0;
	if (!strncmp (&record[pos], ALL_BLANKS, length))	/* Found just a blank string, set to NaN or equivalent */
		value = SHRT_MAX;
	else
		value = (short) atoi (&record[pos]);
	record[pos+length] = keep;
	return (value);
}


int MGD77_Get_int (char *record, int pos, int length, double scale)
{
	int value;
	char keep;
	pos--;	/* Adjust for C array start */
	keep = record[pos+length];
	record[pos+length] = 0;
	if (!strncmp (&record[pos], ALL_BLANKS, length))	/* Found just a blank string, set to NaN or equivalent */
		value = INT_MAX;
	else
		value = atoi (&record[pos]);
	record[pos+length] = keep;
	return (value);
}

byte MGD77_Get_byte (char *record, int pos, int length, double scale) 
{
	byte value;
	char keep;
	pos--;	/* Adjust for C array start */
	keep = record[pos+length];
	record[pos+length] = 0;
	if (!strncmp (&record[pos], ALL_BLANKS, length))	/* Found just a blank string, set to NaN or equivalent */
		value = CHAR_MAX;
	else
		value = (byte) atoi (&record[pos]);
	record[pos+length] = keep;
	return (value);
}

char MGD77_Get_char (char *record, int pos, int length, double scale)
{
	return (record[pos-1]);
}

Text MGD77_Get_Text (char *record, int pos, int length, double scale)
{
	int len;
	Text value;
	pos--;	/* Adjust for C array start */
	len = length + 1;	/* Because of the terminating 0 */
	if ((value = calloc ((size_t) len, sizeof (char))) == NULL) {
		fprintf (stderr, "MGD77: Allocation error in MGD77_Get_Byte for %d bytes\n", len);
		exit (EXIT_FAILURE);
	}
	strncpy (value, &record[pos], length);
	return (value);
}

void MGD77_Put_blanks (FILE *fp, int length)
{
	int i;
	for (i = 0; i < length; i++) putc (' ', fp);
}

void MGD77_Put_float (FILE *fp, float f, int length, double scale, int sign)
{
	if (GMT_is_fnan (f))
		MGD77_Put_blanks (fp, length);
	else
		MGD77_Put_int (fp, (int)irint (f / scale), length, scale, sign);
}

void MGD77_Put_int (FILE *fp, int i, int length, double scale, int sign)
{
	if (i == INT_MAX)
		MGD77_Put_blanks (fp, length);
	else {
		if (sign == 1) {	/* Must encode the sign explicitly as - or + */
			if (i < 0)
				fprintf (fp, "-");
			else
				fprintf (fp, "+");
			length--;
		}
		if (sign == 2) {	/* Special flag to use one less and append a comma */
			length--;
			fprintf (fp, MGD77_fmt[MGD77_fmt_no][length], abs(i));
			fprintf (fp, ",");
		}
		else
			fprintf (fp, MGD77_fmt[MGD77_fmt_no][length], abs(i));
	}
}

void MGD77_Put_short (FILE *fp, short s, int length, double scale, int sign)
{
	if (s == SHRT_MAX)
		MGD77_Put_blanks (fp, length);
	else
		MGD77_Put_int (fp, (int)s, length, scale, sign);
}

void MGD77_Put_byte (FILE *fp, byte b, int length, double scale, int sign)
{
	if (b == CHAR_MAX)
		MGD77_Put_blanks (fp, length);
	else
		MGD77_Put_int (fp, (int)b, length, scale, sign);
}

void MGD77_Put_char (FILE *fp, char c, int length, double scale, int sign)
{
	fprintf (fp, "%c", c);
}

void MGD77_Put_Text (FILE *fp, Text t, int length, double scale, int sign)
{
	fprintf (fp, "%s", t);
}


int MGD77_Read_Header_Sequence (FILE *fp, char *record, int seq, int format)
{
	int got;
	
	if (format == MGD77_FORMAT_ASC) {
		if (seq == 1) {	/* Check for MGD77 file header */
			got = fgetc (fp);		/* Read the first character from the file stream */
			ungetc (got, fp);		/* Put the character back on the stream */
			if (got != '4') {
				fprintf (stderr, "MGD77_Read_Header: No header record present\n");
				return (MGD77_NO_HEADER_REC);
			}
		}
		if (fgets (record, MGD77_RECORD_LENGTH, fp) == NULL) {
			fprintf (stderr, "MGD77_Read_Header: Failure to read header sequence %2.2d\n", seq);
			return (MGD77_ERROR_READ_HEADER_ASC);
		}
		GMT_chop (record);
	}
	else {
		if (fread (record, sizeof (char), MGD77_RECORD_LENGTH, fp) != MGD77_RECORD_LENGTH) {
			fprintf (stderr, "MGD77_Read_Header: Failure to read binary header sequence %2.2d\n", seq);
			return (MGD77_ERROR_READ_HEADER_BIN);
		}
		record[MGD77_RECORD_LENGTH] = '\0';
	}
	
	got = atoi (&record[78]);
	if (got != seq) {
		fprintf (stderr, "MGD77_Read_Header: Expected header sequence %2.2d says it is %2.2d\n", seq, got);
		return (MGD77_WRONG_HEADER_REC);
	}
	return (0);
}

int MGD77_Read_Data_Sequence (FILE *fp, char *record)
{
	MGD77_rec_no++;
	if (fgets (record, MGD77_RECORD_LENGTH, fp)) return (1);
	return (0);
}

void MGD77_Write_Sequence (FILE *fp, int seq, int format)
{
	if (format == MGD77_FORMAT_ASC) {
		if (seq > 0) fprintf (fp, "%2.2d", seq);
		fprintf (fp, "\n");
	}
	else {
		char txt[3];
		sprintf (txt, "%2.2d", seq);
		txt[2] = '\0';
		fwrite ((void *)txt, sizeof (char), 2, fp);
	}
}

void MGD77_Init (struct MGD77_CONTROL *F, BOOLEAN remove_blanks)
{
	/* Initialize MGD77 control system */
	int i;
	memset ((void *)F, 0, sizeof (struct MGD77_CONTROL));		/* Initialize structure */
	MGD77_Path_Init (F);
	MGD77_Init_Columns (F);
	GMT_make_dnan (MGD77_NaN);
	for (i = 0; i < 32; i++) MGD77_this_bit[i] = 1 << i;
	MGD77_Strip_Blanks = remove_blanks;
	gmtdefs.time_system = 4;	/* Use UNIX time as rtime */
}

void MGD77_Init_Columns (struct MGD77_CONTROL *F)
{
	/* Initializes the output columns to equal all the input columns
	 * and using the original order.  To change this the program must
	 * call MGD77_Select_Columns.
	 */
	
	int i, j;

	F->n_out_columns = 25;
	F->bit_pattern = 0;
	F->time_format = GMT_IS_ABSTIME;	/* Default time format is calendar time */
	for (i = 0; i < F->n_out_columns; i++) {
		j = i + 2;		/* Start at 2 since we use 2 = time, 3 = dist, 4 = az, 5 = vel, 6 = weight */
		F->use_column[j] = TRUE;
		F->order[i] = j;
		F->bit_pattern |= (1 << j);		/* Turn on this bit */
	}
	/* Initialize pointers to limit tests */
	
	MGD77_column_test_double[MGD77_EQ]   = MGD77_eq_test;
	MGD77_column_test_double[MGD77_NEQ]  = MGD77_neq_test;
	MGD77_column_test_double[MGD77_LT]   = MGD77_lt_test;
	MGD77_column_test_double[MGD77_LE]   = MGD77_le_test;
	MGD77_column_test_double[MGD77_GE]   = MGD77_ge_test;
	MGD77_column_test_double[MGD77_GT]   = MGD77_gt_test;
	MGD77_column_test_string[MGD77_EQ]   = MGD77_ceq_test;
	MGD77_column_test_string[MGD77_NEQ]  = MGD77_cneq_test;
	MGD77_column_test_string[MGD77_LT]   = MGD77_clt_test;
	MGD77_column_test_string[MGD77_LE]   = MGD77_cle_test;
	MGD77_column_test_string[MGD77_GE]   = MGD77_cge_test;
	MGD77_column_test_string[MGD77_GT]   = MGD77_cgt_test;
}

void MGD77_Select_Columns (char *string, struct MGD77_CONTROL *F)
{
	/* Scan the -Fstring and select which columns to use and which order
	 * they should appear on output.  Default is all columns and the same
	 * order as in the input records.
	 */

	char p[BUFSIZ], word[GMT_LONG_TEXT], value[GMT_LONG_TEXT];
	int i, j, k, constraint, n, pos = 0, ne_alloc = 0, nc_alloc = 0;
	BOOLEAN exact;

	memset ((void *)F->use_column, 0, (size_t)(32 * sizeof (int)));		/* Initialize array */
	memset ((void *)F->order, 0, (size_t)(32 * sizeof (int)));		/* Initialize array */
	F->bit_pattern = 0;

	i = 0;		/* Start at the first ouput column */
	while ((GMT_strtok (string, ",", &pos, p))) {	/* Until we run out of abbreviations */
		/* Must check if we need to break this word into flag[=|<=|>=|<|>value] */
		for (k = constraint = 0; p[k] && constraint == 0; k++) {
			if (p[k] == '>') {
				constraint = MGD77_GT;
				if (p[k+1] == '=') constraint |= MGD77_EQ;
			}
			else if (p[k] == '<') {
				constraint = MGD77_LT;
				if (p[k+1] == '=') constraint |= MGD77_EQ;
			}
			else if (p[k] == '=') {
				constraint = MGD77_EQ;
			}
			else if (p[k] == '!' && p[k+1] == '=') {
				constraint = MGD77_NEQ;
			}
		}
		if (constraint) {	/* Got a constraint, split the p string into word and value */
			strncpy (word, p, k-1);
			word[k-1] = '\0';
			while (p[k] && strchr ("><=!", p[k])) k++;
			strcpy (value, &p[k]);
		}
		else			/* Just copy the word */
			strcpy (word, p);
			
		/* Turn word into lower case if upper case */
		
		n = strlen (word);
		for (j = k = 0; j < n; j++) if (isupper ((int)word[j])) {
			word[j] = tolower ((int)word[j]);
			k++;
		}
		exact = (k == n);			/* TRUE if this constraint must match exactly */
		
		if (!strcmp (word, "time"))		/* Special flag for col 2: time = {year, month, day, hour, min, tz} */
			j = MGD77_TIME;
		else if (!strcmp (word, "atime"))	/* Same */
			j = MGD77_TIME;
		else if (!strcmp (word, "rtime")) {	/* Time relative to EPOCH */
			j = MGD77_TIME;
			F->time_format = GMT_IS_RELTIME;	/* Alternate time format is time relative to EPOCH */
		}
		else if (!strcmp (word, "dist"))	/* Special flag for col 3: ellipsoidal distance in km */
			j = MGD77_DISTANCE;
		else if (!strcmp (word, "edist"))	/* Same */
			j = MGD77_DISTANCE;
		else if (!strcmp (word, "fdist")) {	/* Flat earth approximation (faster) */
			j = MGD77_DISTANCE;
			F->flat_earth = TRUE;
		}
		else if (!strcmp (word, "azim"))	/* Special flag for col 4: ship azimuth in degrees */
			j = MGD77_HEADING;
		else if (!strcmp (word, "vel"))	/* Special flag for col 5: ship velocity in m/s */
			j = MGD77_SPEED;
		else if (!strcmp (word, "weight"))	/* Special flag for col 6: Data set weight */
			j = MGD77_WEIGHT;
		else {
			j = 0;	/* Search for the matching abbreviation in our list */
			while (j < MGD77_N_DATA_FIELDS && strcmp (word, mgd77defs[j].abbrev)) j++;
			if (j == MGD77_N_DATA_FIELDS) {	/* No match, probably due to a typo.  We will bail out */
				fprintf (stderr, "MGD77_Select_Columns: ERROR: Unknown column abbreviation \"%s\"\n", word);
				exit (EXIT_FAILURE);
			}
		}
		if (F->use_column[j] && !constraint) {	/* Already specified this once before for output */
			fprintf (stderr, "MGD77_Select_Columns: ERROR: Abbreviation \"%s\" given more than once\n", word);
			exit (EXIT_FAILURE);
		}
		
		/* OK, here we are ready to update the structure */
		
		if (constraint) {	/* Got a column constraint */
			if (F->n_constraints == nc_alloc) {
				nc_alloc += GMT_SMALL_CHUNK;
				F->Constraint = (struct MGD77_CONSTRAINT *)GMT_memory ((void *)F->Constraint, nc_alloc, sizeof (struct MGD77_CONSTRAINT), "MGD77_Select_Columns");
			}
			if (j < MGD77_N_NUMBER_FIELDS) {	/* Floating point constraint */
				F->Constraint[F->n_constraints].d_constraint = (!strcmp (value, "NaN")) ? MGD77_NaN : atof (value);
				F->Constraint[F->n_constraints].double_test = MGD77_column_test_double[constraint];
			}
			else {
				k = j-MGD77_N_NUMBER_FIELDS;
				F->Constraint[F->n_constraints].c_constraint = (char *)GMT_memory (VNULL, (size_t)(strlen (value) + 1), 1, "MGD77_Select_Columns");
				strcpy (F->Constraint[F->n_constraints].c_constraint, value);
				F->Constraint[F->n_constraints].string_test = MGD77_column_test_string[constraint];
			}
			F->Constraint[F->n_constraints].col = j;
			F->Constraint[F->n_constraints].exact = exact;
			F->n_constraints++;
		}
		else {			/* Got an output column specification */
			F->order[i] = j;
			F->use_column[j] = TRUE;		/* We are using this column on output */
			F->bit_pattern |= (1 << j);		/* Turn on this bit */
			if (exact && (MGD77_this_bit[j] & MGD77_GEOPHYSICAL_BITS)) {		/* This geophysical column must be != NaN for us to output record */
				if (F->n_exact == ne_alloc) {
					ne_alloc += GMT_SMALL_CHUNK;
					F->exact = (int *) GMT_memory ((void *)F->exact, ne_alloc, sizeof (int), "MGD77_Select_Columns");
				}
				F->exact[F->n_exact] = j;
				F->n_exact++;
			}
			else if (exact) {
				fprintf (stderr, "MGD77_Select_Columns: WARNING: Abbreviation \"%s\" not a geophysical observation and cannot be met exactly\n", word);
			}
			i++;					/* Move to the next output column */
		}
	}

	F->n_out_columns = i;
	if (F->n_constraints > 0) F->Constraint = (struct MGD77_CONSTRAINT *)GMT_memory ((void *)F->Constraint, F->n_constraints, sizeof (struct MGD77_CONSTRAINT), "MGD77_Select_Columns");
	F->no_checking = (F->n_constraints == 0 && F->n_exact == 0);	/* Easy street */
}

void MGD77_Set_Home (struct MGD77_CONTROL *F)
{
	char *this;

	if (F->MGD77_HOME) return;	/* Already set elsewhere */

	if ((this = getenv ("MGD77_HOME")) == CNULL) {
		if ((this = getenv ("GMTHOME")) != CNULL) {
			fprintf (stderr, "mgd77: Warning: MGD77_HOME not defined, set to $GMTHOME/share/mgd77\n");
			F->MGD77_HOME = (char *) GMT_memory (VNULL, (size_t)(strlen (this) + 13), 1, "MGD77_Set_Home");
			sprintf (F->MGD77_HOME, "%s/share/mgd77", this);
		}
		else {
			fprintf (stderr, "mgd77: ERROR: Neither MGD77_HOME or GMTHOME defined - give up\n");
			exit (EXIT_FAILURE);
		}
	}
	else {	/* Set default path */
		F->MGD77_HOME = (char *) GMT_memory (VNULL, (size_t)(strlen (this) + 1), 1, "MGD77_Set_Home");
		strcpy (F->MGD77_HOME, this);
	}
}

void MGD77_Path_Init (struct MGD77_CONTROL *F)
{
	int i;
	size_t n_alloc = GMT_SMALL_CHUNK;
	char file[BUFSIZ], line[BUFSIZ];
	FILE *fp;
	
	MGD77_Set_Home (F);

	sprintf (file, "%s%cmgd77_paths.txt", F->MGD77_HOME, DIR_DELIM);
	
	F->n_MGD77_paths = 0;

	if ((fp = fopen (file, "r")) == NULL) {
		fprintf (stderr, "mgd77: Warning: path file %s for MGD77 files not found\n", file);
		fprintf (stderr, "mgd77: (Will only look in current directory for such files)\n");
		return;
	}
	
	F->MGD77_datadir = (char **) GMT_memory (VNULL, n_alloc, sizeof (char *), "MGD77_path_init");
	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Comments */
		if (line[0] == ' ' || line[0] == '\0') continue;	/* Blank line, \n included in count */
		GMT_chop (line);
		F->MGD77_datadir[F->n_MGD77_paths] = GMT_memory (VNULL, (size_t)1, (size_t)(strlen (line)+1), "MGD77_path_init");
#if _WIN32
		for (i = 0; line[i]; i++) if (line[i] == '/') line[i] = DIR_DELIM;
#else
		for (i = 0; line[i]; i++) if (line[i] == '\\') line[i] = DIR_DELIM;
#endif
		strcpy (F->MGD77_datadir[F->n_MGD77_paths], line);
		F->n_MGD77_paths++;
		if (F->n_MGD77_paths == (int)n_alloc) {
			n_alloc += GMT_SMALL_CHUNK;
			F->MGD77_datadir = (char **) GMT_memory ((void *)F->MGD77_datadir, n_alloc, sizeof (char *), "MGD77_path_init");
		}
	}
	fclose (fp);
	F->MGD77_datadir = (char **) GMT_memory ((void *)F->MGD77_datadir, (size_t)F->n_MGD77_paths, sizeof (char *), "MGD77_path_init");
}
	
/* MGD77_Get_Path takes a track name as argument and returns the full path
 * to where this data file can be found.  MGD77_path_init must be called first.
 * Return 1 if there is a problem (not found)
 */
 
int MGD77_Get_Path (char *track_path, char *track, struct MGD77_CONTROL *F)
{	/* Assemple proper path to READ a mgd77 file.
	 * track may be:
	 *  a) a complete hardpath, which is copied verbatim to track_path
	 *  b) a local file with extension, which is copied to track_path
	 *  c) a leg name (no extension), in which we try
	 *	- append .mgd77+ and see if we can find it in listed directories
	 *      - append .mgd77 and see if we can find it in listed directories
	 */
	int id, fmt, f_start, f_stop;
	BOOLEAN append = FALSE;
	char geo_path[BUFSIZ], *suffix[2] = {"mgd77", "mgd77+"};
	
	switch (F->format) {
		case 1:	/* Look for MGD77 ASCII files only */
			f_start = f_stop = 0;
			break;
		case 2:	/* Look for MGD77+ binary files only */
			f_start = f_stop = 1;
			break;
		default:
			f_start = 0;
			f_stop = 1;
			break;
	}
	for (fmt = f_start; fmt <= f_stop; fmt++) {	/* Try either on or both formats */
		if (!strstr (track, ".mgd77")) {	/* No extension, must append .mgd77 or .mgd77+ */
			append = TRUE;
			sprintf (geo_path, "%s.%s", track, suffix[fmt]);
		}
		else
			strcpy (geo_path, track);	/* Extension already there */
	
		F->binary = (geo_path[strlen(geo_path)-1] == '+');
	
		if (geo_path[0] == '/' || geo_path[1] == ':') {	/* Hard path given */
			if (!access (geo_path, R_OK)) {	/* OK, found it */
				strcpy (track_path, geo_path);
				return (0);
			}
			else
				return (-1);	/* Hard path did not work */
		}
	
		/* Here we have a relative path.  First look in current directory */
	
		if (!access (geo_path, R_OK)) {	/* OK, found it */
			strcpy (track_path, geo_path);
			return (0);
		}
	
		/* Then look elsewhere */
	
		for (id = 0; id < F->n_MGD77_paths; id++) {
			if (append)
				sprintf (geo_path, "%s%c%s.%s", F->MGD77_datadir[id], DIR_DELIM, track, suffix[fmt]);
			else
				sprintf (geo_path, "%s%c%s", F->MGD77_datadir[id], DIR_DELIM, track);
			if (!access (geo_path, R_OK)) {
				strcpy (track_path, geo_path);
				return (0);
			}
		}
	}
	
	return (1);	/* No luck */
}

BOOLEAN MGD77_pass_record (struct MGD77_DATA_RECORD *H, struct MGD77_CONTROL *F)
{
	int i, col, n_passed;
	BOOLEAN pass;
	
	if (F->no_checking) return (TRUE);	/* Nothing to check for - get outa here */
	
	if (F->n_exact) {	/* Must make sure that these key geophysical columns are ALL present and not NaN */
		for (i = 0; i < F->n_exact; i++) if (GMT_is_dnan (H->number[F->exact[i]])) return (FALSE);	/* Sorry, one mistake and you're history */
	}
	
	if (F->n_constraints) {	/* Must pass all constraints to be successful */
		for (i = n_passed = 0; i < F->n_constraints; i++) {	/* Must pass all constraints to be successful */
			col = F->Constraint[i].col;
			pass = (col < MGD77_N_NUMBER_FIELDS) ? F->Constraint[i].double_test (H->number[col], F->Constraint[i].d_constraint) : F->Constraint[i].string_test (H->word[col-MGD77_N_NUMBER_FIELDS], F->Constraint[i].c_constraint);
			if (pass) {	/* OK, we survived for now, tally up victories and goto next battle */
				n_passed++;
				continue;
			}

			if (F->Constraint[i].exact) return (FALSE);		/* Oops, we failed a must-pass test... */
		}
		return (n_passed > 0);	/* Pass if we passed at least one test, since failing any exact test would have returned by now */
	}
		
	return (TRUE);	/* We live to fight another day */
}

BOOLEAN MGD77_lt_test (double value, double limit)
{
	/* Test that checks for value < limit */
	
	if (GMT_is_dnan (value)) return (FALSE);	/* Cannot pass a test with a NaN */
	return (value < limit);
}

BOOLEAN MGD77_le_test (double value, double limit)
{
	/* Test that checks for value <= limit */
	
	if (GMT_is_dnan (value)) return (FALSE);	/* Cannot pass a test with a NaN */
	return (value <= limit);
}

BOOLEAN MGD77_eq_test (double value, double limit)
{
	/* Test that checks for value == limit */
	
	if (GMT_is_dnan (value) && GMT_is_dnan (limit)) return (TRUE);	/* Matching two NaNs is OK... */
	if (GMT_is_dnan (value) || GMT_is_dnan (limit)) return (FALSE);	/* ...but if only one of them is NaN we fail */
	return (value == limit);
}

BOOLEAN MGD77_neq_test (double value, double limit)
{
	/* Test that checks for value != limit */
	
	if (GMT_is_dnan (value) && GMT_is_dnan (limit)) return (FALSE);	/* Both NaNs so we fail */
	if (GMT_is_dnan (value) || GMT_is_dnan (limit)) return (TRUE);	/* ...but if only one of them is NaN it is OK */
	return (value != limit);
}

BOOLEAN MGD77_ge_test (double value, double limit)
{
	/* Test that checks for value >= limit */
	
	if (GMT_is_dnan (value)) return (FALSE);	/* Cannot pass a test with a NaN */
	return (value >= limit);
}

BOOLEAN MGD77_gt_test (double value, double limit)
{
	/* Test that checks for value > limit */
	
	if (GMT_is_dnan (value)) return (FALSE);	/* Cannot pass a test with a NaN */
	return (value > limit);
}

BOOLEAN MGD77_clt_test (char *value, char *match)
{
	/* Test that checks for value < match for strings */
	
	return (strcmp (value, match) < 0);
}

BOOLEAN MGD77_cle_test (char *value, char *match)
{
	/* Test that checks for value <= match for strings */
	
	return (strcmp (value, match) <= 0);
}

BOOLEAN MGD77_ceq_test (char *value, char *match)
{
	/* Test that checks for value == match for strings */
	
	return (strcmp (value, match) == 0);
}

BOOLEAN MGD77_cneq_test (char *value, char *match)
{
	/* Test that checks for value != match for strings */
	
	return (strcmp (value, match) != 0);
}

BOOLEAN MGD77_cge_test (char *value, char *match)
{
	/* Test that checks for value >= match for strings */
	
	return (strcmp (value, match) >= 0);
}

BOOLEAN MGD77_cgt_test (char *value, char *match)
{
	/* Test that checks for value > match for strings */
	
	return (strcmp (value, match) > 0);
}

int MGD77_storage_bin (int constraint)
{	/* Returns the index of the limit bin for this kind of constraint */
	int bin;
	
	switch (constraint & 15) {	/* Knocks off the 16 (upper case) flag */
		case MGD77_LT:
		case MGD77_LE:
			bin = 0;
			break;
		case MGD77_EQ:
		case MGD77_NEQ:
			bin = 1;
			break;
		case MGD77_GE:
		case MGD77_GT:
			bin = 2;
			break;
		default:
			bin = 3;
			break;
	}
	
	return (bin);
}

void MGD77_set_unit (char *dist, double *scale)
{
	switch (dist[strlen(dist)-1]) {
		case 'k':	/* km */
			*scale = 1000.0;
			break;
		case 'm':	/* miles */
			*scale = MGD77_METERS_PER_M;
			break;
		case 'n':	/* nautical miles */
			*scale = MGD77_METERS_PER_NM;
			break;
		default:
			*scale = 1.0;
			break;
	}
}

void MGD77_Fatal_Error (int error)
{
	fprintf (stderr, "%s: Error [%d]: ", GMT_program, error);
	switch (error) {
		case MGD77_NO_HEADER_REC:
			fprintf (stderr, "Header record not found");
			break;
		case MGD77_ERROR_READ_HEADER_ASC:
			fprintf (stderr, "Error reading ASCII header record");
			break;
		case MGD77_ERROR_READ_HEADER_BIN:
			fprintf (stderr, "Error reading binary header record");
			break;
		case MGD77_ERROR_WRITE_HEADER_ASC:
			fprintf (stderr, "Error writing ASCII header record");
			break;
		case MGD77_ERROR_WRITE_HEADER_BIN:
			fprintf (stderr, "Error writing binary header record");
			break;
		case MGD77_WRONG_HEADER_REC:
			fprintf (stderr, "Wrong header record was read");
			break;
		case MGD77_NO_DATA_REC:
			fprintf (stderr, "Data record not found");
			break;
		case MGD77_ERROR_READ_ASC_DATA:
			fprintf (stderr, "Error reading ASCII data record");
			break;
		case MGD77_ERROR_READ_BIN_DATA:
			fprintf (stderr, "Error reading binary data record");
			break;
		case MGD77_ERROR_WRITE_ASC_DATA:
			fprintf (stderr, "Error writing ASCII data record");
			break;
		case MGD77_ERROR_WRITE_BIN_DATA:
			fprintf (stderr, "Error writing binary data record");
			break;
		case MGD77_WRONG_DATA_REC_LEN:
			fprintf (stderr, "Data record has incorrect length");
			break;
		case MGD77_ERROR_CONV_DATA_REC:
			fprintf (stderr, "Error converting a field in current data record");
			break;
		default:
			fprintf (stderr, "Unrecognized error");
			break;
	}
		
	exit (EXIT_FAILURE);
}

/* MGD77+ functions will be added down here */

int MGD77_Write_Data_Record_Binary (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *H)
{
	double t;
	
	/* Write text strings */
	
	if (fwrite ((void *)&H->word[0], sizeof (char),  8, F->fp) != 8) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (fwrite ((void *)&H->word[1], sizeof (short), 5, F->fp) != 5) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (fwrite ((void *)&H->word[2], sizeof (int),   6, F->fp) != 6) return (MGD77_ERROR_WRITE_BIN_DATA);
	
	/* Write 1-char variables */
	
	if (MGD77_fwrite_char (H->number[MGD77_TZ], 1.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_char (H->number[MGD77_PTC], 1.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_char (H->number[MGD77_BCC], 1.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_char (H->number[MGD77_BTC], 1.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_char (H->number[MGD77_MSENS], 1.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_char (H->number[MGD77_NQC], 1.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	t = H->time - floor (H->time);		/* Store fraction of second in 60 ms chunks */
	if (MGD77_fwrite_char (t, 1000.0/60.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	
	/* Write 2-byte variables */
	
	if (MGD77_fwrite_short (H->number[MGD77_MAG], 10.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_short (H->number[MGD77_DIUR], 10.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_short (H->number[MGD77_MSD], 1.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_short (H->number[MGD77_EOT], 10.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_short (H->number[MGD77_FAA], 10.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	
	/* Write 4-byte variables */
	
	t = GMT_usert_from_dt (floor (H->time));
	if (MGD77_fwrite_int (t, 1.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_int (H->number[MGD77_LATITUDE], 1.0e6, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_int (H->number[MGD77_LONGITUDE], 1.0e6, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_int (H->number[MGD77_TWT], 1.0e4, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_int (H->number[MGD77_DEPTH], 10.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_int (H->number[MGD77_MTF1], 10.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_int (H->number[MGD77_MTF2], 10.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	if (MGD77_fwrite_int (H->number[MGD77_GOBS], 10.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	
	return (0);
}

int MGD77_Read_Data_Record_Binary (struct MGD77_CONTROL *F, struct MGD77_DATA_RECORD *H)
{	/* Not written yet */
	double dt, t;
	time_t this_t;
	struct tm *T;
	
	/* Read text strings */
	
	if (fread ((void *)&H->word[0], sizeof (char),  8, F->fp) != 8) return (MGD77_ERROR_READ_BIN_DATA);
	if (fread ((void *)&H->word[1], sizeof (short), 5, F->fp) != 5) return (MGD77_ERROR_READ_BIN_DATA);
	if (fread ((void *)&H->word[2], sizeof (int),   6, F->fp) != 6) return (MGD77_ERROR_READ_BIN_DATA);
	
	/* Read 1-char variables */
	
	if (MGD77_fread_char (&H->number[MGD77_TZ], 1.0, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_char (&H->number[MGD77_PTC], 1.0, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_char (&H->number[MGD77_BCC], 1.0, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_char (&H->number[MGD77_BTC], 1.0, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_char (&H->number[MGD77_MSENS], 1.0, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_char (&H->number[MGD77_NQC], 1.0, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_char (&dt, 60.0/1000.0, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	
	/* Read 2-byte variables */
	
	if (MGD77_fread_short (&H->number[MGD77_MAG], 0.1, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_short (&H->number[MGD77_DIUR], 0.1, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_short (&H->number[MGD77_MSD], 1.0, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_short (&H->number[MGD77_EOT], 0.1, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_short (&H->number[MGD77_FAA], 0.1, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	
	/* Read 4-byte variables */
	
	if (MGD77_fread_int (&H->time, 1.0, F->fp)) return (MGD77_ERROR_WRITE_BIN_DATA);
	this_t = (time_t)(H->time - H->number[MGD77_TZ] * 3600.0);	/* Return to local time */
	T = gmtime (&this_t);			/* Break down local Unix time to day, month etc */
	H->time = GMT_dt_from_usert (H->time + dt);	/* Add in sec fraction and convert to GMT time keeping */
	H->number[2] = (double)(T->tm_year + 1900);
	H->number[3] = (double)(T->tm_mon + 1);
	H->number[4] = (double)(T->tm_mday);
	H->number[5] = (double)(T->tm_hour);
	H->number[6] = (double)(T->tm_min) + (T->tm_sec + dt)/60.0;
	
	if (MGD77_fread_int (&H->number[MGD77_LATITUDE], 1.0e-6, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_int (&H->number[MGD77_LONGITUDE], 1.0e-6, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_int (&H->number[MGD77_TWT], 1.0e-4, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_int (&H->number[MGD77_DEPTH], 0.1, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_int (&H->number[MGD77_MTF1], 0.1, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_int (&H->number[MGD77_MTF2], 0.1, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	if (MGD77_fread_int (&H->number[MGD77_GOBS], 0.1, F->fp)) return (MGD77_ERROR_READ_BIN_DATA);
	
	H->number[MGD77_RECTYPE] = 5.0;	/* Record type */
	return (0);
}

int MGD77_fwrite_char (double value, double scale, FILE *fp)
{
	char c;
	
	c = (GMT_is_dnan (value)) ? CHAR_MAX : (char) irint (value * scale);
	if (fwrite ((void *)&c, sizeof (char), 1, fp) != 1) return (1);
	return (0);
}

int MGD77_fwrite_short (double value, double scale, FILE *fp)
{
	short s;
	
	s = (GMT_is_dnan (value)) ? SHRT_MAX : (short) irint (value * scale);
	if (fwrite ((void *)&s, sizeof (short), 1, fp) != 1) return (1);
	return (0);
}

int MGD77_fwrite_int (double value, double scale, FILE *fp)
{
	int i;
	
	i = (GMT_is_dnan (value)) ? INT_MAX : (int) irint (value * scale);
	if (fwrite ((void *)&i, sizeof (int), 1, fp) != 1) return (1);
	return (0);
}

int MGD77_fread_char (double *value, double scale, FILE *fp)
{
	char c;
	
	if (fread ((void *)&c, sizeof (char), 1, fp) != 1) return (1);
	*value = (c == CHAR_MAX) ? MGD77_NaN : ((double)c) * scale;
	return (0);
}

int MGD77_fread_short (double *value, double scale, FILE *fp)
{
	short s;
	
	if (fread ((void *)&s, sizeof (short), 1, fp) != 1) return (1);
	*value = (s == SHRT_MAX) ? MGD77_NaN : ((double)s) * scale;
	return (0);
}

int MGD77_fread_int (double *value, double scale, FILE *fp)
{
	int i;
	
	if (fread ((void *)&i, sizeof (int), 1, fp) != 1) return (1);
	*value = (i == INT_MAX) ? MGD77_NaN : ((double)i) * scale;
	return (0);
}

