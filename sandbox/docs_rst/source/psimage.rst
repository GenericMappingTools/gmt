*******
psimage
*******


psimage - Place images or EPS files on maps

`Synopsis <#toc1>`_
-------------------

**psimage** *imagefile* [ **-W**\ [**-**\ ]\ *width*\ [/*height*] \|
**-E**\ *dpi* ] [ **-C**\ *xpos*/*ypos*\ [/*justify*] ] [ **-F**\ *pen*
] [ **-G**\ [**b**\ \|\ **f**\ \|\ **t**]\ *color* ] [ **-I** ] [
**-J**\ *parameters* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-M** ] [ **-N**\ *nx*\ [/*ny*] ] [ **-O** ] [ **-P** ] [
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-c**\ *copies* ] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**psimage** reads an Encapsulated *PostScript* file or a raster image
file and plots it on a map. The image can be scaled arbitrarily, and
1-bit raster images can `be (1) <be.1.html>`_ inverted, i.e., black
`pixels (on) <pixels.on.html>`_ becomes white (off) and vice versa, `or
(2) <or.2.html>`_ colorized, by assigning different foreground and
background colors, `and (3) <and.3.html>`_ made transparent where one of
back- or foreground is painted only. As an option, the user may choose
to convert colored raster images to grayscale using TV’s
YIQ-transformation. For raster files, the user can select which color to
be made transparent. The user may also choose to replicate the image
which, when preceded by appropriate clip paths, may allow larger
custom-designed fill patterns to be implemented (the **-Gp** mechanism
offered in most **GMT** programs is limited to rasters smaller than 146
by 146).

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*imagefile*
    This must be an Encapsulated *PostScript* (EPS) file or a raster
    image. An EPS file must contain an appropriate BoundingBox. A raster
    file can have a depth of 1, 8, 24, or 32 bits. Only Sun raster files
    are supported natively; other raster formats are automatically
    converted to the Sun format via ImageMagick’s **convert** program,
    if installed.
**-E**\ *dpi*
    Sets the dpi of the image in dots per inch, or use **-W**.
**-W**\ [**-**\ ]\ *width*\ [/*height*]
    Sets the width (and height) of the image in plot coordinates
    (inches, cm, etc.). If *height* is not given, the original aspect
    ratio of the image is maintained. If *width* is negative we use the
    absolute value and interpolate image to the device resolution using
    the *PostScript* image operator. Alternatively, use **-E**.

`Optional Arguments <#toc5>`_
-----------------------------

**-C**\ *xpos*/*ypos*\ [/*justify*]
    Sets position of the image in plot coordinates (inches, cm, etc.)
    from the current origin of the plot. By default, this defines the
    position of the lower left corner of the image, but this can be
    changed by specifying justification [0/0/BL].
**-F**\ *pen*
    Draws a rectangular frame around the image with the given pen [no frame].
**-J**\ *parameters* (\*)
    Select map projection.
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-M**
    Convert color image to monochrome grayshades using the (television)
    YIQ-transformation.
**-N**\ *nx*\ [/*ny*]
    Replicate the image *nx* times horizontally and *ny* times
    vertically. If *ny* is omitted, it will be identical to *nx*
    [Default is 1/1].
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
    *west*, *east*, *south*, and *north* specify the region of interest,
    and you may specify them in decimal degrees or in
    [+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left
    and upper right map coordinates are given instead of w/e/s/n. The
    two shorthands **-Rg** and **-Rd** stand for global domain (0/360
    and -180/+180 in longitude respectively, with -90/+90 in latitude).
    Alternatively, specify the name of an existing grid file and the
    **-R** settings (and grid spacing, if applicable) are copied from
    the grid.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]] (\*)
    Shift plot origin.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*] (\*)
    Select perspective view.

These options are for 1-bit images only. They have no effect when
plotting other images or *PostScript* files.

**-G**\ [**b**\ \|\ **f**\ \|\ **t**]\ *color*
**-Gb**
    Sets background color (replace white pixel) of 1-bit images. Use -
    for transparency (and set **-Gf** to the desired color).
**-Gf**
    Sets foreground color (replace black pixel) of 1-bit images. Use -
    for transparency (and set **-Gb** to the desired color).
**-I**
    Invert 1-bit image before plotting. This is what is done when you
    use **-GP** in other GMT programs.

These options are for 8-, 24-, and 32-bit raster images only. They have
no effect when plotting 1-bit images or *PostScript* files.

**-Gt**
    Assigns the color that is to be made transparent. Sun Raster files
    do not support transparency, so indicate here which color to be made
    transparent.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*] (\*)
    Select perspective view. (Requires **-R** and **-J** for proper
    functioning).
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Examples <#toc6>`_
-------------------

To plot the image contained in the 8-bit raster file scanned\_face.ras,
scaling it to 8 by 10 cm (thereby possibly changing the aspect ratio),
and making the white color transparent, use

psimage scanned\_face.ras -W8c/10c -Gtwhite > image.ps

To plot the image logo.jpg, scaling it be 1 inch wide (height is scaled
accordingly), and outline with a thin, blue pen, use

psimage logo.jpg -W1i -Fthin,blue > image.ps

To include an Encapsulated *PostScript* file tiger.eps with its upper
right corner 2 inch to the right and 1 inch up from the current
location, and have its width scaled to 3 inches, while keeping the
aspect ratio, use

psimage tiger.eps -C2i/1i/TR -W3i > image.ps

To replicate the 1-bit raster image template 1\_bit.ras, colorize it
(brown background and red foreground), and setting each of 5 by 5 tiles
to be 1 cm wide, use

psimage 1\_bit.ras -Gbbrown -Gfred -N5 -W1c > image.ps

`See Also <#toc7>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*gmtcolors*\ (5) <gmtcolors.5.html>`_ ,
`*psxy*\ (1) <psxy.1.html>`_ `*convert*\ (1) <convert.1.html>`_

