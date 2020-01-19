/************************ segy_io.h *******************************
 * Include file for segy_io.c, a suite of functions to
 * help reading and writing those annoying, redundant SEGY header
 * variables and for reading/writing to/from SEGY files.
 * From PASSCAL code base which is in the public domain
 * http://www.passcal.nmt.edu/
 ******************************************************************/

/*!
 * \file segy_io.h
 * \brief
 */

#ifndef SEGY_IO_H
#define SEGY_IO_H

#include "gmt_dev.h"
#include "common_byteswap.h"

#include "segy.h"
#include "segyreel.h"

uint32_t segy_samp_rd (SEGYHEAD *hdr);
int segy_get_reelhd (FILE *fileptr, char *reelhead);
int segy_get_binhd (FILE *fileptr, SEGYREEL *binhead);
SEGYHEAD *segy_get_header (FILE *file_ptr);
float *segy_get_data (FILE *file_ptr, SEGYHEAD *head_ptr);

#endif /* SEGY_IO_H */
