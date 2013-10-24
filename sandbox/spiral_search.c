/* Given a grid and a start (row,col) and optionally a radius:
 * since grid[row][col] is NaN, find nearest node that is not NaN.
 * We wish to search outwards from this node and keep track of non-
 * NaN nodes and return the value and location of the nearest one.
 */

int gmt_grdspiral_search (struct GMT_CTRL *GMT, unsigned int row0, unsigned int col0, double radius, struct GMT_GRID *G, double *answers)
{
	unsigned int T, B, L, R;
	int64_t t_row, b_row, l_col, r_col, step = 0;
	bool done = false, found = false;
	
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
		GMT_memset (answers, 4, double);				/* Reset answers (lon, lat, radius, value) */
		T = scan_grd_row (GMT, t_row, l_col, r_col, G, answers);	/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		B = scan_grd_row (GMT, b_row, l_col, r_col, G, answers);	/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		L = scan_grd_col (GMT, l_col, t_row, b_row, G, answers);	/* Look along this col, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		R = scan_grd_col (GMT, r_col, t_row, b_row, G, answers);	/* Look along this col, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		done = ((T + B + L + R) == 8);
		found = (T == 1 || B == 1 || L == 1 || R == 1);
	} while (!done && !found);
	if (done) return 0;
	return 1;
}

unsigned int scan_grd_row (struct GMT_CTRL *GMT, int64_t row, int64_t l_col, int64_t r_col, struct GMT_GRID *G, double *answers)
{
/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
}

unsigned int scan_grd_col (struct GMT_CTRL *GMT, int64_t col, int64_t t_row, int64_t b_row, struct GMT_GRID *G, double *answers)
{
/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
}
