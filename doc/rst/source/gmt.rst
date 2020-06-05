.. index:: ! gmt

***
gmt
***

.. only:: not man

    The Generic Mapping Tools data processing and display software package

Introduction
------------

GMT is a collection of freely available command-line tools under the GNU LGPL that allows you to
manipulate x,y and x,y,z data sets (filtering, trend fitting, gridding,
projecting, etc.) and produce illustrations ranging from
simple x-y plots, via contour maps, to artificially illuminated surfaces
and 3-D perspective views in black/white or full color. Linear, log10,
and power scaling is supported in addition to over 30 common map
projections. The processing and display routines within GMT are
completely general and will handle any (x,y) or (x,y,z) data as input.

Synopsis
--------

**gmt** is the main program that can start any of the modules:

**gmt** *module* *module-options*
    Starts a given GMT *module* with the *module-options*
    that pertain to that particular module.  A few special commands
    are also available:

**gmt clear** *items*
    Deletes current defaults, or the cache, data or sessions directories.
    Choose between **defaults** (deletes the current gmt.conf file used for the
    current modern session), **cache** (deletes the user's cache directory
    and all of its content), **data** (deletes the user's data download directory
    and all of its content), or **all** (does all of the above).

**gmt begin** [*session-prefix*] [*format*] [*options*]
    Initializes a new GMT session under *modern* mode [Default is *classic* mode].
    All work is performed in a temporary work directory.
    The optional *session-prefix* assigns a name to the session, and this may be used
    as figure name for single-figure sessions [gmtsession].  Likewise, the optional
    *format* can be used to override the default graphics format [PDF].

**gmt figure** *prefix* [*format(s)*] [*options*]
    Specifies the desired name, output format(s) and any custom arguments that should
    be passed to :doc:`psconvert` when producing this figure.  All subsequent plotting
    will be directed to this current figure until another **gmt figure** command is issued
    or the session ends.  The *prefix* is used to build final figure names when extensions
    are automatically appended. The *format* setting is a comma-separated list of desired
    extensions (e.g., pdf,png).

**gmt inset** [*arguments*]
    Allows users to place a map inset by temporarily changing where plotting takes place
    as well as the region and projection, then resets to previous stage.

**gmt subplot** [*arguments*]
    Allows users to create a matrix of panels with automatic labeling and advancement.

**gmt end** [**show**]
    Terminates a GMT modern mode session and automatically converts the registered
    illustration(s) to their specified formats, then eliminates the temporary work
    directory.  The figures are placed in the current directory.

For information on any module, load the module documentation
in your browser via gmt :doc:`docs`, e.g.::

    gmt docs grdimage

If no module is given then several other options are available:

**--help**
    List and description of GMT modules.

**--new-script**\ [=\ *L*]
    Write a GMT modern mode script template to stdout. Optionally append the desired
    scripting language among *bash*, *csh*, or *batch*.  Default is the main shell
    closest to your current shell (e.g., bash for zsh, csh for tcsh).

**--new-glue**\ =\ *name*
    Write the C code glue needed when building third-party supplements as shared
    libraries.  The *name* is the name of the shared library. Run **gmt** in the directory
    of the supplement and the glue code will be written to *stdout*.  Including this C code
    when building the shared library means **gmt** can list available modules via the
    **--show-modules**, **--help** options.  We recommend saving the code to gmt\_\ *name*\_glue.c.

**--show-bindir**
    Show directory of executables and exit.

**--show-citation**
    Show the citation for the latest GMT publication.

**--show-classic**
    List classic module names on stdout and exit.

**--show-cores**
    Show number of available cores.

**--show-datadir**
    Show data directory/ies and exit.

**--show-dataserver**
    Show URL of the remote GMT data server.

**--show-doi**
    Show the DOI of the current release.

**--show-modules**
    List modern module names on stdout and exit.

**--show-library**
    Show the path of the shared GMT library.

**--show-plugindir**
    Show plugin directory and exit.

**--show-sharedir**
    Show share directory and exit.

**--version**
    Print version and exit.

**=**
    Check if that module exist and if so the
    program will exit with status of 0; otherwise the status of exit will
    be non-zero.

Command-line completion
-----------------------

