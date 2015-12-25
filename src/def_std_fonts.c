/*	$Id$ */

/*! . */
int def_std_fonts(struct GMT_CTRL *GMT) {

	/* Listing of "Standard" 35 PostScript fonts found on most PS printers.
	   To add additional fonts, create a similar file called PSL_custom_fonts.txt
	   in GMT/share/postscriptlight and add your extra font information there.
	   The fontheight below is the height of A for unit fontsize.
	   Encoded = 0 if we may reencode this font as needed.
	*/
	int i = 0;
	size_t n_alloc = 0;

	GMT_set_meminc (GMT, GMT_SMALL_CHUNK);	/* Only allocate a small amount */
	GMT->session.font = GMT_malloc (GMT, GMT->session.font, i, &n_alloc, struct GMT_FONTSPEC); 

	/* Use strdup() because the non-hardwired version uses it too and somewhere there must be a free() */
	GMT->session.font[i].height = 0.700;		GMT->session.font[i++].name = strdup("Helvetica");
	GMT->session.font[i].height = 0.709;		GMT->session.font[i++].name = strdup("Helvetica-Bold");
	GMT->session.font[i].height = 0.700;		GMT->session.font[i++].name = strdup("Helvetica-Oblique");
	GMT->session.font[i].height = 0.709;		GMT->session.font[i++].name = strdup("Helvetica-BoldOblique");
	GMT->session.font[i].height = 0.673;		GMT->session.font[i++].name = strdup("Times-Roman");
	GMT->session.font[i].height = 0.685;		GMT->session.font[i++].name = strdup("Times-Bold");
	GMT->session.font[i].height = 0.673;		GMT->session.font[i++].name = strdup("Times-Italic");
	GMT->session.font[i].height = 0.685;		GMT->session.font[i++].name = strdup("Times-BoldItalic");
	GMT->session.font[i].height = 0.620;		GMT->session.font[i++].name = strdup("Courier");
	GMT->session.font[i].height = 0.620;		GMT->session.font[i++].name = strdup("Courier-Bold");
	GMT->session.font[i].height = 0.620;		GMT->session.font[i++].name = strdup("Courier-Oblique");
	GMT->session.font[i].height = 0.620;		GMT->session.font[i++].name = strdup("Courier-BoldOblique");
	GMT->session.font[i].height = 0.679;		GMT->session.font[i++].name = strdup("Symbol");
	GMT->session.font[i].height = 0.734;		GMT->session.font[i++].name = strdup("AvantGarde-Book");
	GMT->session.font[i].height = 0.734;		GMT->session.font[i++].name = strdup("AvantGarde-BookOblique");
	GMT->session.font[i].height = 0.734;		GMT->session.font[i++].name = strdup("AvantGarde-Demi");
	GMT->session.font[i].height = 0.734;		GMT->session.font[i++].name = strdup("AvantGarde-DemiOblique");
	GMT->session.font[i].height = 0.675;		GMT->session.font[i++].name = strdup("Bookman-Demi");
	GMT->session.font[i].height = 0.675;		GMT->session.font[i++].name = strdup("Bookman-DemiItalic");
	GMT->session.font[i].height = 0.675;		GMT->session.font[i++].name = strdup("Bookman-Light");
	GMT->session.font[i].height = 0.675;		GMT->session.font[i++].name = strdup("Bookman-LightItalic");
	GMT->session.font[i].height = 0.700;		GMT->session.font[i++].name = strdup("Helvetica-Narrow");
	GMT->session.font[i].height = 0.706;		GMT->session.font[i++].name = strdup("Helvetica-Narrow-Bold");
	GMT->session.font[i].height = 0.700;		GMT->session.font[i++].name = strdup("Helvetica-Narrow-Oblique");
	GMT->session.font[i].height = 0.706;		GMT->session.font[i++].name = strdup("Helvetica-Narrow-BoldOblique");
	GMT->session.font[i].height = 0.704;		GMT->session.font[i++].name = strdup("NewCenturySchlbk-Roman");
	GMT->session.font[i].height = 0.704;		GMT->session.font[i++].name = strdup("NewCenturySchlbk-Italic");
	GMT->session.font[i].height = 0.704;		GMT->session.font[i++].name = strdup("NewCenturySchlbk-Bold");
	GMT->session.font[i].height = 0.704;		GMT->session.font[i++].name = strdup("NewCenturySchlbk-BoldItalic");
	GMT->session.font[i].height = 0.689;		GMT->session.font[i++].name = strdup("Palatino-Roman");
	GMT->session.font[i].height = 0.700;		GMT->session.font[i++].name = strdup("Palatino-Italic");
	GMT->session.font[i].height = 0.665;		GMT->session.font[i++].name = strdup("Palatino-Bold");
	GMT->session.font[i].height = 0.677;		GMT->session.font[i++].name = strdup("Palatino-BoldItalic");
	GMT->session.font[i].height = 0.610;		GMT->session.font[i++].name = strdup("ZapfChancery-MediumItalic");
	GMT->session.font[i].height = 0.700;		GMT->session.font[i++].name = strdup("ZapfDingbats");

	GMT->session.n_fonts = i;
	return i;
}
