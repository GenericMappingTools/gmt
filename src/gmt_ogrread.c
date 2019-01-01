/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/* Program:	gmt_ogrread.c
 * Purpose:	routine to read files supported by OGR and dumping all vector data of that dataset.
 *
 * The calling syntax is:
 *	s = gmt_ogrread (vector_file);
 *
 *	"s" is a 2D or 3D structure array with fields:
 *
 *	Name:		A string holding the layer name.
 *	SRSWkt:		A string describing the reference system in the Well Known Format.
 *	SRSProj4:	A string describing the reference system as a Proj4 string
 *	BoundingBox:	The 2D dataset BoundingBox as a 2x2 matrix with Xmin/Xmax in first column and Y in second
 *	Type:		Geometry type. E.g. Point, Polygon or LineString
 *	X:		Column vector of doubles with the vector x-coordinates		
 *	Y:		Column vector of doubles with the vector y-coordinates		
 *	Z:		Same for z when vector is 3D, otherwise empty
 *	Islands:	2 columns matrix with start and ending indexes of the main Ring and its islands (if any).
 *			This only applies to Polygon geometries that have interior rings (islands).
 *	BB_geom:	Not currently assigned (would be the BoundingBox of each individual geometry)
 *	Att_number:	Number of attributes of a Feature
 *	Att_names:	Names of the attributes of a Feature
 *	Att_values:	Values of the attributes of a Feature as strings
 *	Att_types:	Because "Att_values" came as strings, this is a vector with the codes allowing
 *			the conversion into their original data types as returned by OGR_Fld_GetType(hField). 
 *			Thus if the code of element n is 2 (OFTReal) Att_values[n] can be converted to double with
 *			atof. See ogr_core.h for the list of codes and their meanings.
 *
 * Now, given the potential complexity of an OGR dataset the "s" structure can be either a 2D or 3D dimensional
 * struct array. On the simpler case of one single layer the structure is 2D. If more layers are present in the
 * dataset, each layer will be stored in each of the planes (pages) of the 3D array.
 * Also, besides the three simpler geometries (Point, LineString & Polygon) we can have also the more complex
 * combinations of MultiPoint|Line|Polygon or even the GeometryCollections. So each plane of the "s" array is
 * organized as in the following example:
 *
 *	F1 [G11 G12 ...   G1N]
 *	F2 |G21 G22 [] ... []|
 *	F3 |G31 [] ....... []|
 *	FM [.................]
 *
 * Each Gij element of the matrix is a "s" structure as describe above. The F1 .. FM are the features collection
 * in the Layer. Because one Feature can have one or many Geometries the array is defined as having as many columns
 * as the maximum number of Geometries in all Features. There are as many rows as Features in the Layer. However,
 * not all fields of the individual Gij are filled. Only those that contain real data. Also, to not waste space
 * and given that the Attributes of all Geometries of a Feature are equal, only the first column Gm1 elements
 * have assigned the "Att_names|values|types". That is, the others are defined but not filled. The same applies
 * to all other matrix elements represented as []. Again, they are defined (they have to be because the matrix
 * must be rectangular) but their members are not filled.
 *
 * One final note regarding the "Islands" struct element. Because we want to keep trace on paternity of interior
 * rings (islands) of a polygon, each one of those interior polygons is appended to the main polygon but separated
 * with one row of NaNs (2 or 3 NaNs depending if vector is 2d or 3D). The "Islands" element contains thus a Nx2
 * matrix with the indexes of the starting and ending positions of the N polygons that were once the Polygon and
 * its interior rings in the OGR model. For Polygons with no islands, "Islands" is an empty ([]) variable. 
 *
 * --------------------------------------------------------------------------------------------------------------
 *
 * Author:	Joaquim Luis
 * Date:	15-may-2018
 * Revision: 1		Based on ogrread.c MEX from Mirone 
 */

#include "gmt_gdalread.h"

struct OGR_FEATURES {
	int   att_number;
	char *name, *SRSWkt, *SRSProj4; 
	char *type;		    /* Geometry type. E.g. Point, Polygon or LineString */
	const char **att_names;	/* Names of the attributes of a Feature */
	const char **att_values;	/* Values of the attributes of a Feature as strings */
	int  *att_types;
	int  *islands;
	double BoundingBox[4];
	double *BB_geom;    /* Not currently assigned (would be the BoundingBox of each individual geometry) */
	double *x, *y, *z;
};

