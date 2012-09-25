*********
grdsample
*********

grdsample - Resample a grid onto a new lattice

`Synopsis <#toc1>`_
-------------------

**grdsample** *in\_grdfile* **-G**\ *out\_grdfile* [
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
] [ **-L**\ *flag* ] [ **-R**\ *west*/*east*/*south*/*north*\ [**r**\ ]
] [ **-T** ] [ **-V**\ [*level*\ ] ] [ **-f**\ *colinfo* ] [
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
] [ **-r** ]

`Description <#toc2>`_
----------------------

**grdsample** reads a grid file and interpolates it to create a new grid
file with either: a different registration (**-r** or **-T**); or, a new
grid-spacing or number of nodes (**-I**), and perhaps also a new
sub-region (**-R**). A bicubic [Default], bilinear, B-spline or
nearest-neighbor interpolation (**-n**) is used, requiring boundary
conditions (**-L**). Note that using **-R** only is equivalent to
**grdcut** or **grdedit -S**. **grdsample** safely creates a fine mesh
from a coarse one; the converse may suffer aliasing unless the data are
filtered using **grdfft** or **grdfilter**.

When **-R** is omitted, the output grid will cover the same region as
the input grid. When **-I** is omitted, the grid spacing of the output
grid will be the same as the input grid. Either **-r** or **-T** can be
used to change the grid registration. When omitted, the output grid will
have the same registration as the input grid.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*in\_grdfile*
    The name of the input 2-D binary grid file. (See GRID FILE FORMAT
    below.)
**-G**\ *out\_grdfile*
    The name of the output grid file. (See GRID FILE FORMAT below.)

`Optional Arguments <#toc5>`_
-----------------------------

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
**-L**\ *flag*
    Boundary condition *flag* may be *x* or *y* or *xy* indicating data
    is periodic in range of x or y or both set by **-R**, or *flag* may
    be *g* indicating geographical conditions (x and y are lon and lat).
    [Default uses "natural" conditions (second partial derivative normal
    to edge is zero) unless the grid is automatically recognized as
    periodic.]
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
**-T**
    Translate between grid and pixel registration; if the input is
    grid-registered, the output will be pixel-registered and vice-versa.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
(\*)
    Select interpolation mode for grids.
**-r**
    Set pixel node registration [gridline].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

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

`Hints <#toc8>`_
----------------

If an interpolation point is not on a node of the input grid, then a NaN
at any node in the neighborhood surrounding the point will yield an
interpolated NaN. Bicubic interpolation [default] yields continuous
first derivatives but requires a neighborhood of 4 nodes by 4 nodes.
Bilinear interpolation [**-n**\ ] uses only a 2 by 2 neighborhood, but
yields only zero-order continuity. Use bicubic when smoothness is
important. Use bilinear to minimize the propagation of NaNs.

`Examples <#toc9>`_
-------------------

To resample the 5 x 5 minute grid in hawaii\_5by5\_topo.nc onto a 1
minute grid:

grdsample hawaii\_5by5\_topo.nc -I1m -Ghawaii\_1by1\_topo.nc

To translate the gridline-registered file surface.nc to pixel
registration while keeping the same region and grid interval:

grdsample surface.nc -T -Gpixel.nc

`See Also <#toc10>`_
--------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdedit*\ (1) <grdedit.html>`_ ,
`*grdfft*\ (1) <grdfft.html>`_ , `*grdfilter*\ (1) <grdfilter.html>`_
