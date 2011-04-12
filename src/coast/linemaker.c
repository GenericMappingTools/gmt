/*
 *	$Id: linemaker.c,v 1.15 2011-04-12 13:06:42 remko Exp $
 */
/*
 *
 * Author: Paul Wessel
 * Date:	18-FEB-98
 * Version:	3.0	Reads/writes netCDF 3.0 files
 *		3.1	Use GMT3.3 + POSIX
 *		3.2	use GMT 4
 */
 
#include "gmt.h"

typedef unsigned short ushort;

struct GMT_BR s;
struct SHORT_PAIR {
	ushort	dx;	/* Relative distance from SW corner of bin, units of B_WIDTH/GSHHS_MAX_DELTA  */
	ushort	dy;
} p;
struct SEGMENT_HEADER {
	ushort n;		/* n points */
	ushort level;		/* Level */
	int first_p;		/* Id of first point */
} seg_head;

struct GMT3_FILE_HEADER {
	int n_bins;		/* Number of blocks */
	int n_points;		/* Total number of points */
	int bsize;		/* Bin size in minutes */
	int nx_bins;		/* # of bins in 0-360 */
	int ny_bins;		/* # of bins in -90 - +90 */
	int n_segments;		/* Total number of segments */
} file_head;	

struct GMT3_BIN_HEADER {
	int first_seg_id;
	int n_segments;
} bin_head;	

