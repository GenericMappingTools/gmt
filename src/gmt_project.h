/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Include file for programs that use the map-projections functions.  Note
 * that most programs will include this by including gmt_dev.h
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 */

/*!
 * \file gmt_project.h
 * \brief Include file for programs that use the map-projections functions.
 */

#ifndef GMT_PROJECT_H
#define GMT_PROJECT_H

#define HALF_DBL_MAX (DBL_MAX/2.0)

/*! GMT_180 is used to see if a value really is exceeding it (beyond roundoff) */
#define GMT_180	(180.0 + GMT_CONV8_LIMIT)
/*! gmt_M_wind_lon will remove central meridian value and adjust so lon fits between -180/+180 */
#define gmt_M_wind_lon(C,lon) {lon -= C->current.proj.central_meridian; while (lon < -GMT_180) lon += 360.0; while (lon > +GMT_180) lon -= 360.0;}

/*! Some shorthand notation for GMT specific cases */
EXTERN_MSC double gmtmap_lat_swap_quick (struct GMT_CTRL *GMT, double lat, double c[]);

#define gmt_M_latg_to_latc(C,lat) gmtmap_lat_swap_quick (C, lat, C->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2C])
#define gmt_M_latg_to_lata(C,lat) gmtmap_lat_swap_quick (C, lat, C->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2A])
#define gmt_M_latc_to_latg(C,lat) gmtmap_lat_swap_quick (C, lat, C->current.proj.lat_swap_vals.c[GMT_LATSWAP_C2G])
#define gmt_M_lata_to_latg(C,lat) gmtmap_lat_swap_quick (C, lat, C->current.proj.lat_swap_vals.c[GMT_LATSWAP_A2G])

/*! Macros returns true if the two coordinates are lon/lat; way should be GMT_IN or GMT_OUT */
#define gmt_M_x_is_lon(C,way) (C->current.io.col_type[way][GMT_X] & GMT_IS_LON)
#define gmt_M_y_is_lon(C,way) (C->current.io.col_type[way][GMT_Y] & GMT_IS_LON)
#define gmt_M_x_is_lat(C,way) (C->current.io.col_type[way][GMT_X] & GMT_IS_LAT)
#define gmt_M_y_is_lat(C,way) (C->current.io.col_type[way][GMT_Y] & GMT_IS_LAT)
#define gmt_M_is_geographic(C,way) (gmt_M_x_is_lon(C,way) && gmt_M_y_is_lat(C,way))
#define gmt_M_is_cartesian(C,way) (!gmt_M_is_geographic(C,way))

#define GMT_N_PROJECTIONS	29	/* Total number of projections in GMT */

/* These numbers should remain flexible. Do not use them in any programming. Use only their symbolic names.
   However, all the first items in each section (i.e. GMT_LINEAR, GMT_MERCATOR,...) should remain the first.
*/
#define GMT_NO_PROJ		-1	/* Projection not specified (initial value) */

/*! Linear projections tagged 0-99 */
#define gmt_M_is_linear(C) (C->current.proj.projection / 100 == 0)

enum gmt_enum_annot {GMT_LINEAR = 0,
	GMT_LOG10,	/* These numbers are only used for GMT->current.proj.xyz_projection[3], */
	GMT_POW,	/* while GMT->current.proj.projection = 0 */
	GMT_TIME,
	GMT_ANNOT_CPT,
	GMT_CUSTOM};

#define GMT_ZAXIS		50

/*! Cylindrical projections tagged 100-199 */
#define gmt_M_is_cylindrical(C) (C->current.proj.projection / 100 == 1)

enum gmt_enum_cyl {GMT_MERCATOR = 100,
	GMT_CYL_EQ,
	GMT_CYL_EQDIST,
	GMT_CYL_STEREO,
	GMT_MILLER,
	GMT_TM,
	GMT_UTM,
	GMT_CASSINI,
	GMT_OBLIQUE_MERC = 150,
	GMT_OBLIQUE_MERC_POLE};

/*! Conic projections tagged 200-299 */
#define gmt_M_is_conical(C) (C->current.proj.projection / 100 == 2)

enum gmt_enum_conic {GMT_ALBERS = 200,
	GMT_ECONIC,
	GMT_POLYCONIC,
	GMT_LAMBERT = 250};

