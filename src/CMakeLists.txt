#
# $Id$
#
# src CMakeLists.txt
#

find_package( NETCDF REQUIRED )
include_directories( ${NETCDF_INCLUDE_DIR} )
add_definitions(${NETCDF_DEFINITIONS})

find_package( GDAL )
if(GDAL_FOUND)
  set(HAVE_GDAL 1 CACHE INTERNAL "System has GDAL")
  include_directories( ${GDAL_INCLUDE_DIR} )
  #add_definitions(${GDAL_DEFINITIONS} -DUSE_GDAL)
  list(APPEND GMT_OPTIONAL_LIBRARIES ${GDAL_LIBRARIES})
endif(GDAL_FOUND)

find_package( PCRE )
if(PCRE_FOUND)
  set(HAVE_PCRE 1 CACHE INTERNAL "System has PCRE")
  include_directories( ${PCRE_INCLUDE_DIR} )
  #add_definitions(${PCRE_DEFINITIONS} -DHAVE_PCRE)
  list(APPEND GMT_OPTIONAL_LIBRARIES ${PCRE_LIBRARIES})
else(PCRE_FOUND)
  find_package( REGEX )
  if(REGEX_FOUND)
    set(HAVE_POSIX_ERE 1 CACHE INTERNAL "System has regex.h")
    include_directories( ${REGEX_INCLUDE_DIR} )
    #add_definitions(-DHAVE_POSIX_ERE)
    list(APPEND GMT_OPTIONAL_LIBRARIES ${REGEX_LIBRARIES})
  endif(REGEX_FOUND)
endif(PCRE_FOUND)

# Commented OpenMP out because things do not link with it activated
#find_package( OpenMP )
#if(OPENMP_FOUND)
#    list(APPEND CMAKE_C_FLAGS ${OpenMP_C_FLAGS})
#endif(OPENMP_FOUND)

find_package( Threads )

include(ConfigureChecks)

if(APPLE)
  # Check for dsymutil
  find_program(DSYMUTIL dsymutil)
endif(APPLE)

include_directories( ${GMT_SOURCE_DIR}/src )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
#include_directories( ${PROJECT_BINARY_DIR} )


# These three lines are temporarily here for the beta release
set(GMT_PROGS2_SRCS gmtaverage.c)
set(GMT_PROGSPS2_SRCS testapi.c)
set(GMT_DEMOS2_SRCS testgmt5.c testgmtio.c testio.c)

# source files
set(GMT_DEMOS_SRCS gmt.c ${GMT_DEMOS2_SRCS})

if(TRIANGLE_D)
  set(GMT_TRIANGLE_SRCS triangle.c triangle.h)
endif(TRIANGLE_D)

set(GMT_PROGS_SRCS blockmean.c blockmedian.c blockmode.c
colmath.c filter1d.c fitcircle.c gmt2kml.c
gmtconvert.c gmtdp.c gmtmath.c gmtselect.c
gmtspatial.c gmtstitch.c gmtvector.c gmtwhich.c
grd2cpt.c grd2rgb.c grd2xyz.c grdblend.c
grdclip.c grdcut.c grdedit.c grdfft.c
grdfilter.c grdgradient.c grdhisteq.c grdinfo.c
grdlandmask.c grdmask.c grdmath.c grdpaste.c
grdproject.c grdsample.c grdreformat.c grdtrack.c
grdtrend.c grdvolume.c greenspline.c kml2gmt.c
makecpt.c mapproject.c minmax.c nearneighbor.c
project.c ps2raster.c sample1d.c spectrum1d.c
splitxyz.c surface.c trend1d.c trend2d.c
triangulate.c xyz2grd.c)
list(APPEND GMT_PROGS_SRCS ${GMT_PROGS2_SRCS})

set(GMT_PROGSPS_SRCS gmtdefaults.c gmtget.c gmtset.c
grdcontour.c grdimage.c grdvector.c grdview.c
psbasemap.c psclip.c pscoast.c pscontour.c
pshistogram.c psimage.c psmask.c psrose.c
psscale.c pstext.c pswiggle.c psxy.c psxyz.c
pslegend.c)
list(APPEND GMT_PROGSPS_SRCS ${GMT_PROGSPS2_SRCS})

