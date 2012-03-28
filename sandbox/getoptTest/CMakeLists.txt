cmake_minimum_required(VERSION 2.8.5)
project (getoptTest)

# set if you like to re-generate the gengetopt parsers
# note: you need a recent gengetopt that supports non-alphanumeric optargs -^ and -/
# patchfile against git branch 95495b9 (git://git.savannah.gnu.org/gengetopt.git):
# patch-gengetopt-git.diff
#set (RE_GENGETOPT TRUE)


# grdimage cmdline example
add_executable (gmt_grdimage gmt_cmdline_grdimage.h gmt_cmdline_grdimage.c gmt_grdimage.c)

# project cmdline example
add_executable (gmt_project gmt_cmdline_project.h gmt_cmdline_project.c gmt_project.c)

# test examples
configure_file (test.sh.in test.sh)

# need a *patched* gengetopt to support non-alphanumeric optargs -^ and -/
find_program (GENGETOPT gengetopt)

if (RE_GENGETOPT AND GENGETOPT)
  # generate gmt_cmdline_project.c and gmt_cmdline_project.h
  add_custom_command(OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gmt_cmdline_project.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gmt_cmdline_project.h
    COMMAND ${GENGETOPT} --no-version --no-help -l
    --input=gmt_project.ggo
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS gmt_project.ggo
    VERBATIM)

  # generate gmt_cmdline_grdimage.c and gmt_cmdline_grdimage.h
  add_custom_command(OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gmt_cmdline_grdimage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gmt_cmdline_grdimage.h
    COMMAND ${GENGETOPT} --no-version --no-help -l
    #--unamed-opts=grd_z\\|grd_r\ grd_g\ grd_b
    --unamed-opts=grd_z\ or\ grd_r\ grd_g\ grd_b
    --input=gmt_grdimage.ggo
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS gmt_grdimage.ggo
    VERBATIM)
endif (RE_GENGETOPT AND GENGETOPT)
