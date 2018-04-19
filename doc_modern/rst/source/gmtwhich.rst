.. index:: ! gmtwhich

*****
which
*****

.. only:: not man

    Find full path to specified files

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt which** *files* [ |-A| ] [ |-C| ] [ |-D| ] [ |-G|\ [**c**\ \|\ **l**\ \|\ **u**] ] [ |SYN_OPT-V| ] [ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**which** reports the full paths to the files given on the command
line. We look for the file in (1) the current directory,
(2) in $GMT_USERDIR (if defined), (3) in $GMT_DATADIR (if defined), or
(4) in $GMT_CACHEDIR (if defined). If
found we print the full path name to the file, just the directory (see
**-D**), or a confirmation (see **-C**). The $GMT_USERDIR and
$GMT_DATADIR environment variables can be colon-separated list of
directories, and we search recursively down any directory that ends with
/ (i.e., /export/data is a single directory whereas /export/data/ will
be searched recursively.) 

Required Arguments
------------------

*files*
    One or more file names of any data type (grids, tables, etc.).

Optional Arguments
------------------

.. _-A:

**-A**
    Only consider files that the user has permission to read [Default consider
    all files found].

.. _-C:

**-C**
    Instead of reporting the paths, print the confirmation Y if the file
    is found and N if it is not.

.. _-D:

**-D**
    Instead of reporting the paths, print the directories that contains
    the files. 

.. _-G:

**-G**\ [**c**\ \|\ **l**\ \|\ **u**]
    If a file argument is a downloadable file (either a full URL, a @file for
    downloading from the GMT Site Cache, or @earth_relief_*.grd) we will try
    to download the file if it is not found in your local data or cache dirs.
    By default [**-Gl**] we download to the current directory. Append **c** to place
    in the user cache directory or **u** user data directory instead.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help.rst_

Examples
--------

To see the full path to the file myjunk.txt, use

   ::

    gmt which myjunk.txt

To download the 10 arc-minute global relief file from the GMT data site, use

   ::

    gmt which -Gu @earth_relief_10m.grd

which will print the path (after downloading if not already present).  The file will
be placed in the user's GMT_USER_DIR.  To obtain a GMT example or test file from the
GMT cache site, try

   ::

    gmt which -Gc @hotspots.txt

which will print the path (after downloading if not already present).  The file will
be placed in the user's GMT_CACHE_DIR directory.


See Also
--------

:doc:`gmt`
