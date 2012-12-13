*********
grdvector
*********

grdvector - Plot vector field from two component grids

`Synopsis <#toc1>`_
-------------------

**grdvector** *compx.nc* *compy.nc* **-J**\ *parameters* [ **-A** ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *cptfile* ] [
**-G**\ *fill* ] [
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
] [ **-K** ] [ **-N** ] [ **-O** ] [ **-P** ] [ **-Q**\ *parameters* ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-S**\ [**l**\ ]\ *scale* ] [ **-T** ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [ **-W**\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-Z** ] [ **-c**\ *copies* ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**grdvector** reads two 2-D grid files which represents the x- and
y-components of a vector field and produces a vector field plot by
drawing vectors with orientation and length according to the information
in the files. Alternatively, polar coordinate components may be used (r,
theta). **grdvector** is basically a short-hand for using 2 calls to
**grd2xyz** and pasting the output through **psxy** **-SV**.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*compx.nc*
    Contains the x-component of the vector field.
*compy.nc*
    Contains the y-component of the vector field. (See GRID FILE FORMATS
    below.)
**-J**\ *parameters* (\*)
    Select map projection.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**
    Means grid files have polar (r, theta) components instead of
    Cartesian (x, y).
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-C**\ *cptfile*
    Use *cptfile* to assign colors based on vector length.
**-G**\ *fill*
    Sets color or shade for vector interiors [Default is no fill].
**-I**
    Only plot vectors at nodes every *x\_inc*, *y\_inc* apart (must be
    multiples of original grid spacing). Append **m** for arc minutes or
    **s** for arc seconds. [Default plots every node].
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-N**
    Do NOT clip vectors at map boundaries [Default will clip].
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**\ *parameters*
    Modify vector parameters. For vector heads, append vector head
    *size* [Default is 0, i.e., stick-plot]. See VECTOR ATTRIBUTES for
    specifying additional attributes.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. Specify a subset of the grid.
**-S**\ [**l**\ ]\ *scale*
    Sets scale for vector length in data units per distance measurement
    unit [1]. Append **c**, **i**, or **p** to indicate the measurement
    unit (cm, inch,or point). Prepend **l** to indicate a fixed length
    for all vectors.
**-T**
    Means azimuth should be converted to angles based on the selected
    map projection.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**\ *pen*
    Set pen attributes used for vector outlines [Default: width =
    default, color = black, style = solid].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-Z**
    Means the angles provided are azimuths rather than direction
    (requires **-A**).
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
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

`Vector Attributes <#toc7>`_
----------------------------

Several modifiers may be appended to the vector-producing options to
specify the placement of vector heads, their shapes, and the
justification of the vector:

**+a**\ *angle* sets the angle of the vector head apex [30].

**+b** places a vector head at the beginning of the vector path [none].

**+e** places a vector head at the end of the vector path [none].

**+g**-\|\ *fill* turns off vector head fill (if -) or sets the vector
head fill [Default fill is used, which may be no fill].

**+l** draws half-arrows, using only the left side [both].

**+n**\ *norm* scales down vector attributes (pen thickness, head size)
with decreasing length, where vectors shorter than *norm* will have
their attributes scaled by length/\ *norm* [arrow attributes remains
invariant to length].

**+p**\ [-][*pen*\ ] sets the vector pen attributes. If *pen* has a
leading - then the head outline is not drawn. [Default pen is used, and
head outline is drawn]

**+r** draws half-arrows, using only the right side [both].

In addition, all but circular vectors may take these modifiers:

**+j**\ *just* determines how the input *x*,\ *y* point relates to the
vector. Choose from **b**\ eginning [default], **e**\ nd, or
**c**\ enter.

**+s** means the input *angle*, *length* is instead the *x*, *y*
coordinates of the vector end point.

`Examples <#toc8>`_
-------------------

To draw the vector field given by the files r.nc and theta.nc on a
linear plot with scale 5 cm per data unit, using vector rather than
stick plot, scale vector magnitudes so that 10 units equal 1 inch, and
center vectors on the node locations, run

grdvector r.nc theta.nc **-Jx**\ 5\ **c** -A -Q0.1i+e+jc
**-S**\ 10\ **i** > gradient.ps

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*grdcontour*\ (1) <grdcontour.html>`_ , `*psxy*\ (1) <psxy.html>`_
