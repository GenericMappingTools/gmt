#include "gmt_dev.h"
#include <math.h>
/*
 * Testing the use of user data provided via a GMT_VECTOR
 * to/from a module that expect to read/write GMT_DATASETs.
 * We do this for all possible type combinations as well
 * as pre-allocated and GMT-allocated output spaces.
 * We repeat the above loops twice: first by specifying
 * vector size via its dimensions and then next by specifying
 * the vector via range and increments.  With 10 data types
 * and 2 ways for allocations and 2 ways for dimensioning we
 * end up with 2 * 2 * 10 * 10 = 400 separate calls to gmtmath.
 * Each input vector is loaded with the index numbers 0 through (NROWS-1)
 * which here is 0 to 4.  The gmtmath command reads the vector,
 * multiplies by 10 and adds 1.  The calling program then sums
 * the output which should be 10*sum(0-4) + 1 = 105. Since the
 * sum of the input is sum(0-4) = 10 the differences is 95.
 * The test is therefore checking that difference.
 *
 * 4/7/2018: Currently failing because I am treating the columns as
 * a set of matrix columns and this is actually not supported.
 * Awaiting feedback on what is best path forward.
 */

/* Dimensions of our test dataset */
#define NROWS	5
#define NCOLS	2

static char *type[] = {"char", "uchar", "short", "ushort", "int", "uint", "long", "ulong", "float", "double"};

void *get_array (unsigned int type, unsigned int set) {
	/* Create an array of given type (return it as void *) and optionally assign data[k] = k */
	double *d = NULL;
	float *f = NULL;
	uint64_t *ul = NULL;
	int64_t *sl = NULL;
	unsigned int *ui = NULL;
	int *si = NULL;
	unsigned short int *uh = NULL;
	short int *sh = NULL;
	unsigned char *uc = NULL;
	char *sc = NULL;
	void *vector = NULL;
	unsigned int k;
	switch (type) {
		case GMT_DOUBLE:
			d = vector = calloc (NROWS, sizeof (double));
			if (set) for (k = 0; k < NROWS; k++) d[k] = k;
			break;
		case GMT_FLOAT:
			f = vector = calloc (NROWS, sizeof (float));
			if (set) for (k = 0; k < NROWS; k++) f[k] = k;
			break;
		case GMT_ULONG:
			ul = vector = calloc (NROWS, sizeof (uint64_t));
			if (set) for (k = 0; k < NROWS; k++) ul[k] = k;
			break;
		case GMT_LONG:
			sl = vector = calloc (NROWS, sizeof (int64_t));
			if (set) for (k = 0; k < NROWS; k++) sl[k] = k;
			break;
		case GMT_UINT:
			ui = vector = calloc (NROWS, sizeof (unsigned int));
			if (set) for (k = 0; k < NROWS; k++) ui[k] = k;
			break;
		case GMT_INT:
			si = vector = calloc (NROWS, sizeof (int));
			if (set) for (k = 0; k < NROWS; k++) si[k] = k;
			break;
		case GMT_USHORT:
			uh = vector = calloc (NROWS, sizeof (unsigned short int));
			if (set) for (k = 0; k < NROWS; k++) uh[k] = k;
			break;
		case GMT_SHORT:
			sh = vector = calloc (NROWS, sizeof (short int));
			if (set) for (k = 0; k < NROWS; k++) sh[k] = k;
			break;
		case GMT_UCHAR:
			uc = vector = calloc (NROWS, sizeof (unsigned char));
			if (set) for (k = 0; k < NROWS; k++) uc[k] = k;
			break;
		case GMT_CHAR:
			sc = vector = calloc (NROWS, sizeof (char));
			if (set) for (k = 0; k < NROWS; k++) sc[k] = k;
			break;
	}
	return (vector);
}

void put_array (void *vector, unsigned int type, char *txt) {
	/* Print one row with the output data */
	double *d = NULL;
	float *f = NULL;
	uint64_t *ul = NULL;
	int64_t *sl = NULL;
	unsigned int *ui = NULL;
	int *si = NULL;
	unsigned short int *uh = NULL;
	short int *sh = NULL;
	unsigned char *uc = NULL;
	char *sc = NULL;
	unsigned int k;
	printf ("%s", txt);
	switch (type) {
		case GMT_DOUBLE:
			d = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", d[k]);
			break;
		case GMT_FLOAT:
		 	f = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", (double)f[k]);
			break;
		case GMT_ULONG:
 			ul = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", (double)ul[k]);
			break;
		case GMT_LONG:
 			sl = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", (double)sl[k]);
			break;
		case GMT_UINT:
 			ui = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", (double)ui[k]);
			break;
		case GMT_INT:
			si = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", (double)si[k]);
			break;
		case GMT_USHORT:
			uh = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", (double)uh[k]);
			break;
		case GMT_SHORT:
			sh = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", (double)sh[k]);
			break;
		case GMT_UCHAR:
			uc = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", (double)uc[k]);
			break;
		case GMT_CHAR:
			sc = vector;	for (k = 0; k < NROWS; k++) printf (" %5.1f", (double)sc[k]);
			break;
	}
	printf ("\n");
}

