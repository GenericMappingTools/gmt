/*******************************************************************************
	Name:     sacio.h

	Purpose:  structure for header of a SAC (Seismic Analysis Code)
		data file, and prototype for basic SAC I/O

	Notes:
	Key to comment flags describing each field:
		Column 1:
			R       required by SAC
		   (blank)  optional
		Column 2:
			A = settable from a priori knowledge
			D = available in data
			F = available in or derivable from SEED fixed data header
			T = available in SEED header tables
			(blank) = not directly available from SEED data, header
				tables, or elsewhere

	Problems:   none known

	References:
		* O'Neill, D. (1987).  IRIS Interim Data Distribution Format
		  (SAC ASCII), Version 1.0 (12 November 1987).  Incorporated
		  Research Institutions for Seismology, 1616 North Fort Myer
		  Drive, Suite 1440, Arlington, Virginia 22209.  11 pp.
		* Tull, J. (1987).  SAC User's Manual, Version 10.2, October 7,
		  1987.  Lawrence Livermore National Laboratory, L-205,
		  Livermore, California 94550.  ??? pp.

	Language:   C, hopefully ANSI standard

	Author:     Dennis O'Neill

	Revisions:
		07/15/88  Dennis O'Neill  Initial preliminary release 0.9
		11/21/88  Dennis O'Neill  Production release 1.0
		01/27/91  Lorraine Hwang  Header number is now version 6
		07/06/93  Xiaoming Ding   structure name sac -> sac_head
								  typedef structure to be SACHEAD
		12/06/96  Lupei Zhu       prototype sacio functions
		05/04/13  Dongdong Tian   modify headers according to SAC v101.5
*******************************************************************************/

#ifndef _SACIO_H
#define _SACIO_H

/*******************************************************************************
						SAC header structure

	The SAC package is originally implemented in FORTRAN language, and the SAC
	file format follows the conventions of FORTRAN language. Each character
	string has a length of 8 bytes (16 bytes for kevnm).

	While reimplementing SAC in C, an extra byte is needed since C use '\0' to
	mark the termination of a string to avoid wiping out the contents of the
	last character.

	The header structure are needed, one for structure in memory and one for
	reading/writing header structure from/to disk.

	While reading a file, first read in the numeric part to the header
	structure, then read the string part to a temporary buffer, then map
	strings from buffer to header structure.

	While writing a file, first write the numeric part from header structure
	to disk, then map the strings from header structure to temporary buffer,
	then write the buffer to disk.

*******************************************************************************/

