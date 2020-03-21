#include "gmt_dev.h"
#include <math.h>
/*
 * Testing the use of user data provided via a GMT_MATRIX
 * to/from a module that expect to read/write GMT_DATASETs.
 * We do this for all possible type combinations as well
 * as pre-allocated and GMT-allocated output spaces.
 */

/* Dimensions of our test dataset */
#define NROWS	3
#define NCOLS	4
#define NM	(NROWS * NCOLS)

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
			d = vector = calloc (NM, sizeof (double));
			if (set) for (k = 0; k < NM; k++) d[k] = k;
			break;
		case GMT_FLOAT:
			f = vector = calloc (NM, sizeof (float));
			if (set) for (k = 0; k < NM; k++) f[k] = (float)k;
			break;
		case GMT_ULONG:
			ul = vector = calloc (NM, sizeof (uint64_t));
			if (set) for (k = 0; k < NM; k++) ul[k] = k;
			break;
		case GMT_LONG:
			sl = vector = calloc (NM, sizeof (int64_t));
			if (set) for (k = 0; k < NM; k++) sl[k] = k;
			break;
		case GMT_UINT:
			ui = vector = calloc (NM, sizeof (unsigned int));
			if (set) for (k = 0; k < NM; k++) ui[k] = k;
			break;
		case GMT_INT:
			si = vector = calloc (NM, sizeof (int));
			if (set) for (k = 0; k < NM; k++) si[k] = k;
			break;
		case GMT_USHORT:
			uh = vector = calloc (NM, sizeof (unsigned short int));
			if (set) for (k = 0; k < NM; k++) uh[k] = k;
			break;
		case GMT_SHORT:
			sh = vector = calloc (NM, sizeof (short int));
			if (set) for (k = 0; k < NM; k++) sh[k] = k;
			break;
		case GMT_UCHAR:
			uc = vector = calloc (NM, sizeof (unsigned char));
			if (set) for (k = 0; k < NM; k++) uc[k] = k;
			break;
		case GMT_CHAR:
			sc = vector = calloc (NM, sizeof (char));
			if (set) for (k = 0; k < NM; k++) sc[k] = k;
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
			d = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", d[k]);
			break;
		case GMT_FLOAT:
		 	f = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", (double)f[k]);
			break;
		case GMT_ULONG:
 			ul = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", (double)ul[k]);
			break;
		case GMT_LONG:
 			sl = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", (double)sl[k]);
			break;
		case GMT_UINT:
 			ui = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", (double)ui[k]);
			break;
		case GMT_INT:
			si = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", (double)si[k]);
			break;
		case GMT_USHORT:
			uh = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", (double)uh[k]);
			break;
		case GMT_SHORT:
			sh = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", (double)sh[k]);
			break;
		case GMT_UCHAR:
			uc = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", (double)uc[k]);
			break;
		case GMT_CHAR:
			sc = vector;	for (k = 0; k < NM; k++) printf (" %5.1f", (double)sc[k]);
			break;
	}
	printf ("\n");
}

double sum_array (void *vector, unsigned int type) {
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
	switch (type) {
		case GMT_DOUBLE:
			d = vector;	for (k = 0; k < NM; k++) sum += d[k];
			break;
		case GMT_FLOAT:
		 	f = vector;	for (k = 0; k < NM; k++) sum += f[k];
			break;
		case GMT_ULONG:
 			ul = vector;	for (k = 0; k < NM; k++) sum += ul[k];
			break;
		case GMT_LONG:
 			sl = vector;	for (k = 0; k < NM; k++) sum += sl[k];
			break;
		case GMT_UINT:
 			ui = vector;	for (k = 0; k < NM; k++) sum += ui[k];
			break;
		case GMT_INT:
			si = vector;	for (k = 0; k < NM; k++) sum += si[k];
			break;
		case GMT_USHORT:
			uh = vector;	for (k = 0; k < NM; k++) sum += uh[k];
			break;
		case GMT_SHORT:
			sh = vector;	for (k = 0; k < NM; k++) sum += sh[k];
			break;
		case GMT_UCHAR:
			uc = vector;	for (k = 0; k < NM; k++) sum += uc[k];
			break;
		case GMT_CHAR:
			sc = vector;	for (k = 0; k < NM; k++) sum += sc[k];
			break;
	}
	return (sum);
}

