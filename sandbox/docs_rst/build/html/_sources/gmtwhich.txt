********
gmtwhich
********


gmtwhich - Find full path to specified files

`Synopsis <#toc1>`_
-------------------

**gmtwhich** *files* [ **-C** ] [ **-V**\ [*level*\ ] ]

`Description <#toc2>`_
----------------------

**gmtwhich** reports the full paths to the files given on the command
line. We look for the file `in (1) <in.1.html>`_ the current directory,
(2) in $GMT\_USERDIR (if defined), (3) in $GMT\_DATADIR (if defined). If
found we print the full path name to the file, or a confirmation (see
**-C**). The $GMT\_USERDIR and $GMT\_DATADIR environment variables can
be colon-separated list of directories, and we search recursively down
any directory that ends with / (i.e., /export/data is a single directory
whereas /export/data/ will be searched recursively.)

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*files*
    One or more file names of any data type (grids, tables, etc.).

`Optional Arguments <#toc5>`_
-----------------------------

**-C**
    Instead of reporting the paths, print the confirmation Y if the file
    is found and N if it is not.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`See Also <#toc6>`_
-------------------

`*gmt*\ <gmt.html>`_

