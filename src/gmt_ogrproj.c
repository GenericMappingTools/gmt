/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
/* Program:	gmt_ogrproj.c
 * Purpose:	routine to do point coordinate transformations indirectly with the proj.4 lib
 *
 * Author:	Joaquim Luis
 * Date:	17-Aug-2017
 */

#include "gdal.h"
#include "ogr_srs_api.h"

OGRCoordinateTransformationH gmt_OGRCoordinateTransformation(struct GMT_CTRL *GMT, const char *pSrcSRS, const char *pDstSRS) {
    /* pSrcSRS and pDstSRS are pointers to strings defining the Source and Destination Referencing
	   System. The SRS can be a +proj Proj.4 string, a WKT, a EPSG:n code or a filename with a WKT (?). 

	   The caller to this function is responsible to free the GDAL object created here with a call to
	   OCTDestroyCoordinateTransformation(hCT);
	*/
	OGRSpatialReferenceH hSrcSRS, hDstSRS; 
	OGRCoordinateTransformationH hCT; 

	/* ------------------ Set the Source projection ----------------------------- */
	hSrcSRS = OSRNewSpatialReference(NULL);
	if (OSRSetFromUserInput(hSrcSRS, pSrcSRS) != OGRERR_NONE) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "OGRPROJ: Translating source SRS failed.\n%s\n", pSrcSRS);
		return NULL;
	}
	/* ------------------- Set the Target projection ---------------------------- */
	CPLErrorReset();
	hDstSRS = OSRNewSpatialReference(NULL);
	if (OSRSetFromUserInput(hDstSRS, pDstSRS) != OGRERR_NONE) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "OGRPROJ: Translating target SRS failed.\n%s\n", pDstSRS);
		OSRDestroySpatialReference(hSrcSRS);	/* It was just created above */
		return NULL;
	}
	/* -------------------------------------------------------------------------- */

	hCT = OCTNewCoordinateTransformation(hSrcSRS, hDstSRS);
	if (hCT == NULL) {
		char *pszSrcWKT = NULL, *pszDstWKT = NULL;
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to create coordinate transformation between the following\n"
					"coordinate systems. This may be because they are not transformable,\n"
					"or because projection services (PROJ.4 DLL/.so) could not be loaded.\n");
		OSRExportToPrettyWkt(hSrcSRS, &pszSrcWKT, FALSE);
		OSRExportToPrettyWkt(hDstSRS, &pszDstWKT, FALSE);
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Source:\n\n%s\n\n%s\n\n", pszSrcWKT, pszDstWKT);
		CPLFree(pszSrcWKT);		CPLFree(pszDstWKT);
	}
	OSRDestroySpatialReference(hSrcSRS);	OSRDestroySpatialReference(hDstSRS);
	return hCT;
}

int gmt_ogrproj(struct GMT_CTRL *GMT, char *pSrcSRS, char *pDstSRS, int n_pts,
                double *xi, double *yi, double *zi, bool insitu, double *xo, double *yo, double *zo) {
    /* pSrcSRS and pDstSRS are pointers to strings defining the Source and Destination 
	   Referencing System. The SRS can be a +proj Proj.4 string, a WKT, a EPSG:n code or a filename with a WKT (?). 
	   n_pts is the number of points to be transformed.
	   xi,yi,zi are pointers to arrays of n_pts points. If only 2D transform is wanted, passs zi = NULL.
	   insitu, is a boolean stating if the transformed points will overwrite the input data in xi,yi,zi (true)
	   or, when false, the output is stored in xo,yo,zo. In this later case, it's user responsibility
	   to allocate the xo,yo[,zo] arrays with same size as xi,yi[,zi].
	*/
	OGRCoordinateTransformationH hCT = gmt_OGRCoordinateTransformation(GMT, pSrcSRS, pDstSRS); 

	if (insitu)
		OCTTransform(hCT, n_pts, xi, yi, zi);
	else {
		int n;
		for (n = 0; n < n_pts; n++) {
			xo[n] = xi[n];
			yo[n] = yi[n];
		}
		if (zi) {		/* If z component too */
			for (n = 0; n < n_pts; n++) zo[n] = zi[n];
		}
		OCTTransform(hCT, n_pts, xo, yo, zo);
	}

	OCTDestroyCoordinateTransformation(hCT);
	return (GMT_NOERROR);
}

void gmt_ogrproj_one_pt(OGRCoordinateTransformationH hCT, double *xi, double *yi, double *zi) {
	/* Suitable to call on a rec-by-rec basis because *hCT must have been initiated outside.
	   Again, *zi may be NULL for the 2D case and after last point the hCT GDAL object must be cleaned by a call to
	   OCTDestroyCoordinateTransformation(hCT);
	*/
	OCTTransform(hCT, 1, xi, yi, zi);
}

void gmt_proj4_fwd(struct GMT_CTRL *GMT, double xi, double yi, double *xo, double *yo) {
	/* Function that have the same signature as the GMT coordinate transforms */
	*xo = xi;	*yo = yi;
	gmt_ogrproj_one_pt(GMT->current.gdal_read_in.hCT_fwd, xo, yo, NULL);
}

void gmt_proj4_inv(struct GMT_CTRL *GMT, double *xi, double *yi, double xo, double yo) {
	/* The inverse transform */
	*xi = xo;	*yi = yo;
	gmt_ogrproj_one_pt(GMT->current.gdal_read_in.hCT_inv, xi, yi, NULL);
}
