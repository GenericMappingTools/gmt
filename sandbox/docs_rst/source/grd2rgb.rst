*******
grd2rgb
*******


grd2rgb - Write r/g/b grid files from a grid file, a raw RGB file, or
SUN rasterfile

`Synopsis <#toc1>`_
-------------------

**grd2rgb** *infile* **-G**\ *template* [ **-C**\ *cptfile* ] [
**-I**\ *xinc*\ [**m**\ \|\ **s**][/\ *yinc*\ [**m**\ \|\ **s**]] ] [
**-L**\ *layer* ] [ **-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] ] [
**-V**\ [*level*\ ] ] [ **-W**\ *width*/*height*\ [/*n\_bytes*] ] [
**-r** ]

`Description <#toc2>`_
----------------------

**grd2rgb** reads one of three types of input files: (1) A Sun 8-, 24-,
or 32-bit raster file; we the write out the red, green, and blue
components (0-255 range) to separate grid files. Since the raster file
header is limited you may use the **-R**, **-I**, **-r** options to set
a complete header record [Default is simply based on the number of rows
and columns]. (2) A binary 2-D grid file; we then convert the z-values
to red, green, blue via the provided cpt file. Optionally, only write
out one of the r, g, b, layers. (3) A RGB or RGBA raw raster file. Since
raw rasterfiles have no header, you have to give the image dimensions
via the **-W** option.

`Required Arguments <#toc3>`_
-----------------------------

*infile*
    `The (1) <The.1.html>`_ Sun raster file, (2) 2-D binary grid file,
    `or (3) <or.3.html>`_ raw raster file to be converted.
**-G**\ *template*
    Provide an output name template for the three output grids. The
    template should be a regular grid file name except it must contain
    the string %c which on output will be replaced by r, g, or b.

`Common Arguments And Specifications <#toc4>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Optional Arguments <#toc5>`_
-----------------------------

**-C**\ *cptfile*
    name of the color palette table (for 2-D binary input grid only).
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
**-L**\ *layer*
    Output only the specified layer (r, g, or b). [Default outputs all 3
    layers].
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**\ *width*/*height*\ [/*n\_bytes*]
    Sets the size of the raw raster file. By default an RGB file (which
    has 3 bytes/pixel) is assumed. For RGBA files use *n\_bytes* = 4.
    Use **-W** for guessing the image size of a RGB raw file, and
    **-W**\ *=/=/4* if the raw image is of the RGBA type. Notice that
    this might be a bit slow because the guessing algorithm makes uses
    of FFTs.
**-r**
    Set pixel node registration [gridline].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Examples <#toc6>`_
-------------------

To use the color palette topo.cpt to create r, g, b component grids from
hawaii\_grv.nc file, use

grd2rgb hawaii\_grv.nc -Ctopo.cpt -Ghawaii\_grv\_%c.nc

To output the red component from the Sun raster radiation.ras file, use

grd2rgb radiation.ras -Lr -Gcomp\_%c.nc

`See Also <#toc7>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*gmt.conf*\ (5) <gmt.conf.5.html>`_ ,
`*grdedit*\ (1) <grdedit.1.html>`_ ,
`*grdimage*\ (1) <grdimage.1.html>`_ ,
`*grdmath*\ (1) <grdmath.1.html>`_ , `*grdview*\ (1) <grdview.1.html>`_