GMT_LOCAL int get_data(struct GMT_CTRL *GMT, struct OGR_FEATURES *out, OGRFeatureH hFeature,
                       OGRFeatureDefnH hFeatureDefn, OGRGeometryH hGeom, int iLayer, int nFeature, int nLayers,
					   int nAttribs, int nMaxGeoms, int recursionLevel) {

	int	is3D, i, j, jj, k, np = 0, nPtsBase, nRings = 0, indStruct, nGeoms, do_recursion;
	int *ptr_i = NULL;
	double	*x = NULL, *y = NULL, *z = NULL;
	double nan = NAN;
	OGRwkbGeometryType eType;
	OGRGeometryH hRing = NULL, hRingIsland = NULL;
	OGRFieldDefnH hField;

	is3D = (OGR_G_GetCoordinateDimension(hGeom) > 2);
	eType = wkbFlatten(OGR_G_GetGeometryType(hGeom));

	/* Find if we are going to do recursive calls */
	do_recursion = (eType == wkbGeometryCollection || eType == wkbMultiPolygon ||
	                eType == wkbMultiLineString || eType == wkbMultiPoint);

	if (!do_recursion) {	/* Multis are break up into pieces by recursive calls, so no need to alloc anything */
		if (eType == wkbPoint || eType == wkbLineString) {
			np = OGR_G_GetPointCount(hGeom);
		}
		else {
			hRing = OGR_G_GetGeometryRef(hGeom, 0);
			np = OGR_G_GetPointCount(hRing);
			nRings = OGR_G_GetGeometryCount(hGeom);
			for (i = 1; i < nRings; i++) {		/* If we have islands (interior rings) */
				hRingIsland = OGR_G_GetGeometryRef(hGeom, i);
				np += OGR_G_GetPointCount(hRingIsland);
			}
			np += (nRings - 1);			/* To account for NaNs separating islands from outer ring */
		}

		x = malloc(np * sizeof(double));
		y = malloc(np * sizeof(double));
		if (is3D) z = malloc(np * sizeof(double));
	}

	nGeoms = OGR_G_GetGeometryCount(hGeom);
	if ((nGeoms == 0) && (eType == wkbPoint || eType == wkbLineString))	/* These geometries will silently return 0 */
		nGeoms = 1;
	else if ((nGeoms > 1) && (eType == wkbPolygon))	/* Other geometries are Islands and those are dealt separately */
		nGeoms = 1;
	else if (nGeoms == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Screammm: No Geometries in this Feature\n");
		return(-1);
	}

	for (j = 0; j < nGeoms; j++) {		/* Loop over the number of geometries in this feature */

		jj = ((eType == wkbPolygon) ? 0 : j) + recursionLevel;	/* Islands don't count for matrix index (they are appended) */
		indStruct = (jj + nFeature * nMaxGeoms) * nLayers;

		if (eType == wkbPoint || eType == wkbLineString) {
			for (i = 0; i < np; i++) {
				x[i] = OGR_G_GetX(hGeom, i);
				y[i] = OGR_G_GetY(hGeom, i);
			}
			if (is3D) {
				for (i = 0; i < np; i++)
					z[i] = OGR_G_GetZ(hGeom, i);
			}
			if (eType == wkbPoint)
				out[indStruct].type = strdup("Point");
			else
				out[indStruct].type = strdup("LineString");
	
		}
		else if (eType == wkbPolygon) {
			nPtsBase = OGR_G_GetPointCount(hRing);	/* Need to ask it again because prev value may have eventual islands*/
			for (i = 0; i < nPtsBase; i++) {
				x[i] = OGR_G_GetX(hRing, i);
				y[i] = OGR_G_GetY(hRing, i);
			}
			if (is3D) {
				for (i = 0; i < nPtsBase; i++)
					z[i] = OGR_G_GetZ(hRing, i);
			}

			if (nRings > 1) {		/* Deal with the Islands */
				int	c, cz, nPtsRing, *pi;
				c = nPtsBase;
				pi = malloc(nRings * 2 * sizeof(int));
				pi[0] = 0;
				for (k = 1; k < nRings; k++) {			/* Loop over islands (interior rings) */
					hRingIsland = OGR_G_GetGeometryRef(hGeom, k);
					nPtsRing = OGR_G_GetPointCount(hRingIsland);
					x[c] = y[c] = nan;
					if (is3D) z[c] = nan;
					c++;	cz = c;
					pi[k] = c;
					for (i = 0; i < nPtsRing; c++, i++) {	/* Loop over number of points in this island */
						x[c] = OGR_G_GetX(hRingIsland, i);
						y[c] = OGR_G_GetY(hRingIsland, i);
					}
					if (is3D) {
						for (i = 0; i < nPtsRing; cz++, i++)
							z[cz] = OGR_G_GetZ(hRingIsland, i);
					}
				}
				/* We still have to fill the second column of Islands, but only now we have the necessary info */
				for (k = 0; k < nRings - 1; k++)
					pi[nRings + k] = pi[k+1] - 2;

				pi[2*nRings - 1] = c - 1;		/* Last element was not assigned in the loop above */
				out[indStruct].islands = pi;
			}
			out[indStruct].type = strdup("Polygon");
		}
		else if (do_recursion) {
			/* When we reach here it's because the current Geometry is of the Multi<something> type.
			 * The way we deal with it is to decompose it in its individual simple geometries, e.g.
			 * Polygon and call this function recursively until all basic geometries, controlled by
			 * the main for loop above [for (j = 0; j < nGeoms; j++)], are processed. */
			int	r;
	
			hRing = OGR_G_GetGeometryRef(hGeom, j);
			r = get_data(out, hFeature, hFeatureDefn, hRing, iLayer, nFeature, nLayers, nAttribs, nMaxGeoms, j);
			if (r)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to get data from element of a Multi<something>\n");
			continue;	/* We are done here */
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unforeseen case -> unknown geometry type\n");
			return(-1);
		}

		out[indStruct].x = x;
		out[indStruct].y = y;
		if (is3D) out[indStruct].z = z;

		if ((j + recursionLevel) == 0) {
			/* Only first column element is set with number of attributes (also called fields by ogr) */
			out[indStruct].att_number = nAttribs;

			out[indStruct].att_names  = (const char **)calloc((size_t)nAttribs, sizeof(char *));
			out[indStruct].att_values = (const char **)calloc((size_t)nAttribs, sizeof(char *));
			ptr_i = malloc(nAttribs * sizeof(int));
			for (i = 0; i < nAttribs; i++) {
				hField  = OGR_FD_GetFieldDefn(hFeatureDefn, i);
				out[indStruct].att_names[i]  = strdup(OGR_Fld_GetNameRef(hField));
				out[indStruct].att_values[i] = strdup(OGR_F_GetFieldAsString(hFeature, i));
				ptr_i[i] = OGR_Fld_GetType(hField);
			}
			out[indStruct].att_types = ptr_i;
		}
		else
			out[indStruct].att_number = 0;
	}

	return(0);
}

