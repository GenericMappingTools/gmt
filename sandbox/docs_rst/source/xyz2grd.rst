*******
xyz2grd
*******


xyz2grd - Convert data table to a grid file

`Synopsis <#toc1>`_
-------------------

**xyz2grd** [ *table* ] **-G**\ *grdfile*
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [
**-A**\ [**f**\ \|\ **l**\ \|\ **m**\ \|\ **n**\ \|\ **r**\ \|\ **s**\ \|\ **u**\ \|\ **z**]
] [ **-D**\ *xname*/*yname*/*zname*/*scale*/*offset*/*title*/*remark* ]
[ **-N**\ *nodata* ] [ **-S**\ [*zfile*\ ] ] [ **-V**\ [*level*\ ] ] [
**-Z**\ [*flags*\ ] ] [ **-bi**\ [*ncol*\ ][**t**\ ] ] [
**-f**\ *colinfo* ] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**xyz2grd** reads one or more z or xyz tables and creates a binary grid
file. **xyz2grd** will report if some of the nodes are not filled in
with data. Such unconstrained nodes are set to a value specified by the
user [Default is NaN]. Nodes with more than one value will be set to the
mean value. As an option (using **-Z**), a 1-column z-table may be read
assuming all nodes are present (z-tables can be in organized in a number
of formats, see **-Z** below.)

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-G**\ *grdfile*
    *grdfile* is the name of the binary output grid file. (See GRID FILE
    FORMAT below.)
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
    *x\_inc* [and optionally *y\_inc*] is the grid spacing. Optionally,
    append a suffix modifier. **Geographical (degrees) coordinates**:
    Append **m** to indicate arc minutes or **s** to indicate arc
    seconds. If one of the units **e**, **f**, **k**, **M**, or **n** is
    appended instead, the increment is assumed to be given in meter,
    feet, km, Miles, or nautical miles, respectively, and will be
    converted to the equivalent degrees longitude at the middle latitude
    of the region (the conversion depends on **PROJ\_ELLIPSOID**). If
    /*y\_inc* is given but set to 0 it will be reset equal to *x\_inc*;
    otherwise it will be converted to degrees latitude. **All
    coordinates**: If **=** is appended then the corresponding max *x*
    (*east*) or *y* (*north*) may be slightly adjusted to fit exactly
    the given increment [by default the increment may be adjusted
    slightly to fit the given domain]. Finally, instead of giving an
    increment you may specify the *number of nodes* desired by appending
    **+** to the supplied integer argument; the increment is then
    recalculated from the number of nodes and the domain. The resulting
    increment value depends on whether you have selected a
    gridline-registered or pixel-registered grid; see Appendix B for
    details. Note: if **-R**\ *grdfile* is used then the grid spacing
    has already been initialized; use **-I** to override the values.
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII [or binary, see **-bi**\ [*ncol*\ ][**t**\ ]]
    files holding z or (x,y,z) values. The xyz triplets do not have to
    be sorted. One-column z tables must be sorted and the **-Z** must be set.
**-A**\ [**f**\ \|\ **l**\ \|\ **m**\ \|\ **n**\ \|\ **r**\ \|\ **s**\ \|\ **u**\ \|\ **z**]
    By default we will calculate mean values if multiple entries fall on
    the same node. Use **-A** to change this behavior, except it is
    ignored if **-Z** is given. Append **f** or **s** to simply keep the
    first or last data point that was assigned to each node. Append
    **l** or **u** to find the lowest (minimum) or upper (maximum) value
    at each node, respectively. Append **m** or **r** to compute mean or
    RMS value at each node, respectively. Append **n** to simply count
    the number of data points that were assigned to each node. Append
    **z** to sum multiple values that belong to the same node.
**-D**\ *xname*/*yname*/*zname*/*scale*/*offset*/*title*/*remark*
    Give values for *xname*, *yname*, *zname*, *scale*, *offset*,
    *title*, and *remark*. To leave some of these values untouched,
    specify = as the value. Alternatively, to allow "/" to be part of
    one of the values, use any non-alphanumeric character (and not the
    equal sign) as separator by both starting and ending with it. For
    example:
    **-D**:*xname*:*yname*:*zname*:*scale*:*offset*:*title*:*remark*:
**-N**\ *nodata*
    No data. Set nodes with no input xyz triplet to this value [Default
    is NaN]. For z-tables, this option is used to replace z-values that
    equal *nodata* with NaN.
**-S**\ [*zfile*\ ]
    Swap the byte-order of the input only. No grid file is produced. You
    must also supply the **-Z** option. The output is written to *zfile*
    (or stdout if not supplied).
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-Z**\ [*flags*\ ]
    Read a 1-column ASCII [or binary] table. This assumes that all the
    nodes are present and sorted according to specified ordering
    convention contained in *flags*. If incoming data represents rows,
    make *flags* start with `**T**\ (op) <T.op.html>`_ if first row is y
    = ymax or `**B**\ (ottom) <B.ottom.html>`_ if first row is y = ymin.
    Then, append **L** or **R** to indicate that first element is at
    left or right end of row. Likewise for column formats: start with
    **L** or **R** to position first column, and then append **T** or
    **B** to position first element in a row. For gridline registered
    grids: If data are periodic in x but the incoming data do not
    contain the (redundant) column at x = xmax, append **x**. For data
    periodic in y without redundant row at y = ymax, append **y**.
    Append **s**\ *n* to skip the first *n* number of bytes (probably a
    header). If the byte-order needs to be swapped, append **w**. Select
    one of several data types (all binary except **a**):

    `` `` `` `` **A** ASCII representation of one or more floating point
    values per record

    `` `` `` `` **a** ASCII representation of a single item per record
    `` `` `` `` **c** signed 1-byte character
    `` `` `` `` **u** unsigned 1-byte character
    `` `` `` `` **h** signed 2-byte integer
    `` `` `` `` **H** unsigned 2-byte integer
    `` `` `` `` **i** signed 4-byte integer
    `` `` `` `` **I** unsigned 4-byte integer
    `` `` `` `` **l** long (8-byte) integer [requires 64-bit mode]
    `` `` `` `` **L** unsigned long (8-byte) integer [requires 64-bit mode]
    `` `` `` `` **f** 4-byte floating point single precision
    `` `` `` `` **d** 8-byte floating point double precision

    Default format is scanline orientation of ASCII numbers: **-ZTLa**.
    Note that **-Z** only applies to 1-column input. The difference
    between **A** and **a** is that the latter can decode both
    *date*\ **T**\ *clock* and *ddd:mm:ss[.xx]* formats while the former
    is strictly for regular floating point values.

**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input. [Default is 3 input columns]. This option only
    applies to xyz input files; see **-Z** for z tables.
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s). Not used with binary data.
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] (\*)
    Select input columns.
**-r**
    Set pixel node registration [gridline].
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Grid Values Precision <#toc6>`_
--------------------------------

Regardless of the precision of the input data, GMT programs that create
grid files will internally hold the grids in 4-byte floating point
arrays. This is done to conserve memory and furthermore most if not all
real data can be stored using 4-byte floating point values. Data with
higher precision (i.e., double precision values) will lose that
precision once GMT operates on the grid or writes out new grids. To
limit loss of precision when processing data you should always consider
normalizing the data prior to processing.

`Grid File Formats <#toc7>`_
----------------------------

By default **GMT** writes out grid as single precision floats in a
COARDS-complaint netCDF file format. However, **GMT** is able to produce
grid files in many other commonly used grid file formats and also
facilitates so called "packing" of grids, writing out floating point
data as 2- or 4-byte integers. To specify the precision, scale and
offset, the user should add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. See `**grdreformat**\ (1) <grdreformat.1.html>`_ and Section 4.17
of the GMT Technical Reference and Cookbook for more information.

When writing a netCDF file, the grid is stored by default with the
variable name "z". To specify another variable name *varname*, append
**?**\ *varname* to the file name. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes.

`Geographical And Time Coordinates <#toc8>`_
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

`Examples <#toc9>`_
-------------------

To create a grid file from the ASCII data in hawaii\_grv.xyz, use

xyz2grd hawaii\_grv.xyz -Ddegree/degree/mGal/1/0/"Hawaiian
Gravity"/"GRS-80 Ellipsoid used" -Ghawaii\_grv\_new.nc -R198/208/18/25
-I5m -V

To create a grid file from the raw binary (3-column, single-precision
scanline-oriented data raw.b, use

xyz2grd raw.b -Dm/m/m/1/0/=/= -Graw.nc -R0/100/0/100 -I1 -V -Z -bi3f

To make a grid file from the raw binary USGS DEM (short integer
scanline-oriented data topo30. on the NGDC global relief Data CD-ROM,
with values of -9999 indicate missing data, one must on some machine
reverse the byte-order. On such machines (like Sun, use

xyz2grd topo30. -Dm/m/m/1/0/=/= -Gustopo.nc -R234/294/24/50 -I30s
-N-9999 -B -ZTLhw

Say you have received a binary file with 4-byte floating points that
were written on a machine of different byte-order than yours. You can
swap the byte-order with

xyz2grd floats.bin -Snew\_floats.bin -V -Zf

`See Also <#toc10>`_
--------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*grd2xyz*\ (1) <grd2xyz.1.html>`_ ,
`*grdedit*\ (1) <grdedit.1.html>`_
`*grdreformat*\ (1) <grdreformat.1.html>`_

