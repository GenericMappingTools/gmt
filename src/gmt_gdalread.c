/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
/* Program:	gmt_gdalread.c
 * Purpose:	routine to read files supported by gdal
 * 		and dumping all or selected band data of that dataset.
 *
 * Author:	Joaquim Luis
 * Date:	27-Aug-2009
 * Revision: 1		Based on gdalread.c MEX from Mirone
 *			The populate_metadata() was modified from mexgdal of John Evans (johnevans@acm.org)
 *
 * Public functions (2):
 *
 *	gmtlib_is_gdal_grid : Determine if a grid is GDAL readable
 *	gmt_gdalread     : Read a GDAL grid
 */

/* Local functions */

GMT_LOCAL GDALDatasetH gdal_open (struct GMT_CTRL *GMT, char *gdal_filename) {
	char *file = NULL, path[PATH_MAX] = {""}, *c = NULL;
	if (gmtlib_found_url_for_gdal (gdal_filename))	/* A vis*** URL, pass to GDAL as is */
		strncpy (path, gdal_filename, PATH_MAX-1);
	else if ((strlen(gdal_filename) > 2) && (c = strchr(&gdal_filename[2], ':'))) {		/* Assume it is a SUBDATASET */
		if (GMT->parent->cache) {
			c[0] = '\0';
			sprintf (path, "%s:%s/%s", gdal_filename, GMT->session.CACHEDIR, &c[1]);
			c[0] = ':';
		}
		else
			strncpy (path, gdal_filename, PATH_MAX-1);
	}
	else if ((file = gmt_getdatapath (GMT, gdal_filename, path, R_OK)) == NULL) {	/* Local file not found */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to find %s.\n", gdal_filename);
		return (NULL);
	}
	return (GDALOpen (path, GA_ReadOnly));
}

GMT_LOCAL int gdal_decode_columns (struct GMT_CTRL *GMT, char *txt, int *whichBands) {
	unsigned int n = 0, i, start, stop, pos = 0;
	char p[GMT_LEN256];
	gmt_M_unused(GMT);

	while ((gmt_strtok (txt, ",", &pos, p))) {
		if (strchr (p, '-')) {
			sscanf (p, "%d-%d", &start, &stop);
			for (i = start; i <= stop; i++)
				whichBands[n++] = i;
		}
		else
			sscanf (p, "%d", &whichBands[n++]);
	}
	for (i = 0; i < n; i++) whichBands[i] += 1;	/* Band numbering in GMT is 0 based */
	return ((int)n);
}

/* ---------------------------------------------------------------------------------
 * record_geotransform:
 *
 * If the gdal file is not internally georeferenced, try to get the world file.
 * Returns -1 in case no world file is found.
 */
GMT_LOCAL int record_geotransform (char *gdal_filename, GDALDatasetH hDataset, double *adfGeoTransform) {
	int status = -1;
	char generic_buffer[5000];

	if (GDALGetGeoTransform(hDataset, adfGeoTransform) == CE_None)
		return (0);

	/* Try a world file.  First the generic extension. If the gdal_filename
	   is, say, "a.tif", then this will look for "a.wld". */
	if (GDALReadWorldFile (gdal_filename, "wld", adfGeoTransform))
		return (0);

	/* Try again, but try "a.tif.wld" instead. */
	snprintf (generic_buffer, 5000, "%s.xxx", gdal_filename);
	status = GDALReadWorldFile (generic_buffer, "wld", adfGeoTransform);
	if (status == 1)
		return (0);

	return (-1);
}

/************************************************************************/
/*                        ReportCorner()                                */
/************************************************************************/

GMT_LOCAL int ReportCorner (struct GMT_CTRL *GMT, GDALDatasetH hDataset, OGRCoordinateTransformationH hTransform,
                            double x, double y, double *xy_c, double *xy_geo) {
	double	dfGeoX, dfGeoY;
	double	adfGeoTransform[6];

	xy_geo[0] = xy_geo[1] = GMT->session.d_NaN;		/* Default return values */
	/* -------------------------------------------------------------------- */
	/*      Transform the point into georeferenced coordinates.             */
	/* -------------------------------------------------------------------- */
	if (GDALGetGeoTransform(hDataset, adfGeoTransform) == CE_None) {
		if (!strcmp(GDALGetDriverShortName(GDALGetDatasetDriver(hDataset)),"netCDF") && GDAL_VERSION_NUM <= 1450) {
			adfGeoTransform[3] *= -1;
			adfGeoTransform[5] *= -1;
		}
		dfGeoX = adfGeoTransform[0] + adfGeoTransform[1] * x + adfGeoTransform[2] * y;
		dfGeoY = adfGeoTransform[3] + adfGeoTransform[4] * x + adfGeoTransform[5] * y;
	}
	else {
		xy_c[0] = x;	xy_c[1] = y;
		return false;
	}

	/* -------------------------------------------------------------------- */
	/*      Report the georeferenced coordinates.                           */
	/* -------------------------------------------------------------------- */
	xy_c[0] = dfGeoX;	xy_c[1] = dfGeoY;

	/* --------------------------------------------------------------------
	 *      Transform to latlong and report.
	 * -------------------------------------------------------------------- */
	if (hTransform != NULL && OCTTransform(hTransform,1,&dfGeoX,&dfGeoY,NULL)) {
		xy_geo[0] = dfGeoX;
		xy_geo[1] = dfGeoY;
	}

	return 0;
}

/* -------------------------------------------------------------------- */
GMT_LOCAL void ComputeRasterMinMax(struct GMT_CTRL *GMT, unsigned char *tmp, GDALRasterBandH hBand, double adfMinMax[2],
			int nXSize, int nYSize, double z_min, double z_max) {
	/* Compute Min/Max of a sub-region. I'm forced to do this because the
	GDALComputeRasterMinMax works only on the entire dataset */
	int	i, bGotNoDataValue;
	int16_t	tmpI16;
	uint16_t	tmpUI16;
	int32_t	tmpI32;
	uint32_t	tmpUI32;
	float	tmpF32;
	double	dfNoDataValue, tmpF64;

	dfNoDataValue = GDALGetRasterNoDataValue(hBand, &bGotNoDataValue);
	switch (GDALGetRasterDataType(hBand)) {
		case GDT_Byte:
			for (i = 0; i < nXSize*nYSize; i++) {
				if (bGotNoDataValue && tmp[i] == dfNoDataValue)
					continue;
				z_min = MIN(tmp[i], z_min);
				z_max = MAX(tmp[i], z_max);
			}
			break;
		case GDT_Int16:
			for (i = 0; i < nXSize*nYSize; i++) {
				memcpy (&tmpI16, &tmp[i * sizeof(int16_t)], sizeof(int16_t));
				if (bGotNoDataValue && tmpI16 == dfNoDataValue)
					continue;
				z_min = MIN(tmpI16, z_min);
				z_max = MAX(tmpI16, z_max);
			}
			break;
		case GDT_UInt16:
			for (i = 0; i < nXSize*nYSize; i++) {
				memcpy (&tmpUI16, &tmp[i * sizeof(uint16_t)], sizeof(uint16_t));
				if (bGotNoDataValue && tmpUI16 == dfNoDataValue)
					continue;
				z_min = MIN(tmpUI16, z_min);
				z_max = MAX(tmpUI16, z_max);
			}
			break;
		case GDT_Int32:
			for (i = 0; i < nXSize*nYSize; i++) {
				memcpy (&tmpI32, &tmp[i * sizeof(int32_t)], sizeof(int32_t));
				if (bGotNoDataValue && tmpI32 == dfNoDataValue)
					continue;
				z_min = MIN(tmpI32, z_min);
				z_max = MAX(tmpI32, z_max);
			}
			break;
		case GDT_UInt32:
			for (i = 0; i < nXSize*nYSize; i++) {
				memcpy (&tmpUI32, &tmp[i * sizeof(uint32_t)], sizeof(uint32_t));
				if (bGotNoDataValue && tmpUI32 == dfNoDataValue)
					continue;
				z_min = MIN(tmpUI32, z_min);
				z_max = MAX(tmpUI32, z_max);
			}
			break;
		case GDT_Float32:
			for (i = 0; i < nXSize*nYSize; i++) {
				memcpy (&tmpF32, &tmp[i * sizeof(float)], sizeof(float));
				if (bGotNoDataValue && tmpF32 == dfNoDataValue)
					continue;
				z_min = MIN(tmpF32, z_min);
				z_max = MAX(tmpF32, z_max);
			}
			break;
		case GDT_Float64:
			for (i = 0; i < nXSize*nYSize; i++) {
				memcpy (&tmpF64, &tmp[i * sizeof(double)], sizeof(double));
				if (bGotNoDataValue && tmpF64 == dfNoDataValue)
					continue;
				z_min = MIN(tmpF64, z_min);
				z_max = MAX(tmpF64, z_max);
			}
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "gdalread: Unsupported data type\n");
			break;
	}
	adfMinMax[0] = z_min;
	adfMinMax[1] = z_max;
}

