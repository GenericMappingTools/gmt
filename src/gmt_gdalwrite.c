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
 *	Contact info: www.generic-mapping-tools.org *
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
 *		do this from the beginning because this method needs a Dataset and we didn't
 *		have any to start with. Only the pointer to 'data'.
 *		This all story needs bo checked for potential memory leaks.
 *
 * Author:	Joaquim Luis
 * Date:	26-April-2011
 *
 * Public functions (2):
 *
 *	gmt_export_image : Write an image to disk stored in an GMT_IMAGE struct
 *	gmt_gdalwrite    : Write a GDAL grid
 */

#define GDAL_TILE_SIZE 256 /* default tile size when creating tiled GTiff */

#define N_GDAL_EXTENSIONS 5
static char *gdal_drv[N_GDAL_EXTENSIONS] = {"GTiff", "GIF", "PNG", "JPEG", "BMP"};

/*----------------------------------------------------------|
 * Public functions that are part of the GMT Devel library  |
 *----------------------------------------------------------|
 */

int gmt_export_image (struct GMT_CTRL *GMT, char *fname, struct GMT_IMAGE *I) {
	/* Take the image stored in the I structure and write to file by calling gmt_gdalwrite()
	   The image format is inferred from the image name (in *fname) file extension,
	   or optionally by appending =<driver> to the file name, where <driver> is a
	   known GDAL driver. */
	uint32_t row, col, band;
	uint64_t k, ijk, b;
	bool     free_data = false;
	char    *ext = NULL, *c = NULL, *pch;
	unsigned char *data = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_GDALWRITE_CTRL *to_GDALW = NULL;

	/* NOTE: in grdimage we have done this so may have to deal with this here.
	if (grid_registration == GMT_GRID_NODE_REG) {	Adjust domain by 1/2 pixel since they are outside the domain
		img_wesn[XLO] -= 0.5 * img_inc[0];		img_wesn[XHI] += 0.5 * img_inc[0];
		img_wesn[YLO] -= 0.5 * img_inc[1];		img_wesn[YHI] += 0.5 * img_inc[1];
	}
	*/

	to_GDALW = gmt_M_memory (GMT, NULL, 1, struct GMT_GDALWRITE_CTRL);
	if (I->header->ProjRefWKT != NULL) {
		to_GDALW->P.ProjRefWKT = I->header->ProjRefWKT;
		to_GDALW->P.active = true;
	}
	else if (I->header->ProjRefPROJ4 != NULL) {
		if (I->header->ProjRefPROJ4[1] == 'x' && I->header->ProjRefPROJ4[2] == 'y') /* -JX. Forget conversion */
			to_GDALW->P.active = false;
		else {
			to_GDALW->P.ProjRefPROJ4 = I->header->ProjRefPROJ4;
			to_GDALW->P.active = true;
		}
	}
	to_GDALW->flipud = 0;
	to_GDALW->geog   = 0;
	to_GDALW->n_columns = (int)I->header->n_columns;
	to_GDALW->n_rows    = (int)I->header->n_rows;
	to_GDALW->n_bands   = I->header->n_bands;
	to_GDALW->registration = I->header->registration;
	to_GDALW->alpha = NULL;
	to_GDALW->x_inc = I->header->inc[0];
	to_GDALW->y_inc = I->header->inc[1];
	if (to_GDALW->P.active) {		/* A referenced image */
		to_GDALW->ULx = I->header->wesn[XLO];
		to_GDALW->ULy = I->header->wesn[YHI];
	}
	else {
		to_GDALW->ULx = 1;
		to_GDALW->ULy = I->header->n_rows;
	}

	ext = gmt_get_ext (fname);
	/* See if the extension if one of the well known image formats */
	for (k = 0; to_GDALW->driver == NULL && k < N_GDAL_EXTENSIONS; k++) {
		if (k == 0 && (!strcasecmp(ext, "tif") || !strcasecmp(ext, "tiff")))	/* Tiffs happen to have a different extension<->driver naming */
			to_GDALW->driver = strdup(gdal_drv[k]);
		else if (!strcasecmp(ext, "jpg"))
			to_GDALW->driver = strdup("JPEG");	/* Jpegs too */
		else if (!strcasecmp (ext, gdal_drv[k]))
			to_GDALW->driver = strdup(gdal_drv[k]);
	}
	if (to_GDALW->driver == NULL) {				/* None of those; need to give a driver */
		if ((c = strchr(fname, ':'))) {			/* Found an ':<driver>' part */
			to_GDALW->driver = strdup(&c[1]);
			c[0] = '\0';						/* Remove the driver code from the name */
			if ((c = strstr(fname, "=gd")))		/* Check if we have not a stray =gd in name*/
				c[0] = '\0';					/* Yes, remove it too */
		}
		else if ((c = strchr (fname, '='))) {    /* Gave file=<format> so pass that along */
			to_GDALW->driver = strdup(&c[1]);
			c[0] = '\0';
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unupported image format. Supported formats are:\nBMP,GIF,JPG,PNG & TIF\n");
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Alternatively, append :<driver> for a valid GDAL driver\n");
			return GMT_NOTSET;
		}
	}

	if (!strncmp (I->header->mem_layout, "TCB", 3)) {
		to_GDALW->type = strdup("uint8");
		data = gmt_M_memory (GMT, NULL, I->header->nm * I->header->n_bands, char);

		for (k = band = 0; band < I->header->n_bands; band++) {
			b = (uint64_t)band * I->header->size;
			for (row = 0; row < I->header->n_rows; row++) {
				for (col = 0; col < I->header->n_columns; col++) {
					ijk = (uint64_t)col * I->header->my + row + I->header->pad[GMT_YHI] + b;
					data[k++] = I->data[ijk];
				}
			}
		}
		if (I->alpha) {		/* We have a transparency layer */
			to_GDALW->alpha = gmt_M_memory (GMT, NULL, I->header->nm, char);
			for (k = row = 0; row < I->header->n_rows; row++)
				for (col = 0; col < I->header->n_columns; col++)
					to_GDALW->alpha[k++] = I->alpha[(uint64_t)col * I->header->my + row + I->header->pad[GMT_YHI]];
		}
		free_data = true;
	}
	else if (!strncmp (I->header->mem_layout, "TRP", 3)) {
		to_GDALW->type = strdup("byte");
		data = I->data;
		if (to_GDALW->P.ProjRefPROJ4) {
			to_GDALW->ULx = I->header->wesn[XLO];
			to_GDALW->ULy = I->header->wesn[YHI];
			to_GDALW->x_inc = I->header->inc[0];
			to_GDALW->y_inc = I->header->inc[1];
		}
		to_GDALW->alpha = I->alpha;
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "%s memory layout is not supported, for now only: T(op)C(ol)B(and) or TRP\n",
		            I->header->mem_layout);
		gmt_M_free (GMT, to_GDALW);
		return GMT_NOTSET;
	}

	HH = gmt_get_H_hidden (I->header);
	if (HH->pocket && (pch = strstr(HH->pocket, "+c")) != NULL) 		/* If we have a list of +c<options> */
		to_GDALW->co_options = strdup(pch);				/* This memory is freed in gmt_gdalwrite */

	strncpy(to_GDALW->layout, I->header->mem_layout, 4);
	to_GDALW->data = data;
	gmt_gdalwrite (GMT, fname, to_GDALW);
	if (free_data) gmt_M_free (GMT, data);
	free (to_GDALW->driver);
	free (to_GDALW->type);
	if (free_data && to_GDALW->alpha) gmt_M_free (GMT, to_GDALW->alpha);
	gmt_M_free (GMT, to_GDALW);

	return GMT_NOERROR;
}

