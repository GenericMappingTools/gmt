/*
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/* GMT convenience functions used by MATLAB/OCTAVE mex functions
 */

#include "gmt_mex.h"

void GMTMEX_grdheader2info (mxArray *plhs[], struct GMT_GRID *G, int item)
{	/* Return the grid's header info via the info array */
	double *hdr = NULL;
	plhs[item] = mxCreateDoubleMatrix (1, 9, mxREAL);
	hdr = mxGetPr (plhs[item]);
	memcpy (hdr, G->header->wesn, 4 * sizeof(double));
	hdr[4] = G->header->z_min;	hdr[5] = G->header->z_max;
	hdr[6] = (double)G->header->registration;
	hdr[7] = G->header->inc[GMT_X];	hdr[8] = G->header->inc[GMT_Y];
}

double *GMTMEX_info2grdheader (struct GMTAPI_CTRL *API, const mxArray *prhs[], int nrhs, struct GMT_GRID *G)
{	/* Return the grid's header info via the info array (nrhs == 3) or x/y arrays (nrhs == 4|5) */
	double *z = NULL;
	if (nrhs == 3) {	/* Gave Z, info */
		double *hdr = NULL;
		z = mxGetData (prhs[0]);
		G->header->nx = mxGetN (prhs[0]);
		G->header->ny = mxGetM (prhs[0]);
		hdr = mxGetData (prhs[1]);
		GMT_memcpy (G->header->wesn, hdr, 4, double);
		G->header->z_min = hdr[4];
		G->header->z_max = hdr[5];
		G->header->registration = (int) irint (hdr[6]);
		G->header->inc[GMT_X] = hdr[7];
		G->header->inc[GMT_Y] = hdr[8];
	}
	else {	/* Gave x, y, Z [reg] */
		double *r = NULL, *x = NULL, *y = NULL;
		GMT_LONG col, row, error = 0;
		x = mxGetData (prhs[0]);
		y = mxGetData (prhs[1]);
		z = mxGetData (prhs[2]);
		G->header->nx = mxGetN (prhs[2]);
		G->header->ny = mxGetM (prhs[2]);
		G->header->inc[GMT_X] = x[1] - x[0];
		G->header->inc[GMT_Y] = y[1] - y[0];
		for (col = 2; !error && col < G->header->nx; col++) 
			if ((x[col] - x[col-1]) != G->header->inc[GMT_X]) error = 1;
		for (row = 2; !error && row < G->header->ny; row++) 
			if ((y[row] - y[row-1]) != G->header->inc[GMT_Y]) error = 1;
		if (error) {
			mexErrMsgTxt ("grdwrite: x and/or y not equidistant");
		}
		if (nrhs == 5) {
			r = mxGetData (prhs[3]);
			G->header->registration = (GMT_LONG)irint (r[0]);
		}
		else
			G->header->registration = GMT_GRIDLINE_REG;
		G->header->wesn[XLO] = (G->header->registration == GMT_PIXEL_REG) ? x[0] - 0.5 * G->header->inc[GMT_X] : x[0];
		G->header->wesn[XHI] = (G->header->registration == GMT_PIXEL_REG) ? x[G->header->nx-1] + 0.5 * G->header->inc[GMT_X] : x[G->header->nx-1];
		G->header->wesn[YLO] = (G->header->registration == GMT_PIXEL_REG) ? y[0] - 0.5 * G->header->inc[GMT_Y] : y[0];
		G->header->wesn[YHI] = (G->header->registration == GMT_PIXEL_REG) ? y[G->header->ny-1] + 0.5 * G->header->inc[GMT_Y] : y[G->header->ny-1];
	}
	GMT_grd_setpad (API->GMT, G->header, API->GMT->current.io.pad);	/* Assign default pad */
	GMT_set_grddim (API->GMT, G->header);
	return (z);
}