struct OGR_FEATURES *gmt_ogrread(struct GMT_CTRL *GMT, char *ogr_filename) {

	int	i, j, ind, iLayer, nEmptyLayers, nEmptyGeoms, nAttribs = 0;
	int	region = 0, verbose = 1;
	int	*layers;		/* Array with layer numbers*/
	int	nLayers;		/* number of layers in dataset */
	double	x_min, y_min, x_max, y_max;

	int	nFeature, nMaxFeatures, nMaxGeoms;
	struct OGR_FEATURES *out = NULL;

	GDALDatasetH hDS;
	OGRLayerH hLayer;
	OGRFeatureH hFeature;
	OGRFeatureDefnH hFeatureDefn;
	OGRGeometryH hGeom, hPolygon, poSpatialFilter = NULL;
	OGRSpatialReferenceH hSRS;
	OGREnvelope sEnvelop;
	OGRwkbGeometryType eType;

	x_min = y_min = x_max = y_max = 0.0;

	GDALAllRegister();

	hDS = GDALOpenEx(ogr_filename, GDAL_OF_VECTOR, NULL, NULL, NULL );
	if (hDS == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to open data source <%s>\n", ogr_filename);
		return NULL;
	}

	nLayers = OGR_DS_GetLayerCount(hDS);	/* Get available layers */
	if (nLayers < 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No OGR layers available\n");
		return NULL;
	}

	if (region) {		/* If we have a sub-region request. */
		poSpatialFilter = OGR_G_CreateGeometry(wkbPolygon);
		hPolygon = OGR_G_CreateGeometry(wkbLinearRing);
		OGR_G_AddPoint(hPolygon, x_min, y_min, 0.0);
		OGR_G_AddPoint(hPolygon, x_min, y_max, 0.0);
		OGR_G_AddPoint(hPolygon, x_max, y_max, 0.0);
		OGR_G_AddPoint(hPolygon, x_max, y_min, 0.0);
		OGR_G_AddPoint(hPolygon, x_min, y_min, 0.0);
		OGR_G_AddGeometryDirectly(poSpatialFilter, hPolygon);
	}

