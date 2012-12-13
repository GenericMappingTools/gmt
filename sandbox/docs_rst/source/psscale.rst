*******
psscale
*******

psscale - Plot a gray or color scale-bar on maps

`Synopsis <#toc1>`_
-------------------

**psscale** **-D**\ *xpos*/*ypos*/*length*/*width*\ [**h**\ ] [
**-A**\ [**a**\ \|\ **l**\ \|\ **c**] ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *cpt\_file* ] [
**-E**\ [**b**\ \|\ **f**][*length*\ ][\ **+n**\ [*text*\ ]] ] [
**-I**\ [*max\_intens*\ \|\ *low\_i*/*high\_i*] ] [ **-J**\ *parameters*
] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-L**\ [**i**\ ][*gap*\ ] ] [ **-M** ] [ **-N**\ *dpi* ] [ **-O** ] [
**-P** ] [ **-Q** ] [
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] ] [
**-S** ] [
**-T**\ [**+p**\ *pen*][\ **+g**\ *fill*][\ **+l**\ \|\ **r**\ \|\ **b**\ \|\ **t**\ *off*]
] [ **-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [
**-V**\ [*level*\ ] ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-Z**\ *zfile* ] [ **-c**\ *copies* ] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**psscale** plots gray scales or color scales on maps. Both horizontal
and vertical scales are supported. For cpt\_files with gradational
colors (i.e., the lower and upper boundary of an interval have different
colors) **psscale** will interpolate to give a continuous scale.
Variations in intensity due to shading/illumination may be displayed by
setting the option **-I**. Colors may be spaced according to a linear
scale, all be equal size, or by providing a file with individual tile
widths. The font used for the annotations along the scale and optional
units is specified by **FONT\_ANNOT\_PRIMARY**. If a label is requested,
it is plotted with **FONT\_LABEL**

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-D**\ *xpos*/*ypos*/*length*/*width*\ [**h**\ ]
    Defines the position of the center/top (for horizontal scale) or
    center/left (for vertical scale) and the dimensions of the scale.
    Give a negative length to reverse the scalebar. Append *h* to get a
    horizontal scale [Default is vertical].

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [**a**\ \|\ **l**\ \|\ **c**]
    Place annotations and labels above (instead of below) horizontal
    scalebars and to the left (instead of the right) of vertical
    scalebars. Append **a** or **l** to move only the annotations or the
    label to the other side. Append **c** if you want to print a
    vertical label as a column of characters (does not work with special
    characters).
**-B**\ [**p**\ \|\ **s**]\ *parameters*
    Set annotation, tick, and gridline interval for the colorbar. The
    x-axis label will plot beneath a horizontal bar (or vertically to
    the right of a vertical bar), except when using **-A**. As an
    option, use the y-axis label to plot the data unit to the right of a
    horizontal bar (and above a vertical bar). When using **-Ba** or
    **-Baf** annotation and/or minor tick intervals are chosen
    automatically. If **-B** is omitted, or no annotation intervals are
    provided, the default is to annotate every color level based on the
    numerical entries in the cpt file (which may be overridden by ULB
    flags in the cpt file). To specify custom text annotations for
    intervals, you must append ;\ *annotation* to each z-slice in the
    cpt file.
**-C**\ *cpt\_file*
    *cpt\_file* is the color palette file to be used. By default all
    color changes are annotated. To use a subset, add an extra column to
    the cpt-file with a L, U, or B to annotate Lower, Upper, or Both
    color segment boundaries (but see **-B**). If not given, **psscale**
    will read stdin. Like **grdview**, **psscale** can understand
    pattern specifications in the cpt file.
**-E**\ [**b**\ \|\ **f**][*length*\ ][\ **+n**\ [*text*\ ]]
    Add sidebar triangles for **b**\ ack- and/or **f**\ oreground
    colors. Add **f** or **b** for only one sidebar triangle [Default
    gives both]. Optionally, append triangle height [Default is half the
    barwidth]. Finally, you can plot a rectangle with the NaN color at
    the start of the bar, labeled with *text* [NaN].
**-I**\ [*max\_intens*\ \|\ *low\_i*/*high\_i*]
    Add illumination effects. Optionally, set the range of intensities
    from - to + *max\_intens*. If not specified, 1 is used.
    Alternatively, append *low/high* intensities to specify an
    asymmetric range [Default is no illumination].
**-J**\ *parameters* (\*)
    Select map projection.
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**\ [**i**\ ][*gap*\ ]
    Gives equal-sized color rectangles. Default scales rectangles
    according to the z-range in the cpt-file (Also see **-Z**). If set,
    any equal interval annotation set with **-B** will be ignored. If
    *gap* is appended and the cpt table is discrete we will center each
    annotation on each rectangle, using the lower boundary z-value for
    the annotation. If **i** is prepended we annotate the interval range
    instead. If **-I** is used then each rectangle will have its
    constant color modified by the specified intensity.
**-M**
    Force a monochrome graybar using the (television) YIQ
    transformation.
**-N**\ *dpi*
    Effective dots-per-inch for the rectangular image making up the
    color scale [600].
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**
    Select logarithmic scale and power of ten annotations. All z-values
    in the cpt file will be converted to p = log10(z) and only integer p
    values will be annotated using the 10^p format [Default is linear
    scale].
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
    *west*, *east*, *south*, and *north* specify the region of interest,
    and you may specify them in decimal degrees or in
    [+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left
    and upper right map coordinates are given instead of w/e/s/n. The
    two shorthands **-Rg** and **-Rd** stand for global domain (0/360
    and -180/+180 in longitude respectively, with -90/+90 in latitude).
    Alternatively, specify the name of an existing grid file and the
    **-R** settings (and grid spacing, if applicable) are copied from
    the grid. Using **-R**\ *unit* expects projected (Cartesian)
    coordinates compatible with chosen **-J** and we inversely project
    to determine actual rectangular geographic region.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.
**-S**
    Do not separate different color intervals with black grid lines.
**-T**\ [**+p**\ *pen*][\ **+g**\ *fill*][\ **+l**\ \|\ **r**\ \|\ **b**\ \|\ **t**\ *off*]
    Place a rectangle as background to the color scale. You must specify
    either a pen outline (with modifier **+p**\ *pen*) or a fill (with
    modifier **+g**\ *fill*), or both. The size of the rectangle is
    computed from **-D** and the current fontsize and offset parameters.
    You can add (or remove) additional space on any side by appending
    **+s**\ *off*, where **s** is one of **l**\ eft, **r**\ ight,
    **b**\ ottom, or **t**\ op.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-Z**\ *zfile*
    File with colorbar-width per color entry. By default, width of entry
    is scaled to color range, i.e., z = 0-100 gives twice the width as z
    = 100-150 (Also see **-L**).
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view. (Required **-R** and **-J** for proper
    functioning).
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

`Examples <#toc6>`_
-------------------

To append a vertical color scale (7.5 cm long; 1.25 cm wide) to the
right of a plot that is 6 inch wide and 4 inch high, using illumination,
and show back- and foreground colors, and annotating every 5 units, use

psscale **-D**\ 6.5\ **i**/2**i**/7.5\ **c**/1.25\ **c** -O -Ccolors.cpt
-I -E -B5:BATHYMETRY:/:m: >> map.ps

`Notes <#toc7>`_
----------------

When the cpt file is discrete and no illumination is specified, the
color bar will be painted using polygons. For all other cases we must
paint with an image. Some color printers may give slightly different
colors for the two methods given identical RGB values.

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*makecpt*\ (1) <makecpt.html>`_ ,
`*grd2cpt*\ (1) <grd2cpt.html>`_
