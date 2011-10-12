/************************ segy_io.h *******************************/
/*  $Id$  */
/* segy_io.h:	Include file for segy_io.c, a suite of functions to */
/* help reading and writing those annoying, redundant SEGY header */
/* variables and for reading/writing to/from SEGY files.          */
/* From PASSCAL code base which is in the public domain		  */
/******************************************************************/

#ifndef SEGY_IO_H
#define SEGY_IO_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if defined(WIN32)
typedef short int16_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#elif  !defined(_INTTYPES_H) && !defined(__OPENNT)
#include <inttypes.h>
#endif

#include "segy.h"
#include "segyreel.h"

unsigned long samp_rd( SEGYHEAD *hdr );
int get_segy_reelhd(FILE * fileptr, char * reelhead );
int get_segy_binhd(FILE * fileptr, SEGYREEL * binhead );
SEGYHEAD *get_segy_header(FILE * file_ptr);
char *get_segy_data( FILE * file_ptr, SEGYHEAD * head_ptr );


#endif /* SEGY_IO_H */
