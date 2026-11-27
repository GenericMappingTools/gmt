#include "gmt_dev.h"
#include "gmt_internals.h"

/* Check the order of projected segment endpoints when a line is split. */
static int check_split (struct GMT_CTRL *GMT, const char *name, double x0, double y0,
                        double x1, double y1, bool vertical, bool high_first, const double *lon, const double *lat) {
	const struct GMT_CONTOUR_SOURCE source = {lon, lat, NULL};
	uint64_t n = 2, *split = NULL;
	double *x = gmt_M_memory (GMT, NULL, n, double);
	double *y = gmt_M_memory (GMT, NULL, n, double);
	int status = 1;

	x[0] = x0; y[0] = y0;
	x[1] = x1; y[1] = y1;
	split = gmtlib_split_line_with_source (GMT, &x, &y, &n, true, &source);
	if (split && n == 4 && split[0] == 1 && split[1] == 2 && split[2] == n) {
		double end, start, expected_end, expected_start;
		if (vertical) {
			end = y[1]; start = y[2];
			expected_end = high_first ? GMT->current.map.height : 0.0;
			expected_start = high_first ? 0.0 : GMT->current.map.height;
		}
		else {
			end = x[1]; start = x[2];
			expected_end = high_first ? gmtlib_right_boundary (GMT, y[1]) : gmtlib_left_boundary (GMT, y[1]);
			expected_start = high_first ? gmtlib_left_boundary (GMT, y[2]) : gmtlib_right_boundary (GMT, y[2]);
		}
		if (fabs (end - expected_end) <= 1e-9 && fabs (start - expected_start) <= 1e-9)
			status = 0;
		else
			fprintf (stderr, "%s: expected crossings %.15g, %.15g; got %.15g, %.15g\n",
			         name, expected_end, expected_start, end, start);
	}
	else
		fprintf (stderr, "%s: expected a split into two two-point segments\n", name);
	if (status == 0) printf ("%s: passed\n", name);
	gmt_M_free (GMT, x);
	gmt_M_free (GMT, y);
	gmt_M_free (GMT, split);
	return status;
}

static int check_geographic_split (struct GMT_CTRL *GMT, const char *name, double lon0, double lat0,
                                   double lon1, double lat1, bool vertical, bool high_first) {
	const double lon[] = {lon0, lon1}, lat[] = {lat0, lat1};
	double x0, y0, x1, y1;
	gmt_geo_to_xy (GMT, lon0, lat0, &x0, &y0);
	gmt_geo_to_xy (GMT, lon1, lat1, &x1, &y1);
	return check_split (GMT, name, x0, y0, x1, y1, vertical, high_first, lon, lat);
}

static int check_existing_crossings (struct GMT_CTRL *GMT, double left, double right, double latitude) {
	double offset = 0.05 * (right - left);
	const double xin[] = {right - offset, right, left, left + offset};
	const double yin[] = {latitude, latitude, latitude, latitude};
	uint64_t n = 4, *split;
	double *x = gmt_M_memory (GMT, NULL, n, double);
	double *y = gmt_M_memory (GMT, NULL, n, double);
	bool pass;

	gmt_M_memcpy (x, xin, n, double);
	gmt_M_memcpy (y, yin, n, double);
	split = gmtlib_split_line (GMT, &x, &y, &n, false);
	pass = split && n == 4 && split[0] == 1 && split[1] == 2 && split[2] == n &&
	       memcmp (x, xin, sizeof (xin)) == 0 && memcmp (y, yin, sizeof (yin)) == 0;
	if (pass) printf ("existing crossings: passed\n");
	else fprintf (stderr, "existing crossings: expected a split with the original coordinates\n");
	gmt_M_free (GMT, x);
	gmt_M_free (GMT, y);
	gmt_M_free (GMT, split);
	return !pass;
}

static bool setup_map (struct GMTAPI_CTRL *API, const char *command) {
	struct GMT_CTRL *GMT = API->GMT;
	struct GMT_OPTION *options = GMT_Create_Options (API, 0, command);
	int error;

	/* Allow a second map to use these options in the same session. */
	GMT->common.J.active = GMT->common.R.active[RSET] = false;
	GMT->common.f.active[GMT_IN] = GMT->common.f.active[GMT_OUT] = false;
	error = options == NULL || GMT_Parse_Common (API, "RJf", options) || gmt_map_setup (GMT, GMT->common.R.wesn);
	if (options) error |= GMT_Destroy_Options (API, &options);
	if (error) fprintf (stderr, "Could not initialize map for %s\n", command);
	return !error;
}