void GMTMEX_grdxy (struct GMTAPI_CTRL *API, mxArray *plhs[], struct GMT_GRID *G, int px, int py)
{	/* Return x,y arrays also */
	GMT_LONG row, col;
	double *xg = NULL, *yg = NULL;
	plhs[px] = mxCreateDoubleMatrix (1, G->header->nx, mxREAL);
	plhs[py] = mxCreateDoubleMatrix (1, G->header->ny, mxREAL);
	xg = mxGetPr (plhs[px]);	yg = mxGetPr (plhs[py]);
	/* Fill in the x and y arrays; note Matlab y array is flipped relative to the GMT y array */
	GMT_col_loop2 (API->GMT, G, col) xg[col] = GMT_grd_col_to_x (API->GMT, col, G->header);
	GMT_row_loop  (API->GMT, G, row) yg[G->header->ny-row-1] = GMT_grd_row_to_y (API->GMT, row, G->header);
}

char *GMTMEX_src_vector_init (struct GMTAPI_CTRL *API, const mxArray *prhs[], int n_cols, int n_start, struct GMT_VECTOR **V)
{	/* Used by programs that expect either an input file name or data vectors x, y[, other cols] */
	char *i_string = NULL;
	if (mxIsChar(prhs[0]))		/* Gave a file name */
		i_string = mxArrayToString (prhs[0]);	/* Load the file name into a char string */
 	else {				/* Input via two or more column vectors */
		GMT_LONG col, in_ID, dim[1] = {n_cols};
		//char buffer[GMT_BUFSIZ];
		i_string = mxMalloc (GMT_BUFSIZ);
		if ((*V = GMT_Create_Data (API, GMT_IS_VECTOR, dim)) == NULL) mexErrMsgTxt ("Failure to alloc GMT source vectors\n");
		for (col = n_start; col < n_cols+n_start; col++) {	/* Hook up one vector per column and determine data type */
			if (mxIsDouble(prhs[col])) {
				(*V)->type[col] = GMTAPI_DOUBLE;
				(*V)->data[col].f8 = mxGetData (prhs[col]);
			}
			else if (mxIsSingle(prhs[col])) {
				(*V)->type[col] = GMTAPI_FLOAT;
				(*V)->data[col].f4 = (float *)mxGetData (prhs[col]);
			}
			else if (mxIsInt32(prhs[col])) {
				(*V)->type[col] = GMTAPI_INT;
				(*V)->data[col].si4 = (int *)mxGetData (prhs[col]);
			}
			else if (mxIsInt16(prhs[col])) {
				(*V)->type[col] = GMTAPI_SHORT;
				(*V)->data[col].si2 = (short int *)mxGetData (prhs[col]);
			}
			else if (mxIsInt8(prhs[col])) {
				(*V)->type[col] = GMTAPI_CHAR;
				(*V)->data[col].sc1 = (char *)mxGetData (prhs[col]);
			}
			else
				mexErrMsgTxt ("Unsupported data type in GMT input.");
		}

		(*V)->n_rows = MAX (mxGetM (prhs[0]), mxGetN (prhs[0]));	/* So it works for both column or row vectors */
		(*V)->n_columns = n_cols;
		if ((in_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_READONLY + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN, V, NULL)) == GMTAPI_NOTSET) {
			mexErrMsgTxt ("Failure to register GMT source vectors\n");
		}
		if (GMT_Encode_ID (API, i_string, in_ID) != GMT_OK) {		/* Make filename with embedded object ID */
			mexErrMsgTxt ("GMTMEX_parser: Failure to encode string\n");
		}
		//i_string = strdup (buffer);
	}
	return (i_string);
}

char *GMTMEX_src_grid_init (struct GMTAPI_CTRL *API, const mxArray *prhs[], int nrhs)
{	/* Used by programs that expect either an input file name or matrix with info or data vectors x, y */
	char *i_string = NULL;
	struct GMT_GRID *G = NULL;
	if (nrhs == 2)		/* Gave a file name */
		i_string = mxArrayToString (prhs[0]);	/* Load the file name into a char string */
 	else {			/* Input via matrix and either info array or x,y arrays */
		GMT_LONG row, col, gmt_ij, in_ID;
		double *z = NULL;
		//char buffer[GMT_BUFSIZ];
		i_string = mxMalloc(GMT_BUFSIZ);

		if ((G = GMT_Create_Data (API, GMT_IS_GRID, NULL)) == NULL) mexErrMsgTxt ("Failure to create grid\n");
		GMT_grd_init (API->GMT, G->header, NULL, FALSE);
		
		/*  Get the Z array and fill in the header info */
		z = GMTMEX_info2grdheader (API, prhs, nrhs, G);
		/*  Allocate memory for the grid */
		G->data = GMT_memory (API->GMT, NULL, G->header->size, float);
		/* Transpose from Matlab orientation to grd orientation */
		GMT_grd_loop (API->GMT, G, row, col, gmt_ij) G->data[gmt_ij] = (float)z[MEX_IJ(G,row,col)];
		if ((in_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REF, GMT_IS_SURFACE, GMT_IN, G, NULL)) == GMTAPI_NOTSET) {
			mexErrMsgTxt ("Failure to register GMT source grid\n");
		}
		if (GMT_Encode_ID (API, i_string, in_ID) != GMT_OK) {	/* Make filename with embedded object ID */
			mexErrMsgTxt ("GMTMEX_parser: Failure to encode string\n");
		}
		//i_string = strdup (buffer);
	}
	return (i_string);
}

