#include<stdio.h>
#include<stdlib.h>
#include "segy_io.h"


/*
 *	$Id$
 * segy_io.c:  A suite of functions to cover reading SEGY header variables.
 *
 * modified by T. Henstock from the PASSCAL software suite.
 * From PASSCAL code base which is in the public domain
 *
 */


/************************ samp_rd() *******************************/
/* Returns Number of Sample in SegyHead (hdr), FALSE otherwise
 *
 * needed since the SEGY standard only allows 32767 samples, which
 * is often exceeded in refraction experiments. */

unsigned long samp_rd(SEGYHEAD *hdr) {
  if (!hdr) {
    printf("samp_rd: Received a NULL pointer\n");
    return (FALSE);
  }
  if (hdr->sampleLength == 32767)
    return (hdr->num_samps);
  else
    return (hdr->sampleLength);
}


/************************ get_segy_reelhd() **************************/
/* read (and discard!) EBCDIC text reel header */

int get_segy_reelhd (FILE *fileptr, char *reelhead) {
	int igot;

        if ((igot = (int)fread (reelhead, (size_t)3200, (size_t)1, fileptr)) != 1) {
		fprintf(stderr,"Error reading SEGY reel header \n");
		exit(1);
		}
	return (TRUE);
}

/*********************** get_segy_binhd() ****************************/
/* read SEGY binary reel header */

int get_segy_binhd (FILE *fileptr, SEGYREEL *binhead) {		
	int igot;

	if ((igot = (int)fread (binhead, (size_t)400, (size_t)1, fileptr)) !=1) {
		fprintf(stderr, "Error reading SEGY binary header \n");
		return(FALSE);
		}

/*	fprintf(stderr, "n_samp %i\n", binhead->nsamp); */

	return (TRUE);
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
  if ((head_ptr = (SEGYHEAD *) calloc((size_t)1, (size_t)240)) == NULL) {
    fprintf(stderr, "Error: Out of memory for SEGY Headers ");
    return (NULL);
  }
  /* read in the header */
  if (fread(head_ptr, (size_t)240, (size_t)1, file_ptr) != 1) {
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
 * DATA IS RETRIEVED and returned as (char *) from this function.
 * (this is probably bad, requiring casts elsewhere, but we just want
 * a block of bytes)
 * 
 * Returns a NULL ptr upon Failure and echos an error message (minus final "\n")
 * to stderr explaining the problem.
 */

char *get_segy_data(FILE *file_ptr, SEGYHEAD *head_ptr) {
  char           *data_ptr;
  unsigned long            size_of_data, num_samps, num_bytes;

	
  num_samps = samp_rd(head_ptr);
  size_of_data = 4;
  num_bytes = size_of_data * num_samps;

  data_ptr = (char *) calloc(num_bytes, sizeof(char));
  if (data_ptr == NULL) {
    fprintf(stderr, "Error: Out of memory for SEGY data ");
    return (NULL);
  }
  /* read in data  */
  if (fread(data_ptr, size_of_data, num_samps, file_ptr) != num_samps) {
    fprintf(stderr, "Error: Unable to read data ");
    free(data_ptr);
    return (NULL);
  }
  return (data_ptr);
}
