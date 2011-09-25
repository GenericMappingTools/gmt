#
# $Id$
#
# meca CMakeLists.txt
#

# source files
set(GMT_MECA_LIB_SRCS
  utilmeca.c
  nrutil.c
  submeca.c
  distaz.c)

set(GMT_MECA_PROGS_SRCS
  psmeca.c
  pspolar.c
  psvelo.c
  pscoupe.c)

# generate header file gmt_meca.h
string(REPLACE ".c" "" GMT_MECA_PROGS "${GMT_MECA_PROGS_SRCS}")
string(REGEX REPLACE ".*[/]" "" GMT_SUPPL_STRING
  "${CMAKE_CURRENT_SOURCE_DIR}") # current dirname
set(GMT_MECA_API_H gmt_${GMT_SUPPL_STRING}.h)
string(TOUPPER ${GMT_SUPPL_STRING} GMT_SUPPL_STRING_UPPER)
set (GMT_API_FUNCTION_LIST) # reset list
foreach(_function ${GMT_MECA_PROGS})
  set (_api_function "EXTERN_MSC GMT_LONG GMT_${_function} (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)")
  list (APPEND GMT_API_FUNCTION_LIST ${_api_function})
endforeach(_function)
string(REPLACE ";" ";\n" GMT_API_FUNCTION_LIST "${GMT_API_FUNCTION_LIST}")
# create header file from template
configure_file(${GMT_SOURCE_DIR}/src/gmt_api.h.cmake ${GMT_MECA_API_H})


include_directories(${CMAKE_CURRENT_BINARY_DIR})

# libgmt_meca
add_library (gmt_meca
  ${GMT_MECA_LIB_SRCS}
  ${GMT_MECA_PROGS_SRCS}
  ${GMT_MECA_API_H})

target_link_libraries(gmt_meca
  gmtlib)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties(gmt_meca
  PROPERTIES
  VERSION "${GMT_PACKAGE_VERSION}"
  SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}")

# create build targets for meca progs
foreach(prog ${GMT_MECA_PROGS})
  add_executable(${prog} ${GMT_PROGRAM})
  set_target_properties(${prog}
    PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMTPSL;FUNC=GMT_${prog}")
  target_link_libraries(${prog} gmt_meca)
endforeach(prog)

# generate Mac debugging symbols
if(DSYMUTIL AND DEBUG_BUILD)
  foreach(target gmt_meca ${GMT_MECA_PROGS})
    list(APPEND DSYM_FILES $<TARGET_FILE:${target}>)
    add_custom_command(TARGET ${target}
      POST_BUILD
      COMMAND ${DSYMUTIL} $<TARGET_FILE:${target}>)
  endforeach(target)
endif(DSYMUTIL AND DEBUG_BUILD)

# add the install targets
install(TARGETS gmt_meca ${GMT_MECA_PROGS}
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

