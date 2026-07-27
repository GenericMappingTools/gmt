:orphan:

Custom GMT Supplements --- In-Tree vs Out-of-Tree
=================================================

.. note::

   The investigation underlying this document and the document itself
   (including the accompanying template files under
   :file:`src/custom_supp_templates/`) were produced by Claude Opus 4.7
   (Anthropic), working from the GMT source tree and a real-world
   out-of-tree supplement port (MB-System's ``mbsystem.dll``). Treat
   the contents as a starting point that has been mechanically verified
   against the GMT source as of the date of writing but has not been
   human-reviewed for stylistic or policy fit with the GMT project.
   Corrections welcome.

This document describes the two ways to build a GMT supplemental shared
library ("supplement" or "suppl"): the **in-tree** path (your supplement
lives under :file:`gmt/src/<name>/` and is built when GMT itself is
built) and the **out-of-tree** path (your supplement is built by an
independent CMake project that links against an already-installed GMT).
Both produce a DLL or shared module that exposes GMT modules via
``gmt --show-modules``, ``gmt --help``, and the external-API key/group
lookup used by Julia, Python, and MATLAB wrappers.

The accompanying :file:`src/custom_supp_templates/in-tree-template/`
and :file:`src/custom_supp_templates/out-of-tree-template/` directories
contain copy-and-modify starter trees for each case.

How a GMT supplement actually works
-----------------------------------

A GMT supplement is just a shared library that the GMT core loads at
runtime through ``dlopen`` / ``LoadLibrary``. GMT discovers libraries
from the ``GMT_CUSTOM_LIBS`` GMT default (or the built-in supplement
path). For each loaded library, GMT looks up symbols **by name**, using
the library **basename** as a prefix. For a supplement DLL named
``mylib``, GMT resolves these five symbols with ``dlsym``:

.. list-table::
   :widths: 35 65
   :header-rows: 1

   * - Symbol
     - Purpose
   * - ``mylib_module_show_all``
     - Pretty-print all modules + purpose for ``gmt --help``
   * - ``mylib_module_list_all``
     - Plain list of modern module names for ``gmt --show-modules``
   * - ``mylib_module_classic_all``
     - Plain list of classic module names for ``gmt --show-classic``
   * - ``mylib_module_keys``
     - Return ``THIS_MODULE_KEYS`` string for an external-API consumer
   * - ``mylib_module_group``
     - Return ``THIS_MODULE_LIB`` string for an external-API consumer

Per individual module ``xxx``, GMT also looks up the actual entry point
``GMT_xxx`` (for example ``GMT_grdbarb``, ``GMT_psbarb``). That function
must be exported from the DLL
(``EXTERN_MSC int GMT_xxx(void *API, int mode, void *args);``).

The five ``_module_*`` lookup functions all share a per-supplement
table called ``static struct GMT_MODULEINFO modules[]``, populated with
one row per module:

.. code-block:: c

   struct GMT_MODULEINFO {
       const char *mname;        /* THIS_MODULE_MODERN_NAME */
       const char *cname;        /* THIS_MODULE_CLASSIC_NAME */
       const char *component;    /* THIS_MODULE_LIB */
       const char *purpose;      /* THIS_MODULE_PURPOSE */
       const char *keys;         /* THIS_MODULE_KEYS */
   };

The table is the heart of a supplement. Everything else is plumbing
around it. The lookup-function bodies are nearly identical across all
supplements; they just delegate to ``GMT_Show_ModuleInfo`` /
``GMT_Get_ModuleInfo`` from GMT's public API.

Because the body is boilerplate and the table content is derived
mechanically from ``#define THIS_MODULE_*`` macros in each module's
``.c``, GMT auto-generates both during the build. That generation is
what the in-tree mechanism gives you for free --- and what the
out-of-tree case must reproduce by hand.

The in-tree mechanism
---------------------

Look at :file:`src/windbarbs/` for the canonical, minimal example: two
module sources (``grdbarb.c``, ``psbarb.c``) plus one shared helper
(``windbarb.c``) and a fourteen-line :file:`CMakeLists.txt`. That
``CMakeLists.txt`` declares only the sources --- no ``add_library``, no
glue file, no ``gen_*`` command:

.. code-block:: cmake

   set (SUPPL_NAME windbarbs)
   set (SUPPL_PROGS_SRCS grdbarb.c psbarb.c)
   set (SUPPL_LIB_SRCS ${SUPPL_PROGS_SRCS} windbarb.c)
   set (SUPPL_EXAMPLE_FILES README.windbarb)

:file:`src/CMakeLists.txt` does the rest. The loop in that file walks
every supplement directory listed in ``GMT_SUPPL_DIRS`` (which by
default includes ``geodesy gsfml gshhg img mgd77 potential segy seis
spotter x2sys windbarbs`` plus anything the user puts in
``SUPPL_EXTRA_DIRS``) and for each one:

1. Calls ``add_subdirectory(<dir>)`` so the supplement's
   :file:`CMakeLists.txt` sets ``SUPPL_NAME``, ``SUPPL_PROGS_SRCS``,
   ``SUPPL_LIB_SRCS``, optionally ``SUPPL_LIB_NAME``,
   ``SUPPL_EXTRA_LIBS``, ``SUPPL_EXTRA_INCLUDES``, ``SUPPL_DLL_RENAME``.
2. Reads those variables back via ``get_subdir_var``.
3. Accumulates per-library source lists into
   ``SUPPL_<lib>_PROGS_SRCS`` and ``SUPPL_<lib>_LIB_SRCS``. (Multiple
   supplement directories can contribute to one library by sharing
   ``SUPPL_LIB_NAME``.)
4. For each resulting library:

   - Runs ``gen_gmt_moduleinfo_h`` from
     :file:`cmake/modules/GmtGenExtraHeaders.cmake`. The macro greps
     every module source for ``THIS_MODULE_MODERN_NAME``,
     ``THIS_MODULE_CLASSIC_NAME``, ``THIS_MODULE_LIB``,
     ``THIS_MODULE_PURPOSE``, ``THIS_MODULE_KEYS`` and writes
     :file:`gmt_<lib>_moduleinfo.h` (one row per module).
   - Runs ``configure_file(gmt_glue.c.in gmt_<lib>_glue.c)`` substituting
     ``@SHARED_LIB_NAME@`` and ``@SHARED_LIB_PURPOSE@``. The generated
     ``.c`` ``#include`` s the generated ``.h`` to fill in ``modules[]``
     and defines the five ``<lib>_module_*`` entry points as
     ``EXTERN_MSC``.
   - Builds the shared library / module from the union of accumulated
     module sources, library sources, generated glue, and generated
     header.

5. Installs the library into :file:`gmt/plugins/`.

That is the entire mechanism. The supplement author writes only the
module sources (with the right ``THIS_MODULE_*`` macros at the top)
plus a fourteen-line :file:`CMakeLists.txt`; everything that exposes
the modules to the runtime is generated.

Adding a custom in-tree supplement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Create :file:`gmt/src/<myname>/` containing your ``.c`` sources
   plus a :file:`CMakeLists.txt` shaped like
   :file:`src/windbarbs/CMakeLists.txt`. See the
   :file:`src/custom_supp_templates/in-tree-template/` directory.
2. Edit (or create) :file:`cmake/ConfigUserAdvanced.cmake` and set
   ``set (SUPPL_EXTRA_DIRS <myname>)``. (Multiple custom supplement
   directories can be listed.)
3. Configure and build GMT normally. Your DLL appears under
   :file:`gmt/plugins/` alongside the official supplements.
4. Verify with ``gmt --show-modules`` (your module names should appear)
   and ``gmt <yourmodule>``.

The five-symbol contract and the ``THIS_MODULE_*`` macro requirement
described above are non-negotiable in either path. The in-tree
mechanism just satisfies them automatically.

The out-of-tree mechanism
-------------------------

A custom supplement built **outside** the GMT source tree (as part of
an independent project --- for example MB-System builds ``mbsystem.dll``
from its own CMake project that links against an installed GMT) does
**not** get the auto-generated glue and moduleinfo header. The
supplement author has to either:

(a) Reproduce the generation in the host project's CMake, or
(b) Hand-author :file:`gmt_<lib>_glue.c` and
    :file:`gmt_<lib>_moduleinfo.h` and treat them as ordinary source
    files.

Option (a) is strongly recommended --- the moduleinfo header should
track the module sources automatically, and hand-editing it is
error-prone. The :file:`src/custom_supp_templates/out-of-tree-template/`
directory shows option (a) end-to-end.

What out-of-tree projects must provide
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. **Their own copy of** :file:`gmt_glue.c.in`. GMT does not install
   :file:`gmt_glue.c.in` to a public location (it lives in the source
   tree only). Vendor a copy into your project; you only need to
   change ``@SHARED_LIB_NAME@`` and ``@SHARED_LIB_PURPOSE@``.

2. **Their own moduleinfo generator.** GMT's
   :file:`cmake/modules/GmtGenExtraHeaders.cmake` is similarly
   source-tree-only and the ``gen_gmt_moduleinfo_h`` macro is tightly
   coupled to GMT's own directory layout (it reads from
   ``${GMT_SRC}/src/${prog}``). Vendor a standalone CMake ``-P``
   script that does the same regex extraction over your project's
   module sources. See
   :file:`src/custom_supp_templates/out-of-tree-template/cmake/GenSupplModuleInfo.cmake`.

