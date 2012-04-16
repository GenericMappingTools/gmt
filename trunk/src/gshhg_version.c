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
 * gshhg_version.c contains helper functions to access the GSHHG version
 *
 * Author:  Florian Wobbe
 * Date:    5-APR-2012
 * Version: 5
 *
 * Modules in this file:
 *
 *  gshhg_get_version            Obtain version information struct from GSHHG file
 *  gshhg_require_min_version    Check if GSHHG file meets the min version
 *                               requirement
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>
#include "gshhg_version.h"

#define BUF_SIZE 64
#define VERSION_ATT_NAME "version"
#define FAILURE_PREFIX "gshhg_version: "

/* Suppress Visual Studio deprecation warnings */
#	ifdef _MSC_VER
#		pragma warning( disable : 4996 )
#	endif

/* Get value from VERSION_ATT_NAME of netCDF file and populate gshhg_version,
 * gshhg_version_major, gshhg_version_minor, and gshhg_version_patch */
int gshhg_get_version (const char* filename, struct GSHHG_VERSION *gshhg_version) {
	int    status;                         /* error status */
	int    ncid;                           /* netCDF ID */
	size_t v_len;                          /* version length */
	char   gshhg_version_string[BUF_SIZE]; /* GSHHG version string */

	/* open shoreline file */
	status = nc_open(filename, NC_NOWRITE, &ncid);
	if (status != NC_NOERR) {
		fprintf(stderr, FAILURE_PREFIX "cannot open file \"%s\" (%d).\n", filename, status);
		return 0;
	}

	/* get length of version string */
	status = nc_inq_attlen (ncid, NC_GLOBAL, VERSION_ATT_NAME, &v_len);
	if (status != NC_NOERR) {
		fprintf(stderr, FAILURE_PREFIX "cannot inquire version attribute length from file \"%s\" (%d).\n", filename, status);
		return 0;
	}
	if (v_len == 0 || v_len > BUF_SIZE) {
		fprintf(stderr, FAILURE_PREFIX "invalid version attribute length: %zd\n", v_len);
		return 0;
	}

	/* get version string */
	status = nc_get_att_text (ncid, NC_GLOBAL, VERSION_ATT_NAME, gshhg_version_string);
	if (status != NC_NOERR) {
		fprintf(stderr, FAILURE_PREFIX "cannot read version attribute from file \"%s\" (%d).\n", filename, status);
		return 0;
	}

	/* null-terminate version string */
	gshhg_version_string[v_len] = '\0';

	/* parse version major, minor, and patch */
	status = sscanf (gshhg_version_string, "%u.%u.%u",
									 &gshhg_version->major, &gshhg_version->minor, &gshhg_version->patch);
	if (status != 3) {
		fprintf(stderr, FAILURE_PREFIX "cannot parse version string \"%s\" (%d).\n", gshhg_version_string, status);
		return 0;
	}

	return 1;
}

/* Check if GSHHG file meets the min version requirement */
int gshhg_require_min_version (const char* filename, const struct GSHHG_VERSION min_version) {
  struct GSHHG_VERSION version;
	/* get version of file */
	if ( ! gshhg_get_version (filename, &version) )
		return 0;

	/* compare versions */
	if ( version.major < min_version.major )
		return 0;
	if ( version.minor < min_version.minor )
		return 0;
	if ( version.patch < min_version.patch )
		return 0;

#ifdef GSHHG_VERSION_DEBUG
	fprintf (stderr, FAILURE_PREFIX "%s\n", filename);
#endif
	return 1;
}

#ifdef STANDALONE

/* Compile with -DSTANDALONE to make an executable.
 * This code is executed by CMake to check the installed GSHHG version. */

int main (int argc, char *argv[]) {
	struct GSHHG_VERSION gshhg_version;
	int status;

	if (argc < 2 || argc > 3) {
		fprintf(stderr, FAILURE_PREFIX "usage: gshhg_version file [min_required_version]\n");
		return EXIT_FAILURE;
	}

	if ( ! gshhg_get_version (argv[1], &gshhg_version) )
		return EXIT_FAILURE;

	/* return version string and exit */
	fprintf(stdout, "%d.%d.%d\n",
					gshhg_version.major,
					gshhg_version.minor,
					gshhg_version.patch);

	if (argc > 2) {
		/* do min version check */
		struct GSHHG_VERSION required_version;
		status = sscanf (argv[2], "%u.%u.%u",
										 &required_version.major,
										 &required_version.minor,
										 &required_version.patch);
		if ( status != 3 ) {
			fprintf (stderr, FAILURE_PREFIX "cannot parse version string \"%s\" (%d).\n", argv[2], status);
		}
		if ( ! gshhg_require_min_version (argv[1], required_version) ) {
			/* version too old */
			fprintf (stderr, FAILURE_PREFIX "version of %s < min required version %s.\n", argv[1], argv[2]);
			return -1;
		}
	}

	/* version >= min required version */
	return EXIT_SUCCESS;
}

#endif /* STANDALONE */