# libpslib
set(GMT_PSL_SRCS pslib.c gmt_notposix.h)

# libgmt
set(GMT_LIB_SRCS gmt_bcr.c gmt_calclock.c gmt_cdf.c gmt_customio.c
gmt_fft.c gmt_grdio.c gmt_io.c  gmt_nc.c gmt_notposix.c gmt_proj.c
gmt_shore.c gmt_stat.c gmt_support.c gmt_regexp.c gmt_vector.c
gmt_version.c gmtapi_parse.c ${GMT_PROGS_SRCS} ${GMT_TRIANGLE_SRCS})

set(GMT_LIB_PS_SRCS gmt_init.c gmt_map.c gmt_plot.c gmtapi_util.c ${GMT_PROGSPS_SRCS})

set(GMT_LIB_HDRS gmt_progpurpose.h gmt_config.h)

# lib targets
set(GMT_LIBS gmtlib pslib)

# manpages
string(REPLACE ".c" ".1" GMT_MAN_1 "${GMT_PROGS_SRCS};${GMT_PROGSPS_SRCS}")
list(APPEND GMT_MAN_1
  gmt.1 gmt_shell_functions.1 gmtlogo.1 isogmt.1 gmtswitch.1)
set(GMT_MAN_3 pslib.3)
set(GMT_MAN_5 gmtcolors.5 gmt.conf.5)
set(GMT_MAN_DEPS common_options.txt
  explain_-A.txt explain_-B.txt explain_-B_full.txt explain_-I.txt
  explain_-J.txt explain_-J_full.txt explain_-Jz.txt explain_-Jz_full.txt
  explain_-K.txt explain_-K_full.txt explain_-L_scale.txt explain_-O.txt
  explain_-O_full.txt explain_-P.txt explain_-P_full.txt explain_-R.txt
  explain_-R_full.txt explain_-Rgeo.txt explain_-Rz.txt explain_-Rz_full.txt
  explain_-T_rose.txt explain_-U.txt explain_-U_full.txt explain_-V.txt
  explain_-V_full.txt explain_-XY.txt explain_-XY_full.txt
  explain_-aspatial.txt explain_-aspatial_full.txt explain_-bi.txt
  explain_-bi_full.txt explain_-bo.txt explain_-bo_full.txt explain_-c.txt
  explain_-c_full.txt explain_-f.txt explain_-f_full.txt explain_-g.txt
  explain_-g_full.txt explain_-h.txt explain_-h_full.txt explain_-icols.txt
  explain_-icols_full.txt explain_-n.txt explain_-n_full.txt
  explain_-ocols.txt explain_-ocols_full.txt explain_-s.txt
  explain_-s_full.txt explain_-t.txt explain_-t_full.txt explain_colon.txt
  explain_color.txt explain_commonitems.txt explain_contdump.txt
  explain_contlabel.txt explain_distunits.txt explain_fill.txt
  explain_float.txt explain_font.txt explain_grd_coord.txt
  explain_grd_inout.txt explain_grd_input.txt explain_grd_output.txt
  explain_gshhs.txt explain_help.txt explain_intables.txt
  explain_labelinfo.txt explain_nodereg.txt explain_nodereg_full.txt
  explain_pen.txt explain_perspective.txt explain_perspective_full.txt
  explain_precision.txt explain_symbols.txt refs.i)


## generate header files (this must be rewritten with cmake macros to be platform independent)

set(GMT_EXTRA_HDRS
  gmt_progcases.h gmt_prognames.h gmt_colornames.h
  gmt_datums.h gmt_grdkeys.h gmt_keywords.h
  gmt_dimensions.h gmtapi_errno.h gmt_color_rgb.h
  gmtmath_explain.h gmtmath.h gmtmath_man.i
  grdmath_explain.h grdmath.h grdmath_man.i
  gmt_keycases.h gmtapi_errstr.h
  Colors.i Ellipsoids.i Fonts.i grdreformat_man.i)