3. **A CMakeLists.txt that wires up the generator → glue → library.**
   See
   :file:`src/custom_supp_templates/out-of-tree-template/CMakeLists.txt`.

4. **The right library output name.** GMT calls ``dlsym`` using the
   **DLL basename**, so the CMake target's ``OUTPUT_NAME`` (and any
   ``*_DLL_RENAME``) must match the ``SHARED_LIB_NAME`` you bake into
   the glue. If the DLL is :file:`mylib.dll`, the glue must export
   ``mylib_module_list_all`` etc. --- mismatch means GMT silently
   finds nothing.

5. **DLL export visibility.** On Windows the entry points need
   ``__declspec(dllexport)``. The vendored :file:`gmt_glue.c.in` uses
   ``EXTERN_MSC``, which GMT's :file:`declspec.h` flips to
   ``dllexport`` when ``LIBRARY_EXPORTS`` is defined. Add

   .. code-block:: cmake

      target_compile_definitions(<target> PRIVATE LIBRARY_EXPORTS)

   to your CMakeLists. Each module's ``GMT_<modulename>`` entry point
   should also be declared ``EXTERN_MSC`` (or you can rely on CMake's
   ``CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS``, but the explicit attribute is
   cleaner and matches what in-tree supplements do).

Real-world worked example
~~~~~~~~~~~~~~~~~~~~~~~~~

