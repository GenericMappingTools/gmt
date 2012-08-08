# This is part of the netCDF package.
# Copyright 2005 University Corporation for Atmospheric Research/Unidata
# See COPYRIGHT file for conditions of use.
# 
# This entire directory will be skipped if netCDF-4 is not enabled.
#

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Link to our assembled library.
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la

# These are netCDF-4 test programs.
NC4_TESTS = tst_dims tst_dims2 tst_dims3 tst_files tst_files4 tst_vars	\
tst_varms tst_unlim_vars tst_converts tst_converts2 tst_grps tst_grps2	\
tst_compounds tst_compounds2 tst_compounds3 tst_opaques tst_strings	\
tst_strings2 tst_interops tst_interops4 tst_interops5 tst_interops6	\
tst_enums tst_coords tst_coords2 tst_coords3 tst_vars3 tst_vars4	\
tst_chunks tst_chunks2 tst_utf8 tst_fills tst_fills2 tst_fillbug	\
tst_xplatform tst_xplatform2 tst_h_atts2 tst_endian_fill tst_atts	\
t_type cdm_sea_soundings tst_camrun tst_vl tst_atts1 tst_atts2		\
tst_vars2 tst_files5 tst_files6 tst_sync

check_PROGRAMS = $(NC4_TESTS)

# Add these if large file tests are turned on.
if LARGE_FILE_TESTS
check_PROGRAMS += tst_large tst_large2
endif

TESTS = $(NC4_TESTS)

# If the v2 API was built, add its test program.
if BUILD_V2
check_PROGRAMS += tst_v2 
TESTS += tst_v2
endif # BUILD_V2

if LARGE_FILE_TESTS
check_PROGRAMS += tst_large
TESTS += tst_large
endif # LARGE_FILE_TESTS

# If benchmarks were turned on, build and run a bunch more tests.
if BUILD_BENCHMARKS
check_PROGRAMS += tst_create_files bm_file tst_chunks3 tst_ar4	\
tst_ar4_3d tst_ar4_4d bm_many_objs tst_h_many_atts bm_many_atts	\
tst_files2 tst_files3 tst_ar5 tst_h_files3 tst_mem

TESTS += tst_ar4_3d tst_create_files run_bm_test1.sh run_bm_elena.sh	\
run_bm_test2.sh run_tst_chunks.sh run_bm_ar4.sh tst_files2 tst_files3	\
tst_ar5 tst_h_files3 tst_mem

# This will run a parallel I/O benchmark for parallel builds.
if TEST_PARALLEL
TESTS += run_par_bm_test.sh
endif # TEST_PARALLEL

benchmarks: check
	./run_bm_radar_2D.sh	
	./run_bm_radar_2D_compression1.sh	
	./run_bm.sh
	./run_tst_chunks.sh
	./run_bm_ar4.sh
endif # BUILD_BENCHMARKS

# These are the tests for HDF4.
if USE_HDF4
check_PROGRAMS += tst_interops2 
TESTS += tst_interops2
if USE_HDF4_FILE_TESTS
check_PROGRAMS += tst_interops3
TESTS += run_get_hdf4_files.sh tst_interops3
endif # USE_HDF4_FILE_TESTS
#tst_interops2_LDADD = ${lib_LTLIBRARIES} -lmfhdf -ldf -ljpeg -lhdf5_hl	\
#-lhdf5 -lz
endif # USE_HDF4

# This will run a bunch of the test programs with valgrind, the memory
# checking tool. (Valgrind must be present for this to work.)
if USE_VALGRIND_TESTS
TESTS += run_valgrind_tests.sh run_valgrind_tests2.sh
if USE_HDF4
TESTS += run_hdf4_valgrind_tests.sh
endif # USE_HDF4
endif # USE_VALGRIND_TESTS

# This are extra tests that will only be run if netcdf-4 is configured
# with --enable-parallel-tests.
if TEST_PARALLEL
check_PROGRAMS += tst_mpi_parallel tst_parallel tst_parallel3	\
tst_parallel4 tst_nc4perf
TESTS += run_par_test.sh
if USE_PNETCDF
check_PROGRAMS += tst_parallel2
TESTS += run_pnetcdf_test.sh
endif
endif

EXTRA_DIST = run_par_test.sh run_bm.sh run_bm_test1.sh run_bm_test2.sh	\
run_bm_radar_2D.sh run_bm_radar_2D_compression1.sh run_par_bm_test.sh	\
run_bm_elena.sh run_par_bm_radar_2D.sh run_bm_radar_2D_endianness1.sh	\
run_tst_chunks.sh ref_chunks1.cdl ref_chunks2.cdl run_pnetcdf_test.sh	\
run_get_hdf4_files.sh run_valgrind_tests.sh run_valgrind_tests2.sh	\
run_bm_ar4.sh ref_tst_compounds.nc run_hdf4_valgrind_tests.sh		\
ref_tst_xplatform2_1.nc ref_tst_xplatform2_2.nc ref_tst_dims.nc		\
ref_tst_interops4.nc

CLEANFILES = tst_mpi_parallel.bin cdm_sea_soundings.nc bm_chunking.nc	\
bm_radar.nc bm_radar1.nc radar_3d_compression_test.txt			\
radar_3d_compression.txt radar_2d_compression.txt			\
radar_3d_chunking.txt tst_floats_1D.cdl floats_1D_3.nc floats_1D.cdl	\
tst_*.nc tst_floats2_*.cdl tst_ints2_*.cdl tst_shorts2_*.cdl		\
tst_elena_*.cdl tst_simple*.cdl tst_chunks.cdl pr_A1.* tauu_A1.*	\
usi_01.* thetau_01.* tst_*.nc tst_*.h5

if USE_HDF4_FILE_TESTS
DISTCLEANFILES = AMSR_E_L2_Rain_V10_200905312326_A.hdf	\
AMSR_E_L3_DailyLand_V06_20020619.hdf			\
MYD29.A2009152.0000.005.2009153124331.hdf		\
MYD29.A2002185.0000.005.2007160150627.hdf		\
MOD29.A2000055.0005.005.2006267200024.hdf
endif # HDF4_FILE_TESTS