GMT provides basic command-line completion (tab completion) for bash.
The completion rules are either installed in ``/etc/bash_completion.d/gmt``
or ``<prefix>/share/tools/gmt_completion.bash``.  Depending on the
distribution, you may still need to source the gmt completion file from
``~/.bash_completion`` or ``~/.bashrc``.  For more information see Section
:ref:`command-line-completion` in the CookBook.

GMT Modules
-----------

Run **gmt --help** to print the list of all core and supplementals modules
within GMT, and a very short description of their purpose.
Detailed information about each program can be found in the separate manual pages.

Custom Modules
--------------

The **gmt** program can also load custom modules from shared libraries
built as specified in the GMT API documentation.  This way your modules
can benefit from the GMT infrastructure and extend GMT in specific ways.

The Common GMT Options
----------------------

.. include:: common_SYN_OPTs.rst_

|SYN_OPT-B|
**-J**\ *parameters*
**-Jz**\|\ **Z**\ *parameters*
|SYN_OPT-Rz|
|SYN_OPT-U|
|SYN_OPT-V|
|SYN_OPT-X|
|SYN_OPT-Y|
|SYN_OPT-a|
|SYN_OPT-b|
|SYN_OPT-c|
|SYN_OPT-d|
|SYN_OPT-e|
|SYN_OPT-f|
|SYN_OPT-g|
|SYN_OPT-h|
|SYN_OPT-i|
|SYN_OPT-j|
|SYN_OPT-l|
|SYN_OPT-n|
|SYN_OPT-o|
|SYN_OPT-p|
|SYN_OPT-q|
|SYN_OPT-r|
|SYN_OPT-s|
|SYN_OPT-t|
|SYN_OPT-x|
|SYN_OPT-:|

Description
-----------

These are all the common GMT options that remain the same for all GMT
modules. No space between the option flag and the associated arguments.

.. include:: explain_-B_full.rst_

.. include:: explain_-J_full.rst_

.. include:: explain_-Jz_full.rst_

.. include:: explain_-Jproj_full.rst_

.. include:: explain_-R_full.rst_

.. include:: explain_-Rz_full.rst_

.. include:: explain_-U_full.rst_

.. include:: explain_-V_full.rst_

.. include:: explain_-XY_full.rst_

.. include:: explain_-aspatial_full.rst_

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bi_full.rst_

.. include:: explain_-bo_full.rst_

.. |Add_-c| unicode:: 0x20 .. just an invisible code
.. include:: explain_-c_full.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d_full.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di_full.rst_

.. |Add_-do| unicode:: 0x20 .. just an invisible code
.. include:: explain_-do_full.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e_full.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f_full.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g_full.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h_full.rst_

.. include:: explain_-icols_full.rst_

.. |Add_-j| unicode:: 0x20 .. just an invisible code
.. include:: explain_distcalc_full.rst_

.. include:: explain_-l_full.rst_

.. include:: explain_-n_full.rst_

.. include:: explain_-ocols_full.rst_

.. include:: explain_perspective_full.rst_

.. |Add_-q| unicode:: 0x20 .. just an invisible code
.. include:: explain_-q_full.rst_

.. |Add_-qi| unicode:: 0x20 .. just an invisible code
.. include:: explain_-qi_full.rst_

.. |Add_-qo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-qo_full.rst_

.. include:: explain_nodereg_full.rst_

.. include:: explain_-s_full.rst_

.. include:: explain_-t_full.rst_

.. include:: explain_core_full.rst_

.. include:: explain_colon_full.rst_

.. include:: explain_help.rst_

.. include:: explain_color.rst_

.. include:: explain_fill.rst_

.. include:: explain_font.rst_

.. include:: explain_pen.rst_

.. include:: explain_precision.rst_

.. include:: explain_grd_inout.rst_

Classic Mode Options
--------------------

These options are only used in classic mode and are listed here just for reference.

.. include:: explain_-K_full.rst_

.. include:: explain_-O_full.rst_

.. include:: explain_-P_full.rst_

See Also
--------

Look up the individual man pages for more details and full syntax. Run
``gmt --help`` to list all GMT programs and to show all installation
directories. For an explanation of the various GMT settings in this
man page (like :term:`FORMAT_FLOAT_OUT`), see the man page of the GMT
configuration file :doc:`gmt.conf`. Information is also available on the
GMT documentation site https://docs.generic-mapping-tools.org/

See Also
--------

:doc:`docs`