	layers = (int *)malloc(nLayers * sizeof(int));

	/* Get MAX number of features of all layers */
	for (i = j = nEmptyLayers = 0, nMaxFeatures = nMaxGeoms = 1; i < nLayers; i++) {
		hLayer = OGR_DS_GetLayer(hDS, i);

		if (region) OGR_L_SetSpatialFilter(hLayer, poSpatialFilter);

		nMaxFeatures = MAX((int)OGR_L_GetFeatureCount(hLayer, 1), nMaxFeatures);
		OGR_L_ResetReading(hLayer);
		hFeature = OGR_L_GetNextFeature(hLayer);
		if (hFeature == NULL) {		/* Yes, this can happen. Probably on crapy files */
			nEmptyLayers++;
			continue;
		}
		layers[j++] = i;		/* Store indices of non-empty layers */
		hGeom = OGR_F_GetGeometryRef(hFeature);
		hFeatureDefn = OGR_L_GetLayerDefn(hLayer);
		eType = wkbFlatten(OGR_G_GetGeometryType(hGeom));
		if (eType != wkbPolygon)	/* For simple polygons, next would return only the number of interior rings */
			nMaxGeoms = MAX(OGR_G_GetGeometryCount(hGeom), nMaxGeoms);

		OGR_F_Destroy(hFeature);
	}
	if (nEmptyLayers) nLayers -= nEmptyLayers;

	out = gmt_M_memory (GMT, NULL, (size_t)nMaxGeoms * nMaxFeatures * nLayers, struct OGR_FEATURES *); 

	for (iLayer = nFeature = nEmptyGeoms = 0; iLayer < nLayers; iLayer++) {

		ind = nMaxGeoms * nMaxFeatures * iLayer;	/* n_columns * n_rows * n_layers */
		hLayer = OGR_DS_GetLayer(hDS, layers[iLayer]);
		OGR_L_ResetReading(hLayer);
		hFeatureDefn = OGR_L_GetLayerDefn(hLayer);

		out[ind].name = strdup((char *)OGR_FD_GetName(hFeatureDefn));
		hSRS = OGR_L_GetSpatialRef(hLayer);	/* Do not free it later */
		if (hSRS) {				/* Get Layer's SRS. */
			char *pszWKT = NULL, *pszProj4 = NULL;
			if (OSRExportToProj4(hSRS, &pszProj4) == OGRERR_NONE)
				out[ind].SRSProj4 = strdup(pszProj4);
			if (OSRExportToPrettyWkt(hSRS, &pszWKT, 1) == OGRERR_NONE)
				out[ind].SRSWkt = strdup(pszWKT);
		}

		/* Get this layer BoundingBox as two column vectors of X and Y respectively. */
		if ((OGR_L_GetExtent(hLayer, &sEnvelop, 1)) == OGRERR_NONE) {
			out[ind].BoundingBox[0] = sEnvelop.MinX;		out[ind].BoundingBox[1] = sEnvelop.MaxX;
			out[ind].BoundingBox[2] = sEnvelop.MinY;		out[ind].BoundingBox[3] = sEnvelop.MaxY;
		}
		else {
			out[ind].BoundingBox[0] = out[ind].BoundingBox[2] = -DBL_MAX;
			out[ind].BoundingBox[1] = out[ind].BoundingBox[3] =  DBL_MAX;
		}

		nAttribs = OGR_FD_GetFieldCount(hFeatureDefn);

		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Importing %lld features from layer <%s>\n",
		            OGR_L_GetFeatureCount(hLayer, 1), out[ind].name);

		while ((hFeature = OGR_L_GetNextFeature(hLayer)) != NULL) {
			hGeom = OGR_F_GetGeometryRef(hFeature);
			eType = wkbFlatten(OGR_G_GetGeometryType(hGeom));
			if (hGeom != NULL)
				get_data(GMT, out, hFeature, hFeatureDefn, hGeom, iLayer, nFeature, nLayers, nAttribs, nMaxGeoms, 0);
			else
				nEmptyGeoms++;
			nFeature++;		/* Counter to number of features in this layer */

			OGR_F_Destroy(hFeature);
		}

		if (nEmptyGeoms > 0)
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "There were %d empty geometries in this layer\n", nEmptyGeoms);
	}

	free(layers);
	GDALClose(hDS);
	return out;
}
