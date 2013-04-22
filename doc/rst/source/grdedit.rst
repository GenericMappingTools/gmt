*******
grdedit
*******

grdedit - Modify header or content of a grid

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**grdedit** *grid* [ **-A** ] [
**-D**\ *xname*/*yname*/*zname*/*scale*/*offset*/*title*/*remark* ] [
**-E** ] [ **-N**\ *table* ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-S** ] [ **-T** ]
[ **-V**\ [*level*\ ] ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-:**\ [**i**\ \|\ **o**] ]

|No-spaces|

`Description <#toc2>`_
----------------------

**grdedit** reads the header information in a binary 2-D grid file and
replaces the information with values provided on the command line [if
any]. As an option, global, geographical grids (with 360 degrees
longitude range) can be rotated in the east-west direction, and
individual nodal values can be replaced from a table of *x*, *y*, *z*
values. **grdedit** only operates on files containing a grdheader. Note:
If it is important to retain the original data you should work on a copy
of that file. 

`Required Arguments <#toc4>`_
-----------------------------

*grid*
    Name of the 2-D grid file to modify. (See GRID FILE FORMATS below).

`Optional Arguments <#toc5>`_
-----------------------------

**-A**
    If necessary, adjust the file’s *x\_inc*, *y\_inc* to be compatible
    with its domain (or a new domain set with **-R**). Older grid files
    (i.e., created prior to **GMT** 3.1) often had excessive slop in
    *x\_inc*, *y\_inc* and an adjustment is necessary. Newer files are
    created correctly.
**-D**\ *xname*/*yname*/*zname*/*scale*/*offset*/*title*/*remark*
    Give new values for *xname*, *yname*, *zname*, *scale*, *offset*,
    *title*, and *remark*. To leave some of the values untouched,
    specify = as the new value. Alternatively, to allow "/" to be part
    of one of the values, use any non-alphanumeric character (and not
    the equal sign) as separator by both starting and ending with it.
    For example:
    **-D**:*xname*:*yname*:*zname*:*scale*:*offset*:*title*:*remark*:
**-E**
    Transpose the grid and exchange the *x* and *y* information.
    Incompatible with the other options.
**-N**\ *table*
    Read the ASCII (or binary; see **-bi**\ [*ncols*\ ][*type*\ ]) file
    *table* and replace the corresponding nodal values in the grid with
    these *x*,\ *y*,\ *z* values. 

.. |Add_-R| replace:: The new w/e/s/n values will
    replace those in the grid, and the *x\_inc*, *y\_inc* values are
    adjusted, if necessary.
.. include:: explain_-R.rst_

**-S**
    For global, geographical grids only. Grid values will be shifted
    longitudinally according to the new borders given in **-R**.
**-T**
    Make necessary changes in the header to convert a
    gridline-registered grid to a pixel-registered grid, or vice-versa.
    Basically, gridline-registered grids will have their domain extended
    by half the x- and y-increments whereas pixel-registered grids will
    have their domain shrunk by the same amount. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 3 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout.rst_

.. include:: explain_grd_coord.rst_

`Examples <#toc8>`_
-------------------

Let us assume the file data.nc covers the area 300/310/10/30. We want to
change the boundaries from geodetic longitudes to geographic and put a
new title in the header. We accomplish this by

    grdedit data.nc -R-60/-50/10/30 -D=/=/=/=/=/"Gravity Anomalies"/=

The grid world.nc has the limits 0/360/-72/72. To shift the data so that
the limits would be -180/180/-72/72, use

    grdedit world.nc -R-180/180/-72/72 -S

The file junk.nc was created prior to **GMT** 3.1 with incompatible
**-R** and **-I** arguments. To reset the x- and y-increments we run

    grdedit junk.nc -A

The file junk.nc was created prior to **GMT** 4.1.3 and does not contain
the required information to indicate that the grid is geographic. To add
this information, run

    grdedit junk.nc -fg

`See Also <#toc9>`_
-------------------

`gmt5 <gmt5.html>`_, `grd2xyz <grd2xyz.html>`_, `xyz2grd <xyz2grd.html>`_
