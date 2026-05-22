#
# Out-of-tree custom GMT supplement template.
# See ../README.md for the full mechanism description.
#
# This builds a shared library `mylib` that ships GMT modules and is
# loadable by an already-installed GMT (set GMT_CUSTOM_LIBS to point at
# the built DLL, or copy the DLL into GMT's plugin directory).
#
# To adapt:
#   1. Replace MYLIB_SUPPL_NAME / MYLIB_SUPPL_PURPOSE.
#   2. Rename gmt_mylib_glue.c.in to gmt_<yourname>_glue.c.in (or keep
#      and tweak the configure_file source path).
#   3. List your module .c files in MYLIB_MODULE_SRCS.
#   4. Each module .c must define THIS_MODULE_LIB equal to
#      MYLIB_SUPPL_NAME, plus THIS_MODULE_MODERN_NAME/CLASSIC_NAME,
#      THIS_MODULE_PURPOSE, THIS_MODULE_KEYS. See mymodule.c.
#

cmake_minimum_required(VERSION 3.16)
project(mylib_suppl C)

find_package(GMT CONFIG)
if (NOT GMT_FOUND)
    message(STATUS "GMTConfig.cmake not found, falling back to FindGMT.cmake")
    find_package(GMT REQUIRED)
endif ()

# ---- supplement identity ----------------------------------------------------
# The DLL name (OUTPUT_NAME below) MUST equal MYLIB_SUPPL_NAME, because
# GMT resolves modules with dlsym("<DLLname>_module_<verb>_all").
set(MYLIB_SUPPL_NAME    mylib)
set(MYLIB_SUPPL_PURPOSE "Custom GMT modules for ...")

set(MYLIB_MODULE_SRCS
    mymodule.c
    # add more module .c files here
)

# ---- generate moduleinfo header --------------------------------------------
set(MYLIB_MODULEINFO_H "${CMAKE_CURRENT_BINARY_DIR}/gmt_${MYLIB_SUPPL_NAME}_moduleinfo.h")
set(MYLIB_GLUE_C       "${CMAKE_CURRENT_BINARY_DIR}/gmt_${MYLIB_SUPPL_NAME}_glue.c")

set(_mylib_moduleinfo_inputs)
foreach (_s ${MYLIB_MODULE_SRCS})
    list(APPEND _mylib_moduleinfo_inputs "${CMAKE_CURRENT_SOURCE_DIR}/${_s}")
endforeach ()

add_custom_command(
    OUTPUT "${MYLIB_MODULEINFO_H}"
    COMMAND ${CMAKE_COMMAND}
        -D "SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        -D "OUTPUT_FILE=${MYLIB_MODULEINFO_H}"
        -D "PROGS_SRCS=${MYLIB_MODULE_SRCS}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenSupplModuleInfo.cmake"
    DEPENDS
        ${_mylib_moduleinfo_inputs}
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenSupplModuleInfo.cmake"
    COMMENT "Generating ${MYLIB_SUPPL_NAME} moduleinfo header"
    VERBATIM)

# ---- generate glue translation unit -----------------------------------------
# These two vars are consumed by gmt_mylib_glue.c.in via configure_file.
set(SHARED_LIB_NAME    "${MYLIB_SUPPL_NAME}")
set(SHARED_LIB_PURPOSE "${MYLIB_SUPPL_PURPOSE}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/gmt_mylib_glue.c.in"
    "${MYLIB_GLUE_C}"
    @ONLY)

# ---- build shared library ---------------------------------------------------
add_library(${MYLIB_SUPPL_NAME} SHARED
    ${MYLIB_MODULE_SRCS}
    ${MYLIB_GLUE_C}
    ${MYLIB_MODULEINFO_H})

set_target_properties(${MYLIB_SUPPL_NAME} PROPERTIES
    OUTPUT_NAME ${MYLIB_SUPPL_NAME}
    PREFIX ""
    VERSION 0
    SOVERSION 0)

# GMT's declspec.h flips EXTERN_MSC to __declspec(dllexport) when
# LIBRARY_EXPORTS is defined. Required so our module entry points and
# the glue's <lib>_module_* symbols are exported on Windows.
target_compile_definitions(${MYLIB_SUPPL_NAME} PRIVATE LIBRARY_EXPORTS)

# The generated glue and moduleinfo header live in the binary dir.
target_include_directories(${MYLIB_SUPPL_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(${MYLIB_SUPPL_NAME} PRIVATE GMT::GMT GMT::PSL)

install(TARGETS ${MYLIB_SUPPL_NAME}
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib)
