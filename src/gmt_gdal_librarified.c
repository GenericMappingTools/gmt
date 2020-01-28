/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/

#include "gmt_dev.h"
#include "gmt_internals.h"

#if defined(HAVE_GDAL) && (GDAL_VERSION_MAJOR >= 2) && (GDAL_VERSION_MINOR >= 1)

#include <gdal_utils.h>

GMT_LOCAL char **breakMe(struct GMT_CTRL *GMT, char *in) {
	/* Breake a string "-aa -bb -cc dd" into tokens "-aa" "-bb" "-cc" "dd" */
	/* Based on GMT_Create_Options() */
	unsigned int pos = 0, k;
	int  n_args = 0;
	bool quoted;
	size_t n_alloc = GMT_SMALL_CHUNK;
	char p[GMT_BUFSIZ] = {""}, *txt_in = strdup (in);	/* Passed a single text string */
	char **args = NULL;
	args = gmt_M_memory (GMT, NULL, n_alloc, char *);
	/* txt_in can contain options that take multi-word text strings, e.g., -B+t"My title".  We avoid the problem of splitting
	 * these items by temporarily replacing spaces inside quoted strings with ASCII 31 US (Unit Separator), do the strtok on
	 * space, and then replace all ASCII 31 with space at the end (we do the same for tab using ASCII 29 GS (group separator) */
	for (k = 0, quoted = false; txt_in[k]; k++) {
		if (txt_in[k] == '\"') quoted = !quoted;	/* Initially false, becomes true at start of quote, then false when exit quote */
		else if (quoted && txt_in[k] == '\t') txt_in[k] = GMT_ASCII_GS;
		else if (quoted && txt_in[k] == ' ')  txt_in[k] = GMT_ASCII_US;
	}
	while ((gmt_strtok (txt_in, " ", &pos, p))) {	/* Break up string into separate words, and strip off double quotes */
		int i, o;
		for (k = 0; p[k]; k++) {
			if (p[k] == GMT_ASCII_GS)
				p[k] = '\t';
			else if (p[k] == GMT_ASCII_US)
				p[k] = ' ';						/* Replace spaces and tabs masked above */
		}
		for (i = o = 0; p[i]; i++)
			if (p[i] != '\"') p[o++] = p[i];	/* Ignore double quotes */
		p[o] = '\0';
		args[n_args++] = strdup(p);

		if (n_args == n_alloc) {
			n_alloc += GMT_SMALL_CHUNK;
			args = gmt_M_memory(GMT, args, n_alloc, char *);
		}
	}
	for (k = 0; txt_in[k]; k++)	/* Restore input string to prestine condition */
		if (txt_in[k] == GMT_ASCII_GS) txt_in[k] = '\t';
		else if (txt_in[k] == GMT_ASCII_US) txt_in[k] = ' ';	/* Replace spaces and tabs masked above */

	free (txt_in);
	return args;
}

int gmt_gdal_info (struct GMT_CTRL *GMT, char *gdal_filename, char *opts) {
	char	*info = NULL, **args;
	GDALDatasetH	hDataset;
	GDALInfoOptions *psOptions;

	/* Open gdal - */

	GDALAllRegister();

	hDataset = GDALOpen(gdal_filename, GA_ReadOnly);

	if (hDataset == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GDALOpen failed %s\n", CPLGetLastErrorMsg());
		return (-1);
	}

	args = breakMe(GMT, opts);
	psOptions = GDALInfoOptionsNew(args, NULL);
	info = GDALInfo(hDataset, psOptions);
	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GDAL Info\n\n%s\n", info);

	GDALInfoOptionsFree(psOptions);
	GDALClose(hDataset);
	GDALDestroyDriverManager();
	return 0;
}


int gmt_gdal_grid(struct GMT_CTRL *GMT, char *gdal_filename, char *opts, char *outname) {
	char **args, ext_opts[GMT_LEN512] = {""};
	unsigned char *tmp = NULL;
	int   nXSize, nYSize, nPixelSize;
	int   bUsageError, gdal_code;
	struct GMT_GRID *Grid = NULL;
	GDALDatasetH	hSrcDS, hDstDS;
	GDALGridOptions *psOptions;
	GDALRasterBandH	hBand;

	/* Open gdal - */

	GDALAllRegister();

	hSrcDS = GDALOpenEx(gdal_filename, GDAL_OF_VECTOR | GDAL_OF_VERBOSE_ERROR, NULL, NULL, NULL);

	if (hSrcDS == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GDALOpen failed %s\n", CPLGetLastErrorMsg());
		return -1;
	}
	
	if ((Grid = GMT_Create_Data (GMT->parent, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL,
	                             GMT_GRID_DEFAULT_REG, 0, NULL)) == NULL)
		return GMT->parent->error;

	sprintf(ext_opts, "-of MEM -ot Float32 -txe %lf %lf -tye %lf %lf -outsize %d %d ",
	        Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI],
			Grid->header->n_columns, Grid->header->n_rows);
	strcat(ext_opts, opts); 

	args = breakMe(GMT, ext_opts);
	psOptions = GDALGridOptionsNew(args, NULL);
#ifdef WIN32
	hDstDS = GDALGrid("NUL", hSrcDS, psOptions, &bUsageError);
#else
	hDstDS = GDALGrid("/dev/null", hSrcDS, psOptions, &bUsageError);
#endif
	if (bUsageError == TRUE) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gdal_grid: failure\n");
		GDALDestroyDriverManager();
		return -1;
	}

	hBand = GDALGetRasterBand(hDstDS, 1);
	nPixelSize = GDALGetDataTypeSize(GDALGetRasterDataType(hBand)) / 8;	/* /8 because return value is in BITS */
	nXSize = GDALGetRasterXSize(hDstDS);
	nYSize = GDALGetRasterYSize(hDstDS);

	if ((tmp = calloc((size_t)nYSize * (size_t)nXSize, nPixelSize)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gdalread: failure to allocate enough memory\n");
		GDALDestroyDriverManager();
		return -1;
	}

	if ((gdal_code = GDALRasterIO(hBand, GF_Read, 0, 0, nXSize, nYSize, tmp,
	                nXSize, nYSize, GDALGetRasterDataType(hBand), 0, 0)) != CE_None) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GDALRasterIO failed to open band [err = %d]\n", gdal_code);
	}
	Grid->data = (float *)tmp;
	if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, outname, Grid) != GMT_NOERROR)
		return GMT->parent->error;

	GDALGridOptionsFree(psOptions);
	GDALClose(hSrcDS);
	GDALDestroyDriverManager();
	return 0;
}

#endif		//defined(HAVE_GDAL) && ...