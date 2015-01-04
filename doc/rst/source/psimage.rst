.. index:: ! psimage

*******
psimage
*******

.. only:: not man

    psimage - Place images or EPS files on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psimage** *imagefile* [ **-W**\ [**-**\ ]\ *width*\ [/*height*] \|
**-E**\ *dpi* ] [ **-D**\ [**g**\ \|\ **j**\ \|\ **n**\ \|\ **x**]\ *anchor*\ [/*justify*]\ [/*dx*/*dy*] ]
[ **-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]*pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]] ]
[ **-G**\ [**b**\ \|\ **f**\ \|\ **t**]\ *color* ] [ **-I** ] [
**-J**\ *parameters* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-M** ] [ **-N**\ *nx*\ [/*ny*] ] [ **-O** ] [ **-P** ] [
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**] ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*]
] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**]]
] [ **-c**\ *copies* ] [
**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*] ]

|No-spaces|

Description
-----------

**psimage** reads an Encapsulated PostScript file or a raster image
file and plots it on a map. The image can be scaled arbitrarily, and
1-bit raster images can be (1) inverted, i.e., black
pixels (on) becomes white (off) and vice versa, or
(2) colorized, by assigning different foreground and
background colors, and (3) made transparent where one of
back- or foreground is painted only. As an option, the user may choose
to convert colored raster images to grayscale using TV's
YIQ-transformation. For raster files, the user can select which color to
be made transparent. The user may also choose to replicate the image
which, when preceded by appropriate clip paths, may allow larger
custom-designed fill patterns to be implemented (the **-Gp** mechanism
offered in most GMT programs is limited to rasters smaller than 146
by 146).

Required Arguments
------------------

*imagefile*
    This must be an Encapsulated PostScript (EPS) file or a raster
    image. An EPS file must contain an appropriate BoundingBox. A raster
    file can have a depth of 1, 8, 24, or 32 bits and is read via GDAL.
    Note: If GDAL was not configured during GMT installation then only
    Sun raster files are supported natively.  You must then convert other
    formats to Sun raster files before use.
**-E**\ *dpi*
    Sets the dpi of the image in dots per inch, or use **-W**.
**-W**\ [**-**]\ *width*\ [/*height*]
    Sets the width (and height) of the image in plot coordinates
    (inches, cm, etc.). If *height* is not given, the original aspect
    ratio of the image is maintained. If *width* is negative we use the
    absolute value and interpolate image to the device resolution using
    the PostScript image operator. Alternatively, use **-E**.

Optional Arguments
------------------

**-D**\ [**g**\ \|\ **j**\ \|\ **n**\ \|\ **x**]\ *anchor*\ [/*justify*]\ [/*dx*/*dy*]
    Sets *anchor* position *x0*/*y0* of the image using one of four coordinate systems:
    (1) Use **-Dg** for map (user) coordinates, (2) use **-Dj** for setting *anchor* via
    a 2-char justification code that refers to the (invisible) map domain rectangle,
    (3) use **-Dn** for normalized (0-1) coordinates, or (4) use **-Dx** for plot coordinates
    (inches, cm, etc.).  All but **-Dx** requires both **-R** and **-J** to be specified.
    By default, the *anchor* is assumed to be the lower left corner of the image, but this
    can be changed by specifying a 2-char justification code *justify* (see :doc:`pstext`).
    Finally, you can offset the image by *dx*/*dy* away from the *anchor* point in the
    direction implied by *justify* [LB].

**-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]]
    Without further options, draws a rectangular border around the image using
    **MAP\_FRAME\_PEN**; specify a different pen with **+p**\ *pen*.
    Add **+g**\ *fill* to fill the image box [no fill].
    Append **+c**\ *clearance* where *clearance* is either *gap*, *xgap*\ /\ *ygap*,
    or *lgap*\ /\ *rgap*\ /\ *bgap*\ /\ *tgap* where these items are uniform, separate in
    x- and y-direction, or individual side spacings between scale and border. 
    Append **+i** to draw a secondary, inner border as well. We use a uniform
    *gap* between borders of 2\ **p** and the **MAP\_DEFAULTS\_PEN**
    unless other values are specified. Append **+r** to draw rounded
    rectangular borders instead, with a 6\ **p** corner radius. You can
    override this radius by appending another value. Finally, append
    **+s** to draw an offset background shaded region. Here, *dx*/*dy*
    indicates the shift relative to the foreground frame
    [4\ **p**/-4\ **p**] and *shade* sets the fill style to use for shading.

.. |Add_-J| replace:: (Used only with **-p**)
.. include:: explain_-J.rst_

.. include:: explain_-Jz.rst_

.. include:: explain_-K.rst_

**-M**
    Convert color image to monochrome grayshades using the (television)
    YIQ-transformation.
**-N**\ *nx*\ [/*ny*]
    Replicate the image *nx* times horizontally and *ny* times
    vertically. If *ny* is omitted, it will be identical to *nx*
    [Default is 1/1].

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

.. |Add_-R| replace:: (Used only with **-p**)
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-XY.rst_

.. include:: explain_-c.rst_

These options are for 1-bit images only. They have no effect when
plotting other images or PostScript files.

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
no effect when plotting 1-bit images or PostScript files.

**-Gt**
    Assigns the color that is to be made transparent. Sun Raster files
    do not support transparency, so indicate here which color to be made
    transparent.

.. |Add_perspective| replace:: (Requires **-R** and **-J** for proper functioning).
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

:doc:`gmt`, :doc:`pslegend`,
To plot the image logo.jpg, scaling it be 1 inch wide (height is scaled
accordingly), and outline with a thin, blue pen, use

   ::

    gmt psimage logo.jpg -W1i -F+pthin,blue > image.ps

To include an Encapsulated PostScript file tiger.eps with its upper
right corner 2 inch to the right and 1 inch up from the current
location, and have its width scaled to 3 inches, while keeping the
aspect ratio, use

   ::

    gmt psimage tiger.eps -Dx2i/1i/TR -W3i > image.ps

To replicate the 1-bit raster image template 1_bit.ras, colorize it
(brown background and red foreground), and setting each of 5 by 5 tiles
to be 1 cm wide, use

   ::

    gmt psimage 1_bit.ras -Gbbrown -Gfred -N5 -W1c > image.ps

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`, :doc:`gmtlogo`
:doc:`pslegend`, :doc:`psscale`
:doc:`psxy`,
:manpage:`convert(1)`
