/************************ segy_io.h *******************************/
/*  $Id: segy_io.h,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $  */
/* segy_io.h:	Include file for segy_io.c, a suite of functions to */
/* help reading and writing those annoying, redundant SEGY header */
/* variables and for reading/writing to/from SEGY files.          */
/******************************************************************/

#ifndef SEGY_IO_H
#define SEGY_IO_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#include "segy.h"
#include "segyreel.h"

unsigned long samp_rd( SEGYHEAD *hdr );
int get_segy_reelhd(FILE * fileptr, char * reelhead );
int get_segy_binhd(FILE * fileptr, SEGYREEL * binhead );
SEGYHEAD *get_segy_header(FILE * file_ptr);
char *get_segy_data( FILE * file_ptr, SEGYHEAD * head_ptr );


#endif /* SEGY_IO_H */
