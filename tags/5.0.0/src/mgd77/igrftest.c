#include "mgd77.h"

/* Print out IGRF values for 1960 to 2005 at (0,0) on Jan 1 that year */

int main (int argc, char **argv)
{
	GMT_LONG y, mode = 1;
	double IGRF[7];
	
	if (argc > 1) mode = 2;
	for (y = 1960; y <= 2005; y++) {
		MGD77_igrf10syn (0, (double)y, mode, 0.0, 0.0, 45.0, IGRF);
		printf ("Year = %d  F = %8.1f  H = %8.1f X = %8.1f Y = %8.1f Z = %8.1f D = %6.1f I = %6.1f\n", y, IGRF[0], IGRF[1], IGRF[2], IGRF[3], IGRF[4], IGRF[5], IGRF[6]);
	}
}