GMT_LOCAL int write_jp2 (struct GMT_CTRL *GMT, struct GMT_GDALWRITE_CTRL *prhs, GDALRasterBandH hBand, void *data, int n_rows, int n_cols) {
	int error = 0, i, j;
	float *t = (float *)data;
	uint64_t k, n, nm = (uint64_t)n_rows * n_cols;
	/* In gmt_gdal_write_grd we made the pointer to point to the beginning of the non-padded zone, so to make it
	   coherent we retriet pad[0]. However, nothing of this is taking into account a -R subregion so all of this
	   (and not only this case) will probably fail for that case.
	*/
	t -= prhs->pad[0];
	if (prhs->orig_type == GMT_UCHAR) {
		char *dataT = gmt_M_memory(GMT, NULL, nm, char);
		for (i = 0, k = 0; i < n_rows; i++) {
			n = (uint64_t)i*prhs->nXSizeFull + prhs->pad[0];
			for (j = 0; j < n_cols; j++)
				dataT[k++] = (char)t[n + j];
		}
		error = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, dataT, n_cols, n_rows, GDT_Byte, 0, 0);
		gmt_M_free (GMT, dataT);
	}
	else if (prhs->orig_type == GMT_USHORT) {
		short int *dataT = gmt_M_memory(GMT, NULL, nm, unsigned short int);
		for (i = 0, k = 0; i < n_rows; i++) {
			n = (uint64_t)i*prhs->nXSizeFull + prhs->pad[0];
			for (j = 0; j < n_cols; j++)
				dataT[k++] = (unsigned short int)t[n + j];
		}
		error = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, dataT, n_cols, n_rows, GDT_UInt16, 0, 0);
		gmt_M_free (GMT, dataT);
	}
	else if (prhs->orig_type == GMT_SHORT) {
		short int *dataT = gmt_M_memory(GMT, NULL, nm, short int);
		for (i = 0, k = 0; i < n_rows; i++) {
			n = (uint64_t)i*prhs->nXSizeFull + prhs->pad[0];
			for (j = 0; j < n_cols; j++)
				dataT[k++] = (short int)t[n + j];
		}
		error = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, dataT, n_cols, n_rows, GDT_Int16, 0, 0);
		gmt_M_free (GMT, dataT);
	}
	else if (prhs->orig_type == GMT_UINT) {
		unsigned int *dataT = gmt_M_memory(GMT, NULL, nm, unsigned int);
		for (i = 0, k = 0; i < n_rows; i++) {
			n = (uint64_t)i*prhs->nXSizeFull + prhs->pad[0];
			for (j = 0; j < n_cols; j++)
				dataT[k++] = (unsigned int)t[n + j];
		}
		error = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, dataT, n_cols, n_rows, GDT_UInt32, 0, 0);
		gmt_M_free (GMT, dataT);
	}
	else if (prhs->orig_type == GMT_INT) {
		int *dataT = gmt_M_memory(GMT, NULL, nm, int);
		for (i = 0, k = 0; i < n_rows; i++) {
			n = (uint64_t)i*prhs->nXSizeFull + prhs->pad[0];
			for (j = 0; j < n_cols; j++)
				dataT[k++] = (int)t[n + j];
		}
		error = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, dataT, n_cols, n_rows, GDT_Int32, 0, 0);
		gmt_M_free (GMT, dataT);
	}

	return error;
}