char *GMTMEX_dest_grid_init (struct GMTAPI_CTRL *API, GMT_LONG *out_ID, int nlhs, char *options)
{	/* Associate output grid with Matlab grid */
	char buffer[GMTAPI_STRLEN], *o_string = NULL;
	if ((*out_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REF, GMT_IS_SURFACE, GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) {
		mexErrMsgTxt ("Failure to register GMT destination grid\n");
	}
	if (nlhs == 0) {
		if (strstr (options, "-G")) 	/* User gave -G<file> among the options */
			return (NULL);		/* No output will be send to Matlab */
		else
			mexErrMsgTxt ("Error: neither -G option nor left hand side output args.");
	}
	o_string = mxMalloc(GMTAPI_STRLEN);
	if (GMT_Encode_ID (API, o_string, *out_ID) != GMT_OK) {	/* Make filename with embedded object ID */
		mexErrMsgTxt ("GMTMEX_parser: Failure to encode string\n");
	}
	//o_string = strdup (buffer);
	return (o_string);
}

char *GMTMEX_dest_vector_init (struct GMTAPI_CTRL *API, GMT_LONG n_cols, struct GMT_VECTOR **V, int nlhs, char *options)
{	/* Associate output data with Matlab/Octave vectors */
	char *o_string = NULL;
	GMT_LONG out_ID, col;
	//char buffer[GMTAPI_STRLEN];

	o_string = mxMalloc(GMTAPI_STRLEN);
	if (nlhs == 0) {
		if (strstr (options, ">")) 	/* User gave > file among the options */
			return (NULL);		/* No output will be send to Matlab */
		else
			mexErrMsgTxt ("Error: neither output file name with the '>' "
					"redirection operator nor left hand side output args.");
	}
	if ((*V = GMT_Create_Data (API, GMT_IS_VECTOR, &n_cols)) == NULL) mexErrMsgTxt ("Failure to alloc GMT source vectors\n");
	for (col = 0; col < n_cols; col++) (*V)->type[col] = GMTAPI_DOUBLE;
	(*V)->alloc_mode = GMT_REFERENCE;
	if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_REF + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_OUT, V, NULL)) == GMTAPI_NOTSET) {
		mexErrMsgTxt ("Failure to register GMT destination vectors\n");
	}
		
	if (GMT_Encode_ID (API, o_string, out_ID) != GMT_OK) {	/* Make filename with embedded object ID */
		mexErrMsgTxt ("GMTMEX_parser: Failure to encode string\n");
	}
	//o_string = strdup (buffer);
	return (o_string);
}

void GMTMEX_prep_mexgrd (struct GMTAPI_CTRL *API, mxArray *plhs[], int nlhs, struct GMT_GRID *G)
{	/* Turn a GMT grid into a 2-D Z matrix (and possibly x,y arrays) for passing back to Matlab/Octave */
	GMT_LONG px = -1, py = -1, pz = -1, pi = -1, row, col, gmt_ij;
	float *z = NULL;
	
	pz = (nlhs >= 3) ? 2 : 0;
	if (nlhs == 2 || nlhs == 4) pi = nlhs - 1;
	if (nlhs > 2) {px = 0; py = 1;}

	/* A. Create 2-D matrices for the return matrix */
	plhs[pz] = mxCreateNumericMatrix (G->header->ny, G->header->nx, mxSINGLE_CLASS, mxREAL);
	z = mxGetData (plhs[pz]);

	/* B. Load the real grd array into a double matlab array by
              transposing from padded GMT grd format to unpadded matlab format */

	GMT_grd_loop (API->GMT, G, row, col, gmt_ij) z[MEX_IJ(G,row,col)] = G->data[gmt_ij];
    
	/* C. Create header and x,y arrays, if requested  */

	if (pi >= 0) GMTMEX_grdheader2info (plhs, G, pi);	/* Also return info array */
	if (px >= 0) GMTMEX_grdxy (API, plhs, G, px, py);	/* Return x,y arrays also */
}

