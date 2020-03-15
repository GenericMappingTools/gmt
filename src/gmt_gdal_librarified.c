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

#if defined(HAVE_GDAL) && ((GDAL_VERSION_MAJOR >= 2) && (GDAL_VERSION_MINOR >= 1)) || (GDAL_VERSION_MAJOR >= 3)

#include <gdal_utils.h>

/* ------------------------------------------------------------------------------------------------------------ */
GMT_LOCAL GDALDatasetH gdal_vector (struct GMT_CTRL *GMT, char *fname) {
	/* Write data into a GDAL Vector memory dataset */
	unsigned int nt, ns, nr;
	double x, y, z;
	GDALDriverH hDriver;
	GDALDatasetH hDS;
	OGRLayerH hLayer;
	OGRFieldDefnH hFieldDefn;
	OGRFeatureH hFeature;
	OGRGeometryH hPt;
	struct GMT_DATASET *D = NULL;

	if ((D = GMT_Read_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_PLP, GMT_READ_NORMAL, NULL, fname, NULL)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to read input data.\n");
		return NULL;
	}
	if (D->n_columns != 3) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "This dataset doesn't have 3 columns as required.\n");
		return NULL;
	}

	GDALAllRegister();

	hDriver = GDALGetDriverByName("Memory");			/* Intrmediary MEM diver to use as arg to GDALCreateCopy method */

	hDS = GDALCreate(hDriver, "mem", 0, 0, 0, GDT_Unknown, NULL);
	if (hDS == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Creation of MEM file failed - %d\n%s\n", CPLGetLastErrorNo(), CPLGetLastErrorMsg());
		GDALDestroyDriverManager();
		return NULL;
	}

	hLayer = GDALDatasetCreateLayer(hDS, "point_out", NULL, wkbPoint, NULL);
	if (hLayer == NULL ) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Layer creation failed.\n");
		GDALDestroyDriverManager();
		return NULL;
	}

	hFieldDefn = OGR_Fld_Create("Name", OFTString);

	OGR_Fld_SetWidth(hFieldDefn, 32);

	if (OGR_L_CreateField(hLayer, hFieldDefn, TRUE) != OGRERR_NONE) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Creating Name field failed.\n");
		GDALDestroyDriverManager();
		return NULL;
	}

	OGR_Fld_Destroy(hFieldDefn);

	for (nt = 0; nt < D->n_tables; nt++) {
		for (ns = 0; ns < D->table[nt]->n_segments; ns++) {
			for (nr = 0; nr < D->table[nt]->segment[ns]->n_rows; nr++) {
				x = D->table[nt]->segment[ns]->data[0][nr];
				y = D->table[nt]->segment[ns]->data[1][nr];
				z = D->table[nt]->segment[ns]->data[2][nr];
				hFeature = OGR_F_Create(OGR_L_GetLayerDefn(hLayer));
				OGR_F_SetFieldString(hFeature, OGR_F_GetFieldIndex(hFeature, "Name"), "0");

				hPt = OGR_G_CreateGeometry(wkbPoint);
				OGR_G_SetPoint(hPt, 0, x, y, z);

				OGR_F_SetGeometry(hFeature, hPt);
				OGR_G_DestroyGeometry(hPt);

				if (OGR_L_CreateFeature(hLayer, hFeature) != OGRERR_NONE) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to create feature in dataset.\n");
					GDALDestroyDriverManager();
					return NULL;
				}
				OGR_F_Destroy(hFeature);
			}
		}
	}

	if (GMT_Destroy_Data (GMT->parent, &D)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure while freeing input data\n");
	}

	return hDS;
}

