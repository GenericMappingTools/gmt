/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * gshhs_version.c contains helper functions to access the GSHHS version
 *
 * Author:  Florian Wobbe
 * Date:    5-APR-2012
 * Version: 5
 *
 * Modules in this file:
 *
 *  gshhs_get_version            Obtain version information struct from GSHHS file
 *  gshhs_require_min_version    Check if GSHHS file meets the min version
 *                               requirement
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <netcdf.h>
#include "gshhs_version.h"

#define BUF_SIZE 64
#define VERSION_ATT_NAME "version"
#define FAILURE_PREFIX "gshhs_version: "


/* Get value from VERSION_ATT_NAME of netCDF file and populate gshhs_version,
 * gshhs_version_major, gshhs_version_minor, and gshhs_version_patch */
int gshhs_get_version (const char* filename, struct GSHHS_VERSION *gshhs_version) {
	int    status;                         /* error status */
	int    ncid;                           /* netCDF ID */
	size_t v_len;                          /* version length */
	char   gshhs_version_string[BUF_SIZE]; /* GSHHS version string */

	/* open shoreline file */
	status = nc_open(filename, NC_NOWRITE, &ncid);
	if (status != NC_NOERR) {
		fprintf(stderr, FAILURE_PREFIX "cannot open file \"%s\" (%d).\n", filename, status);
		return false;
	}

	/* get length of version string */
	status = nc_inq_attlen (ncid, NC_GLOBAL, VERSION_ATT_NAME, &v_len);
	if (status != NC_NOERR) {
		fprintf(stderr, FAILURE_PREFIX "cannot inquire version attribute length from file \"%s\" (%d).\n", filename, status);
		return false;
	}
	if (v_len == 0 || v_len > BUF_SIZE) {
		fprintf(stderr, FAILURE_PREFIX "invalid version attribute length: %zd\n", v_len);
		return false;
	}

	/* get version string */
	status = nc_get_att_text (ncid, NC_GLOBAL, VERSION_ATT_NAME, gshhs_version_string);
	if (status != NC_NOERR) {
		fprintf(stderr, FAILURE_PREFIX "cannot read version attribute from file \"%s\" (%d).\n", filename, status);
		return false;
	}

	/* null-terminate version string */
	gshhs_version_string[v_len] = '\0';

	/* parse version major, minor, and patch */
	status = sscanf (gshhs_version_string, "%u.%u.%u",
									 &gshhs_version->major, &gshhs_version->minor, &gshhs_version->patch);
	if (status != 3) {
		fprintf(stderr, FAILURE_PREFIX "cannot parse version string \"%s\" (%d).\n", gshhs_version_string, status);
		return false;
	}

	return true;
}

/* Check if GSHHS file meets the min version requirement */
int gshhs_require_min_version (const char* filename, const struct GSHHS_VERSION min_version) {
  struct GSHHS_VERSION version;
	/* get version of file */
	if ( ! gshhs_get_version (filename, &version) )
		return false;

	/* compare versions */
	if ( version.major < min_version.major )
		return false;
	if ( version.minor < min_version.minor )
		return false;
	if ( version.patch < min_version.patch )
		return false;

#ifdef GSHHS_VERSION_DEBUG
	fprintf (stderr, FAILURE_PREFIX "%s\n", filename);
#endif
	return true;
}

#ifdef STANDALONE

/* Compile with -DSTANDALONE to make an executable.
 * This code is executed by CMake to check the installed GSHHS version. */

int main (int argc, char *argv[]) {
	struct GSHHS_VERSION gshhs_version;
	int status;

	if (argc < 2 || argc > 3) {
		fprintf(stderr, FAILURE_PREFIX "usage: gshhs_version file [min_required_version]\n");
		return EXIT_FAILURE;
	}

	if ( ! gshhs_get_version (argv[1], &gshhs_version) )
		return EXIT_FAILURE;

	/* return version string and exit */
	fprintf(stdout, "%d.%d.%d\n",
					gshhs_version.major,
					gshhs_version.minor,
					gshhs_version.patch);

	if (argc > 2) {
		/* do min version check */
		struct GSHHS_VERSION required_version;
		status = sscanf (argv[2], "%u.%u.%u",
										 &required_version.major,
										 &required_version.minor,
										 &required_version.patch);
		if ( status != 3 ) {
			fprintf (stderr, FAILURE_PREFIX "cannot parse version string \"%s\" (%d).\n", argv[2], status);
		}
		if ( ! gshhs_require_min_version (argv[1], required_version) ) {
			/* version too old */
			fprintf (stderr, FAILURE_PREFIX "verion of %s < min required version %s.\n", argv[1], argv[2]);
			return -1;
		}
	}

	/* version >= min required version */
	return EXIT_SUCCESS;
}

#endif /* STANDALONE */
