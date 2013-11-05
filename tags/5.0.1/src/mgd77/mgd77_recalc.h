/*--------------------------------------------------------------------
 *	$Id$
 *
 *    Copyright (c) 2004-2012 by P. Wessel
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* Here lies some defines and a structure of items that are used when we
 * must recalculate anomalies from original observations (e.g., mag from mtf1
 * and the latest IGRF) and undo Precision Depth Recorder wrap-arounds.
 * THese issues come into play if the mgd77+ file has been updated with E77
 * information that have activated these corrections.  THe data returned
 * by the calling program will have all corrections applied to them unless
 * the master switch in the MGD77 structure that controls adjustments have
 * been turned OFF [Default is ON].
 */

#define N_E77_CORR_FIELDS	5
/* The array indices 0-3 for these 4 fields */
#define E77_CORR_FIELD_TWT	(MGD77_COL_ADJ_TWT-1)
#define E77_CORR_FIELD_DEPTH	(MGD77_COL_ADJ_DEPTH-1)
#define E77_CORR_FIELD_MAG	(MGD77_COL_ADJ_MAG-1)
#define E77_CORR_FIELD_FAA	(MGD77_COL_ADJ_FAA-1)
#define E77_CORR_FIELD_FAA_EOT	(MGD77_COL_ADJ_FAA_EOT-1)

#define N_E77_AUX_FIELDS	7
/* The array indices 0-6 for the 7 aux fields */
#define E77_AUX_FIELD_TIME	0
#define E77_AUX_FIELD_LAT	1
#define E77_AUX_FIELD_LON	2
#define E77_AUX_FIELD_TWT	3
#define E77_AUX_FIELD_MTF1	4
#define E77_AUX_FIELD_GOBS	5
#define E77_AUX_FIELD_EOT	6

struct MGD77_E77_APPLY {
	/* Structure with information about specific corrections to data columns:
	   1. Undo PDR wrap-around effects in the observed TWT
	   2. Recalculate depth using Carter corrections from twt
	   3. Recalculate mag using the latest IGRF reference field and mtf1
	   4. Recalculate faa using the latest IGF80 reference field and gobs
	*/
	GMT_LONG apply_corrections;	/* TRUE if one or more corrections are requested */
	GMT_LONG correction_requested[N_E77_CORR_FIELDS];	/* TRUE for each field we must correct */
	GMT_LONG got_it[MGD77_SET_COLS];	/* TRUE for each original MGD77 column that was requested among the output columns */
	int needed[N_E77_AUX_FIELDS];	/* 0 if aux field not used, 1 if part of output cols, 2 is allocated separately */
	int col[N_E77_CORR_FIELDS];	/* The output column number for each corrected field */
	int id[N_E77_CORR_FIELDS];	/* The id number for each corrected field */
	double *aux[N_E77_AUX_FIELDS];
};
