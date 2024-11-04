/*--------------------------------------------------------------------
 *	Copyright (c) 2004-2012 by J. Luis
 *
 * 	This program is part of Mirone and is free software; you can redistribute
 * 	it and/or modify it under the terms of the GNU Lesser General Public
 * 	License as published by the Free Software Foundation; either
 * 	version 2.1 of the License, or any later version.
 * 
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * 	Lesser General Public License for more details.
 *
 *	Contact info: w3.ualg.pt/~jluis/mirone
 *--------------------------------------------------------------------*/

#define ISF_NULL			9999999
#define ISF_LINE_LEN        140
#define ISF_COMM_LEN        80
#define ISF_EVID_LEN        8
#define ISF_REGION_LEN      65 
#define ISF_ETYPE_LEN       2 
#define ISF_AUTHOR_LEN      9
#define ISF_ORIGID_LEN      8
#define ISF_MAGTYPE_LEN     5
#define ISF_STA_LEN         5
#define ISF_NET_LEN         9
#define ISF_CHAN_LEN        3
#define ISF_PHASE_LEN       8
#define ISF_ARRID_LEN       8
#define ISF_F_TYPE_LEN      3
#define ISF_F_PLANE_LEN     5
#define ISF_I_LOCTYPE_LEN   6
#define ISF_COUNTRY_LEN     3
#define ISF_POSTCODE_LEN    10
#define ISF_I_SCALE_LEN     5
#define ISF_AUXID_LEN       4
#define ISF_GROUPID_LEN     8

#define ISF_NUM_STA     200
#define ISF_NUM_PARAM   100

char isf_error[ISF_LINE_LEN*2];
char isf_prev_line_type[ISF_LINE_LEN];

/* Utilities used by other functions */

int check_prev_line_type(char *line_type);
void print_float(FILE *fp,float x,int width,int max_prec);
int is_null(int i);
int check_int(char *s);
int check_float(char *s);
int partline(char *part,char *line,int offset,int numchars);
int check_whole(char *s);
int all_blank(char *s);


/* Functions to set variables to impossible values */

int nullify_data_type(char *data_type, char *subtype, char *data_format, char *subformat);

int nullify_event_id(char *evid, char *region);

int nullify_origin(int *yyyy, int *mm, int *dd, int *hh, int *mi, 
                int *ss, int *msec, char *timfix, float *stime, float *sdobs,
				float *lat, float *lon, char *epifix, float *smaj, float *smin,
                int *strike, float *depth, char *depfix, float *sdepth,
                int *ndef, int *nsta, int *gap, float *mindist,
                float *maxdist, char *antype, char *loctype, char *etype,
                char *author, char *origid);

int nullify_origin_param(char **param, char **value, int *numparam);

int nullify_momten(int *scale_factor, float *scalar_moment, float *fclvd,
                    float *mrr, float *mtt, float *mpp, float *mrt, 
                    float *mtp, float *mpr,int *nsta1, int *nsta2, char *author,
                    float *scalar_moment_unc, float *fclvd_unc,
                    float *mrr_unc, float *mtt_unc, float *mpp_unc,
                    float *mrt_unc, float *mtp_unc, float *mpr_unc,
                    int *ncomp1, int *ncomp2, float *duration);

int nullify_fault_plane (char *f_type, float *strike, float *dip, float *rake,
                    int *np, int *ns, char *f_plane, char *author);

int nullify_axes(int *scale_factor, float *t_val, float *t_azim,
                    float *t_pl, float *b_val, float *b_azim, float *b_pl,
                    float *p_val, float *p_azim, float *p_pl, char *author);

int nullify_axes_err(float *t_val_unc, float *t_azim_unc, float *t_pl_unc,
                    float *b_val_unc, float *b_azim_unc, float *b_pl_unc,
                    float *p_val_unc, float *p_azim_unc, float *p_pl_unc,
                    float *fclvd);