int deploy_test (unsigned int intype, unsigned int outtype, int alloc_in_GMT, int V) {
	/* Run the test using the specified in and out types */
	uint64_t dim[4] = {NCOLS, NROWS, 1, 0};		/* ncols, nrows, nlayers, type */
	int bad = 0;
	unsigned int out_via = (outtype + 1) * 100 + GMT_IS_POINT;	/* To get GMT_VIA_<type */
	unsigned int mode = GMT_SESSION_EXTERNAL;
	double diff;
	//void *API = NULL;                           /* The API control structure */
	struct GMT_MATRIX *M[2] = {NULL, NULL};     /* Structure to hold input/output datasets as matrix */
	char input[GMT_VF_LEN] = {""};               /* String to hold virtual input filename */
	char output[GMT_VF_LEN] = {""};              /* String to hold virtual output filename */
	char args[128] = {""};            			/* String to hold module command arguments */
	void *in_data = NULL, *out_data = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V) mode += (6 << 16);				/* Activate -Vd */
	in_data = get_array (intype, 1);			/* Create dummy user dataset in_data[] = k */

	/* Initialize a GMT session */
	API = GMT_Create_Session ("test", 2U, mode, NULL);
 	/* Create a blank matrix container that will hold our user in_data */
	if ((M[GMT_IN] = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_MATRIX, GMT_IS_POINT, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (EXIT_FAILURE);
	/* Hook the user input array up to this container */
	GMT_Put_Matrix (API, M[GMT_IN], intype, 0, in_data);
	/* Associate our matrix container with a virtual dataset file to "read" from */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_MATRIX, GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, M[GMT_IN], input);
	if (alloc_in_GMT)	/* Request matrix container for output data to be allocated by GMT */
	    GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_MATRIX, out_via, GMT_OUT, NULL, output);
	else {	/* Preallocate array space here in the app */
		out_data = get_array (outtype, 0);	/* Make user space for output */
 		/* Create a blank matrix container that will hold our user out_data, but pass dimensions so we know */
		M[GMT_OUT] = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_MATRIX, GMT_IS_POINT, GMT_IS_OUTPUT, dim, NULL, NULL, 0, 0, NULL);
		/* Hook the user output array up to this containers */
		GMT_Put_Matrix (API, M[GMT_OUT], outtype, 0, out_data);
		/* Associate our data matrix with a virtual dataset file to "write" to */
		GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_MATRIX, GMT_IS_POINT, GMT_OUT, M[GMT_OUT], output);
	}
	/* Prepare the module arguments to multiply the input dataset by 10 then add 1 */
	sprintf (args, "%s 10 MUL 1 ADD = %s", input, output);
	/* Call the gmtmath module */
	GMT_Call_Module (API, "gmtmath", GMT_MODULE_CMD, args);
	if (alloc_in_GMT) {	/* Obtain the matrix container from the output virtual file */
    	M[GMT_OUT] = GMT_Read_VirtualFile (API, output);
		/* Get the pointer to the modified user output array */
		out_data = GMT_Get_Matrix (API, M[GMT_OUT]);
	}
	/* Close the virtual files */
	GMT_Close_VirtualFile (API, input);
	GMT_Close_VirtualFile (API, output);
	diff = sum_array (out_data, outtype) - sum_array (in_data, intype) - 495.0;	/* Expected diff = 0 */
	if (fabs (diff) > 0.0) {
		fprintf (stderr, "\nTest matrix/dataset/matrix for Input = index [%s], output = 10*input + 1 [%s]\n", type[intype], type[outtype]);
		fprintf (stderr, "Misfit = %g\n", diff);
		/* Print out the input and output values */
		put_array (in_data, intype, "Input: ");
		put_array (out_data, outtype, "Output:");
		bad = 1;
		fprintf (stderr, "\n\n");
	}
	/* Destroy session, which will free all GMT-allocated memory */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	free (in_data);
	if (alloc_in_GMT == 0) free (out_data);
	return bad;
}

int main (int argc, char *argv[]) {
	unsigned int in, answer, out, bad = 0, n = 0, V = (argc > 1), Q = (argc > 2);
	unsigned int quiet = (argc == 2 && !strcmp (argv[1], "-q"));
	char *passfail[2] = {"PASS", "FAIL"};
	if (quiet) V = 0;
	for (in = GMT_CHAR; in <= GMT_DOUBLE; in++) {
		for (out = GMT_CHAR; out <= GMT_DOUBLE; out++) {
			if (!quiet) printf ("Test matrix/dataset/matrix(prealloc) for Input = index [%s], output = 10*input + 1 [%s] :", type[in], type[out]);
			answer = deploy_test (in, out, 1, V);
			if (!quiet || answer) printf ("%s\n", passfail[answer]);
			bad += answer;
			n ++;
			if (Q) out = in = GMT_DOUBLE;
		}
	}
	if (bad && !quiet) printf ("%d of %d combinations with preallocated output memory failed the test\n", bad, n);
	for (in = GMT_CHAR, bad = 0; in <= GMT_DOUBLE; in++) {
		for (out = GMT_CHAR; out <= GMT_DOUBLE; out++) {
			if (!quiet) printf ("Test matrix/dataset/matrix(GMT alloc) for Input = index [%s], output = 10*input + 1 [%s]", type[in], type[out]);
			answer = deploy_test (in, out, 0, V);
			if (!quiet || answer) printf (" :%s\n", passfail[answer]);
			bad += answer;
			n ++;
			if (Q) out = in = GMT_DOUBLE;
		}
	}
	if (bad && !quiet) printf ("%d of %d combinations with GMT-allocated output memory failed the test\n", bad, n);
	exit (0);
}
