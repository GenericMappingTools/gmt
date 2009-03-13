#define DELAUNAY	0
#define VORONOI		1
#define TRI_NROW	6	/* Don't request arc indeces from STRIPACK  */

struct STRIPACK_DELAUNAY {	/* Information about Delaunay triangulation */
	int n;	/* Number of Delaunay triangles */
	int *tri;	/* Delaunay triplet node numbers and more */
};
struct STRIPACK_VORONOI {	/* Information about Voronoi polygons */
	double *lon, *lat;	/* Voronoi polygon vertices */
	int n;				/* Number of boundary nodes for Voronoi */
	int *lend, *listc, *lptr;	/* Voronoi vertex lists and pointers */			
};
struct STRIPACK {
	int mode;	/* VORONOI or DELAUNAY */
	struct STRIPACK_DELAUNAY D;
	struct STRIPACK_VORONOI V;
};
struct STRPACK_ARC {
	int begin, end;
};

void stripack_lists (GMT_LONG n, double *x, double *y, double *z, struct STRIPACK *T);
double stripack_areas (double *V1, double *V2, double *V3);
void cart_to_geo (GMT_LONG n, double *x, double *y, double *z, double *lon, double *lat);
int compare_arc (const void *p1, const void *p2);
void geo_to_cart (double alat, double alon, double *a, int rads);
void ssrfpack_grid (double *x, double *y, double *z, double *w, int n, int mode, double *par, BOOLEAN vartens, struct GRD_HEADER *h, double *f);

#ifdef G95
/* If linking via g95 instead we need to use these start/stop functions */
void g95_runtime_start (int argc, char *argv[]);
void g95_runtime_stop ();
#endif
