******
grdcut
******


grdcut - Extract subregion from a grid

`Synopsis <#toc1>`_
-------------------

**grdcut** *ingrid* **-G**\ *outgrid*
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [ **-V**\ [*level*\ ] ]
[ **-Z**\ [**n**\ ]\ *min/max*] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

`Description <#toc2>`_
----------------------

**grdcut** will produce a new *outgrid* file which is a subregion of
*ingrid*. The subregion is specified with **-R** as in other programs;
the specified range must not exceed the range of *ingrid*. If in doubt,
run **grdinfo** to check range. Alternatively, define the subregion
indirectly via a range check on the node values. Complementary to
**grdcut** there is **grdpaste**, which will join together two grid
files along a common edge.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*ingrid*
    This is the input grid file.
**-G**\ *outgrid*
    This is the output grid file.
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. This defines the subregion to be cut out.

`Optional Arguments <#toc5>`_
-----------------------------

**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-Z**\ [**n**\ ]\ *min/max*]
    Determine the new rectangular region so that all nodes outside this
    region are also outside the given *z*-range [-inf/+inf]. To indicate
    no limit on min or max, specify a hyphen (-). Normally, any NaNs
    encountered are simply skipped. Use **-Zn** to consider a NaN to be
    outside the *z*-range.
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
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
data. When reading grids, the format is generally automatically
recognized. If not, the same suffix can be added to input grid file
names. See `**grdreformat**\ (1) <grdreformat.1.html>`_ and Section 4.17
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
`**grdreformat**\ (1) <grdreformat.1.html>`_ and Section 4.18 of the GMT
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

Suppose you have used **surface** to grid ship gravity in the region
between 148E - 162E and 8N - 32N, and you do not trust the gridding near
the edges, so you want to keep only the area between 150E - 160E and 10N
- 30N, then:

grdcut grav\_148\_162\_8\_32.nc -Ggrav\_150\_160\_10\_30.nc
-R150/160/10/30 -V To return the subregion of a grid such that any
boundary strips where all values are entirely above 0, try

grdcut bathy.nc -Gtrimmed\_bathy.nc -Z-/0 -V

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*grdpaste*\ (1) <grdpaste.1.html>`_ ,
`*grdinfo*\ (1) <grdinfo.1.html>`_