int gmt_gdalwrite (struct GMT_CTRL *GMT, char *fname, struct GMT_GDALWRITE_CTRL *prhs) {
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

	int  n_cols, n_rows, i;
	int  typeCLASS, typeCLASS_f, nColors, n_byteOffset, n_bands, registration;
	int  is_geog = 0, gdal_err = 0;
	uint64_t nn, ijk = 0;
	void *data;
	unsigned char *outByte = NULL, *img = NULL, *tmpByte;
	float *ptr;

	if (prhs->driver) pszFormat = prhs->driver;
	adfGeoTransform[0] =  prhs->ULx;
	adfGeoTransform[3] =  prhs->ULy;
	adfGeoTransform[1] =  prhs->x_inc;
	adfGeoTransform[5] = -prhs->y_inc;
	registration = prhs->registration;
	is_geog = prhs->geog;
	n_cols = prhs->n_columns;
	n_rows = prhs->n_rows;
	n_bands = prhs->n_bands;
	data = prhs->data;

	/* Find out in which data type was given the input array */
	/* The two first cases below are messy. Decision should be made by a mem layout code stored in prhs */
	if (!strcmp(prhs->type,"byte")) {		/* This case arrives here via grdimage */
		uint64_t imsize = gmt_M_get_nm (GMT, n_cols, n_rows);
		typeCLASS = GDT_Byte;
		n_byteOffset = 1;
		outByte = gmt_M_memory (GMT, NULL, imsize, unsigned char);
	}
	else if (!strcmp(prhs->type,"uint8")) {
		typeCLASS = GDT_Byte;
		img = (unsigned char *)data;
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
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_gdalwrite: Unsupported input data class!\n");
		return -1;
	}

	/* Jpeg2000 driver doesn't accept float arrays so we'll have to copy it into a int16 */
	if (!strcasecmp(pszFormat,"JP2OpenJPEG")) {
		if (prhs->orig_type == GMT_UCHAR)       typeCLASS_f = GDT_Byte;
		else if (prhs->orig_type == GMT_USHORT) typeCLASS_f = GDT_UInt16;
		else if (prhs->orig_type == GMT_SHORT)  typeCLASS_f = GDT_Int16;
		else if (prhs->orig_type == GMT_UINT)   typeCLASS_f = GDT_Int32;
		else if (prhs->orig_type == GMT_INT)    typeCLASS_f = GDT_UInt32;
		else {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_gdalwrite: The Jpeg2000 driver does not support floats.\n");
			gmt_M_free(GMT, outByte);
			return -1;
		}
	}
	else
		typeCLASS_f = typeCLASS;

	if (prhs->C.active) {
		nColors = prhs->C.n_colors;
		ptr = prhs->C.cpt;
		hColorTable = GDALCreateColorTable (GPI_RGB);
		for (i = 0; i < nColors; i++) {
			sEntry.c1 = (short)(ptr[i] * 255);
			sEntry.c2 = (short)(ptr[i+nColors] * 255);
			sEntry.c3 = (short)(ptr[i+2*nColors] * 255);
			sEntry.c4 = (short)255;
			GDALSetColorEntry(hColorTable, i, &sEntry);
		}
	}

	/* If grid limits were in grid registration, convert them to pixel reg */
	if (registration == 0) {
		adfGeoTransform[0] -= adfGeoTransform[1]/2.;
		adfGeoTransform[3] -= adfGeoTransform[5]/2.;
	}

	/* If we have a PROJ4 string, convert (try) it to WKT */
	if (prhs->P.active) {
		OGRSpatialReferenceH  hSRS_2 = NULL;

		hSRS_2 = OSRNewSpatialReference(NULL);

		if (prhs->P.ProjRefWKT != NULL) {
			projWKT = prhs->P.ProjRefWKT;
		}
		else if (OSRImportFromProj4(hSRS_2, prhs->P.ProjRefPROJ4) == CE_None) {
			char	*pszPrettyWkt = NULL;
			OSRExportToPrettyWkt(hSRS_2, &pszPrettyWkt, false);
			projWKT = pszPrettyWkt;
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_gdalwrite failed to convert the proj4 string\n%s\n to WKT\n",
					prhs->P.ProjRefPROJ4);
		}

		OSRDestroySpatialReference(hSRS_2);
	}

	pfnProgress = GDALDummyProgress;

	GDALAllRegister();

	hDriver = GDALGetDriverByName("MEM");		/* Intrmediary MEM diver to use as arg to GDALCreateCopy method */
	hDriverOut = GDALGetDriverByName(pszFormat);	/* The true output format driver */

	if (hDriverOut == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_gdalwrite: Output driver %s not recognized\n", pszFormat);
		if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING)) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "The following format drivers are configured and support output:\n");
			for (i = 0; i < GDALGetDriverCount(); i++) {
				hDriver = GDALGetDriver(i);
				if (GDALGetMetadataItem(hDriver, GDAL_DCAP_CREATE, NULL) != NULL ||
				    GDALGetMetadataItem(hDriver, GDAL_DCAP_CREATECOPY, NULL) != NULL)
					GMT_Report (GMT->parent, GMT_MSG_WARNING, "  %s: %s\n",
					            GDALGetDriverShortName(hDriver), GDALGetDriverLongName(hDriver));
			}
		}
		GDALDestroyDriverManager();
		gmt_M_free(GMT, outByte);
		return(-1);
	}

	if (prhs->alpha)		/* If transparency, number of requested bands must increment by one */
		hDstDS = GDALCreate(hDriver, "mem", n_cols, n_rows, n_bands+1, typeCLASS, NULL);
	else
		hDstDS = GDALCreate(hDriver, "mem", n_cols, n_rows, n_bands, typeCLASS_f, NULL);


	if (hDstDS == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "GDALOpen failed - %d\n%s\n", CPLGetLastErrorNo(), CPLGetLastErrorMsg());
		GDALDestroyDriverManager();
		gmt_M_free(GMT, outByte);
		return (-1);
	}
	GDALSetGeoTransform(hDstDS, adfGeoTransform);

	if (prhs->co_options) {
		unsigned int  pos = 0;
		char token[64];
		while (gmt_strtok (prhs->co_options, "+", &pos, token)) {
			if (token[0] == 'c')
				papszOptions = CSLAddString(papszOptions, &token[1]);	/* Jump the 'c' */
		}
	}

	/* Use default compression with GeoTiff driver, unless co_options were passed. Than the above should have taken care of it. */
	if (!strcasecmp(pszFormat,"GTiff") && !prhs->co_options) {
		papszOptions = CSLAddString(papszOptions, "COMPRESS=DEFLATE");
		/* tiles are less efficient in small grids (padding) and are not
		 * supported everywhere, when n_cols < tile_width || n_rows < tile_height */
		if (n_cols > 3 * GDAL_TILE_SIZE && n_rows > 3 * GDAL_TILE_SIZE)
			papszOptions = CSLAddString(papszOptions, "TILED=YES");

		/* Be respectful to data type registration */
		if (registration == 0)
			GDALSetMetadataItem(hDstDS, "AREA_OR_POINT", "Point", NULL);
		else
			GDALSetMetadataItem(hDstDS, "AREA_OR_POINT", "Area", NULL);
	}
	if (prhs->co_options)
		free (prhs->co_options);		/* Was allocated with an strdup() in gmt_gdal_write_grd() */

	/* This was the only trick I found to set a "projection". */
	if (is_geog || projWKT || !strcasecmp(pszFormat,"GTiff")) {
		hSRS = OSRNewSpatialReference(NULL);
		if (is_geog && !projWKT)	/* Only thing we know is that it is Geog */
			OSRSetFromUserInput(hSRS, "+proj=latlong +datum=WGS84");
		else if (projWKT)				/* Even if is_geog == true, use the WKT string */
			OSRSetFromUserInput(hSRS, projWKT);
		else
			OSRSetFromUserInput(hSRS, "LOCAL_CS[\"Unknown\"]");		/* Need a SRS for AREA_OR_POINT to be taken into account in GeoTiff */

		OSRExportToWkt(hSRS, &pszSRS_WKT);
		OSRDestroySpatialReference(hSRS);
		GDALSetProjection(hDstDS, pszSRS_WKT);
	}

	for (i = 0; i < n_bands; i++) {
		/* A problem with writing to the MEM driver is that it tests that we don't overflow
		   but the issue is that the test is done on the MEM declared size, whilst we are
		   actually using a larger array, and the dimensions passed to GDALRasterIO refer
		   to it. The trick was to offset the initial position of the 'data' array in
		   gmt_gdal_write_grd and adapt the line stride here (last GDALRasterIO argument).
		   Thanks to Even Roualt, see:
		   osgeo-org.1560.n6.nabble.com/gdal-dev-writing-a-subregion-with-GDALRasterIO-td4960500.html */
		hBand = GDALGetRasterBand(hDstDS, i+1);
		if (i == 1 && hColorTable != NULL) {
			if (GDALSetRasterColorTable(hBand, hColorTable) == CE_Failure)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "\tERROR creating Color Table");
			GDALDestroyColorTable(hColorTable);
		}
		switch (typeCLASS) {
			case GDT_Byte:
				if (rint(prhs->nan_value) == prhs->nan_value)
					/* Only set NoData if nan_value contains an integer value */
					GDALSetRasterNoDataValue(hBand, prhs->nan_value);
				if (!strcmp(prhs->type, "byte")) {
					/* This case arrives here from a separate path. It started in grdimage and an originally
					   data was in uchar but padded and possibly 3D (RGB) */
					uint64_t imsize = gmt_M_get_nm (GMT, n_cols, n_rows);
					tmpByte = (unsigned char *)data;
					for (nn = 0; nn < imsize; nn++)
						outByte[nn] = tmpByte[nn*n_bands + i];

					if ((gdal_err = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, outByte, n_cols, n_rows, typeCLASS, 0, 0)) != CE_None)
						GMT_Report(GMT->parent, GMT_MSG_ERROR, "GDALRasterIO failed to write band %d [err = %d]\n", i, gdal_err);
				}
				else {
					/* Here 'data' was converted to uchar in gmt_customio.c/gmt_gdal_write_grd */
					ijk = i * gmt_M_get_nm (GMT, n_cols, n_rows);
					if ((gdal_err = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, &img[ijk], n_cols, n_rows, typeCLASS, 0, 0)) != CE_None)
						GMT_Report(GMT->parent, GMT_MSG_ERROR, "GDALRasterIO failed to write band %d [err = %d]\n", i, gdal_err);
				}
				if (i == n_bands - 1 && prhs->alpha && prhs->layout[3] != 'A') {		/* Time to write the alpha layer. */
					hBand = GDALGetRasterBand(hDstDS, i + 2);
					if ((gdal_err = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, prhs->alpha, n_cols, n_rows, typeCLASS, 0, 0)) != CE_None)
						GMT_Report(GMT->parent, GMT_MSG_ERROR, "GDALRasterIO failed to write alpha band [err = %d]\n", gdal_err);
				}
				break;
			case GDT_UInt16:
			case GDT_Int16:
			case GDT_UInt32:
			case GDT_Int32:
				if (rint(prhs->nan_value) == prhs->nan_value)
					/* Only set NoData if nan_value contains an integer value */
					GDALSetRasterNoDataValue(hBand, prhs->nan_value);
				if ((gdal_err = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, data, n_cols, n_rows, typeCLASS, 0, 0)) != CE_None)
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "GDALRasterIO failed to write band %d [err = %d]\n", i, gdal_err);
				break;
			case GDT_Float32:
				GDALSetRasterNoDataValue(hBand, prhs->nan_value);
				if (!strcasecmp(pszFormat,"JP2OpenJPEG")) {			/* JP2 driver doesn't accept floats, so we must make a copy */
					if ((gdal_err = write_jp2 (GMT, prhs, hBand, data, n_rows, n_cols)) != CE_None)
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "GDALRasterIO failed to write band %d [err = %d]\n", i, gdal_err);
				}
				else {
					if ((gdal_err = GDALRasterIO(hBand, GF_Write, 0, 0, n_cols, n_rows, data, n_cols, n_rows, typeCLASS, 0,
					                             prhs->nXSizeFull * n_byteOffset)) != CE_None)
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "GDALRasterIO failed to write band %d [err = %d]\n", i, gdal_err);
				}
				break;
		}

		/* Compute and set image statistics (if possible) */
		GDALComputeRasterStatistics(hBand, 0, NULL, NULL, NULL, NULL, NULL, NULL);

	}

	hOutDS = GDALCreateCopy(hDriverOut, fname, hDstDS, bStrict, papszOptions, pfnProgress, NULL);
	if (hOutDS != NULL) GDALClose(hOutDS);

	CPLFree(pszSRS_WKT);
	GDALClose(hDstDS);
	GDALDestroyDriverManager();
	gmt_M_free(GMT, outByte);
	if (papszOptions != NULL) CSLDestroy (papszOptions);

	if (gmt_strlcmp(pszFormat,"netCDF")) {
		/* Change some attributes written by GDAL (not finished) */
		int ncid;
		int err;
		gmt_M_err_trap (nc_open (fname, NC_WRITE, &ncid));
		gmt_M_err_trap (nc_put_att_text (ncid, NC_GLOBAL, "history", strlen(prhs->command), prhs->command));
		gmt_M_err_trap (nc_close (ncid));
	}

	return (GMT_NOERROR);
}
