.. index:: ! gmtset

***
set
***

.. only:: not man

    Change individual GMT default parameters

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt set** [ |-C| \| |-D|\ [**s**\ \|\ **u**] \|
|-G|\ *defaultsfile* ] [ **-**\ [**BJRXxYycp**\ ]\ *value* ]
PARAMETER1 [=] *value1* PARAMETER2 [=] *value2* PARAMETER3 [=] *value3*
...

|No-spaces|

Description
-----------

**set** will adjust individual GMT defaults settings in the
current directory's :doc:`gmt.conf` file. If no such file exists one will
be created. The main purpose of **set** is temporarily to change
certain parameters inside a shell script, e.g., set the map frame type to
plain, run the script, and reset to fancy.  Only parameters that differ
from the GMT SI system defaults will be written.  Optionally, you can specify
one or more temporary changes directly on any GMT command line with
the syntax **-**\ **-PARAMETER**\ =\ *VALUE*; such changes are only in effect
for that command and do not permanently change the default settings on
disk.

Required Arguments
------------------

PARAMETER *value*
    Provide one or several pairs of parameter/value combinations that
    you want to modify. For a complete listing of available parameters
    and their meaning, see the :doc:`gmt.conf` man page.

Optional Arguments
------------------

.. _-C:

**-C**
    Convert a .gmtdefaults4 file created by GMT4 to a :doc:`gmt.conf` file
    used by modern GMT. The original file is retained.

.. _-D:

**-D**\ [**s**\ \|\ **u**]
    Modify the GMT defaults based on the system settings. Append
    **u** for US defaults or **s** for SI defaults. [**-D** alone gives
    the version selected at compile time]

.. _-G:

**-G**\ *defaultsfile*
    Name of specific :doc:`gmt.conf` file to read and modify [Default looks
    first in current directory, then in your home directory, then in
    ~/.gmt and finally in the system defaults].

**-**\ [**BJRXxYycp**]\ *value*
    Set the expansion of any of these shorthand options. 

.. include:: explain_help.rst_

Examples
--------

To change annotation font to 12-point Helvetica, select grid-crosses of
size 0.1 inch, and set annotation offset to 0.2 cm:

   ::

    gmt set FONT_ANNOT_PRIMARY 12p,Helvetica \
            MAP_GRID_CROSS_SIZE_PRIMARY 0.1i MAP_ANNOT_OFFSET_PRIMARY 0.2c

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtdefaults`, :doc:`gmtget`