static int plot_cropped_contour (const char *name, double west) {
	struct GMTAPI_CTRL *API = GMT_Create_Session (name, 2U, GMT_SESSION_EXTERNAL | GMT_SESSION_NOEXIT, NULL);
	const unsigned int family = GMT_IS_GRID | GMT_VIA_MATRIX;
	static double data[181][361];
	double range[] = {west, west + 360.0, -90.0, 90.0}, inc[] = {1.0, 1.0};
	char input[GMT_VF_LEN], args[GMT_LEN256];
	struct GMT_MATRIX *matrix;
	unsigned int row, col;
	int error = EXIT_FAILURE;

	if (API == NULL) return EXIT_FAILURE;
	for (row = 0; row < 181; row++) {
		double lat = 90.0 - row;
		for (col = 0; col < 361; col++) data[row][col] = 0.2 * cos ((west + col - 45.0) * D2R) + sin (lat * D2R);
	}
	matrix = GMT_Create_Data (API, family, GMT_IS_SURFACE, GMT_CONTAINER_ONLY | GMT_GRID_IS_GEO, NULL, range, inc, GMT_GRID_NODE_REG, 0, NULL);
	if (matrix && !GMT_Put_Matrix (API, matrix, GMT_DOUBLE, 0, data) &&
	    !GMT_Open_VirtualFile (API, family, GMT_IS_SURFACE, GMT_IN | GMT_IS_REFERENCE, matrix, input)) {
		snprintf (args, sizeof (args), "%s png A,E144", name);
		error = GMT_Call_Module (API, "begin", GMT_MODULE_CMD, args);
		if (!error) error = GMT_Call_Module (API, "basemap", GMT_MODULE_CMD, "-R-135/225/-60/30 -JW45/10c -B0");
		snprintf (args, sizeof (args), "%s -An -C-0.8, -W0.9p,magenta -fg", input);
		if (!error) error = GMT_Call_Module (API, "grdcontour", GMT_MODULE_CMD, args);
		if (!error) error = GMT_Call_Module (API, "end", GMT_MODULE_CMD, "");
		error |= GMT_Close_VirtualFile (API, input);
	}
	error |= GMT_Destroy_Session (API);
	return error;
}

int main (void) {
	struct GMTAPI_CTRL *API = GMT_Create_Session ("testapi_contour_wrap", 2U, GMT_SESSION_NOEXIT, NULL);
	struct GMT_CTRL *GMT;
	const double tm_seam = nextafter (90.0, 180.0);
	const bool legacy_vertical = CHAR_MIN < 0;	/* The old splitter uses the native char type for the axis. */
	double left, right, x0, y0, height, lon[2], lat[2];
	int status = 0;

	if (API == NULL) return EXIT_FAILURE;
	GMT = API->GMT;
	if (!setup_map (API, "-R-180/180/-80/80 -JT0/0/10c -fg")) goto map_error;
	height = GMT->current.map.height;
	status |= check_geographic_split (GMT, "transverse Mercator top to bottom", tm_seam, -55.0, 89.0, -55.0, true, true);
	status |= check_geographic_split (GMT, "transverse Mercator bottom to top", 90.0, -55.0, 91.0, -55.0, true, false);
	/* Longitude winding on the far side of TM can trigger a horizontal split. */
	y0 = 0.75 * height;
	right = GMT->current.map.width;
	gmt_xy_to_geo (GMT, &lon[0], &lat[0], right, y0);
	gmt_xy_to_geo (GMT, &lon[1], &lat[1], 0.0, y0);
	lon[1] += 360.0;
	status |= check_split (GMT, "transverse Mercator horizontal line", right, y0, 0.0, y0, false, false, lon, lat);

	if (!setup_map (API, "-R-180/180/-60/60 -JQ0/10c -fg")) goto map_error;
	height = GMT->current.map.height;
	y0 = 0.5 * height;
	left = gmtlib_left_boundary (GMT, y0);
	right = gmtlib_right_boundary (GMT, y0);
	/* The grid's source longitudes continue through the boundary in both directions. */
	status |= check_geographic_split (GMT, "cylindrical east to west", 180.0, 0.0, 198.0, 0.0, false, true);
	status |= check_geographic_split (GMT, "cylindrical west to east", -180.0, 0.0, -198.0, 0.0, false, false);
	status |= check_existing_crossings (GMT, left, right, y0);
	/* Switching from TM leaves this vertical line on the legacy ordering path. */
	x0 = 0.5 * (left + right);
	gmt_xy_to_geo (GMT, &lon[0], &lat[0], x0, height);
	gmt_xy_to_geo (GMT, &lon[1], &lat[1], x0, 0.05 * height);
	status |= check_split (GMT, "cylindrical vertical line after transverse Mercator", x0, height, x0, 0.05 * height, legacy_vertical, false, lon, lat);
	status |= check_geographic_split (GMT, "global full-width contour", 180.0, 15.0, -180.0, 15.0, false, false);

	if (!setup_map (API, "-R-60/60/-30/30 -JQ0/10c -fg")) goto map_error;
	/* A 2-by-2 latitude grid gives an east-to-west contour at 15 degrees across the entire region. */
	status |= check_geographic_split (GMT, "regional horizontal contour", 60.0, 15.0, -60.0, 15.0, false, false);

	if (!setup_map (API, "-R-60/60/-30/30 -JT0/0/10c -fg")) goto map_error;
	height = GMT->current.map.height;
	x0 = 0.5 * GMT->current.map.width;
	gmt_xy_to_geo (GMT, &lon[0], &lat[0], x0, height);
	gmt_xy_to_geo (GMT, &lon[1], &lat[1], x0, 0.05 * height);
	status |= check_split (GMT, "regional transverse Mercator vertical line", x0, height, x0, 0.05 * height, legacy_vertical, false, lon, lat);

	if (!setup_map (API, "-R-180/180/-90/90 -JW0/10c -fg")) goto map_error;
	status |= check_geographic_split (GMT, "Mollweide west to east", -179.0, 5.0, -181.0, 50.0, false, false);
	status |= check_geographic_split (GMT, "Mollweide east to west", -181.0, 50.0, -179.0, 5.0, false, true);
	status |= GMT_Destroy_Session (API);
	status |= plot_cropped_contour ("matrix", 0.0);
	status |= plot_cropped_contour ("aligned", -135.0);
	return status ? EXIT_FAILURE : EXIT_SUCCESS;

map_error:
	GMT_Destroy_Session (API);
	return EXIT_FAILURE;
}