/* Azimuthal projections tagged 300-399 */
#define gmt_M_is_azimuthal(C) (C->current.proj.projection / 100 == 3)
#define gmt_M_is_perspective(C) (C->current.proj.projection == GMT_ORTHO || C->current.proj.projection == GMT_GENPER)
enum gmt_enum_azim {GMT_STEREO = 300,
	GMT_LAMB_AZ_EQ,
	GMT_ORTHO,
	GMT_AZ_EQDIST,
	GMT_GNOMONIC,
	GMT_GENPER,
	GMT_POLAR = 350};

/* Misc projections tagged 400-499 */
#define gmt_M_is_misc(C) (C->current.proj.projection / 100 == 4)
enum gmt_enum_misc {GMT_MOLLWEIDE = 400,
	GMT_HAMMER,
	GMT_SINUSOIDAL,
	GMT_VANGRINTEN,
	GMT_ROBINSON,
	GMT_ECKERT4,
	GMT_ECKERT6,
	GMT_WINKEL};

/* All projections from proj.4 lib */
enum gmt_enum_allprojs {GMT_PROJ4_PROJS = 500};

/*! The various GMT measurement units */
enum gmt_enum_units {GMT_IS_METER = 0,
	GMT_IS_KM,
	GMT_IS_MILE,
	GMT_IS_NAUTICAL_MILE,
	GMT_IS_INCH,
	GMT_IS_CM,
	GMT_IS_PT,
	GMT_IS_FOOT,
	GMT_IS_SURVEY_FOOT,
	GMT_N_UNITS,
	GMT_IS_NOUNIT = -1};

/* gmt_M_is_rect_graticule means parallels and meridians are orthogonal, but does not imply linear spacing */
#define gmt_M_is_rect_graticule(C) (C->current.proj.projection <= GMT_MILLER)

/* gmt_M_is_nonlinear_graticule means parallels and meridians are not orthogonal or have nonlinear spacing */
#define gmt_M_is_nonlinear_graticule(C)	(!(C->current.proj.projection == GMT_CYL_EQDIST || C->current.proj.projection == GMT_LINEAR) || \
	C->current.proj.xyz_projection[GMT_X] == GMT_LOG10 || C->current.proj.xyz_projection[GMT_X] == GMT_POW || \
	C->current.proj.xyz_projection[GMT_Y] == GMT_LOG10 || C->current.proj.xyz_projection[GMT_Y] == GMT_POW)

#define gmt_M_is_spherical(C) (C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].flattening < 1.0e-10)
#define gmt_M_is_flatearth(C) (!strcmp (C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].name, "FlatEarth"))

/* Return 0 for Flat Earth, 1 for Great-circles, 2 for geodesics, and 3 for loxodromes */
#define gmt_M_sph_mode(C) (gmt_M_is_flatearth (C) ? GMT_FLATEARTH : (gmt_M_is_spherical (C) ? GMT_GREATCIRCLE : (C->current.map.loxodrome ? GMT_LOXODROME : GMT_GEODESIC)))

//#define gmt_M_360_range(w,e) (doubleAlmostEqual (fabs((e) - (w)), 360.0))	/* PW: Reconsider this later perhaps but for now too tight [see issue #954] */
#define gmt_M_180_range(s,n) (doubleAlmostEqual (fabs((n) - (s)), 180.0))
#define gmt_M_360_range(w,e) (gmt_M_is_zero (fabs ((e) - (w)) - 360.0))
#define gmt_M_is_pole(y) (doubleAlmostEqual (fabs(y), 90.0))
#define gmt_M_is_Npole(y) (gmt_M_is_zero(y-90.0))
#define gmt_M_is_Spole(y) (gmt_M_is_zero(y+90.0))
#define gmt_M_is_zero(x) (fabs (x) < GMT_CONV8_LIMIT)

#ifndef D2R
#define D2R (M_PI / 180.0)
#endif
#ifndef R2D
#define R2D (180.0 / M_PI)
#endif

/* UTM offsets */

#define GMT_FALSE_EASTING    500000.0
#define GMT_FALSE_NORTHING 10000000.0

/* Number of proj4 look-ups */

#define GMT_N_PROJ4 31

/* Number of nodes in Robinson interpolation */

#define GMT_N_ROBINSON	19

struct GMT_LATSWAP_CONSTS {
	double  c[GMT_LATSWAP_N][4];	/* Coefficients in 4-term series  */
	double	ra;			/* Authalic   radius (sphere for equal-area)  */
	double	rm;			/* Meridional radius (sphere for N-S distance)  */
	bool spherical;			/* True if no conversions need to be done.  */
};

