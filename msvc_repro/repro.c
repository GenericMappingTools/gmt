/* Reduced from the y block of gmtnc_grd_info (GMT src/gmt_nc.c).  The call to get_att() writes into
 * dummy[]; its return value (has_range) is only tested on one branch, but dummy[] is read later on all
 * of them.  For a 1-row grid has_vector is false and the expected result is dummy = {-0.2, 0.2}. */
#include <stdio.h>
#include <math.h>
int get_att (int id, const char *name, double *out);
void report (const char *msg, double a, double b);

struct hdr { unsigned n_rows; int registration; int row_order; };

/* V1: smallest form */
__declspec(noinline) void v1 (struct hdr *h, int vec_ok, double *xy, double *out) {
	double dummy[2];
	int has_vector, has_range;
	dummy[0] = 0.0, dummy[1] = (double) h->n_rows - 1;
	if ((has_vector = vec_ok) && h->n_rows > 1) {
		dummy[0] = xy[0], dummy[1] = xy[h->n_rows-1];
	}
	if (has_vector && h->n_rows == 1) has_vector = 0;
	has_range = !get_att (1, "actual_range", dummy);
	if (has_vector && has_range) {
		if (fabs (dummy[0] - xy[0]) > 0.1) dummy[0] = xy[0];
	}
	else if (has_vector) {
		dummy[0] = xy[0] - 0.5, dummy[1] = xy[h->n_rows-1] + 0.5;
	}
	out[0] = dummy[0], out[1] = dummy[1];
}

/* V2: as V1 plus the shape of the real function: the has_vector && has_range branch has several
 * paths with calls, the else branch has a call, and a swap follows. */
__declspec(noinline) void v2 (struct hdr *h, int vec_ok, double *xy, double *out) {
	double dummy[2], min, max, dy = 0.0, threshold = 0.0;
	int has_vector, has_range, registration = 0;
	dummy[0] = 0.0, dummy[1] = (double) h->n_rows - 1;
	registration = 0;
	if ((has_vector = vec_ok) && h->n_rows > 1) {
		dummy[0] = xy[0], dummy[1] = xy[h->n_rows-1];
		dy = fabs (xy[1] - xy[0]);
	}
	if (has_vector && h->n_rows == 1) has_vector = 0;
	has_range = !get_att (1, "actual_range", dummy);
	if (has_vector && has_range) {
		threshold = 0.5 * dy;
		min = xy[0], max = xy[h->n_rows-1];
		if (min > max) h->row_order = -1;
		if (min > max) { double t = min; min = max; max = t; }
		if (fabs (dummy[0] - min) > threshold || fabs (dummy[1] - max) > threshold) {
			report ("conflict", dummy[0], min);
			dummy[0] = xy[0], dummy[1] = xy[h->n_rows-1];
			has_range = 0;
			if (fmod (dummy[0], dy) > threshold) { registration = 1; dummy[0] -= 0.5 * dy, dummy[1] += 0.5 * dy; }
		}
		else dummy[0] = xy[0], dummy[1] = xy[h->n_rows-1];
	}
	else if (has_vector) {
		threshold = 0.5 * dy;
		dummy[0] = xy[0], dummy[1] = xy[h->n_rows-1];
		if (fmod (dummy[0], dy) > threshold) { registration = 1; dummy[0] -= 0.5 * dy, dummy[1] += 0.5 * dy; }
	}
	else {
		if ((h->n_rows % 2) == 1 && h->registration == 0) report ("guessing", 0.0, 0.0);
	}
	if (dummy[0] > dummy[1]) {
		double t = dummy[0]; dummy[0] = dummy[1]; dummy[1] = t;
		h->row_order = -1;
	}
	else if (has_vector && xy[0] > xy[h->n_rows-1]) h->row_order = -1;
	else h->row_order = 1;
	out[0] = dummy[0] + 0.0 * registration, out[1] = dummy[1];
}

/* V3: as V2 but the x block runs first on the same locals, like in the real function. */
__declspec(noinline) void v3 (struct hdr *h, int vec_ok, double *xy, double *out) {
	double dummy[2], min, max, dx = 0.0, dy = 0.0, threshold = 0.0;
	int has_vector, has_range, registration = 0;
	unsigned ncols = 6;
	dummy[0] = 0.0, dummy[1] = (double) ncols - 1;
	if ((has_vector = vec_ok) && ncols > 1) {
		dummy[0] = xy[0], dummy[1] = xy[ncols-1];
		dx = fabs (xy[1] - xy[0]);
	}
	has_range = !get_att (0, "actual_range", dummy);
	if (has_vector && has_range) {
		threshold = 0.5 * dx;
		min = xy[0], max = xy[ncols-1];
		if (fabs (dummy[0] - min) > threshold || fabs (dummy[1] - max) > threshold) { dummy[0] = min, dummy[1] = max; has_range = 0; }
	}
	else if (has_vector) dummy[0] = xy[0], dummy[1] = xy[ncols-1];
	out[2] = dummy[0], out[3] = dummy[1];
	/* y block */
	dummy[0] = 0.0, dummy[1] = (double) h->n_rows - 1;
	registration = 0;
	if ((has_vector = vec_ok) && h->n_rows > 1) {
		dummy[0] = xy[0], dummy[1] = xy[h->n_rows-1];
		dy = fabs (xy[1] - xy[0]);
	}
	if (has_vector && h->n_rows == 1) has_vector = 0;
	has_range = !get_att (1, "actual_range", dummy);
	if (has_vector && has_range) {
		threshold = 0.5 * dy;
		min = xy[0], max = xy[h->n_rows-1];
		if (fabs (dummy[0] - min) > threshold || fabs (dummy[1] - max) > threshold) {
			report ("conflict", dummy[0], min);
			dummy[0] = xy[0], dummy[1] = xy[h->n_rows-1];
			has_range = 0;
		}
	}
	else if (has_vector) {
		dummy[0] = xy[0] - 0.5 * dy, dummy[1] = xy[h->n_rows-1] + 0.5 * dy;
		registration = 1;
	}
	else {
		if ((h->n_rows % 2) == 1 && h->registration == 0) report ("guessing", 0.0, 0.0);
	}
	if (dummy[0] > dummy[1]) { double t = dummy[0]; dummy[0] = dummy[1]; dummy[1] = t; }
	out[0] = dummy[0] + 0.0 * registration, out[1] = dummy[1];
}

typedef void (*fn) (struct hdr *, int, double *, double *);

int main (void) {
	static const char *name[] = {"v1", "v2", "v3"};
	fn f[3] = {v1, v2, v3};
	double xy[6] = {-1.0, -0.6, -0.2, 0.2, 0.6, 1.0};
	int k, bad = 0;
	for (k = 0; k < 3; k++) {
		struct hdr h = {1, 1, 0};	/* one row, pixel registration: has_vector ends up false */
		double out[4] = {-99.0, -99.0, -99.0, -99.0};
		f[k] (&h, 1, xy, out);
		printf ("%s: y range = %g %g  (expected -0.2 0.2)  %s\n", name[k], out[0], out[1],
		        (out[0] == -0.2 && out[1] == 0.2) ? "OK" : "WRONG");
		if (!(out[0] == -0.2 && out[1] == 0.2)) bad++;
	}
	return bad;
}
