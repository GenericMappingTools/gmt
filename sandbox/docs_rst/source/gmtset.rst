******
gmtset
******

gmtset - Change individual **GMT** default parameters

`Synopsis <#toc1>`_
-------------------

**gmtset** [ **-C** \| **-D**\ [**s**\ \|\ **u**] \|
**-G**\ *defaultsfile* ] [ **-**\ [**BJRXxYycp**\ ]\ *value* ]
PARAMETER1 [=] *value1* PARAMETER2 [=] *value2* PARAMETER3 [=] *value3*
...

`Description <#toc2>`_
----------------------

**gmtset** will adjust individual **GMT** defaults settings in the
current directory’s **gmt.conf** file. If no such file exists one will
be created. The main purpose of **gmtset** is temporarily to change
certain parameters inside a shell script, e.g., set the dots-per-inch to
72, run the script, and reset to 1200 dpi. Optionally, you can specify
one or more temporary changes directly on any **GMT** command line with
the syntax **--PARAMETER**\ =\ *VALUE*; such changes are only in effect
for that command and do not permanently change the default settings on
disk.

`Required Arguments <#toc3>`_
-----------------------------

PARAMETER *value*
    Provide one or several pairs of parameter/value combinations that
    you want to modify. For a complete listing of available parameters
    and their meaning, see the **gmt.conf** man page.

`Optional Arguments <#toc4>`_
-----------------------------

**-C**
    Convert a .gmtdefaults4 file created by GMT4 to a **gmt.conf** file
    used by GMT5. The original file is retained.
**-D**\ [**s**\ \|\ **u**]
    Modify the **GMT** defaults based on the system settings. Append
    **u** for US defaults or **s** for SI defaults. [**-D** alone gives
    the version selected at compile time]
**-G**\ *defaultsfile*
    Name of specific **gmt.conf** file to read and modify [Default looks
    first in current directory, then in your home directory, then in
    ~/.gmt and finally in the system defaults].
**-**\ [**BJRXxYycp**\ ]\ *value*
    Set the expansion of any of these shorthand options.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc5>`_
-------------------

To change annotation font to 12-point Helvetica, select grid-crosses of
size 0.1 inch, and set annotation offset to 0.2 cm:

gmtset FONT\_ANNOT\_PRIMARY 12p,Helvetica
MAP\_GRID\_CROSS\_SIZE\_PRIMARY 0.1i MAP\_ANNOT\_OFFSET\_PRIMARY 0.2c

`See Also <#toc6>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*gmtdefaults*\ (1) <gmtdefaults.html>`_ ,
`*gmtget*\ (1) <gmtget.html>`_
