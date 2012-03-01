/*
Copyright 2006, University Corporation for Atmospheric Research. See
COPYRIGHT file for copying and redistribution conditions.

This file is part of the NetCDF CF Library. 

This file handles the libcf file stuff.

Ed Hartnett, 9/1/06

$Id$
*/

#include <config.h>
#include <libcf.h>
#include <libcf_int.h>
#include <netcdf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* Get the version string for the CF Library (max length: 
   NC_MAX_NAME + 1). */
int
nccf_inq_libvers(char *version_string)
{
   char ver[NC_MAX_NAME + 1];

   sprintf(ver, "libcf library version %s", VERSION);
   if (version_string)
      strcpy(version_string, ver);

   return CF_NOERR;
}

/* Append string to a named global attribute. Create the attribute if
 * it doesn't exist. */
static int
nccf_append_att(int ncid, const char *name, const char *string)
{
   char *att_str = NULL;
   size_t len, new_len;
   int ret;

   /* Find out if there is an attribute of this name. */
   ret = nc_inq_attlen(ncid, NC_GLOBAL, name, &len);

   if (ret == NC_ENOTATT)
   {
      /* Create the attribute. I will null-terminate this
       * attribute. */
      if ((ret = nc_put_att_text(ncid, NC_GLOBAL, name, 
				 strlen(string) + 1, string)))
	 return ret;
   }
   else if (ret == NC_NOERR)
   {
      /* The attribute already exists. Get memory to hold the existing
       * att plus our version string. Add one for the space, and one
       * for a terminating null. */
      new_len = len + strlen(string) + 1;
      if (!(att_str = malloc(new_len + 1)))
	 return CF_ENOMEM;

      /* Get the existing attribute value. */
      if ((ret = nc_get_att_text(ncid, NC_GLOBAL, name, att_str)))
	 BAIL(CF_ENETCDF);

      /* If it's already in the string, our work is done.*/
      if (strstr(att_str, string))
      {
	 free(att_str);
	 return CF_NOERR;
      }

      /* Append our string to the existing att. */
      att_str[len] = 0;
      strcat(att_str, " ");
      strcat(att_str, string);

      /* Delete the existing attribute, so we can rewrite it. */
      if ((ret = nc_del_att(ncid, NC_GLOBAL, name)))
	 BAIL(ret);

      /* Rewrite the attribute with our string appended. */
      if ((ret = nc_put_att_text(ncid, NC_GLOBAL, name, 
				 strlen(att_str) + 1, att_str)))
	 BAIL(ret);
   }

  exit:
   if (att_str) 
      free(att_str);
   return ret;
}

/* From the CF 1.0 conventions:

"We recommend that the NUG defined attribute Conventions be given the
string value "CF-1.0" to identify datasets that conform to these
conventions. */

/*  Mark a file with the CF-1.0 string in a Conventions attribute. */
int 
nccf_def_convention(int ncid)
{
   return nccf_append_att(ncid, CF_CONVENTIONS, CF_CONVENTION_STRING);
}

int 
nccf_inq_convention(int ncid, int *cf_convention)
{
   size_t len, new_len;
   char *existing_att = NULL;
   int ret = CF_NOERR;

   /* Find out if there is a conventions attribute. */
   ret = nc_inq_attlen(ncid, NC_GLOBAL, CF_CONVENTIONS, &len);

   if (ret == NC_NOERR)
   {
      /* Get memory to hold the existing att plus our version
       * string. */
      new_len = len + strlen(CF_CONVENTION_STRING) + 1;
      if (!(existing_att = malloc(new_len)))
	 return CF_ENOMEM;

      /* Get the existing att. */
      if ((ret = nc_get_att_text(ncid, NC_GLOBAL, CF_CONVENTIONS, 
				 existing_att)))
	 BAIL(CF_ENETCDF);

      /* If it's already in the string, our work is done.*/
      if (strstr(existing_att, CF_CONVENTION_STRING))
      {
	 if (cf_convention)
	    *cf_convention = 1;
	 ret = CF_NOERR;
      }
   }
   else if (ret == NC_ENOTATT)
   {
      /* No conventions att means no cf conventions. ;-( But this is
       * not an error. */
      if (cf_convention)
	 *cf_convention = 0;
      ret = NC_NOERR;
   }
   else
      BAIL(CF_ENETCDF);

  exit:
   if (existing_att) 
      free(existing_att);
   return ret;
}


/* From the CF 1.0 conventions:

"The general description of a file's contents should be contained in
the following attributes: title, history, institution, source, comment
and references (2.6.2). For backwards compatibility with COARDS none
of these attributes is required, but their use is recommended to
provide human readable documentation of the file contents."
*/

/* Add CF_recomended attributes to a file. Any NULLs will cause the
 * cooresponding attribute to not be written. */
int
nccf_def_file(int ncid, const char *title, const char *history)
{
   int ret;

   /* Make sure the file is labled with "CF-1.0" in a Conventions
    * attribute. */
   if ((ret = nccf_def_convention(ncid)))
      return ret;

   /* If title already exists this will return an error. */
   if (title)
      if ((ret = nc_put_att_text(ncid, NC_GLOBAL, CF_TITLE,
				    strlen(title) + 1, title)))
	 return ret;

   /* This will append to the history attribute, creating it if
    * needed. */
   if (history)
      ret = nccf_add_history(ncid, history);

   return ret;
}


