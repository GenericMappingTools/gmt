/************************ segy_io.h *******************************
 * $Id$
 * Include file for segy_io.c, a suite of functions to
 * help reading and writing those annoying, redundant SEGY header
 * variables and for reading/writing to/from SEGY files.
 * From PASSCAL code base which is in the public domain
 * http://www.passcal.nmt.edu/
 ******************************************************************/

#ifndef SEGY_IO_H
#define SEGY_IO_H

#include "gmt.h"
#include "common_byteswap.h"

#include "segy.h"
#include "segyreel.h"

uint32_t samp_rd (SEGYHEAD *hdr);
int get_segy_reelhd (FILE *fileptr, char *reelhead);
int get_segy_binhd (FILE *fileptr, SEGYREEL *binhead);
SEGYHEAD *get_segy_header (FILE *file_ptr);
float *get_segy_data (FILE *file_ptr, SEGYHEAD *head_ptr);

#endif /* SEGY_IO_H */