void GMTMEX_prep_mextbl (struct GMTAPI_CTRL *API, mxArray *plhs[], int nlhs, struct GMT_VECTOR *V)
{	/* We must duplicate output vectors since Matlab won't allow mixing of allocated arrays from outside Matlab */
	GMT_LONG row, p;
	double *z = NULL;
	for (p = 0; p < nlhs; p++) {
		plhs[p] = mxCreateNumericMatrix (V->n_rows, 1, mxDOUBLE_CLASS, mxREAL);
		z = mxGetData (plhs[p]);
		GMT_memcpy (z, V->data[p].f8, V->n_rows, double);
	}
}

char *GMTMEX_options_init (struct GMTAPI_CTRL *API, const mxArray *prhs[], int nrhs)
{	/* Secure string with the user's program options and parse -V, if included */
	int k = 1, j;
	char *options = NULL, *s = NULL;

	if (!mxIsChar(prhs[nrhs-1])) return (NULL);	/* No options in this case */
	options = mxArrayToString (prhs[nrhs-1]);

	if ((s = strstr (options, "-V"))) {	/* User gave -V[level] among the options */
		GMT_LONG level;
		s += 2;	/* Skip to char after -V */
		level = (isdigit ((int)s[0]) && s[0] >= '0' && s[0] <= '4') ? (GMT_LONG)(s[0] - '0') : GMT_MSG_NORMAL;
		API->GMT->current.setting.verbose = level;
	}
	while (options[k] && !(options[k] == '>' && options[k-1] == ' ')) k++;		/* Test if ... */
	if (k < strlen(options) - 1) {			/* User used the "... > file ..." construct */
		int	len;
		len = strlen(options);
		options = mxRealloc(options, len+1);	/* Increase by one because we need to insert a '-' */
		for (j = len; j > k; j--)
			options[j] = options[j-1];	/* Make room to the to-be-inseted '-' */
		k++;
		options[k-1] = '-';
		while (options[k+1] == ' ') {	/* Remove all spaces between the '>' and filename */
			for (j = k+1; j < strlen(options)-1; j++)
				options[j] = options[j+1];
			options[j] = '\0';
		}
	}
	/* Now test for this mistake "... -> file ...". Note that now no need to expand the options string */
	else if ((s = strstr (options, "-> "))) {
		while (s[2] == ' ') {		/* Remove all spaces between the '>' and filename */
			for (j = 2; j < strlen(s)-1; j++)
				s[j] = s[j+1];
			s[j] = '\0';
		}
	}
	return (options);
}

char *GMTMEX_build_cmd (struct GMTAPI_CTRL *API, char *src, char *options, char *dest, GMT_LONG mode)
{	/* Create the command based on options, src, and dist, which depends slightly on output type */
	char *cmd;
	cmd = mxMalloc (GMT_BUFSIZ);
	if (mode == GMT_IS_GRID) {
		if (dest)
			sprintf (cmd, "%s %s -G%s", src, options, dest);
		else
			sprintf (cmd, "%s %s", src, options);
	}
	else if (mode == GMT_IS_DATASET)
		if (dest)
			sprintf (cmd, "%s %s > %s", src, options, dest);
		else
			sprintf (cmd, "%s %s", src, options);
	else {	/* PostScript */
		(src) ? sprintf (cmd, "%s %s", src, options) : sprintf (cmd, "%s", options);
	}
	return (cmd);
}

void GMTMEX_free (char *input, char *output, char *options, char *cmd) {
	/* Free temporary local variables */
	if (input) mxFree (input);
	if (output) mxFree (output);	
	if (options) mxFree (options);	
	mxFree (cmd);
}