struct GMT_THREE_D {
	double view_azimuth, view_elevation;
	double cos_az, sin_az, cos_el, sin_el;
	double corner_x[4], corner_y[4];
	double xmin, xmax, ymin, ymax;
	double world_x, world_y, world_z;	/* Users coordinates of fixed point */
	double view_x, view_y;			/* Desired projected 2-D coordinates of fixed point */
	double x_off, y_off;			/* Offsets to the final projected coordinates */
	double sign[4];		/* Used to determine direction of tickmarks etc */
	double level;		/* Indicates the last level of the perspective plane (if any) */
	unsigned int view_plane;	/* Determines on which plane needs to be projected */
	int plane;		/* Indicates which last plane was plotted in perspective (-1 = none) */
	unsigned int quadrant;	/* quadrant we're looking from */
	unsigned int z_axis;	/* Which z-axis to draw. */
	unsigned int face[3];	/* Tells if this facet has normal in pos direction */
	bool draw[4];		/* axes to draw */
	bool fixed;		/* true if we want a given point to be fixed in the projection [for animations] */
	bool world_given;	/* true if a fixed world point was given in -E ..+glon/lat/z */
	bool view_given;	/* true if a fixed projected point was given in -E ..+cx0/y0 */
};

struct GMT_DATUM {	/* Main parameter for a particular datum */
	double a, b, f, e_squared, ep_squared;
	double xyz[7];
	int ellipsoid_id;	/* Ellipsoid GMT ID number (or -1) */
};

struct GMT_DATUM_CONV {
	bool h_given;		/* true if we have incoming height data [h = 0] */
	double da;		/* Major semi-axis in meters */
	double df;		/* Flattening */
	double e_squared;	/* Eccentricity squared (e^2 = 2*f - f*f) */
	double one_minus_f;	/* 1 - f */
	double dxyz[3];		/* Ellipsoids offset in meter from Earth's center of mass for x,y, and z */
	double bursa[7];	/* Bursa-Wolf seven parameters */
	struct GMT_DATUM from, to;	/* The old and new datums */
};

struct GMT_PROJ4 {	/* Used to assign proj4 projections from GMT projections */
	char *name;
	unsigned int id;
};

struct GMT_PROJ {

	struct GMT_THREE_D z_project;
	struct GMT_DATUM_CONV datum;	/* For datum conversions */
	struct GMT_PROJ4 *proj4;	/* A read-only resource we allocate once and pass pointer around */
	void (*fwd) (struct GMT_CTRL *, double, double, double *, double *);/* Pointers to the selected forward mapping function */
	void (*inv) (struct GMT_CTRL *, double *, double *, double, double);/* Pointers to the selected inverse mapping function */
	void (*fwd_x) (struct GMT_CTRL *, double, double *);	/* Pointers to the selected linear x forward function */
	void (*fwd_y) (struct GMT_CTRL *, double, double *);	/* Pointers to the selected linear y forward function */
	void (*fwd_z) (struct GMT_CTRL *, double, double *);	/* Pointers to the selected linear z forward function */
	void (*inv_x) (struct GMT_CTRL *, double *, double);	/* Pointers to the selected linear x inverse function */
	void (*inv_y) (struct GMT_CTRL *, double *, double);	/* Pointers to the selected linear y inverse function */
	void (*inv_z) (struct GMT_CTRL *, double *, double);	/* Pointers to the selected linear z inverse function */

	double pars[16];		/* Raw unprocessed map-projection parameters as passed on command line */
	double z_pars[2];		/* Raw unprocessed z-projection parameters as passed on command line */

	/* Common projection parameters */

	int projection;		/* Gives the id number for the projection used (-1 if not set) */
	int projection_GMT;	/* A copy of the above to use when using the Proj.4 lib for coord/map transforms */
	int autoscl[2];		/* If +1 then set x/y-scale to y/x-scale and compute height/width, -1 reverses direction */
	int corner[4];		/* In/on/out status of map corners w.r.t. current polygon for -R...+r regions */