/* ------------------------------------------------------------------------------------------------------------ */
GMT_LOCAL char **breakMe(struct GMT_CTRL *GMT, char *in) {
	/* Breake a string "-aa -bb -cc dd" into tokens "-aa" "-bb" "-cc" "dd" */
	/* Based on GMT_Create_Options() */
	unsigned int pos = 0, k, n_args = 0;
	bool quoted;
	size_t n_alloc = 4 * GMT_SMALL_CHUNK;
	char p[GMT_LEN512] = {""}, *txt_in;	/* Passed a single text string */
	char **args = NULL;

	if (!in)		/* If empty, return empty */
		return NULL;

	txt_in = strdup (in);
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

/* ------------------------------------------------------------------------------------------------------------ */
GMT_LOCAL void free_args(struct GMT_CTRL *GMT, char **args) {
	int k = 0;
	while (k < 4 * GMT_SMALL_CHUNK && args[k]) {	/*  256 should be way larger then actual number of allocated items */
		free (args[k]);
		k++;
	}
	gmt_M_free (GMT, args);
}

/* ------------------------------------------------------------------------------------------------------------ */
GMT_LOCAL int save_grid_with_GMT(struct GMT_CTRL *GMT, GDALDatasetH hDstDS, struct GMT_GRID *Grid, char *fname) {
	/* Save a grid using the GMT machinery */
	unsigned char *tmp = NULL;
	int   nXSize, nYSize, nPixelSize, gdal_code;
	GDALRasterBandH	hBand;

	hBand = GDALGetRasterBand(hDstDS, 1);
	nPixelSize = GDALGetDataTypeSize(GDALGetRasterDataType(hBand)) / 8;	/* /8 because return value is in BITS */
	nXSize = GDALGetRasterXSize(hDstDS);
	nYSize = GDALGetRasterYSize(hDstDS);

	if (nXSize != (int)Grid->header->n_columns || nYSize != (int)Grid->header->n_rows) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Programming error. Output grid dimensions not what is expected.\n");
		return -1;
	}

	if (nPixelSize != sizeof(float)) {		/* If outdata type is not 4 bytes, must create a tmp to copy from because GMT requires floats */
		size_t k;
		if ((tmp = calloc(Grid->header->nm, sizeof(float))) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "grdgdal: failure to allocate temporary memory\n");
			return -1;
		}
		if ((gdal_code = GDALRasterIO(hBand, GF_Read, 0, 0, nXSize, nYSize, tmp,
		                              nXSize, nYSize, GDALGetRasterDataType(hBand), 0, 0)) != CE_None) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "GDALRasterIO failed to open band [err = %d]\n", gdal_code);
			return -1;
		}
		for (k = 0; k < Grid->header->nm; k++)
			Grid->data[k] = (float)tmp[k];

		free(tmp);
	}
	else {
		if ((gdal_code = GDALRasterIO(hBand, GF_Read, 0, 0, nXSize, nYSize, (void *)Grid->data,
		                              nXSize, nYSize, GDALGetRasterDataType(hBand), 0, 0)) != CE_None) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "GDALRasterIO failed to open band [err = %d]\n", gdal_code);
			return -1;
		}
	}

	gmt_grd_flip_vertical (Grid->data, (unsigned)nXSize, (unsigned)nYSize, 0, sizeof(float));
	if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA,
						NULL, fname, Grid) != GMT_NOERROR)
		return GMT->parent->error;
	GDALClose(hDstDS);
	return 0;
}

/* ------------------------------------------------------------------------------------------------------------ */
char *out_name(struct GMT_GDALLIBRARIFIED_CTRL *GDLL) {
	/* Pick the right output name when saving grids depending on if that writing is done with GMT or GDAL */
	if (GDLL->M.write_gdal)			/* Write grid with the GDAL machinery */
		return GDLL->fname_out;
	else
#ifdef WIN32
		return "NUL";
#else
		return "/dev/null";
#endif
}