#ifdef NOTYET

/* New parser for all GMT mex modules based on design discussed by PW and JL on Mon, 2/21/11 */
/* Wherever we say "Matlab" we mean "Matlab of Octave" */

/* For the Mex interface we will wish to pass either filenames or matrices via GMT command options.
 * We select a Matlab matrix by suppying $ as the file name.  The parser will then find these $
 * arguments and replace them with references to the arrays via the GMT API mechanisms.
 * This requires us to know which options in a module may accept a file name.  As an example,
 * consider surface whose -L option may take a grid.  To pass a Matlab/Octave grid already in memory
 * we would use -L$ and give the grid as an argument to the module, e.g.,
 * Z = surface ('-R0/50/0/50 -I1 -V xyzfile -L$', lowmatrix);
 * For each option that may take a file we need to know what kind of file and if this is input or output.
 * We encode this in a 3-character word, where the first char is the option, the second is the data type,
 * and the third is I(n) or O(out).  E.g., the surface example would have the word LGI.  The data types
 * are P(olygons), L(ines), D(point data), G(rid), C(PT file), T(ext table). [We originally only had
 * D for datasets but perhaps the geometry needs to be passed too (if known); hence the P|L|D char]
 * In addition, the only common option that might take a file is -R which may take a grid as input.
 * We check for that in addition to the module-specific info passed via the key variable.
 *
 * The actual reading/writing will occur in gmtapi_util.c where we will add a case MEX: for each type.
 * and here we will use mx* for allocation for stuff that is sent to Matlab, and GMT_memory for things
 * that are read and reformatted from Matlab.  This includes packing up GMT grids into Matlab grid structs
 * and vice versa.
 */

#define GMT_MEX_EXPLICIT	-2
#define GMT_MEX_IMPLICIT	-1

GMT_LONG gmtmex_find_option (char option, char *key[], GMT_LONG n_keys) {
	/* gmtmex_find_option determines if the given option is among the special options that might take $ as filename */
	GMT_LONG pos = -1, k;
	for (k = 0; pos == -1 && k < n_keys; k++) if (key[k][0] == option) pos = k;
	return (pos);	/* -1 if not found, otherwise the position in the key array */
}

GMT_LONG gmtmex_get_arg_pos (char *arg)
{	/* Look for a $ in the arg; if found return position, else return -1. Skips $ inside quoted texts */
	GMT_LONG pos, mute = FALSE, k;
	for (k = 0, pos = -1; pos == -1 && k < strlen (arg); k++) {
		if (arg[k] == '\"' || arg[k] == '\'') mute = !mute;	/* Do not consider $ inside quotes */
		if (!mute && arg[k] == '$') pos = k;	/* Found a $ sign */
	}
	return (pos);	/* Either -1 (not found) or in the 0-(strlen(arg)-1) range [position of $] */
}

void gmtmex_get_key_pos (char *key[], GMT_LONG n_keys, struct GMT_OPTIONS *head, GMT_LONG def[])
{	/* Must determine if default input and output have been set via program options or if they should be added explicitly.
 	 * As an example, consider the GMT command grdfilter in.nc -Fg200k -Gfilt.nc.  In Matlab this might be
	 * filt = GMT_grdfilter ('$ -Fg200k -G$', in);
	 * However, it is more natural not to specify the lame -G$, i.e.
	 * filt = GMT_grdfilter ('$ -Fg200k', in);
	 * In that case we need to know that -G is the default way to specify the output grid and if -G is not given we
	 * must associate -G with the first left-hand-side item (here filt).
	 */
	GMT_LONG pos, k;
	struct GMT_OPTIONS *opt = NULL;
	def[GMT_IN] = def[GMT_OUT] = GMT_MEX_IMPLICIT;	/* Initialize to setting the i/o implicitly */
	
	for (opt = head; opt; opt = opt->next) {	/* Loop over the module options to see if inputs and outputs are set explicitly or implicitly */
		pos = gmtmex_find_option (opt->option, key, n_keys);	/* First see if this option is one that might take $ */
		if (pos == -1) continue;		/* No, it was some other harmless option, e.g., -J, -O ,etc. */
		/* Here, the current option is one that might take an input or output file. See if it matches
		 * the UPPERCASE I or O [default source/dest] rather than the standard i|o (optional input/output) */
		if (key[pos][2] == 'I') def[GMT_IN]  = GMT_MEX_EXPLICIT;	/* Default input  is actually set explicitly via option setting now indicated by key[pos] */
		if (key[pos][2] == 'O') def[GMT_OUT] = GMT_MEX_EXPLICIT;	/* Default output is actually set explicitly via option setting now indicated by key[pos] */
	}
	/* Here, if def[] == GMT_MEX_IMPLICIT (the default in/out option was NOT given), then we want to return the corresponding entry in key */
	for (pos = 0; pos < n_keys; pos++) {	/* For all module options that might take a file */
		if (key[pos][2] == 'I' && def[GMT_IN]  == GMT_MEX_IMPLICIT) def[GMT_IN]  = pos;	/* Must add implicit input; use def to determine option,type */
		if (key[pos][2] == 'O' && def[GMT_OUT] == GMT_MEX_IMPLICIT) def[GMT_OUT] = pos;	/* Must add implicit output; use def to determine option,type */
	}
}