	bool search;		/* true if we are allowed to search along oblique boundary for min/max lon/lats */
	bool units_pr_degree;	/* true if scale is given as inch (or cm)/degree.  false for 1:xxxxx */
	bool north_pole;		/* true if projection is on northern hemisphere, false on southern */
	bool edge[4];		/* true if the edge is a map boundary */
	bool three_D;		/* Parameters for 3-D projections */
	bool JZ_set;		/* true if -Jz|Z was set */
	bool GMT_convert_latitudes;	/* true if using spherical code with authalic/conformal latitudes */
	bool inv_coordinates;	/* true if -fp[unit] was given and we must first recover lon,lat during reading */
	bool N_hemi;		/* true if we only allow northern hemisphere oblique Mercator poles */
	bool sph_inside;	/* true is we must use the spherical in-on-out machinery [false = Cartesian] */
	unsigned int n_antipoles;	/* Number of antipole coordinates so far [used for -JE only] */
	struct GMT_LATSWAP_CONSTS lat_swap_vals;

	enum gmt_enum_units inv_coord_unit;		/* Index to scale that converts input map coordinates to meter before inverting for lon,lat */
	char unit_name[GMT_N_UNITS][GMT_LEN16];	/* Names of the various distance units */
	double m_per_unit[GMT_N_UNITS];	/* Meters in various units.  Use to scale units to meters */
	double origin[3];		/* Projected values of the logical origin for the projection (x, y, z) */
	double rect[4], zmin, zmax;	/* Extreme projected values */
	double rect_m[4];		/* Extreme projected original meter values */
	double scale[3];		/* Scaling for meters to map-distance (typically inch) conversion (x, y, z) */
	double i_scale[3];		/* Inverse Scaling for meters to map-distance (typically inch) conversion (x, y, z) */
	double z_level;			/* Level at which to draw basemap [0] */
	double unit;			/* Gives meters pr plot unit (0.01 or 0.0254) */
	double central_meridian;	/* Central meridian for projection [NaN] */
	double lon0, lat0;		/* Projection center [NaN/NaN if not specified in -J] */
	double pole;			/* +90 pr -90, depending on which pole */
	double mean_radius;		/* Mean radius given the PROJ_* settings */
	double EQ_RAD, i_EQ_RAD;	/* Current ellipsoid parameters */
	double ECC, ECC2, ECC4, ECC6;	/* Powers of eccentricity */
	double M_PR_DEG, KM_PR_DEG;	/* Current spherical approximations to convert degrees to dist */
	double DIST_M_PR_DEG;		/* Current spherical approximations to convert degrees to m even if -J was not set */
	double DIST_KM_PR_DEG;		/* Current spherical approximations to convert degrees to km even if -J was not set */
	double half_ECC, i_half_ECC;	/* 0.5 * ECC and 0.5 / ECC */
	double one_m_ECC2, i_one_m_ECC2; /* 1.0 - ECC2 and inverse */
	unsigned int gave_map_width;	/* nonzero if map width (1), height (2), max dim (3) or min dim (4) is given instead of scale.  0 for 1:xxxxx */

	uint64_t n_geodesic_calls;	/* Number of calls for geodesics in this session */
	uint64_t n_geodesic_approx;	/* Number of calls for geodesics in this session that exceeded iteration limit */

	double f_horizon, rho_max;	/* Azimuthal horizon (deg) and in plot coordinates */

	/* Linear plot parameters */

	unsigned int xyz_projection[3];	/* For linear projection, 0 = linear, 1 = log10, 2 = pow */
	bool xyz_pos[3];		/* true if x,y,z-axis increases in normal positive direction */
	bool compute_scale[3];	/* true if axes lengths were set rather than scales */
	double xyz_pow[3];		/* For GMT_POW projection */
	double xyz_ipow[3];

	/* Center of radii for all conic projections */

	double c_x0, c_y0;

	/* Lambert conformal conic parameters. */

	double l_N, l_i_N, l_Nr, l_i_Nr;
	double l_F, l_rF, l_i_rF;
	double l_rho0;

	/* Oblique Mercator Projection (Spherical version )*/

	double o_sin_pole_lat, o_cos_pole_lat;	/* Pole of rotation */
	double o_pole_lon, o_pole_lat;	/* In degrees */
	double o_beta;			/* lon' = beta for central_meridian (degrees) */
	double o_shift;			/* Projected distance between oblique equator and chosen oblique latitude */
	bool o_spole;			/* True if pole is in the southern hemisphere */
	double o_FP[3], o_FC[3], o_IP[3], o_IC[3];

	/* TM and UTM Projections */

