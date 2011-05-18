/* $Id: psldemo.c,v 1.2 2011-05-18 01:52:53 remko Exp $
 *
 * psldemo.c: Test all PSL functions at least once
 */

#include <stdio.h>
#include <strings.h>
#include "pslib.h"

int main (int argc, char **argv) {
	struct PSL_CTRL *PSL = NULL;
	double Letter[2] = {612.0, 792.0}, size[7];
	double rgb[7][4] = {{1.0, 0.0, 0.0, 0.0}, {0.5, 0.7, 0.1, 0.0}, {-1.0, -1.0, -1.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {1.0, 1.0, 1.0, 0.0},
		{-3.0, -3.0, -3.0, 0.0}, {0.9, 0.9, 9.0, 0.0}};
	double offset[2] = {0.1, 0.1}, scales[2] = {1.0, 1.0};
	double x[6] = {35.0, 50.0, 60., 90.0, 110.0, 132.0}, y[6] = {25.0, 10.0, 50.0, 87.0, 65.0, 75.0};
	PSL_LONG outline[3] = {PSL_OUTLINE, PSL_NONE, PSL_OUTLINE};
	PSL_LONG type[14] = {PSL_CROSS, PSL_XDASH, PSL_YDASH, PSL_PLUS, PSL_DOT, PSL_CIRCLE, PSL_DIAMOND,
		PSL_HEXAGON, PSL_INVTRIANGLE, PSL_OCTAGON, PSL_PENTAGON, PSL_SQUARE, PSL_STAR, PSL_TRIANGLE};
	char *para = "\t@%5%PSL@%% was created to make the generation of @%6%PostScript@%% page description code easier.  \
@%6%PostScript@%% is a page description language that was developed by @;255/0/0;Adobe@;; for specifying how a printer should render a \
page of text or graphics.  It uses a reverse Polish notation that puts and gets items from a stack to draw lines, \
text, and images and even perform calculations.\r \t@%5%PSL@%% is a @_self-contained@_ library that presents a series of functions \
that can be used to create plots.  The resulting @%6%PostScript@%% code is ASCII text.";
	int i, k;
	struct imageinfo h;
	unsigned char *picture;
	
	size[0] = 0.3;
	
	PSL = New_PSL_Ctrl (argv[0]);
	PSL->init.unit = PSL_INCH;
	PSL->internal.compress = PSL_LZW;
	PSL->internal.verbose = PSL_YES;
	PSL->internal.comments = PSL_YES;
	PSL->init.encoding = strdup ("Standard+");
	PSL_beginsession (PSL);
	PSL_beginplot (PSL, NULL, PSL_PORTRAIT, PSL_INIT, PSL_RGB, "rr", scales, Letter, NULL);

	/* Plot rectangle below the symbols */
	
	PSL_setlinewidth (PSL, 2.0);
	PSL_setfill (PSL, rgb[6], PSL_OUTLINE);
	PSL_plotbox (PSL, 0.1, -0.4, 1.9, 8.2);
	
	/* Try some symbols */
	for (k = 0; k < 3; k++) {
		PSL_setfill (PSL, rgb[k], outline[k]);
		for (i = 0; i < 14; i++) PSL_plotsymbol (PSL, 0.4 + k*0.6, i*0.6, size, type[i]);
	}

	/* Plot a simple x-y axis and a data set */
	PSL_setlinewidth (PSL, 1.0);
	PSL_beginaxes (PSL, 2.6, 0.0, 4.0, 3.0, 30.0, 100.0, 145.0, 0.0);
	PSL_plotaxis (PSL, 20.0, "Some Axis", 10.0, 0);
	PSL_plotaxis (PSL, 25.0, "A Reversed Axis", 10.0, 3);
	PSL_setlinewidth (PSL, 5.0);
	PSL_plotline (PSL, x, y, 6, PSL_MOVE + PSL_STROKE);
	PSL_setlinewidth (PSL, 1.0);
	PSL_setfill (PSL, rgb[0], outline[0]);
	for (i = 0; i < 6; i++) PSL_plotsymbol (PSL, x[i], y[i], size, PSL_STAR);

	/* Plot some patterns within the x-y axis */
	rgb[5][1] = PSL_setpattern (PSL, -1, "../doc/examples/ex16/circuit.ras", 100, rgb[2], rgb[2]);
	PSL_setfill (PSL, rgb[5], PSL_NO);
	PSL_plotbox (PSL, 80.0, 30.0, 140, 50);

	size[0] = 0.5;
	rgb[5][1] = PSL_setpattern (PSL, 13, "", 100, rgb[3], rgb[2]);
	PSL_setcolor (PSL, rgb[5], PSL_IS_STROKE);
	PSL_plotsymbol (PSL, 80.0, 30.0, size, PSL_DOT);

	PSL_setcolor (PSL, rgb[0], PSL_IS_STROKE);
	PSL_setfill (PSL, rgb[5], PSL_OUTLINE);
	PSL_plotsymbol (PSL, 95.0, 30.0, size, PSL_SQUARE);

	size[0] = 100.0; size[1] = 0.5; size[2] = 0.4;
	rgb[5][1] = PSL_setpattern (PSL, 13, "", 100, rgb[2], rgb[3]);
	PSL_plotsymbol (PSL, 110.0, 30.0, size, PSL_ROTRECT);

	rgb[5][1] = PSL_setpattern (PSL, 14, "", 100, rgb[1], rgb[2]);
	PSL_setfill (PSL, rgb[5], PSL_OUTLINE);
	PSL_plotsymbol (PSL, 125.0, 30.0, size, PSL_ELLIPSE);

	size[0] = 0.3; size[1] = 45.0; size[2] = 315.0;
	rgb[5][1] = PSL_setpattern (PSL, 14, "", 100, rgb[1], rgb[0]);
	PSL_plotsymbol (PSL, 140.0, 30.0, size, PSL_WEDGE);

	/* Return to normal coordinates */
	PSL_endaxes (PSL);
	
	/* Plot a range of lines with different attributes */
	PSL_setcolor (PSL, rgb[3], PSL_IS_STROKE);
	PSL_setlinewidth (PSL, 2.0);
	PSL_plotsegment (PSL, 2.2, 3.2, 4.5, 3.2);
	PSL_setlinecap (PSL, PSL_ROUND_CAP);
	PSL_plotsegment (PSL, 2.2, 3.3, 4.5, 3.3);
	PSL_setdash (PSL, "5 4", 0.0);
	PSL_plotsegment (PSL, 2.2, 3.4, 4.5, 3.4);
	PSL_setdash (PSL, "8 2 4 1 2 1 4 2", 4.0);
	PSL_setlinewidth (PSL, 1.0);
	PSL_setlinecap (PSL, PSL_BUTT_CAP);
	PSL_plotsegment (PSL, 2.2, 3.5, 4.5, 3.5);
	PSL_setlinecap (PSL, PSL_SQUARE_CAP);
	PSL_setdash (PSL, "1 1", 0.0);
	PSL_plotsegment (PSL, 2.2, 3.6, 4.5, 3.6);
	PSL_setlinecap (PSL, PSL_ROUND_CAP);
	PSL_setlinewidth (PSL, 3.0);
	PSL_setdash (PSL, "0 4", 0.0);
	PSL_plotsegment (PSL, 2.2, 3.7, 4.5, 3.7);

	/* Plot some text boxes */
	PSL_setdash (PSL, NULL, 0.0);
	PSL_setlinewidth (PSL, 0.5);
	PSL_setcolor (PSL, rgb[0], PSL_IS_STROKE);
	PSL_plottext (PSL, 2.2, 4.6, 30.0, "Hei Verden!", 0.0, PSL_BL, PSL_NONE);
	PSL_setfill (PSL, rgb[1], PSL_OUTLINE);
	PSL_plottext (PSL, 4.5, 4.5, 30.0, "Hoi Wereld!", 0.0, PSL_TR, PSL_OUTLINE);
	PSL_setfill (PSL, rgb[2], PSL_OUTLINE);
	PSL_plottext (PSL, 2.2, 4.0, 30.0, "Ol\337 Mundo!", 0.0, PSL_BL, PSL_OUTLINE);

	PSL_setfill (PSL, rgb[1], PSL_OUTLINE);
	PSL_plottextbox (PSL, 5.0, 4.0, 30.0, "Hey World!", 90.0, PSL_BC, offset, PSL_YES);
	PSL_setcolor (PSL, rgb[3], PSL_IS_STROKE);
	PSL_setfill (PSL, rgb[4], PSL_OUTLINE);
	PSL_plottext (PSL, 5.0, 4.0, 30.0, "Hey World!", 90.0, PSL_BC, PSL_OUTLINE);
	PSL_setfill (PSL, rgb[3], PSL_NONE);
	PSL_setfont (PSL, 6);
	PSL_plottext (PSL, 2.2, 5.3, 22.0, "E = mc@+2@+, @~D@~g = 2@~pr@~gh", 0.0, PSL_BL, PSL_NONE);
	PSL_plottext (PSL, 6.5, 7.0, 18.0, "E = mc@+2@+@;255/0/0; + @~e@~@;;?", 0.0, PSL_TR, PSL_NONE);
	PSL_plottext (PSL, 6.5, 6.5, 18.0, "1 @Angstr@om", 0.0, PSL_TR, PSL_NONE);
	PSL_setfont (PSL, 33);
	PSL_plottext (PSL, 3.25, 8.75, 32.0, "PSL v5.0 Demonstration Page", 0.0, PSL_TC, PSL_NONE);

	PSL_setfont (PSL, 4);
	PSL_setlinewidth (PSL, 1.5);
	PSL_setfill (PSL, rgb[6], PSL_OUTLINE);
	PSL_setparagraph (PSL, 0.15, 2.5, PSL_JUST);
	PSL_plotparagraphbox (PSL, 2.2, 8.1, 12.0, para, 0.0, PSL_TL, offset, 0);
	PSL_plotparagraph (PSL, 2.2, 8.1, 12.0, NULL, 0.0, PSL_TL);

	/* Plot an image by itself */
	PSL_loadimage (PSL, "vader.ras", &h, &picture);
	PSL_plotcolorimage (PSL, 5.0, 8.2, 1.5, 0.0, PSL_TL, picture, h.width, h.height, h.depth);
	PSL_free (PSL, picture);
	
	PSL_endplot (PSL, PSL_FINALIZE);
	PSL_endsession (PSL);
	return (0);
}
