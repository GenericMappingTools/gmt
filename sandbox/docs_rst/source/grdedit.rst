*******
grdedit
*******

grdedit - Modify header or content of a grid

`Synopsis <#toc1>`_
-------------------

**grdedit** *grid* [ **-A** ] [
**-D**\ *xname*/*yname*/*zname*/*scale*/*offset*/*title*/*remark* ] [
**-E** ] [ **-N**\ *table* ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-S** ] [ **-T** ]
[ **-V**\ [*level*\ ] ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**grdedit** reads the header information in a binary 2-D grid file and
replaces the information with values provided on the command line [if
any]. As an option, global, geographical grids (with 360 degrees
longitude range) can be rotated in the east-west direction, and
individual nodal values can be replaced from a table of *x*, *y*, *z*
values. **grdedit** only operates on files containing a grdheader.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

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
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. The new w/e/s/n values will replace
    those in the grid, and the *x\_inc*, *y\_inc* values are adjusted,
    if necessary.
**-S**
    For global, geographical grids only. Grid values will be shifted
    longitudinally according to the new borders given in **-R**.
**-T**
    Make necessary changes in the header to convert a
    gridline-registered grid to a pixel-registered grid, or vice-versa.
    Basically, gridline-registered grids will have their domain extended
    by half the x- and y-increments whereas pixel-registered grids will
    have their domain shrunk by the same amount.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 3 input columns].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Grid File Formats <#toc6>`_
----------------------------

By default **GMT** writes out grid as single precision floats in a
COARDS-complaint netCDF file format. However, **GMT** is able to produce
grid files in many other commonly used grid file formats and also
facilitates so called "packing" of grids, writing out floating point
data as 1- or 2-byte integers. To specify the precision, scale and
offset, the user should add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. When reading grids, the format is generally automatically
recognized. If not, the same suffix can be added to input grid file
names. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20
of the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. The **?**\ *varname* suffix can also be used for output
grids to specify a variable name different from the default: "z". See
`**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20 of the GMT
Technical Reference and Cookbook for more information, particularly on
how to read splices of 3-, 4-, or 5-dimensional grids.

`Geographical And Time Coordinates <#toc7>`_
--------------------------------------------

When the output grid type is netCDF, the coordinates will be labeled
"longitude", "latitude", or "time" based on the attributes of the input
data or grid (if any) or on the **-f** or **-R** options. For example,
both **-f0x** **-f1t** and **-R**\ 90w/90e/0t/3t will result in a
longitude/time grid. When the x, y, or z coordinate is time, it will be
stored in the grid as relative time since epoch as specified by
**TIME\_UNIT** and **TIME\_EPOCH** in the **gmt.conf** file or on the
command line. In addition, the **unit** attribute of the time variable
will indicate both this unit and epoch.

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

`*gmt*\ (1) <gmt.html>`_ , `*grd2xyz*\ (1) <grd2xyz.html>`_ ,
`*xyz2grd*\ (1) <xyz2grd.html>`_
