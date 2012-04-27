/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: testapi allows us to test the API i/o functions.
 *
 */

#include "gmt.h"

/* Control structure for testapi */

struct TESTAPI_CTRL {
	struct T {	/* -T sets data type */
		GMT_LONG active;
		GMT_LONG mode;
	} T;
	struct I {	/* -I sets input method */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG via;
	} I;
	struct W {	/* -W sets output method */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG via;
	} W;
};

void *New_testapi_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TESTAPI_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct TESTAPI_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return (C);
}

void Free_testapi_Ctrl (struct GMT_CTRL *GMT, struct TESTAPI_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);
}

GMT_LONG GMT_testapi_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "testapi %s [API] - test API i/o methods for any data type\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: testapi -If|s|d|c|r[/v|m] -Td|t|g|c|i|v|m -Wf|s|d|c|r[/v|m] [%s]\n", GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-I Specify input resource.  Choose among:\n");
	GMT_message (GMT, "\t   f : File\n");
	GMT_message (GMT, "\t   s : Stream\n");
	GMT_message (GMT, "\t   d : File descriptor\n");
	GMT_message (GMT, "\t   c : Memory Copy\n");
	GMT_message (GMT, "\t   r : Memory Reference\n");
	GMT_message (GMT, "\t   Optionally, append /v or /m to c|r to get data via vector or matrix.\n");
	GMT_message (GMT, "\t   This is only valid for -Td|g.\n");
	GMT_message (GMT, "\t-T Specify data type.  Choose among:\n");
	GMT_message (GMT, "\t   d : Dataset\n");
	GMT_message (GMT, "\t   t : Textset\n");
	GMT_message (GMT, "\t   g : Grid\n");
	GMT_message (GMT, "\t   C : CPT\n");
	GMT_message (GMT, "\t   i : Image\n");
	GMT_message (GMT, "\t   v : Vector\n");
	GMT_message (GMT, "\t   m : Matrix\n");
	GMT_message (GMT, "\t-W Specify write destination.  Choose among:\n");
	GMT_message (GMT, "\t   f : File\n");
	GMT_message (GMT, "\t   s : Stream\n");
	GMT_message (GMT, "\t   d : File descriptor\n");
	GMT_message (GMT, "\t   c : Memory Copy\n");
	GMT_message (GMT, "\t   r : Memory Reference\n");
	GMT_message (GMT, "\t   Optionally, append /v or /m to c|r to put data via vector or matrix.\n");
	GMT_message (GMT, "\t   This is only valid for -Td|g.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "V.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_testapi_parse (struct GMTAPI_CTRL *C, struct TESTAPI_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Processes program-specific parameters */

			case 'I':	/* Input */
				Ctrl->I.active = TRUE;
				switch (opt->arg[0]) {
					case 'f': Ctrl->I.mode = GMT_IS_FILE; break;
					case 's': Ctrl->I.mode = GMT_IS_STREAM; break;
					case 'd': Ctrl->I.mode = GMT_IS_FDESC; break;
					case 'c': Ctrl->I.mode = GMT_IS_COPY; break;
					case 'r': Ctrl->I.mode = GMT_IS_REF; break;
				}
				if (opt->arg[1] == '/' && opt->arg[2] == 'v') Ctrl->I.via = GMT_VIA_VECTOR;
				if (opt->arg[1] == '/' && opt->arg[2] == 'm') Ctrl->I.via = GMT_VIA_MATRIX;
				break;
			case 'T':	/* Type */
				Ctrl->T.active = TRUE;
				switch (opt->arg[0]) {
					case 'd': Ctrl->T.mode = GMT_IS_DATASET; break;
					case 't': Ctrl->T.mode = GMT_IS_TEXTSET; break;
					case 'g': Ctrl->T.mode = GMT_IS_GRID; break;
					case 'c': Ctrl->T.mode = GMT_IS_CPT; break;
					case 'i': Ctrl->T.mode = GMT_IS_IMAGE; break;
					case 'v': Ctrl->T.mode = GMT_IS_VECTOR; break;
					case 'm': Ctrl->T.mode = GMT_IS_MATRIX; break;
				}
				break;

			case 'W':	/* Output */
				Ctrl->W.active = TRUE;
				switch (opt->arg[0]) {
					case 'f': Ctrl->W.mode = GMT_IS_FILE; break;
					case 's': Ctrl->W.mode = GMT_IS_STREAM; break;
					case 'd': Ctrl->W.mode = GMT_IS_FDESC; break;
					case 'c': Ctrl->W.mode = GMT_IS_COPY; break;
					case 'r': Ctrl->W.mode = GMT_IS_REF; break;
				}
				if (opt->arg[1] == '/' && opt->arg[2] == 'v') Ctrl->W.via = GMT_VIA_VECTOR;
				if (opt->arg[1] == '/' && opt->arg[2] == 'm') Ctrl->W.via = GMT_VIA_MATRIX;
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->I.via == GMT_VIA_VECTOR && Ctrl->T.mode != GMT_IS_DATASET && !(Ctrl->I.mode == GMT_IS_COPY || Ctrl->I.mode == GMT_IS_REF)) n_errors++;
	if (Ctrl->I.via == GMT_VIA_MATRIX && !(Ctrl->T.mode == GMT_IS_DATASET || Ctrl->T.mode == GMT_IS_GRID) && !(Ctrl->I.mode == GMT_IS_COPY || Ctrl->I.mode == GMT_IS_REF)) n_errors++;
	if (Ctrl->W.via == GMT_VIA_VECTOR && Ctrl->T.mode != GMT_IS_DATASET && !(Ctrl->W.mode == GMT_IS_COPY || Ctrl->W.mode == GMT_IS_REF)) n_errors++;
	if (Ctrl->W.via == GMT_VIA_MATRIX && !(Ctrl->T.mode == GMT_IS_DATASET || Ctrl->T.mode == GMT_IS_GRID) && !(Ctrl->W.mode == GMT_IS_COPY || Ctrl->W.mode == GMT_IS_REF)) n_errors++;
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_testapi_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_testapi (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = 0, in_ID, out_ID, par[1] = {2}, via[2] = {0, 0};
	GMT_LONG geometry[7] = {GMT_IS_POINT, GMT_IS_TEXT, GMT_IS_SURFACE, GMT_IS_TEXT, GMT_IS_SURFACE, GMT_IS_POINT, GMT_IS_SURFACE};
	uint64_t k;
	
	float *fdata = NULL;
	double *ddata = NULL;
	
	char *ikind[7] = {"DATASET", "TEXTSET", "GRID", "CPT", "IMAGE", "VECTOR", "MATRIX"};
	char *method[6] = {"FILE", "STREAM", "FDESC", "COPY", "REF", "READONLY"};
	char *append[3] = {"", " via VECTOR", " via MATRIX"};
	char *ifile[7] = {"dtesti.txt", "ttesti.txt", "gtesti.nc", "ctesti.cpt", "itesti.jpg", "vtesti.bin", "mtesti.bin"};
	char *ofile[7] = {"dtesto.txt", "ttesto.txt", "gtesto.nc", "ctesto.cpt", "itesto.jpg", "vtesto.bin", "mtesto.bin"};
	char string[GMTAPI_STRLEN];

	FILE *fp = NULL;
	int *fdp = NULL, fd = 0;
	
	struct TESTAPI_CTRL *Ctrl = NULL;
	struct GMT_MATRIX *M = NULL;
	struct GMT_VECTOR *V = NULL;
	void *In = NULL, *Out = NULL, *Intmp = NULL, *Outtmp = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_testapi_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_testapi_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_testapi", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-V", ">", options)) Return (API->error);
	Ctrl = New_testapi_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_testapi_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the testapi main code ----------------------------*/

	via[GMT_IN] = Ctrl->I.via / 100;	via[GMT_OUT] = Ctrl->W.via / 100;
	if (Ctrl->I.via == GMT_VIA_MATRIX) {	/* We will use a matrix in memory as data source */
		if ((M = GMT_Create_Data (API, GMT_IS_MATRIX, NULL)) == NULL) Return (API->error);
		if (Ctrl->T.mode == GMT_IS_DATASET) {	/* Mimic the dtest.txt table */
			M->n_rows = 9;	M->n_columns = 2;	M->n_layers = 1;	M->dim = 9;	M->type = GMTAPI_FLOAT;	M->size = M->n_rows * M->n_columns * M->n_layers;
			fdata = GMT_memory (GMT, NULL, M->size, float);
			for (k = 0; k < (uint64_t)M->n_rows; k++) {
				fdata[2*k] = (float)k;	fdata[2*k+1] = (float)k*10;
			}
			fdata[0] = fdata[1] = fdata[8] = fdata[9] = GMT->session.f_NaN;
		}
		else {	/* Mimic the gtest.nc grid as table */
			M->n_rows = 6;	M->n_columns = 6;	M->n_layers = 1;	M->dim = 6;	M->type = GMTAPI_FLOAT;	M->size = M->n_rows * M->n_columns * M->n_layers;
			M->limit[XLO] = 0.0;	M->limit[XHI] = 5.0;	M->limit[YLO] = 0.0;	M->limit[YHI] = 5.0;	M->limit[4] = 0.0;	M->limit[5] = 25.0;
			fdata = GMT_memory (GMT, NULL, M->size, float);
			for (k = 0; k < M->size; k++) fdata[k] = (float)((int)(k%M->n_columns + (M->n_columns - 1 - k/M->n_columns) * M->n_rows));
		}
		M->data.f4 = fdata;
	}
	else if (Ctrl->I.via == GMT_VIA_VECTOR) {	/* We will use vectors in memory as data source */
		if ((V = GMT_Create_Data (API, GMT_IS_VECTOR, par)) == NULL) Return (API->error);
		V->n_rows = 9;
		fdata = GMT_memory (GMT, NULL, V->n_rows, float);
		ddata = GMT_memory (GMT, NULL, V->n_rows, double);
		for (k = 0; k < V->n_rows; k++) {
			fdata[k] = (float)k;	ddata[k] = k*10.0;
		}
		fdata[0] = fdata[4] = GMT->session.f_NaN;
		ddata[0] = ddata[4] = GMT->session.d_NaN;
		V->data[GMT_X].f4 =  fdata;	V->type[GMT_X] = GMTAPI_FLOAT;
		V->data[GMT_Y].f8 =  ddata;	V->type[GMT_Y] = GMTAPI_DOUBLE;
	}
	
	/* Get input and register it */
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Read %s %s with method %s%s and write to %s with method %s%s\n", ikind[Ctrl->T.mode], ifile[Ctrl->T.mode], method[Ctrl->I.mode], append[via[GMT_IN]], ofile[Ctrl->T.mode], method[Ctrl->W.mode], append[via[GMT_OUT]]);
	
	if (GMT_Init_IO (API, Ctrl->T.mode, geometry[Ctrl->T.mode], GMT_IN, GMT_REG_FILES_IF_NONE, options) != GMT_OK) {	/* Registers default input destination, unless already set */
		Return (API->error);
	}

	if (Ctrl->T.mode == GMT_IS_IMAGE) GMT_set_pad (GMT, 0);	/* Temporary turn off padding (and thus BC setting) since we will use image exactly as is */
	switch (Ctrl->I.mode) {
		case GMT_IS_FILE:	/* Pass filename */
			if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, ifile[Ctrl->T.mode], NULL)) == GMTAPI_NOTSET) {
				Return (API->error);
			}
			break;
		case GMT_IS_STREAM:
			switch (Ctrl->T.mode) {	/* Can only do d, t, c */
				case GMT_IS_DATASET: case GMT_IS_TEXTSET: case GMT_IS_CPT:
					fp = GMT_fopen (GMT, ifile[Ctrl->T.mode], "r");
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, fp, NULL)) == GMTAPI_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_report (GMT, GMT_MSG_FATAL, "GMT_IS_STREAM only allows d, t, c!\n");
					Return (GMT_WRONG_KIND);
					break;
			}
			break;
		case GMT_IS_FDESC:
			switch (Ctrl->T.mode) {	/* Can only do d, t, c */
				case GMT_IS_DATASET: case GMT_IS_TEXTSET: case GMT_IS_CPT:
					fd = open (ifile[Ctrl->T.mode], O_RDONLY);
					fdp = &fd;
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, fdp, NULL)) == GMTAPI_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_report (GMT, GMT_MSG_FATAL, "GMT_IS_FDESC only allows d, t, c!\n");
					Return (GMT_WRONG_KIND);
					break;
			}
			break;
		case GMT_IS_COPY: case GMT_IS_REF:
			switch (Ctrl->I.via) {
				case GMT_VIA_MATRIX:	/* Get the dataset|grid via a user matrix */
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, GMT_IS_COPY + Ctrl->I.via, geometry[Ctrl->T.mode], GMT_IN, M, NULL)) == GMTAPI_NOTSET) {
						Return (API->error);
					}
					if ((Intmp = GMT_Get_Data (API, in_ID, 0, NULL)) == NULL) {
						Return (API->error);
					}
					break;
				case GMT_VIA_VECTOR:	/* Get the dataset|grid via a user vectors */
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, GMT_IS_COPY + Ctrl->I.via, geometry[Ctrl->T.mode], GMT_IN, V, NULL)) == GMTAPI_NOTSET) {
						Return (API->error);
					}
					if ((Intmp = GMT_Get_Data (API, in_ID, 0, NULL)) == NULL) {
						Return (API->error);
					}
					break;
				default:		/* Get directly from file */
					if ((Intmp = GMT_Read_Data (API, Ctrl->T.mode, GMT_IS_FILE, geometry[Ctrl->T.mode], NULL, 0, ifile[Ctrl->T.mode], NULL)) == NULL) {
						Return (API->error);
					}
					break;
			}
			if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, Intmp, NULL)) == GMTAPI_NOTSET) {
				Return (API->error);
			}
			break;
		default:
			GMT_report (GMT, GMT_MSG_FATAL, "Bad Input mode\n");
			Return (GMT_WRONG_KIND);
			break;
	}
		
	/* Now get the data from the registered source */
	
	if ((In = GMT_Get_Data (API, in_ID, 0, NULL)) == NULL) {
		Return (API->error);
	}
	if (Ctrl->T.mode == GMT_IS_IMAGE) GMT_set_pad (GMT, 2);	/* Reset to GMT default */
	
	if (Ctrl->T.mode == GMT_IS_IMAGE) {	/* Since writing is not supported we just make a plot via GMT_psimage */
		char buffer[GMT_BUFSIZ];
		if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, GMT_IS_REF, geometry[Ctrl->T.mode], GMT_IN, In, NULL)) == GMTAPI_NOTSET) {
			Return (API->error);
		}
		if (GMT_Encode_ID (API, string, in_ID) != GMT_OK) {
			Return (API->error);	/* Make filename with embedded object ID */
		}
		sprintf (buffer, "%s -W6i -P -F0.25p --PS_MEDIA=letter --PS_CHAR_ENCODING=Standard+", string);
		if (GMT_psimage (API, 0, buffer) != GMT_OK) {	/* Plot the image */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, GMT_CLOBBER, &Intmp) != GMT_OK) {
			Return (API->error);
		}
		GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");
		Return (GMT_OK);
	}
	
	/* Get output and register it */
	
	switch (Ctrl->W.mode) {
		case GMT_IS_FILE:	/* Pass filename */
			if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, ofile[Ctrl->T.mode], NULL)) == GMTAPI_NOTSET) {
				Return (API->error);
			}
			break;
		case GMT_IS_STREAM:
			switch (Ctrl->T.mode) {	/* Can only do d, t, c */
				case GMT_IS_DATASET: case GMT_IS_TEXTSET: case GMT_IS_CPT:
					fp = GMT_fopen (GMT, ofile[Ctrl->T.mode], "w");
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, fp, NULL)) == GMTAPI_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_report (GMT, GMT_MSG_FATAL, "GMT_IS_STREAM only allows d, t, c!\n");
					Return (GMT_WRONG_KIND);
					break;
			}
			break;
		case GMT_IS_FDESC:
			switch (Ctrl->T.mode) {	/* Can only do d, t, c */
				case GMT_IS_DATASET: case GMT_IS_TEXTSET: case GMT_IS_CPT:
#ifdef WIN32
					/* I think they exist on Win too, but no mutch time to find out how. JL */
					fd = open (ofile[Ctrl->T.mode], O_WRONLY | O_CREAT);
#else
					fd = open (ofile[Ctrl->T.mode], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
#endif
					fdp = &fd;
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, fdp, NULL)) == GMTAPI_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_report (GMT, GMT_MSG_FATAL, "GMT_IS_FDESC only allows d, t, c!\n");
					Return (GMT_WRONG_KIND);
					break;
			}
			break;
		case GMT_IS_COPY: case GMT_IS_REF:
			switch (Ctrl->W.via) {
				case GMT_VIA_MATRIX:	/* Put the dataset|grid via a user matrix */
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode + Ctrl->W.via, geometry[Ctrl->T.mode], GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) {
						Return (API->error);
					}
					break;
				case GMT_VIA_VECTOR:	/* Put the dataset|grid via a user vector */
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode + Ctrl->W.via, geometry[Ctrl->T.mode], GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) {
						Return (API->error);
					}
					break;
			}
			break;
		default:
			GMT_report (GMT, GMT_MSG_FATAL, "Bad Input mode\n");
			Return (GMT_WRONG_KIND);
			break;
	}
			
	/* Now put the data to the registered destination */
	
	if (GMT_Init_IO (API, Ctrl->T.mode, geometry[Ctrl->T.mode], GMT_OUT, GMT_REG_FILES_IF_NONE, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Put_Data (API, out_ID, 0, In) != GMT_OK) {
		Return (API->error);
	}
	
	if (Ctrl->W.mode == GMT_IS_COPY || Ctrl->W.mode == GMT_IS_REF) {	/* Must write out what is in memory to the file */
		if ((Out = GMT_Retrieve_Data (API, out_ID)) == NULL) {
			Return (API->error);
		}
		if (Ctrl->W.via) {	/* Must first read into proper GMT container, then write to file */
			if ((Outtmp = GMT_Read_Data (API, Ctrl->T.mode, GMT_IS_COPY + Ctrl->W.via, geometry[Ctrl->T.mode], NULL, 0, Out, NULL)) == NULL) {
				Return (API->error);
			}
			if (GMT_Write_Data (API, Ctrl->T.mode, GMT_IS_FILE, geometry[Ctrl->T.mode], NULL, 0, ofile[Ctrl->T.mode], Outtmp) != GMT_OK) {
				Return (API->error);
			}
		}
		else {
			if (GMT_Write_Data (API, Ctrl->T.mode, GMT_IS_FILE, geometry[Ctrl->T.mode], NULL, 0, ofile[Ctrl->T.mode], Out) != GMT_OK) {
				Return (API->error);
			}
		}
	}
	
	if (GMT_Destroy_Data (API, GMT_CLOBBER, &Intmp) != GMT_OK) {
		Return (API->error);
	}
	if (GMT_Destroy_Data (API, GMT_CLOBBER, &M) != GMT_OK) {
		Return (API->error);
	}
	if (GMT_Destroy_Data (API, GMT_CLOBBER, &V) != GMT_OK) {
		Return (API->error);
	}
	if (!(Ctrl->I.mode == GMT_IS_REF && Ctrl->W.mode == GMT_IS_REF) && GMT_Destroy_Data (API, GMT_CLOBBER, &Out) != GMT_OK) {
		Return (API->error);
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");
	Return (GMT_OK);
}
