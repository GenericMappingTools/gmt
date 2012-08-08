SUBDIRS = shared make_hgrid make_vgrid make_solo_mosaic	\
transfer_to_mosaic_grid fregrid river_regrid make_topog	\
make_coupler_mosaic

# These tests must be turned on with --enable-gridspec-file-tests.
if BUILD_GRIDSPEC_FILE_TESTS
TESTS = get_data.sh test.sh test_all.sh
endif

EXTRA_DIST = test.sh test_all.sh get_data.sh
CLEANFILES = *.nc


