/* Stand-in for netCDF's nc_get_att_double: lives in another translation unit so it cannot be inlined. */
#include <string.h>
int get_att (int id, const char *name, double *out) {
	(void)id;
	if (strcmp (name, "actual_range")) return -43;	/* attribute not found */
	out[0] = -0.2;
	out[1] = 0.2;
	return 0;
}
void report (const char *msg, double a, double b) {	/* stand-in for GMT_Report */
	(void)msg; (void)a; (void)b;
}
