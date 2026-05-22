#
# In-tree custom supplement template — drop this whole directory into
# gmt/src/<myname>/ and add `set (SUPPL_EXTRA_DIRS <myname>)` to your
# cmake/ConfigUserAdvanced.cmake. The top-level GMT CMake will then
# build a separate shared library (DLL on Windows, .so MODULE on Unix)
# and install it next to the official supplements.
#
# See ../README.md for the full mechanism description, and
# gmt/src/windbarbs/CMakeLists.txt for the canonical real-world example.
#
# The variables understood by src/CMakeLists.txt are:
#
#   SUPPL_NAME           Display name of the supplement package.
#   SUPPL_LIB_NAME       (optional) Output library/DLL basename. Defaults
#                        to ${GMT_SUPPL_LIB_NAME} ("supplements"), which
#                        bundles your modules into the standard supp DLL.
#                        Set this to a distinct value if you want a
#                        SEPARATE DLL — then the DLL basename, the
#                        SHARED_LIB_NAME baked into the auto-generated
#                        glue, and your THIS_MODULE_LIB strings must all
#                        agree.
#   SUPPL_PROGS_SRCS     List of .c files, each defining one module
#                        with THIS_MODULE_* macros and a GMT_<name>
#                        entry point.
#   SUPPL_LIB_SRCS       SUPPL_PROGS_SRCS plus any helper .c files that
#                        do not define a module but provide shared code.
#   SUPPL_HEADERS        (optional) Public headers to install when
#                        BUILD_DEVELOPER is TRUE.
#   SUPPL_EXTRA_INCLUDES (optional) Extra include directories.
#   SUPPL_EXTRA_LIBS     (optional) Extra libraries to link against.
#   SUPPL_DLL_RENAME     (optional, Windows) Override the DLL basename.
#                        If set, this MUST agree with THIS_MODULE_LIB in
#                        every module source.
#   SUPPL_LIB_PURPOSE    (optional) One-line description shown by
#                        `gmt --help` for this supplement.
#   SUPPL_EXAMPLE_FILES  (optional) README and example files installed
#                        under share/doc/gmt/<name>/.
#   SUPPL_EXAMPLE_PROGS  (optional) Example scripts installed under
#                        share/doc/gmt/<name>/.
#

set (SUPPL_NAME mymodules)

# Uncomment and set if you want a separate DLL named libmymodules.dll
# rather than bundling into the standard supplements library. If you do
# this, every module .c in SUPPL_PROGS_SRCS must use the matching
# `#define THIS_MODULE_LIB "mymodules"`.
#set (SUPPL_LIB_NAME mymodules)
#set (SUPPL_LIB_PURPOSE "Custom GMT modules for ...")

set (SUPPL_PROGS_SRCS mymodule.c)

# If you have helper translation units that do NOT define a module
# (no THIS_MODULE_* macros, no GMT_<name> entry point), list them here
# in addition to SUPPL_PROGS_SRCS. Otherwise mirror SUPPL_PROGS_SRCS.
set (SUPPL_LIB_SRCS ${SUPPL_PROGS_SRCS})

#set (SUPPL_EXTRA_INCLUDES )
#set (SUPPL_EXTRA_LIBS )
#set (SUPPL_EXAMPLE_FILES README.mymodules)
#set (SUPPL_EXAMPLE_PROGS example_mymodule.sh)
