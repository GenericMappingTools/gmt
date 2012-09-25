********
grdpaste
********

grdpaste - Join two grids along their common edge

`Synopsis <#toc1>`_
-------------------

**grdpaste** *file\_a.nc file\_b.nc* **-G**\ *outfile.nc* [
**-V**\ [*level*\ ] ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

`Description <#toc2>`_
----------------------

**grdpaste** will combine *file\_a.nc* and *file\_b.nc* into
*outfile.nc* by pasting them together along their common edge. Files
*file\_a.nc* and *file\_b.nc* must have the same dx, dy and have one
edge in common. If in doubt, check with **grdinfo** and use **grdcut**
and/or **grdsample** if necessary to prepare the edge joint. For
geographical grids, use **-f** to handle periodic longitudes.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*file\_a.nc*
    One of two files to be pasted together.
*file\_b.nc*
    The other of two files to be pasted together.
**-G**\ *outfile.nc*
    The name for the combined output.

`Optional Arguments <#toc5>`_
-----------------------------

**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
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

Suppose file\_a.nc is 150E - 180E and 0 - 30N, and file\_b.nc is 150E -
180E, -30S - 0, then you can make outfile.nc which will be 150 - 180 and
-30S - 30N by:

grdpaste file\_a.nc file\_b.nc -Goutfile.nc -V -fg

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdblend*\ (1) <grdblend.html>`_ ,
`*grdcut*\ (1) <grdcut.html>`_ , `*grdinfo*\ (1) <grdinfo.html>`_ ,
`*grdsample*\ (1) <grdsample.html>`_