# this is terrible:
foreach(_filename ${GMT_PROGS_SRCS} ${GMT_PROGSPS_SRCS})
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}")
    file(READ ${_filename} file_CONTENT)
    string(REGEX MATCH "GMT_message[^%]+%s \\[API\\][^;]+" prog_purpose "${file_CONTENT}")
    string(REGEX REPLACE "GMT_message \\(GMT, ([\"][^ ]+)[^-]+(.*\\\\n)\\\\n.+"
      "\\1 \\2\"" prog_purpose ${prog_purpose})
    list(APPEND GMT_PROG_PURPOSE ${prog_purpose})
  endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}")
endforeach(_filename)

# cmake separates elements in an appended variable or list by ";"
string(REPLACE ";" "\n  " GMT_PROG_PURPOSE "${GMT_PROG_PURPOSE}")


#configure_file(GMTprogs.txt GMTprogs.txt COPYONLY)
configure_file(gmt_config.h.cmake gmt_config.h)
configure_file(gmt_notposix.h.cmake gmt_notposix.h)
configure_file(gmt_progpurpose.h.cmake gmt_progpurpose.h)
configure_file(gmt_version.h.cmake gmt_version.h)
configure_file(isogmt.cmake isogmt @ONLY)

# generate extra files (not yet ported to cmake)
if(UNIX)
  configure_file(gmtmath.c gmtmath.c COPYONLY)
  configure_file(grdmath.c grdmath.c COPYONLY)
  add_custom_command(
    OUTPUT ${GMT_EXTRA_HDRS}
    COMMAND ${GMT_SOURCE_DIR}/cmake/generateExtraFilesNotPorted.sh ${GMT_SOURCE_DIR}
    COMMAND bash ${GMT_SOURCE_DIR}/src/make_math.sh gmt
    COMMAND bash ${GMT_SOURCE_DIR}/src/make_math.sh grd
    DEPENDS ${GMT_SOURCE_DIR}/cmake/generateExtraFilesNotPorted.sh Colors.txt
    Datums.txt Ellipsoids.txt gmt_keywords.txt GMTprogs.txt gmtapi_errors.d gmt_progpurpose.h
    ${GMT_PROGS_SRCS} ${GMT_PROGSPS_SRCS} ${GMT_LIB_SRCS}
    COMMENT "Regenerating header files..."
    VERBATIM)
endif(UNIX)
## end generate header files

# gmtprogram
set(GMT_PROGRAM ${GMT_SOURCE_DIR}/src/gmtprogram.c)

# extra definitions for triangle
set_source_files_properties(triangle
  PROPERTIES
  COMPILE_DEFINITIONS "NO_TIMER;TRILIBRARY;REDUCED;CDT_ONLY"
  COMPILE_FLAGS "-include float.h"
)


# libpslib
add_library (pslib ${GMT_PSL_SRCS})

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties(pslib
  PROPERTIES
  VERSION ${GMT_PACKAGE_VERSION}
  SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}")

# libgmt
add_library (gmtlib ${GMT_LIB_HDRS} ${GMT_EXTRA_HDRS} ${GMT_LIB_SRCS} ${GMT_LIB_PS_SRCS})

