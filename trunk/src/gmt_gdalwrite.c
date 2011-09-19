/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
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

int GMT_gdalwrite (struct GMT_CTRL *C, char *fname, struct GDALWRITE_CTRL *prhs) {
	int	flipud, i_x_nXYSize, bQuiet = FALSE, bStrict = FALSE;
	char **papszOptions = NULL, *projWKT = NULL;
	char *pszFormat = "GTiff"; 
	double adfGeoTransform[6] = {0,1,0,0,0,1}; 
	char *pszSRS_WKT = NULL;
	OGRSpatialReferenceH hSRS;
	GDALDatasetH	hDstDS, hOutDS;
	GDALDriverH	hDriver, hDriverOut;
	GDALRasterBandH hBand;
	GDALColorTableH	hColorTable = NULL;
	GDALColorEntry	sEntry;
	GDALProgressFunc    pfnProgress = GDALTermProgress;

	int	nx, ny, i, nn, n_bands, registration = 1;
	int	typeCLASS, nColors;
	int	is_geog = 0;
	void	*data;
	unsigned char *outByte = NULL, *tmpByte;
	float	*ptr;

	pszFormat = prhs->driver;
	adfGeoTransform[0] =  prhs->ULx;
	adfGeoTransform[3] =  prhs->ULy;
	adfGeoTransform[1] =  prhs->x_inc;
	adfGeoTransform[5] = -prhs->y_inc;
	registration = prhs->registration;
	flipud  = prhs->flipud;
	is_geog = prhs->geog;
	nx = prhs->nx;
	ny = prhs->ny;
	n_bands = prhs->n_bands;
	data = (void *)prhs->data;

	/* Find out in which data type was given the input array */
	if (!strcmp(prhs->type,"byte")) {
		typeCLASS = GDT_Byte;
		outByte = GMT_memory (C, NULL, nx*ny, unsigned char);
	}
	else if (!strcmp(prhs->type,"float")) {
		typeCLASS = GDT_Float32;
	}
	else {
		GMT_report (C, GMT_MSG_FATAL, "GMT_gdalwrite: Unsuported input data class!\n");
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
			OSRExportToPrettyWkt( hSRS_2, &pszPrettyWkt, FALSE );
			projWKT = pszPrettyWkt;
		}
		else {
			GMT_report (C, GMT_MSG_FATAL, "Warning: GMT_gdalwrite failed to convert the proj4 string\n%s\n to WKT\n", 
					prhs->P.ProjectionRefPROJ4);
		}

		OSRDestroySpatialReference( hSRS_2 );
	}

	bQuiet = TRUE;
	pfnProgress = GDALDummyProgress;

	GDALAllRegister();

	hDriver = GDALGetDriverByName( "MEM" );		/* Intrmediary MEM diver to use as arg to GDALCreateCopy method */
	hDriverOut = GDALGetDriverByName( pszFormat );	/* The true output format driver */
    
	if( hDriverOut == NULL ) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_gdalwrite: Output driver %s not recognized\n", pszFormat );
		/* The following is s bit idiot. The loop should only be executed is verbose so requires */
		GMT_report (C, GMT_MSG_NORMAL, "The following format drivers are configured and support output:\n" );
		for (i = 0; i < GDALGetDriverCount(); i++) {
			hDriver = GDALGetDriver(i);
			if ( GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) != NULL || 
			     GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATECOPY, NULL ) != NULL )
				GMT_report (C, GMT_MSG_NORMAL, "  %s: %s\n", 
					GDALGetDriverShortName( hDriver ), GDALGetDriverLongName( hDriver ) );
		}
		return(-1);
	}

	/* Use compression with GeoTiff driver */
	if (!strcmp(pszFormat,"GTiff")) {
		papszOptions = CSLAddString( papszOptions, "COMPRESS=DEFLATE" ); 
		/* Florian says: tiles are not supported everywhere, so leave out */
		/* papszOptions = CSLAddString( papszOptions, "TILED=YES" ); */
	}

	hDstDS = GDALCreate( hDriver, "mem", nx, ny, n_bands, typeCLASS, NULL );

	if (hDstDS == NULL) {
		GMT_report (C, GMT_MSG_FATAL, "GDALOpen failed - %d\n%s\n", CPLGetLastErrorNo(), CPLGetLastErrorMsg());
		return(-1);
	}
	GDALSetGeoTransform( hDstDS, adfGeoTransform ); 

	/* This was the only trick I found to set a "projection". */
	if (is_geog || projWKT) {
		hSRS = OSRNewSpatialReference( NULL );
		if (is_geog && !projWKT)	/* Only thing we know is that it is Geog */
			OSRSetFromUserInput( hSRS, "+proj=latlong +datum=WGS84" );
		else				/* Even if is_geog == TRUE, use the WKT string */ 
			OSRSetFromUserInput( hSRS, projWKT );
		OSRExportToWkt( hSRS, &pszSRS_WKT );
		OSRDestroySpatialReference( hSRS );
		GDALSetProjection( hDstDS, pszSRS_WKT );
	}

	for (i = 0; i < n_bands; i++) {
		hBand = GDALGetRasterBand( hDstDS, i+1 ); 
		if( i == 1 && hColorTable != NULL ) {
			if (GDALSetRasterColorTable( hBand, hColorTable ) == CE_Failure)
				GMT_report(C, GMT_MSG_FATAL, "\tERROR creating Color Table");
			GDALDestroyColorTable( hColorTable );
		}
		i_x_nXYSize = i*nx*ny;		/* We don't need to recompute this everytime */
		switch( typeCLASS ) {
			case GDT_Byte:
			 	tmpByte = (unsigned char *)data;	
				for (nn = 0; nn < nx*ny; nn++) {
					outByte[nn] = tmpByte[nn*n_bands + i];
				}
				GDALRasterIO( hBand, GF_Write, 0, 0, nx, ny, outByte, nx, ny, typeCLASS, 0, 0 );
				break;
			case GDT_Float32:
				GDALRasterIO( hBand, GF_Write, 0, 0, nx, ny, (float *)data, nx, ny, typeCLASS, 0, 0 );
				break;
		}
	}

	hOutDS = GDALCreateCopy( hDriverOut, fname, hDstDS, bStrict, papszOptions, pfnProgress, NULL );
	if ( hOutDS != NULL ) GDALClose( hOutDS );

	GDALClose( hDstDS );

	if (outByte) GMT_free(C, outByte);

	return (GMT_NOERROR);
}