/* ------------------------------------------------------------------------------------------------------------ */
GMT_LOCAL void add_defaults(struct GMT_CTRL *GMT, struct GMT_GDALLIBRARIFIED_CTRL *GDLL, char *ext_opts) {
	/* Add defaults to a netCDF output when file is to be written by GDAL, or just set a MEM driver if for GMT */
	if (GDLL->opts) strcat(ext_opts, GDLL->opts);
	if (GDLL->M.write_gdal) {
		char *ext;
		if ((ext = gmt_get_ext (GDLL->fname_out)) != NULL) {
			if (!strcasecmp(ext, "nc") || !strcasecmp(ext, "grd")) {
				if (!GDLL->opts) {
					strcat(ext_opts, " -of netCDF -co FORMAT=NC4 -co ZLEVEL=5 -co COMPRESS=DEFLATE -co CHUNKING=YES");
				}
				else {
					if (!strstr(GDLL->opts, "netCDF=")) strcat(ext_opts, " -of netCDF");
					if (!strstr(GDLL->opts, "FORMAT=")) strcat(ext_opts, " -co FORMAT=NC4");
					if (!strstr(GDLL->opts, "ZLEVEL=")) strcat(ext_opts, " -co ZLEVEL=5");
					if (!strstr(GDLL->opts, "COMPRESS=")) strcat(ext_opts, " -co COMPRESS=DEFLATE");
					if (!strstr(GDLL->opts, "CHUNKING=")) strcat(ext_opts, " -co CHUNKING=YES");
				}
			}
		}
	}
	else
		strcat(ext_opts, " -of MEM");	/* For GMT we need the data in the MEM driver */

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "gdal options used: %s\n", ext_opts);
}


/* ------------------------------------------------------------------------------------------------------------ */
GMT_LOCAL int init_open(struct GMT_CTRL *GMT, struct GMT_GDALLIBRARIFIED_CTRL *GDLL, GDALDatasetH *hSrcDS, struct GMT_GRID **Grid, unsigned int mode) {
	/* Initialize GDAL, read data and create a GMT Grid container
	   These operations are common to several functions, so wrap them in a function */

	GDALAllRegister();

	if (mode == GMT_IS_DATASET) {				/* Read vector data */
		if (GDLL->M.read_gdal) 		/* Read input data with the GDAL machinery */
			*hSrcDS = GDALOpenEx(GDLL->fname_in, GDAL_OF_VECTOR | GDAL_OF_VERBOSE_ERROR, NULL, NULL, NULL);
		else
			*hSrcDS = gdal_vector (GMT, GDLL->fname_in);

		if (*hSrcDS == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GDALOpen failed %s\n", CPLGetLastErrorMsg());
			return -1;
		}
	}
	else {							/* Read raster data directly in GDAL */
		*hSrcDS = GDALOpen(GDLL->fname_in, GA_ReadOnly);
	}

	if ((*Grid = GMT_Create_Data (GMT->parent, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL,
		                          GMT_GRID_DEFAULT_REG, 0, NULL)) == NULL)
		return GMT->parent->error;
	return 0;
}

/* ------------------------------------------------------------------------------------------------------------ */
GMT_LOCAL int sanitize_and_save(struct GMT_CTRL *GMT, struct GMT_GDALLIBRARIFIED_CTRL *GDLL, int bUsageError,
                                GDALDatasetH *hSrcDS, GDALDatasetH *hDstDS, struct GMT_GRID *Grid, char **args,
                                char *prog) {
	int error = 0;
	if (bUsageError == TRUE) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gdal_%s: failure\n", prog);
		return -1;
	}

	if (!GDLL->M.write_gdal) 		/* Write grid with the GMT machinery */
		error = save_grid_with_GMT(GMT, hDstDS, Grid, GDLL->fname_out);

	free_args(GMT, args);
	GDALClose(hSrcDS);
	return error;
}

/* ------------------------------------------------------------------------------------------------------------ */
int gmt_gdal_info (struct GMT_CTRL *GMT, struct GMT_GDALLIBRARIFIED_CTRL *GDLL) {
	char **args;
	GDALDatasetH	hSrcDS;
	GDALInfoOptions *psOptions;

	GDALAllRegister();

	hSrcDS = GDALOpen(GDLL->fname_in, GA_ReadOnly);

	if (hSrcDS == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "GDALOpen failed %s\n", CPLGetLastErrorMsg());
		return -1;
	}

	args = breakMe(GMT, GDLL->opts);
	psOptions = GDALInfoOptionsNew(args, NULL);
	GMT_Message (GMT->parent, GMT_TIME_NONE, "GDAL Info\n\n%s\n", GDALInfo(hSrcDS, psOptions));

	if (args) free_args(GMT, args);
	GDALClose(hSrcDS);
	GDALInfoOptionsFree(psOptions);
	GDALDestroyDriverManager();
	return 0;
}

