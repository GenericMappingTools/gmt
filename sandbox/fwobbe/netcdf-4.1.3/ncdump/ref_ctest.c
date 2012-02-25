#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
	   (void) fprintf(stderr, "line %d of %s: %s\n", line, file, nc_strerror(stat));
        exit(1);
    }
}

int
main() {			/* create ctest0.nc */

   int  stat;			/* return status */
   int  ncid;			/* netCDF id */

   /* dimension ids */
   int Dr_dim;
   int D1_dim;
   int D2_dim;
   int D3_dim;
   int dim_MINUS_name_MINUS_dashes_dim;
   int dim_PERIOD_name_PERIOD_dots_dim;
   int dim_PLUS_name_PLUS_plusses_dim;
   int dim_ATSIGN_name_ATSIGN_ats_dim;

   /* dimension lengths */
   size_t Dr_len = NC_UNLIMITED;
   size_t D1_len = 1;
   size_t D2_len = 2;
   size_t D3_len = 3;
   size_t dim_MINUS_name_MINUS_dashes_len = 4;
   size_t dim_PERIOD_name_PERIOD_dots_len = 5;
   size_t dim_PLUS_name_PLUS_plusses_len = 6;
   size_t dim_ATSIGN_name_ATSIGN_ats_len = 7;

   /* variable ids */
   int c_id;
   int b_id;
   int s_id;
   int i_id;
   int f_id;
   int d_id;
   int cr_id;
   int br_id;
   int sr_id;
   int ir_id;
   int fr_id;
   int dr_id;
   int c1_id;
   int b1_id;
   int s1_id;
   int i1_id;
   int f1_id;
   int d1_id;
   int c2_id;
   int b2_id;
   int s2_id;
   int i2_id;
   int f2_id;
   int d2_id;
   int c3_id;
   int b3_id;
   int s3_id;
   int i3_id;
   int f3_id;
   int d3_id;
   int cr1_id;
   int br2_id;
   int sr3_id;
   int f11_id;
   int d12_id;
   int c13_id;
   int s21_id;
   int i22_id;
   int f23_id;
   int c31_id;
   int b32_id;
   int s33_id;
   int sr11_id;
   int ir12_id;
   int fr13_id;
   int cr21_id;
   int br22_id;
   int sr23_id;
   int fr31_id;
   int dr32_id;
   int cr33_id;
   int c111_id;
   int b112_id;
   int s113_id;
   int f121_id;
   int d122_id;
   int c123_id;
   int s131_id;
   int i132_id;
   int f133_id;
   int f211_id;
   int d212_id;
   int c213_id;
   int s221_id;
   int i222_id;
   int f223_id;
   int c231_id;
   int b232_id;
   int s233_id;
   int s311_id;
   int i312_id;
   int f313_id;
   int var_MINUS_name_MINUS_dashes_id;
   int var_PERIOD_name_PERIOD_dots_id;
   int var_PLUS_name_PLUS_plusses_id;
   int var_ATSIGN_name_ATSIGN_ats_id;

   /* rank (number of dimensions) for each variable */
#  define RANK_c 0
#  define RANK_b 0
#  define RANK_s 0
#  define RANK_i 0
#  define RANK_f 0
#  define RANK_d 0
#  define RANK_cr 1
#  define RANK_br 1
#  define RANK_sr 1
#  define RANK_ir 1
#  define RANK_fr 1
#  define RANK_dr 1
#  define RANK_c1 1
#  define RANK_b1 1
#  define RANK_s1 1
#  define RANK_i1 1
#  define RANK_f1 1
#  define RANK_d1 1
#  define RANK_c2 1
#  define RANK_b2 1
#  define RANK_s2 1
#  define RANK_i2 1
#  define RANK_f2 1
#  define RANK_d2 1
#  define RANK_c3 1
#  define RANK_b3 1
#  define RANK_s3 1
#  define RANK_i3 1
#  define RANK_f3 1
#  define RANK_d3 1
#  define RANK_cr1 2
#  define RANK_br2 2
#  define RANK_sr3 2
#  define RANK_f11 2
#  define RANK_d12 2
#  define RANK_c13 2
#  define RANK_s21 2
#  define RANK_i22 2
#  define RANK_f23 2
#  define RANK_c31 2
#  define RANK_b32 2
#  define RANK_s33 2
#  define RANK_sr11 3
#  define RANK_ir12 3
#  define RANK_fr13 3
#  define RANK_cr21 3
#  define RANK_br22 3
#  define RANK_sr23 3
#  define RANK_fr31 3
#  define RANK_dr32 3
#  define RANK_cr33 3
#  define RANK_c111 3
#  define RANK_b112 3
#  define RANK_s113 3
#  define RANK_f121 3
#  define RANK_d122 3
#  define RANK_c123 3
#  define RANK_s131 3
#  define RANK_i132 3
#  define RANK_f133 3
#  define RANK_f211 3
#  define RANK_d212 3
#  define RANK_c213 3
#  define RANK_s221 3
#  define RANK_i222 3
#  define RANK_f223 3
#  define RANK_c231 3
#  define RANK_b232 3
#  define RANK_s233 3
#  define RANK_s311 3
#  define RANK_i312 3
#  define RANK_f313 3
#  define RANK_var_MINUS_name_MINUS_dashes 0
#  define RANK_var_PERIOD_name_PERIOD_dots 0
#  define RANK_var_PLUS_name_PLUS_plusses 0
#  define RANK_var_ATSIGN_name_ATSIGN_ats 0

   /* variable shapes */
   int cr_dims[RANK_cr];
   int br_dims[RANK_br];
   int sr_dims[RANK_sr];
   int ir_dims[RANK_ir];
   int fr_dims[RANK_fr];
   int dr_dims[RANK_dr];
   int c1_dims[RANK_c1];
   int b1_dims[RANK_b1];
   int s1_dims[RANK_s1];
   int i1_dims[RANK_i1];
   int f1_dims[RANK_f1];
   int d1_dims[RANK_d1];
   int c2_dims[RANK_c2];
   int b2_dims[RANK_b2];
   int s2_dims[RANK_s2];
   int i2_dims[RANK_i2];
   int f2_dims[RANK_f2];
   int d2_dims[RANK_d2];
   int c3_dims[RANK_c3];
   int b3_dims[RANK_b3];
   int s3_dims[RANK_s3];
   int i3_dims[RANK_i3];
   int f3_dims[RANK_f3];
   int d3_dims[RANK_d3];
   int cr1_dims[RANK_cr1];
   int br2_dims[RANK_br2];
   int sr3_dims[RANK_sr3];
   int f11_dims[RANK_f11];
   int d12_dims[RANK_d12];
   int c13_dims[RANK_c13];
   int s21_dims[RANK_s21];
   int i22_dims[RANK_i22];
   int f23_dims[RANK_f23];
   int c31_dims[RANK_c31];
   int b32_dims[RANK_b32];
   int s33_dims[RANK_s33];
   int sr11_dims[RANK_sr11];
   int ir12_dims[RANK_ir12];
   int fr13_dims[RANK_fr13];
   int cr21_dims[RANK_cr21];
   int br22_dims[RANK_br22];
   int sr23_dims[RANK_sr23];
   int fr31_dims[RANK_fr31];
   int dr32_dims[RANK_dr32];
   int cr33_dims[RANK_cr33];
   int c111_dims[RANK_c111];
   int b112_dims[RANK_b112];
   int s113_dims[RANK_s113];
   int f121_dims[RANK_f121];
   int d122_dims[RANK_d122];
   int c123_dims[RANK_c123];
   int s131_dims[RANK_s131];
   int i132_dims[RANK_i132];
   int f133_dims[RANK_f133];
   int f211_dims[RANK_f211];
   int d212_dims[RANK_d212];
   int c213_dims[RANK_c213];
   int s221_dims[RANK_s221];
   int i222_dims[RANK_i222];
   int f223_dims[RANK_f223];
   int c231_dims[RANK_c231];
   int b232_dims[RANK_b232];
   int s233_dims[RANK_s233];
   int s311_dims[RANK_s311];
   int i312_dims[RANK_i312];
   int f313_dims[RANK_f313];

   /* attribute vectors */
   int c_att_MINUS_name_MINUS_dashes[1];
   int c_att_PERIOD_name_PERIOD_dots[1];
   int c_att_PLUS_name_PLUS_plusses[1];
   int c_att_ATSIGN_name_ATSIGN_ats[1];
   int s_b[4];
   short s_s[3];
   int i_i[3];
   float i_f[3];
   double i_d[3];
   int cdf_Gb[2];
   short cdf_Gs[3];
   int cdf_Gi[3];
   float cdf_Gf[3];
   double cdf_Gd[3];
   int cdf_Gatt_MINUS_name_MINUS_dashes[1];
   int cdf_Gatt_PERIOD_name_PERIOD_dots[1];
   int cdf_Gatt_PLUS_name_PLUS_plusses[1];
   int cdf_Gatt_ATSIGN_name_ATSIGN_ats[1];

   /* enter define mode */
   stat = nc_create("ctest0.nc", NC_CLOBBER, &ncid);
   check_err(stat,__LINE__,__FILE__);

   /* define dimensions */
   stat = nc_def_dim(ncid, "Dr", Dr_len, &Dr_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "D1", D1_len, &D1_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "D2", D2_len, &D2_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "D3", D3_len, &D3_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "dim-name-dashes", dim_MINUS_name_MINUS_dashes_len, &dim_MINUS_name_MINUS_dashes_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "dim.name.dots", dim_PERIOD_name_PERIOD_dots_len, &dim_PERIOD_name_PERIOD_dots_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "dim+name+plusses", dim_PLUS_name_PLUS_plusses_len, &dim_PLUS_name_PLUS_plusses_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "dim@name@ats", dim_ATSIGN_name_ATSIGN_ats_len, &dim_ATSIGN_name_ATSIGN_ats_dim);
   check_err(stat,__LINE__,__FILE__);

   /* define variables */

   stat = nc_def_var(ncid, "c", NC_CHAR, RANK_c, 0, &c_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "b", NC_BYTE, RANK_b, 0, &b_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "s", NC_SHORT, RANK_s, 0, &s_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "i", NC_INT, RANK_i, 0, &i_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "f", NC_FLOAT, RANK_f, 0, &f_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "d", NC_DOUBLE, RANK_d, 0, &d_id);
   check_err(stat,__LINE__,__FILE__);

   cr_dims[0] = Dr_dim;
   stat = nc_def_var(ncid, "cr", NC_CHAR, RANK_cr, cr_dims, &cr_id);
   check_err(stat,__LINE__,__FILE__);

   br_dims[0] = Dr_dim;
   stat = nc_def_var(ncid, "br", NC_BYTE, RANK_br, br_dims, &br_id);
   check_err(stat,__LINE__,__FILE__);

   sr_dims[0] = Dr_dim;
   stat = nc_def_var(ncid, "sr", NC_SHORT, RANK_sr, sr_dims, &sr_id);
   check_err(stat,__LINE__,__FILE__);

   ir_dims[0] = Dr_dim;
   stat = nc_def_var(ncid, "ir", NC_INT, RANK_ir, ir_dims, &ir_id);
   check_err(stat,__LINE__,__FILE__);

   fr_dims[0] = Dr_dim;
   stat = nc_def_var(ncid, "fr", NC_FLOAT, RANK_fr, fr_dims, &fr_id);
   check_err(stat,__LINE__,__FILE__);

   dr_dims[0] = Dr_dim;
   stat = nc_def_var(ncid, "dr", NC_DOUBLE, RANK_dr, dr_dims, &dr_id);
   check_err(stat,__LINE__,__FILE__);

   c1_dims[0] = D1_dim;
   stat = nc_def_var(ncid, "c1", NC_CHAR, RANK_c1, c1_dims, &c1_id);
   check_err(stat,__LINE__,__FILE__);

   b1_dims[0] = D1_dim;
   stat = nc_def_var(ncid, "b1", NC_BYTE, RANK_b1, b1_dims, &b1_id);
   check_err(stat,__LINE__,__FILE__);

   s1_dims[0] = D1_dim;
   stat = nc_def_var(ncid, "s1", NC_SHORT, RANK_s1, s1_dims, &s1_id);
   check_err(stat,__LINE__,__FILE__);

   i1_dims[0] = D1_dim;
   stat = nc_def_var(ncid, "i1", NC_INT, RANK_i1, i1_dims, &i1_id);
   check_err(stat,__LINE__,__FILE__);

   f1_dims[0] = D1_dim;
   stat = nc_def_var(ncid, "f1", NC_FLOAT, RANK_f1, f1_dims, &f1_id);
   check_err(stat,__LINE__,__FILE__);

   d1_dims[0] = D1_dim;
   stat = nc_def_var(ncid, "d1", NC_DOUBLE, RANK_d1, d1_dims, &d1_id);
   check_err(stat,__LINE__,__FILE__);

   c2_dims[0] = D2_dim;
   stat = nc_def_var(ncid, "c2", NC_CHAR, RANK_c2, c2_dims, &c2_id);
   check_err(stat,__LINE__,__FILE__);

   b2_dims[0] = D2_dim;
   stat = nc_def_var(ncid, "b2", NC_BYTE, RANK_b2, b2_dims, &b2_id);
   check_err(stat,__LINE__,__FILE__);

   s2_dims[0] = D2_dim;
   stat = nc_def_var(ncid, "s2", NC_SHORT, RANK_s2, s2_dims, &s2_id);
   check_err(stat,__LINE__,__FILE__);

   i2_dims[0] = D2_dim;
   stat = nc_def_var(ncid, "i2", NC_INT, RANK_i2, i2_dims, &i2_id);
   check_err(stat,__LINE__,__FILE__);

   f2_dims[0] = D2_dim;
   stat = nc_def_var(ncid, "f2", NC_FLOAT, RANK_f2, f2_dims, &f2_id);
   check_err(stat,__LINE__,__FILE__);

   d2_dims[0] = D2_dim;
   stat = nc_def_var(ncid, "d2", NC_DOUBLE, RANK_d2, d2_dims, &d2_id);
   check_err(stat,__LINE__,__FILE__);

   c3_dims[0] = D3_dim;
   stat = nc_def_var(ncid, "c3", NC_CHAR, RANK_c3, c3_dims, &c3_id);
   check_err(stat,__LINE__,__FILE__);

   b3_dims[0] = D3_dim;
   stat = nc_def_var(ncid, "b3", NC_BYTE, RANK_b3, b3_dims, &b3_id);
   check_err(stat,__LINE__,__FILE__);

   s3_dims[0] = D3_dim;
   stat = nc_def_var(ncid, "s3", NC_SHORT, RANK_s3, s3_dims, &s3_id);
   check_err(stat,__LINE__,__FILE__);

   i3_dims[0] = D3_dim;
   stat = nc_def_var(ncid, "i3", NC_INT, RANK_i3, i3_dims, &i3_id);
   check_err(stat,__LINE__,__FILE__);

   f3_dims[0] = D3_dim;
   stat = nc_def_var(ncid, "f3", NC_FLOAT, RANK_f3, f3_dims, &f3_id);
   check_err(stat,__LINE__,__FILE__);

   d3_dims[0] = D3_dim;
   stat = nc_def_var(ncid, "d3", NC_DOUBLE, RANK_d3, d3_dims, &d3_id);
   check_err(stat,__LINE__,__FILE__);

   cr1_dims[0] = Dr_dim;
   cr1_dims[1] = D1_dim;
   stat = nc_def_var(ncid, "cr1", NC_CHAR, RANK_cr1, cr1_dims, &cr1_id);
   check_err(stat,__LINE__,__FILE__);

   br2_dims[0] = Dr_dim;
   br2_dims[1] = D2_dim;
   stat = nc_def_var(ncid, "br2", NC_BYTE, RANK_br2, br2_dims, &br2_id);
   check_err(stat,__LINE__,__FILE__);

   sr3_dims[0] = Dr_dim;
   sr3_dims[1] = D3_dim;
   stat = nc_def_var(ncid, "sr3", NC_SHORT, RANK_sr3, sr3_dims, &sr3_id);
   check_err(stat,__LINE__,__FILE__);

   f11_dims[0] = D1_dim;
   f11_dims[1] = D1_dim;
   stat = nc_def_var(ncid, "f11", NC_FLOAT, RANK_f11, f11_dims, &f11_id);
   check_err(stat,__LINE__,__FILE__);

   d12_dims[0] = D1_dim;
   d12_dims[1] = D2_dim;
   stat = nc_def_var(ncid, "d12", NC_DOUBLE, RANK_d12, d12_dims, &d12_id);
   check_err(stat,__LINE__,__FILE__);

   c13_dims[0] = D1_dim;
   c13_dims[1] = D3_dim;
   stat = nc_def_var(ncid, "c13", NC_CHAR, RANK_c13, c13_dims, &c13_id);
   check_err(stat,__LINE__,__FILE__);

   s21_dims[0] = D2_dim;
   s21_dims[1] = D1_dim;
   stat = nc_def_var(ncid, "s21", NC_SHORT, RANK_s21, s21_dims, &s21_id);
   check_err(stat,__LINE__,__FILE__);

   i22_dims[0] = D2_dim;
   i22_dims[1] = D2_dim;
   stat = nc_def_var(ncid, "i22", NC_INT, RANK_i22, i22_dims, &i22_id);
   check_err(stat,__LINE__,__FILE__);

   f23_dims[0] = D2_dim;
   f23_dims[1] = D3_dim;
   stat = nc_def_var(ncid, "f23", NC_FLOAT, RANK_f23, f23_dims, &f23_id);
   check_err(stat,__LINE__,__FILE__);

   c31_dims[0] = D3_dim;
   c31_dims[1] = D1_dim;
   stat = nc_def_var(ncid, "c31", NC_CHAR, RANK_c31, c31_dims, &c31_id);
   check_err(stat,__LINE__,__FILE__);

   b32_dims[0] = D3_dim;
   b32_dims[1] = D2_dim;
   stat = nc_def_var(ncid, "b32", NC_BYTE, RANK_b32, b32_dims, &b32_id);
   check_err(stat,__LINE__,__FILE__);

   s33_dims[0] = D3_dim;
   s33_dims[1] = D3_dim;
   stat = nc_def_var(ncid, "s33", NC_SHORT, RANK_s33, s33_dims, &s33_id);
   check_err(stat,__LINE__,__FILE__);

   sr11_dims[0] = Dr_dim;
   sr11_dims[1] = D1_dim;
   sr11_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "sr11", NC_SHORT, RANK_sr11, sr11_dims, &sr11_id);
   check_err(stat,__LINE__,__FILE__);

   ir12_dims[0] = Dr_dim;
   ir12_dims[1] = D1_dim;
   ir12_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "ir12", NC_INT, RANK_ir12, ir12_dims, &ir12_id);
   check_err(stat,__LINE__,__FILE__);

   fr13_dims[0] = Dr_dim;
   fr13_dims[1] = D1_dim;
   fr13_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "fr13", NC_FLOAT, RANK_fr13, fr13_dims, &fr13_id);
   check_err(stat,__LINE__,__FILE__);

   cr21_dims[0] = Dr_dim;
   cr21_dims[1] = D2_dim;
   cr21_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "cr21", NC_CHAR, RANK_cr21, cr21_dims, &cr21_id);
   check_err(stat,__LINE__,__FILE__);

   br22_dims[0] = Dr_dim;
   br22_dims[1] = D2_dim;
   br22_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "br22", NC_BYTE, RANK_br22, br22_dims, &br22_id);
   check_err(stat,__LINE__,__FILE__);

   sr23_dims[0] = Dr_dim;
   sr23_dims[1] = D2_dim;
   sr23_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "sr23", NC_SHORT, RANK_sr23, sr23_dims, &sr23_id);
   check_err(stat,__LINE__,__FILE__);

   fr31_dims[0] = Dr_dim;
   fr31_dims[1] = D3_dim;
   fr31_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "fr31", NC_FLOAT, RANK_fr31, fr31_dims, &fr31_id);
   check_err(stat,__LINE__,__FILE__);

   dr32_dims[0] = Dr_dim;
   dr32_dims[1] = D3_dim;
   dr32_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "dr32", NC_DOUBLE, RANK_dr32, dr32_dims, &dr32_id);
   check_err(stat,__LINE__,__FILE__);

   cr33_dims[0] = Dr_dim;
   cr33_dims[1] = D3_dim;
   cr33_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "cr33", NC_CHAR, RANK_cr33, cr33_dims, &cr33_id);
   check_err(stat,__LINE__,__FILE__);

   c111_dims[0] = D1_dim;
   c111_dims[1] = D1_dim;
   c111_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "c111", NC_CHAR, RANK_c111, c111_dims, &c111_id);
   check_err(stat,__LINE__,__FILE__);

   b112_dims[0] = D1_dim;
   b112_dims[1] = D1_dim;
   b112_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "b112", NC_BYTE, RANK_b112, b112_dims, &b112_id);
   check_err(stat,__LINE__,__FILE__);

   s113_dims[0] = D1_dim;
   s113_dims[1] = D1_dim;
   s113_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "s113", NC_SHORT, RANK_s113, s113_dims, &s113_id);
   check_err(stat,__LINE__,__FILE__);

   f121_dims[0] = D1_dim;
   f121_dims[1] = D2_dim;
   f121_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "f121", NC_FLOAT, RANK_f121, f121_dims, &f121_id);
   check_err(stat,__LINE__,__FILE__);

   d122_dims[0] = D1_dim;
   d122_dims[1] = D2_dim;
   d122_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "d122", NC_DOUBLE, RANK_d122, d122_dims, &d122_id);
   check_err(stat,__LINE__,__FILE__);

   c123_dims[0] = D1_dim;
   c123_dims[1] = D2_dim;
   c123_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "c123", NC_CHAR, RANK_c123, c123_dims, &c123_id);
   check_err(stat,__LINE__,__FILE__);

   s131_dims[0] = D1_dim;
   s131_dims[1] = D3_dim;
   s131_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "s131", NC_SHORT, RANK_s131, s131_dims, &s131_id);
   check_err(stat,__LINE__,__FILE__);

   i132_dims[0] = D1_dim;
   i132_dims[1] = D3_dim;
   i132_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "i132", NC_INT, RANK_i132, i132_dims, &i132_id);
   check_err(stat,__LINE__,__FILE__);

   f133_dims[0] = D1_dim;
   f133_dims[1] = D3_dim;
   f133_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "f133", NC_FLOAT, RANK_f133, f133_dims, &f133_id);
   check_err(stat,__LINE__,__FILE__);

   f211_dims[0] = D2_dim;
   f211_dims[1] = D1_dim;
   f211_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "f211", NC_FLOAT, RANK_f211, f211_dims, &f211_id);
   check_err(stat,__LINE__,__FILE__);

   d212_dims[0] = D2_dim;
   d212_dims[1] = D1_dim;
   d212_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "d212", NC_DOUBLE, RANK_d212, d212_dims, &d212_id);
   check_err(stat,__LINE__,__FILE__);

   c213_dims[0] = D2_dim;
   c213_dims[1] = D1_dim;
   c213_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "c213", NC_CHAR, RANK_c213, c213_dims, &c213_id);
   check_err(stat,__LINE__,__FILE__);

   s221_dims[0] = D2_dim;
   s221_dims[1] = D2_dim;
   s221_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "s221", NC_SHORT, RANK_s221, s221_dims, &s221_id);
   check_err(stat,__LINE__,__FILE__);

   i222_dims[0] = D2_dim;
   i222_dims[1] = D2_dim;
   i222_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "i222", NC_INT, RANK_i222, i222_dims, &i222_id);
   check_err(stat,__LINE__,__FILE__);

   f223_dims[0] = D2_dim;
   f223_dims[1] = D2_dim;
   f223_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "f223", NC_FLOAT, RANK_f223, f223_dims, &f223_id);
   check_err(stat,__LINE__,__FILE__);

   c231_dims[0] = D2_dim;
   c231_dims[1] = D3_dim;
   c231_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "c231", NC_CHAR, RANK_c231, c231_dims, &c231_id);
   check_err(stat,__LINE__,__FILE__);

   b232_dims[0] = D2_dim;
   b232_dims[1] = D3_dim;
   b232_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "b232", NC_BYTE, RANK_b232, b232_dims, &b232_id);
   check_err(stat,__LINE__,__FILE__);

   s233_dims[0] = D2_dim;
   s233_dims[1] = D3_dim;
   s233_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "s233", NC_SHORT, RANK_s233, s233_dims, &s233_id);
   check_err(stat,__LINE__,__FILE__);

   s311_dims[0] = D3_dim;
   s311_dims[1] = D1_dim;
   s311_dims[2] = D1_dim;
   stat = nc_def_var(ncid, "s311", NC_SHORT, RANK_s311, s311_dims, &s311_id);
   check_err(stat,__LINE__,__FILE__);

   i312_dims[0] = D3_dim;
   i312_dims[1] = D1_dim;
   i312_dims[2] = D2_dim;
   stat = nc_def_var(ncid, "i312", NC_INT, RANK_i312, i312_dims, &i312_id);
   check_err(stat,__LINE__,__FILE__);

   f313_dims[0] = D3_dim;
   f313_dims[1] = D1_dim;
   f313_dims[2] = D3_dim;
   stat = nc_def_var(ncid, "f313", NC_FLOAT, RANK_f313, f313_dims, &f313_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "var-name-dashes", NC_DOUBLE, RANK_var_MINUS_name_MINUS_dashes, 0, &var_MINUS_name_MINUS_dashes_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "var.name.dots", NC_DOUBLE, RANK_var_PERIOD_name_PERIOD_dots, 0, &var_PERIOD_name_PERIOD_dots_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "var+name+plusses", NC_DOUBLE, RANK_var_PLUS_name_PLUS_plusses, 0, &var_PLUS_name_PLUS_plusses_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "var@name@ats", NC_DOUBLE, RANK_var_ATSIGN_name_ATSIGN_ats, 0, &var_ATSIGN_name_ATSIGN_ats_id);
   check_err(stat,__LINE__,__FILE__);

   /* assign attributes */
   c_att_MINUS_name_MINUS_dashes[0] = 4;
   stat = nc_put_att_int(ncid, c_id, "att-name-dashes", NC_INT, 1, c_att_MINUS_name_MINUS_dashes);
   check_err(stat,__LINE__,__FILE__);
   c_att_PERIOD_name_PERIOD_dots[0] = 5;
   stat = nc_put_att_int(ncid, c_id, "att.name.dots", NC_INT, 1, c_att_PERIOD_name_PERIOD_dots);
   check_err(stat,__LINE__,__FILE__);
   c_att_PLUS_name_PLUS_plusses[0] = 6;
   stat = nc_put_att_int(ncid, c_id, "att+name+plusses", NC_INT, 1, c_att_PLUS_name_PLUS_plusses);
   check_err(stat,__LINE__,__FILE__);
   c_att_ATSIGN_name_ATSIGN_ats[0] = 7;
   stat = nc_put_att_int(ncid, c_id, "att@name@ats", NC_INT, 1, c_att_ATSIGN_name_ATSIGN_ats);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, b_id, "c", 1, "");
   check_err(stat,__LINE__,__FILE__);
   s_b[0] = 0;
   s_b[1] = 127;
   s_b[2] = -128;
   s_b[3] = -1;
   stat = nc_put_att_int(ncid, s_id, "b", NC_BYTE, 4, s_b);
   check_err(stat,__LINE__,__FILE__);
   s_s[0] = -32768;
   s_s[1] = 0;
   s_s[2] = 32767;
   stat = nc_put_att_short(ncid, s_id, "s", NC_SHORT, 3, s_s);
   check_err(stat,__LINE__,__FILE__);
   i_i[0] = -2147483647;
   i_i[1] = 0;
   i_i[2] = 2147483647;
   stat = nc_put_att_int(ncid, i_id, "i", NC_INT, 3, i_i);
   check_err(stat,__LINE__,__FILE__);
   i_f[0] = -9.9999996e+35;
   i_f[1] = 0;
   i_f[2] = 9.9999996e+35;
   stat = nc_put_att_float(ncid, i_id, "f", NC_FLOAT, 3, i_f);
   check_err(stat,__LINE__,__FILE__);
   i_d[0] = -1e+308;
   i_d[1] = 0;
   i_d[2] = 1e+308;
   stat = nc_put_att_double(ncid, i_id, "d", NC_DOUBLE, 3, i_d);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, f_id, "c", 1, "x");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, d_id, "c", 8, "abcd\tZ$&");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "Gc", 1, "");
   check_err(stat,__LINE__,__FILE__);
   cdf_Gb[0] = -128;
   cdf_Gb[1] = 127;
   stat = nc_put_att_int(ncid, NC_GLOBAL, "Gb", NC_BYTE, 2, cdf_Gb);
   check_err(stat,__LINE__,__FILE__);
   cdf_Gs[0] = -32768;
   cdf_Gs[1] = 0;
   cdf_Gs[2] = 32767;
   stat = nc_put_att_short(ncid, NC_GLOBAL, "Gs", NC_SHORT, 3, cdf_Gs);
   check_err(stat,__LINE__,__FILE__);
   cdf_Gi[0] = -2147483647;
   cdf_Gi[1] = 0;
   cdf_Gi[2] = 2147483647;
   stat = nc_put_att_int(ncid, NC_GLOBAL, "Gi", NC_INT, 3, cdf_Gi);
   check_err(stat,__LINE__,__FILE__);
   cdf_Gf[0] = -9.9999996e+35;
   cdf_Gf[1] = 0;
   cdf_Gf[2] = 9.9999996e+35;
   stat = nc_put_att_float(ncid, NC_GLOBAL, "Gf", NC_FLOAT, 3, cdf_Gf);
   check_err(stat,__LINE__,__FILE__);
   cdf_Gd[0] = -1e+308;
   cdf_Gd[1] = 0;
   cdf_Gd[2] = 1e+308;
   stat = nc_put_att_double(ncid, NC_GLOBAL, "Gd", NC_DOUBLE, 3, cdf_Gd);
   check_err(stat,__LINE__,__FILE__);
   cdf_Gatt_MINUS_name_MINUS_dashes[0] = -1;
   stat = nc_put_att_int(ncid, NC_GLOBAL, "Gatt-name-dashes", NC_INT, 1, cdf_Gatt_MINUS_name_MINUS_dashes);
   check_err(stat,__LINE__,__FILE__);
   cdf_Gatt_PERIOD_name_PERIOD_dots[0] = -2;
   stat = nc_put_att_int(ncid, NC_GLOBAL, "Gatt.name.dots", NC_INT, 1, cdf_Gatt_PERIOD_name_PERIOD_dots);
   check_err(stat,__LINE__,__FILE__);
   cdf_Gatt_PLUS_name_PLUS_plusses[0] = -3;
   stat = nc_put_att_int(ncid, NC_GLOBAL, "Gatt+name+plusses", NC_INT, 1, cdf_Gatt_PLUS_name_PLUS_plusses);
   check_err(stat,__LINE__,__FILE__);
   cdf_Gatt_ATSIGN_name_ATSIGN_ats[0] = -4;
   stat = nc_put_att_int(ncid, NC_GLOBAL, "Gatt@name@ats", NC_INT, 1, cdf_Gatt_ATSIGN_name_ATSIGN_ats);
   check_err(stat,__LINE__,__FILE__);

   /* leave define mode */
   stat = nc_enddef (ncid);
   check_err(stat,__LINE__,__FILE__);

   {			/* store c */
    static char c = '2';
    stat = nc_put_var_text(ncid, c_id, &c);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store b */
    static signed char b = -2;
    stat = nc_put_var_schar(ncid, b_id, &b);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s */
    static short s = -5;
    stat = nc_put_var_short(ncid, s_id, &s);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store i */
    static int i = -20;
    stat = nc_put_var_int(ncid, i_id, &i);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f */
    static float f = -9;
    stat = nc_put_var_float(ncid, f_id, &f);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store d */
    static double d = -10.;
    stat = nc_put_var_double(ncid, d_id, &d);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store cr */
    static size_t cr_start[RANK_cr];
    static size_t cr_count[RANK_cr];
    static char cr[] = {"ab"};
    Dr_len = 2;			/* number of records of cr data */
    cr_start[0] = 0;
    cr_count[0] = Dr_len;
    stat = nc_put_vara_text(ncid, cr_id, cr_start, cr_count, cr);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store br */
    static size_t br_start[RANK_br];
    static size_t br_count[RANK_br];
    static signed char br[] = {-128, 127};
    Dr_len = 2;			/* number of records of br data */
    br_start[0] = 0;
    br_count[0] = Dr_len;
    stat = nc_put_vara_schar(ncid, br_id, br_start, br_count, br);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store sr */
    static size_t sr_start[RANK_sr];
    static size_t sr_count[RANK_sr];
    static short sr[] = {-32768, 32767};
    Dr_len = 2;			/* number of records of sr data */
    sr_start[0] = 0;
    sr_count[0] = Dr_len;
    stat = nc_put_vara_short(ncid, sr_id, sr_start, sr_count, sr);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store ir */
    static size_t ir_start[RANK_ir];
    static size_t ir_count[RANK_ir];
    static int ir[] = {-2147483646, 2147483647};
    Dr_len = 2;			/* number of records of ir data */
    ir_start[0] = 0;
    ir_count[0] = Dr_len;
    stat = nc_put_vara_int(ncid, ir_id, ir_start, ir_count, ir);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store fr */
    static size_t fr_start[RANK_fr];
    static size_t fr_count[RANK_fr];
    static float fr[] = {-9.9999996e+35, 9.9999996e+35};
    Dr_len = 2;			/* number of records of fr data */
    fr_start[0] = 0;
    fr_count[0] = Dr_len;
    stat = nc_put_vara_float(ncid, fr_id, fr_start, fr_count, fr);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store dr */
    static size_t dr_start[RANK_dr];
    static size_t dr_count[RANK_dr];
    static double dr[] = {-1.e+308, 1.e+308};
    Dr_len = 2;			/* number of records of dr data */
    dr_start[0] = 0;
    dr_count[0] = Dr_len;
    stat = nc_put_vara_double(ncid, dr_id, dr_start, dr_count, dr);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store c1 */
    static char c1[] = {""};
    stat = nc_put_var_text(ncid, c1_id, c1);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store b1 */
    static signed char b1[] = {-128};
    stat = nc_put_var_schar(ncid, b1_id, b1);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s1 */
    static short s1[] = {-32768};
    stat = nc_put_var_short(ncid, s1_id, s1);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store i1 */
    static int i1[] = {-2147483646};
    stat = nc_put_var_int(ncid, i1_id, i1);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f1 */
    static float f1[] = {-9.9999996e+35};
    stat = nc_put_var_float(ncid, f1_id, f1);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store d1 */
    static double d1[] = {-1.e+308};
    stat = nc_put_var_double(ncid, d1_id, d1);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store c2 */
    static char c2[] = {"ab"};
    stat = nc_put_var_text(ncid, c2_id, c2);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store b2 */
    static signed char b2[] = {-128, 127};
    stat = nc_put_var_schar(ncid, b2_id, b2);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s2 */
    static short s2[] = {-32768, 32767};
    stat = nc_put_var_short(ncid, s2_id, s2);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store i2 */
    static int i2[] = {-2147483646, 2147483647};
    stat = nc_put_var_int(ncid, i2_id, i2);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f2 */
    static float f2[] = {-9.9999996e+35, 9.9999996e+35};
    stat = nc_put_var_float(ncid, f2_id, f2);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store d2 */
    static double d2[] = {-1.e+308, 1.e+308};
    stat = nc_put_var_double(ncid, d2_id, d2);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store c3 */
    static char c3[] = {"\001\300."};
    stat = nc_put_var_text(ncid, c3_id, c3);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store b3 */
    static signed char b3[] = {-128, 127, -1};
    stat = nc_put_var_schar(ncid, b3_id, b3);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s3 */
    static short s3[] = {-32768, 0, 32767};
    stat = nc_put_var_short(ncid, s3_id, s3);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store i3 */
    static int i3[] = {-2147483646, 0, 2147483647};
    stat = nc_put_var_int(ncid, i3_id, i3);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f3 */
    static float f3[] = {-9.9999996e+35, 0, 9.9999996e+35};
    stat = nc_put_var_float(ncid, f3_id, f3);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store d3 */
    static double d3[] = {-1.e+308, 0., 1.e+308};
    stat = nc_put_var_double(ncid, d3_id, d3);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store cr1 */
    static size_t cr1_start[RANK_cr1];
    static size_t cr1_count[RANK_cr1];
    static char cr1[] = {"xy"};
    Dr_len = 2;			/* number of records of cr1 data */
    cr1_start[0] = 0;
    cr1_start[1] = 0;
    cr1_count[0] = Dr_len;
    cr1_count[1] = D1_len;
    stat = nc_put_vara_text(ncid, cr1_id, cr1_start, cr1_count, cr1);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store br2 */
    static size_t br2_start[RANK_br2];
    static size_t br2_count[RANK_br2];
    static signed char br2[] = {-24, -26, -20, -22};
    Dr_len = 2;			/* number of records of br2 data */
    br2_start[0] = 0;
    br2_start[1] = 0;
    br2_count[0] = Dr_len;
    br2_count[1] = D2_len;
    stat = nc_put_vara_schar(ncid, br2_id, br2_start, br2_count, br2);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store sr3 */
    static size_t sr3_start[RANK_sr3];
    static size_t sr3_count[RANK_sr3];
    static short sr3[] = {-375, -380, -385, -350, -355, -360};
    Dr_len = 2;			/* number of records of sr3 data */
    sr3_start[0] = 0;
    sr3_start[1] = 0;
    sr3_count[0] = Dr_len;
    sr3_count[1] = D3_len;
    stat = nc_put_vara_short(ncid, sr3_id, sr3_start, sr3_count, sr3);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f11 */
    static float f11[] = {-2187};
    stat = nc_put_var_float(ncid, f11_id, f11);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store d12 */
    static double d12[] = {-3000., -3010.};
    stat = nc_put_var_double(ncid, d12_id, d12);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store c13 */
    static char c13[] = {"\tb\177"};
    stat = nc_put_var_text(ncid, c13_id, c13);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s21 */
    static short s21[] = {-375, -350};
    stat = nc_put_var_short(ncid, s21_id, s21);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store i22 */
    static int i22[] = {-24000, -24020, -23600, -23620};
    stat = nc_put_var_int(ncid, i22_id, i22);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f23 */
    static float f23[] = {-2187, -2196, -2205, -2106, -2115, -2124};
    stat = nc_put_var_float(ncid, f23_id, f23);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store c31 */
    static char c31[] = {"+- "};
    stat = nc_put_var_text(ncid, c31_id, c31);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store b32 */
    static signed char b32[] = {-24, -26, -20, -22, -16, -18};
    stat = nc_put_var_schar(ncid, b32_id, b32);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s33 */
    static short s33[] = {-375, -380, -385, -350, -355, -360, -325, -330, -335};
    stat = nc_put_var_short(ncid, s33_id, s33);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store sr11 */
    static size_t sr11_start[RANK_sr11];
    static size_t sr11_count[RANK_sr11];
    static short sr11[] = {2500, 2375};
    Dr_len = 2;			/* number of records of sr11 data */
    sr11_start[0] = 0;
    sr11_start[1] = 0;
    sr11_start[2] = 0;
    sr11_count[0] = Dr_len;
    sr11_count[1] = D1_len;
    sr11_count[2] = D1_len;
    stat = nc_put_vara_short(ncid, sr11_id, sr11_start, sr11_count, sr11);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store ir12 */
    static size_t ir12_start[RANK_ir12];
    static size_t ir12_count[RANK_ir12];
    static int ir12[] = {640000, 639980, 632000, 631980};
    Dr_len = 2;			/* number of records of ir12 data */
    ir12_start[0] = 0;
    ir12_start[1] = 0;
    ir12_start[2] = 0;
    ir12_count[0] = Dr_len;
    ir12_count[1] = D1_len;
    ir12_count[2] = D2_len;
    stat = nc_put_vara_int(ncid, ir12_id, ir12_start, ir12_count, ir12);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store fr13 */
    static size_t fr13_start[RANK_fr13];
    static size_t fr13_count[RANK_fr13];
    static float fr13[] = {26244, 26235, 26226, 25515, 25506, 25497};
    Dr_len = 2;			/* number of records of fr13 data */
    fr13_start[0] = 0;
    fr13_start[1] = 0;
    fr13_start[2] = 0;
    fr13_count[0] = Dr_len;
    fr13_count[1] = D1_len;
    fr13_count[2] = D3_len;
    stat = nc_put_vara_float(ncid, fr13_id, fr13_start, fr13_count, fr13);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store cr21 */
    static size_t cr21_start[RANK_cr21];
    static size_t cr21_count[RANK_cr21];
    static char cr21[] = {"@DHL"};
    Dr_len = 2;			/* number of records of cr21 data */
    cr21_start[0] = 0;
    cr21_start[1] = 0;
    cr21_start[2] = 0;
    cr21_count[0] = Dr_len;
    cr21_count[1] = D2_len;
    cr21_count[2] = D1_len;
    stat = nc_put_vara_text(ncid, cr21_id, cr21_start, cr21_count, cr21);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store br22 */
    static size_t br22_start[RANK_br22];
    static size_t br22_count[RANK_br22];
    static signed char br22[] = {64, 62, 68, 66, 56, 54, 60, 58};
    Dr_len = 2;			/* number of records of br22 data */
    br22_start[0] = 0;
    br22_start[1] = 0;
    br22_start[2] = 0;
    br22_count[0] = Dr_len;
    br22_count[1] = D2_len;
    br22_count[2] = D2_len;
    stat = nc_put_vara_schar(ncid, br22_id, br22_start, br22_count, br22);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store sr23 */
    static size_t sr23_start[RANK_sr23];
    static size_t sr23_count[RANK_sr23];
    static short sr23[] = {2500, 2495, 2490, 2525, 2520, 2515, 2375, 2370, 2365, 2400, 2395, 2390};
    Dr_len = 2;			/* number of records of sr23 data */
    sr23_start[0] = 0;
    sr23_start[1] = 0;
    sr23_start[2] = 0;
    sr23_count[0] = Dr_len;
    sr23_count[1] = D2_len;
    sr23_count[2] = D3_len;
    stat = nc_put_vara_short(ncid, sr23_id, sr23_start, sr23_count, sr23);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store fr31 */
    static size_t fr31_start[RANK_fr31];
    static size_t fr31_count[RANK_fr31];
    static float fr31[] = {26244, 26325, 26406, 25515, 25596, 25677};
    Dr_len = 2;			/* number of records of fr31 data */
    fr31_start[0] = 0;
    fr31_start[1] = 0;
    fr31_start[2] = 0;
    fr31_count[0] = Dr_len;
    fr31_count[1] = D3_len;
    fr31_count[2] = D1_len;
    stat = nc_put_vara_float(ncid, fr31_id, fr31_start, fr31_count, fr31);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store dr32 */
    static size_t dr32_start[RANK_dr32];
    static size_t dr32_count[RANK_dr32];
    static double dr32[] = {40000., 39990., 40100., 40090., 40200., 40190., 39000., 38990., 39100., 39090., 39200., 39190.};
    Dr_len = 2;			/* number of records of dr32 data */
    dr32_start[0] = 0;
    dr32_start[1] = 0;
    dr32_start[2] = 0;
    dr32_count[0] = Dr_len;
    dr32_count[1] = D3_len;
    dr32_count[2] = D2_len;
    stat = nc_put_vara_double(ncid, dr32_id, dr32_start, dr32_count, dr32);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store cr33 */
    static size_t cr33_start[RANK_cr33];
    static size_t cr33_count[RANK_cr33];
    static char cr33[] = {"1\000\000two3\000\0004\000\0005\000\000six"};
    Dr_len = 2;			/* number of records of cr33 data */
    cr33_start[0] = 0;
    cr33_start[1] = 0;
    cr33_start[2] = 0;
    cr33_count[0] = Dr_len;
    cr33_count[1] = D3_len;
    cr33_count[2] = D3_len;
    stat = nc_put_vara_text(ncid, cr33_id, cr33_start, cr33_count, cr33);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store c111 */
    static char c111[] = {"@"};
    stat = nc_put_var_text(ncid, c111_id, c111);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store b112 */
    static signed char b112[] = {64, 62};
    stat = nc_put_var_schar(ncid, b112_id, b112);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s113 */
    static short s113[] = {2500, 2495, 2490};
    stat = nc_put_var_short(ncid, s113_id, s113);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f121 */
    static float f121[] = {26244, 26325};
    stat = nc_put_var_float(ncid, f121_id, f121);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store d122 */
    static double d122[] = {40000., 39990., 40100., 40090.};
    stat = nc_put_var_double(ncid, d122_id, d122);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store c123 */
    static char c123[] = {"one2\000\000"};
    stat = nc_put_var_text(ncid, c123_id, c123);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s131 */
    static short s131[] = {2500, 2525, 2550};
    stat = nc_put_var_short(ncid, s131_id, s131);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store i132 */
    static int i132[] = {640000, 639980, 640400, 640380, 640800, 640780};
    stat = nc_put_var_int(ncid, i132_id, i132);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f133 */
    static float f133[] = {26244, 26235, 26226, 26325, 26316, 26307, 26406, 26397, 26388};
    stat = nc_put_var_float(ncid, f133_id, f133);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f211 */
    static float f211[] = {26244, 25515};
    stat = nc_put_var_float(ncid, f211_id, f211);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store d212 */
    static double d212[] = {40000., 39990., 39000., 38990.};
    stat = nc_put_var_double(ncid, d212_id, d212);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s221 */
    static short s221[] = {2500, 2525, 2375, 2400};
    stat = nc_put_var_short(ncid, s221_id, s221);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store i222 */
    static int i222[] = {640000, 639980, 640400, 640380, 632000, 631980, 632400, 632380};
    stat = nc_put_var_int(ncid, i222_id, i222);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f223 */
    static float f223[] = {26244, 26235, 26226, 26325, 26316, 26307, 25515, 25506, 25497, 25596, 25587, 25578};
    stat = nc_put_var_float(ncid, f223_id, f223);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store c231 */
    static char c231[] = {"@DHHLP"};
    stat = nc_put_var_text(ncid, c231_id, c231);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store b232 */
    static signed char b232[] = {64, 62, 68, 66, 72, 70, 56, 54, 60, 58, 64, 62};
    stat = nc_put_var_schar(ncid, b232_id, b232);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s233 */
    static short s233[] = {2500, 2495, 2490, 2525, 2520, 2515, 2550, 2545, 2540, 2375, 2370, 2365, 2400, 2395, 2390, 2425, 2420, 2415};
    stat = nc_put_var_short(ncid, s233_id, s233);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store s311 */
    static short s311[] = {2500, 2375, 2250};
    stat = nc_put_var_short(ncid, s311_id, s311);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store i312 */
    static int i312[] = {640000, 639980, 632000, 631980, 624000, 623980};
    stat = nc_put_var_int(ncid, i312_id, i312);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store f313 */
    static float f313[] = {26244, 26235, 26226, 25515, 25506, 25497, 24786, 24777, 24768};
    stat = nc_put_var_float(ncid, f313_id, f313);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store var-name-dashes */
    static double var_MINUS_name_MINUS_dashes = -1.;
    stat = nc_put_var_double(ncid, var_MINUS_name_MINUS_dashes_id, &var_MINUS_name_MINUS_dashes);
    check_err(stat,__LINE__,__FILE__);
   }

   {			/* store var.name.dots */
    static double var_PERIOD_name_PERIOD_dots = -2.;
    stat = nc_put_var_double(ncid, var_PERIOD_name_PERIOD_dots_id, &var_PERIOD_name_PERIOD_dots);
    check_err(stat,__LINE__,__FILE__);
   }
   stat = nc_close(ncid);
   check_err(stat,__LINE__,__FILE__);
   return 0;
}