GMT_LONG gmtmex_get_arg_dir (char option, char *key[], GMT_LONG n_keys, GMT_LONG *data_type, GMT_LONG *geometry)
{
	GMT_LONG item;
	
	/* 1. First determine if this option is one of the choices in key */
	
	item = gmtmex_find_option (option, key, n_keys);
	if (item == -1) mexErrMsgTxt ("GMTMEX_parser: This option does not allow $ arguments\n");	/* This means a coding error we must fix */
	
	/* 2. Assign direction, data_type, and geometry */
	
	switch (key[item][1]) {	/* 2nd char contains the data type code */
		case 'G':
			*data_type = GMT_IS_GRID;
			*geometry = GMT_IS_SURFACE;
			break;
		case 'P':
			*data_type = GMT_IS_DATASET;
			*geometry = GMT_IS_POLY;
			break;
		case 'L':
			*data_type = GMT_IS_DATASET;
			*geometry = GMT_IS_LINE;
			break;
		case 'D':
			*data_type = GMT_IS_DATASET;
			*geometry = GMT_IS_POINT;
			break;
		case 'C':
			*data_type = GMT_IS_CPT;
			*geometry = GMT_IS_TEXT;
			break;
		case 'T':
			*data_type = GMT_IS_TEXTSET;
			*geometry = GMT_IS_TEXT;
			break;
		case 'I':
			*data_type = GMT_IS_IMAGE;
			*geometry = GMT_IS_SURFACE;
			break;
		default:
			mexErrMsgTxt ("GMTMEX_parser: Bad data_type character in 3-char module code!\n");
			break;
	}
	/* Third key character contains the in/out code */
	if (key[item][2] == 'I') key[item][2] == 'i';	/* This was the default input option set explicitly; no need to add later */
	if (key[item][2] == 'O') key[item][2] == 'o';	/* This was the default output option set explicitly; no need to add later */
	return ((key[item][2] == 'i') ? GMT_IN : GMT_OUT);
}

