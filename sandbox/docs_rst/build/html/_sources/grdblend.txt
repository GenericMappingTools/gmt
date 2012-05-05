********
grdblend
********


grdblend - Blend several partially over-lapping grids into one large
grid

`Synopsis <#toc1>`_
-------------------

**grdblend** [ *blendfile* \| *grid1* *grid2* ... ] **-G**\ *outgrid*
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [
**-C**\ **f**\ \|\ **l**\ \|\ **o**\ \|\ **u** ] [ **-N**\ *nodata* ] [
**-Q** ] [ **-Z**\ *scale* ] [ **-V**\ [*level*\ ] ] [ **-W** ] [
**-f**\ *colinfo* ] [ **-r** ]

`Description <#toc2>`_
----------------------

**grdblend** reads a listing of grid files and blend parameters and
creates a binary grid file by blending the other grids using
cosine-taper weights. **grdblend** will report if some of the nodes are
not filled in with data. Such unconstrained nodes are set to a value
specified by the user [Default is NaN]. Nodes with more than one value
will be set to the weighted average value. Any input grid that does not
share the final output grid’s node registration and grid spacing will
automatically be resampled via calls to grdsample. Note: Due to the
row-by-row i/o nature of operations in grdblend we only support the
netCDF and native binary grid formats for both input and output.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-G**\ *outgrid*
    *outgrid* is the name of the binary output grid file. (See GRID FILE
    FORMATS below). Only netCDF and native binary grid formats are can
    be written directly. Other output format choices will be handled by
    reformatting the output once blending is complete.
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

*blendfile*
    ASCII file with one record per grid file to include in the blend.
    Each record must contain three items, separated by spaces or tabs:
    the gridfile name, the **-R**-setting for the interior region, and
    the relative weight *wr*. In the combined weighting scheme, this
    grid will be given zero weight outside its domain, weight = *wr*
    inside the interior region, and a 2-D cosine-tapered weight between
    those end-members in the boundary strip. However, if a negative *wr*
    is given then the sense of tapering is inverted (i.e., zero weight
    inside its domain). If the inner region should instead exactly match
    the grid region then specify a - instead of the **-R**-setting.
    Finally, if only a gridfile name is found we will use the grid
    domain as inner region and set weight to 1. If the ASCII file is not
    given **grdblend** will read standard input. Alternatively, if you
    have more than one grid file to blend and you wish (a) all input
    grids to have the same `weight (1) <weight.1.html>`_ and (b) all
    grids should use their region as the interior region, then you may
    simply list all the grids on the command line instead of providing a
    *blendfile*. You must specify at least 2 input grids for this
    mechanism to work. Any grid that is not coregistered with the
    desired output layout implied by **-R**, **-I** (and **-r**) will
    first be resampled via **grdsample**. Also, grids that are not in
    netCDF or native binary format will first be reformatted via
    **grdreformat**.
**-C**
    Clobber mode: Instead of blending, simply pick the value of one of
    the grids that covers a node. Select from the following modes: **f**
    for the first grid to visit a node; **o** for the last grid to visit
    a node; **l** for the grid with the lowest value, and **u** for the
    grid with the uppermost value. For modes **f** and **o** the
    ordering of grids in the *blendfile* will dictate which grid
    contributes to the final result. Weights and cosine tapering are not
    considered when clobber mode is active.
**-N**\ *nodata*
    No data. Set nodes with no input grid to this value [Default is NaN].
**-Q**
    Create a header-less grid file suitable for use with **grdraster**.
    Requires that the output grid file is a native format (i.e., not netCDF).
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**
    Do not blend, just output the weights used for each node. This
    option is valid when only one input grid is provided [Default makes
    the blend].
**-Z**\ *scale*
    Scale output values by *scale* before writing to file. [1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-r**
    Set pixel node registration [gridline].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Grid File Formats <#toc6>`_
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

To create a grid file from the four grid files piece\_?.nc, make the
blendfile like this

piece\_1.nc\ `` `` `` `` -R<subregion\_1> 1
piece\_2.nc\ `` `` `` `` -R<subregion\_2> 1
piece\_3.nc\ `` `` `` `` -R<subregion\_3> 1
piece\_4.nc\ `` `` `` `` -R<subregion\_4> 1

Then run

grdblend blend.job -Gblend.nc -R<full\_region> -I<dx/dy> -V

To blend all the grids called MB\_\*.nc given them all equal weight, try

grdblend MB\_\*.nc -Gblend.nc -R<full\_region> -I<dx/dy> -V

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*grd2xyz*\ (1) <grd2xyz.1.html>`_ ,
`*grdedit*\ (1) <grdedit.1.html>`_
`*grdraster*\ (1) <grdraster.1.html>`_