int nullify_effects(char *heard, char *felt, char *damage,
                    char *casualties, char *uplift, char *subsidence,
                    char *fault, char *tsunami, char *seiche, char *volcano,
                    char *acoustic, char *gravity, char *t_wave, 
                    char *liquification, char *geyser, char *landslide,
                    char *sandblow, char *cracks, char *lights, char *odours,
                    char *loctype, float *lat, float *lon, float *dist,
                    float *azim, char *country, char *postcode, char *net,
                    char *sta, float *intensity1, char *modifier,
                    float *intensity2, char *scale, char* author);

int nullify_netmag(char *magtype, char* magind, float* mag, float* magerr, int* nsta, char *author, char* origid);
int nullify_netmag_sta(char **sta, int *n);
int  nullify_netmag_basis(char *param, char *value);
int nullify_phase(char *sta, float *dist, float *esaz, char *phase,
                    int *hh, int *mi, int *ss,
                    int *msec, float *timeres, float *azim, float *azimres,
                    float *slow, float *slowres, char *timedef, char *azimdef,
                    char *slowdef, float *snr, float *amp, float *per,
                    char *picktype, char *sp_fm, char *detchar, char *magtype,
                    char *magind, float *mag, char *arrid);

int nullify_phase_measure(char **param, char **value, int *numparam);
int nullify_phase_origid(char *origid);
int nullify_phase_info(char *net, char *chan, char *filter, float *filter_min,
                    float *filter_max, char *phase, int *yyyy, int *mm,
                    int *dd, float *time_unc, float *time_weight, 
                    float *azim_unc, float *azim_weight, float *slow_unc,
                    float *slow_weight, float *amp_unc, float *per_unc,
                    float *mag_unc, char *author, char *arrid);

int nullify_phase_min(float *timeoffset, float *azoffset, float *slowoffset, float *ampoffset, float *peroffset,
                    float *magoffset);

int nullify_phase_max(float *timeoffset, float *azoffset, float *slowoffset, float *ampoffset, float *peroffset,
                    float *magoffset);

int nullify_phase_correc(float *timecorr, float *azcorr,float *slowcorr, float *ampcorr, float *percorr,float *magcorr);

int nullify_phase_original(char *chan, char *sta, int *yyyy, int *mm,
                        int *dd, int *hh, int *mi, int *ss, int *msec,
                        float *azim, float *slow, float *amp, float *per,
                        float *mag);

int nullify_comment(char *comment);

/* Functions for reading ISF format files. */

int read_data_type(char *line, char *data_type, char *subtype, char *data_format,char *subformat);
int read_event_id(char *line, char *evid, char *region);
int read_origin_head(char *line);
int read_origin(char *line, int *yyyy, int *mm, int *dd, int *hh, int *mi, 
                int *ss, int *msec, char *timfix, float *stime, float *sdobs,
				float *lat, float *lon, char *epifix, float *smaj, float *smin,
                int *strike, float *depth, char *depfix, float *sdepth,
                int *ndef, int *nsta, int *gap, float *mindist,
                float *maxdist, char *antype, char *loctype, char *etype,
                char *author, char *origid);

int read_origin_prime(char *line);
int read_origin_centroid(char *line);
int read_origin_param(char *line, char **param, char **value, char **error, int *n);
int read_momten_head_1(char *line);
int read_momten_head_2(char *line);

int read_momten_line_1(char *line, int *scale_factor, float *scalar_moment,
                       float *fclvd, float *mrr, float *mtt, float *mpp,
                       float *mrt, float *mtp, float *mpr, int *nsta1,
                       int *nsta2, char *author);


int read_momten_line_2(char *line, float *scalar_moment_unc, float *fclvd_unc,
                       float *mrr_unc, float *mtt_unc, float *mpp_unc,
                       float *mrt_unc, float *mtp_unc, float *mpr_unc,
                       int *ncomp1, int *ncomp2, float *duration);

int read_fault_plane_head (char *line);

int read_fault_plane (char *line, char *f_type, float *strike, float *dip,
                       float *rake, int *np, int *ns, char *f_plane,
                       char *author);
					  
int read_axes_head(char *line);

int read_axes_err_head(char *line);

