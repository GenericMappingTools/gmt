/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->Vh"

/* Control structure for testapi */

struct TESTAPI_CTRL {
	struct T {	/* -T sets data type */
		bool active;
		int mode;
	} T;
	struct I {	/* -I sets input method */
		bool active;
		unsigned int mode;
		enum GMT_enum_via via;
	} I;
	struct W {	/* -W sets output method */
		bool active;
		unsigned int mode;
		enum GMT_enum_via via;
	} W;
};

void *New_testapi_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TESTAPI_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct TESTAPI_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_testapi_Ctrl (struct GMT_CTRL *GMT, struct TESTAPI_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);
}

int GMT_testapi_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_Message (API, GMT_TIME_NONE, "testapi - test API i/o methods for any data type\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: testapi -If|s|d|c|r[/v|m] -Td|t|g|c|i|v|m -Wf|s|d|c|r[/v|m] [%s] [%s]\n", GMT_V_OPT, GMT_h_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-I Specify input resource.  Choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f : File\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s : Stream\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d : File descriptor\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c : Memory Copy\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r : Memory Reference\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append /v or /m to c|r to get data via vector or matrix.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This is only valid for -Td|g.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify data type.  Choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d : Dataset\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t : Textset\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g : Grid\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   C : CPT\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i : Image\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   v : Vector\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m : Matrix\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Specify write destination.  Choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f : File\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s : Stream\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d : File descriptor\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c : Memory Copy\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r : Memory Reference\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append /v or /m to c|r to put data via vector or matrix.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This is only valid for -Td|g.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V,h,.");
	
	return (EXIT_FAILURE);
}

