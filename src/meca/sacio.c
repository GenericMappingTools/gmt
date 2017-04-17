/*******************************************************************************
 *                                  sacio.c                                    *
 *  SAC I/O functions:                                                         *
 *      read_sac_head    read SAC header                                       *
 *      read_sac         read SAC binary data                                  *
 *      read_sac_xy      read SAC binary XY data                               *
 *      read_sac_pdw     read SAC data in a partial data window (cut option)   *
 *      write_sac        Write SAC binary data                                 *
 *      write_sac_xy     Write SAC binary XY data                              *
 *      new_sac_head     Create a new minimal SAC header                       *
 *      sac_head_index   Find the offset of specified SAC head fields          *
 *      issac            Check if a file in in SAC format                      *
 *                                                                             *
 *  Author: Dongdong Tian @ USTC                                               *
 *                                                                             *
 *  Revisions:                                                                 *
 *      2014-03-19  Dongdong Tian   Modified from Prof. Lupei Zhu's code       *
 *      2014-08-02  Dongdong Tian   Better function naming and coding style    *
 *      2014-08-13  Dongdong Tian   Add new functions:                          *
 *                                  - read_sac_xy                              *
 *                                  - write_sac_xy                             *
 *                                  - sac_head_index                           *
 *      2016-03-01  Dongdong Tian   Add new function: issac                    *
 *                                                                             *
 ******************************************************************************/

/* 
The SAC I/O functions are initially written by Prof. Lupei Zhu,
and modified by Dongdong Tian.

Original License:

Permission to use, copy, modify, and distribute the package
and supporting documentation for any purpose without fee is hereby granted,
provided that the above copyright notice appear in all copies, that both
that copyright notice and this permission notice appear in supporting documentation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "sacio.h"

/* function prototype for local use */
static void    byte_swap       (char *pt, size_t n);
static int     check_sac_nvhdr (const int nvhdr);
static void    map_chdr_in     (char *memar, char *buff);
static int     read_head_in    (const char *name, SACHEAD *hd, FILE *strm);
#if 0		/* These functions do not seem to be used anywhere */
static void    map_chdr_out    (char *memar, char *buff);
static int     write_head_out  (const char *name, SACHEAD hd, FILE *strm);
#endif