double sum_arrays (void *vector[], unsigned int type) {
	/* Print one row with the output data */
	double *d = NULL;
	float *f = NULL;
	uint64_t *ul = NULL;
	int64_t *sl = NULL;
	unsigned int *ui = NULL;
	int *si = NULL;
	unsigned short int *uh = NULL;
	short int *sh = NULL;
	unsigned char *uc = NULL;
	char *sc = NULL;
	unsigned int k;
	double sum = 0.0;
	for (unsigned int col = 0; col < NCOLS; col++) {
		switch (type) {
			case GMT_DOUBLE:
				d = vector[col];	for (k = 0; k < NROWS; k++) sum += d[k];
				break;
			case GMT_FLOAT:
			 	f = vector[col];	for (k = 0; k < NROWS; k++) sum += f[k];
				break;
			case GMT_ULONG:
	 			ul = vector[col];	for (k = 0; k < NROWS; k++) sum += ul[k];
				break;
			case GMT_LONG:
	 			sl = vector[col];	for (k = 0; k < NROWS; k++) sum += sl[k];
				break;
			case GMT_UINT:
	 			ui = vector[col];	for (k = 0; k < NROWS; k++) sum += ui[k];
				break;
			case GMT_INT:
				si = vector[col];	for (k = 0; k < NROWS; k++) sum += si[k];
				break;
			case GMT_USHORT:
				uh = vector[col];	for (k = 0; k < NROWS; k++) sum += uh[k];
				break;
			case GMT_SHORT:
				sh = vector[col];	for (k = 0; k < NROWS; k++) sum += sh[k];
				break;
			case GMT_UCHAR:
				uc = vector[col];	for (k = 0; k < NROWS; k++) sum += uc[k];
				break;
			case GMT_CHAR:
				sc = vector[col];	for (k = 0; k < NROWS; k++) sum += sc[k];
				break;
		}
	}
	return (sum);
}