int get_attrib_from_string(struct GMT_GDALREAD_OUT_CTRL *Ctrl, GDALRasterBandH hBand, int nBand, double  *dfNoDataValue) {
	/* Since several methods to get band's attributes for HDF5 Datasets are not yet implemented in GDAL2.1.0
	   namely GDALGetRasterScale() and friends, the temporary work-around is to fish them from the Metadata
	   strings that we can access via GDALGetMetadata().
	*/
	char *pch, *pch2, **papszMetadataBand = NULL;
	int i, nCounterBand;

	papszMetadataBand = GDALGetMetadata(hBand, NULL);
	nCounterBand = CSLCount(papszMetadataBand);

	for (i = 0; i < nCounterBand; i++) {
		if ((pch = strstr(papszMetadataBand[i], "add_offset")) != NULL) {
			/* Fish the value from a string of the type "geophysical_data_sst_add_offset=0" */
			if ((pch2 = strchr(pch, '=')) != NULL)
				Ctrl->band_field_names[nBand].ScaleOffset[1] = atof(&pch2[1]);
		}
		else if ((pch = strstr(papszMetadataBand[i], "scale_factor")) != NULL) {
			if ((pch2 = strchr(pch, '=')) != NULL)
				Ctrl->band_field_names[nBand].ScaleOffset[0] = atof(&pch2[1]);
		}
		else if ((pch = strstr(papszMetadataBand[i], "_FillValue")) != NULL) {
			if ((pch2 = strchr(pch, '=')) != NULL) {
				*dfNoDataValue = atof(&pch2[1]);
				Ctrl->band_field_names[nBand].nodata = *dfNoDataValue;
			}
		}
	}

	return (GMT_NOERROR);
}