/* a SAC structure containing all null values */
static SACHEAD sac_null = {
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345., -12345., -12345., -12345., -12345.,
	-12345 , -12345 , -12345 , -12345 , -12345 ,
	-12345 , -12345 , -12345 , -12345 , -12345 ,
	-12345 , -12345 , -12345 , -12345 , -12345 ,
	-12345 , -12345 , -12345 , -12345 , -12345 ,
	-12345 , -12345 , -12345 , -12345 , -12345 ,
	-12345 , -12345 , -12345 , -12345 , -12345 ,
	-12345 , -12345 , -12345 , -12345 , -12345 ,
	-12345 , -12345 , -12345 , -12345 , -12345 ,
	{ '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
	{ '-','1','2','3','4','5',' ',' ' }
};

/*
 *  read_sac_head
 *
 *  Description: Read binary SAC header from file.
 *
 *  IN:
 *      const char *name : File name
 *  OUT:
 *      SACHEAD    *hd   : SAC header
 *
 *  Return: 0 if success, -1 if failed
 *
 */
int read_sac_head(const char *name, SACHEAD *hd) {
	FILE    *strm;
	int     lswap;

	if ((strm = fopen(name, "rb")) == NULL) {
		fprintf(stderr, "Unable to open %s\n", name);
		return -1;
	}

	lswap = read_head_in(name, hd, strm);

	fclose(strm);

	return ((lswap == -1) ? -1 : 0);
}

/*
 *  read_sac
 *
 *  Description: Read binary SAC data from file.
 *
 *  IN:
 *      const char *name : file name
 *  OUT:
 *      SACHEAD    *hd   : SAC header to be filled
 *  Return: float pointer to the data array, NULL if failed.
 *
 */
float *read_sac(const char *name, SACHEAD *hd) {
	FILE    *strm;
	float   *ar;
	int     lswap;
	size_t  sz;

	if ((strm = fopen(name, "rb")) == NULL) {
		fprintf(stderr, "Unable to open %s\n", name);
		return NULL;
	}

	lswap = read_head_in(name, hd, strm);

	if (lswap == -1) {
		fclose(strm);
		return NULL;
	}

	sz = (size_t) hd->npts * SAC_DATA_SIZEOF;
	if (hd->iftype == IXY) sz *= 2;

	/* coverity[tainted_data] */	/* For a Coverity issue. Do not delete */
	if ((ar = (float *)malloc(sz)) == NULL) {
		fprintf(stderr, "Error in allocating memory for reading %s\n", name);
		fclose(strm);
		return NULL;
	}

	if (fread((char*)ar, sz, 1, strm) != 1) {
		fprintf(stderr, "Error in reading SAC data %s\n", name);
		free(ar);
		fclose(strm);
		return NULL;
	}
	fclose(strm);

	if (lswap == true) byte_swap((char*)ar, sz);

	return ar;
}

#if 0		/* These functions do not seem to be used anywhere */
/*
 *  read_sac_xy
 *
 *  Description:    read SAC XY binary file
 *
 *  IN:
 *      const char *name    :   file name
 *  OUT:
 *      SACHEAD *hd         :   SAC head to be filled
 *      float *xdata        :   pointer for X
 *      float *ydata        :   pointer for Y
 *
 *  Return: 0 for success, -1 for fail
 *
 */
int read_sac_xy(const char *name, SACHEAD *hd, float *xdata, float *ydata) {
	float *data;
	size_t npts;

	/* coverity[tainted_data] */	/* For a Coverity issue. Do not delete */
	if ((data = read_sac(name, hd)) == NULL)  return -1;

	npts = (size_t)hd->npts;
	if ((xdata = (float *)malloc(npts*SAC_DATA_SIZEOF)) == NULL) {
		fprintf(stderr, "Error in allocating memory for %s\n", name);
		free(data);
		return -1;
	}
	if ((ydata = (float *)malloc(npts*SAC_DATA_SIZEOF)) == NULL) {
		fprintf(stderr, "Error in allocating memory for %s\n", name);
		free(data);
		free(xdata);
		return -1;
	}

	memcpy(xdata, data,      npts*SAC_DATA_SIZEOF);
	memcpy(ydata, data+npts, npts*SAC_DATA_SIZEOF);

	free(data);
	return 0;
}

/*
 *  write_sac
 *
 *  Description:    write binary SAC data
 *
 *  IN:
 *      const char *name    :   file name
 *      SACHEAD     hd      :   header
 *      const float *ar     :   float data array
 *
 *  Return:
 *      -1  :   fail
 *      0   :   succeed
 *
 */
/* coverity[pass_by_value] */	/* For a Coverity issue. Do not delete */
int write_sac(const char *name, SACHEAD hd, const float *ar) {
	FILE    *strm;
	size_t  sz;

	if ((strm = fopen(name, "wb")) == NULL) {
		fprintf(stderr, "Error in opening file for writing %s\n", name);
		return -1;
	}

	if (write_head_out(name, hd, strm) == -1) {
		fclose(strm);
		return -1;
	}

	sz = (size_t)hd.npts * SAC_DATA_SIZEOF;
	if (hd.iftype == IXY) sz *= 2;

	if (fwrite(ar, sz, 1, strm) != 1) {
		fprintf(stderr, "Error in writing SAC data for writing %s\n", name);
		fclose(strm);
		return -1;
	}
	fclose(strm);
	return 0;
}

/*
 *  write_sac_xy
 *
 *  Description:    write binary SAC XY data
 *
 *  IN:
 *      const char *name    :   file name
 *      SACHEAD     hd      :   header
 *      const float *xdata  :   float data array for X
 *      const float *ydata  :   float data array for Y
 *
 *  Return:
 *      -1  :   fail
 *      0   :   succeed
 *
 */
/* coverity[pass_by_value] */	/* For a Coverity issue. Do not delete */
int write_sac_xy(const char *name, SACHEAD hd, const float *xdata, const float *ydata) {
	float *ar;
	int npts;
	int error;
	size_t sz;

	npts = hd.npts;
	sz = (size_t)npts * SAC_DATA_SIZEOF;

	if ((ar = (float *)malloc(sz*2)) == NULL) {
		fprintf(stderr, "Error in allocating memory for file %s\n", name);
		return -1;
	}
	memcpy(ar,      xdata, sz);
	memcpy(ar+npts, ydata, sz);

	/* needed for XY data */
	hd.iftype = IXY;
	hd.leven = false;

	error = write_sac(name, hd, ar);

	free(ar);
	return error;
}
#endif

/*
 *  read_sac_pdw
 *
 *  Description:
 *      Read portion of data from file.
 *
 *  Arguments:
 *      const char  *name   :   file name
 *      SACHEAD     *hd     :   SAC header to be filled
 *      int         tmark   :   time mark in SAC header
 *                                  -5  ->  b;
 *                                  -4  ->  e;
 *                                  -3  ->  o;
 *                                  -2  ->  a;
 *                                  0-9 ->  Tn;
 *                                  others -> t=0;
 *      float       t1      :   begin time is tmark + t1
 *      float       t2      :   end time is tmark + t2
 *
 *  Return:
 *      float pointer to the data array, NULL if failed.
 *
 */
float *read_sac_pdw(const char *name, SACHEAD *hd, int tmark, float t1, float t2) {
	FILE    *strm;
	int     lswap;
	float   tref;
	int     nt1, nt2, npts, nn;
	float   *ar, *fpt;

	if ((strm = fopen(name, "rb")) == NULL) {
		fprintf(stderr, "Error in opening %s\n", name);
		return NULL;
	}

	lswap = read_head_in(name, hd, strm);

	if (lswap == -1) {
		fclose(strm);
		return NULL;
	}

	nn = (int)((t2-t1)/hd->delta);
	if (nn <= 0 || (ar = (float *)calloc((size_t)nn, SAC_DATA_SIZEOF)) == NULL) {
		fprintf(stderr, "Error allocating memory for reading %s n=%d\n", name, nn);
		fclose(strm);
		return NULL;
	}

	tref = 0.;
	if ((tmark >= -5&&tmark<=-2) || (tmark >= 0 && tmark <= 9)) {
		tref = *((float *) hd + TMARK + tmark);
		if (fabs(tref+12345.)<0.1) {
			fprintf(stderr, "Time mark undefined in %s\n", name);
			free(ar);
			fclose(strm);
			return NULL;
		}
	}
	t1 += tref;
	nt1 = (int)((t1 - hd->b) / hd->delta);
	nt2 = nt1 + nn;
	npts = hd->npts;
	hd->npts = nn;
	hd->b   = t1;
	hd->e   = t1 + nn * hd->delta;

	if (nt1 > npts || nt2 < 0) {
		fclose(strm);
		return ar;    /* return zero filled array */
	}
	/* maybe warnings are needed! */

	if (nt1 < 0) {
		fpt = ar - nt1;
		nt1 = 0;
	}
	else {
		if (fseek(strm, nt1*SAC_DATA_SIZEOF, SEEK_CUR) < 0) {
			fprintf(stderr, "Error in seek %s\n", name);
			free(ar);
			fclose(strm);
			return NULL;
		}
		fpt = ar;
	}
	if (nt2>npts) nt2 = npts;
	nn = nt2 - nt1;

	if (fread((char *)fpt, (size_t)nn * SAC_DATA_SIZEOF, 1, strm) != 1) {
		fprintf(stderr, "Error in reading SAC data %s\n", name);
		free(ar);
		fclose(strm);
		return NULL;
	}
	fclose(strm);

	if (lswap == true) byte_swap((char*)ar, (size_t)nn*sizeof(float));

	return ar;
}

/*
 *  new_sac_head
 *
 *  Description: create a new SAC header with required fields
 *
 *  IN:
 *      float   dt  :   sample interval
 *      int     ns  :   number of points
 *      float   b0  :   starting time
 */
SACHEAD new_sac_head(float dt, int ns, float b0) {
	SACHEAD hd = sac_null;
	hd.delta    =   dt;
	hd.npts     =   ns;
	hd.b        =   b0;
	hd.o        =   0.;
	hd.e        =   b0+(ns-1)*dt;
	hd.iztype   =   IO;
	hd.iftype   =   ITIME;
	hd.leven    =   true;
	hd.nvhdr    =   SAC_HEADER_MAJOR_VERSION;
	return hd;
}

/*
 *  sac_head_index
 *
 *  Description: return the index of a specified sac head field
 *
 *  In:
 *      const char *name    :   name of sac head field
 *  Return:
 *      index of a specified field in sac head
 *
 */
int sac_head_index(const char *name) {
	const char fields[SAC_HEADER_NUMBERS+SAC_HEADER_STRINGS][10] = {
		"delta",    "depmin",   "depmax",   "scale",    "odelta",
		"b",        "e",        "o",        "a",        "internal1",
		"t0",       "t1",       "t2",       "t3",       "t4",
		"t5",       "t6",       "t7",       "t8",       "t9",
		"f",        "resp0",    "resp1",    "resp2",    "resp3",
		"resp4",    "resp5",    "resp6",    "resp7",    "resp8",
		"resp9",    "stla",     "stlo",     "stel",     "stdp",
		"evla",     "evlo",     "evel",     "evdp",     "mag",
		"user0",    "user1",    "user2",    "user3",    "user4",
		"user5",    "user6",    "user7",    "user8",    "user9",
		"dist",     "az",       "baz",      "gcarc",    "internal2",
		"internal3","depmen",   "cmpaz",    "cmpinc",   "xminimum",
		"xmaximum", "yminimum", "ymaximum", "unused1",  "unused2",
		"unused3",  "unused4",  "unused5",  "unused6",  "unused7",
		"nzyear",   "nzjday",   "nzhour",   "nzmin",    "nzsec",
		"nzmsec",   "nvhdr",    "norid",    "nevid",    "npts",
		"internal4","nwfid",    "nxsize",   "nysize",   "unused8",
		"iftype",   "idep",     "iztype",   "unused9",  "iinst",
		"istreg",   "ievreg",   "ievtyp",   "iqual",    "isynth",
		"imagtyp",  "imagsrc",  "unused10", "unused11", "unused12",
		"unused13", "unused14", "unused15", "unused16", "unused17",
		"leven",    "lpspol",   "lovrok",   "lcalda",   "unused18",
		"kstnm",    "kevnm",    "kevnmmore",
		"khole",    "ko",       "ka",
		"kt0",      "kt1",      "kt2",
		"kt3",      "kt4",      "kt5",
		"kt6",      "kt7",      "kt8",
		"kt9",      "kf",       "kuser0",
		"kuser1",   "kuser2",   "kcmpnm",
		"knetwk",   "kdatrd",   "kinst",
	};
	int i;

	for (i = 0; i < SAC_HEADER_NUMBERS + SAC_HEADER_STRINGS; i++)
#ifdef _WIN32
		if ((stricmp(name, fields[i]) == 0))  return i;
#else
		if ((strcasecmp(name, fields[i]) == 0))  return i;
#endif

	return -1;
}

/*
 *  issac
 *
 *  Description: check if a file is in SAC format
 *
 *  In:
 *      const char *name    :   sac filename
 *  Return:
 *      -1 : fail
 *      true  : is a SAC file
 *      false  : not a SAC file
 *
 */
int issac(const char *name) {
	FILE *strm;
	int nvhdr;

	if ((strm = fopen(name, "rb")) == NULL) {
		fprintf(stderr, "Unable to open %s\n", name);
		return -1;
	}

	if (fseek(strm, SAC_VERSION_LOCATION * SAC_DATA_SIZEOF, SEEK_SET)) {
		fclose(strm);
		return false;
	}
	if (fread(&nvhdr, sizeof(int), 1, strm) != 1) {
		fclose(strm);
		return false;
	}
	fclose(strm);
	
	if (check_sac_nvhdr(nvhdr) == -1)
		return false;
	else
		return true;
}

/******************************************************************************
 *                                                                            *
 *              Functions below are only for local use!                       *
 *                                                                            *
 ******************************************************************************/

/*
 *  byte_swap : reverse the byte order of 4 bytes int/float.
 *
 *  IN:
 *      char    *pt : pointer to byte array
 *      size_t   n  : number of bytes
 *  Return: none
 *
 *  Notes:
 *      For 4 bytes,
 *      byte swapping means taking [0][1][2][3],
 *      and turning it into [3][2][1][0]
 */
static void byte_swap(char *pt, size_t n) {
	size_t  i;
	char    tmp;
	for (i = 0; i < n; i += 4) {
		tmp     =   pt[i+3];
		pt[i+3] =   pt[i];
		pt[i]   =   tmp;

		tmp     =   pt[i+2];
		pt[i+2] =   pt[i+1];
		pt[i+1] =   tmp;
	}
}

/*
 *  check_sac_nvhdr
 *
 *  Description: Determine the byte order of the SAC file
 *
 *  IN:
 *      const int nvhdr : nvhdr from header
 *
 *  Return:
 *      false   no byte order swap is needed
 *      true    byte order swap is needed
 *      -1      not in sac format ( nvhdr != SAC_HEADER_MAJOR_VERSION )
 *
 */
static int check_sac_nvhdr(const int nvhdr) {
	int lswap = false;

	if (nvhdr != SAC_HEADER_MAJOR_VERSION) {
		byte_swap((char*) &nvhdr, SAC_DATA_SIZEOF);
		if (nvhdr == SAC_HEADER_MAJOR_VERSION)
			lswap = true;
		else
			lswap = -1;
	}
	return lswap;
}

/*
 *  map_chdr_in:
 *       map strings from buffer to memory
 */
static void map_chdr_in(char *memar, char *buff) {
	char    *ptr1, *ptr2;
	int     i;

	ptr1 = memar;
	ptr2 = buff;

	memcpy(ptr1, ptr2, 8);
	*(ptr1+8) = '\0';
	ptr1 += 9;
	ptr2 += 8;

	memcpy(ptr1, ptr2, 16);
	*(ptr1+16) = '\0';
	ptr1 += 18;
	ptr2 += 16;

	for (i=0; i<21; i++) {
		memcpy(ptr1, ptr2, 8);
		*(ptr1+8) = '\0';
		ptr1 += 9;
		ptr2 += 8;
	}
}

/*
 *  read_head_in:
 *      read sac header in and deal with possible byte swap.
 *
 *  IN:
 *      const char *name : file name, only for debug
 *      SACHEAD    *hd   : header to be filled
 *      FILE       *strm : file handler
 *
 *  Return:
 *      0   :   Succeed and no byte swap
 *      1   :   Succeed and byte swap
 *     -1   :   fail.
 */
static int read_head_in(const char *name, SACHEAD *hd, FILE *strm) {
	char   *buffer;
	int     lswap;

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
	if (sizeof(float) != SAC_DATA_SIZEOF || sizeof(int) != SAC_DATA_SIZEOF) {
		fprintf(stderr, "Mismatch in size of basic data type!\n");
		return -1;
	}

	/* read numeric parts of the SAC header */
	if (fread(hd, SAC_HEADER_NUMBERS_SIZE, 1, strm) != 1) {
		fprintf(stderr, "Error in reading SAC header %s\n", name);
		return -1;
	}

	/* Check Header Version and Endian  */
	lswap = check_sac_nvhdr(hd->nvhdr);
	if (lswap == -1) {
		fprintf(stderr, "Warning: %s not in sac format.\n", name);
		return -1;
	}
	else if (lswap == true) {
		byte_swap((char *)hd, SAC_HEADER_NUMBERS_SIZE);
	}

	/* read string parts of the SAC header */
	if ((buffer = (char *)malloc(SAC_HEADER_STRINGS_SIZE)) == NULL) {
		fprintf(stderr, "Error in allocating memory %s\n", name);
		return -1;
	}
	if (fread(buffer, SAC_HEADER_STRINGS_SIZE, 1, strm) != 1) {
		fprintf(stderr, "Error in reading SAC header %s\n", name);
		free(buffer);
		return -1;
	}
	map_chdr_in((char *)(hd)+SAC_HEADER_NUMBERS_SIZE, buffer);
	free(buffer);

	return lswap;
}

#if 0		/* These functions do not seem to be used anywhere */
/*
 *  write_head_out
 *
 *  IN:
 *      const char *name : file name, only for debug
 *      SACHEAD     hd   : header to be written
 *      FILE       *strm : file handler
 *
 *  Return:
 *      -1  :   failed.
 *       0  :   success.
 */
static int write_head_out(const char *name, SACHEAD hd, FILE *strm) {
	char *buffer;

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
	if (sizeof(float) != SAC_DATA_SIZEOF || sizeof(int) != SAC_DATA_SIZEOF) {
		fprintf(stderr, "Mismatch in size of basic data type!\n");
		return -1;
	}

	if (fwrite(&hd, SAC_HEADER_NUMBERS_SIZE, 1, strm) != 1) {
		fprintf(stderr, "Error in writing SAC data for writing %s\n", name);
		return -1;
	}

	if ((buffer = (char *)malloc(SAC_HEADER_STRINGS_SIZE)) == NULL) {
		fprintf(stderr, "Error in allocating memory %s\n", name);
		free(buffer);
		return -1;
	}
	map_chdr_out((char *)(&hd)+SAC_HEADER_NUMBERS_SIZE, buffer);

	if (fwrite(buffer, SAC_HEADER_STRINGS_SIZE, 1, strm) != 1) {
		fprintf(stderr, "Error in writing SAC data for writing %s\n", name);
		free(buffer);
		return -1;
	}
	free(buffer);

	return 0;
}
/*
 *   map_chdr_out:
 *      map strings from memory to buffer
 */
static void map_chdr_out(char *memar, char *buff) {
	char    *ptr1, *ptr2;
	int     i;

	ptr1 = memar;
	ptr2 = buff;

	memcpy(ptr2, ptr1, 8);
	ptr1 += 9;
	ptr2 += 8;

	memcpy(ptr2, ptr1, 16);
	ptr1 += 18;
	ptr2 += 16;

	for (i = 0; i < 21; i++) {
		memcpy(ptr2, ptr1, 8);
		ptr1 += 9;
		ptr2 += 8;
	}
}

#endif