	double t_lat0;
	double t_e2, t_M0;
	double t_c1, t_c2, t_c3, t_c4;
	double t_i1, t_i2, t_i3, t_i4, t_i5;
	double t_r, t_ir;		/* Short for GMT->current.proj.EQ_RAD * GMT->current.setting.proj_scale_factor and its inverse */
	int utm_hemisphere;	/* -1 for S, +1 for N, 0 if to be set by -R */
	unsigned int utm_zonex;	/* The longitude component 1-60 */
	char utm_zoney;			/* The latitude component A-Z */

	/* Lambert Azimuthal Equal-Area Projection */

	double sinp;
	double cosp;
	double Dx, Dy, iDx, iDy;	/* Fudge factors for projections w/authalic lats */

	/* Stereographic Projection */

	double s_c, s_ic;
	double r;		/* Radius of projected sphere in plot units (inch or cm) */
	bool polar;		/* True if projection pole coincides with S or N pole */

	/* Mollweide, Hammer-Aitoff and Winkel Projection */

	double w_x, w_y, w_iy, w_r;

	/* Winkel Tripel Projection */

	double r_cosphi1;	/* = cos (50.467) */

	/* Robinson Projection */

	double n_cx, n_cy;	/* = = 0.8487R, 1.3523R */
	double n_i_cy;
	double n_phi[GMT_N_ROBINSON], n_X[GMT_N_ROBINSON], n_Y[GMT_N_ROBINSON];
	double n_x_coeff[3*GMT_N_ROBINSON], n_y_coeff[3*GMT_N_ROBINSON], n_yx_coeff[3*GMT_N_ROBINSON], n_iy_coeff[3*GMT_N_ROBINSON];

	/* Eckert IV Projection */

	double k4_x, k4_y, k4_ix, k4_iy;

	/* Eckert VI Projection */

	double k6_r, k6_ir;

	/* Cassini Projection */

	double c_M0, c_c1, c_c2, c_c3, c_c4;
	double c_i1, c_i2, c_i3, c_i4, c_i5, c_p;

	/* All Cylindrical Projections */

	double j_x, j_y, j_ix, j_iy;

	/* Albers Equal-area conic parameters. */

	double a_n, a_i_n;
	double a_C, a_n2ir2, a_test, a_Cin;
	double a_rho0;

	/* Equidistant conic parameters. */

	double d_n, d_i_n;
	double d_G, d_rho0;

	/* Van der Grinten parameters. */

	double v_r, v_ir;

	/* General Perspective parameters */
	double g_H, g_R;
	double g_P, g_P_inverse;
	double g_lon0;
	double g_sphi1, g_cphi1;
	double g_phig, g_sphig, g_cphig;
	double g_sdphi, g_cdphi;
	double g_B, g_D, g_L, g_G, g_J;
	double g_BLH, g_DG, g_BJ, g_DHJ, g_LH2, g_HJ;
	double g_sin_tilt, g_cos_tilt;
	double g_azimuth, g_sin_azimuth, g_cos_azimuth;
	double g_sin_twist, g_cos_twist;
	double g_width;
	double g_yoffset;
	double g_rmax;
	double g_max_yt;
	double g_xmin, g_xmax;
	double g_ymin, g_ymax;

	unsigned int g_debug;
	int g_box, g_outside, g_longlat_set, g_sphere, g_radius, g_auto_twist;
	bool windowed;
	
	/* Polar (cylindrical) projection */

	double p_base_angle;
	bool got_azimuths, got_elevations, z_down;

	/* PROJ4 variables */
	double proj4_x0, proj4_y0, proj4_scl;
	bool is_proj4;
};

enum GMT_enum_frame {GMT_IS_PLAIN = 0,	/* Plain baseframe */
	GMT_IS_INSIDE	= 1,	/* Plain frame ticks/annotations on the inside of boundary */
	GMT_IS_GRAPH	= 2,	/* Plain fram with arrow extensions on axes */
	GMT_IS_FANCY	= 4,	/* Fancy baseframe */
	GMT_IS_ROUNDED	= 12};	/* Fancy baseframe, rounded */

/* Define the 6 axis items that each axis can have (some are mutually exclusive: only one ANNOT/INTV for upper and lower) */

enum GMT_enum_tick {GMT_ANNOT_UPPER = 0,	/* Tick annotations closest to the axis */
	GMT_ANNOT_LOWER,	/* Tick annotations farthest from the axis*/
	GMT_TICK_UPPER,		/* Frame tick marks closest to the axis */
	GMT_TICK_LOWER,		/* Frame tick marks farthest to the axis */
	GMT_GRID_UPPER,		/* Gridline spacing */
	GMT_GRID_LOWER};	/* Gridline spacing */