GMT_LOCAL int populate_metadata (struct GMT_CTRL *GMT, struct GMT_GDALREAD_OUT_CTRL *Ctrl, char *gdal_filename, int got_R,
                                 int nXSize, int nYSize, double dfULX, double dfULY, double dfLRX, double dfLRY,
								 double z_min, double z_max, int first_layer) {
/* =============================================================================================== */
/*
 * This routine queries the GDAL raster file for some metadata
 *
 * Fields:
 *    ProjRefPROJ4:  a Proj4 type string describing the projection
 *    GeoTransform:
 *        a 6-tuple.  Entries are as follows.
 *            [0] --> top left x
 *            [1] --> w-e pixel resolution
 *            [2] --> rotation, 0 if image is "north up"
 *            [3] --> top left y
 *            [4] --> rotation, 0 if image is "north up"
 *            [5] --> n-s pixel resolution
 *
 *    DriverShortName:  describes the driver used to query *this* raster file
 *    DriverLongName:   describes the driver used to query *this* raster file
 *    RasterXSize, RasterYSize:
 *        These are the primary dimensions of the raster.  See "Overview", though.
 *    RasterCount:
 *        Number of raster bands present in the file.
 *
 *    ColorMap:
 *        A Mx4 int array with the colormap of first band, or NULL if it does not exists
 *
 *    color_interp:
 *
 *    Corners:
 *        Also a structure array with the Fields:
 *            LL, UL, UR, LR. Each of this contains the (x,y) coords (or pixel/line coords
 *            if image is not referenced) of the LL - LowerLeft - point, and so on.
 *
 *    GEOGCorners:
 *        A structure array with the same field as 'Corners' but in geographical coordinates
 *        (if that is possible, or otherwise the LL values are set to NaN).
 *
 *    hdr:
 *        contains a row vector with (xmin, xmax, ymin, ymax, zmin, zmax, pixel_reg, xinc, yinc)
 *        for this data set. Pixel_reg is 1 for pixel registration and 0 for grid node
 *        registration, but it is actually set the -F option [default is 0]
 *
 *    nodata:
 *        A special marker value used to mark nodes that contain not valid data. Its value is
 *        fetch from that of the first Band. In case that band has not defined 'nodata' than the
 *        GMT->session.d_NaN value is used. Besides this, the Ctrl->band_field_names[nBand].nodata
 *        also stores the nodata for each band exactly as returned by GDALGetRasterNoDataValue().
 *        That is, without checking for the contents of 'status' that is used to indicate if a
 *        nodata values is actually associated with that layer.
 *
 * */

	GDALDriverH hDriver;		/* This is the driver chosen by the GDAL library to query the dataset. */
	GDALDatasetH hDataset;		/* pointer structure used to query the gdal file. */
	GDALRasterBandH hBand;
	GDALColorTableH	hTable;
	GDALColorEntry	sEntry;
	OGRCoordinateTransformationH hTransform = NULL;

	const char  *pszProjection = NULL;
	int     i, j;
	int     status, bSuccess;	/* success or failure */
	int     nBand, raster_count;
	int     bGotMin, bGotMax;	/* To know if driver transmitted Min/Max */
	bool    got_noDataValue = false, pixel_reg = false, compute_minmax = false;
	double  adfGeoTransform[6];	/* bounds on the dataset */
	double  tmpdble;		/* temporary value */
	double  xy_c[2], xy_geo[4][2];	/* Corner coordinates in the local coords system and geogs (if it exists) */
	double  adfMinMax[2];	/* Dataset Min Max */
	double  dfNoDataValue = 0;

	/* ------------------------------------------------------------------------- */
	/* Open the file (if we can). */
	/* ------------------------------------------------------------------------- */
	hDataset = gdal_open (GMT, gdal_filename);
	if (hDataset == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to open %s.\n", gdal_filename);
		gmtlib_GDALDestroyDriverManager(GMT->parent);
		return (-1);
	}

	gmt_M_tic (GMT);

	/* ------------------------------------------------------------------------- */
	/* Record the ProjectionRef */
	/* ------------------------------------------------------------------------- */
	if (GDALGetProjectionRef(hDataset) != NULL) {
		OGRSpatialReferenceH  hSRS;
		char *pszProjection;
		char *pszResult = NULL;
		pszProjection = (char *)GDALGetProjectionRef(hDataset);

		hSRS = OSRNewSpatialReference(NULL);
		/* First in PROJ4 format */
		if (OSRImportFromWkt(hSRS, &pszProjection) == CE_None) {
			OSRExportToProj4(hSRS, &pszResult);
			Ctrl->ProjRefPROJ4 = strdup(pszResult);
			CPLFree(pszResult);
		}
		else
			Ctrl->ProjRefPROJ4 = NULL;

		/* Now in WKT format */
		if (OSRImportFromWkt(hSRS, &pszProjection) == CE_None) {
			char	*pszPrettyWkt = NULL;
			OSRExportToPrettyWkt(hSRS, &pszPrettyWkt, false);
			Ctrl->ProjRefWKT = strdup(pszPrettyWkt);
			CPLFree(pszPrettyWkt);
		}
		else
			Ctrl->ProjRefWKT = NULL;

		OSRDestroySpatialReference(hSRS);
	}

	/* ------------------------------------------------------------------------- */
	/* Record the geotransform. */
	/* ------------------------------------------------------------------------- */
	status = record_geotransform (gdal_filename, hDataset, adfGeoTransform);
	if (!strcmp(GDALGetDriverShortName(GDALGetDatasetDriver(hDataset)),"netCDF") && GDAL_VERSION_NUM <= 1450) {
		adfGeoTransform[3] *= -1;
		adfGeoTransform[5] *= -1;
	}
	if (status == 0) {
		Ctrl->GeoTransform[0] = adfGeoTransform[0];
		Ctrl->GeoTransform[1] = adfGeoTransform[1];
		Ctrl->GeoTransform[2] = adfGeoTransform[2];
		Ctrl->GeoTransform[3] = adfGeoTransform[3];
		Ctrl->GeoTransform[4] = adfGeoTransform[4];
		Ctrl->GeoTransform[5] = adfGeoTransform[5];
		if (got_R) {			/* Need to change those */
			Ctrl->GeoTransform[0] = dfULX;
			Ctrl->GeoTransform[3] = dfULY;
		}
	}
	else if (adfGeoTransform[1] != 1 && adfGeoTransform[5] != 1) {	/* Patch a possible GDAL bug. Raised after issue #1030 */
		adfGeoTransform[1] = adfGeoTransform[5] = 1;
		GMT_Report(GMT->parent, GMT_MSG_WARNING, "GDAL seamed to have returned garbage in adfGeoTransform. Arbitrarily setting inc = 1.\n");
	}

	/* ------------------------------------------------------------------------- */
	/* Get driver information */
	/* ------------------------------------------------------------------------- */
	hDriver = GDALGetDatasetDriver (hDataset);

	Ctrl->DriverShortName = GDALGetDriverShortName (hDriver);
	Ctrl->DriverLongName  = GDALGetDriverLongName (hDriver);

	if (!got_R) {		/* That is, if we are using the entire dataset */
		Ctrl->RasterXsize = GDALGetRasterXSize (hDataset);
		Ctrl->RasterYsize = GDALGetRasterYSize (hDataset);
	}
	else if (got_R && nXSize == 0 && nYSize == 0) {		/* metadata_only */
		int	anSrcWin[4];
		anSrcWin[0] = anSrcWin[1] = anSrcWin[2] = anSrcWin[3] = 0;
		if (adfGeoTransform[2] != 0.0 || adfGeoTransform[4] != 0.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "The -projwin option was used, but the geotransform is rotated."
			                                         " This configuration is not supported.\n");
			GDALClose(hDataset);
			gmtlib_GDALDestroyDriverManager(GMT->parent);
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Quitting with error\n");
			return(-1);
		}

		anSrcWin[0] = irint ((dfULX - adfGeoTransform[0]) / adfGeoTransform[1]);
		anSrcWin[1] = irint ((dfULY - adfGeoTransform[3]) / adfGeoTransform[5]);
		anSrcWin[2] = irint ((dfLRX - dfULX) / adfGeoTransform[1]);
		anSrcWin[3] = irint ((dfLRY - dfULY) / adfGeoTransform[5]);

		if (anSrcWin[0] < 0 || anSrcWin[1] < 0
		    || anSrcWin[0] + anSrcWin[2] > GDALGetRasterXSize(hDataset)
		    || anSrcWin[1] + anSrcWin[3] > GDALGetRasterYSize(hDataset)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Computed -srcwin falls outside raster size of %dx%d.\n",
			            GDALGetRasterXSize(hDataset), GDALGetRasterYSize(hDataset));
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Quitting with error\n");
			return(-1);
		}
		Ctrl->RasterXsize = nXSize = anSrcWin[2];
		Ctrl->RasterYsize = nYSize = anSrcWin[3];
	}
	else {
		Ctrl->RasterXsize = nXSize;
		Ctrl->RasterYsize = nYSize;
	}

	Ctrl->RasterCount = raster_count = GDALGetRasterCount(hDataset);

	/* ------------------------------------------------------------------------- */
	/* Get some metadata for each band. */
	/* ------------------------------------------------------------------------- */

	Ctrl->band_field_names = gmt_M_memory(GMT, NULL, raster_count, struct GDAL_BAND_FNAMES);

	/* ==================================================================== */
	/*      Loop over bands.                                                */
	/* ==================================================================== */
	for (nBand = 0; nBand < raster_count; nBand++) {

		hBand = GDALGetRasterBand(hDataset, nBand+1);

		if (!got_R) {		/* Not sure about what will really happen in this case */
			Ctrl->band_field_names[nBand].XSize = GDALGetRasterBandXSize(hBand);
			Ctrl->band_field_names[nBand].YSize = GDALGetRasterBandYSize(hBand);
		}
		else {
			Ctrl->band_field_names[nBand].XSize = nXSize;
			Ctrl->band_field_names[nBand].YSize = nYSize;
		}

		Ctrl->band_field_names[nBand].DataType = strdup(GDALGetDataTypeName(GDALGetRasterDataType (hBand)));
		Ctrl->band_field_names[nBand].nodata = (double)GDALGetRasterNoDataValue(hBand, &status);

		/* Get band's Min/Max. If the band has no record of it we won't compute it */
		adfMinMax[0] = GDALGetRasterMinimum(hBand, &bGotMin);
		adfMinMax[1] = GDALGetRasterMaximum(hBand, &bGotMax);
		if (bGotMin && bGotMax) {
			Ctrl->band_field_names[nBand].MinMax[0] = adfMinMax[0];
			Ctrl->band_field_names[nBand].MinMax[1] = adfMinMax[1];
		}
		else {		/* No Min/Max, we won't computer it either */
			Ctrl->band_field_names[nBand].MinMax[0] = GMT->session.d_NaN;
			Ctrl->band_field_names[nBand].MinMax[1] = GMT->session.d_NaN;
		}

		/* Get band's Scale/Offset. If the band does not have them use the neutral 1/0 values */
		if (GDALGetRasterScale(hBand, &bSuccess) != 1 || GDALGetRasterOffset(hBand, &bSuccess) != 0) {
			Ctrl->band_field_names[nBand].ScaleOffset[0] = GDALGetRasterScale (hBand, &bSuccess);
			Ctrl->band_field_names[nBand].ScaleOffset[1] = GDALGetRasterOffset(hBand, &bSuccess);
		}
		else {
			Ctrl->band_field_names[nBand].ScaleOffset[0] = 1.0;
			Ctrl->band_field_names[nBand].ScaleOffset[1] = 0.0;
		}

		/* Very soft guess if have grid or pixel registration. This will be confirmed latter on main */
		if (nBand == 0) pixel_reg = (GDALGetRasterDataType(hBand) == GDT_Byte);

		/* Here the Mirone code has a chunk to read overviews info, but since we have not
		   yet any use for it I won't implement that just yet.

		   Continuing the lazy path, and according to the same excuse that we don't have yet
		   any use for it, we won't check if individual bands have different colormaps.
		   Note, however, that first band colormap is captured anyway (if exists, off course) */

		if (bSuccess == 0 && strstr(Ctrl->DriverShortName, "HDF5") != NULL) {	/* several methods for HDF5 driver are not implemented */
			GMT_Report(GMT->parent, GMT_MSG_INFORMATION, "An HDF5 file. Trying to get scale_offset from string metadata.\n");
			dfNoDataValue = GMT->session.d_NaN;
			get_attrib_from_string(Ctrl, hBand, nBand, &dfNoDataValue);			/* Go get them from the metadata in strings. */
			if (!isnan(dfNoDataValue)) got_noDataValue = true;
		}
	}

	/* ------------------------------------------------------------------------- */
	/* Get the Color Interpretation Name */
	/* ------------------------------------------------------------------------- */
	hBand = GDALGetRasterBand(hDataset, first_layer);
	if (raster_count > 0)
		Ctrl->color_interp = GDALGetColorInterpretationName(GDALGetRasterColorInterpretation(hBand));
	else
		Ctrl->color_interp = NULL;

	/* ------------------------------------------------------------------------- */
	/* Get the first band NoData Value */
	/* ------------------------------------------------------------------------- */
	if (!got_noDataValue) {		/* May have been found if get_attrib_from_string() was acalled above */
		dfNoDataValue = GDALGetRasterNoDataValue(hBand, &status);
		if (status)
			Ctrl->nodata = dfNoDataValue;
		else
			Ctrl->nodata = GMT->session.d_NaN;
	}

	/* ------------------------------------------------------------------------- */
	/* Get the Color Map of first band (if any) */
	/* ------------------------------------------------------------------------- */
	if (GDALGetRasterColorInterpretation(hBand) == GCI_PaletteIndex
		&& (hTable = GDALGetRasterColorTable(hBand)) != NULL) {

		Ctrl->nIndexedColors = GDALGetColorEntryCount(hTable);
		Ctrl->ColorMap = gmt_M_memory(GMT, NULL, Ctrl->nIndexedColors*4+1, int);
		for (i = 0; i < Ctrl->nIndexedColors; i++) {
			GDALGetColorEntryAsRGB(hTable, i, &sEntry);
			gmt_M_set_rgba (Ctrl->ColorMap, i, 0, Ctrl->nIndexedColors, sEntry.c1);
			gmt_M_set_rgba (Ctrl->ColorMap, i, 1, Ctrl->nIndexedColors, sEntry.c2);
			gmt_M_set_rgba (Ctrl->ColorMap, i, 2, Ctrl->nIndexedColors, sEntry.c3);
			gmt_M_set_rgba (Ctrl->ColorMap, i, 3, Ctrl->nIndexedColors, sEntry.c4);
		}
		/* Mark the end of the ColorMap */
		Ctrl->ColorMap[Ctrl->nIndexedColors*4] = -1;
		compute_minmax = true;
	}
	else {
		/* Before giving up, check if band is a 1-bit type */
		const char *pszNBits = GDALGetMetadataItem(hBand, "NBITS", "IMAGE_STRUCTURE");
		if (pszNBits && !strcmp(pszNBits, "1")) {
			/* Create a two-color index table for black and white only */
			Ctrl->nIndexedColors = 2;
			Ctrl->ColorMap = gmt_M_memory(GMT, NULL, Ctrl->nIndexedColors*4+1, int);
			for (j = 0; j < 3; j++) {	/* For R, G, B */
				gmt_M_set_rgba (Ctrl->ColorMap, 0, j, Ctrl->nIndexedColors, 0);
				gmt_M_set_rgba (Ctrl->ColorMap, 1, j, Ctrl->nIndexedColors, 255);
			}
			/* Then set A */
			gmt_M_set_rgba (Ctrl->ColorMap, 0, 3, Ctrl->nIndexedColors, 255);	/* JL, THIS SHOULD BE 255, NO??? IT WAS 1*/
			gmt_M_set_rgba (Ctrl->ColorMap, 1, 3, Ctrl->nIndexedColors, 255);
			/* Mark the end of the colormap */
			Ctrl->ColorMap[Ctrl->nIndexedColors*4] = -1;
			compute_minmax = true;
		}
		else {	/* Apparently no ColorMap, set it */
			Ctrl->ColorMap = NULL;
			Ctrl->nIndexedColors  = 0;
		}
	}

	/* ------------------------------------------------------------------------- */
	/* Record corners. */
	/* ------------------------------------------------------------------------- */

	/* -------------------------------------------------------------------- */
	/*      Setup projected to lat/long transform if appropriate.           */
	/* -------------------------------------------------------------------- */
	if (!got_R) {
		if (GDALGetGeoTransform(hDataset, adfGeoTransform) == CE_None)
			pszProjection = GDALGetProjectionRef(hDataset);

		if (pszProjection != NULL && strlen(pszProjection) > 0) {
			OGRSpatialReferenceH hProj, hLatLong = NULL;
			hProj = OSRNewSpatialReference( pszProjection );
			if (hProj != NULL)
				hLatLong = OSRCloneGeogCS(hProj);

			if (hLatLong != NULL) {
				CPLPushErrorHandler( CPLQuietErrorHandler );
				hTransform = OCTNewCoordinateTransformation( hProj, hLatLong );
				CPLPopErrorHandler();
				OSRDestroySpatialReference(hLatLong);
			}

			if (hProj != NULL)
				OSRDestroySpatialReference(hProj);
		}
	}

	if (!got_R)					/* Lower Left */
		ReportCorner (GMT, hDataset, hTransform, 0.0, GDALGetRasterYSize(hDataset), xy_c, xy_geo[0]);
	else
		xy_c[0] = dfULX, xy_c[1] = dfLRY;
	Ctrl->Corners.LL[0] = Ctrl->hdr[0] = xy_c[0];	/* xmin, ymin */
	Ctrl->Corners.LL[1] = Ctrl->hdr[2] = xy_c[1];

	if (!got_R)					/* Upper Left */
		ReportCorner (GMT, hDataset, hTransform, 0.0, 0.0, xy_c, xy_geo[1]);
	else
		xy_c[0] = dfULX, xy_c[1] = dfULY;
	Ctrl->Corners.UL[0] = xy_c[0];
	Ctrl->Corners.UL[1] = xy_c[1];

	if (!got_R)					/* Upper Right */
		ReportCorner (GMT, hDataset, hTransform, GDALGetRasterXSize(hDataset), 0.0, xy_c, xy_geo[2]);
	else
		xy_c[0] = dfLRX, xy_c[1] = dfULY;
	Ctrl->Corners.UR[0] = Ctrl->hdr[1] = xy_c[0];	/* xmax, ymax */
	Ctrl->Corners.UR[1] = Ctrl->hdr[3] = xy_c[1];

	if (!got_R)					/* Lower Right */
		ReportCorner (GMT, hDataset, hTransform, GDALGetRasterXSize(hDataset), GDALGetRasterYSize(hDataset), xy_c, xy_geo[3]);
	else
		xy_c[0] = dfLRX, xy_c[1] = dfLRY;
	Ctrl->Corners.LR[0] = xy_c[0];
	Ctrl->Corners.LR[1] = xy_c[1];

	if (hTransform != NULL)
		OCTDestroyCoordinateTransformation(hTransform);

	/* Must check that geog grids have not y_max > 90.0 We'll be eps tolerant ... but how do I know that it's geog?*/

	/* --------------------------------------------------------------------------------------
	 * Record Geographical corners (if they exist)
	 * -------------------------------------------------------------------------------------- */
	if (!got_R) {
		if (!gmt_M_is_dnan(xy_geo[0][0])) {
			Ctrl->GEOGCorners.LL[0] = xy_geo[0][0]; Ctrl->GEOGCorners.LL[1] = xy_geo[0][1];
			Ctrl->GEOGCorners.UL[0] = xy_geo[1][0]; Ctrl->GEOGCorners.UL[1] = xy_geo[1][1];
			Ctrl->GEOGCorners.UR[0] = xy_geo[2][0]; Ctrl->GEOGCorners.UR[1] = xy_geo[2][1];
			Ctrl->GEOGCorners.LR[0] = xy_geo[3][0]; Ctrl->GEOGCorners.LR[1] = xy_geo[3][1];
		}
		else
			Ctrl->GEOGCorners.LL[0] = Ctrl->GEOGCorners.LL[1] = GMT->session.d_NaN;
	}
	else
		Ctrl->GEOGCorners.LL[0] = Ctrl->GEOGCorners.LL[1] = GMT->session.d_NaN;

	/* ------------------------------------------------------------------------- */
	/* Fill in the rest of the GMT header values (If ...) */
	if (raster_count > 0) {
		int type = GDALGetRasterDataType (hBand);
		if (type == GDT_Float32 || type == GDT_Float64) compute_minmax = true;
		hBand = GDALGetRasterBand(hDataset, 1);	/* Ensure hBand points to the first layer */
		if (z_min == DBL_MAX && (compute_minmax || type != GDT_Byte)) {	/* We don't know yet the dataset Min/Max and it seems to be a grid */
			GDALComputeRasterMinMax(hBand, compute_minmax, adfMinMax);	/* Unfortunately, cannot always trust metadata min/max */
			Ctrl->hdr[4] = adfMinMax[0];
			Ctrl->hdr[5] = adfMinMax[1];
		}
		else {
			Ctrl->hdr[4] = z_min;
			Ctrl->hdr[5] = z_max;
		}

		/* This test should give us info if a GeoTiff GRID (not image) file is grid registered but I lack one
		   example file to test it. The example mentioned in issue http://gmtrac.soest.hawaii.edu/issues/254
		   (where all this (re)started) not only is bugged as does not carry the AREA_OR_POINT metadata.
		   So we'll check for the "Area" keyword and if found we will respect it and set grid to pix reg */
		if (gmtlib_file_is_jpeg2000_tile (GMT->parent, gdal_filename) != GMT_NOTSET && (Ctrl->RasterXsize % 2) == 0 && (Ctrl->RasterYsize % 2) == 0)
			/* PW: Reading GMT server special JP2 tiles: even size implies pixel registration */
			pixel_reg = true;
		else if (!pixel_reg && GDALGetMetadataItem(hDataset, "AREA_OR_POINT", NULL) &&
			!strcmp(GDALGetMetadataItem(hDataset, "AREA_OR_POINT", NULL), "Area"))
			pixel_reg = true;
		else if (!pixel_reg && GDALGetMetadataItem(hDataset, "GDALMD_AREA_OR_POINT", NULL) &&
			!strcmp(GDALGetMetadataItem(hDataset, "GDALMD_AREA_OR_POINT", NULL), "Area"))
			pixel_reg = true;

		Ctrl->hdr[6] = pixel_reg;
		Ctrl->hdr[7] = adfGeoTransform[1];
		Ctrl->hdr[8] = fabs(adfGeoTransform[5]);

		if (Ctrl->hdr[2] > Ctrl->hdr[3]) {	/* Sometimes GDAL does it: y_min > y_max. If so, revert it */
			tmpdble = Ctrl->hdr[2];
			Ctrl->hdr[2] = Ctrl->hdr[3];
			Ctrl->hdr[3] = tmpdble;
		}

		if (got_R) {
			Ctrl->hdr[0] = dfULX;	Ctrl->hdr[1] = dfLRX;
			Ctrl->hdr[2] = dfLRY;	Ctrl->hdr[3] = dfULY;
		}
	}

	GDALClose (hDataset);

	gmt_M_toc (GMT, "In populate_metadata");

	return (GMT_NOERROR);
}

