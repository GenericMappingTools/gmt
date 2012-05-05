*******
grd2xyz
*******


grd2xyz - Convert grid file to data table

`Synopsis <#toc1>`_
-------------------

**grd2xyz** *grid* [ **-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ]
[ **-V**\ [*level*\ ] ] [ **-W**\ [*weight*\ ] ] [ **-Z**\ [*flags*\ ] ]
[ **-bo**\ [*ncol*\ ][**t**\ ] ] [ **-f**\ *colinfo* ] [
**-ho**\ [*n*\ ] ] [ **-o**\ *cols*\ [,*...*] ] [
**-s**\ [*cols*\ ][\ **a**\ \|\ **r**] ]

`Description <#toc2>`_
----------------------

**grd2xyz** reads one or more binary 2-D grid files and writes out
xyz-triplets in ASCII [or binary] format to standard output. Modify the
precision of the ASCII output format by editing the
**FORMAT\_FLOAT\_OUT** parameter in your **gmt.conf** file or use
**--D\_FORMAT**\ =\ *format* on the command line, or choose binary
output using single or double precision storage. As an option you may
output z-values without the (x,y) coordinates; see **-Z** below.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*grid*
    Names of 2-D binary grid files to be converted. (See GRID FILE
    FORMATS below.)

`Optional Arguments <#toc5>`_
-----------------------------

**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. Using the **-R** option will select
    a subsection of the grid. If this subsection exceeds the boundaries
    of the grid, only the common region will be output.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**\ [*weight*\ ]
    Write out x,y,z,w, where w is the supplied *weight* (or 1 if not
    supplied) [Default writes x,y,z only].
**-Z**\ [*flags*\ ]
    Write a 1-column ASCII [or binary] table. Output will be organized
    according to the specified ordering convention contained in *flags*.
    If data should be written by rows, make *flags* start with
    `**T**\ (op) <T.op.html>`_ if first row is y = ymax or
    `**B**\ (ottom) <B.ottom.html>`_ if first row is y = ymin. Then,
    append **L** or **R** to indicate that first element should start at
    left or right end of row. Likewise for column formats: start with
    **L** or **R** to position first column, and then append **T** or
    **B** to position first element in a row. For gridline registered
    grids: If grid is periodic in x but the outcoming data should not
    contain the (redundant) column at x = xmax, append **x**. For grid
    periodic in y, skip writing the redundant row at y = ymax by
    appending **y**. If the byte-order needs to be swapped, append
    **w**. Select one of several data types (all binary except **a**):

    `` `` `` `` **a** ASCII representation of a single item per record
    `` `` `` `` **c** signed 1-byte character
    `` `` `` `` **u** unsigned 1-byte character
    `` `` `` `` **h** short 2-byte integer
    `` `` `` `` **H** unsigned short 2-byte integer
    `` `` `` `` **i** 4-byte integer
    `` `` `` `` **I** unsigned 4-byte integer
    `` `` `` `` **l** long (8-byte) integer [requires 64-bit mode]
    `` `` `` `` **L** unsigned long (8-byte) integer [requires 64-bit
    mode]
    `` `` `` `` **f** 4-byte floating point single precision
    `` `` `` `` **d** 8-byte floating point double precision

    Default format is scanline orientation of ASCII numbers: **-ZTLa**.
    Note that **-Z** only applies to 1-column output.

**-bo**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary output. [Default is 3]. This option only applies to
    xyz output; see **-Z** for z table output.
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns. See also **TIME
    COORDINATES** below. **-h** Output 1 header record based on
    information in the first grid file header. Ignored if binary output
    is selected. [Default is no header].
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-s**\ [*cols*\ ][\ **a**\ \|\ **r**] (\*)
    Set handling of NaN records.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Ascii Format Precision <#toc6>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Grid File Formats <#toc7>`_
----------------------------

**GMT** is able to recognize many of the commonly used grid file
formats, as well as the precision, scale and offset of the values
contained in the grid file. When **GMT** needs a little help with that,
you can add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. See `**grdreformat**\ (1) <grdreformat.1.html>`_ and Section 4.17
of the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. See `**grdreformat**\ (1) <grdreformat.1.html>`_ and
Section 4.18 of the GMT Technical Reference and Cookbook for more
information, particularly on how to read splices of 3-, 4-, or
5-dimensional grids.

`Time Coordinates <#toc8>`_
---------------------------

Time coordinates in netCDF grids, be it the x, y, or z coordinate, will
be recognized as such. The variable’s BD(unit) attribute is parsed to
determine the unit and epoch of the time coordinate in the grid. Values
are then converted to the internal time system specified by
**TIME\_UNIT** and **TIME\_EPOCH** in the **gmt.conf** file or on the
command line. The default output is relative time in that time system,
or absolute time when using the option **-f0T**, **-f1T**, or **-f2T**
for x, y, or z coordinate, respectively.

`Examples <#toc9>`_
-------------------

To edit individual values in the 5’ by 5’ hawaii\_grv.nc file, dump the
.nc to ASCII:

grd2xyz hawaii\_grv.nc > hawaii\_grv.xyz

To write a single precision binary file without the x,y positions from
the file raw\_data.nc file, using scanline orientation, run

grd2xyz raw\_data.nc -ZTLf > hawaii\_grv.b

`See Also <#toc10>`_
--------------------

`*gmt.conf*\ (5) <gmt.conf.5.html>`_ , `*gmt*\ (1) <gmt.1.html>`_ ,
`*grdedit*\ (1) <grdedit.1.html>`_ ,
`*grdreformat*\ (1) <grdreformat.1.html>`_ ,
`*xyz2grd*\ (1) <xyz2grd.1.html>`_