typedef struct sac_head {
	float delta;            /* RF increment between evenly spaced samples     */
	float depmin;           /*    minimum value of dependent variable         */
	float depmax;           /*    maximum value of dependent variable         */
	float scale;            /*    amplitude scale factor (not used)           */
	float odelta;           /*    Observed increment                          */
	float b;                /* RD begining value of the independent variable  */
	float e;                /* RD ending value of the independent variable    */
	float o;                /*    event origin time(seconds wrt referece time)*/
	float a;                /*    1st arrival time (seconds wrt referece time)*/
	float internal1;        /*    internal use                                */
	float t0;               /*    user-defined time pick                      */
	float t1;               /*    user-defined time pick                      */
	float t2;               /*    user-defined time pick                      */
	float t3;               /*    user-defined time pick                      */
	float t4;               /*    user-defined time pick                      */
	float t5;               /*    user-defined time pick                      */
	float t6;               /*    user-defined time pick                      */
	float t7;               /*    user-defined time pick                      */
	float t8;               /*    user-defined time pick                      */
	float t9;               /*    user-defined time pick                      */
	float f;                /*    end time of event, sec > 0                  */
	float resp0;            /*    instrument respnse parameter (not used)     */
	float resp1;            /*    instrument respnse parameter (not used)     */
	float resp2;            /*    instrument respnse parameter (not used)     */
	float resp3;            /*    instrument respnse parameter (not used)     */
	float resp4;            /*    instrument respnse parameter (not used)     */
	float resp5;            /*    instrument respnse parameter (not used)     */
	float resp6;            /*    instrument respnse parameter (not used)     */
	float resp7;            /*    instrument respnse parameter (not used)     */
	float resp8;            /*    instrument respnse parameter (not used)     */
	float resp9;            /*    instrument respnse parameter (not used)     */
	float stla;             /*  T station latititude (degree, north positive) */
	float stlo;             /*  T station longitude (degree, east positive)   */
	float stel;             /*  T station elevation (meters, not used)        */
	float stdp;             /*  T station depth (meters, not used)            */
	float evla;             /*    event latitude (degree, north positive)     */
	float evlo;             /*    event longitude (degree, east positive)     */
	float evel;             /*    event elevation (meters, not used)          */
	float evdp;             /*    event depth (kilometer, previously meters)  */
	float mag;              /*    event magnitude                             */
	float user0;            /*    User defined variable storage area          */
	float user1;            /*    User defined variable storage area          */
	float user2;            /*    User defined variable storage area          */
	float user3;            /*    User defined variable storage area          */
	float user4;            /*    User defined variable storage area          */
	float user5;            /*    User defined variable storage area          */
	float user6;            /*    User defined variable storage area          */
	float user7;            /*    User defined variable storage area          */
	float user8;            /*    User defined variable storage area          */
	float user9;            /*    User defined variable storage area          */
	float dist;             /*    station-event distance (km)                 */
	float az;               /*    event-station azimuth                       */
	float baz;              /*    station-event azimuth                       */
	float gcarc;            /*    station-event great arc length (degrees)    */
	float internal2;        /*    internal use                                */
	float internal3;        /*    internal use                                */
	float depmen;           /*    mean value of dependent variable            */
	float cmpaz;            /*  T component azimuth (degree CW from north)    */
	float cmpinc;           /*  T component inclination (degree from vertical)*/
	float xminimum;         /*    minimum value of X (spectral files only)    */
	float xmaximum;         /*    maximum value of X (spectral files only)    */
	float yminimum;         /*    minimum value of Y (spectral files only)    */
	float ymaximun;         /*    maximum value of Y (spectral files only)    */
	float unused1;          /*    reserved for future use                     */
	float unused2;          /*    reserved for future use                     */
	float unused3;          /*    reserved for future use                     */
	float unused4;          /*    reserved for future use                     */
	float unused5;          /*    reserved for future use                     */
	float unused6;          /*    reserved for future use                     */
	float unused7;          /*    reserved for future use                     */
	int   nzyear;           /*  F GMT year corresponding to zero time of file */
	int   nzjday;           /*  F GMT julia day                               */
	int   nzhour;           /*  F GMT hour                                    */
	int   nzmin;            /*  F GMT minite                                  */
	int   nzsec;            /*  F GMT second                                  */
	int   nzmsec;           /*  F GMT millisecond                             */
	int   nvhdr;            /* R  header version number (6)                   */
	int   norid;            /*    origin ID (CSS 3.0)                         */
	int   nevid;            /*    event ID (CSS 3.0)                          */
	int   npts;             /* RF number of points per data component         */
	int   internal4;        /*    internal use                                */
	int   nwfid;            /*    waveform ID (CSS 3.0)                       */
	int   nxsize;           /*    spectral length (spectral files only)       */
	int   nysize;           /*    spectral width (spectral files only)        */
	int   unused8;          /*    reserved for future use                     */
	int   iftype;           /* RA type of file                                */
	int   idep;             /*    type of dependent variable                  */
	int   iztype;           /*    reference time equivalence                  */
	int   unused9;          /*    reserved for future use                     */
	int   iinst;            /*    type of recording instrument (not used)     */
	int   istreg;           /*    station geographic region (not used)        */
	int   ievreg;           /*    event geographic region (not used)          */
	int   ievtyp;           /*    type of event                               */
	int   iqual;            /*    quality of data (not used)                  */
	int   isynth;           /*    synthetic data flag (not used)              */
	int   imagtyp;          /*    magnitude type                              */
	int   imagsrc;          /*    source of magnitude information             */
	int   unused10;         /*    reserved for future use                     */
	int   unused11;         /*    reserved for future use                     */
	int   unused12;         /*    reserved for future use                     */
	int   unused13;         /*    reserved for future use                     */
	int   unused14;         /*    reserved for future use                     */
	int   unused15;         /*    reserved for future use                     */
	int   unused16;         /*    reserved for future use                     */
	int   unused17;         /*    reserved for future use                     */
	int   leven;            /* RA true if data is evenly spaced               */
	int   lpspol;           /*    station polarity flag (left hand rule)      */
	int   lovrok;           /*    overwrite permission                        */
	int   lcalda;           /*    true if to calculate distance, azimuth      */
	int   unused18;         /*    reserved for future use                     */
	char  kstnm[9];         /*  F station name                                */
	char  kevnm[18];        /*    event name                                  */
	char  khole[9];         /*    nuclear: hole id; Other: location id;       */
	char  ko[9];            /*    event origin time id                        */
	char  ka[9];            /*    1st arrival time id                         */
	char  kt0[9];           /*    time pick 0 id                              */
	char  kt1[9];           /*    time pick 1 id                              */
	char  kt2[9];           /*    time pick 2 id                              */
	char  kt3[9];           /*    time pick 3 id                              */
	char  kt4[9];           /*    time pick 4 id                              */
	char  kt5[9];           /*    time pick 5 id                              */
	char  kt6[9];           /*    time pick 6 id                              */
	char  kt7[9];           /*    time pick 7 id                              */
	char  kt8[9];           /*    time pick 8 id                              */
	char  kt9[9];           /*    time pick 9 id                              */
	char  kf[9];            /*    end of event id                             */
	char  kuser0[9];        /*    User defined variable storage area          */
	char  kuser1[9];        /*    User defined variable storage area          */
	char  kuser2[9];        /*    User defined variable storage area          */
	char  kcmpnm[9];        /*  F channel name, three charaters               */
	char  knetwk[9];        /*    name of seismic network                     */
	char  kdatrd[9];        /*    date data was read onto computer            */
	char  kinst[9];         /*    generic name of recording instrument        */
} SACHEAD;

