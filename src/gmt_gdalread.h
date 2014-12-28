/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#include <gdal.h>
#include <ogr_srs_api.h>
#include <cpl_string.h>
#include <cpl_conv.h>

/* Structure to control which options are transmited to GMT_gdalwrite */
struct GDALWRITE_CTRL {
	char *driver;		/* The GDAL diver name */
	char *type;			/* Data type */
	char *command;		/* command line */
	char *title;
	char *remark;
	int  geog;
	int  nx, ny;		/* Number of columns & rows of the region to be saved */
	int  nXSizeFull;	/* Total number of columns of the data array including padding */
	int  n_bands;
	int  pad[4];
	int  flipud;
	int  registration;		/* Registration type. 0 -> grid registration; 1 -> pixel reg */
	double	ULx, ULy;		/* x_min & y_max */
	double	x_inc, y_inc;	/* Grid/Image increments */
	double	nan_value; /* unlike the nan_value in struct GMT_GRID_HEADER this one is of type double */
	void	*data;
	struct GW_C {	/* Color map */
		int active;
		int n_colors;
		float *cpt;
	} C;
	struct GW_P {			/* Proj4 string */
		int	active;
		char	*ProjectionRefPROJ4;
	} P;
};

/* Structure to control which options are transmited to GMT_gdalread */
struct GDALREAD_CTRL {
	struct GD_B {	/* Band selection */
		int active;
		char *bands;
	} B;
	struct GD_I {	/* Interleaving by pixel (only for char data) */
		int active;
	} I;
	struct GD_L {	/* Left-Right flip */
		int active;
	} L;
	struct GD_M {	/* Metadata only */
		int active;
	} M;
	struct GD_P {	/* Preview mode */
		int active;
		char *jump;
	} P;
	struct GD_p {	/* Pad array in output */
		int active;
		int pad;
	} p;
	struct GD_W {	/* Convert proj4 string into WKT */
		int active;
	} W;
	struct GD_R {	/* Sub-region in referenced coords */
		int active;
		char *region;
	} R;
	struct GD_Z {	/* Tell to store data in a complex array */
		int active;
		int complex_mode; /* 1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	} Z;
	struct GD_cp {	/* Send in a pointer with allocated chars */
		int active;
		unsigned char *grd;
	} c_ptr;
	struct GD_fp {	/* Send in a pointer with allocated floats */
		int active;
		float *grd;
	} f_ptr;
	struct GD_r {	/* Sub-region in row/column coords */
		int active;
		char *region;
	} r;
	struct GD_reg {	/* Registration type. Used only when sending a sub-region request. Than we need to know this */
		double x_inc, y_inc;	/* Grid increments */
		int val;	/* 0 [default] means grid registration, 1 -> pixel registration */
	} registration;
	struct GD_hdr {	/* Some fields of the header structure */
		int active;
		unsigned int mx, my;
		char side[1];		/* If array is going to pasted (grdpaste), tell in what side 'lrtb' */
		int offset;
	} mini_hdr;
};

/* Structure to hold metadata info in a per bands basis read */
struct GDAL_BAND_FNAMES {
	char		*DataType;
	int	XSize;
	int	YSize;
	double		nodata;
	double		MinMax[2];
	double		ScaleOffset[2];
};

/* Structure with the output data transmited by GMT_gdalread */
struct GD_CTRL {
	/* active is true if the option has been activated */
	struct UInt8 {			/* Declare byte pointer */
		int active;
		unsigned char *data;
	} UInt8;
	struct UInt16 {			/* Declare short int pointers */
		int active;
		unsigned short int *data;
	} UInt16;
	struct Int16 {			/* Declare unsigned short int pointers */
		int active;
		short int *data;
	} Int16;
	struct UInt32 {			/* Declare unsigned int pointers */
		int active;
		int *data;
	} UInt32;
	struct Int32 {			/* Declare int pointers */
		int active;
		int *data;
	} Int32;
	struct Float {			/* Declare float pointers */
		int active;
		float *data;
	} Float;
	struct Double {			/* Declare double pointers */
		int active;
		double *data;
	} Double;

	double	hdr[9];
	double	GeoTransform[6];
	double	nodata;
	char	*ProjectionRefPROJ4;
	char	*ProjectionRefWKT;
	const char	*DriverShortName;
	const char	*DriverLongName;
	const char	*ColorInterp;
	int	*ColorMap;
	int	RasterXsize;
	int	RasterYsize;
	int	RasterCount;	/* Total number of bands in file */
	int	nActualBands;	/* Number of bands that were actually sent back */
	struct Corners {
		double LL[2], UL[2], UR[2], LR[2];
	} Corners;
	struct GEOGCorners {
		double LL[2], UL[2], UR[2], LR[2];
	} GEOGCorners;

	struct GDAL_BAND_FNAMES *band_field_names;
};
