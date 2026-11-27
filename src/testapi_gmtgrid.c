#include "gmt_dev.h"
/*
 * Testing the use of user grid memory allocated externally.
 */

static char *copy_string (const char *string) {
	char *copy;
	size_t size;
	if (string == NULL) return NULL;
	size = strlen (string) + 1;
	if ((copy = malloc (size)) == NULL) return NULL;
	memcpy (copy, string, size);
	return copy;
}

int main () {
	unsigned int mode = GMT_SESSION_EXTERNAL;
	struct GMT_GRID *G = NULL;
	struct GMT_GRID *G_read = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMTAPI_CTRL *API = NULL;
	struct GMT_GRID_HEADER header_before;
	struct GMT_GRID_HEADER_HIDDEN hidden_before;
	uint64_t dim[2] = {5, 4};
	char args[1000] = {""};
	char input[GMT_VF_LEN] = {""};
	gmt_grdfloat *data = NULL;
	gmt_grdfloat *data_before = NULL;
	char *proj4_before = NULL, *wkt_before = NULL;
	char *title_before = NULL, *cpt_before = NULL, *command_before = NULL, *remark_before = NULL, *pocket_before = NULL;
	char *proj4 = NULL, *wkt = NULL;
	size_t n;
	int status = EXIT_FAILURE;

	API = GMT_Create_Session ("testapi_gmtgrid", 2U, mode, NULL);
	if (API == NULL) goto cleanup;

	/* Create a container for the grid */
	G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 2, NULL);
	if (G == NULL) goto cleanup;

	/* Allocate memory of the data array in the external program (C or PyGMT) */
	n = G->header->size;
	data = malloc (sizeof (gmt_grdfloat) * n);
	if (data == NULL) goto cleanup;
	for (size_t i = 0; i < n; i++) data[i] = (gmt_grdfloat)i;

	/* Assign the user data to the GMT_GRID structure */
	G->data = data;

	/* Using the grid */
	if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN, G, input) != GMT_NOERROR) goto cleanup;
	sprintf (args, "%s", input);
	if (GMT_Call_Module (API, "grd2xyz", GMT_MODULE_CMD, args) != GMT_NOERROR) goto cleanup;
	if (GMT_Close_VirtualFile (API, input) != GMT_NOERROR) goto cleanup;

	/* Exercise lossless source preservation when writing a scaled integer grid. */
	for (size_t i = 0; i < n; i++) data[i] = (gmt_grdfloat)(0.037 + 0.13 * i);
	G->header->ProjRefPROJ4 = proj4 = copy_string ("+proj=longlat +datum=WGS84");
	G->header->ProjRefWKT = wkt = copy_string ("");
	if (proj4 == NULL || wkt == NULL) goto cleanup;
	HH = gmt_get_H_hidden (G->header);
	HH->title = copy_string ("hidden title");
	HH->cpt = copy_string ("hidden.cpt");
	HH->command = copy_string ("test command");
	HH->remark = copy_string ("hidden remark");
	HH->pocket = copy_string ("hidden pocket");
	if (HH->title == NULL || HH->cpt == NULL || HH->command == NULL || HH->remark == NULL || HH->pocket == NULL) goto cleanup;

	data_before = malloc (sizeof (gmt_grdfloat) * n);
	if (data_before == NULL) goto cleanup;
	memcpy (data_before, data, sizeof (gmt_grdfloat) * n);
	header_before = *G->header;
	header_before.hidden = G->header->hidden;
	header_before.ProjRefPROJ4 = G->header->ProjRefPROJ4;
	header_before.ProjRefWKT = G->header->ProjRefWKT;
	hidden_before = *HH;
	proj4_before = copy_string (G->header->ProjRefPROJ4);
	wkt_before = copy_string (G->header->ProjRefWKT);
	title_before = copy_string (HH->title);
	cpt_before = copy_string (HH->cpt);
	command_before = copy_string (HH->command);
	remark_before = copy_string (HH->remark);
	pocket_before = copy_string (HH->pocket);
	if (proj4_before == NULL || wkt_before == NULL || title_before == NULL || cpt_before == NULL ||
	    command_before == NULL || remark_before == NULL || pocket_before == NULL) goto cleanup;

	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA,
	                    NULL, "testapi_gmtgrid.nc=ns+s0.1", G) != GMT_NOERROR) {
		fprintf (stderr, "GMT_Write_Data failed with error %d\n", API->error);
		goto cleanup;
	}

	if (memcmp (data, data_before, sizeof (gmt_grdfloat) * n) != 0 ||
	    memcmp (&header_before, G->header, sizeof (header_before)) != 0 ||
	    memcmp (&hidden_before, HH, sizeof (hidden_before)) != 0 ||
	    strcmp (proj4_before, G->header->ProjRefPROJ4) != 0 ||
	    strcmp (wkt_before, G->header->ProjRefWKT) != 0 ||
	    strcmp (title_before, HH->title) != 0 ||
	    strcmp (cpt_before, HH->cpt) != 0 ||
	    strcmp (command_before, HH->command) != 0 ||
	    strcmp (remark_before, HH->remark) != 0 ||
	    strcmp (pocket_before, HH->pocket) != 0) {
		fprintf (stderr, "GMT_Write_Data modified the source grid or header\n");
		goto cleanup;
	}

	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA | GMT_IO_RESET,
	                    NULL, "testapi_gmtgrid.nc=ns+s0.1", G) != GMT_NOERROR) {
		fprintf (stderr, "Repeated GMT_Write_Data failed with error %d\n", API->error);
		goto cleanup;
	}
	if (memcmp (data, data_before, sizeof (gmt_grdfloat) * n) != 0 ||
	    memcmp (&header_before, G->header, sizeof (header_before)) != 0 ||
	    memcmp (&hidden_before, HH, sizeof (hidden_before)) != 0 ||
	    strcmp (proj4_before, G->header->ProjRefPROJ4) != 0 ||
	    strcmp (wkt_before, G->header->ProjRefWKT) != 0 ||
	    strcmp (title_before, HH->title) != 0 ||
	    strcmp (cpt_before, HH->cpt) != 0 ||
	    strcmp (command_before, HH->command) != 0 ||
	    strcmp (remark_before, HH->remark) != 0 ||
	    strcmp (pocket_before, HH->pocket) != 0) {
		fprintf (stderr, "Repeated GMT_Write_Data modified the source grid or header\n");
		goto cleanup;
	}

	G_read = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL,
	                        NULL, "testapi_gmtgrid.nc", NULL);
	if (G_read == NULL) {
		fprintf (stderr, "Failed to read the grid written by GMT_Write_Data\n");
		goto cleanup;
	}
	if (G_read->header->n_rows != G->header->n_rows || G_read->header->n_columns != G->header->n_columns) {
		fprintf (stderr, "Written grid dimensions do not match the source grid\n");
		goto cleanup;
	}
	for (uint32_t row = 0; row < G_read->header->n_rows; row++) {
		for (uint32_t col = 0; col < G_read->header->n_columns; col++) {
			uint64_t source_ij = gmt_M_ijp (G->header, row, col);
			uint64_t output_ij = gmt_M_ijp (G_read->header, row, col);
			gmt_grdfloat expected = rintf (data_before[source_ij] / 0.1f) * 0.1f;
			if (fabs ((double)G_read->data[output_ij] - expected) > 1e-5) {
				fprintf (stderr, "Written grid value mismatch at row %u, column %u: expected %g, got %g\n",
				         row, col, (double)expected, (double)G_read->data[output_ij]);
				goto cleanup;
			}
		}
	}
	{
		struct GMT_GRID *read_grid = G_read;
		G_read = NULL;
		if (GMT_Destroy_Data (API, &read_grid) != GMT_NOERROR) goto cleanup;
	}

	/* An out-of-range packed value forces the NetCDF backend to fail after copy creation. */
	{
		uint64_t failure_ij = gmt_M_ijp (G->header, 1, 1);
		int write_error;
		data[failure_ij] = 1.0e9f;
		write_error = GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE,
		                              GMT_CONTAINER_AND_DATA | GMT_IO_RESET, NULL, "testapi_gmtgrid.nc=ns+s0.1", G);
		if (write_error != GMT_GRID_WRITE_ERROR) {
			fprintf (stderr, "Expected out-of-range NetCDF write to fail in the grid writer, got %d\n", write_error);
			goto cleanup;
		}
		for (size_t i = 0; i < n; i++) {
			gmt_grdfloat expected = (i == failure_ij) ? 1.0e9f : data_before[i];
			if (data[i] != expected) {
				fprintf (stderr, "Failed write modified source grid data at index %zu\n", i);
				goto cleanup;
			}
		}
		if (memcmp (&header_before, G->header, sizeof (header_before)) != 0 ||
		    memcmp (&hidden_before, HH, sizeof (hidden_before)) != 0 ||
		    strcmp (proj4_before, G->header->ProjRefPROJ4) != 0 ||
		    strcmp (wkt_before, G->header->ProjRefWKT) != 0 ||
		    strcmp (title_before, HH->title) != 0 ||
		    strcmp (cpt_before, HH->cpt) != 0 ||
		    strcmp (command_before, HH->command) != 0 ||
		    strcmp (remark_before, HH->remark) != 0 ||
		    strcmp (pocket_before, HH->pocket) != 0) {
			fprintf (stderr, "Failed write modified source grid header\n");
			goto cleanup;
		}
		data[failure_ij] = data_before[failure_ij];
	}
	status = EXIT_SUCCESS;

cleanup:
	if (G_read) {
		struct GMT_GRID *read_grid = G_read;
		G_read = NULL;
		if (GMT_Destroy_Data (API, &read_grid) != GMT_NOERROR) status = EXIT_FAILURE;
	}
	free (data_before);
	free (proj4_before);
	free (wkt_before);
	free (title_before);
	free (cpt_before);
	free (command_before);
	free (remark_before);
	free (pocket_before);
	if (HH) {
		free (HH->title);
		free (HH->cpt);
		free (HH->command);
		free (HH->remark);
		free (HH->pocket);
		HH->title = HH->cpt = HH->command = HH->remark = HH->pocket = NULL;
	}
	if (G && G->header) {
		G->header->ProjRefPROJ4 = NULL;
		G->header->ProjRefWKT = NULL;
	}
	if (API && GMT_Destroy_Session (API) != GMT_NOERROR) status = EXIT_FAILURE;
	free (proj4);
	free (wkt);
	free (data);
	return status;
}
