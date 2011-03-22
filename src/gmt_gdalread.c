/*--------------------------------------------------------------------
 *	$Id: gmt_gdalread.c,v 1.18 2011-03-22 00:01:40 jluis Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/* Program:	GMT_gdalread.c
 * Purpose:	routine to read files supported by gdal
 * 		and dumping all or selected band data of that dataset.
 *
 * Author:	Joaquim Luis
 * Date:	27-Aug-2009
 * Revision: 1		Based on gdalread.c MEX from Mirone
 *			The populate_metadata() was modified from mexgdal of John Evans (johnevans@acm.org)
 *
 */

int record_geotransform (char *gdal_filename, GDALDatasetH hDataset, double *adfGeoTransform);
int populate_metadata (struct GMT_CTRL *C, struct GD_CTRL *, char * ,GMT_LONG , GMT_LONG, GMT_LONG, int, int, double, double, double, double, double, double);
int ReportCorner (struct GMT_CTRL *C, GDALDatasetH hDataset, double x, double y, double *xy_c, double *xy_geo);
void ComputeRasterMinMax (struct GMT_CTRL *C, char *tmp, GDALRasterBandH hBand, double adfMinMax[2], GMT_LONG nXSize, GMT_LONG nYSize, double, double);
int gdal_decode_columns (char *txt, GMT_LONG *whichBands, GMT_LONG n_col);

