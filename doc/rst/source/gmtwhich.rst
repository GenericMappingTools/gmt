********
gmtwhich
********

gmtwhich - Find full path to specified files

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**gmtwhich** *files* [ **-C** ] [ **-D** ] [ **-V**\ [*level*\ ] ]

|No-spaces|

`Description <#toc2>`_
----------------------

**gmtwhich** reports the full paths to the files given on the command
line. We look for the file `in (1) <in.html>`_ the current directory,
(2) in $GMT\_USERDIR (if defined), (3) in $GMT\_DATADIR (if defined). If
found we print the full path name to the file, just the directory (see
**-D**), or a confirmation (see **-C**). The $GMT\_USERDIR and
$GMT\_DATADIR environment variables can be colon-separated list of
directories, and we search recursively down any directory that ends with
/ (i.e., /export/data is a single directory whereas /export/data/ will
be searched recursively.) 

`Required Arguments <#toc4>`_
-----------------------------

*files*
    One or more file names of any data type (grids, tables, etc.).

`Optional Arguments <#toc5>`_
-----------------------------

**-C**
    Instead of reporting the paths, print the confirmation Y if the file
    is found and N if it is not.
**-D**
    Instead of reporting the paths, print the directories that contains
    the files. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help.rst_

`See Also <#toc6>`_
-------------------

`gmt5 <gmt5.html>`_
