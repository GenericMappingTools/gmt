.. index:: ! gmtwhich
.. include:: module_core_purpose.rst_

*****
which
*****

|gmtwhich_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt which** *files*
[ |-A| ]
[ |-C| ]
[ |-D| ]
[ |-G|\ [**a**\|\ **c**\|\ **l**\|\ **u**] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**which** reports the paths to the files given on the command
line. We look for the file in (1) the current directory,
(2) in **$GMT_USERDIR** (if defined), (3) in **$GMT_DATADIR** (if defined), or
(4) in **$GMT_CACHEDIR** (if defined). If
found we print the path name to the file, just the directory (see
|-D|), or a confirmation (see |-C|). The **$GMT_USERDIR** and
**$GMT_DATADIR** environment variables can be comma-separated list of
directories, and we search recursively down any directory that ends with
/ (i.e., /export/data is a single directory whereas /export/data/ will
be searched recursively.)  If the file is in the current directory then
we just return the relative path (i.e., the file name); otherwise we
return the full path.

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

**-G**\ [**a**\|\ **c**\|\ **l**\|\ **u**]
    If a file is downloadable (either a complete URL, an @file for
    downloading from the GMT data server, or @earth_relief_xxy or any other of the
    remote datasets at https://www.generic-mapping-tools.org/remote-datasets/)
    we will try to download the file if it is not found in your local data or cache dirs.

    - **a** - Place files in the appropriate folder under the user directory (this is where
      GMT normally places downloaded files).
    - **c** - Download to the user cache directory.
    - **l** - Download to the current directory [Default].
    - **u** - Download to the user data directory (i.e., ignoring any subdirectory structure).

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: explain_help.rst_

Examples
--------

To see the path to the file myjunk.txt, use::

    gmt which myjunk.txt

To download the 10 arc-minute global relief file from the GMT data server, use::

    gmt which -Ga @earth_relief_10m_g

which will print the path (after downloading if not already present).  The file will
be placed in the appropriate folder under the user's **$GMT_USERDIR**.  To obtain a GMT
example or test file from the GMT data server, try::

    gmt which -Gc @hotspots.txt

which will print the path (after downloading if not already present).  The file will
be placed in the user's **$GMT_CACHEDIR** directory.

See Also
--------

:doc:`gmt`