target_link_libraries(gmtlib
    ${NETCDF_LIBRARIES}
    ${GMT_OPTIONAL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    pslib)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties(gmtlib
  PROPERTIES
  OUTPUT_NAME gmt
  VERSION ${GMT_PACKAGE_VERSION}
  SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
  LINK_FLAGS "${USER_GMTLIB_LINK_FLAGS}")

# create build targets for gmt progs
string(REPLACE ".c" "" GMT_PROGS "${GMT_PROGS_SRCS}")
foreach(_gmtprog ${GMT_PROGS})
    add_executable(${_gmtprog} ${GMT_PROGRAM})
    set_target_properties(${_gmtprog}
      PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${_gmtprog}")
    target_link_libraries(${_gmtprog} gmtlib)
endforeach(_gmtprog)

# create build targets for gmt progsps
string(REPLACE ".c" "" GMT_PROGSPS "${GMT_PROGSPS_SRCS}")
foreach(_gmtprogps ${GMT_PROGSPS})
  add_executable(${_gmtprogps} ${GMT_PROGRAM})
  set_target_properties(${_gmtprogps}
    PROPERTIES COMPILE_DEFINITIONS
    "FUNC_MODE=GMTAPI_GMTPSL;FUNC=GMT_${_gmtprogps}")
  target_link_libraries(${_gmtprogps} gmtlib)
endforeach(_gmtprogps)

# generate gmt_modules.h
set(GMT_SUPPL_STRING "modules")
foreach(_function ${GMT_PROGS} ${GMT_PROGSPS})
  set (_api_function "EXTERN_MSC GMT_LONG GMT_${_function} (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)")
  list (APPEND GMT_API_FUNCTION_LIST ${_api_function})
endforeach(_function)
string(REPLACE ";" ";\n" GMT_API_FUNCTION_LIST "${GMT_API_FUNCTION_LIST}")
# create header file from template
configure_file(${GMT_SOURCE_DIR}/src/gmt_api.h.cmake gmt_${GMT_SUPPL_STRING}.h)

# build targets for demos
string(REPLACE ".c" "" GMT_DEMOS "${GMT_DEMOS_SRCS}")
foreach(_gmtdemo ${GMT_DEMOS})
  add_executable(${_gmtdemo} ${_gmtdemo}.c)
  target_link_libraries(${_gmtdemo} gmtlib)
endforeach(_gmtdemo)

add_executable(psldemo psldemo.c)
target_link_libraries(psldemo pslib)

# generate Mac debugging symbols
if(DSYMUTIL AND DEBUG_BUILD)
  foreach(target ${GMT_LIBS} ${GMT_PROGS} ${GMT_PROGSPS})
    add_custom_command(TARGET ${target}
      POST_BUILD
      COMMAND ${DSYMUTIL} $<TARGET_FILE:${target}>
      VERBATIM)
  endforeach(target)
  add_custom_target(allclean make clean
    COMMAND rm -rf *.dSYM
    COMMENT "Removing dSYM files")
endif(DSYMUTIL AND DEBUG_BUILD)

# add the install targets
install(TARGETS ${GMT_LIBS} ${GMT_PROGS} ${GMT_PROGSPS} ${GMT_DEMOS} psldemo
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

# install extra files
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/isogmt
  gmtlogo
  gmtswitch
  gmt_shell_functions.sh
  DESTINATION bin)

if(CMAKE_COMPILER_IS_GNUCC)
  # generate manpages
  foreach(_manpage ${GMT_MAN_1} ${GMT_MAN_3} ${GMT_MAN_5})
    string(REGEX REPLACE "\\.[135]" ".txt" _man_src ${_manpage})
    add_custom_command(
      OUTPUT ${_manpage}
      COMMAND ${CMAKE_C_COMPILER} - -E -w -P -nostdinc
      -I${CMAKE_CURRENT_SOURCE_DIR}
      -I${CMAKE_CURRENT_BINARY_DIR}
      < ${CMAKE_CURRENT_SOURCE_DIR}/${_man_src}
      > ${_manpage}
      DEPENDS ${GMT_MAN_DEPS} ${GMT_EXTRA_HDRS}
      COMMENT "Generate ${_manpage}"
      VERBATIM)
  endforeach(_manpage)

  # manpage target
  add_custom_target(gmt_man ALL
    DEPENDS
    ${GMT_MAN_1} ${GMT_MAN_3} ${GMT_MAN_5})

  # install manpages
  foreach(_manfile ${GMT_MAN_1})
    list(APPEND GMT_MAN_1_FILES "${CMAKE_CURRENT_BINARY_DIR}/${_manfile}")
  endforeach(_manfile)
  foreach(_manfile ${GMT_MAN_3})
    list(APPEND GMT_MAN_3_FILES "${CMAKE_CURRENT_BINARY_DIR}/${_manfile}")
  endforeach(_manfile)
  foreach(_manfile ${GMT_MAN_5})
    list(APPEND GMT_MAN_5_FILES "${CMAKE_CURRENT_BINARY_DIR}/${_manfile}")
  endforeach(_manfile)
  install(FILES ${GMT_MAN_1_FILES}
    DESTINATION ${GMT_SHARE_PATH}/man/man1)
  install(FILES ${GMT_MAN_3_FILES}
    DESTINATION ${GMT_SHARE_PATH}/man/man3)
  install(FILES ${GMT_MAN_5_FILES}
    DESTINATION ${GMT_SHARE_PATH}/man/man5)
endif(CMAKE_COMPILER_IS_GNUCC)

# add subtrees
add_subdirectory( meca )