MB-System builds ``mbsystem.dll`` out-of-tree against an installed GMT.
Its :file:`src/gmt/CMakeLists.txt` followed the recipe above (using a
vendored ``MbsysGenModuleInfo.cmake`` and ``gmt_mbsystem_glue.c.in``)
after diagnosing that an earlier hand-written ``mbgmt_module.c``
exported the wrong symbol names (``gmt_mbgmt_module_show_all`` instead
of ``mbsystem_module_list_all``), which is why ``gmt --show-modules``
showed nothing and Julia's ``GMT_Encode_Options`` returned null keys.

The MB-System fix is a close analogue of the
:file:`src/custom_supp_templates/out-of-tree-template/` provided here;
consult that project as a secondary reference if you want to see the
mechanism integrated into a larger codebase.

Module source requirements (both paths)
---------------------------------------

Every module ``.c`` must define these macros near the top, before any
``#include "gmt_dev.h"``:

.. code-block:: c

   #define THIS_MODULE_CLASSIC_NAME  "mymodule"     /* required */
   #define THIS_MODULE_MODERN_NAME   "mymodule"     /* required */
   #define THIS_MODULE_LIB           "mylib"        /* must equal DLL basename */
   #define THIS_MODULE_PURPOSE       "One-line description"
   #define THIS_MODULE_KEYS          "<G{,>X}"      /* API key string; "" if none */
   #define THIS_MODULE_NEEDS         "Jg"           /* projection requirements; "" if none */
   #define THIS_MODULE_OPTIONS       "->BJKOPRUVXY" /* common options accepted */

The legacy ``THIS_MODULE_NAME`` (a single string used by very old GMT5
ports) is **not** recognised by the modern generator. If you are
porting old code, add explicit ``THIS_MODULE_MODERN_NAME`` and
``THIS_MODULE_CLASSIC_NAME`` (typically both equal to the old name) or
adapt your generator to fall back. The vendored
:file:`src/custom_supp_templates/out-of-tree-template/cmake/GenSupplModuleInfo.cmake`
does the fallback for convenience; the in-tree GMT generator does not.

Each module must also expose:

.. code-block:: c

   EXTERN_MSC int GMT_mymodule(void *V_API, int mode, void *args);

as a defined function (not just a declaration). That is what GMT
dispatches to when the user runs ``gmt mymodule``.

Loading a custom supplement at runtime
--------------------------------------

After building and installing, point GMT at the new library. Either:

- Set the GMT default ``GMT_CUSTOM_LIBS`` to the absolute path of the
  DLL (or to a colon/semicolon separated list of paths), for example
  ``gmt set GMT_CUSTOM_LIBS C:/path/to/mylib.dll``.
- Or drop the DLL into GMT's default plugin directory
  (:file:`<gmt-install>/lib/gmt/plugins/` on Unix,
  :file:`<gmt-install>/bin/gmt_plugins/` on Windows).

Verify:

.. code-block:: sh

   gmt --show-modules        # your modules should appear in the list
   gmt mymodule --help       # your module should run

If ``gmt --show-modules`` does not list your modules but the DLL
clearly loaded (no error printed), the most likely cause is a mismatch
between the DLL basename and the ``SHARED_LIB_NAME`` baked into the
glue. Inspect the DLL with ``dumpbin /exports mylib.dll`` (MSVC) or
``nm -D mylib.so`` (Unix) and confirm that ``mylib_module_list_all``
and friends are present.

File overview in :file:`src/custom_supp_templates/`
---------------------------------------------------

.. code-block:: text

   src/custom_supp_templates/
   ├── README.md                                this document (markdown copy)
   ├── in-tree-template/
   │   ├── CMakeLists.txt                       drop-in for src/<myname>/
   │   └── mymodule.c                           skeleton module source
   └── out-of-tree-template/
       ├── CMakeLists.txt                       standalone CMake project
       ├── gmt_mylib_glue.c.in                  vendored from gmt_glue.c.in
       ├── mymodule.c                           skeleton module source
       └── cmake/
           └── GenSupplModuleInfo.cmake         vendored moduleinfo generator