int GMT_gdalread (struct GMT_CTRL *C, char *gdal_filename, struct GDALREAD_CTRL *prhs, struct GD_CTRL *Ctrl) {
	const char	*format = NULL;
	int	bGotNodata, metadata_only = FALSE;
	int	do_BIP = FALSE;	/* For images if BIP == TRUE data is stored Pixel interleaved, otherwise Band interleaved */
	int	nRGBA = 3;	/* 3 for RGB only and 4 for RGB with alpha channel (If needed, value is updated bellow) */
	int	complex = 0;	/* 0 real only. 1|2 if complex array is to hold real (1) and imaginary (2) parts */ 
	int	nPixelSize, nBands, i, nReqBands = 0;
	int	anSrcWin[4], xOrigin = 0, yOrigin = 0;
	int	jump = 0, nXSize = 0, nYSize = 0, nX, nY, nXYSize, nBufXSize, nBufYSize;
	GMT_LONG	pixel_reg = FALSE, correct_bounds = FALSE, fliplr = FALSE;
	GMT_LONG	n, m, nn, got_R = FALSE, got_r = FALSE, error = FALSE;
	GMT_LONG	*whichBands = NULL, *mVector = NULL, *nVector = NULL;
	GMT_LONG	n_alloc, n_commas, n_dash, pad = 0, i_x_nXYSize, startColPos, nXSize_withPad;
	GMT_LONG	incStep = 1;	/* 1 for real only arrays and 2 for complex arrays (index step increment) */
	char	*tmp = NULL;
	float	*tmpF32 = NULL;
	double	*tmpF64 = NULL, dfNoData, adfMinMax[2];
	double	dfULX = 0.0, dfULY = 0.0, dfLRX = 0.0, dfLRY = 0.0;
	double	z_min = 1e50, z_max = -1e50;
	GDALDatasetH	hDataset;
	GDALRasterBandH	hBand;
	GDALDriverH	hDriver;
	GInt16	*tmpI16 = NULL;
	GUInt16	*tmpUI16 = NULL;
	GInt32	*tmpI32 = NULL;
	GUInt32	*tmpUI32 = NULL;

	Ctrl->band_field_names = NULL;		/* So we can test before trying to read its fields */ 
	GMT_memset (anSrcWin, 4, int);

	if (prhs->B.active) {		/* We have a selected bands request */
		for (n = 0, n_commas = 0; prhs->B.bands[n]; n++) if (prhs->B.bands[n] == ',') n_commas = n;
		for (n = 0, n_dash = 0; prhs->B.bands[n]; n++) if (prhs->B.bands[n] == '-') n_dash = n;
		nn = MAX(n_commas, n_dash);
		if (nn)
			nn = atoi(&prhs->B.bands[nn+1]);
		else
			nn = atoi(prhs->B.bands);
		whichBands = calloc(nn, sizeof(GMT_LONG));
		nReqBands = gdal_decode_columns (prhs->B.bands, whichBands, nn);
	}
	if (prhs->C.active)
		correct_bounds = TRUE;

	if (prhs->F.active)
		pixel_reg = TRUE;

	if (prhs->I.active)
		do_BIP = TRUE;

	if (prhs->L.active)
		fliplr = TRUE;

	if (prhs->M.active)
		metadata_only = TRUE;

	if (prhs->p.active)
		pad = prhs->p.pad;

	if (prhs->R.active) {
		got_R = TRUE;
		error += GMT_parse_common_options (C, "R", 'R', prhs->R.region);
		if (!error) {
			dfULX = C->common.R.wesn[XLO];	dfLRX = C->common.R.wesn[XHI];
			dfLRY = C->common.R.wesn[YLO];	dfULY = C->common.R.wesn[YHI];
		}
	}

	if (prhs->r.active) { 		/* Region is given in pixels */
		got_r = TRUE;
		error += GMT_parse_common_options (C, "R", 'R', prhs->r.region);
		if (!error) {
			dfULX = C->common.R.wesn[XLO];	dfLRX = C->common.R.wesn[XHI];
			dfLRY = C->common.R.wesn[YLO];	dfULY = C->common.R.wesn[YHI];
		}
	}

	if (error) {
		GMT_report (C, GMT_MSG_FATAL, "ERROR: GMT_gdalread failed to extract a Sub-region\n");
		return (-1);
	}

	if (prhs->P.active)
		jump = atoi(prhs->P.jump);

	if (prhs->Z.active) {
		complex = prhs->Z.complex;
		if (complex) incStep = 2;
	}

	/* Open gdal - */

	GDALAllRegister();

	if (prhs->W.active) {
		OGRSpatialReferenceH  hSRS;
		/* const char *str = Ctrl->ProjectionRefPROJ4; */

		hSRS = OSRNewSpatialReference(NULL);

		if( OSRImportFromProj4( hSRS, Ctrl->ProjectionRefPROJ4) == CE_None ) {
			char	*pszPrettyWkt = NULL;
			OSRExportToPrettyWkt( hSRS, &pszPrettyWkt, FALSE );
			Ctrl->ProjectionRefWKT = pszPrettyWkt; 
		}
		else {
			Ctrl->ProjectionRefWKT = CNULL; 
			GMT_report (C, GMT_MSG_FATAL, 
				"WARNING: GMT_gdalread failed to convert the proj4 string\n%s\n to WKT\n", 
				Ctrl->ProjectionRefPROJ4);
		}

		OSRDestroySpatialReference( hSRS );
		return (GMT_NOERROR);
	}

	if (metadata_only) {
		populate_metadata (C, Ctrl, gdal_filename, correct_bounds, pixel_reg, got_R, 
				nXSize, nYSize, dfULX, dfULY, dfLRX, dfLRY, z_min, z_max);
		return (GMT_NOERROR);
	}

	hDataset = GDALOpen(gdal_filename, GA_ReadOnly);

	if (hDataset == NULL) {
		GMT_report (C, GMT_MSG_FATAL, "GDALOpen failed %s\n", CPLGetLastErrorMsg());
		return (-1);
	}

	/* Some formats (tipically DEMs) have their origin at Bottom Left corner.
	   For those we have to flip the data matrix to be in accord with matlab (Top Left) */

	hDriver = GDALGetDatasetDriver(hDataset);
	format = GDALGetDriverShortName(hDriver);

	if (!strcmp(format,"ESAT"))	/* ENVISAT data are flipped left-right */
		fliplr = TRUE;

	if (got_R || got_r) {
		/* -------------------------------------------------------------------- */
		/*      Compute the source window from the projected source window      */
		/*      if the projected coordinates were provided.  Note that the      */
		/*      projected coordinates are in ulx, uly, lrx, lry format,         */
		/*      while the anSrcWin is xoff, yoff, xsize, ysize with the         */
		/*      xoff,yoff being the ulx, uly in pixel/line.                     */
		/* -------------------------------------------------------------------- */
		double	adfGeoTransform[6];

		GDALGetGeoTransform( hDataset, adfGeoTransform );

		if( adfGeoTransform[2] != 0.0 || adfGeoTransform[4] != 0.0 ) {
			GMT_report (C, GMT_MSG_FATAL, "The -projwin option was used, but the geotransform is rotated." 
							" This configuration is not supported.\n");
			GDALClose( hDataset );
			GDALDestroyDriverManager();
			return (-1);
		}

		if (got_R) {	/* Region in map coordinates */
			anSrcWin[0] = (int) ((dfULX - adfGeoTransform[0]) / adfGeoTransform[1] + 0.001);
			anSrcWin[1] = (int) ((dfULY - adfGeoTransform[3]) / adfGeoTransform[5] + 0.001);
			anSrcWin[2] = (int) ((dfLRX - dfULX) / adfGeoTransform[1] + 0.5);
			anSrcWin[3] = (int) ((dfLRY - dfULY) / adfGeoTransform[5] + 0.5);
			if (GDAL_VERSION_NUM < 1700 && !strcmp(format,"netCDF")) {
				/* PATCH against the never ending GDAL bug of reading netCDF files */
				anSrcWin[1] = GDALGetRasterYSize(hDataset) - (anSrcWin[1] + anSrcWin[3]) - 1;
			}
		}
		else {		/* Region in pixel/line */
			anSrcWin[0] = (int) (dfULX);
			anSrcWin[1] = (int) (dfLRY);
			anSrcWin[2] = (int) (dfLRX - dfULX);
			anSrcWin[3] = (int) (dfULY - dfLRY);
		}

		if( anSrcWin[0] < 0 || anSrcWin[1] < 0
			|| anSrcWin[0] + anSrcWin[2] > GDALGetRasterXSize(hDataset)
			|| anSrcWin[1] + anSrcWin[3] > GDALGetRasterYSize(hDataset) ) {
			GMT_report (C, GMT_MSG_FATAL, "Computed -srcwin falls outside raster size of %dx%d.\n", 
					GDALGetRasterXSize(hDataset), GDALGetRasterYSize(hDataset) );
			return (-1);
		}
		xOrigin = anSrcWin[0];
		yOrigin = anSrcWin[1];
		nXSize = anSrcWin[2];
		nYSize = anSrcWin[3];
		if (correct_bounds) {	/* Patch for the bug reading GMT grids */
			nXSize++;
			nYSize++;
		}
	}
	else {			/* Use entire dataset */
		xOrigin = yOrigin = 0;
		nXSize = GDALGetRasterXSize(hDataset);
		nYSize = GDALGetRasterYSize(hDataset);
	}

	/* The following assumes that all bands have the same PixelSize. Otherwise ... */
	hBand = GDALGetRasterBand(hDataset,1);
	nPixelSize = GDALGetDataTypeSize(GDALGetRasterDataType(hBand)) / 8;	/* /8 because return value is in BITS */

	if (jump) {
		nBufXSize = GDALGetRasterXSize(hDataset) / jump;
		nBufYSize = GDALGetRasterYSize(hDataset) / jump;
	}
	else {
		nBufXSize = nXSize;	nBufYSize = nYSize;
	}

	nBands = GDALGetRasterCount(hDataset);
	nXYSize = nBufXSize * nBufYSize;

	if (nReqBands) nBands = MIN(nBands,nReqBands);	/* If a band selection was made */

	n_alloc = nBands * (nBufXSize + 2*pad) * (nBufYSize + 2*pad);
	switch ( GDALGetRasterDataType(hBand) ) {
		case GDT_Byte:
			Ctrl->UInt8.data = GMT_memory (C, NULL, n_alloc, unsigned char);
			if (nBands == 4 && do_BIP) {	/* Assume fourth band holds the alpha channel */
				nRGBA = 4;
			}
			else if (nBands < 3 || nBands > 4) {
				GMT_message (C, "gdalread: BIP request ignored since n of bands is not 3 or 4\n");
				do_BIP = FALSE;
			}
			break;
		case GDT_Int16:
			Ctrl->Int16.data = GMT_memory (C, NULL, n_alloc, short int);
			break;
		case GDT_UInt16:
			Ctrl->UInt16.data = GMT_memory (C, NULL, n_alloc, unsigned short int);
			break;
		case GDT_Int32:
			Ctrl->Int32.data = GMT_memory (C, NULL, n_alloc, int);
			break;
		case GDT_UInt32:
			Ctrl->UInt32.data = GMT_memory (C, NULL, n_alloc, int);
			break;
		case GDT_Float32:
		case GDT_Float64:
			if (prhs->fpointer.active)	/* We have a pointer with already allocated memory ready to use */
				Ctrl->Float.data = prhs->fpointer.grd;
			else {
				if (!complex)
					Ctrl->Float.data = GMT_memory (C, NULL, n_alloc, float);
				else
					Ctrl->Float.data = GMT_memory (C, NULL, 2 * n_alloc, float);
			}
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "gdalread: Unsupported data type\n");
			break;
		
	}

	tmp = calloc(nBufYSize * nBufXSize, nPixelSize);
	if (tmp == NULL) {
		GMT_report (C, GMT_MSG_FATAL, "gdalread: failure to allocate enough memory\n");
		return(-1);
	}

	/* ------ compute two vectors indices that will be used inside loops below --------- */
	/* In the "Preview" mode those guys bellow are different and what we need is the BufSize */
	if (jump) {
		nX = nBufXSize;		nY = nBufYSize;
	}
	else {
		nX = nXSize;		nY = nYSize;
	}

	mVector = GMT_memory(C, NULL, nY, GMT_LONG);
	for (m = 0; m < nY; m++) mVector[m] = m*nX;
	nVector = GMT_memory(C, NULL, nX, GMT_LONG);
	/* --------------------------------------------------------------------------------- */

	for ( i = 0; i < nBands; i++ ) {
		if (!nReqBands)		/* No band selection, read them sequentialy */
			hBand = GDALGetRasterBand( hDataset, i+1 );
		else			/* Band selection. Read only the requested ones */
			hBand = GDALGetRasterBand( hDataset, (int)whichBands[i] );

		GDALRasterIO(hBand, GF_Read, xOrigin, yOrigin, nXSize, nYSize,
			tmp, nBufXSize, nBufYSize, GDALGetRasterDataType(hBand), 0, 0 );

        	dfNoData = GDALGetRasterNoDataValue(hBand, &bGotNodata);
		/* If we didn't computed it yet, its time to do it now */
		if (got_R) ComputeRasterMinMax(C, tmp, hBand, adfMinMax, nXSize, nYSize, z_min, z_max);

		/* In the "Preview" mode those guys bellow are different and what we need is the BufSize */
		if (jump) {
			nXSize = nBufXSize;
			nYSize = nBufYSize;
			i_x_nXYSize = i*(nXSize+2*pad)*(nYSize+2*pad);		/* We don't need to recompute this everytime */
		}
		else
			i_x_nXYSize= i * (nBufXSize + 2*pad) * (nBufYSize + 2*pad);

		nXSize_withPad = nXSize + 2 * pad;
		startColPos = pad + i_x_nXYSize + (complex > 1);	/* Take into account nBands, Padding and Complex */

		switch ( GDALGetRasterDataType(hBand) ) {
			case GDT_Byte:
				for (n = 0; n < nXSize; n++)
					if (do_BIP)
						nVector[n] = n * nRGBA + i_x_nXYSize;
					else
						nVector[n] = n;
				Ctrl->UInt8.active = TRUE;
				if (fliplr) {
					for (m = 0; m < nYSize; m++) {
						nn = m * nXSize + i_x_nXYSize;
						for (n = nXSize-1; n >= 0; n--)
							Ctrl->UInt8.data[nn++] = tmp[mVector[m]+n];
					}
				}
				else
					for (m = 0; m < nYSize; m++) {
						nn = m * nXSize + i_x_nXYSize;
						for (n = 0; n < nXSize; n++)
							Ctrl->UInt8.data[nVector[n]] = tmp[mVector[m]+n];
					}
				break;
			case GDT_Int16:
				tmpI16 = (GInt16 *) tmp;
				Ctrl->Int16.active = TRUE;
				if (fliplr) {
					for (m = 0; m < nYSize; m++) {
						nn = pad + (pad+m)*(nXSize_withPad) + i_x_nXYSize;
						for (n = nXSize-1; n >= 0; n--)
							Ctrl->Int16.data[nn++] = tmpI16[mVector[m]+n];
					}
				}
				else {
					for (m = 0; m < nYSize; m++) {
						nn = pad + (pad+m)*(nXSize_withPad) + i_x_nXYSize;
						for (n = 0; n < nXSize; n++)
							Ctrl->Int16.data[nn++] = tmpI16[mVector[m]+n];
					}
				}
				break;
			case GDT_UInt16:
				tmpUI16 = (GUInt16 *) tmp;
				Ctrl->UInt16.active = TRUE;
				if (fliplr) {
					for (m = 0; m < nYSize; m++) {
						nn = pad + (pad+m)*(nXSize_withPad) + i_x_nXYSize;
						for (n = nXSize-1; n >= 0; n--)
							Ctrl->UInt16.data[nn++] = tmpUI16[mVector[m]+n];
					}
				}
				else {
					for (m = 0; m < nYSize; m++) {
						nn = pad + (pad+m)*(nXSize_withPad) + i_x_nXYSize;
						for (n = 0; n < nXSize; n++)
							Ctrl->UInt16.data[nn++] = tmpUI16[mVector[m]+n];
					}
				}
				break;
			case GDT_Int32:
				tmpI32 = (GInt32 *) tmp;
				Ctrl->Int32.active = TRUE;
				if (fliplr) {
					for (m = 0; m < nYSize; m++) {
						nn = pad + (pad+m)*(nXSize_withPad) + i_x_nXYSize;
						for (n = nXSize-1; n >= 0; n--)
							Ctrl->Int32.data[nn++] = tmpI32[mVector[m]+n];
					}
				}
				else {
					for (m = 0; m < nYSize; m++) {
						nn = pad + (pad+m)*(nXSize_withPad) + i_x_nXYSize;
						for (n = 0; n < nXSize; n++)
							Ctrl->Int32.data[nn++] = tmpI32[mVector[m]+n];
					}
				}
				break;
			case GDT_UInt32:
				tmpUI32 = (GUInt32 *) tmp;
				Ctrl->UInt32.active = TRUE;
				if (fliplr) {
					for (m = 0; m < nYSize; m++) {
						nn = pad + (pad+m)*(nXSize_withPad) + i_x_nXYSize;
						for (n = nXSize-1; n >= 0; n--)
							Ctrl->UInt32.data[nn++] = tmpUI32[mVector[m]+n];
					}
				}
				else {
					for (m = 0; m < nYSize; m++) {
						nn = pad + (pad+m)*(nXSize_withPad) + i_x_nXYSize;
						for (n = 0; n < nXSize; n++)
							Ctrl->UInt32.data[nn++] = tmpUI32[mVector[m]+n];
					}
				}
				break;
			case GDT_Float32:
				tmpF32 = (float *) tmp;
				Ctrl->Float.active = TRUE;
				for (m = 0; m < nYSize; m++) {
					nn = (pad+m)*(nXSize_withPad) + startColPos;
					for (n = 0; n < nXSize; n++) {
						Ctrl->Float.data[nn] = tmpF32[mVector[m]+n];
						nn += incStep;
					}
				}
				break;
			case GDT_Float64:	/* For now we don't care about doubles */
				tmpF64 = (double *) tmp;
				Ctrl->Float.active = TRUE;
				for (m = 0; m < nYSize; m++) {
					nn = (pad+m)*(nXSize_withPad) + startColPos;
					for (n = 0; n < nXSize; n++) {
						Ctrl->Float.data[nn] = (float)tmpF64[mVector[m]+n];
						nn += incStep;
					}
				}
				break;
			default:
				CPLAssert( FALSE );
		}
	}

	GMT_free(C, mVector);
	free(tmp);
	if (prhs->B.active) free(whichBands);
	if (nVector) GMT_free(C, nVector);

	GDALClose(hDataset);

	populate_metadata (C, Ctrl, gdal_filename, correct_bounds, pixel_reg, got_R, 
				nXSize, nYSize, dfULX, dfULY, dfLRX, dfLRY, z_min, z_max);

	return (GMT_NOERROR);
}