/*******************************************************************************
						  SAC Enumerated tyep

	Definitions of constants for SAC enumerated data values.

	undocumented == couldn't find a definition for it (denio, 07/15/88)
*******************************************************************************/
#define ITIME   1       /* file: time series data    */
#define IRLIM   2       /* file: real&imag spectrum  */
#define IAMPH   3       /* file: ampl&phas spectrum  */
#define IXY     4       /* file: gen'l x vs y data   */
#define IUNKN   5       /* x data: unknown type      */
						/* zero time: unknown        */
						/* event type: unknown       */
#define IDISP   6       /* x data: displacement (nm) */
#define IVEL    7       /* x data: velocity (nm/sec) */
#define IACC    8       /* x data: accel (cm/sec/sec)*/
#define IB      9       /* zero time: start of file  */
#define IDAY   10       /* zero time: 0000 of GMT day*/
#define IO     11       /* zero time: event origin   */
#define IA     12       /* zero time: 1st arrival    */
#define IT0    13       /* zero time: user timepick 0*/
#define IT1    14       /* zero time: user timepick 1*/
#define IT2    15       /* zero time: user timepick 2*/
#define IT3    16       /* zero time: user timepick 3*/
#define IT4    17       /* zero time: user timepick 4*/
#define IT5    18       /* zero time: user timepick 5*/
#define IT6    19       /* zero time: user timepick 6*/
#define IT7    20       /* zero time: user timepick 7*/
#define IT8    21       /* zero time: user timepick 8*/
#define IT9    22       /* zero time: user timepick 9*/
#define IRADNV 23       /* undocumented              */
#define ITANNV 24       /* undocumented              */
#define IRADEV 25       /* undocumented              */
#define ITANEV 26       /* undocumented              */
#define INORTH 27       /* undocumented              */
#define IEAST  28       /* undocumented              */
#define IHORZA 29       /* undocumented              */
#define IDOWN  30       /* undocumented              */
#define IUP    31       /* undocumented              */
#define ILLLBB 32       /* undocumented              */
#define IWWSN1 33       /* undocumented              */
#define IWWSN2 34       /* undocumented              */
#define IHGLP  35       /* undocumented              */
#define ISRO   36       /* undocumented              */
#define INUCL  37       /* event type: nuclear shot  */
#define IPREN  38       /* event type: nuke pre-shot */
#define IPOSTN 39       /* event type: nuke post-shot*/
#define IQUAKE 40       /* event type: earthquake    */
#define IPREQ  41       /* event type: foreshock     */
#define IPOSTQ 42       /* event type: aftershock    */
#define ICHEM  43       /* event type: chemical expl */
#define IOTHER 44       /* event type: other source  */
						/* data quality: other problm*/