int GMT_testapi_parse (struct GMT_CTRL *GMT, struct TESTAPI_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Processes program-specific parameters */

			case 'I':	/* Input */
				Ctrl->I.active = true;
				switch (opt->arg[0]) {
					case 'f': Ctrl->I.mode = GMT_IS_FILE; break;
					case 's': Ctrl->I.mode = GMT_IS_STREAM; break;
					case 'd': Ctrl->I.mode = GMT_IS_FDESC; break;
					case 'c': Ctrl->I.mode = GMT_IS_DUPLICATE; break;
					case 'r': Ctrl->I.mode = GMT_IS_REFERENCE; break;
				}
				if (opt->arg[1] == '/' && opt->arg[2] == 'v') Ctrl->I.via = GMT_VIA_VECTOR;
				if (opt->arg[1] == '/' && opt->arg[2] == 'm') Ctrl->I.via = GMT_VIA_MATRIX;
				break;
			case 'T':	/* Type */
				Ctrl->T.active = true;
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
				Ctrl->W.active = true;
				switch (opt->arg[0]) {
					case 'f': Ctrl->W.mode = GMT_IS_FILE; break;
					case 's': Ctrl->W.mode = GMT_IS_STREAM; break;
					case 'd': Ctrl->W.mode = GMT_IS_FDESC; break;
					case 'c': Ctrl->W.mode = GMT_IS_DUPLICATE; break;
					case 'r': Ctrl->W.mode = GMT_IS_REFERENCE; break;
				}
				if (opt->arg[1] == '/' && opt->arg[2] == 'v') Ctrl->W.via = GMT_VIA_VECTOR;
				if (opt->arg[1] == '/' && opt->arg[2] == 'm') Ctrl->W.via = GMT_VIA_MATRIX;
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->I.via == GMT_VIA_VECTOR && Ctrl->T.mode != GMT_IS_DATASET && !(Ctrl->I.mode == GMT_IS_DUPLICATE || Ctrl->I.mode == GMT_IS_REFERENCE)) n_errors++;
	if (Ctrl->I.via == GMT_VIA_MATRIX && !(Ctrl->T.mode == GMT_IS_DATASET || Ctrl->T.mode == GMT_IS_GRID) && !(Ctrl->I.mode == GMT_IS_DUPLICATE || Ctrl->I.mode == GMT_IS_REFERENCE)) n_errors++;
	if (Ctrl->W.via == GMT_VIA_VECTOR && Ctrl->T.mode != GMT_IS_DATASET && !(Ctrl->W.mode == GMT_IS_DUPLICATE || Ctrl->W.mode == GMT_IS_REFERENCE)) n_errors++;
	if (Ctrl->W.via == GMT_VIA_MATRIX && !(Ctrl->T.mode == GMT_IS_DATASET || Ctrl->T.mode == GMT_IS_GRID) && !(Ctrl->W.mode == GMT_IS_DUPLICATE || Ctrl->W.mode == GMT_IS_REFERENCE)) n_errors++;
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_testapi_Ctrl (GMT, Ctrl); bailout (code);}
//#define Return(code) {Free_testapi_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_testapi (void *V_API, int mode, void *args)
{
	int error = 0, in_ID, out_ID, via[2] = {0, 0};
	int geometry[7] = {GMT_IS_POINT, GMT_IS_NONE, GMT_IS_SURFACE, GMT_IS_NONE, GMT_IS_SURFACE, GMT_IS_POINT, GMT_IS_SURFACE};
	uint64_t k;
	
	float *fdata = NULL;
	double *ddata = NULL;
	
	char *ikind[7] = {"DATASET", "TEXTSET", "GRID", "CPT", "IMAGE", "VECTOR", "MATRIX"};
	char *method[6] = {"FILE", "STREAM", "FDESC", "COPY", "REF", "READONLY"};
	char *append[3] = {"", " via VECTOR", " via MATRIX"};
	char *ifile[7] = {"dtesti.txt", "ttesti.txt", "gtesti.nc", "ctesti.cpt", "itesti.jpg", "vtesti.bin", "mtesti.bin"};
	char *ofile[7] = {"dtesto.txt", "ttesto.txt", "gtesto.nc", "ctesto.cpt", "itesto.jpg", "vtesto.bin", "mtesto.bin"};
	char string[GMT_STR16];

	FILE *fp = NULL;
	int *fdp = NULL, fd = 0;
	
	struct TESTAPI_CTRL *Ctrl = NULL;
	struct GMT_MATRIX *M = NULL;
	struct GMT_VECTOR *V = NULL;
	void *In = NULL, *Out = NULL, *Intmp = NULL, *Outtmp = NULL;
	struct GMT_CTRL *GMT = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_testapi_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_testapi_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	//GMT = GMT_begin_module (API, NULL, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	GMT = API->GMT;
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_testapi_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_testapi_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the testapi main code ----------------------------*/

	via[GMT_IN] = Ctrl->I.via / 100;	via[GMT_OUT] = Ctrl->W.via / 100;
	if (Ctrl->I.via == GMT_VIA_MATRIX) {	/* We will use a matrix in memory as data source */
		if (Ctrl->T.mode == GMT_IS_DATASET) {	/* Mimic the dtest.txt table */
			uint64_t dim[3] = {1, 9, 2};
			if ((M = GMT_Create_Data (API, GMT_IS_MATRIX, GMT_IS_SURFACE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
			M->dim = 9;	M->type = GMT_FLOAT;	M->size = M->n_rows * M->n_columns * M->n_layers;
			fdata = GMT_memory (GMT, NULL, M->size, float);
			for (k = 0; k < (uint64_t)M->n_rows; k++) {
				fdata[2*k] = (float)k;	fdata[2*k+1] = (float)k*10;
			}
			fdata[0] = fdata[1] = fdata[8] = fdata[9] = GMT->session.f_NaN;
		}
		else {	/* Mimic the gtest.nc grid as table */
			uint64_t dim[3] = {1, 6, 6};
			if ((M = GMT_Create_Data (API, GMT_IS_MATRIX, GMT_IS_SURFACE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
			M->dim = 6;	M->type = GMT_FLOAT;	M->size = M->n_rows * M->n_columns * M->n_layers;
			M->range[XLO] = 0.0;	M->range[XHI] = 5.0;	M->range[YLO] = 0.0;	M->range[YHI] = 5.0;	M->range[4] = 0.0;	M->range[5] = 25.0;
			fdata = GMT_memory (GMT, NULL, M->size, float);
			for (k = 0; k < M->size; k++) fdata[k] = (float)((int)(k%M->n_columns + (M->n_columns - 1 - k/M->n_columns) * M->n_rows));
		}
		M->data.f4 = fdata;
	}
	else if (Ctrl->I.via == GMT_VIA_VECTOR) {	/* We will use vectors in memory as data source */
		uint64_t dim[2] = {2, 9};
		if ((V = GMT_Create_Data (API, GMT_IS_VECTOR, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
		fdata = GMT_memory (GMT, NULL, V->n_rows, float);
		ddata = GMT_memory (GMT, NULL, V->n_rows, double);
		for (k = 0; k < V->n_rows; k++) {
			fdata[k] = (float)k;	ddata[k] = k*10.0;
		}
		fdata[0] = fdata[4] = GMT->session.f_NaN;
		ddata[0] = ddata[4] = GMT->session.d_NaN;
		V->data[GMT_X].f4 =  fdata;	V->type[GMT_X] = GMT_FLOAT;
		V->data[GMT_Y].f8 =  ddata;	V->type[GMT_Y] = GMT_DOUBLE;
	}
	
	/* Get input and register it */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Read %s %s with method %s%s and write to %s with method %s%s\n", ikind[Ctrl->T.mode], ifile[Ctrl->T.mode], method[Ctrl->I.mode], append[via[GMT_IN]], ofile[Ctrl->T.mode], method[Ctrl->W.mode], append[via[GMT_OUT]]);
	
	if (GMT_Init_IO (API, Ctrl->T.mode, geometry[Ctrl->T.mode], GMT_IN, GMT_ADD_FILES_IF_NONE, 0, options) != GMT_OK) {	/* Registers default input destination, unless already set */
		Return (API->error);
	}

	if (Ctrl->T.mode == GMT_IS_IMAGE) GMT_set_pad (GMT, 0U);	/* Temporary turn off padding (and thus BC setting) since we will use image exactly as is */
	switch (Ctrl->I.mode) {
		case GMT_IS_FILE:	/* Pass filename */
			if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, NULL, ifile[Ctrl->T.mode])) == GMT_NOTSET) {
				Return (API->error);
			}
			break;
		case GMT_IS_STREAM:
			switch (Ctrl->T.mode) {	/* Can only do d, t, c */
				case GMT_IS_DATASET: case GMT_IS_TEXTSET: case GMT_IS_CPT:
					fp = GMT_fopen (GMT, ifile[Ctrl->T.mode], "r");
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, NULL, fp)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_Report (API, GMT_MSG_NORMAL, "GMT_IS_STREAM only allows d, t, c!\n");
					Return (GMT_NOT_A_VALID_FAMILY);
					break;
			}
			break;
		case GMT_IS_FDESC:
			switch (Ctrl->T.mode) {	/* Can only do d, t, c */
				case GMT_IS_DATASET: case GMT_IS_TEXTSET: case GMT_IS_CPT:
					fd = open (ifile[Ctrl->T.mode], O_RDONLY);
					fdp = &fd;
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, NULL, fdp)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_Report (API, GMT_MSG_NORMAL, "GMT_IS_FDESC only allows d, t, c!\n");
					Return (GMT_NOT_A_VALID_FAMILY);
					break;
			}
			break;
		case GMT_IS_DUPLICATE: case GMT_IS_REFERENCE:
			switch (Ctrl->I.via) {
				case GMT_VIA_MATRIX:	/* Get the dataset|grid via a user matrix */
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, GMT_IS_DUPLICATE + Ctrl->I.via, geometry[Ctrl->T.mode], GMT_IN, NULL, M)) == GMT_NOTSET) {
						Return (API->error);
					}
					if ((Intmp = GMT_Get_Data (API, in_ID, 0, NULL)) == NULL) {
						Return (API->error);
					}
					break;
				case GMT_VIA_VECTOR:	/* Get the dataset|grid via a user vectors */
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, GMT_IS_DUPLICATE + Ctrl->I.via, geometry[Ctrl->T.mode], GMT_IN, NULL, V)) == GMT_NOTSET) {
						Return (API->error);
					}
					if ((Intmp = GMT_Get_Data (API, in_ID, 0, NULL)) == NULL) {
						Return (API->error);
					}
					break;
				default:		/* Get directly from file */
					if ((Intmp = GMT_Read_Data (API, Ctrl->T.mode, GMT_IS_FILE, geometry[Ctrl->T.mode], GMT_READ_NORMAL, NULL, ifile[Ctrl->T.mode], NULL)) == NULL) {
						Return (API->error);
					}
					break;
			}
			if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, NULL, Intmp)) == GMT_NOTSET) {
				Return (API->error);
			}
			break;
		default:
			GMT_Report (API, GMT_MSG_NORMAL, "Bad Input mode\n");
			Return (GMT_NOT_A_VALID_METHOD);
			break;
	}
		
	/* Now get the data from the registered source */
	
	if ((In = GMT_Get_Data (API, in_ID, 0, NULL)) == NULL) {
		Return (API->error);
	}
	if (Ctrl->T.mode == GMT_IS_IMAGE) GMT_set_pad (GMT, API->pad);	/* Reset to GMT default */
	
	if (Ctrl->T.mode == GMT_IS_IMAGE) {	/* Since writing is not supported we just make a plot via GMT_psimage */
		char buffer[GMT_BUFSIZ];
		if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, GMT_IS_REFERENCE, geometry[Ctrl->T.mode], GMT_IN, NULL, In)) == GMT_NOTSET) {
			Return (API->error);
		}
		if (GMT_Encode_ID (API, string, in_ID) != GMT_OK) {
			Return (API->error);	/* Make filename with embedded object ID */
		}
		sprintf (buffer, "%s -W6i -P -F0.25p --PS_MEDIA=letter --PS_CHAR_ENCODING=Standard+", string);
		if (GMT_Call_Module (API, "psimage", GMT_MODULE_CMD, buffer) != GMT_OK) {	/* Plot the image */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &Intmp) != GMT_OK) {
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");
		Return (GMT_OK);
	}
	
	/* Get output and register it */
	
	switch (Ctrl->W.mode) {
		case GMT_IS_FILE:	/* Pass filename */
			if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, NULL, ofile[Ctrl->T.mode])) == GMT_NOTSET) {
				Return (API->error);
			}
			break;
		case GMT_IS_STREAM:
			switch (Ctrl->T.mode) {	/* Can only do d, t, c */
				case GMT_IS_DATASET: case GMT_IS_TEXTSET: case GMT_IS_CPT:
					fp = GMT_fopen (GMT, ofile[Ctrl->T.mode], "w");
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, NULL, fp)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_Report (API, GMT_MSG_NORMAL, "GMT_IS_STREAM only allows d, t, c!\n");
					Return (GMT_NOT_A_VALID_FAMILY);
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
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, NULL, fdp)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_Report (API, GMT_MSG_NORMAL, "GMT_IS_FDESC only allows d, t, c!\n");
					Return (GMT_NOT_A_VALID_FAMILY);
					break;
			}
			break;
		case GMT_IS_DUPLICATE: case GMT_IS_REFERENCE:
			switch (Ctrl->W.via) {
				case GMT_VIA_MATRIX:	/* Put the dataset|grid via a user matrix */
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode + Ctrl->W.via, geometry[Ctrl->T.mode], GMT_OUT, NULL, NULL)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				case GMT_VIA_VECTOR:	/* Put the dataset|grid via a user vector */
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode + Ctrl->W.via, geometry[Ctrl->T.mode], GMT_OUT, NULL, NULL)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, NULL, NULL)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
			}
			break;
		default:
			GMT_Report (API, GMT_MSG_NORMAL, "Bad Input mode\n");
			Return (GMT_NOT_A_VALID_METHOD);
			break;
	}
			
	/* Now put the data to the registered destination */
	
	if (GMT_Init_IO (API, Ctrl->T.mode, geometry[Ctrl->T.mode], GMT_OUT, GMT_ADD_EXISTING, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Put_Data (API, out_ID, 0, In) != GMT_OK) {
		Return (API->error);
	}
	
	if (Ctrl->W.mode == GMT_IS_DUPLICATE || Ctrl->W.mode == GMT_IS_REFERENCE) {	/* Must write out what is in memory to the file */
		if ((Out = GMT_Retrieve_Data (API, out_ID)) == NULL) {
			Return (API->error);
		}
		if (Ctrl->W.via) {	/* Must first read into proper GMT container, then write to file */
			if ((Outtmp = GMT_Read_Data (API, Ctrl->T.mode, GMT_IS_DUPLICATE + Ctrl->W.via, geometry[Ctrl->T.mode], GMT_READ_NORMAL, NULL, Out, NULL)) == NULL) {
				Return (API->error);
			}
			if (GMT_Write_Data (API, Ctrl->T.mode, GMT_IS_FILE, geometry[Ctrl->T.mode], GMT_WRITE_SET, NULL, ofile[Ctrl->T.mode], Outtmp) != GMT_OK) {
				Return (API->error);
			}
		}
		else {
			if (GMT_Write_Data (API, Ctrl->T.mode, GMT_IS_FILE, geometry[Ctrl->T.mode], GMT_WRITE_SET, NULL, ofile[Ctrl->T.mode], Out) != GMT_OK) {
				Return (API->error);
			}
		}
	}
	
	if (GMT_Destroy_Data (API, &Intmp) != GMT_OK) {
		Return (API->error);
	}
	if (GMT_Destroy_Data (API, &M) != GMT_OK) {
		Return (API->error);
	}
	if (GMT_Destroy_Data (API, &V) != GMT_OK) {
		Return (API->error);
	}
	if (!(Ctrl->I.mode == GMT_IS_REFERENCE && Ctrl->W.mode == GMT_IS_REFERENCE) && GMT_Destroy_Data (API, &Out) != GMT_OK) {
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");
	Return (GMT_OK);
}

int main (int argc, char *argv[]) {

	int status = 0;			/* Status code from GMT API */
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session (argv[0], 2U, 0U, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 2. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_testapi (API, argc-1, (argv+1));

	/* 3. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);

	exit (status);
}
