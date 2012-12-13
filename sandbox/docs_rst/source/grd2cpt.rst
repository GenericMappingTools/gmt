*******
grd2cpt
*******

grd2cpt - Make linear or histogram-equalized color palette table from
grid

`Synopsis <#toc1>`_
-------------------

**grd2cpt** *grid* [ **-A**\ [**+**\ ]\ *transparency* ] [
**-C**\ *cptmaster* ] [ **-D**\ [**i**\ \|\ **o**] ] [ **-E**\ *nlevels*
] [ **-F**\ [**R**\ \|\ **r**\ \|\ **h**\ \|\ **c** ] [ **-I** ] [
**-L**\ *minlimit/maxlimit* ] [ **-M** ] [ **-N** ] [
**-Q**\ [**i**\ \|\ **o**] ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-S**\ *zstart/zstop/zinc* ] [
**-T**\ **-**\ \|\ **+**\ \|\ **\_**\ \|\ **=** ] [ **-V**\ [*level*\ ]
] [ **-W** ] [ **-Z** ]

`Description <#toc2>`_
----------------------

**grd2cpt** reads one or more grid files and writes a color palette
(cpt) file to standard output. The cpt file is based on an existing
master cpt file of your choice, and the mapping from data value to
colors is through the data’s cumulative distribution function (CDF), so
that the colors are histogram equalized. Thus if the grid(s) and the
resulting cpt file are used in **grdimage** with a linear projection,
the colors will be uniformly distributed in area on the plot. Let z be
the data values in the grid. Define CDF(Z) = (# of z < Z) / (# of z in
grid). (NaNs are ignored). These z-values are then normalized to the
master cpt file and colors are sampled at the desired intervals.

The color palette includes three additional colors beyond the range of
z-values. These are the background color (B) assigned to values lower
than the lowest *z*-value, the foreground color (F) assigned to values
higher than the highest *z*-value, and the NaN color (N) painted
whereever values are undefined.

If the master cpt file includes B, F, and N entries, these will be
copied into the new master file. If not, the parameters
**COLOR\_BACKGROUND**, **COLOR\_FOREGROUND**, and **COLOR\_NAN** from
the **gmt.conf** file or the command line will be used. This default
behavior can be overruled using the options **-D**, **-M** or **-N**.

The color model (RGB, HSV or CMYK) of the palette created by **makecpt**
will be the same as specified in the header of the master cpt file. When
there is no **COLOR\_MODEL** entry in the master cpt file, the
**COLOR\_MODEL** specified in the **gmt.conf** file or on the command
line will be used.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*grid*
    Names of one or more grid files used to derive the color palette
    table. All grids need to have the same size and dimensions. (See
    GRID FILE FORMATS below).

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [**+**\ ]\ *transparency*
    Sets a constant level of transparency (0-100) for all color slices.
    Prepend **+** to also affect the fore-, back-, and nan-colors
    [Default is no transparency, i.e., 0 (opaque)].
**-C**\ *cptmaster*
    Selects the master color table to use in the interpolation. Choose
    among the built-in tables (type **grd2cpt** to see the list) or give
    the name of an existing cpt file [Default gives a rainbow cpt file].
**-D**\ [**i**\ \|\ **o**]
    Select the back- and foreground colors to match the colors for
    lowest and highest *z*-values in the output cpt file [Default uses
    the colors specified in the master file, or those defined by the
    parameters **COLOR\_BACKGROUND**, **COLOR\_FOREGROUND**, and
    **COLOR\_NAN**]. Append **i** to match the colors for the lowest and
    highest values in the input (instead of the output) cpt file.
**-E**\ *nlevels*
    Create a linear color table by dividing the grid z-range into
    *nlevels* equidistant slices.
**-F**\ [**R**\ \|\ **r**\ \|\ **h**\ \|\ **c**]
    Force output cpt file to written with r/g/b codes, gray-scale values
    or color name (**R**, default) or r/g/b codes only (**r**), or h-s-v
    codes (**h**), or c/m/y/k codes (**c**).
**-I**
    Reverses the sense of color progression in the master cpt file. Also
    exchanges the foreground and background colors, including those
    specified by the parameters **COLOR\_BACKGROUND** and
    **COLOR\_FOREGROUND**.
**-L**\ *minlimit/maxlimit*
    Limit range of cpt file to *minlimit/maxlimit*, and don’t count data
    outside range when estimating CDF(Z). [Default uses min and max of
    data.]
**-M**
    Overrule background, foreground, and NaN colors specified in the
    master cpt file with the values of the parameters
    **COLOR\_BACKGROUND**, **COLOR\_FOREGROUND**, and **COLOR\_NAN**
    specified in the **gmt.conf** file or on the command line. When
    combined with **-D**, only **COLOR\_NAN** is considered.
**-N**
    Do not write out the background, foreground, and NaN-color fields
    [Default will write them].
**-Q**\ [**i**\ \|\ **o**]
    Selects a logarithmic interpolation scheme [Default is linear].
    **-Qi** expects input z-values to be log10(z), assigns colors, and
    writes out z [Default]. **-Qo** takes log10(z) first, assigns
    colors, and writes out z.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
**-S**\ *zstart/zstop/zinc* or **-S**\ *n*
    Set steps in cpt file. Calculate entries in cpt file from *zstart*
    to *zstop* in steps of (*zinc*). Default chooses arbitrary values by
    a crazy scheme. Use **-S**\ *n* to select *n* points from a
    cumulative normal distribution.
**-T**\ **-**\ \|\ **+**\ \|\ **\_**\ \|\ **=**
    Force the color table to be symmetric about zero (from -R to +R).
    Append flag to set the range R: **-** for R =\|zmin\|, **+** for R =
    \|zmax\|, **\_** for R = min(\|zmin\|, \|zmax\|), or **=** for R =
    max(\|zmin\|, \|zmax\|).
**-V**
    Verbose operation. This will write CDF(Z) estimates to stderr.
    [Default is silent.]
**-W**
    Do not interpolate the input color table but pick the output colors
    starting at the beginning of the map. This is particularly useful in
    combination with a categorical color table. Cannot be used in
    combination with **-Z**.
**-Z**
    Will create a continuous color palette. [Default is discontinuous,
    i.e., constant color intervals]
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

**GMT** is able to recognize many of the commonly used grid file
formats, as well as the precision, scale and offset of the values
contained in the grid file. When **GMT** needs a little help with that,
you can add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.17 of
the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. See `**grdreformat**\ (1) <grdreformat.html>`_ and
Section 4.18 of the GMT Technical Reference and Cookbook for more
information, particularly on how to read splices of 3-, 4-, or
5-dimensional grids.

`Examples <#toc7>`_
-------------------

Sometimes you don’t want to make a cpt file (yet) but would find it
helpful to know that 90% of your data lie between z1 and z2, something
you cannot learn from **grdinfo**. So you can do this to see some points
on the CDF(Z) curve (use **-V** option to see more):

grd2cpt mydata.nc -V > /dev/null

To make a cpt file with entries from 0 to 200 in steps of 20, and ignore
data below zero in computing CDF(Z), and use the built-in master cpt
file relief, run

grd2cpt mydata.nc -Crelief -L0/10000 -S0/200/20 > mydata.cpt

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*grdhisteq*\ (1) <grdhisteq.html>`_ , `*grdinfo*\ (1) <grdinfo.html>`_
, `*makecpt*\ (1) <makecpt.html>`_