#define IGOOD  45       /* data quality: good        */
#define IGLCH  46       /* data quality: has glitches*/
#define IDROP  47       /* data quality: has dropouts*/
#define ILOWSN 48       /* data quality: low s/n     */
#define IRLDTA 49       /* data is real data         */
#define IVOLTS 50       /* file: velocity (volts)    */
#define IMB    52       /* undocumented              */
#define IMS    53       /* undocumented              */
#define IML    54       /* undocumented              */
#define IMW    55       /* undocumented              */
#define IMD    56       /* undocumented              */
#define IMX    57       /* undocumented              */
#define INEIC  58       /* undocumented              */
#define IPDEQ  59       /* undocumented              */
#define IPDEW  60       /* undocumented              */
#define IPDE   61       /* undocumented              */
#define IISC   62       /* undocumented              */
#define IREB   63       /* undocumented              */
#define IUSGS  64       /* undocumented              */
#define IBRK   65       /* undocumented              */
#define ICALTECH 66     /* undocumented              */
#define ILLNL  67       /* undocumented              */
#define IEVLOC 68       /* undocumented              */
#define IJSOP  69       /* undocumented              */
#define IUSER  70       /* undocumented              */
#define IUNKNOWN 71     /* undocumented              */
#define IQB     72      /* undocumented              */
#define IQB1    73      /* undocumented              */
#define IQB2    74      /* undocumented              */
#define IQBX    75      /* undocumented              */
#define IQMT    76      /* undocumented              */
#define IEQ     77      /* undocumented              */
#define IEQ1    78      /* undocumented              */
#define IEQ2    79      /* undocumented              */
#define IME     80      /* undocumented              */
#define IEX     81      /* undocumented              */
#define INU     82      /* undocumented              */
#define INC     83      /* undocumented              */
#define IO_     84      /* undocumented              */
#define IL      85      /* undocumented              */
#define IR      86      /* undocumented              */
#define IT      87      /* undocumented              */
#define IU      88      /* undocumented              */
#define IEQ3    89      /* undocumented              */
#define IEQ0    90      /* undocumented              */
#define IEX0    91      /* undocumented              */
#define IQC     92      /* undocumented              */
#define IQB0    93      /* undocumented              */
#define IGEY    94      /* undocumented              */
#define ILIT    95      /* undocumented              */
#define IMET    96      /* undocumented              */
#define IODOR   97      /* undocumented              */
#define IOS    103      /* undocumented              */

/* True/false definitions */
#undef false
#define false   0
#undef true
#define true    1

#define SAC_FLOAT_UNDEF (-12345.0)
#define SAC_INT_UNDEF   (-12345)
#define SAC_CHAR8_UNDEF "-12345  "
#define SAC_CHAR16_UNDEF "-12345          "

/* Format strings for writing headers for SAC ASCII files */
#define FCS "%15.7f%15.7f%15.7f%15.7f%15.7f\n"  /* for floats */
#define ICS "%10d%10d%10d%10d%10d\n"            /* for integers */
#define CCS1 "%-8.8s%-8.8s%-8.8s\n"             /* for strings */
#define CCS2 "%-8.8s%-16.16s\n"                 /* for strings */

/* Number of floats in the SAC Header */
#define SAC_HEADER_FLOATS       70
/* Number of ints in the SAC Header */
#define SAC_HEADER_INTS         40      /* 15 + 20 + 5 */
/* Number of numeric values in the SAC Header  (4 bytes) */
#define SAC_HEADER_NUMBERS      ( SAC_HEADER_FLOATS + SAC_HEADER_INTS )
/* Number of strings in the SAC Header  (8 or 9 bytes) */
#define SAC_HEADER_STRINGS      24      /* 24 = 23 + 1 */

/* size of integers or floats on disk or in memory
 * make sure sizeof(float) == 4 && sizeof(int)==4
 */
#define SAC_DATA_SIZEOF             4
/* Size of a character string stored on disk for a SAC header */
#define SAC_HEADER_STRING_LENGTH_FILE   8
/* Size of a character string stored in memory for a SAC header */
#define SAC_HEADER_STRING_LENGTH        9

/* Size of floats in the SAC Header */
#define SAC_HEADER_FLOATS_SIZE  ( SAC_HEADER_FLOATS * SAC_DATA_SIZEOF )
/* Size of ints in the SAC Header */
#define SAC_HEADER_INTS_SIZE  ( SAC_HEADER_INTS * SAC_DATA_SIZEOF )
/* Size of numeric headers on disk */
#define SAC_HEADER_NUMBERS_SIZE ( SAC_HEADER_FLOATS_SIZE + SAC_HEADER_INTS_SIZE )
/* Size of string headers on disk */
#define SAC_HEADER_STRINGS_SIZE ( SAC_HEADER_STRINGS * SAC_HEADER_STRING_LENGTH_FILE )

/* SAC Header Version Number */
#define SAC_HEADER_MAJOR_VERSION 6
/* offset of nvhdr relative to struct SACHEAD */
#define SAC_VERSION_LOCATION 76

/* offset of T0 relative to pointer to struct SACHEAD */
#define TMARK   10

/* offset of USER0 relative to pointer to struct SACHEAD */
#define USERN   40

/* function prototype of basic SAC I/O */
int read_sac_head(const char *name, SACHEAD *hd);
float *read_sac(const char *name, SACHEAD *hd);
int read_sac_xy(const char *name, SACHEAD *hd, float *xdata, float *ydata);
float *read_sac_pdw(const char *name, SACHEAD *hd, int tmark, float t1, float t2);
int write_sac(const char *name, SACHEAD hd, const float *ar);
int write_sac_xy(const char *name, SACHEAD hd, const float *xdata, const float *ydata);
SACHEAD new_sac_head(float dt, int ns, float b0);
int sac_head_index(const char *name);
int issac(const char *name);

#endif /* sacio.h */