int main (int argc, char **argv)
{
	int i, dims, kind, counts[16];
	int *bin_firstseg, *seg_start;
	short *bin_nseg, *seg_n, *seg_level, *pt_dx, *pt_dy;
	char *name[2] = {"Political boundaries", "Rivers"};
	
	size_t start, count;
	
	char file[512], *prefix;
	
	FILE *fp_bin, *fp_seg, *fp_pt;
	
	argc = GMT_begin (argc, argv);

	if (argc < 2) {
		fprintf (stderr, "usage: linemaker line_prefix [b|r]\n");
		fprintf (stderr, "	b = border, r = river [b]\n");
		exit (EXIT_FAILURE);
	}
	
	prefix = argv[1];
	kind = (argc == 3 && argv[2][0] == 'r') ? 1 : 0;
	
	sprintf (file, "%s.bin", prefix);
	if ((fp_bin = fopen (file, "rb")) == NULL) {
		fprintf (stderr, "linemaker: Cannot open %s bin file %s\n", name[kind], file);
		exit (EXIT_FAILURE);
	}
	sprintf (file, "%s.seg", prefix);
	if ((fp_seg = fopen (file, "rb")) == NULL) {
		fprintf (stderr, "linemaker: Cannot open %s seg file %s\n", name[kind], file);
		exit (EXIT_FAILURE);
	}
	sprintf (file, "%s.pt", prefix);
	if ((fp_pt = fopen (file, "rb")) == NULL) {
		fprintf (stderr, "linemaker: Cannot open %s point file %s\n", name[kind], file);
		exit (EXIT_FAILURE);
	}
		
	sprintf (file, "%s.cdf", prefix);
	GMT_err_fail (nc_create (file, NC_CLOBBER, &s.cdfid), file);
	
	for (i = 0; i < 16; i++) counts[i] = 0;
	
	fprintf (stderr, "linemaker: Process header file\n");

	if (fread ((void *)&file_head, sizeof (struct GMT3_FILE_HEADER), (size_t)1, fp_bin) != 1) {
		fprintf (stderr, "linemaker: Error reading file header\n");
		exit (EXIT_FAILURE);
	}
	
	s.bin_size = file_head.bsize;
	s.bin_nx = file_head.nx_bins;
	s.bin_ny = file_head.ny_bins;
	s.n_bin = file_head.n_bins;
	s.n_seg = file_head.n_segments;
	s.n_pt = file_head.n_points;
			
	bin_firstseg = (int *) GMT_memory (VNULL, s.n_bin, sizeof (int), "linemaker");
	bin_nseg     = (short *) GMT_memory (VNULL, s.n_bin, sizeof (short), "linemaker");
	
	for (i = 0; i < s.n_bin; i++) {
	
		if (fread ((void *)&bin_head, sizeof (struct GMT3_BIN_HEADER), (size_t)1, fp_bin) != 1) {
			fprintf (stderr, "linemaker: Error reading bin header %d\n", i);
			exit (EXIT_FAILURE);
		}
		
		bin_firstseg[i] = bin_head.first_seg_id;
		bin_nseg[i] = bin_head.n_segments;
	}
	
	fclose (fp_bin);
	
	fprintf (stderr, "linemaker: Process segment file\n");

	seg_n  = (short *) GMT_memory (VNULL, s.n_seg, sizeof (short), "linemaker");
	seg_level  = (short *) GMT_memory (VNULL, s.n_seg, sizeof (short), "linemaker");
	seg_start = (int *) GMT_memory (VNULL, s.n_seg, sizeof (int), "linemaker");

	for (i = 0; i < s.n_seg; i++) {
	
		if (fread ((void *)&seg_head, sizeof (struct SEGMENT_HEADER), (size_t)1, fp_seg) != 1) {
			fprintf (stderr, "linemaker: Error reading seg header %d\n", i);
			exit (EXIT_FAILURE);
		}
		
		seg_n[i] = seg_head.n;
		counts[seg_head.level]++;
		switch (seg_head.level) {	/* Reset level info to go 0-10 */
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				seg_level[i] = seg_head.level;
				break;
			case 6:
			case 7:
			case 8:
				seg_level[i] = seg_head.level - 1;
				break;
			case 10:
			case 11:
				seg_level[i] = seg_head.level - 2;
				break;
			case 13:
				seg_level[i] = seg_head.level - 3;
				break;
			default:
				fprintf (stderr, "linemaker: Error: found level == %d for segment!\n", seg_head.level);
				exit (EXIT_FAILURE);
				break;
		}
			
				
		seg_start[i] = seg_head.first_p;
	}
	
	fclose (fp_seg);

	fprintf (stderr, "linemaker: Process point file\n");

	pt_dx = (short *) GMT_memory (VNULL, s.n_pt, sizeof (short), "linemaker");
	pt_dy = (short *) GMT_memory (VNULL, s.n_pt, sizeof (short), "linemaker");

	for (i = 0; i < s.n_pt; i++) {
	
		if (fread ((void *)&p, sizeof (struct SHORT_PAIR), (size_t)1, fp_pt) != 1) {
			fprintf (stderr, "linemaker: Error reading point %d\n", i);
			exit (-1);
		}
		
		pt_dx[i] = p.dx;
		pt_dy[i] = p.dy;
	}
	
	fclose (fp_pt);
	
	/* Create array of segment addresses, npoints, and type */
			
	if (s.bin_size == 60)
		strcpy (s.units, "1/65535 of 1 degree relative to south-west corner of bin");
	else
		sprintf (s.units, "1/65535 of %d degrees relative to south-west corner of bin", s.bin_size/60);

	/* define variables */

	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_scalar", (size_t)1, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Bin_size_in_minutes", NC_INT, (size_t)1, &dims, &s.bin_size_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_bins_in_360_longitude_range", NC_INT, (size_t)1, &dims, &s.bin_nx_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_bins_in_180_degree_latitude_range", NC_INT, (size_t)1, &dims, &s.bin_ny_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_bins_in_file", NC_INT, (size_t)1, &dims, &s.n_bin_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_segments_in_file", NC_INT, (size_t)1, &dims, &s.n_seg_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_points_in_file", NC_INT, (size_t)1, &dims, &s.n_pt_id), file);
			
	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_bin_arrays", s.n_bin, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Id_of_first_segment_in_a_bin", NC_INT, (size_t)1, &dims, &s.bin_firstseg_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_segments_in_a_bin", NC_SHORT, (size_t)1, &dims, &s.bin_nseg_id), file);
			
	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_segment_arrays", s.n_seg, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_points_for_a_segment", NC_SHORT, (size_t)1, &dims, &s.seg_n_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Hierarchial_level_of_a_segment", NC_SHORT, (size_t)1, &dims, &s.seg_level_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Id_of_first_point_in_a_segment", NC_INT, (size_t)1, &dims, &s.seg_start_id), file);

	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_point_arrays", s.n_pt, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Relative_longitude_from_SW_corner_of_bin", NC_SHORT, (size_t)1, &dims, &s.pt_dx_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Relative_latitude_from_SW_corner_of_bin", NC_SHORT, (size_t)1, &dims, &s.pt_dy_id), file);

	/* assign attributes */
	
	sprintf (s.title, "%s derived from CIA WDB-II data", name[kind]);
	sprintf (s.source, "Processed by Paul Wessel and Walter H. F. Smith, 1994-%d", YEAR);
	sprintf (s.version, "%s", DVER);

	GMT_err_fail (nc_put_att_text (s.cdfid, s.pt_dx_id, "units", strlen(s.units), s.units), file);
	GMT_err_fail (nc_put_att_text (s.cdfid, s.pt_dy_id, "units", strlen(s.units), s.units), file);
	GMT_err_fail (nc_put_att_text (s.cdfid, NC_GLOBAL, "title", strlen(s.title), s.title), file);
	GMT_err_fail (nc_put_att_text (s.cdfid, NC_GLOBAL, "source", strlen(s.source), s.source), file);
	GMT_err_fail (nc_put_att_text (s.cdfid, NC_GLOBAL, "version", strlen(s.version), s.version), file);

	/* leave define mode */
	
        GMT_err_fail (nc_enddef (s.cdfid), file);

	start = 0;
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.bin_size_id, &start, &s.bin_size), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.bin_nx_id, &start, &s.bin_nx), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.bin_ny_id, &start, &s.bin_ny), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.n_bin_id, &start, &s.n_bin), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.n_seg_id, &start, &s.n_seg), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.n_pt_id, &start, &s.n_pt), file);
			
	count = s.n_bin;
        GMT_err_fail (nc_put_vara_int(s.cdfid, s.bin_firstseg_id, &start, &count, bin_firstseg), file);
        GMT_err_fail (nc_put_vara_short(s.cdfid, s.bin_nseg_id, &start, &count, bin_nseg), file);
			
	count = s.n_seg;
        GMT_err_fail (nc_put_vara_short(s.cdfid, s.seg_n_id, &start, &count, seg_n), file);
        GMT_err_fail (nc_put_vara_short(s.cdfid, s.seg_level_id, &start, &count, seg_level), file);
        GMT_err_fail (nc_put_vara_int(s.cdfid, s.seg_start_id, &start, &count, seg_start), file);
				
	count = s.n_pt;
        GMT_err_fail (nc_put_vara_short(s.cdfid, s.pt_dx_id, &start, &count, pt_dx), file);
        GMT_err_fail (nc_put_vara_short(s.cdfid, s.pt_dy_id, &start, &count, pt_dy), file);
				
        GMT_err_fail (nc_close (s.cdfid), file);
	
	GMT_free ((void *)bin_firstseg);
	GMT_free ((void *)bin_nseg);
	
	GMT_free ((void *)seg_n);
	GMT_free ((void *)seg_level);
	GMT_free ((void *)seg_start);
	
	GMT_free ((void *)pt_dx);
	GMT_free ((void *)pt_dy);

	for (i = 0; i < 16; i++) {
		if (counts[i]) fprintf (stderr, "linemaker: Level %2d: %d items\n", i, counts[i]);
	}
	
	GMT_end (argc, argv);
	
	exit (EXIT_SUCCESS);
}
