*******
grdclip
*******

grdclip - Clip the range of grids

`Synopsis <#toc1>`_
-------------------

**grdclip** *ingrid* **-G**\ *outgrid* [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-Sa**\ *high/above* ] [ **-Sb**\ *low/below* ] [ **-V**\ [*level*\ ] ]

`Description <#toc2>`_
----------------------

**grdclip** will set values < *low* to *below* and/or values > *high* to
*above*. Useful when you want all of a continent or an ocean to fall
into one color or gray shade in image processing, or clipping of the
range of data values is required. *above/below* can be any number or NaN
(Not a Number). You must choose at least one of **-Sa** or **-Sb**. Use
**-R** to only extract a subset of the *ingrid* file.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*ingrid*
    The input 2-D binary grid file.
**-G**\ *outgrid*
    *outgrid* is the modified output grid file.

`Optional Arguments <#toc5>`_
-----------------------------

**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. Using the **-R** option will select
    a subsection of *ingrid* grid. If this subsection exceeds the
    boundaries of the grid, only the common region will be extracted.
**-Sa**\ *high/above*
    Set all data[i] > *high* to *above*.
**-Sb**\ *low/below*
    Set all data[i] < *low* to *below*.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
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

`Examples <#toc7>`_
-------------------

To set all values > 70 to NaN and all values < 0 to 0 in file data.nc:

grdclip data.nc -Gnew\_data.nc -Sa70/NaN -Sb0/0 -V

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdlandmask*\ (1) <grdlandmask.html>`_ ,
`*grdmask*\ (1) <grdmask.html>`_ , `*grdmath*\ (1) <grdmath.html>`_ ,
`*grd2xyz*\ (1) <grd2xyz.html>`_ , `*xyz2grd*\ (1) <xyz2grd.html>`_
