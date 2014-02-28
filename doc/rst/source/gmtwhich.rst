.. index:: ! gmtwhich

********
gmtwhich
********

.. only:: not man

    gmtwhich - Find full path to specified files

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmtwhich** *files* [ **-A** ] [ **-C** ] [ **-D** ] [ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

**gmtwhich** reports the full paths to the files given on the command
line. We look for the file in (1) the current directory,
(2) in $GMT_USERDIR (if defined), (3) in $GMT_DATADIR (if defined). If
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

**-A**
    Only consider files that the user has permission to read [Default consider
    all files found].
**-C**
    Instead of reporting the paths, print the confirmation Y if the file
    is found and N if it is not.
**-D**
    Instead of reporting the paths, print the directories that contains
    the files. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help.rst_

See Also
--------

:doc:`gmt`
