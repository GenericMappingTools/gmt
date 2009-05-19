#define DELAUNAY	0
#define VORONOI		1
#define INTERPOLATE	2

#define TRI_NROW	6	/* Don't request arc indeces from STRIPACK  */

struct STRIPACK_DELAUNAY {	/* Information about Delaunay triangulation */
	int n;	/* Number of Delaunay triangles */
	int *tri;	/* Delaunay triplet node numbers and more */
};

struct STRIPACK_VORONOI {	/* Information about Voronoi polygons */
	double *lon, *lat;	/* Voronoi polygon vertices */
	int n;				/* Number of boundary nodes for Voronoi */
	int *lend, *listc, *lptr;	/* Voronoi vertex lists and pointers */
	int *list;		/* Additional list from trmesh */		
};

struct STRIPACK_INTERPOLATE {	/* Information about triangles */
	int *lend, *list, *lptr;	/* lists and pointers */
};

struct STRIPACK {
	int mode;	/* VORONOI, DELAUNAY, or INTERPOLATE */
	struct STRIPACK_DELAUNAY D;
	struct STRIPACK_VORONOI V;
	struct STRIPACK_INTERPOLATE I;
};

struct STRPACK_ARC {
	int begin, end;
};

EXTERN_MSC void stripack_lists (GMT_LONG n, double *x, double *y, double *z, BOOLEAN verbose, struct STRIPACK *T);
EXTERN_MSC double stripack_areas (double *V1, double *V2, double *V3);
EXTERN_MSC void cart_to_geo (GMT_LONG n, double *x, double *y, double *z, double *lon, double *lat);
EXTERN_MSC int compare_arc (const void *p1, const void *p2);
EXTERN_MSC void geo_to_cart (double alat, double alon, double *a, int rads);
EXTERN_MSC void ssrfpack_grid (double *x, double *y, double *z, double *w, GMT_LONG n, int mode, double *par, BOOLEAN vartens, BOOLEAN verbose, struct GRD_HEADER *h, double *f);