/* ------------------------------------------------------------------------------------------------------------ */
int gmt_gdal_dem (struct GMT_CTRL *GMT, struct GMT_GDALLIBRARIFIED_CTRL *GDLL) {
	char ext_opts[GMT_LEN512] = {""}, **args;
	char *method = NULL, *cpt_name = NULL;
	int   bUsageError = 0, error = 0;
	struct GMT_GRID *Grid = NULL;
	GDALDatasetH	hSrcDS, hDstDS;
	GDALDEMProcessingOptions *psOptions;

	if ((error = init_open(GMT, GDLL, &hSrcDS, &Grid, GMT_IS_GRID)))	/* Init GDAL and read input data */
		return error;

	add_defaults(GMT, GDLL, &ext_opts[0]);

	args = breakMe(GMT, ext_opts);
	psOptions = GDALDEMProcessingOptionsNew(args, NULL);
	method = GDLL->dem_method ? GDLL->dem_method : "hillshade";
	cpt_name = GDLL->dem_cpt ? GDLL->dem_cpt : NULL;
	hDstDS = GDALDEMProcessing(out_name(GDLL), hSrcDS, method, cpt_name, psOptions, &bUsageError);
	error = sanitize_and_save(GMT, GDLL, bUsageError, hSrcDS, hDstDS, Grid, args, "dem");

	GDALDEMProcessingOptionsFree(psOptions);
    OGRCleanupAll();
	GDALDestroyDriverManager();
	return error;
}

/* ------------------------------------------------------------------------------------------------------------ */
int gmt_gdal_grid(struct GMT_CTRL *GMT, struct GMT_GDALLIBRARIFIED_CTRL *GDLL) {
	char ext_opts[GMT_LEN512] = {""}, **args;
	int   bUsageError = 0, error = 0;
	double dx = 0, dy = 0;
	struct GMT_GRID *Grid = NULL;
	GDALDatasetH	hSrcDS, hDstDS;
	GDALGridOptions *psOptions;

	if ((error = init_open(GMT, GDLL, &hSrcDS, &Grid, GMT_IS_DATASET)))	/* Init GDAL and read input data */
		return error;

	if (GDLL->M.write_gdal && Grid->header->registration == 0) {
		/* Since GDAL writes only pixel-reg grids, expand limits so that inc is respected */
		dx = Grid->header->inc[0] / 2;		dy = Grid->header->inc[1] / 2;
	}
	sprintf(ext_opts, "-ot Float32 -txe %lf %lf -tye %lf %lf -outsize %d %d ",
			Grid->header->wesn[XLO]-dx, Grid->header->wesn[XHI]+dx, Grid->header->wesn[YLO]-dy,
			Grid->header->wesn[YHI]+dy, Grid->header->n_columns, Grid->header->n_rows);

	add_defaults(GMT, GDLL, &ext_opts[0]);

	args = breakMe(GMT, ext_opts);
	psOptions = GDALGridOptionsNew(args, NULL);
	hDstDS = GDALGrid(out_name(GDLL), hSrcDS, psOptions, &bUsageError);
	error = sanitize_and_save(GMT, GDLL, bUsageError, hSrcDS, hDstDS, Grid, args, "grid");

	GDALGridOptionsFree(psOptions);
	GDALDestroyDriverManager();
	return error;
}