int read_axes(char *line, int *scale_factor, float *t_val, float *t_azim,
              float *t_pl, float *b_val, float *b_azim, float *b_pl,
              float *p_val, float *p_azim, float *p_pl, char *author);

int read_axes_err(char *line, float *t_val_unc, float *t_azim_unc,
                  float *t_pl_unc, float *b_val_unc, float *b_azim_unc,
                  float *b_pl_unc, float *p_val_unc, float *p_azim_unc,
                  float *p_pl_unc, float *fclvd);

int read_netmag_head(char *line);

int read_netmag(char *line, char *magtype, char* magind, float* mag,
                float* magerr, int* nsta, char *author, char* origid);

int read_netmag_sta(char *line, char **sta, int* n);

int read_netmag_basis(char *line, char *param, char *value);

int read_effects_head(char *line);

int read_effects(char *line, char *heard, char *felt, char *damage,
                 char *casualties, char *uplift, char *subsidence,
                 char *fault, char *tsunami, char *seiche, char *volcano,
                 char *acoustic, char *gravity, char *t_wave, 
                 char *liquification, char *geyser, char *landslide,
                 char *sandblow, char *cracks, char *lights, char *odours,
                 char *loctype, float *lat, float *lon, float *dist,
                 float *azim, char *country, char *postcode, char *net,
                 char *sta, float *intensity1, char *modifier,
                 float *intensity2, char *scale, char* author);

int read_phase_head(char *line);

int read_phase_origid(char *line, char *origid);

int read_phase(char *line, char *sta, float *dist, float *esaz,
               char *phase,int *hh,
               int *mi, int *ss, int *msec, float *timeres, float *azim,
               float *azimres, float *slow, float *slowres, char *timedef,
               char *azimdef, char *slowdef, float *snr, float *amp,
               float *per, char *picktype, char *sp_fm, char *detchar,
               char *magtype, char *magind, float *mag, char *arrid);

int read_phase_info_head(char *line);

int read_phase_info(char *line, char *net, char *chan, char *filter, 
					float *filter_min,
                    float *filter_max, char *phase, int *yyyy, int *mm,
                    int *dd, float *time_unc, float *time_weight, 
                    float *azim_unc, float *azim_weight, float *slow_unc,
                    float *slow_weight, float *amp_unc, float *per_unc,
                    float *mag_unc, char *author, char *arrid);

int read_phase_measure(char *line, char **param, char **value, char **error, int *n);

int read_phase_min(char *line, float *timeoffset, float *azoffset,
                   float *slowoffset, float *ampoffset, float *peroffset,
                   float *magoffset);

int read_phase_max(char *line, float *timeoffset, float *azoffset,
                   float *slowoffset, float *ampoffset, float *peroffset,
                   float *magoffset);

int read_phase_correc(char *line, float *timecorr, float *azcorr,
                      float *slowcorr, float *ampcorr, float *percorr,
                      float *magcorr );

int read_phase_original(char *line, char *chan, char *sta, int *yyyy, int *mm,
                        int *dd, int *hh, int *mi, int *ss, int *msec,
                        float *azim, float *slow, float *amp, float *per,
                        float *mag);


int read_comment(char *line,char *comment);

int read_stop(char *line);

int read_event_data(FILE *fp, char *line, int *yyyy, int *mm, int *dd, int *hh, int *mi, 
                int *ss, int *msec, char *timfix, float *stime, float *sdobs,
                float *lat, float *lon, char *epifix, float *smaj, float *smin,
                int *strike, float *depth, char *depfix, float *sdepth,
                int *ndef, int *nsta, int *gap, float *mindist, float *maxdist,
                char *antype, char *loctype, char *etype, char *author, char *origid,
                float *lats, float *lons, float *rms, float *depths, int *years,
		int *months, int *days, int *hours, int *minutes, int *idx_min_rms);

int read_mags(FILE *fp, char *line, char *magtype, char* magind, float* mag, float* magerr,
		int* nsta, char* author, char* origid, char **mag_t, float *mags);