/* Append to the global "history" attribute, with a timestamp. Create
 * the attribute if it does not exist. */
#define NCCF_MAX_TIMESTAMP_LEN 35
int 
nccf_add_history(int ncid, const char *history)
{
   time_t now;
   char *hist_str;
   char timestamp[NCCF_MAX_TIMESTAMP_LEN + 1];
   struct tm *timptr;
   int ret;

   if (!history)
      return NC_NOERR;

   /* Get the date and time. */
   time(&now);
   if (!(timptr = localtime(&now)))
      return CF_ETIME;
   if (!strftime(timestamp, NCCF_MAX_TIMESTAMP_LEN, "%x %X", timptr))
      return CF_ETIME;

   /* Allocate space for this string, with the time prepended. */
   if (!(hist_str = malloc(strlen(history) + strlen(timestamp) + 2)))
      return CF_ENOMEM;

   /* Create a string with the time and then the user's history
    * comment. */
   sprintf(hist_str, "%s %s\n", timestamp, history);
   
   /* Now append that to the existing history att, or create one. */
   ret = nccf_append_att(ncid, CF_HISTORY, hist_str);

   /* Free our memory. */
   free(hist_str);

   return ret;
}

/* Read the CF annotations. Recall that in C, strlens do not include
 * the null terminator. To get the lengths before the strings (in
 * order to allocatate) pass NULL for any or all strngs and the
 * lengths will be returned. Then call the funtion again after
 * allocating memory. 

 * The CF version is guarenteed to be less than NC_MAX_NAME.

 * Any of these pointer args may be NULL, in which case it will be
 * ignored.
*/
int
nccf_inq_file(int ncid, size_t *title_lenp, char *title, 
	      size_t *history_lenp, char *history)
{
   int ret;

   /* Find length of title. */
   if (title_lenp)
      if ((ret = nc_inq_attlen(ncid, NC_GLOBAL, CF_TITLE, title_lenp)))
	 return ret;

   /* Get title. */
   if (title)
      if ((ret = nc_get_att_text(ncid, NC_GLOBAL, CF_TITLE, title)))
	 return ret;

   /* Find length of history. */
   if (history_lenp)
      if ((ret = nc_inq_attlen(ncid, NC_GLOBAL, CF_HISTORY, history_lenp)))
	 return ret;

   /* Get history. */
   if (history)
      if ((ret = nc_get_att_text(ncid, NC_GLOBAL, CF_HISTORY, history)))
	 return ret;

   return NC_NOERR;

}
/* Add any or all of these four attributes to a file or variable. */
int 
nccf_def_notes(int ncid, int varid, const char *institution, 
	       const char *source, const char *comment, 
	       const char *references)
{
   int ret;

   if (institution)
      if ((ret = nc_put_att_text(ncid, varid, CF_INSTITUTION,
				    strlen(institution) + 1, institution)))
	 return ret;

   if (source)
      if ((ret = nc_put_att_text(ncid, varid, CF_SOURCE,
				    strlen(source) + 1, source)))
	 return ret;

   if (comment)
      if ((ret = nc_put_att_text(ncid, varid, CF_COMMENT,
				    strlen(comment) + 1, comment)))
	 return ret;

   if (references)
      if ((ret = nc_put_att_text(ncid, varid, CF_REFERENCES,
				    strlen(references) + 1, references)))
	 return ret;

   return CF_NOERR;
}

/* Read any or all of these four attributes of a file or
 * variable. */
int nccf_inq_notes(int ncid, int varid, size_t *institution_lenp, 
		   char *institution, size_t *source_lenp, char *source, 
		   size_t *comment_lenp, char *comment, 
		   size_t *references_lenp, char *references)
{
   int ret;

   if (institution_lenp)
      if ((ret = nc_inq_attlen(ncid, varid, CF_INSTITUTION, 
				  institution_lenp)))
	 return ret;

   if (institution)
      if ((ret = nc_get_att_text(ncid, varid, CF_INSTITUTION,
				    institution)))
	 return ret;

   if (source_lenp)
      if ((ret = nc_inq_attlen(ncid, varid, CF_SOURCE, 
				  source_lenp)))
	 return ret;

   if (source)
      if ((ret = nc_get_att_text(ncid, varid, CF_SOURCE,
				    source)))
	 return ret;

   if (comment_lenp)
      if ((ret = nc_inq_attlen(ncid, varid, CF_COMMENT, 
				  comment_lenp)))
	 return ret;

   if (comment)
      if ((ret = nc_get_att_text(ncid, varid, CF_COMMENT,
				    comment)))
	 return ret;

   if (references_lenp)
      if ((ret = nc_inq_attlen(ncid, varid, CF_REFERENCES, 
				  references_lenp)))
	 return ret;

   if (references)
      if ((ret = nc_get_att_text(ncid, varid, CF_REFERENCES,
				    references)))
	 return ret;
   
   return NC_NOERR;
}

