/* Given a grid and a start (row,col) and optionally a radius:
 * since grid[row][col] is NaN, find nearest node that is not NaN.
 * We wish to search outwards from this node and keep track of non-
 * NaN nodes and return the value and location of the nearest one.
 * The answers array contains the following:
 * answers[0] : min distance to non-NaN node
 *
 */

struct GMT_ZSEARCH {
	double *x, *y;	/* Arrays with grid coordinates */
	double radius;	/* Smallest distance so far to non-NaN node */
	double max_radius;	/* Only consider radii less than this cutoff */
	uint64_t row0, col0;	/* Location of our NaN node */
	uint64_t row, col;	/* Location of nearest non-NaN node */
	struct GMT_GRID *G;	/* Pointer to the grid */
};

/* Usage in grdtrack: */

	#1. Early on:
	if (want_nearest) {
		Allocate S;
		S->G = Grid;
		S->x = GMT_Get_Coord (...);
		S->y = GMT_Get_Coord (...);
		S->max_radius = given_max_radius or DBL_MAX;
	}
	/* Then in loop over points */
	if (grdtrack returns NaN at given location && want_nearest) {
		S->row0 = j;	S->col0 = i;	/* Given location */
		if (gmt_grdspiral_search (GMT, S)) {
			<return value at S->row, S->col>
			<possibly return S->x[S->col], S->y[S->row]>
		}
	}
int gmt_grdspiral_search (struct GMT_CTRL *GMT, struct GMT_ZSEARCH *S)
{
	unsigned int T, B, L, R;
	int64_t t_row, b_row, l_col, r_col, step = 0;
	bool done = false, found = false;
	
	S->radius = DBL_MAX;
	do {	/* Keep spiraling until done or found something */
		step++;	/* Step outwards from central row0,col0 node */
		t_row = row0 - step;	b_row = row0 + step;	/* Next 2 rows to scan */
		l_col = col0 - step;	r_col = col0 + step;	/* Next 2 cols to scan */
		
		/* Each search is a square frame surrounding (row0, col0).  Since the
		 * nearest non-NaN node might be found anywhere along this frame we
		 * must search all the nodes along the 4 sides and then see which was
		 * the closest one.  Two situations can arise:
		 * 1) No non-NaN nodes found. Go to next frame unless
		 *    a) max_radius in this frame > radius and we just return NaN
		 *    b) If no radius limit we continue to go until we exceed grid borders
		 * 2) Found non-NaN node and return the one closest to (row0,col0). */
		T = scan_grd_row (GMT, t_row, l_col, r_col, S);	/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		B = scan_grd_row (GMT, b_row, l_col, r_col, S);	/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		L = scan_grd_col (GMT, l_col, t_row, b_row, S);	/* Look along this col, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		R = scan_grd_col (GMT, r_col, t_row, b_row, S);	/* Look along this col, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		done = ((T + B + L + R) == 8U);	/* Now completely outside grid on all sides */
		found = (T == 1 || B == 1 || L == 1 || R == 1);	/* Found a non-NaN and its distance from node */
	} while (!done && !found);
	if (found) return 1;
	return 0;
}

unsigned int scan_grd_row (struct GMT_CTRL *GMT, int64_t row, int64_t l_col, int64_t r_col, struct GMT_ZSEARCH *S)
{
	/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is returned, 0 if all NaN */
	unsigned int ret_code = 0;
	uint64_t col, node;
	double r;
	if (row < 0 || row >= S->G->header->ny) return 2;	/* Outside grid */
	if (l_col < 0) l_col = 0;	/* Start inside grid */
	if (r_col >= S->G->header->nx) r_col = S->G->header->nx - 1;	/* End inside grid */
	for (col = l_col; col <= r_col; col++) {	/* Search along this row */
		node = GMT_IJP (S->G->header, row, col);
		if (GMT_is_fnan (S->G->data[node])) continue;	/* A NaN node */
		r = GMT_distance (GMT, S->x0, S->y0, S->x[col], S->y[row]);
		if (r > S->max_distance) continue;	/* Basically not close enough */
		if (r < S->radius) {	/* Great, this one is closer */
			S->radius = r;
			S->row = row;
			S->col = col;
			ret_code = 1;
		}
	}
	return (ret_code);
}

unsigned int scan_grd_col (struct GMT_CTRL *GMT, int64_t col, int64_t t_row, int64_t b_row, struct GMT_GRID *G, double *answers)
{
/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN or beyond max radius*/
	unsigned int ret_code = 0;
	uint64_t row, node;
	double r;
	if (col < 0 || col >= S->G->header->nx) return 2;	/* Outside grid */
	if (t_row < 0) t_row = 0;	/* Start inside grid */
	if (b_row >= S->G->header->ny) b_row = S->G->header->ny - 1;	/* End inside grid */
	for (row = t_row; row <= b_row; row++) {	/* Search along this column */
		node = GMT_IJP (S->G->header, row, col);
		if (GMT_is_fnan (S->G->data[node])) continue;	/* A NaN node */
		r = GMT_distance (GMT, S->x0, S->y0, S->x[col], S->y[row]);
		if (r < S->radius) {	/* Great, this one is closer */
			S->radius = r;
			S->row = row;
			S->col = col;
			ret_code = 1;
		}
	}
	return (ret_code);
}
