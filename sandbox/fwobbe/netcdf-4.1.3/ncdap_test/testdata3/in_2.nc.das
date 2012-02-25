Attributes {
    lat {
    }
    lat_var {
    }
    lat_wgt {
    }
    lev {
        String purpose "Monotonically increasing coordinate pressure";
        String units "hybrid_sigma_pressure";
        String positive "down";
        String A_var "hyam";
        String B_var "hybm";
        String P0_var "P0";
        String PS_var "PS";
        String bounds "ilev";
    }
    lon_T42 {
    }
    lat_T42 {
    }
    rlev {
        String purpose "Monotonically decreasing coordinate pressure";
    }
    lon {
    }
    time {
    }
    lsmlev {
        String purpose "Homebrew level coordinate for LSM";
        String long_name "Soil depth";
        String units "meter";
    }
    wvl {
        String long_name "Wavelength";
        String units "meter";
    }
    hyam {
        String long_name "hybrid A coefficient at layer midpoints";
    }
    hybm {
        String long_name "hybrid B coefficient at layer midpoints";
    }
    P0 {
        String long_name "reference pressure";
        String units "pascal";
    }
    PS {
        String long_name "surface pressure";
        String units "pascal";
    }
    fl_dmn {
        String long_name "Character coordinate";
        String units "[chr]";
    }
    nbdate {
        String long_name "base date as 6 or 8 digit integer (YYMMDD or YYYYMMDD)";
    }
    date {
        String long_name "current date as 6 or 8 digit integer (YYMMDD or YYYYMMDD)";
    }
    lon_wgt {
        String long_name "Gaussian weights";
        String purpose "Gaussian weights which sum to two for n = 4. These weights are all have floor of 0.0 so should cause SIGFPE when applied to integer types in weighted average.";
    }
    msk_prt_mss_prt {
        String long_name "partial mask, partial missing value example";
        Float32 missing_value 9.999999617e+35;
    }
    mss_val {
        String long_name "partial missing value example";
        Float32 missing_value 9.999999617e+35;
    }
    mss_val_scl {
        String long_name "scalar missing value";
        Float32 missing_value 9.999999617e+35;
    }
    mss_val_fst {
        String long_name "offset partial missing value example";
        Float32 missing_value -999.0000000;
    }
    fll_val {
        String long_name "_FillValue example";
        Float32 _FillValue -999.0000000;
    }
    no_mss_val {
        String long_name "no missing value";
    }
    val_one_mss {
        String long_name "one regular value, one missing value";
        Float32 missing_value 9.999999617e+35;
    }
    pck {
        String long_name "Packed scalar variable";
        String note "Original packed value was 1s with scale_factor = 2.0d and add_offset = 1.0d. Unpacked value should be 3.0 = 2.0d*1s + 1.0d. NCO algorithms would pack this variable as scale_factor = 0.0d and add_offset = 3.0d.";
        Float64 scale_factor 2.0000000000000000;
        Float64 add_offset 1.0000000000000000;
    }
    pck_arr {
        String long_name "Packed array";
        String note "Packed value is -32767s, 0s, 1s, 32767s, unpacked is same in double";
        Float64 scale_factor 1.0000000000000000;
        Float64 add_offset 0.0000000000000000;
    }
    upk {
        String long_name "Unpacked scalar variable";
        String note "Unpacked value is 3.0d0, upk=unpack(pck)= 2.0d0*1s + 1.0d0 = 3.0d0. Packing this variable should create an NC_SHORT scalar = 0s with packing attribute add_offset=3.0d and either no scale_factor (ncap) or scale_factor = 0.0d (ncpdq).";
    }
    upk_arr {
        String long_name "Unpacked array";
        String note "Unpacked value is -32767.d, 0.d, 1.d, 32767.d, packed is same in short. Packing algorithm should yield an NC_SHORT array = [] with packing attributes scale_factor=1.0d, add_offset=0.0d";
    }
    val_one_int {
        String long_name "scalar equal to 1";
        Int32 missing_value -99;
    }
    val_one_one_int {
        String long_name "1, 1";
        Int32 missing_value -99;
    }
    val_max_max_sht {
        String long_name "17000, 17000";
        Int16 missing_value -99;
    }
    val_one_mss_int {
        String long_name "1, mss_val";
        Int32 missing_value -99;
    }
    val_half {
        String long_name "Scalar with value 0.5";
        Float32 missing_value 9.999999617e+35;
    }
    val_half_half {
        String long_name "0.5,0.5";
        Float32 missing_value 9.999999617e+35;
    }
    wgt_one {
        String long_name "all values are one";
    }
    mss_val_all {
        String long_name "all missing values example";
        Float32 missing_value 9.999999617e+35;
    }
    scalar_var {
        String long_name "scalar variable";
        String units "Fraction";
    }
    float_var {
        String long_name "float";
    }
    double_var {
        String long_name "double";
    }
    double_var2 {
        String long_name "double";
        Float64 missing_value 1.0000000000000000e+36;
    }
    int_var {
        String long_name "int";
    }
    short_var {
        String long_name "short";
    }
    char_var {
        String long_name "char";
    }
    char_var_space {
        String long_name "Character variable with whitespace on ends";
    }
    char_var_nul {
        String long_name "Character variable containing one NUL";
    }
    char_var_multinul {
        String long_name "Character variable containing multiple NULs";
    }
    fl_nm {
        String long_name "Variable contains a file name";
    }
    fl_nm_arr {
        String long_name "Variable that contains a short array of file names";
        String units "[sng]";
    }
    non_nul_trm_char_one_dmn {
        String long_name "Variable contains a one-dimensional array of characters that is not NUL-terminated";
        String units "[chr]";
    }
    non_nul_trm_char_two_dmn {
        String long_name "Variable contains a two-dimensional array of characters that are not NUL-terminated";
        String units "[chr]";
    }
    byte_var {
        String long_name "byte";
    }
    zero {
        String long_name "zero";
    }
    one {
        String long_name "one";
    }
    two {
        String long_name "two";
    }
    e_dbl {
        String long_name "e, base of the natural logarithm";
    }
    e_flt {
        String long_name "e, base of the natural logarithm";
    }
    three {
        String long_name "three";
    }
    four {
        String long_name "four";
    }
    negative_one {
        String long_name "negative one";
    }
    lev_var {
        String long_name "lev_var";
    }
    lev_wgt {
        String long_name "lev_wgt";
    }
    g {
        String long_name "g";
    }
    z {
        String long_name "Height";
        String units "meter";
        String purpose "Height stored with a monotonically increasing coordinate";
    }
    rz {
        String long_name "Height";
        String units "meter";
        String purpose "Height stored with a monotonically decreasing coordinate";
    }
    one_dmn_var {
    }
    att_var {
        Byte byte_att 0;
        String char_att "Sentence one.\\012Sentence two.\\012";
        Int16 short_att 37;
        Int32 int_att 73;
        Float32 float_att 73.00000000, 72.00000000, 71.00000000;
        Float64 double_att 73.000000000000000;
    }
    bnd_var {
        Byte byte_att 0;
        String char_att "Sentence one.\\012Sentence two.\\012";
        Int16 short_att 37;
        Int32 int_att 73;
        Float32 float_att 73.00000000;
        Float64 double_att 73.000000000000000;
    }
    three_dmn_var {
        String long_name "three dimensional variable with CCM coordinate convention C=[lat,lev,lon], Fortran=(lon,lev,lat)";
        String units "Fraction";
    }
    three_dmn_var_crd {
        String long_name "three dimensional variable with COORDS coordinate convention C=[lev,lat,lon], Fortran=(lon,lat,lev)";
        String units "Fraction";
    }
    prs_sfc {
        String long_name "Surface pressure";
        String units "pascal";
    }
    two_dmn_var {
        String long_name "two dimensional variable";
        String units "Fraction";
    }
    mask {
        String long_name "mask variable like ORO";
        String units "Fraction";
    }
    ORO {
        String long_name "Orography";
        String units "Fraction";
    }
    weight {
        String long_name "Gaussian weight";
        String units "Fraction";
    }
    gw {
        String long_name "gw variable like gw";
        String units "Fraction";
    }
    gw_T42 {
        String long_name "gw variable like gw_T42";
        String units "Fraction";
    }
    rec_var_flt {
        String long_name "record variable, float";
    }
    rec_var_dbl {
        String long_name "record variable, double";
    }
    one_dmn_rec_var {
        String long_name "one dimensional record variable";
        String units "second";
    }
    tpt {
        String long_name "Temperature";
        String units "kelvin";
        String hieght "Leave mispelled for NCO User's guide example";
    }
    rec_var_dbl_mss_val_dbl_upk {
        String long_name "record variable, double, with double missing values";
        String purpose "This variable is used to generate the packed variable rec_var_dbl_mss_val_dbl_pck, so its missing_value should not be too far out of range, i.e., it should be representable by a short. However, the missing_value should itself be the same type as the unpacked variable, NC_DOUBLE in this case.";
        Float64 missing_value -999.00000000000000;
    }
    rec_var_dbl_mss_val_dbl_pck {
        String long_name "record variable, double, with double missing values, packed";
        String purpose "Packed version of rec_var_dbl_mss_val_dbl_upk";
        Float64 missing_value -999.00000000000000;
        Float64 scale_factor -9.1554131380178503e-05;
        Float64 add_offset 5.0000000000000000;
    }
    rec_var_flt_mss_val_flt {
        String long_name "record variable, float, with float missing values";
        Float32 missing_value 9.999999617e+35;
    }
    rec_var_flt_pck {
        String long_name "record variable, float, packed";
        String purpose "Demonstrate that rounding of means of packed data are handled correctly";
        Float32 scale_factor 0.1000000015;
        Float32 add_offset 100.0000000;
    }
    rec_var_dbl_pck {
        String long_name "record variable, double, packed";
        String purpose "Demonstrate that rounding of means of packed data are handled correctly";
        Float64 scale_factor 0.10000000000000001;
        Float64 add_offset 100.00000000000000;
    }
    rec_var_flt_mss_val_dbl {
        String long_name "record variable, float, with double missing values";
        Float64 missing_value 1.0000000000000000e+36;
    }
    rec_var_flt_mss_val_int {
        String long_name "record variable, float, with integer missing values";
        Int32 missing_value -999;
    }
    rec_var_int_mss_val_int {
        String long_name "record variable, integer, with integer missing values";
        Int32 missing_value -999;
    }
    rec_var_int_mss_val_flt {
        String long_name "record variable, integer, with float missing values";
        Float32 missing_value -999.0000000;
    }
    rec_var_int_mss_val_dbl {
        String long_name "record variable, integer, with double missing values";
        Float64 missing_value -999.00000000000000;
    }
    rec_var_dbl_mss_val_dbl_pck_lng {
        String long_name "record variable, double packed as long, with double missing values";
        String purpose "although not usual, packing doubles into longs (rather than shorts) is still a considerable cost savings";
        Float64 missing_value -999.00000000000000;
        Float64 scale_factor -9.1554131380178503e-05;
        Float64 add_offset 5.0000000000000000;
    }
    one_dmn_rec_var_sng {
        String long_name "one dimensional record variable of string";
    }
    time_lon {
        String long_name "Record variable of longitude coordinate";
    }
    two_dmn_rec_var_sng {
        String long_name "two dimensional record variable of string";
    }
    two_dmn_rec_var {
        String long_name "two dimensional record variable";
        String units "watt meter-2";
    }
    three_dmn_rec_var {
        String long_name "three dimensional record variable";
        String units "watt meter-2";
    }
    three_dmn_var_dbl {
        String long_name "three dimensional record variable of type double";
        String units "watt meter-2";
        Float64 missing_value -99.000000000000000;
    }
    three_dmn_var_int {
        String long_name "three dimensional record variable of type int";
        String units "watt meter-2";
        Int32 missing_value -99;
    }
    three_dmn_var_sht {
        String long_name "three dimensional record variable";
        String units "watt meter-2";
        Int16 missing_value -99;
    }
    four_dmn_rec_var {
        String long_name "four dimensional record variable";
        String units "watt meter-2";
    }
    three_double_dmn {
    }
    time_udunits {
        String units "hours since 1900-01-01 00:00:0.0";
        String delta_t "0000-00-00 06:00:0.0";
    }
    u {
        String long_name "Zonal wind speed";
        String units "meter second-1";
    }
    v {
        String long_name "Meridional wind speed";
        String units "meter second-1";
    }
    NC_GLOBAL {
        String Conventions "NCAR-CSM";
        String history "History global attribute.\\012";
        Float64 julian_day 200000.04000000001;
    }
    DODS_EXTRA {
        String Unlimited_Dimension "time";
    }
}
