/*--------------------------------------------------------------------
 *
 *	Copyright (c) 2012-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * gmt_hidden.h contains the hidden support structures for the 5 GMT resources
 * GMT_DATASET, GMT_GRID, GMT_IMAGE, GMT_PALETTE, and GMT_POSTSCRIPT
 * as well as the two auxiliary resources GMT_MATRIX and GMT_VECTOR.
 *
 * Author:	Paul Wessel
 * Date:	27-OCT-2017
 * Version:	6 API
 */

/*!
 * \file gmt_resources.h
 * \brief Definitions for the GMT resources (GMT_GRID, GMT_DATASET, etc...)
 */

#ifndef GMT_HIDDEN_H
#define GMT_HIDDEN_H

GMT_LOCAL inline struct GMT_DATASET_HIDDEN     * gmt_get_DD_hidden (struct GMT_DATASET *p)     {return (p->hidden);}
GMT_LOCAL inline struct GMT_DATATABLE_HIDDEN   * gmt_get_DT_hidden (struct GMT_DATATABLE *p)   {return (p->hidden);}
GMT_LOCAL inline struct GMT_DATASEGMENT_HIDDEN * gmt_get_DS_hidden (struct GMT_DATASEGMENT *p) {return (p->hidden);}
GMT_LOCAL inline struct GMT_PALETTE_HIDDEN     * gmt_get_C_hidden (struct GMT_PALETTE *p)      {return (p->hidden);}
GMT_LOCAL inline struct GMT_POSTSCRIPT_HIDDEN  * gmt_get_P_hidden (struct GMT_POSTSCRIPT *p)   {return (p->hidden);}
GMT_LOCAL inline struct GMT_VECTOR_HIDDEN      * gmt_get_V_hidden (struct GMT_VECTOR *p)       {return (p->hidden);}
GMT_LOCAL inline struct GMT_MATRIX_HIDDEN      * gmt_get_M_hidden (struct GMT_MATRIX *p)       {return (p->hidden);}
GMT_LOCAL inline struct GMT_GRID_HIDDEN        * gmt_get_G_hidden (struct GMT_GRID *p)         {return (p->hidden);}
GMT_LOCAL inline struct GMT_IMAGE_HIDDEN       * gmt_get_I_hidden (struct GMT_IMAGE *p)        {return (p->hidden);}
GMT_LOCAL inline struct GMT_GRID_HEADER_HIDDEN * gmt_get_H_hidden (struct GMT_GRID_HEADER *p)  {return (p->hidden);}

/* Here are the GMT data types used for tables */

struct GMT_OGR {	/* Struct with all things GMT/OGR for a table */
	/* The first parameters are usually set once per data set and do not change */
	unsigned int geometry;		/* @G: The geometry of this data set, if known [0 otherwise] */
	unsigned int n_aspatial;	/* @T: The number of aspatial fields */
	unsigned int rec_type;		/* Overall record type of aspatial data (GMT_IS_DATA, GMT_IS_TEXT, or GMT_IS_MIXED) */
	char *region;			/* @R: The region textstring [NULL if not set] */
	char *proj[4];			/* @J: The 1-4 projection strings [NULL if not set] */
	enum GMT_enum_type *type;	/* @T: The data types of the aspatial fields [NULL if not set]  */
	char **name;			/* @N The names of the aspatial fields [NULL if not set]  */
	/* The following are for OGR data only. It is filled during parsing (current segment) but is then copied to the segment header so it can be accessed later */
	enum GMT_enum_pol pol_mode;	/* @P: Either GMT_IS_PERIMETER or GMT_IS_HOLE (for polygons only) */
	char **tvalue;			/* @D: The text values of the current aspatial fields */
	double *dvalue;			/* @D: Same but converted to double (assumed possible) */
};

struct GMT_OGR_SEG {	/* Struct with GMT/OGR aspatial data for a segment */
	enum GMT_enum_pol pol_mode;	/* @P: Either GMT_IS_PERIMETER or GMT_IS_HOLE (for polygons only) */
	unsigned int n_aspatial;	/* @T: The number of aspatial fields */
	char **tvalue;			/* @D: The values of the current aspatial fields (uses GMT_OGR's n_aspatial as length) */
	double *dvalue;			/* @D: Same but converted to double (assumed possible) */
};

