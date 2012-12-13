***********
grdgradient
***********

grdgradient - Compute directional derivative or gradient from a grid

`Synopsis <#toc1>`_
-------------------

**grdgradient** *in\_grdfile* **-G**\ *out\_grdfile* [
**-A**\ *azim*\ [/*azim2*] ] [ **-D**\ [**c**\ ][**o**\ ][**n**\ ] ] [
**-E**\ [**s\|p**\ ]\ *azim/elev*\ [/*ambient*/*diffuse*/*specular*/*shine*]
] [ **-L**\ *flag* ] [
**-N**\ [**e**\ ][**t**\ ][*amp*\ ][/\ *sigma*\ [/*offset*]] ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-S**\ *slopefile*
] [ **-V**\ [*level*\ ] ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

`Description <#toc2>`_
----------------------

**grdgradient** may be used to compute the directional derivative in a
given direction (**-A**), or the direction (**-S**) [and the magnitude
(**-D**)] of the vector gradient of the data.

Estimated values in the first/last row/column of output depend on
boundary conditions (see **-L**).

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*in\_grdfile*
    2-D grid file from which to compute directional derivative. (See
    GRID FILE FORMATS below).
**-G**\ *out\_grdfile*
    Name of the output grid file for the directional derivative. (See
    GRID FILE FORMATS below).

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ *azim*\ [/*azim2*]
    Azimuthal direction for a directional derivative; *azim* is the
    angle in the x,y plane measured in degrees positive clockwise from
    north (the +y direction) toward east (the +x direction). The
    negative of the directional derivative, -[dz/dx\*sin(*azim*) +
    dz/dy\*cos(\ *azim*)], is found; negation yields positive values
    when the slope of z(x,y) is downhill in the *azim* direction, the
    correct sense for shading the illumination of an image (see
    **grdimage** and **grdview**) by a light source above the x,y plane
    shining from the *azim* direction. Optionally, supply two azimuths,
    **-A**\ *azim*/*azim2*, in which case the gradients in each of these
    directions are calculated and the one larger in magnitude is
    retained; this is useful for illuminating data with two directions
    of lineated structures, e.g., **-A**\ *0*/*270* illuminates from the
    north (top) and west (left).
**-D**\ [**c**\ ][**o**\ ][**n**\ ]
    Find the direction of the gradient of the data. By default, the
    directions are measured clockwise from north, as *azim* in **-A**
    above. Append **c** to use conventional Cartesian angles measured
    counterclockwise from the positive x (east) direction. Append **o**
    to report orientations (0-180) rather than directions (0-360).
    Append **n** to add 90 degrees to all angles (e.g., to give
    orientation of lineated features).
**-E**\ [**s\|p**\ ]\ *azim/elev*\ [/*ambient*/*diffuse*/*specular*/*shine*]
    Compute Lambertian radiance appropriate to use with **grdimage** and
    **grdview**. The Lambertian Reflection assumes an ideal surface that
    reflects all the light that strikes it and the surface appears
    equally bright from all viewing directions. *azim* and *elev* are
    the azimuth and elevation of light vector. Optionally, supply
    *ambient* *diffuse* *specular* *shine* which are parameters that
    control the reflectance properties of the surface. Default values
    are: *0.55*/ *0.6*/*0.4*/*10* To leave some of the values untouched,
    specify = as the new value. For example **-E**\ *60*/*30*/*=*/*0.5*
    sets the *azim* *elev* and *diffuse* to 60, 30 and 0.5 and leaves
    the other reflectance parameters untouched. Append **s** to use a
    simpler Lambertian algorithm. Note that with this form you only have
    to provide the azimuth and elevation parameters. Append **p** to use
    the Peucker piecewise linear approximation (simpler but faster
    algorithm; in this case the *azim* and *elev* are hardwired to 315
    and 45 degrees. This means that even if you provide other values
    they will be ignored.)
**-L**\ *flag*
    Boundary condition *flag* may be *x* or *y* or *xy* indicating data
    is periodic in range of x or y or both, or *flag* may be *g*
    indicating geographical conditions (x and y are lon and lat).
    [Default uses "natural" conditions (second partial derivative normal
    to edge is zero).]
**-N**\ [**e**\ ][**t**\ ][*amp*\ ][/\ *sigma*\ [/*offset*]]
    Normalization. [Default: no normalization.] The actual gradients *g*
    are offset and scaled to produce normalized gradients *gn* with a
    maximum output magnitude of *amp*. If *amp* is not given, default
    *amp* = 1. If *offset* is not given, it is set to the average of
    *g*. **-N** yields *gn* = *amp* \* (*g* - *offset*)/max(abs(\ *g* -
    *offset*)). **-Ne** normalizes using a cumulative Laplace
    distribution yielding *gn* = *amp* \* (1.0 -
    exp(\ `sqrt(2) <sqrt.2.html>`_ \* (*g* - *offset*)/ *sigma*)) where
    *sigma* is estimated using the L1 norm of (*g* - *offset*) if it is
    not given. **-Nt** normalizes using a cumulative Cauchy distribution
    yielding *gn* = (2 \* *amp* / PI) \* atan( (*g* - *offset*)/
    *sigma*) where *sigma* is estimated using the L2 norm of (*g* -
    *offset*) if it is not given.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. Using the **-R** option will select
    a subsection of *in\_grdfile* grid. If this subsection exceeds the
    boundaries of the grid, only the common region will be extracted.
**-S**\ *slopefile*
    Name of output grid file with scalar magnitudes of gradient vectors.
    Requires **-D** but makes **-G** optional.
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

`Hints <#toc6>`_
----------------

If you don’t know what **-N** options to use to make an intensity file
for **grdimage** or **grdview**, a good first try is **-Ne**\ 0.6.

Usually 255 shades are more than enough for visualization purposes. You
can save 75% disk space by appending =nb/a to the output filename
*out\_grdfile*.

If you want to make several illuminated maps of subregions of a large
data set, and you need the illumination effects to be consistent across
all the maps, use the **-N** option and supply the same value of *sigma*
and *offset* to **grdgradient** for each map. A good guess is *offset* =
0 and *sigma* found by **grdinfo** **-L2** or **-L1** applied to an
unnormalized gradient grd.

If you simply need the *x*- or *y*-derivatives of the grid, use
**grdmath**.

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

`Examples <#toc8>`_
-------------------

To make a file for illuminating the data in geoid.nc using exp-
normalized gradients in the range [-0.6,0.6] imitating light sources in
the north and west directions:

grdgradient geoid.nc -A0/270 -Ggradients.nc=nb/a -Ne0.6 -V

To find the azimuth orientations of seafloor fabric in the file topo.nc:

grdgradient topo.nc -Dno -Gazimuths.nc -V

`References <#toc9>`_
---------------------

Horn, B.K.P., Hill-Shading and the Reflectance Map, Proceedings of the
IEEE, Vol. 69, No. 1, January 1981, pp. 14-47.
(http://people.csail.mit.edu/bkph/papers/Hill-Shading.pdf)

`See Also <#toc10>`_
--------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*grdhisteq*\ (1) <grdhisteq.html>`_ ,
`*grdimage*\ (1) <grdimage.html>`_ , `*grdview*\ (1) <grdview.html>`_ ,
`*grdvector*\ (1) <grdvector.html>`_
