/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: testapi allows us to test the API i/o functions.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_OPTIONS "->Vh"

/* Control structure for testapi */

struct TESTAPI_CTRL {
	struct T {	/* -T sets data type */
		bool active;
		int mode;
	} T;
	struct I {	/* -I sets input method */
		bool active;
		unsigned int mode;
	} I;
	struct W {	/* -W sets output method */
		bool active;
		unsigned int mode;
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TESTAPI_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct TESTAPI_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct TESTAPI_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	GMT_Message (API, GMT_TIME_NONE, "testapi - test API i/o methods for any data type\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: testapi -If|s|d|c|r -Td|t|g|c|i|p|m|v -Wf|s|d|c|r|m|v [%s] [%s]\n", GMT_V_OPT, GMT_h_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-I Specify input resource.  Choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f : File\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s : Stream\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d : File descriptor\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c : Memory Copy\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r : Memory Reference\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify data type.  Choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d : Dataset\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g : Grid\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c : CPT\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i : Image\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p : PostScript\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t : Textset\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   v : Vector\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m : Matrix\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Specify write destination.  Choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f : File\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s : Stream\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d : File descriptor\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c : Memory Copy\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r : Memory Reference\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V,h,.");
	
	return (EXIT_FAILURE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct TESTAPI_CTRL *Ctrl, struct GMT_OPTION *options) {

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
				break;
			case 'T':	/* Type */
				Ctrl->T.active = true;
				switch (opt->arg[0]) {
					case 'd': Ctrl->T.mode = GMT_IS_DATASET; break;
					case 'g': Ctrl->T.mode = GMT_IS_GRID; break;
					case 'c': Ctrl->T.mode = GMT_IS_PALETTE; break;
					case 'i': Ctrl->T.mode = GMT_IS_IMAGE; break;
					case 'p': Ctrl->T.mode = GMT_IS_POSTSCRIPT; break;
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
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); bailout (code);}

int GMT_testapi (void *V_API, int mode, void *args) {
	int error = 0, in_ID, out_ID;
	int geometry[] = {GMT_IS_POINT, GMT_IS_SURFACE, GMT_IS_SURFACE, GMT_IS_NONE, GMT_IS_NONE, GMT_IS_NONE, GMT_IS_SURFACE, GMT_IS_POINT, GMT_IS_NONE};
	
	char *ikind[] = {"DATASET", "GRID", "IMAGE", "PALETTE", "POSTSCRIPT", "TEXTSET", "MATRIX", "VECTOR", "COORD"};
	char *method[] = {"FILE", "STREAM", "FDESC", "COPY", "REF"};
	char *ifile[] = {"dtesti.txt", "gtesti.nc", "itesti.jpg", "ctesti.cpt", "ptesti.ps", "ttesti.txt", "mtesti.bin", "vtesti.bin", "-"};
	char *ofile[] = {"dtesto.txt", "gtesto.nc", "itesto.jpg", "ctesto.cpt", "ptesto.ps", "ttesto.txt", "mtesto.bin", "vtesto.bin", "-"};
	char string[GMT_STR16];

	FILE *fp = NULL;
	int *fdp = NULL, fd = 0;
	
	struct TESTAPI_CTRL *Ctrl = NULL;
	struct GMT_MATRIX *M = NULL;
	struct GMT_VECTOR *V = NULL;
	void *In = NULL, *Out = NULL, *Intmp = NULL;
	struct GMT_CTRL *GMT = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

#if 0
	GMT = gmt_init_module (API, NULL, THIS_MODULE_CLASSIC_NAME, &GMT_cpy); /* Save current state */
#endif
	GMT = API->GMT;
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the testapi main code ----------------------------*/
	
	/* Get input and register it */
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Read %s %s with method %s and write to %s with method %s\n", ikind[Ctrl->T.mode], ifile[Ctrl->T.mode], method[Ctrl->I.mode], ofile[Ctrl->T.mode], method[Ctrl->W.mode]);
	
	if (GMT_Init_IO (API, Ctrl->T.mode, geometry[Ctrl->T.mode], GMT_IN, GMT_ADD_FILES_IF_NONE, 0, options) != GMT_NOERROR) {	/* Registers default input destination, unless already set */
		Return (API->error);
	}

	if (Ctrl->T.mode == GMT_IS_IMAGE) gmt_set_pad (GMT, 0U);	/* Temporary turn off padding (and thus BC setting) since we will use image exactly as is */
	switch (Ctrl->I.mode) {
		case GMT_IS_FILE:	/* Pass filename */
			if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, NULL, ifile[Ctrl->T.mode])) == GMT_NOTSET) {
				Return (API->error);
			}
			break;
		case GMT_IS_STREAM:
			switch (Ctrl->T.mode) {	/* Can only do d, c, p, m, v */
				case GMT_IS_DATASET: case GMT_IS_PALETTE: case GMT_IS_POSTSCRIPT: case GMT_IS_MATRIX: case GMT_IS_VECTOR:
					fp = gmt_fopen (GMT, ifile[Ctrl->T.mode], "r");
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, NULL, fp)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_Report (API, GMT_MSG_NORMAL, "GMT_IS_STREAM only allows d, t, c, p, m, v!\n");
					Return (GMT_NOT_A_VALID_FAMILY);
					break;
			}
			break;
		case GMT_IS_FDESC:
			switch (Ctrl->T.mode) {	/* Can only do d, c, p, m, v */
				case GMT_IS_DATASET: case GMT_IS_PALETTE: case GMT_IS_POSTSCRIPT: case GMT_IS_MATRIX: case GMT_IS_VECTOR:
					fd = open (ifile[Ctrl->T.mode], O_RDONLY);
					fdp = &fd;
					if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->I.mode, geometry[Ctrl->T.mode], GMT_IN, NULL, fdp)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_Report (API, GMT_MSG_NORMAL, "GMT_IS_FDESC only allows d, t, c, p, m, v!\n");
					Return (GMT_NOT_A_VALID_FAMILY);
					break;
			}
			break;
		case GMT_IS_DUPLICATE: case GMT_IS_REFERENCE:
			if ((Intmp = GMT_Read_Data (API, Ctrl->T.mode, GMT_IS_FILE, geometry[Ctrl->T.mode], GMT_READ_NORMAL, NULL, ifile[Ctrl->T.mode], NULL)) == NULL) {
				Return (API->error);
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
	
	if (Ctrl->T.mode == GMT_IS_IMAGE) {	/* Since writing is not supported we just make a plot via GMT_psimage */
		char buffer[GMT_BUFSIZ];

		gmt_set_pad(GMT, API->pad);	/* Reset to GMT default */
		if ((in_ID = GMT_Register_IO (API, Ctrl->T.mode, GMT_IS_REFERENCE, geometry[Ctrl->T.mode], GMT_IN, NULL, In)) == GMT_NOTSET) {
			Return (API->error);
		}
		if (GMT_Encode_ID (API, string, in_ID) != GMT_NOERROR) {
			Return (API->error);	/* Make filename with embedded object ID */
		}
		sprintf (buffer, "%s -Dx0/0+w6i -P -F+p0.25p+c0 --PS_MEDIA=letter --PS_CHAR_ENCODING=Standard+", string);
		if (GMT_Call_Module (API, "psimage", GMT_MODULE_CMD, buffer) != GMT_NOERROR) {	/* Plot the image */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &Intmp) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}
	
	/* Get output and register it */
	
	switch (Ctrl->W.mode) {
		case GMT_IS_FILE:	/* Pass filename */
			if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, NULL, ofile[Ctrl->T.mode])) == GMT_NOTSET) {
				Return (API->error);
			}
			break;
		case GMT_IS_STREAM:
			switch (Ctrl->T.mode) {	/* Can only do d, c, p, m, v */
				case GMT_IS_DATASET: case GMT_IS_PALETTE: case GMT_IS_POSTSCRIPT: case GMT_IS_MATRIX: case GMT_IS_VECTOR:
					fp = gmt_fopen (GMT, ofile[Ctrl->T.mode], "w");
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, NULL, fp)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_Report (API, GMT_MSG_NORMAL, "GMT_IS_STREAM only allows d, t, c, p, m, v!\n");
					Return (GMT_NOT_A_VALID_FAMILY);
					break;
			}
			break;
		case GMT_IS_FDESC:
			switch (Ctrl->T.mode) {	/* Can only do d, c, p, m, v */
				case GMT_IS_DATASET: case GMT_IS_PALETTE: case GMT_IS_POSTSCRIPT: case GMT_IS_MATRIX: case GMT_IS_VECTOR:
#ifdef WIN32
					/* I think they exist on Win too, but no much time to find out how. JL */
					fd = open (ofile[Ctrl->T.mode], O_WRONLY | O_CREAT);
#else
					fd = open (ofile[Ctrl->T.mode], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
#endif
					fdp = &fd;
					if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, NULL, fdp)) == GMT_NOTSET) {
						Return (API->error);
					}
					break;
				default:
					GMT_Report (API, GMT_MSG_NORMAL, "GMT_IS_FDESC only allows d, t, c, p, m, v!\n");
					Return (GMT_NOT_A_VALID_FAMILY);
					break;
			}
			break;
		case GMT_IS_DUPLICATE: case GMT_IS_REFERENCE:
			if ((out_ID = GMT_Register_IO (API, Ctrl->T.mode, Ctrl->W.mode, geometry[Ctrl->T.mode], GMT_OUT, NULL, NULL)) == GMT_NOTSET) {
				Return (API->error);
			}
			break;
		default:
			GMT_Report (API, GMT_MSG_NORMAL, "Bad Input mode\n");
			Return (GMT_NOT_A_VALID_METHOD);
			break;
	}
			
	/* Now put the data to the registered destination */
	
	if (GMT_Init_IO (API, Ctrl->T.mode, geometry[Ctrl->T.mode], GMT_OUT, GMT_ADD_EXISTING, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Put_Data (API, out_ID, 0, In) != GMT_NOERROR) {
		Return (API->error);
	}
	
	if (Ctrl->W.mode == GMT_IS_DUPLICATE || Ctrl->W.mode == GMT_IS_REFERENCE) {	/* Must write out what is in memory to the file */
		if ((Out = GMT_Retrieve_Data (API, out_ID)) == NULL) {
			Return (API->error);
		}
		if (GMT_Write_Data (API, Ctrl->T.mode, GMT_IS_FILE, geometry[Ctrl->T.mode], GMT_WRITE_SET, NULL, ofile[Ctrl->T.mode], Out) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	
	if (GMT_Destroy_Data (API, &Intmp) != GMT_NOERROR) {
		Return (API->error);
	}
	if (GMT_Destroy_Data (API, &M) != GMT_NOERROR) {
		Return (API->error);
	}
	if (GMT_Destroy_Data (API, &V) != GMT_NOERROR) {
		Return (API->error);
	}
	if (!(Ctrl->I.mode == GMT_IS_REFERENCE && Ctrl->W.mode == GMT_IS_REFERENCE) && GMT_Destroy_Data (API, &Out) != GMT_NOERROR) {
		Return (API->error);
	}
	Return (GMT_NOERROR);
}

int main (int argc, char *argv[]) {

	int status = 0;				/* Status code from GMT API */
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session (argv[0], GMT_PAD_DEFAULT, GMT_SESSION_NORMAL, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 2. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_testapi (API, argc-1, (argv+1));

	/* 3. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);

	exit (status);
}