struct GMT_DATASEGMENT_HIDDEN {    /* Supporting information hidden from the API */
	enum GMT_enum_write mode;	/* 0 = output segment, 1 = output header only, 2 = skip segment */
	enum GMT_enum_pol pol_mode;	/* Either GMT_IS_PERIMETER  [-Pp] or GMT_IS_HOLE [-Ph] (for polygons only) */
	uint64_t id;			/* The internal number of the segment */
	size_t n_alloc;			/* The current allocation length of rows */
	unsigned int range;		/* Longitude reporting scheme, e.g. GMT_IS_GIVEN_RANGE [0] */
	int pole;			/* Spherical polygons only: If it encloses the S (-1) or N (+1) pole, or none (0) */
	double dist;			/* Distance from a point to this feature */
	double lat_limit;		/* For polar caps: the latitude of the point closest to the pole */
	struct GMT_OGR_SEG *ogr;	/* NULL unless OGR/GMT metadata exist for this segment */
	struct GMT_DATASEGMENT *next;	/* NULL unless polygon and has holes and pointing to next hole */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
};

struct GMT_DATATABLE_HIDDEN {	/* Supporting information hidden from the API */
	uint64_t id;            /* The internal number of the table */
	size_t n_alloc;         /* The current allocation length of segments */
	double dist;            /* Distance from a point to this feature */
	enum GMT_enum_write mode;	/* 0 = output table, 1 = output header only, 2 = skip table */
	struct GMT_OGR *ogr;    /* Pointer to struct with all things GMT/OGR (if MULTI-geometry and not MULTIPOINT) */
	char *file[2];          /* Name of file or source [0 = in, 1 = out] */
};

struct GMT_DATASET_HIDDEN {	/* Supporting information hidden from the API */
	uint64_t id;			/* The internal number of the data set */
	size_t n_alloc;			/* The current allocation length of tables */
	uint64_t dim[4];		/* Only used by GMT_Duplicate_Data to override dimensions */
	unsigned int alloc_level;	/* The level it was allocated at */
	enum GMT_enum_write io_mode;	/* -1 means write OGR format (requires proper -a),
					 * 0 means write everything to one destination [Default],
					 * 1 means use table->file[GMT_OUT] to write separate table,
					 * 2 means use segment->file[GMT_OUT] to write separate segments.
					 * 3 is same as 2 but with no filenames we create filenames from tbl and seg numbers */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
};

struct GMT_PALETTE_HIDDEN {	/* Supporting information hidden from the API */
	uint64_t id;			/* The internal number of the data set */
	size_t n_alloc;            	/* Memory allocated so far */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
	unsigned int alloc_level;	/* The level it was allocated at */
	unsigned int auto_scale;	/* If 1 then we must resample to fit actual data range */
	unsigned int skip;		/* true if current z-slice is to be skipped */
	unsigned int z_adjust[2];	/* 1 if +u<unit> was parsed and scale set, 3 if z has been adjusted, 0 otherwise */
	unsigned int z_mode[2];	 	/* 1 if +U<unit> was parsed, 0 otherwise */
	unsigned int z_unit[2];	 	/* Unit enum specified via +u<unit> */
	double z_unit_to_meter[2];	/* Scale, given z_unit, to convert z from <unit> to meters */
};

