*******
grdinfo
*******


grdinfo - Extract information from grids

`Synopsis <#toc1>`_
-------------------

**grdinfo** *grdfiles* [ **-C** ] [ **-F** ] [ **-I**\ [*dx*\ [/*dy*]] ]
[ **-L**\ [**0**\ \|\ **1**\ \|)2)] ] [ **-M** ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-T**\ [**s**\ *dz* ] [ **-V**\ [*level*\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

`Description <#toc2>`_
----------------------

**grdinfo** reads a 2-D binary grid file and reports metadata and
various statistics for the (*x*,\ *y*,\ *z*) data in the grid file(s).
The output information contains the minimum/maximum values for *x*, *y*,
and *z*, where the min/max of *z* occur, the *x*- and *y*-increments,
and the number of *x* and *y* nodes, and [optionally] the mean, standard
deviation, and/or the median, L1 scale of *z*, and number of nodes set
to NaN. We also report if the grid is pixel- or gridline-registered and
if it is a Cartesian or Geographic data set (based on metadata in the
file).

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*grdfile*
    The name of one or several 2-D grid files. (See GRID FILE FORMATS
    below.)

`Optional Arguments <#toc5>`_
-----------------------------

**-C**
    Formats the report using tab-separated fields on a single line. The
    output is *w e s n z0 z1 dx dy nx ny*\ [ *x0 y0 x1 y1* ] [ *med
    scale* ] [*mean std rms*\ ] [*n\_nan*\ ]. The data in brackets are
    output only if the corresponding options **-M**, **-L1**, **-L2**,
    and **-M** are used, respectively. If the **-I** option is used, the
    output format is instead *NF w e s n z0 z1*, where *NF* is the total
    number of grids read and *w e s n* are rounded off (see **-I**).
**-F**
    Report grid domain and x/y-increments in world mapping format
    [Default is generic]. Does not apply to the **-C** option.
**-I**\ [*dx*\ [/*dy*]]
    Report the min/max of the region to the nearest multiple of *dx* and
    *dy*, and output this in the form **-R**\ *w/e/s/n* (unless **-C**
    is set). To report the actual grid region, select **-I-**. If no
    argument is given then we report the grid increment in the form
    **-I**\ *xinc/yinc*.
**-L**\ [**0**\ \|\ **1**\ \|)2)]
**-L0**
    Report range of z after actually scanning the data, not just
    reporting what the header says.
**-L1**
    Report median and L1 scale of z (L1 scale = 1.4826 \* Median
    Absolute Deviation (MAD)).
**-L2**
    Report mean, standard deviation, and rms of z.
**-M**
    Find and report the location of min/max z-values, and count and
    report the number of nodes set to NaN, if any.
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. Using the **-R** option will select
    a subsection of the input grid(s). If this subsection exceeds the
    boundaries of the grid, only the common region will be extracted.
**-T**\ *dz*
    Determine min and max z-value, round off to multiples of *dz*, and
    report as the text string **-T**\ *zmin/zmax/dz* for use by
    **makecpt**. To get a symmetrical range about zero, using the max
    absolute multiple of *dz*, use **-Ts**\ *dz* instead.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Grid File Formats <#toc6>`_
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

`Examples <#toc7>`_
-------------------

To obtain all the information about the data set in file
hawaii\_topo.nc:

grdinfo -L1 -L2 -M hawaii\_topo.nc

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*grd2cpt*\ (1) <grd2cpt.1.html>`_ ,
`*grd2xyz*\ (1) <grd2xyz.1.html>`_ , `*grdedit*\ (1) <grdedit.1.html>`_