/* ------------------------------------------------------------------------------------------------------------ */
int gmt_gdal_rasterize(struct GMT_CTRL *GMT, struct GMT_GDALLIBRARIFIED_CTRL *GDLL) {
	char ext_opts[GMT_LEN512] = {""}, **args;
	int   bUsageError = 0, error = 0;
	double dx = 0, dy = 0;
	struct GMT_GRID *Grid = NULL;
	GDALDatasetH	hSrcDS, hDstDS;
	GDALRasterizeOptions *psOptions;

	if ((error = init_open(GMT, GDLL, &hSrcDS, &Grid, GMT_IS_DATASET)))	/* Init GDAL and read input data */
		return error;

	if (GDLL->M.write_gdal && Grid->header->registration == 0) {
		/* Since GDAL writes only pixel-reg grids, expand limits so that inc is respected */
		dx = Grid->header->inc[0] / 2;		dy = Grid->header->inc[1] / 2;
	}
	sprintf(ext_opts, "-ot Float32 -te %lf %lf %lf %lf -ts %d %d ",
			Grid->header->wesn[XLO]-dx, Grid->header->wesn[YLO]-dy, Grid->header->wesn[XHI]+dx,
			Grid->header->wesn[YHI]+dy, Grid->header->n_columns, Grid->header->n_rows);

	add_defaults(GMT, GDLL, &ext_opts[0]);

	args = breakMe(GMT, ext_opts);
	psOptions = GDALRasterizeOptionsNew(args, NULL);
	hDstDS = GDALRasterize(out_name(GDLL), NULL, hSrcDS, psOptions, &bUsageError);
	error = sanitize_and_save(GMT, GDLL, bUsageError, hSrcDS, hDstDS, Grid, args, "rasterize");

	GDALRasterizeOptionsFree(psOptions);
	GDALDestroyDriverManager();
	return error;
}

/* ------------------------------------------------------------------------------------------------------------ */
int gmt_gdal_translate (struct GMT_CTRL *GMT, struct GMT_GDALLIBRARIFIED_CTRL *GDLL) {
	char ext_opts[GMT_LEN512] = {""}, **args;
	int   bUsageError = 0, error = 0;
	struct GMT_GRID *Grid = NULL;
	GDALDatasetH	hSrcDS, hDstDS;
	GDALTranslateOptions *psOptions;

	if ((error = init_open(GMT, GDLL, &hSrcDS, &Grid, GMT_IS_GRID)))	/* Init GDAL and read input data */
		return error;
	add_defaults(GMT, GDLL, &ext_opts[0]);

	args = breakMe(GMT, ext_opts);
	psOptions = GDALTranslateOptionsNew(args, NULL);
	hDstDS = GDALTranslate(out_name(GDLL), hSrcDS, psOptions, &bUsageError);
	error = sanitize_and_save(GMT, GDLL, bUsageError, hSrcDS, hDstDS, Grid, args, "translate");

	GDALTranslateOptionsFree(psOptions);
	GDALDestroyDriverManager();
	return error;
}

/* ------------------------------------------------------------------------------------------------------------ */
int gmt_gdal_warp (struct GMT_CTRL *GMT, struct GMT_GDALLIBRARIFIED_CTRL *GDLL) {
	char ext_opts[GMT_LEN512] = {""}, **args;
	int   bUsageError = 0, error = 0;
	struct GMT_GRID *Grid = NULL;
	GDALDatasetH	hSrcDS, hDstDS;
	GDALWarpAppOptions *psOptions;

	if ((error = init_open(GMT, GDLL, &hSrcDS, &Grid, GMT_IS_GRID)))	/* Init GDAL and read input data */
		return error;
	add_defaults(GMT, GDLL, &ext_opts[0]);

	args = breakMe(GMT, ext_opts);
	psOptions = GDALWarpAppOptionsNew(args, NULL);
	//hDstDS = GDALWarp(out_name(GDLL), hDstDS, nSrcCount, pahSrcDS, psOptions, &bUsageError);
	hDstDS = GDALWarp(out_name(GDLL), NULL, 1, &hSrcDS, psOptions, &bUsageError);
	error = sanitize_and_save(GMT, GDLL, bUsageError, hSrcDS, hDstDS, Grid, args, "warp");

	GDALWarpAppOptionsFree(psOptions);
	GDALDestroyDriverManager();
	return error;
}
#endif		//defined(HAVE_GDAL) && ...