struct GMT_POSTSCRIPT_HIDDEN {	/* Supporting information hidden from the API */
	uint64_t id;			/* The internal number of the data set */
	size_t n_alloc;			/* Memory allocated so far */
	unsigned int alloc_level;	/* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
};

struct GMT_VECTOR_HIDDEN {	/* Supporting information hidden from the API */
	uint64_t id;			/* The internal number of the data set */
	unsigned int alloc_level;	/* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
};

struct GMT_MATRIX_HIDDEN {	/* Supporting information hidden from the API */
	uint64_t id;			/* The internal number of the data set */
	int pad;			/* The internal number of the data set */
	unsigned int alloc_level;	/* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
};

struct GMT_GRID_HEADER_HIDDEN {
	/* ---- Variables "hidden" from the API ----
	 * This section is flexible.  It is not copied to any grid header
	 * or stored in any file.  It is considered private */
	unsigned int trendmode;          /* Holds status for detrending of grids.  0 if not detrended, 1 if mean, 2 if mid-value, and 3 if LS plane removed */
	unsigned int arrangement;        /* Holds status for complex grid as how the read/imag is placed in the grid (interleaved, R only, etc.) */
	unsigned int BC[4];              /* Boundary condition applied on each side via pad [0 = not set, 1 = natural, 2 = periodic, 3 = data] */
	unsigned int grdtype;            /* 0 for Cartesian, > 0 for geographic and depends on 360 periodicity [see GMT_enum_grdtype in gmt_grd.h] */
	unsigned int reset_pad;          /* true in cases where we need a subset from a memory grid and must compute node index separately */
	char name[GMT_GRID_NAME_LEN256]; /* Actual name of the file after any ?<varname> and =<stuff> has been removed */
	char varname[GMT_GRID_VARNAME_LEN80];/* NetCDF: variable name */
	int row_order;                   /* NetCDF: k_nc_start_south if S->N, k_nc_start_north if N->S */
	int z_id;                        /* NetCDF: id of z field */
	int ncid;                        /* NetCDF: file ID */
	int xy_dim[2];                   /* NetCDF: dimension order of x and y; normally {1, 0} */
	size_t t_index[3];               /* NetCDF: index of higher coordinates */
	size_t data_offset;              /* NetCDF: distance from the beginning of the in-memory grid */
	size_t n_alloc;                  /* Bytes allocated for this grid */
	unsigned int stride;             /* NetCDF: distance between two rows in the in-memory grid */
	double r_inc[2];                 /* Reciprocal incs, i.e. 1/inc */
	char   flags[4];                 /* Flags used for ESRI grids */
	char  *pocket;                   /* GDAL: A working variable handy to transmit info between funcs e.g. +b<band_info> to gdalread */
	double bcr_threshold;            /* sum of cardinals must >= threshold in bilinear; else NaN */
	unsigned int has_NaNs;           /* Is 2 if the grid contains any NaNs, 1 if it does not, and 0 if no check has yet happened */
	unsigned int bcr_interpolant;    /* Interpolation function used (0, 1, 2, 3) */
	unsigned int bcr_n;              /* Width of the interpolation function */
	unsigned int nxp;                /* if X periodic, nxp > 0 is the period in pixels  */
	unsigned int nyp;                /* if Y periodic, nxp > 0 is the period in pixels  */
	unsigned int no_BC;              /* If true we skip BC stuff entirely */
	unsigned int gn;                 /* true if top    edge will be set as N pole  */
	unsigned int gs;                 /* true if bottom edge will be set as S pole  */
	unsigned int is_netcdf4;         /* true if netCDF-4/HDF5 format */
	enum GMT_enum_type orig_datatype; /* GMT_FLOAT, GMT_SHORT, etc how the source grid was represented */
	size_t z_chunksize[2];           /* chunk size (lat,lon) */
	unsigned int z_scale_given;	 /* 1 if +s was specified */
	unsigned int z_offset_given;	 /* 1 if +o was specified */
	unsigned int z_shuffle;          /* if shuffle filter is turned on */
	unsigned int z_deflate_level;    /* if deflate filter is in use */
	unsigned int z_scale_autoadjust; /* if z_scale_factor should be auto-detected */
	unsigned int z_offset_autoadjust;/* if z_add_offset should be auto-detected */
					 /* xy_*[] is separate settings for GMT_IN and GMT_OUT */
	unsigned int xy_adjust[2];	 /* 1 if +u<unit> was parsed and scale set, 3 if xy has been adjusted, 0 otherwise */
	unsigned int xy_mode[2];	 /* 1 if +U<unit> was parsed, 0 otherwise */
	unsigned int xy_unit[2];	 /* Unit enum specified via +u<unit> */
	double xy_unit_to_meter[2];	 /* Scale, given xy_unit, to convert xy from <unit> to meters */
	uint64_t (*index_function) (struct GMT_GRID_HEADER *, uint64_t, uint64_t, uint64_t);	/* Pointer to index function (for images only) */
};

struct GMT_GRID_HIDDEN {	/* Supporting information hidden from the API */
	unsigned int id;                /* The internal number of the grid */
	unsigned int alloc_level;       /* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode; /* Allocation mode [GMT_ALLOC_INTERNALLY] */
	void *extra;                    /* Row-by-row machinery information [NULL] */
};

struct GMT_IMAGE_HIDDEN {	/* Supporting information hidden from the API */
	uint64_t id;                    /* The internal number of the data set */
	unsigned int alloc_level;       /* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
};

/* Get the segments next segment (for holes in perimeters */
GMT_LOCAL inline struct GMT_DATASEGMENT * gmt_get_next_S (struct GMT_DATASEGMENT *S) {struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S); return (SH->next);}

#endif /* GMT_HIDDEN_H */
