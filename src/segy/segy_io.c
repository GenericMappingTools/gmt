/*
 * $Id$
 * segy_io.c:  A suite of functions to cover reading SEGY header variables.
 *
 * modified by T. Henstock from the PASSCAL software suite.
 * From PASSCAL code base which is in the public domain
 * http://www.passcal.nmt.edu/
 *
 * Edit F. Wobbe: use stdint sizes, fix buffer overflow check
 */

#include <stdio.h>
#include <stdlib.h>
#include "segy_io.h"

/************************ samp_rd() *******************************/
/* Returns Number of Sample in SegyHead (hdr), false otherwise
 *
 * needed since the SEGY standard only allows 2^16 samples, which
 * is often exceeded in refraction experiments. */

uint32_t samp_rd(SEGYHEAD *hdr) {
  if (!hdr) {
    printf("samp_rd: Received a NULL pointer\n");
    return (false);
  }
  if (hdr->sampleLength == 0xffff && hdr->num_samps > 0xffff)
		/* buffer overflow */
    return (hdr->num_samps);
  else
    return (hdr->sampleLength);
}

/************************ get_segy_reelhd() **************************/
/* read (and discard!) EBCDIC text reel header */

int get_segy_reelhd (FILE *fileptr, char *reelhead) {
	if (fread (reelhead, 3200, 1, fileptr) != 1) {
		fprintf(stderr,"Error reading SEGY reel header \n");
		exit(1);
	}
	return (true);
}

/*********************** get_segy_binhd() ****************************/
/* read SEGY binary reel header */

int get_segy_binhd (FILE *fileptr, SEGYREEL *binhead) {
	if (fread (binhead, 400, 1, fileptr) !=1) {
		fprintf(stderr, "Error reading SEGY binary header \n");
		return(false);
		}
	return (true);
}

/************************ get_segy_header() **************************/
/*
 * Returns a SEGY header structure given a file pointer. This SEGY header
 * structure is dynamically allocated using calloc and should be free()'d
 * when it's usefulness is over.
 *
 * NO DATA IS RETRIEVED
 *
 * Returns a NULL ptr upon Failure and echos an error message (minus final "\n")
 * to stderr explaining the problem.
 */

SEGYHEAD *get_segy_header(FILE *file_ptr) {
  SEGYHEAD       *head_ptr;

  /* get memory for SegyHead'er */
  if ((head_ptr = calloc (1, 240)) == NULL) {
    fprintf(stderr, "Error: Out of memory for SEGY Headers ");
    return (NULL);
  }
  /* read in the header */
  if (fread (head_ptr, 240, 1, file_ptr) != 1) {
    fprintf(stderr, "Error: Unable to read next trace header -- end of file?\n");
    free(head_ptr);
    return (NULL);
  }
  return (head_ptr);
}

/************************ get_segy_data()   **************************/
/*
 * This SEGY data pointer is dynamically allocated using calloc and should be
 * free()'d when it's usefulness is over.
 *
 * Original function returned a (char *) but casting from (char *) violates
 * strict-aliasing rules.
 *
 * Returns a NULL ptr upon Failure and echos an error message (minus final "\n")
 * to stderr explaining the problem.
 */

float *get_segy_data(FILE *file_ptr, SEGYHEAD *head_ptr) {
  float    *data_ptr;
  uint32_t num_samps;

  num_samps = samp_rd(head_ptr);

  data_ptr = calloc(num_samps, sizeof(float));
  if (data_ptr == NULL) {
    fprintf(stderr, "Error: Out of memory for SEGY data ");
    return (NULL);
  }
  /* read in data  */
  if (fread(data_ptr, sizeof (float), num_samps, file_ptr) != num_samps) {
    fprintf(stderr, "Error: Unable to read data ");
    free(data_ptr);
    return (NULL);
  }
  return (data_ptr);
}