/*----------------------------------------------------------|
 * Public functions that are part of the GMT Devel library  |
 *----------------------------------------------------------|
 */

int gmtlib_is_gdal_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	GDALDatasetH hDataset;
	GDALDriverH	hDriver;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	GDALAllRegister();
	if (strstr(HH->name, ".jp2") || strstr(HH->name, ".JP2"))
		if ((hDriver = GDALGetDriverByName("JP2OpenJPEG")) != NULL && (hDriver = GDALGetDriverByName("JP2ECW")) != NULL)
			GDALDeregisterDriver(hDriver);		/* Deregister the JP2ECW driver. That is, prefer the OpenJPEG one */
	hDataset = gdal_open (GMT, HH->name);

	if (hDataset == NULL)
		return (GMT_GRDIO_BAD_VAL);
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "File %s reads with GDAL driver %s\n",
	            HH->name, GDALGetDriverShortName(GDALGetDatasetDriver(hDataset)));
	GDALClose (hDataset);
	gmtlib_GDALDestroyDriverManager(GMT->parent);
	header->type = GMT_GRID_IS_GD;

	return GMT_NOERROR;
}

int gmt_gdalread (struct GMT_CTRL *GMT, char *gdal_filename, struct GMT_GDALREAD_IN_CTRL *prhs, struct GMT_GDALREAD_OUT_CTRL *Ctrl) {
	/* PW 9/24/2021: I have updated the algorithm to be able to handle periodic geographic images (e.g., @earth_day) where
	 * it is possible the users -Rw/e/s/n will straddle the periodic boundary (180).  In that case we must read the east
	 * and west image chunks separately and place then in the same output buffer with careful attention to offsets.
	 * For this reason, variables like xOrigin, nXSize, nX, nBufXSize, pad_w, and pad_e are now arrays of length 2 so that
	 * for the straddling cases each chunk will have its own correct dimensions and pad settings.  Most of the time n_pieces
	 * will be 1 and we only use the first entry in those arrays, but for straddling cases we will need both */
	const char	*format = NULL;
	int	nRGBA = 1;	/* 1 for BSQ; 3 for RGB and 4 for RGBA (If needed, value is updated below) */
	int	complex_mode = 0;	/* 0 real only. 1|2 if complex array is to hold real (1) and imaginary (2) parts */
	int	nPixelSize, nBands, i, nReqBands = 0;
	int	anSrcWin[4], xOrigin[2] = {0,0}, yOrigin = 0;
	int	jump = 0, nXSize[2] = {0,0}, nYSize = 0, nX[2] = {0,0}, nY;
	int nBufXSize[2] = {0,0}, nBufYSize, buffy, startRow = 0, endRow;
	int nRowsPerBlock, nBlocks, nYOff, row_i, row_e;
	int k, pad_w[2] = {0,0}, pad_e[2] = {0,0}, pad_s = 0, pad_n = 0;    /* Different pads for when sub-regioning near the edges */
	int	incStep = 1;	/* 1 for real only arrays and 2 for complex arrays (index step increment) */
	int error = 0, gdal_code = 0, first_layer;
	int piece, n_pieces = 1;	/* Normally 1, but will be 2 if the desired image subset is split across a periodic boundary */
	int XDim, YDim;	/* Dimension of image */
	int status;
	bool   do_BIP;		/* For images if BIP == true data is stored Pixel interleaved, otherwise Band interleaved */
	bool   metadata_only;
	bool   pixel_reg = false;	/* GDAL decides everything is pixel reg, we make our decisions based on data type */
	bool   fliplr, got_R = false, got_r = false;
	bool   topdown = false, rowmajor = true;               /* arrays from GDAL have this order */
	bool   just_copy = false, copy_flipud = false;
	int	   *whichBands = NULL;
	int64_t *rowVec = NULL, *colVec = NULL;
	int64_t  off, i_x_nXYSize, startColPos = 0, indent = 0, col_indent = 0, nXSize_withPad = 0, nYSize_withPad;
	int64_t  n_alloc, n, m, nn, mm, ij, layer_size, layer_offset = 0;
	unsigned char *tmp = NULL;
	double  adfMinMax[2];
	double	adfGeoTransform[6];
	double  dfULX = 0.0, dfULY = 0.0, dfLRX = 0.0, dfLRY = 0.0;
	double  z_min = DBL_MAX, z_max = -DBL_MAX;
	GDALDatasetH	hDataset;
	GDALRasterBandH	hBand;
	GDALDriverH	hDriver;

	Ctrl->band_field_names = NULL;		/* So we can test before trying to read its fields */
	Ctrl->RasterCount = 0;	/* To avoid attempting to use Ctrl->band_field_names[i] */
	gmt_M_memset (anSrcWin, 4, int);

	if (prhs->B.active) {		/* We have a selected bands request */
		whichBands = gmt_M_memory (GMT, NULL, 128, int);		/* 128 is huge enough */
		nReqBands = gdal_decode_columns (GMT, prhs->B.bands, whichBands);
	}
	else if (prhs->f_ptr.active) {
		/* Here we are going to read to a grid so if no band info was provided, default to read only the
		   first band. This avoids, for example, allocate and read all 3 bands in a RGB image and send
		   back a full 3 band array to gmt_gdal_read_grd that will only keep the first band. */
		nReqBands = 1;
		whichBands = gmt_M_memory (GMT, NULL, 1, int);
		whichBands[0] = 1;
		Ctrl->Float.active = true;		/* Signals that output will be in the float array, no matter input type */
	}
	first_layer = (nReqBands) ? whichBands[0] : 1;		/* The one used to get data type and header info */

	if (nReqBands && GMT->current.setting.verbose >= GMT_MSG_INFORMATION) {
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "gmt_gdalread: Read band(s):");
		for (k = 0; k < nReqBands; k++)
			GMT_Message (GMT->parent, GMT_TIME_NONE, "\t%d", whichBands[k]);
		GMT_Message (GMT->parent, GMT_TIME_NONE, "\n");
	}

	do_BIP = prhs->I.active;
	if (nReqBands == 1) do_BIP = false;	/* This must overrule any -I option settings */
	fliplr = prhs->L.active;
	metadata_only = prhs->M.active;

	if (!metadata_only && GMT->current.gdal_read_in.O.mem_layout[0]) {    /* first char T(op)|B(ot), second R(ow)|C(ol), third B(and)|P(ixel)|L(ine) */
		if (GMT->current.gdal_read_in.O.mem_layout[0] == 'T')
			topdown = true;
		if (GMT->current.gdal_read_in.O.mem_layout[1] == 'C')
			rowmajor  = false;
		if (GMT->current.gdal_read_in.O.mem_layout[2] == 'B' || GMT->current.gdal_read_in.O.mem_layout[2] == 'L')
			do_BIP = false;
		if (topdown && rowmajor) just_copy = true;		/* Means we will send out the data as it came from gdal */
		if (!topdown && rowmajor) copy_flipud = true;	/* Means we will send out the data as it came from gdal */
		/* Send back the info that I->header->mem_layout must be updated */
		strncpy(prhs->O.mem_layout, GMT->current.gdal_read_in.O.mem_layout, 4);
	}
	if (!metadata_only && !prhs->O.mem_layout[0]) {	/* If caller did not ask the layout, assign it to the true one used here. */
		prhs->O.mem_layout[0] = topdown  ? 'T' : 'B';
		prhs->O.mem_layout[1] = rowmajor ? 'R' : 'C';
		prhs->O.mem_layout[2] = do_BIP   ? 'P' : 'B';
		prhs->O.mem_layout[3] = 'a';		/* If later we find image has 4 layers, this will become 'A' */
	}

	if (prhs->p.active) {	/* Initialize the pad for piece 0 only (other set as needed later) */
		pad_w[0] = prhs->p.pad[XLO];
		pad_e[0] = prhs->p.pad[XHI];
		pad_s    = prhs->p.pad[YLO];
		pad_n    = prhs->p.pad[YHI];
	}

	if (prhs->R.active) {	/* Decode -Rw/e/s/n string */
		double wesn[4];
		got_R = true;
		error += (GMT_Get_Values (GMT->parent, prhs->R.region, wesn, 4) < 4);

		if (!error) {
			double dx = 0, dy = 0;
			if (!prhs->registration.val) {	/* Subregion coords are grid-reg. Need to convert to pix-reg */
				dx = prhs->registration.x_inc / 2;
				dy = prhs->registration.y_inc / 2;
			}
			dfULX = wesn[XLO] - dx;
			dfLRX = wesn[XHI] + dx;
			dfLRY = wesn[YLO] - dy;
			dfULY = wesn[YHI] + dy;
		}
	}
	else if (prhs->r.active) { 		/* Region is given in pixels instead */
		double wesn[4];
		got_r = true;
		error += (GMT_Get_Values (GMT->parent, prhs->R.region, wesn, 4) < 4);
		if (!error) {
			dfULX = wesn[XLO];	dfLRX = wesn[XHI];
			dfLRY = wesn[YLO];	dfULY = wesn[YHI];
		}
	}

	if (error) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_gdalread: Failed to decode the specified Sub-region\n");
		gmt_M_free (GMT, whichBands);
		return (-1);
	}

	if (prhs->P.active)
		jump = atoi(prhs->P.jump);

	if (prhs->Z.active) {	/* Get either the real or imaginary pixels only */
		complex_mode = prhs->Z.complex_mode;
		if (complex_mode) incStep = 2;
	}

	/* Open gdal - */

	GDALAllRegister();

	if (prhs->W.active) {
		OGRSpatialReferenceH  hSRS;
		hSRS = OSRNewSpatialReference(NULL);

		if (Ctrl->ProjRefPROJ4 && OSRImportFromProj4(hSRS, Ctrl->ProjRefPROJ4) == CE_None) {	/* My be NULL if +unavailable */
			char	*pszPrettyWkt = NULL;
			OSRExportToPrettyWkt(hSRS, &pszPrettyWkt, false);
			Ctrl->ProjRefWKT = strdup(pszPrettyWkt);
			CPLFree(pszPrettyWkt);
		}
		else {
			Ctrl->ProjRefWKT = NULL;
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "gmt_gdalread: Failed to convert the proj4 string\n%s\nto WKT\nThis happens for example when no conversion between PROJ4 and the WKT is done by GDAL.",
					Ctrl->ProjRefPROJ4);
		}

		OSRDestroySpatialReference(hSRS);
		gmt_M_free (GMT, whichBands);
		return (GMT_NOERROR);
	}

	if (getenv("GDAL_HTTP_UNSAFESSL"))	/* The fact that it exist is not enough. It might be from a GMT process only. */
		CPLSetConfigOption ("GDAL_HTTP_UNSAFESSL", getenv("GDAL_HTTP_UNSAFESSL"));
	if (getenv("CURL_CA_BUNDLE"))		/* And the same for this one. */
		CPLSetConfigOption ("CURL_CA_BUNDLE", getenv("CURL_CA_BUNDLE"));

	if (strstr(gdal_filename, ".jp2") || strstr(gdal_filename, ".JP2"))
		if ((hDriver = GDALGetDriverByName("JP2OpenJPEG")) != NULL && (hDriver = GDALGetDriverByName("JP2ECW")) != NULL)
			GDALDeregisterDriver(hDriver);		/* Deregister the JP2ECW driver. That is, prefer the OpenJPEG one */

	if (metadata_only) {	/* Just get the header info and return with it */
		if (populate_metadata (GMT, Ctrl, gdal_filename, got_R, nXSize[0], nYSize, dfULX, dfULY, dfLRX, dfLRY, z_min, z_max, first_layer))
			return(-1);

		/* Return registration based on data type of first band. Byte is pixel reg otherwise set grid registration */
		if (!Ctrl->hdr[6]) {		/* Grid registration */
			Ctrl->hdr[0] += Ctrl->hdr[7] / 2;	Ctrl->hdr[1] -= Ctrl->hdr[7] / 2;
			Ctrl->hdr[2] += Ctrl->hdr[8] / 2;	Ctrl->hdr[3] -= Ctrl->hdr[8] / 2;
		}
		gmt_M_free (GMT, whichBands);
		return (GMT_NOERROR);
	}

	/* Read the image */

	hDataset = gdal_open (GMT, gdal_filename);

	if (hDataset == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_gdalread: gdal_open failed %s\n", CPLGetLastErrorMsg());
		gmt_M_free (GMT, whichBands);
		return (-1);
	}

	/* Some formats (typically DEMs) have their origin at Bottom Left corner.
	   For those we have to flip the data matrix to be in accord with matlab (Top Left) */

	hDriver = GDALGetDatasetDriver(hDataset);
	format = GDALGetDriverShortName(hDriver);

	if (!strcmp(format,"ESAT"))	/* ENVISAT data are flipped left-right */
		fliplr = true;

	/* Get the image size */
	XDim = GDALGetRasterXSize(hDataset);
	YDim = GDALGetRasterYSize(hDataset);

	/* See if there are geographic coordinates in the metadata */
	status = GDALGetGeoTransform(hDataset, adfGeoTransform);
	if (status == CE_Failure && got_R) {
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "gmt_gdalread: Image %s does not have geographic coordinates so -R is ignored for subregion\n", gdal_filename);
		got_R = false;
	}

	if (got_R || got_r) {
		/* -------------------------------------------------------------------- */
		/*      Compute the source window from the projected source window      */
		/*      if the projected coordinates were provided.  Note that the      */
		/*      projected coordinates are in ulx, uly, lrx, lry format,         */
		/*      while the anSrcWin is xoff, yoff, xsize, ysize with the         */
		/*      xoff,yoff being the ulx, uly in pixel/line.                     */
		/* -------------------------------------------------------------------- */

		//GDALGetGeoTransform(hDataset, adfGeoTransform);

		if (adfGeoTransform[2] != 0.0 || adfGeoTransform[4] != 0.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR,
			            "gmt_gdalread: The -projwin option was used, but the geotransform is rotated. This configuration is not supported.\n");
			GDALClose(hDataset);
			gmtlib_GDALDestroyDriverManager(GMT->parent);
			gmt_M_free (GMT, whichBands);
			return (-1);
		}

		if (got_R) {	/* Region in map coordinates */
			anSrcWin[0] = irint (((dfULX - adfGeoTransform[0]) / adfGeoTransform[1]));
			anSrcWin[1] = irint (((dfULY - adfGeoTransform[3]) / adfGeoTransform[5]));
			anSrcWin[2] = irint (((dfLRX - dfULX) / adfGeoTransform[1]));
			anSrcWin[3] = irint (((dfLRY - dfULY) / adfGeoTransform[5]));
			if (GDAL_VERSION_NUM < 1700 && !strcmp(format,"netCDF")) {
				/* PATCH against the old GDAL bugs of reading netCDF files */
				anSrcWin[1] = GDALGetRasterYSize(hDataset) - (anSrcWin[1] + anSrcWin[3]) - 1;
			}
		}
		else {		/* Region in pixel/line */
			anSrcWin[0] = irint (dfULX);
			anSrcWin[1] = irint (dfLRY);
			anSrcWin[2] = irint (dfLRX - dfULX);
			anSrcWin[3] = irint (dfULY - dfLRY);
		}

		/* First check if we are outside valid y-range since that check is always Cartesian */
		if (anSrcWin[1] < 0 || (anSrcWin[1] + anSrcWin[3]) > YDim) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_gdalread: Computed -srcwin falls outside raster size of %dx%d in the y-direction.\n",
				XDim, YDim);
			gmtlib_GDALDestroyDriverManager(GMT->parent);
			gmt_M_free (GMT, whichBands);
			return (-1);
		}
		yOrigin = anSrcWin[1];	/* These can now be set */
		nYSize  = anSrcWin[3];

		xOrigin[0] = anSrcWin[0];	/* This will be correct for the first (and perhaps only) of the 1-2 areas unless truly outside */
		nXSize[0]  = anSrcWin[2];	/* If periodic then xOrigin+nXSize may exceed XDim and we will rotate it */
		if (prhs->R.periodic) {	/* Watch for straddling */
			if (xOrigin[0] < 0)	/* Ensure we straddle the east boundary for the first of two pieces */
				xOrigin[0] += XDim;
			if ((xOrigin[0] + nXSize[0]) > XDim) {	/* Must read the two areas straddling the periodic boundary separately */
				n_pieces = 2;	/* Number of sub-areas */
				xOrigin[1] = 0;	/* The second starts all the way to the west */
				nXSize[1] = xOrigin[0] + nXSize[0] - XDim;	/* What goes beyond XDim is the width of the west piece */
				nXSize[0] = XDim - xOrigin[0];	/* Truncate the east piece width accordingly */
				if (nXSize[1] > XDim) nXSize[1] = XDim - nXSize[0];	/* Safety valve since file only has XDim values along a row */
				pad_w[1] = 0;	pad_e[1] = pad_e[0];	pad_e[0] = 0;	/* Set the "seam" pads to zero since no longer at the edges */
			}
		}
		else {	/* Same (Cartesian) test as for y-extent */
			if (xOrigin[0] < 0 || (xOrigin[0] + anSrcWin[2]) > XDim) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_gdalread: Computed -srcwin falls outside raster size of %dx%d in the x-direction.\n",
					XDim, YDim);
				gmtlib_GDALDestroyDriverManager(GMT->parent);
				gmt_M_free (GMT, whichBands);
				return (-1);
			}
		}
	}
	else {			/* Use entire dataset so only need a single chunk */
		xOrigin[0] = yOrigin = 0;
		nXSize[0] = XDim;
		nYSize = YDim;
	}

	/* The following assumes that all bands have the same PixelSize, data type. Otherwise ... */
	hBand = GDALGetRasterBand(hDataset, first_layer);
	nPixelSize = GDALGetDataTypeSize(GDALGetRasterDataType(hBand)) / 8;	/* /8 because return value is in BITS */

	if (jump) {
		nBufXSize[0] = XDim / jump;
		nBufYSize = YDim / jump;
	}
	else {
		nBufXSize[0] = nXSize[0];
		nBufXSize[1] = nXSize[1];
		nBufYSize = nYSize;
	}

	nBands = GDALGetRasterCount(hDataset);

	if (nBands == 4) prhs->O.mem_layout[3] = 'A';

	if (nReqBands) nBands = MIN(nBands,nReqBands);	/* If a band selection was made */

	/* Determine allocation size for the subset (actual allocation depends on whether a pointer is passed in or not) */
	layer_size = ((size_t)(nBufXSize[0] + nBufXSize[1] + MAX(pad_w[0],pad_w[1])) + MAX(pad_e[0],pad_e[1])) * ((size_t)nBufYSize + pad_s + pad_n);
	n_alloc = ((size_t)nBands) * layer_size;

	switch (GDALGetRasterDataType(hBand)) {
		case GDT_Byte:
			if (prhs->c_ptr.active)	/* We have a pointer with already allocated memory ready to use */
				Ctrl->UInt8.data = prhs->c_ptr.grd;
			else
				Ctrl->UInt8.data = gmt_M_memory (GMT, NULL, n_alloc, uint8_t); /* aka unsigned char */

			if (do_BIP) {
				if (nBands == 4)	/* Assume fourth band holds the alpha channel */
					nRGBA = 4;
				else if (nBands == 3)
					nRGBA = 3;
				else                /* BIP request ignored since number of bands is not 3 or 4 */
					do_BIP = false;
			}
			break;
		case GDT_Int16:
			if (prhs->f_ptr.active)		/* Use the previously allocated float pointer */
				Ctrl->Float.data = prhs->f_ptr.grd;
			else if (prhs->c_ptr.active) 	/* Use the previously allocated pointer */
				Ctrl->Int16.data = (int16_t *)prhs->c_ptr.grd;
			else
				Ctrl->Int16.data = gmt_M_memory (GMT, NULL, n_alloc, int16_t);
			break;
		case GDT_UInt16:
			if (prhs->f_ptr.active) {	/* Use the previously allocated float pointer */
				Ctrl->Float.data = prhs->f_ptr.grd;
				Ctrl->Float.active = true;		/* In case it was not set yet */
			}
			else if (prhs->c_ptr.active) {	/* Use the previously allocated pointer */
				Ctrl->UInt16.data = (uint16_t *)prhs->c_ptr.grd;
				Ctrl->UInt16.active = true;		/* In case it was not set yet */
			}
			else
				Ctrl->UInt16.data = gmt_M_memory (GMT, NULL, n_alloc, uint16_t);
			break;
		case GDT_Int32:
			if (prhs->f_ptr.active)		/* Use the previously allocated float pointer */
				Ctrl->Float.data = prhs->f_ptr.grd;
			else if (prhs->c_ptr.active) 	/* Use the previously allocated pointer */
				Ctrl->Int32.data = (int32_t *)prhs->c_ptr.grd;
			else
				Ctrl->Int32.data = gmt_M_memory (GMT, NULL, n_alloc, int32_t);
			break;
		case GDT_UInt32:
			if (prhs->f_ptr.active)		/* Use the previously allocated float pointer */
				Ctrl->Float.data = prhs->f_ptr.grd;
			else if (prhs->c_ptr.active) 	/* Use the previously allocated pointer */
				Ctrl->UInt32.data = (uint32_t *)prhs->c_ptr.grd;
			else
				Ctrl->UInt32.data = gmt_M_memory (GMT, NULL, n_alloc, uint32_t);
			break;
		case GDT_Float32:
		case GDT_Float64:
			if (prhs->f_ptr.active)	/* We have a pointer with already allocated memory ready to use */
				Ctrl->Float.data = prhs->f_ptr.grd;
			else {
				if (!complex_mode)
					Ctrl->Float.data = gmt_M_memory (GMT, NULL, n_alloc, float);
				else
					Ctrl->Float.data = gmt_M_memory (GMT, NULL, 2 * n_alloc, float);
			}
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "gdalread: Unsupported data type\n");
			break;
	}

	/* ------ compute two vectors indices that will be used inside loops below --------- */
	/* In the "Preview" mode those guys below are different and what we need is the BufSize */
	if (jump)
		nX[0] = nBufXSize[0],	nX[1] = nBufXSize[1],	nY = nBufYSize;
	else
		nX[0] = nXSize[0],	nX[1] = nXSize[1],	nY = nYSize;

	/* --------------------------------------------------------------------------------- */
	nXSize_withPad = nXSize[0] + nXSize[1] + pad_w[0] + pad_e[0] + pad_w[1] + pad_e[1];	/* Only one of w and e will be nonzero if padding is on */

	gmt_M_tic (GMT);

	for (piece = 0; piece < n_pieces; piece++) {	/* Mostly doing this loop once, but for subregion straddling periodic boundary we get here twice */
		/* 16 Mb worth of rows */
		nRowsPerBlock = MIN(nYSize, (int)(1024 * 1024 * 16 / (nXSize[piece] * nPixelSize)));
		nBlocks = (int)ceil((float)nYSize / nRowsPerBlock);

		if (!(just_copy || copy_flipud)) {	/* Need temp memory to hold return from GDAL */
			if ((tmp = calloc((size_t)nRowsPerBlock * (size_t)nBufXSize[piece], nPixelSize)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "gdalread: failure to allocate enough memory\n");
				gmtlib_GDALDestroyDriverManager(GMT->parent);
				gmt_M_free (GMT, whichBands);
				return(-1);
			}
		}
		else {
			nRowsPerBlock = nYSize;
			nBlocks = 1;
		}

		/* Can't find why but have to multiply by nRGBA otherwise it crashes in win32 builds */
		rowVec = gmt_M_memory(GMT, NULL, (nRowsPerBlock * nRGBA) * nBlocks, size_t);
		for (m = 0; m < nY; m++) rowVec[m] = m * nX[piece];	/* Steps in pixels as we go down rows */
		colVec = gmt_M_memory(GMT, NULL, (nX[piece]+pad_w[piece]+pad_e[piece]) * nRGBA, size_t);	/* For now this will be used only to select BIP ordering */
		for (i = 0; i < nBands; i++) {
			if (!nReqBands)		/* No band selection, read them sequentially */
				hBand = GDALGetRasterBand(hDataset, i+1);
			else			/* Band selection. Read only the requested ones */
				hBand = GDALGetRasterBand(hDataset, (int)whichBands[i]);

			/* Decide if grid or pixel registration based on the data type of first band actually sent back to the GMT machinery */
			if (i == 0 && piece == 0) {	/* Do this check just once */
				pixel_reg = (GDALGetRasterDataType(hBand) == GDT_Byte);

				/* This test should give us info if a GeoTiff GRID (not image) file is grid registered but I lack one
				   example file to test it. The example mentioned in issue http://gmtrac.soest.hawaii.edu/issues/254
				   (where all this (re)started) not only is bugged as it does not carry the AREA_OR_POINT metadata.
				   So we'll check for the "Area" keyword and if found we will respect it and set grid to pix reg
				*/
				if (gmtlib_file_is_jpeg2000_tile (GMT->parent, gdal_filename) != GMT_NOTSET && (Ctrl->RasterXsize % 2) == 0 && (Ctrl->RasterYsize % 2) == 0)
					/* PW: Reading GMT server special JP2 tiles: even size implies pixel registration */
					pixel_reg = true;
				else if (!pixel_reg && GDALGetMetadataItem(hDataset, "AREA_OR_POINT", NULL) &&
					!strcmp(GDALGetMetadataItem(hDataset, "AREA_OR_POINT", NULL), "Area"))
					pixel_reg = true;
				else if (!pixel_reg && GDALGetMetadataItem(hDataset, "GDALMD_AREA_OR_POINT", NULL) &&
					!strcmp(GDALGetMetadataItem(hDataset, "GDALMD_AREA_OR_POINT", NULL), "Area"))
					pixel_reg = true;
			}

			i_x_nXYSize = i * ((size_t)nBufXSize[piece] + pad_w[piece] + pad_e[piece]) * ((size_t)nBufYSize + pad_s + pad_n);

			for (k = 0; k < nBlocks; k++) {
				nYOff = yOrigin + k * nRowsPerBlock;	/* Move data Y origin to the beginning of next block to be read */
				row_i = k * nRowsPerBlock;
				row_e = (k + 1) * nRowsPerBlock;
				buffy = nRowsPerBlock;
				for (m = 0; m < nRowsPerBlock; m++) rowVec[m+k*nRowsPerBlock] = m * nX[piece];
				if (k == nBlocks-1) {					/* Last block only by chance is not smaller than the others */
					buffy = nBufYSize - k * nRowsPerBlock;
					row_e = nYSize;
					nYOff = yOrigin + k * nRowsPerBlock;
				}
				startRow = row_i;
				endRow   = row_e;

				if (just_copy || copy_flipud)					/* In this case nBlocks was set to 1 above */
					tmp = &Ctrl->UInt8.data[i_x_nXYSize];		/* These cases don't need any temporary array */

				if ((gdal_code = GDALRasterIO(hBand, GF_Read, xOrigin[piece], nYOff, nXSize[piece], buffy, tmp,
			                 nBufXSize[piece], buffy, GDALGetRasterDataType(hBand), 0, 0)) != CE_None) {
					GMT_Report (GMT->parent, GMT_MSG_WARNING, "gmt_gdalread: GDALRasterIO failed to open band %d [err = %d]\n", i, gdal_code);
					continue;
				}

				/* If we didn't compute it yet, its time to do it now (for this block) */
				if (got_R) {	/* Update z_min, z_max in case we get here more than once (multiple blocks or two pieces) */
					ComputeRasterMinMax(GMT, tmp, hBand, adfMinMax, nXSize[piece], buffy, z_min, z_max);
					z_min = adfMinMax[0];	z_max = adfMinMax[1];
				}

				/* In the "Preview" mode those guys below are different and what we need is the BufSize */
				if (jump) {
					nXSize[piece] = nBufXSize[piece];		nYSize = nBufYSize;
				}

				startColPos = indent + pad_w[piece] + i_x_nXYSize + (complex_mode > 1);	/* Take into account indent (for second piece), nBands, Padding and Complex */
				nYSize_withPad = nYSize + pad_n + pad_s;

				if (prhs->mini_hdr.active) {		/* Read into a padded zone that can be larger then grid dims (grdpaste) */
					if (prhs->mini_hdr.side[0] == 'l' || prhs->mini_hdr.side[0] == 'r') {
						nXSize_withPad = prhs->mini_hdr.mx;
						if (prhs->mini_hdr.side[0] == 'r')
							startColPos += prhs->mini_hdr.offset;
					}
					else if (prhs->mini_hdr.side[0] == 'b') {
						startRow = prhs->mini_hdr.offset + k * nRowsPerBlock;
						endRow = startRow + nRowsPerBlock;
						if (k == nBlocks-1)
							endRow = prhs->mini_hdr.offset + nYSize;
					}
				}

				switch (GDALGetRasterDataType(hBand)) {
					case GDT_Byte:
						/* This chunk is kind of complicated because we want to take into account several different cases */
						for (n = 0; n < nXSize[piece]; n++) {
							if (do_BIP)			/* Vector for Pixel Interleaving */
								colVec[n] = n * nRGBA + i;
							else {				/* Vector for Band Sequential */
								if (topdown)
									colVec[n] = n * nY + i_x_nXYSize;
								else
									colVec[n] = n + i_x_nXYSize;
							}
						}

						Ctrl->UInt8.active = true;
						if (do_BIP || !GMT->current.gdal_read_in.O.mem_layout[0]) {
							/* Currently all calls to send image to GMT (BIP case) must come through here */
							if (rowmajor) {
								for (m = row_i; m < row_e; m++) {
									off = nRGBA * (indent+pad_w[piece]) + (pad_n+m) * (nRGBA * nXSize_withPad); /* Remember, nRGBA is variable */
									for (n = 0; n < nXSize[piece]; n++) {
										Ctrl->UInt8.data[colVec[n] + off] = tmp[rowVec[m]+n];
										//fprintf (stderr, "row = %d col = %d data(%d,%d,%d) = data(%d) = %d  off = %d\n", (int)m, (int)(xOrigin[piece]+n), (int)m, (int)(col_indent+colVec[n]), (int)i, (int)(colVec[n] + off), Ctrl->UInt8.data[colVec[n] + off], (int)off);
									}
								}
							}
							else {
								for (n = 0; n < nXSize[piece]; n++) {
									off = nRGBA * pad_n + (pad_n+n) * (nRGBA * nYSize_withPad) + i;
									for (m = row_i; m < row_e; m++)
										Ctrl->UInt8.data[m * nRGBA + off] = tmp[rowVec[m] + n];
								}
							}
						}
						else {
							if (just_copy) {	/* Here we send out the array as is, but the usage of a tmp array was a waste. Needs fix */
								memcpy (&Ctrl->UInt8.data[i_x_nXYSize], tmp, (size_t)nBufYSize * (size_t)nBufXSize[piece]);
							}
							else if (copy_flipud) {
								/* We could copy and flip but the idea here is also to not use a tmp array too */
								memcpy (&Ctrl->UInt8.data[i_x_nXYSize], tmp, (size_t)nBufYSize * (size_t)nBufXSize[piece]);
								gmt_grd_flip_vertical (&Ctrl->UInt8.data[i_x_nXYSize], (unsigned)nX[piece], (unsigned)nY, 0, 1);
							}
							else if (fliplr) {				/* No BIP option yet, and maybe never */
								for (m = row_i; m < row_e; m++) {
									nn = (pad_w[piece]+m)*nXSize_withPad + startColPos;
									for (n = nXSize[piece]-1; n >= 0; n--)
										Ctrl->UInt8.data[nn++] = tmp[rowVec[m]+n];
								}
							}
							else if (topdown) {			/* No BIP option yet, and maybe never */
								for (m = row_i; m < row_e; m++) {
									for (n = 0; n < nXSize[piece]; n++) {
										ij = colVec[n] + m;
										Ctrl->UInt8.data[ij] = tmp[rowVec[m]+n];
									}
								}
							}
						}
						break;
					case GDT_Int16:
						if (!prhs->f_ptr.active) Ctrl->Int16.active = true;
						if (gmtlib_file_is_jpeg2000_tile (GMT->parent, gdal_filename) != GMT_NOTSET) {
							/* PW: Special case of jp2 tile with possible NaNs that are not recognized without the .aux.xml file */
							float f_NaN = GMT->session.f_NaN;	/* Shorthand */
							for (m = startRow, mm = 0; m < endRow; m++, mm++) {
								nn = layer_offset + (pad_w[piece]+m)*nXSize_withPad + startColPos;
								for (n = fliplr ? nXSize[piece]-1 : 0; fliplr ? n >= 0 : n < nXSize[piece]; fliplr ? n-- : n++)
									if (prhs->f_ptr.active) {
										int16_t tmpI16;
										memcpy (&tmpI16, &tmp[(rowVec[mm] + n) * sizeof(int16_t)], sizeof(int16_t));
										Ctrl->Float.data[nn++] = (tmpI16 == -32768) ? f_NaN : tmpI16;
									}
									else
										memcpy(&Ctrl->Int16.data[nn++], &tmp[(rowVec[mm] + n) * sizeof(int16_t)], sizeof(int16_t));
							}
						}
						else {	/* All other short int cases */
							for (m = startRow, mm = 0; m < endRow; m++, mm++) {
								nn = layer_offset + (pad_w[piece]+m)*nXSize_withPad + startColPos;
								for (n = fliplr ? nXSize[piece]-1 : 0; fliplr ? n >= 0 : n < nXSize[piece]; fliplr ? n-- : n++)
									if (prhs->f_ptr.active) {
										int16_t tmpI16;
										memcpy (&tmpI16, &tmp[(rowVec[mm] + n) * sizeof(int16_t)], sizeof(int16_t));
										Ctrl->Float.data[nn++] = tmpI16;
									}
									else
										memcpy(&Ctrl->Int16.data[nn++], &tmp[(rowVec[mm] + n) * sizeof(int16_t)], sizeof(int16_t));
							}
						}
						break;
					case GDT_UInt16:
						if (!prhs->f_ptr.active) Ctrl->UInt16.active = true;
						for (m = startRow, mm = 0; m < endRow; m++, mm++) {
							nn = layer_offset + (pad_w[piece]+m)*nXSize_withPad + startColPos;
							for (n = fliplr ? nXSize[piece]-1 : 0; fliplr ? n >= 0 : n < nXSize[piece]; fliplr ? n-- : n++)
								if (prhs->f_ptr.active) {
									uint16_t tmpUI16;
									memcpy (&tmpUI16, &tmp[(rowVec[mm] + n) * sizeof(uint16_t)], sizeof(uint16_t));
									Ctrl->Float.data[nn++] = tmpUI16;
								}
								else
									memcpy (&Ctrl->UInt16.data[nn++], &tmp[(rowVec[mm] + n) * sizeof(uint16_t)], sizeof(uint16_t));
						}
						break;
					case GDT_Int32:
						if (!prhs->f_ptr.active) Ctrl->Int32.active = true;
						for (m = startRow, mm = 0; m < endRow; m++, mm++) {
							nn = layer_offset + (pad_w[piece]+m)*nXSize_withPad + startColPos;
							for (n = fliplr ? nXSize[piece]-1 : 0; fliplr ? n >= 0 : n < nXSize[piece]; fliplr ? n-- : n++)
								if (prhs->f_ptr.active) {
									int32_t tmpI32;
									memcpy (&tmpI32, &tmp[(rowVec[mm] + n) * sizeof(int32_t)], sizeof(int32_t));
									Ctrl->Float.data[nn++] = (float)tmpI32;
								}
								else
									memcpy (&Ctrl->Int32.data[nn++], &tmp[(rowVec[mm] + n) * sizeof(int32_t)], sizeof(int32_t));
						}
						break;
					case GDT_UInt32:
						if (!prhs->f_ptr.active) Ctrl->UInt32.active = true;
						for (m = startRow, mm = 0; m < endRow; m++, mm++) {
							nn = layer_offset + (pad_w[piece]+m)*nXSize_withPad + startColPos;
							for (n = fliplr ? nXSize[piece]-1 : 0; fliplr ? n >= 0 : n < nXSize[piece]; fliplr ? n-- : n++)
								if (prhs->f_ptr.active) {
									int32_t tmpUI32;
									memcpy (&tmpUI32, &tmp[(rowVec[mm] + n) * sizeof(int32_t)], sizeof(int32_t));
									Ctrl->Float.data[nn++] = (float)tmpUI32;
								}
								else
									memcpy (&Ctrl->UInt32.data[nn++], &tmp[(rowVec[mm] + n) * sizeof(int32_t)], sizeof(int32_t));
						}
						break;
					case GDT_Float32:
						Ctrl->Float.active = true;
						for (m = startRow, mm = 0; m < endRow; m++, mm++) {
							nn = layer_offset + (pad_w[piece]+m)*nXSize_withPad + startColPos;
							for (n = 0; n < nXSize[piece]; n++) {
								memcpy (&Ctrl->Float.data[nn], &tmp[(rowVec[mm]+n) * sizeof(float)], sizeof(float));
								nn += incStep;
							}
						}
						break;
					case GDT_Float64:	/* For now we don't care about doubles */
						Ctrl->Float.active = true;
						for (m = startRow, mm = 0; m < endRow; m++, mm++) {
							nn = layer_offset + (pad_w[piece]+m)*nXSize_withPad + startColPos;
							for (n = 0; n < nXSize[piece]; n++) {
								double tmpF64;
								memcpy (&tmpF64, &tmp[(rowVec[mm]+n) * sizeof(double)], sizeof(double));
								Ctrl->Float.data[nn] = (float)tmpF64;
								nn += incStep;
							}
						}
						break;
					default:
						CPLAssert(false);
				}
			}
		}
		gmt_M_free (GMT, colVec);
		gmt_M_free (GMT, rowVec);
		if (!(just_copy || copy_flipud))
			gmt_M_str_free (tmp);
		col_indent = nXSize[piece];	/* The second time (if there is one) we must step in to pick up western section */
		indent = pad_w[piece] + nXSize[piece];	/* The second time (if there is one) we must step in to pick up western section */
		layer_offset += layer_size;	/* As of now, this is never used after increment for float images */
#if 0	/* This code is problematic and commented out for now. PW, 5/15/2016 */
		if (Ctrl->Float.active && !isnan(prhs->N.nan_value)) {
			for (m = startRow, mm = 0; m < endRow ; m++, mm++) {
				nn = layer_offset + (pad_w[piece]+m)*nXSize_withPad + startColPos;
				for (n = 0; n < nXSize[piece]; n++) {
					if (Ctrl->Float.data[nn] == prhs->N.nan_value) Ctrl->Float.data[nn] = GMT->session.f_NaN;
					nn += incStep;
				}
			}
		}
#endif
	}
	gmt_M_free (GMT, whichBands);

	GDALClose(hDataset);

	gmt_M_toc (GMT, "gmt_gdalread: After gdalread data reading");

	populate_metadata (GMT, Ctrl, gdal_filename, got_R, nXSize[0]+nXSize[1], nYSize, dfULX, dfULY, dfLRX, dfLRY, z_min, z_max, first_layer);

	gmtlib_GDALDestroyDriverManager(GMT->parent);

	/* Return registration based on data type of the actually read first band.
	   We do this at the end because 'populate_metadata' scans all bands in file
	   and cannot know which one was actually read. */
	if (!pixel_reg) {		/* Grid registration */
		Ctrl->hdr[0] += Ctrl->hdr[7] / 2;	Ctrl->hdr[1] -= Ctrl->hdr[7] / 2;
		Ctrl->hdr[2] += Ctrl->hdr[8] / 2;	Ctrl->hdr[3] -= Ctrl->hdr[8] / 2;
	}

	Ctrl->nActualBands = nBands;	/* Number of bands that were actually read in */

	return (GMT_NOERROR);
}