/* Some convenient macros for axis routines */


/* The array side in GMT_PLOT_FRAME follows the order south, east, north, west (CCW loop) + z.
 * Ro avoid using confusing indices 0-4 we define very brief constants S_SIDE, E_SIDE, N_SIDE
 * W_SIDE and Z_SIDE that should be used instead. */

#ifndef S_SIDE
#define S_SIDE 0
#endif
#ifndef E_SIDE
#define E_SIDE 1
#endif
#ifndef N_SIDE
#define N_SIDE 2
#endif
#ifndef W_SIDE
#define W_SIDE 3
#endif
#ifndef Z_SIDE
#define Z_SIDE 4
#endif

struct GMT_PLOT_AXIS_ITEM {		/* Information for one type of tick/annotation */
	double interval;		/* Distance between ticks in user units */
	unsigned int parent;		/* Id of axis this item belongs to (0,1,2) */
	bool active;			/* true if we want to use this item */
	bool generated;			/* true if this is an auto-generated interval */
	bool special;			/* true if custom interval annotations */
	unsigned int flavor;		/* Index into month/day name abbreviation array (0-2) */
	bool upper_case;		/* true if we want upper case text (used with flavor) */
	char type;			/* One of a, A, i, I, f, F, g, G */
	char unit;			/* User's interval unit (y, M, u, d, h, m, c) */
};

struct GMT_PLOT_AXIS {		/* Information for one time axis */
	unsigned int id;		/* 0 (x), 1(y), or 2(z) */
	unsigned int type;		/* GMT_LINEAR, GMT_LOG10, GMT_POW, GMT_TIME */
	unsigned int special;		/* See gmt_enum_annot values */
	unsigned int label_mode;	/* 0 = parallel to all axes, 1 = always horizontal on map */
	bool substitute_pi;		/* True if we need to plot fractions of pi on this axis */
	bool use_angle;			/* True if we got +a<angle>|n|p for this axis */
	struct GMT_PLOT_AXIS_ITEM item[8];	/* see above defines for which is which */
	double phase;			/* Phase offset for strides: (knot-phase)%interval = 0  */
	double angle;			/* Annotations angle set by user */
	char label[GMT_LEN256];		/* Label of the axis */
	char secondary_label[GMT_LEN256];	/* Optionally use this label when axis is right or top */
	char unit[GMT_LEN64];		/* Axis unit appended to annotations */
	char prefix[GMT_LEN64];		/* Axis prefix starting all annotations */
	char *file_custom;		/* File with custom annotations */
};

struct GMT_PLOT_FRAME {		/* Various parameters for plotting of time axis boundaries */
	struct GMT_PLOT_AXIS axis[3];	/* One each for x, y, and z */
	char header[GMT_LEN256];	/* Plot title */
	struct GMT_FILL fill;		/* Fill for the basemap inside, if paint == true */
	bool plotted_header;		/* true if header has been plotted */
	bool init;			/* true if -B was used at all */
	bool set;			/* true if -B was used to set any increments */
	bool draw;			/* true if -B<int> was used, even -B0, as sign to draw axes */
	bool drawz;			/* true if -B<int> was used, even -Bz0, as sign to draw z axes */
	bool paint;			/* true if -B +g<fill> was used */
	bool draw_box;			/* true if a 3-D Z-box is desired */
	bool no_frame;			/* true if we just want gridlines but no frame, i.e +n was used */
	bool check_side;		/* true if lon and lat annotations should be on x and y axis only */
	bool primary;			/* true if current axis is primary, false if secondary */
	bool set_both;			/* true if -B argument applies to both x and y axes */
	bool obl_grid;			/* true if +o was given to draw oblique gridlines */
	unsigned int set_frame[2];	/* 1 if a -B<WESNframe> setting was given */
	unsigned int horizontal;	/* 1 is S/N annotations should be parallel to axes, 2 if forced */
	unsigned int side[5];		/* Which sides (0-3 in plane; 4 = z) to plot. 2 is annot/draw, 1 is draw, 0 is not */
	unsigned int z_axis[4];		/* Which axes to use for the 3-D z-axis [auto] */
};

#endif /* GMT_PROJECT_H */