/* =============================================================================================== */
/*
 * This routine queries the GDAL raster file for some metadata
 *
 * Fields:
 *    ProjectionRefPROJ4:  a Proj4 type string describing the projection
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
 *    ColorInterp:
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
 * */
int populate_metadata (struct GMT_CTRL *C, struct GD_CTRL *Ctrl, char *gdal_filename , GMT_LONG correct_bounds,
			GMT_LONG pixel_reg, GMT_LONG got_R, int nXSize, int nYSize, double dfULX, double dfULY,
			double dfLRX, double dfLRY, double z_min, double z_max) {

	GDALDriverH hDriver;		/* This is the driver chosen by the GDAL library to query the dataset. */
	GDALDatasetH hDataset;		/* pointer structure used to query the gdal file. */
	GDALRasterBandH hBand;
	GDALColorTableH	hTable;
	GDALColorEntry	sEntry;

	int	i, n_colors;		/* Number of colors in the eventual Color Table */ 
	int	status, bSuccess;	/* success or failure */
	int	xSize, ySize; 		/* Dimensions of the dataset */
	int	nBand, raster_count;
	double 	adfGeoTransform[6];	/* bounds on the dataset */
	double	tmpdble;		/* temporary value */
	double	xy_c[2], xy_geo[4][2];	/* Corner coordinates in the local coords system and geogs (if it exists) */
	int	bGotMin, bGotMax;	/* To know if driver transmited Min/Max */
	double	adfMinMax[2];		/* Dataset Min Max */
	double	nodata;

	/* ------------------------------------------------------------------------- */
	/* Open the file (if we can). */
	/* ------------------------------------------------------------------------- */
	hDataset = GDALOpen ( gdal_filename, GA_ReadOnly );
	if ( hDataset == NULL ) {
		GMT_report (C, GMT_MSG_FATAL, "Unable to open %s.\n", gdal_filename );
		return ( -1 );
	}

	/* ------------------------------------------------------------------------- */
	/* Record the ProjectionRef */
	/* ------------------------------------------------------------------------- */
	if( GDALGetProjectionRef( hDataset ) != NULL ) {
		OGRSpatialReferenceH  hSRS;
		char *pszProjection;
		char *pszResult = NULL;
		pszProjection = (char *) GDALGetProjectionRef( hDataset );

		hSRS = OSRNewSpatialReference(NULL);
		/* First in PROJ4 format */
		if( OSRImportFromWkt( hSRS, &pszProjection) == CE_None ) {
			OSRExportToProj4( hSRS, &pszResult );
			Ctrl->ProjectionRefPROJ4 = pszResult; 
		}
		else
			Ctrl->ProjectionRefPROJ4 = CNULL; 

		/* Now in WKT format */
		if( OSRImportFromWkt( hSRS, &pszProjection ) == CE_None ) {
			char	*pszPrettyWkt = NULL;
			OSRExportToPrettyWkt( hSRS, &pszPrettyWkt, FALSE );
			Ctrl->ProjectionRefWKT = pszPrettyWkt; 
			CPLFree( pszPrettyWkt );
		}
		else
			Ctrl->ProjectionRefWKT = CNULL; 

		OSRDestroySpatialReference( hSRS );
	}

	/* ------------------------------------------------------------------------- */
	/* Record the geotransform. */
	/* ------------------------------------------------------------------------- */
	status = record_geotransform ( gdal_filename, hDataset, adfGeoTransform );
	if (!strcmp(GDALGetDriverShortName(GDALGetDatasetDriver(hDataset)),"netCDF") && 
		    GDAL_VERSION_NUM <= 1450) {
		adfGeoTransform[3] *= -1;
		adfGeoTransform[5] *= -1;
	}
	if ( status == 0 ) {
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

	/* ------------------------------------------------------------------------- */
	/* Get driver information */
	/* ------------------------------------------------------------------------- */
	hDriver = GDALGetDatasetDriver( hDataset );

	Ctrl->DriverShortName = GDALGetDriverShortName( hDriver );
	Ctrl->DriverLongName  = GDALGetDriverLongName( hDriver );

	if (!got_R) {		/* That is, if we are using the entire dataset */
		xSize = GDALGetRasterXSize( hDataset );
		ySize = GDALGetRasterYSize( hDataset );
	}
	else if (got_R && nXSize == 0 && nYSize == 0) {		/* metadata_only */
		int	anSrcWin[4];
		anSrcWin[0] = anSrcWin[1] = anSrcWin[2] = anSrcWin[3] = 0;
		if( adfGeoTransform[2] != 0.0 || adfGeoTransform[4] != 0.0 ) {
			GMT_report (C, GMT_MSG_FATAL, "The -projwin option was used, but the geotransform is rotated."
							" This configuration is not supported.\n");
			GDALClose( hDataset );
			GDALDestroyDriverManager();
			GMT_report (C, GMT_MSG_FATAL, "Quiting with error\n");
			return(-1);
		}

		anSrcWin[0] = (int) ((dfULX - adfGeoTransform[0]) / adfGeoTransform[1] + 0.001);
		anSrcWin[1] = (int) ((dfULY - adfGeoTransform[3]) / adfGeoTransform[5] + 0.001);
		anSrcWin[2] = (int) ((dfLRX - dfULX) / adfGeoTransform[1] + 0.5);
		anSrcWin[3] = (int) ((dfLRY - dfULY) / adfGeoTransform[5] + 0.5);

		if( anSrcWin[0] < 0 || anSrcWin[1] < 0
			|| anSrcWin[0] + anSrcWin[2] > GDALGetRasterXSize(hDataset)
			|| anSrcWin[1] + anSrcWin[3] > GDALGetRasterYSize(hDataset) ) {
			GMT_report (C, GMT_MSG_FATAL, "Computed -srcwin falls outside raster size of %dx%d.\n", 
							GDALGetRasterXSize(hDataset), GDALGetRasterYSize(hDataset) );
			GMT_report (C, GMT_MSG_FATAL, "Quiting with error\n");
			return(-1);
		}
		nXSize = anSrcWin[2];	nYSize = anSrcWin[3];
		if (correct_bounds) {	/* Patch for the hostility towards grid registration grids */
			nXSize++;	nYSize++;
		}

		xSize = nXSize;
		ySize = nYSize;
	}
	else {
		xSize = nXSize;
		ySize = nYSize;
	}

	Ctrl->RasterXsize = xSize;
	Ctrl->RasterYsize = ySize;
	raster_count = GDALGetRasterCount( hDataset );
	Ctrl->RasterCount = raster_count;

	/* ------------------------------------------------------------------------- */
	/* Get some metadata for each band. */
	/* ------------------------------------------------------------------------- */

	Ctrl->band_field_names = GMT_memory(C, NULL, raster_count, struct GDAL_BAND_FNAMES );

	/* ==================================================================== */
	/*      Loop over bands.                                                */
	/* ==================================================================== */
	for ( nBand = 0; nBand < raster_count; ++nBand ) {

		hBand = GDALGetRasterBand( hDataset, nBand+1 );

		if (!got_R) {		/* Not sure about what will realy happen in this case */
			Ctrl->band_field_names[nBand].XSize = (GMT_LONG)GDALGetRasterBandXSize( hBand );
			Ctrl->band_field_names[nBand].YSize = (GMT_LONG)GDALGetRasterBandYSize( hBand );
		}
		else {
			Ctrl->band_field_names[nBand].XSize = (GMT_LONG)nXSize;
			Ctrl->band_field_names[nBand].YSize = (GMT_LONG)nYSize;
		}

		Ctrl->band_field_names[nBand].DataType = strdup( GDALGetDataTypeName( GDALGetRasterDataType ( hBand )) );
		Ctrl->band_field_names[nBand].nodata = (double)GDALGetRasterNoDataValue( hBand, &status );
	
		/* Get band's Min/Max. If the band has no record of it we won't compute it */
		adfMinMax[0] = GDALGetRasterMinimum( hBand, &bGotMin );
		adfMinMax[1] = GDALGetRasterMaximum( hBand, &bGotMax );
		if ( bGotMin && bGotMax ) {
			Ctrl->band_field_names[nBand].MinMax[0] = adfMinMax[0];
			Ctrl->band_field_names[nBand].MinMax[1] = adfMinMax[1];
		}
		else {		/* No Min/Max, we won't computer it either */
			Ctrl->band_field_names[nBand].MinMax[0] = C->session.d_NaN;
			Ctrl->band_field_names[nBand].MinMax[1] = C->session.d_NaN;
		}

		/* Get band's Scale/Offset. If the band does not have them use the neutral 1/0 values */
		if (GDALGetRasterScale( hBand, &bSuccess ) != 1 || GDALGetRasterOffset( hBand, &bSuccess ) != 0) {
			Ctrl->band_field_names[nBand].ScaleOffset[0] = GDALGetRasterScale ( hBand, &bSuccess );
			Ctrl->band_field_names[nBand].ScaleOffset[1] = GDALGetRasterOffset( hBand, &bSuccess );
		}
		else {
			Ctrl->band_field_names[nBand].ScaleOffset[0] = 1.0;
			Ctrl->band_field_names[nBand].ScaleOffset[1] = 0.0;
		}

		/* Here the Mirone code has a chunk to read overviews info, but since we have not
		   yet any use for it I won't implement that just yet.

		   Continuing the lazy path, and according to the same excuse that we don't have yet
		   any use for it, we won't check if individual bands have different colormaps.
		   Note, however, that first band colormap is captured anyway (if exists, off course) */

	}

	/* ------------------------------------------------------------------------- */
	/* Get the Color Interpretation Name */
	/* ------------------------------------------------------------------------- */
	hBand = GDALGetRasterBand( hDataset, 1 );
	if (raster_count > 0)
		Ctrl->ColorInterp = GDALGetColorInterpretationName( GDALGetRasterColorInterpretation(hBand) );
	else
		Ctrl->ColorInterp = strdup ("nikles");

	/* ------------------------------------------------------------------------- */
	/* Get the first band NoData Value */
	/* ------------------------------------------------------------------------- */
	nodata = (double) (GDALGetRasterNoDataValue ( hBand, &status ) );
	if ( status == 0 ) 
		Ctrl->nodata = nodata;
	else
		Ctrl->nodata = 0;	/* How the hell I set a novalue-nodatavalue ? */

	/* ------------------------------------------------------------------------- */
	/* Get the Color Map of first band (if any) */
	/* ------------------------------------------------------------------------- */
	if( GDALGetRasterColorInterpretation(hBand) == GCI_PaletteIndex 
		&& (hTable = GDALGetRasterColorTable( hBand )) != NULL ) {

		n_colors = GDALGetColorEntryCount( hTable );
		Ctrl->ColorMap = GMT_memory(C, NULL, n_colors*4, int); 
		for( i = 0; i < n_colors; i++ ) {
			GDALGetColorEntryAsRGB( hTable, i, &sEntry );
			Ctrl->ColorMap[i*4]   = sEntry.c1;
			Ctrl->ColorMap[i*4+1] = sEntry.c2;
			Ctrl->ColorMap[i*4+2] = sEntry.c3;
			Ctrl->ColorMap[i*4+3] = sEntry.c4;
		}
	}
	else
		Ctrl->ColorMap = NULL;

	/* ------------------------------------------------------------------------- */
	/* Record corners. */
	/* ------------------------------------------------------------------------- */
	if (!got_R)					/* Lower Left */ 
		ReportCorner (C, hDataset, 0.0, GDALGetRasterYSize(hDataset), xy_c, xy_geo[0]);
	else
		{xy_c[0] = dfULX;	xy_c[1] = dfLRY;}
	Ctrl->Corners.LL[0] = Ctrl->hdr[0] = xy_c[0];	/* xmin, ymin */
	Ctrl->Corners.LL[1] = Ctrl->hdr[2] = xy_c[1];

	if (!got_R)					/* Upper Left */
		ReportCorner(C, hDataset, 0.0, 0.0, xy_c, xy_geo[1]);				
	else
		{xy_c[0] = dfULX;	xy_c[1] = dfULY;}
	Ctrl->Corners.UL[0] = xy_c[0];
	Ctrl->Corners.UL[1] = xy_c[1];

	if (!got_R)					/* Upper Rigt */
		ReportCorner(C, hDataset, GDALGetRasterXSize(hDataset), 0.0, xy_c, xy_geo[2]);
	else
		{xy_c[0] = dfLRX;	xy_c[1] = dfULY;}
	Ctrl->Corners.UR[0] = Ctrl->hdr[1] = xy_c[0];	/* xmax, ymax */
	Ctrl->Corners.UR[1] = Ctrl->hdr[3] = xy_c[1];

	if (!got_R)					/* LR */
		ReportCorner(C, hDataset, GDALGetRasterXSize(hDataset), GDALGetRasterYSize(hDataset), xy_c, xy_geo[3]);
	else
		{xy_c[0] = dfLRX;	xy_c[1] = dfLRY;}
	Ctrl->Corners.LR[0] = xy_c[0];
	Ctrl->Corners.LR[1] = xy_c[1];

	/* --------------------------------------------------------------------------------------
	 * Record Geographical corners (if they exist)
	 * -------------------------------------------------------------------------------------- */
	if( !got_R ) {
		if( !GMT_is_dnan(xy_geo[0][0]) ) {
			Ctrl->GEOGCorners.LL[0] = xy_geo[0][0]; Ctrl->GEOGCorners.LL[1] = xy_geo[0][1];
			Ctrl->GEOGCorners.UL[0] = xy_geo[1][0]; Ctrl->GEOGCorners.UL[1] = xy_geo[1][1];
			Ctrl->GEOGCorners.UR[0] = xy_geo[2][0]; Ctrl->GEOGCorners.UR[1] = xy_geo[2][1];
			Ctrl->GEOGCorners.LR[0] = xy_geo[3][0]; Ctrl->GEOGCorners.LR[1] = xy_geo[3][1];
		}
		else {
			Ctrl->GEOGCorners.LL[0] = Ctrl->GEOGCorners.LL[1] = C->session.d_NaN;
		}
	}
	else {
		Ctrl->GEOGCorners.LL[0] = Ctrl->GEOGCorners.LL[1] = C->session.d_NaN;
	}

	/* ------------------------------------------------------------------------- */
	/* Fill in the rest of the GMT header values (If ...) */
	if (raster_count > 0) {
		if (z_min == 1e50) {		/* We don't know yet the dataset Min/Max */
			GDALComputeRasterMinMax( hBand, TRUE, adfMinMax );
			Ctrl->hdr[4] = adfMinMax[0]; 
			Ctrl->hdr[5] = adfMinMax[1]; 
		}
		else {
			Ctrl->hdr[4] = z_min;
			Ctrl->hdr[5] = z_max;
		}	

		if (!pixel_reg)			/* See if we want grid or pixel registration */
			Ctrl->hdr[6] = 0; 
		else
			Ctrl->hdr[6] = 1; 
		Ctrl->hdr[7] = adfGeoTransform[1];
		Ctrl->hdr[8] = fabs(adfGeoTransform[5]);

		if (Ctrl->hdr[2] > Ctrl->hdr[3]) {	/* Sometimes GDAL does it: y_min > y_max. If so, revert it */
			tmpdble = Ctrl->hdr[2];
			Ctrl->hdr[2] = Ctrl->hdr[3];
			Ctrl->hdr[3] = tmpdble;
		}

		if (correct_bounds && !got_R) {
			Ctrl->hdr[0] += Ctrl->hdr[7] / 2;	Ctrl->hdr[1] -= Ctrl->hdr[7] / 2;
			Ctrl->hdr[2] += Ctrl->hdr[8] / 2;	Ctrl->hdr[3] -= Ctrl->hdr[8] / 2;
		}
		else if (got_R) {
			Ctrl->hdr[0] = dfULX;	Ctrl->hdr[1] = dfLRX;
			Ctrl->hdr[2] = dfLRY;	Ctrl->hdr[3] = dfULY;
		}
	}


	GDALClose ( hDataset );

	return ( GMT_NOERROR );
}

/************************************************************************/
/*                        ReportCorner()                                */
/************************************************************************/

int ReportCorner (struct GMT_CTRL *C, GDALDatasetH hDataset, double x, double y, double *xy_c, double *xy_geo) {
	double	dfGeoX, dfGeoY;
	const char  *pszProjection = NULL;
	double	adfGeoTransform[6];
	OGRCoordinateTransformationH hTransform = NULL;
	OGRSpatialReferenceH hProj, hLatLong = NULL;

	xy_geo[0] = xy_geo[1] = C->session.d_NaN;		/* Default return values */
/* -------------------------------------------------------------------- */
/*      Transform the point into georeferenced coordinates.             */
/* -------------------------------------------------------------------- */
	if( GDALGetGeoTransform( hDataset, adfGeoTransform ) == CE_None ) {
        	pszProjection = GDALGetProjectionRef(hDataset);
		if (!strcmp(GDALGetDriverShortName(GDALGetDatasetDriver(hDataset)),"netCDF") && 
			    GDAL_VERSION_NUM <= 1450) {
			adfGeoTransform[3] *= -1;
			adfGeoTransform[5] *= -1;
		}
        	dfGeoX = adfGeoTransform[0] + adfGeoTransform[1] * x + adfGeoTransform[2] * y;
        	dfGeoY = adfGeoTransform[3] + adfGeoTransform[4] * x + adfGeoTransform[5] * y;
	}
	else {
		xy_c[0] = x;	xy_c[1] = y;
        	return FALSE;
	}

/* -------------------------------------------------------------------- */
/*      Report the georeferenced coordinates.                           */
/* -------------------------------------------------------------------- */
	xy_c[0] = dfGeoX;	xy_c[1] = dfGeoY;

/* -------------------------------------------------------------------- */
/*      Setup transformation to lat/long.                               */
/* -------------------------------------------------------------------- */
    if( pszProjection != NULL && strlen(pszProjection) > 0 ) {

        hProj = OSRNewSpatialReference( pszProjection );
        if( hProj != NULL )
            hLatLong = OSRCloneGeogCS( hProj );

        if( hLatLong != NULL ) {
            CPLPushErrorHandler( CPLQuietErrorHandler );
            hTransform = OCTNewCoordinateTransformation( hProj, hLatLong );
            CPLPopErrorHandler();
            OSRDestroySpatialReference( hLatLong );
        }

        if( hProj != NULL )
            OSRDestroySpatialReference( hProj );
    }
/*
 * --------------------------------------------------------------------
 *      Transform to latlong and report.                                
 * --------------------------------------------------------------------
 */
	if( hTransform != NULL && OCTTransform(hTransform,1,&dfGeoX,&dfGeoY,NULL) )
		xy_geo[0] = dfGeoX;	xy_geo[1] = dfGeoY;

	if( hTransform != NULL )
		OCTDestroyCoordinateTransformation( hTransform );

	return TRUE;
}

/* ---------------------------------------------------------------------------------
 * record_geotransform:
 *
 * If the gdal file is not internally georeferenced, try to get the world file.
 * Returns -1 in case no world file is found.
 */
int record_geotransform ( char *gdal_filename, GDALDatasetH hDataset, double *adfGeoTransform ) {
	int status = -1;
	char generic_buffer[5000];

	if( GDALGetGeoTransform( hDataset, adfGeoTransform ) == CE_None )
		return (0);

	/* Try a world file.  First the generic extension. If the gdal_filename
	   is, say, "a.tif", then this will look for "a.wld". */
	if ( GDALReadWorldFile ( gdal_filename, "wld", adfGeoTransform ) )
		return (0);

	/* Try again, but try "a.tif.wld" instead. */
	sprintf ( generic_buffer, "%s.xxx", gdal_filename );
	status = GDALReadWorldFile ( generic_buffer, "wld", adfGeoTransform );
	if (status == 1)
		return (0);

	return (-1);
}

/* -------------------------------------------------------------------- */
void ComputeRasterMinMax(struct GMT_CTRL *C, char *tmp, GDALRasterBandH hBand, double adfMinMax[2], 
			GMT_LONG nXSize, GMT_LONG nYSize, double z_min, double z_max) {
	/* Compute Min/Max of a sub-region. I'm forced to do this because the
	GDALComputeRasterMinMax works only on the entire dataset */
	int	i, bGotNoDataValue;
	GInt16	*tmpI16 = NULL;
	GUInt16	*tmpUI16 = NULL;
	GInt32	*tmpI32 = NULL;
	GUInt32	*tmpUI32 = NULL;
	float	*tmpF32 = NULL;
	double	dfNoDataValue, *tmpF64;

        dfNoDataValue = GDALGetRasterNoDataValue(hBand, &bGotNoDataValue);
	switch( GDALGetRasterDataType(hBand) ) {
		case GDT_Byte:
			for (i = 0; i < nXSize*nYSize; i++) {
				if( bGotNoDataValue && tmp[i] == dfNoDataValue ) continue;
				z_min = MIN(tmp[i], z_min);
				z_max = MAX(tmp[i], z_max);
			}
			break;
		case GDT_Int16:
			tmpI16 = (GInt16 *) tmp;
			for (i = 0; i < nXSize*nYSize; i++) {
				if( bGotNoDataValue && tmpI16[i] == dfNoDataValue ) continue;
				z_min = MIN(tmpI16[i], z_min);
				z_max = MAX(tmpI16[i], z_max);
			}
			break;
		case GDT_UInt16:
			tmpUI16 = (GUInt16 *) tmp;
			for (i = 0; i < nXSize*nYSize; i++) {
				if( bGotNoDataValue && tmpUI16[i] == dfNoDataValue ) continue;
				z_min = MIN(tmpUI16[i], z_min);
				z_max = MAX(tmpUI16[i], z_max);
			}
			break;
		case GDT_Int32:
			tmpI32 = (GInt32 *) tmp;
			for (i = 0; i < nXSize*nYSize; i++) {
				if( bGotNoDataValue && tmpI32[i] == dfNoDataValue ) continue;
				z_min = MIN(tmpI32[i], z_min);
				z_max = MAX(tmpI32[i], z_max);
			}
			break;
		case GDT_UInt32:
			tmpUI32 = (GUInt32 *) tmp;
			for (i = 0; i < nXSize*nYSize; i++) {
				if( bGotNoDataValue && tmpUI32[i] == dfNoDataValue ) continue;
				z_min = MIN(tmpUI32[i], z_min);
				z_max = MAX(tmpUI32[i], z_max);
			}
			break;
		case GDT_Float32:
			tmpF32 = (float *) tmp;
			for (i = 0; i < nXSize*nYSize; i++) {
				if( bGotNoDataValue && tmpF32[i] == dfNoDataValue ) continue;
				z_min = MIN(tmpF32[i], z_min);
				z_max = MAX(tmpF32[i], z_max);
			}
			break;
		case GDT_Float64:
			tmpF64 = (double *) tmp;
			for (i = 0; i < nXSize*nYSize; i++) {
				if( bGotNoDataValue && tmpF64[i] == dfNoDataValue ) continue;
				z_min = MIN(tmpF64[i], z_min);
				z_max = MAX(tmpF64[i], z_max);
			}
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "gdalread: Unsupported data type\n");
			break;
	}
	adfMinMax[0] = z_min;
	adfMinMax[1] = z_max;
}

/* -------------------------------------------------------------------- */
int gdal_decode_columns (char *txt, GMT_LONG *whichBands, GMT_LONG n_col) {
	GMT_LONG n = 0, i, start, stop, pos = 0;
	char p[1024];

	while ((GMT_strtok (txt, ",", &pos, p))) {
		if (strchr (p, '-'))
			sscanf (p, "%" GMT_LL "d-%" GMT_LL "d", &start, &stop);
		else {
			sscanf (p, "%" GMT_LL "d", &start);
			stop = start;
		}
		stop = MIN (stop, n_col);
		for (i = start; i <= stop; i++) {
			whichBands[n] = i;
			n++;
		}
	}
	return ((int)n);
}
