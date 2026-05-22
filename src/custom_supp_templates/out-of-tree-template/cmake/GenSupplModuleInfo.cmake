# GenSupplModuleInfo.cmake — generate gmt_<lib>_moduleinfo.h for an
# out-of-tree GMT supplement.
#
# This script is invoked in CMake script mode by add_custom_command in
# the parent CMakeLists.txt:
#
#   cmake -P GenSupplModuleInfo.cmake
#     -D SRC_DIR=<absolute path to dir holding the module .c files>
#     -D OUTPUT_FILE=<absolute path to gmt_<lib>_moduleinfo.h to write>
#     -D PROGS_SRCS="<semicolon-separated list of module .c basenames>"
#
# Output is the body of a `static struct GMT_MODULEINFO modules[]`
# initializer — one row per module — to be #included by the
# gmt_<lib>_glue.c translation unit.
#
# Adapted from GMT's cmake/modules/GmtGenExtraHeaders.cmake
# (LGPL, GMT Team), with one deliberate addition: if a source has only
# the legacy THIS_MODULE_NAME macro (no MODERN/CLASSIC dual macros),
# fall back to using THIS_MODULE_NAME for both fields. This makes the
# generator usable on projects still being ported from GMT5-era code.
# The in-tree GMT generator does NOT do this fallback; if you want
# strict parity, delete the fallback block and add MODERN/CLASSIC
# macros to every module.

if (NOT DEFINED OUTPUT_FILE)
    message (FATAL_ERROR "GenSupplModuleInfo: OUTPUT_FILE not set")
endif ()
if (NOT DEFINED SRC_DIR)
    message (FATAL_ERROR "GenSupplModuleInfo: SRC_DIR not set")
endif ()
if (NOT DEFINED PROGS_SRCS)
    message (FATAL_ERROR "GenSupplModuleInfo: PROGS_SRCS not set")
endif ()

separate_arguments (PROGS_SRCS)

set (_moduleinfo "")
foreach (_prog_src ${PROGS_SRCS})
    file (READ "${SRC_DIR}/${_prog_src}" _src)

    set (_modern "")
    set (_classic "")
    set (_legacy "")
    set (_lib "")
    set (_purpose "")
    set (_keys "")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_MODERN_NAME[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_modern "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_CLASSIC_NAME[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_classic "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_NAME[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_legacy "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_LIB[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_lib "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_PURPOSE[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_purpose "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_KEYS[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_keys "${CMAKE_MATCH_1}")

    # Legacy-fallback. Remove this block for in-tree-equivalent strict mode.
    if ("${_modern}" STREQUAL "")
        set (_modern "${_legacy}")
    endif ()
    if ("${_classic}" STREQUAL "")
        set (_classic "${_legacy}")
    endif ()

    if ("${_modern}" STREQUAL "")
        message (WARNING "GenSupplModuleInfo: ${_prog_src} has no THIS_MODULE_*_NAME — skipping")
        continue ()
    endif ()
    if ("${_lib}" STREQUAL "")
        message (WARNING "GenSupplModuleInfo: ${_prog_src} has no THIS_MODULE_LIB — skipping")
        continue ()
    endif ()
    if ("${_purpose}" STREQUAL "")
        message (WARNING "GenSupplModuleInfo: ${_prog_src} has no THIS_MODULE_PURPOSE")
    endif ()

    set (_row "\t{\"${_modern}\", \"${_classic}\", \"${_lib}\", \"${_purpose}\", \"${_keys}\"},")
    if (_moduleinfo)
        set (_moduleinfo "${_moduleinfo}\n${_row}")
    else ()
        set (_moduleinfo "${_row}")
    endif ()
endforeach ()

file (WRITE "${OUTPUT_FILE}" "${_moduleinfo}\n")