int deploy_test (unsigned int intype, unsigned int outtype, int alloc_in_GMT, int def_mode, int verbose) {
	/* Run the test using the specified in and out types */
	uint64_t dim[3] = {NCOLS, NROWS, 0};		/* ncols, nrows, type */
	int bad = 0;
	unsigned int out_via = (outtype + 1) * 100 + GMT_IS_POINT;	/* To get GMT_VIA_<type */
	unsigned int mode = GMT_SESSION_EXTERNAL;
	double diff, range[2] = {1.0, NROWS}, inc[2] = {1.0, 1.0};
	//void *API = NULL;                           /* The API control structure */
	struct GMT_VECTOR *V[2] = {NULL, NULL};     /* Structure to hold input/output datasets as vectors */
	char input[GMT_VF_LEN] = {""};               /* String to hold virtual input filename */
	char output[GMT_VF_LEN] = {""};              /* String to hold virtual output filename */
	char args[128] = {""};            			/* String to hold module command arguments */
	void *in_data[NCOLS] = {NULL, NULL}, *out_data[NCOLS] = {NULL, NULL};
	struct GMTAPI_CTRL *API = NULL;

	if (verbose) mode += (6 << 16);				/* Activate -Vd */
	in_data[GMT_X] = get_array (intype, 1);			/* Create dummy user dataset in_data[] = k */
	in_data[GMT_Y] = get_array (intype, 1);			/* Create dummy user dataset in_data[] = k */

	/* Initialize a GMT session */
	API = GMT_Create_Session ("test", 2U, mode, NULL);
 	/* Create a blank vector container that will hold our user in_data */
	if (def_mode == 0) {	/* Use dimensions to allocate */
		if ((V[GMT_IN] = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (EXIT_FAILURE);
	}
	else {	/* Use col dimension and range and inc to allocate space */
		dim[1] = 0;	/* So that we compute it in GMT_Create_Vector */
		if ((V[GMT_IN] = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_CONTAINER_ONLY, dim, range, inc, 0, 0, NULL)) == NULL) return (EXIT_FAILURE);
	}
	/* Hook the user input arrays up to this container */
	GMT_Put_Vector (API, V[GMT_IN], GMT_X, intype, in_data[GMT_X]);
	GMT_Put_Vector (API, V[GMT_IN], GMT_Y, intype, in_data[GMT_Y]);
	/* Associate our vectors container with a virtual dataset file to "read" from */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, V[GMT_IN], input);
	if (alloc_in_GMT)	/* Request vectors container for output data to be allocated by GMT */
		GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, out_via, GMT_OUT, NULL, output);
	else {	/* Preallocate array space here in the app */
		out_data[GMT_X] = get_array (outtype, 0);	/* Make user space for output */
		out_data[GMT_Y] = get_array (outtype, 0);	/* Make user space for output */
 		/* Create a blank vectors container that will hold our user out_data, and pass dim so we can set those correctly */
		if (def_mode == 0) {	/* Use dimensions to allocate */
			V[GMT_OUT] = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IS_OUTPUT, dim, NULL, NULL, 0, 0, NULL);
		}
		else {	/* Use region and inc instead */
			V[GMT_OUT] = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IS_OUTPUT, dim, range, inc, 0, 0, NULL);
		}
		/* Hook the user output array up to this containers */
		GMT_Put_Vector (API, V[GMT_OUT], GMT_X, outtype, out_data[GMT_X]);
		GMT_Put_Vector (API, V[GMT_OUT], GMT_Y, outtype, out_data[GMT_Y]);
   		/* Associate our data vectors with a virtual dataset file to "write" to */
    		GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_OUT, V[GMT_OUT], output);
	}
	/* Prepare the module arguments to multiply the input dataset by 10 then add 1 */
	sprintf (args, "%s 10 MUL 1 ADD = %s", input, output);
	/* Call the gmtmath module */
	GMT_Call_Module (API, "gmtmath", GMT_MODULE_CMD, args);
	if (alloc_in_GMT) {	/* Obtain the vectors container from the output virtual file */
    		V[GMT_OUT] = GMT_Read_VirtualFile (API, output);
		/* Get the pointer to the modified user output array */
		out_data[GMT_X] = GMT_Get_Vector (API, V[GMT_OUT], GMT_X);
		out_data[GMT_Y] = GMT_Get_Vector (API, V[GMT_OUT], GMT_Y);
	}
	/* Close the virtual files */
	GMT_Close_VirtualFile (API, input);
	GMT_Close_VirtualFile (API, output);
	diff = sum_arrays (out_data, outtype) - sum_arrays (in_data, intype) - 95.0;	/* Expected diff = 0 */
	if (fabs (diff) > 0.0) {
		fprintf (stderr, "\nTest vectors/dataset/vectors for Input = index [%s], output = 10*input + 1 [%s]\n", type[intype], type[outtype]);
		fprintf (stderr, "Misfit = %g\n", diff);
		/* Print out the input and output values */
		put_array (in_data, intype, "Input: ");
		put_array (out_data, outtype, "Output:");
		bad = 1;
		fprintf (stderr, "\n\n");
	}
	/* Destroy session, which will free all GMT-allocated memory */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	free (in_data[GMT_X]);	free (in_data[GMT_Y]);
	if (alloc_in_GMT == 0) { free (out_data[GMT_X]); free (out_data[GMT_Y]); }
	return bad;
}

int main (int argc, char *argv[]) {
	unsigned int in, mem_mode, def_mode, answer, out, bad = 0, n = 0, V = (argc > 1), Q = (argc > 2);
	char *passfail[2] = {"PASS", "FAIL"}, *kind[2] = {"prealloc", "GMTalloc"}, *def[2] = {"dim", "R/I"};
	unsigned int quiet = (argc == 2 && !strcmp (argv[1], "-q"));
	if (quiet) V = 0;
	for (def_mode = 0; def_mode < 2; def_mode++) {
		for (mem_mode = 0; mem_mode < 2; mem_mode++) {
			for (in = GMT_CHAR; in <= GMT_DOUBLE; in++) {
				for (out = GMT_CHAR; out <= GMT_DOUBLE; out++) {
					if (!quiet) printf ("[%s] Test vectors/dataset/vectors(%s) for Input = index [%6s], output = 10*input + 1 [%6s] :", def[def_mode], kind[mem_mode], type[in], type[out]);
					answer = deploy_test (in, out, mem_mode, def_mode, V);
					if (!quiet || answer) printf ("%s\n", passfail[answer]);
					bad += answer;
					n ++;
					if (Q) out = in = GMT_DOUBLE;
				}
			}
			if (bad && !quiet) printf ("%d of %d combinations with preallocated output memory failed the test\n", bad, n);
		}
	}
	exit (0);
}