GMT_LONG GMTMEX_parser (struct GMTAPI_CTRL *API, mxArray *plhs[], int nlhs, const mxArray *prhs[], int nrhs, char **key, GMT_LONG n_keys, struct GMT_OPTIONS **head)
{
	/* API controls all things within GMT.
	 * plhs (and nlhs) are the outputs specified on the left side of the equal sign in Matlab.
	 * prhs (and nrhs) are the inputs specified after the option string in the GMT-mex function.
	 * key is array with 3-char codes for current module i/o.
	 * opt is the linked list of GMT options passed in.
	 */
	
	GMT_LONG lr_pos[2] = {0, 0};	/* These position keeps track where we are in the L and R pointer arrays */
	GMT_LONG direction;		/* Either GMT_IN or GMT_OUT */
	GMT_LONG data_type;		/* Either GMT_IS_DATASET, GMT_IS_TEXTSET, GMT_IS_GRID, GMT_IS_CPT, GMT_IS_IMAGE */
	GMT_LONG geometry;		/* Either GMT_IS_TEXT, GMT_IS_POINT, GMT_IS_LINE, GMT_IS_POLY, or GMT_IS_SURFACE */
	GMT_LONG def[2];		/* Either GMT_MEX_EXPLICIT or the item number in the keys array */
	char name[GMTAPI_STRLEN];	/* Used to hold the GMT API embedded file name, e.g., @GMTAPI@-###### */
	char buffer[GMT_BUFSIZ];	/* Temp buffer */
	struct GMT_OPTIONS *opt;	/* Pointer to a GMT option structure */
	void *ptr = NULL;		/* Void pointer used to point to either L or R side pointer argument */
	
	gmtmex_get_key_pos (key, n_keys, head, def);	/* Determine if we must add the primary in and out arguments to the option list */
	for (direction = GMT_IN; direction <= GMT_OUT; direction++) {
		if (def[direction] == GMT_MEX_EXPLICIT) continue;	/* Source or destination was set explicitly; skip */
		/* Must add the primary input or output from prhs[0] or plhs[0] */
		(void)gmtmex_get_arg_dir (key[def[direction]][0], key, n_keys, &data_type, &geometry);		/* Get info about the data set */
		ptr = (direction == GMT_IN) ? prhs[lr_pos[direction]] : lrhs[lr_pos[direction]];	/* Pick the next left or right side pointer */
		/* Register a Matlab/Octave entity as a source or destination */
		if ((ID = GMT_Register_IO (API, data_type, GMT_IS_REF + GMT_VIA_MEX, geometry, direction, ptr, NULL)) == GMTAPI_NOTSET) {
			mexErrMsgTxt ("GMTMEX_parser: Failure to register GMT source or destination\n");
		}
		lr_pos[direction]++;		/* Advance counter for next time */
		if (GMT_Encode_ID (API, name, ID) != GMT_OK) {	/* Make filename with embedded object ID */
			mexErrMsgTxt ("GMTMEX_parser: Failure to encode string\n");
		}
		GMT_Make_Option (API, key[def[direction]][0], name, &new_ptr);	/* Create the missing (implicit) GMT option */
		GMT_Append_Option (API, new_ptr, head);				/* Append it to the option list */
	}
		
	for (opt = *head; opt; opt = opt->next) {	/* Loop over the module options given */
		/* Determine if this option as a $ in its argument and if so return its position in pos; return -1 otherwise */
		if ((pos = gmtmex_get_arg_pos (opt->arg)) == -1) continue;	/* No $ argument found or it is part of a text string */
		
		/* Determine several things about this option, such as direction, data type, method, and geometry */
		direction == gmtmex_get_arg_dir (opt->option, key, n_keys, &data_type, &geometry);
		ptr = (direction == GMT_IN) ? prhs[lr_pos[direction]] : lrhs[lr_pos[direction]];	/* Pick the next left or right side pointer */
		/* Register a Matlab/Octave entity as a source or destination */
		if ((ID = GMT_Register_IO (API, data_type, GMT_IS_REF + GMT_VIA_MEX, geometry, direction, ptr, NULL)) == GMTAPI_NOTSET) {
			mexErrMsgTxt ("GMTMEX_parser: Failure to register GMT source or destination\n");
		}
		if (GMT_Encode_ID (API, name, ID) != GMT_OK) {	/* Make filename with embedded object ID */
			mexErrMsgTxt ("GMTMEX_parser: Failure to encode string\n");
		}
		lr_pos[direction]++;		/* Advance counter for next time */
		
		/* Replace the option argument with the embedded file */
		opt->arg[pos] = '\0';	/* Chop off the stuff starting at the $ sign */
		sprintf (buffer, "%s%s", opt->arg, name);	/* Make a new option argument that replaces the $ with name */
		opt->arg[pos] = '$';	/* Restore the $ sign in the old argument */
		free (opt->arg);	/* Free the old option argument */
		opt->arg = strdup (buffer);	/* Allocate and set the new argument with the embedded filename */
	}
	/* Here, a command line '-F200k -G$ $ -L$ -P' has been changed to '-F200k -G@GMTAPI@-000001 @GMTAPI@-000002 -L@GMTAPI@-000003 -P'
	 * where the @GMTAPI@-00000x are encodings to registered resources or destinations */
}
#endif
