/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu *
 *--------------------------------------------------------------------*/

/* Program:	GMT_gdawrite.c
 * Purpose:	routine to write files supported by gdal
 *
 *		Note: this is a quite crude version tested only with RGB images.
 *		I also had to make a trick in order to be able to use the more general
 *		GDALCreateCopy method. Otherwise one could not create images in the more
 *		common formats such as JPEG or PNG.
 *		The trick is to create an intermediary MEM diver, write the data to it
 *		and finally use it as a dataset argument to GDALCreateCopy. One could not
 *		do this from the begining because this method needs a Dataset and we didn't
 *		have any to start with. Only the pointer to 'data'.
 *		This all story needs bo checked for potential memory leaks. 
 *
 * Author:	Joaquim Luis
 * Date:	26-April-2011
 *
 */

#define GDAL_TILE_SIZE 256 /* default tile size when creating tiled GTiff */

int GMT_gdalwrite (struct GMT_CTRL *GMT, char *fname, struct GDALWRITE_CTRL *prhs) {
	int	bStrict = false;
	char **papszOptions = NULL, *projWKT = NULL;
	char *pszFormat = "GTiff"; 
	double adfGeoTransform[6] = {0,1,0,0,0,1}; 
	char *pszSRS_WKT = NULL;
	OGRSpatialReferenceH hSRS;
	GDALDatasetH     hDstDS, hOutDS;
	GDALDriverH      hDriver, hDriverOut;
	GDALRasterBandH  hBand;
	GDALColorTableH  hColorTable = NULL;
	GDALColorEntry   sEntry;
	GDALProgressFunc pfnProgress = GDALTermProgress;

	int  nx, ny, i, nn;
	int  typeCLASS, nColors, n_byteOffset, n_bands, registration = 1;
	int  is_geog = 0;
	void *data;
	unsigned char *outByte = NULL, *tmpByte;
	float *ptr;

	pszFormat = prhs->driver;
	adfGeoTransform[0] =  prhs->ULx;
	adfGeoTransform[3] =  prhs->ULy;
	adfGeoTransform[1] =  prhs->x_inc;
	adfGeoTransform[5] = -prhs->y_inc;
	registration = prhs->registration;
	is_geog = prhs->geog;
	nx = prhs->nx;
	ny = prhs->ny;
	n_bands = prhs->n_bands;
	data = prhs->data;

	/* Find out in which data type was given the input array */
	if (!strcmp(prhs->type,"byte")) {		/* This case arrives here via grdimage */
		typeCLASS = GDT_Byte;
		n_byteOffset = 1;
		outByte = GMT_memory (GMT, NULL, nx*ny, unsigned char);
	}
	else if (!strcmp(prhs->type,"uint8")) {
		typeCLASS = GDT_Byte;
		n_byteOffset = 1;
	}
	else if (!strcmp(prhs->type,"uint16")) {
		typeCLASS = GDT_UInt16;
		n_byteOffset = 2;
	}
	else if (!strcmp(prhs->type,"int16")) {
		typeCLASS = GDT_Int16;
		n_byteOffset = 2;
	}
	else if (!strcmp(prhs->type,"uint32")) {
		typeCLASS = GDT_UInt32;
		n_byteOffset = 4;
	}
	else if (!strcmp(prhs->type,"int32")) {
		typeCLASS = GDT_Int32;
		n_byteOffset = 4;
	}
	else if (!strcmp(prhs->type,"float32")) {
		typeCLASS = GDT_Float32;
		n_byteOffset = 4;
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_gdalwrite: Unsuported input data class!\n");
		return(-1);
	}

	if (prhs->C.active) {
		nColors = prhs->C.n_colors;
		ptr = prhs->C.cpt;
		hColorTable = GDALCreateColorTable (GPI_RGB); 
		for (i = 0; i < nColors; i++) {
			sEntry.c1 = (short)(ptr[i] * 255); 
			sEntry.c2 = (short)(ptr[i+nColors] * 255); 
			sEntry.c3 = (short)(ptr[i+2*nColors] * 255); 
			sEntry.c4 = (short)255; 
			GDALSetColorEntry( hColorTable, i, &sEntry ); 
		}
	}

	/* If grid limits were in grid registration, convert them to pixel reg */
	if (registration == 0) {
		adfGeoTransform[0] -= adfGeoTransform[1]/2.;
		adfGeoTransform[3] -= adfGeoTransform[5]/2.;
	}

	/* If we have a PROJ4 string, convert (try) it to WKT */
	if (prhs->P.active) {
		OGRSpatialReferenceH  hSRS_2;

		hSRS_2 = OSRNewSpatialReference(NULL);

		if( OSRImportFromProj4( hSRS_2, prhs->P.ProjectionRefPROJ4) == CE_None ) {
			char	*pszPrettyWkt = NULL;
			OSRExportToPrettyWkt( hSRS_2, &pszPrettyWkt, false );
			projWKT = pszPrettyWkt;
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: GMT_gdalwrite failed to convert the proj4 string\n%s\n to WKT\n", 
					prhs->P.ProjectionRefPROJ4);
		}

		OSRDestroySpatialReference( hSRS_2 );
	}

	pfnProgress = GDALDummyProgress;

	GDALAllRegister();

	hDriver = GDALGetDriverByName( "MEM" );		/* Intrmediary MEM diver to use as arg to GDALCreateCopy method */
	hDriverOut = GDALGetDriverByName( pszFormat );	/* The true output format driver */
    
	if( hDriverOut == NULL ) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_gdalwrite: Output driver %s not recognized\n", pszFormat );
		/* The following is s bit idiot. The loop should only be executed is verbose so requires */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "The following format drivers are configured and support output:\n" );
		for (i = 0; i < GDALGetDriverCount(); i++) {
			hDriver = GDALGetDriver(i);
			if ( GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) != NULL || 
			     GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATECOPY, NULL ) != NULL )
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "  %s: %s\n", 
					GDALGetDriverShortName( hDriver ), GDALGetDriverLongName( hDriver ) );
		}
		GDALDestroyDriverManager();
		return(-1);
	}

	/* Use compression with GeoTiff driver */
	if (!strcasecmp(pszFormat,"GTiff")) {
		papszOptions = CSLAddString( papszOptions, "COMPRESS=DEFLATE" ); 
		/* tiles are less efficient in small grids (padding) and are not
		 * supported everywhere, when nx < tile_width || ny < tile_height */
		if ( nx > 3 * GDAL_TILE_SIZE && ny > 3 * GDAL_TILE_SIZE )
			papszOptions = CSLAddString( papszOptions, "TILED=YES" );
	}

	hDstDS = GDALCreate( hDriver, "mem", nx, ny, n_bands, typeCLASS, NULL );

	if (hDstDS == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GDALOpen failed - %d\n%s\n", CPLGetLastErrorNo(), CPLGetLastErrorMsg());
		GDALDestroyDriverManager();
		if (papszOptions != NULL) CSLDestroy (papszOptions);
		return(-1);
	}
	GDALSetGeoTransform( hDstDS, adfGeoTransform ); 

	/* This was the only trick I found to set a "projection". */
	if (is_geog || projWKT) {
		hSRS = OSRNewSpatialReference( NULL );
		if (is_geog && !projWKT)	/* Only thing we know is that it is Geog */
			OSRSetFromUserInput( hSRS, "+proj=latlong +datum=WGS84" );
		else				/* Even if is_geog == true, use the WKT string */ 
			OSRSetFromUserInput( hSRS, projWKT );
		OSRExportToWkt( hSRS, &pszSRS_WKT );
		OSRDestroySpatialReference( hSRS );
		GDALSetProjection( hDstDS, pszSRS_WKT );
	}

	for (i = 0; i < n_bands; i++) {
		/* A problem with writing to the MEM driver is that it tests that we dont overflow
		   but the issue is that the test is done on the MEM declared size, whilst we are
		   actually using a larger array, and the dimensions passed to GDALRasterIO refer
		   to it. The trick was to offset the initial position of the 'data' array in 
		   GMT_gdal_write_grd and adapt the line stride here (last GDALRasterIO argument).
		   Thanks to Even Roualt, see: 
		   osgeo-org.1560.n6.nabble.com/gdal-dev-writing-a-subregion-with-GDALRasterIO-td4960500.html */
		hBand = GDALGetRasterBand( hDstDS, i+1 ); 
		if( i == 1 && hColorTable != NULL ) {
			if (GDALSetRasterColorTable( hBand, hColorTable ) == CE_Failure)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "\tERROR creating Color Table");
			GDALDestroyColorTable( hColorTable );
		}
		switch( typeCLASS ) {
			case GDT_Byte:
				if (rint(prhs->nan_value) == prhs->nan_value)
					/* Only set NoData if nan_value contains an integer value */
					GDALSetRasterNoDataValue(hBand, prhs->nan_value);
				if (strcmp(prhs->type,"uint8")) {
					/* This case arrives here from a separate path. It started in grdimage and an originaly
					   data was in uchar but padded and possibly 3D (RGB) */
					tmpByte = (unsigned char *)data;
					for (nn = 0; nn < nx*ny; nn++) {
						outByte[nn] = tmpByte[nn*n_bands + i];
					}
					GDALRasterIO( hBand, GF_Write, 0, 0, nx, ny, outByte, nx, ny, typeCLASS, 0, 0 );
				}
				else
					/* Here 'data' was converted to uchar in gmt_customio.c/GMT_gdal_write_grd */
					GDALRasterIO( hBand, GF_Write, 0, 0, nx, ny, data, nx, ny, typeCLASS, 0, 0 );
				break;
			case GDT_UInt16:
			case GDT_Int16:
			case GDT_UInt32:
			case GDT_Int32:
				if (rint(prhs->nan_value) == prhs->nan_value)
					/* Only set NoData if nan_value contains an integer value */
					GDALSetRasterNoDataValue(hBand, prhs->nan_value);
				GDALRasterIO( hBand, GF_Write, 0, 0, nx, ny, data, nx, ny, typeCLASS, 0, 0 );
				break;
			case GDT_Float32:
				GDALSetRasterNoDataValue(hBand, prhs->nan_value);
				GDALRasterIO( hBand, GF_Write, 0, 0, nx, ny, data, nx, ny, typeCLASS, 0, 
				              prhs->nXSizeFull * n_byteOffset );
				break;
		}

		/* Compute and set image statistics (if possible) */
		GDALComputeRasterStatistics(hBand, 0, NULL, NULL, NULL, NULL, NULL, NULL);

	}

	hOutDS = GDALCreateCopy( hDriverOut, fname, hDstDS, bStrict, papszOptions, pfnProgress, NULL );
	if ( hOutDS != NULL ) GDALClose( hOutDS );

	GDALClose( hDstDS );
	GDALDestroyDriverManager();
	if (outByte) GMT_free(GMT, outByte);
	if (papszOptions != NULL) CSLDestroy (papszOptions);

	if (GMT_strlcmp(pszFormat,"netCDF")) {
		/* Change some attributes written by GDAL (not finished) */
		int ncid;
		int err;
		GMT_err_trap (nc_open (fname, NC_WRITE, &ncid));
		GMT_err_trap (nc_put_att_text (ncid, NC_GLOBAL, "history", strlen(prhs->command), prhs->command));
		GMT_err_trap (nc_close (ncid));
	}

	return (GMT_NOERROR);
}
