.. index:: ! image

*****
image
*****

.. only:: not man

    Place images or EPS files on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt image** *imagefile*
[ |SYN_OPT-B| ]
[ |-D|\ *refpoint* ]
[ |-F|\ *box* ]
[ |-G|\ [*color*\ ][**+b**\ \|\ **+f**\ \|\ **+t**] ]
[ |-I| ]
[ |-J|\ *parameters* ]
[ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-M| ]
[ |SYN_OPT-Rz| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**image** reads an Encapsulated PostScript file or a raster image
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
    EPS files are supported.

Optional Arguments
------------------

.. _-D:

**-D**\ [**g**\ \|\ **j**\ \|\ **J**\ \|\ **n**\ \|\ **x**]\ *refpoint*\ **+r**\ *dpi*\ **+w**\ [**-**]\ *width*\ [/*height*]\ [**+j**\ *justify*]\ [**+n**\ *nx*\ [/*ny*] ]\ [**+o**\ *dx*\ [/*dy*]]
    Sets reference point on the map for the image using one of four coordinate systems:
    (1) Use **-Dg** for map (user) coordinates, (2) use **-Dj** or **-DJ** for setting *refpoint* via
    a 2-char justification code that refers to the (invisible) map domain rectangle,
    (3) use **-Dn** for normalized (0-1) coordinates, or (4) use **-Dx** for plot coordinates
    (inches, cm, etc.).  All but **-Dx** requires both **-R** and **-J** to be specified.
    By default, the anchor point on the scale is assumed to be the bottom left corner (BL), but this
    can be changed by appending **+j** followed by a 2-char justification code *justify* (see :doc:`text`).
    Note: If **-Dj** is used then *justify* defaults to the same as *refpoint*,
    if **-DJ** is used then *justify* defaults to the mirror opposite of *refpoint*.
    Add **+o** to offset the color scale by *dx*/*dy* away from the *refpoint* point in
    the direction implied by *justify* (or the direction implied by **-Dj** or **-DJ**).
    Specify image size in one of two ways:
    Use **+r**\ *dpi* to set the dpi of the image in dots per inch, or use
    **+w**\ [**-**]\ *width*\ [/*height*] to
    set the width (and height) of the image in plot coordinates
    (inches, cm, etc.). If *height* is not given, the original aspect
    ratio of the image is maintained. If *width* is negative we use the
    absolute value and interpolate image to the device resolution using
    the PostScript image operator. Optionally, use **+n**\ *nx*\ [/*ny*] to
    replicate the image *nx* times horizontally and *ny* times
    vertically. If *ny* is omitted, it will be identical to *nx* [Default is 1/1].

.. _-F:

**-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*]][\ **+s**\ [[*dx*/*dy*/][*shade*]]]
    Without further options, draws a rectangular border around the image using
    **MAP_FRAME_PEN**; specify a different pen with **+p**\ *pen*.
    Add **+g**\ *fill* to fill the image box [no fill].
    Append **+c**\ *clearance* where *clearance* is either *gap*, *xgap*\ /\ *ygap*,
    or *lgap*\ /\ *rgap*\ /\ *bgap*\ /\ *tgap* where these items are uniform, separate in
    x- and y-direction, or individual side spacings between scale and border.
    Append **+i** to draw a secondary, inner border as well. We use a uniform
    *gap* between borders of 2\ **p** and the **MAP_DEFAULTS_PEN**
    unless other values are specified. Append **+r** to draw rounded
    rectangular borders instead, with a 6\ **p** corner radius. You can
    override this radius by appending another value. Finally, append
    **+s** to draw an offset background shaded region. Here, *dx*/*dy*
    indicates the shift relative to the foreground frame
    [4\ **p**/-4\ **p**] and *shade* sets the fill style to use for shading [gray50].

.. _-G:

**-G**\ [*color*\ ][**+b**\ \|\ **+f**\ \|\ **+t**]
    Change certain pixel values to another color or make them transparent.
    For 1-bit images you can specify an alternate *color* for the background (**+b**)
    or the foreground (**+f**) pixels, or give no color to make those pixels
    transparent.  Alternatively, for color images you can select a single *color*
    that should be made transparent instead.

.. _-I:

**-I**
   Invert 1-bit image before plotting. This is what is done when you
   use **-GP** to invert patterns in other GMT plotting programs.  Ignored
   if used with color images.

.. _-J:

.. |Add_-J| replace:: (Used only with **-p**)
.. include:: explain_-J.rst_

.. include:: explain_-Jz.rst_

.. _-M:

**-M**
    Convert color image to monochrome grayshades using the (television)
    YIQ-transformation.

.. _-R:

.. |Add_-R| replace:: (Used only with **-p**)
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-X:

.. include:: explain_-XY.rst_

.. |Add_perspective| replace:: (Requires **-R** and **-J** for proper functioning).
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

Notes
-----

The **-G** and **-I** options are for raster images only. They have
no effect when placing Encapsulated *PostScript* files.

Examples
--------

To plot the image logo.jpg, scaling it be 1 inch wide (height is scaled
accordingly), and outline with a thin, blue pen, use

   ::

    gmt image logo.jpg -Dx0/0+w1i -F+pthin,blue -pdf image

To include an Encapsulated PostScript file tiger.eps with its upper
right corner 2 inch to the right and 1 inch up from the current
location, and have its width scaled to 3 inches, while keeping the
aspect ratio, use

   ::

    gmt image tiger.eps -Dx2i/1i+jTR+w3i -pdf image

To replicate the 1-bit raster image template 1_bit.ras, colorize it
(brown background and red foreground), and setting each of 5 by 5 tiles
to be 1 cm wide, use

   ::

    gmt image 1_bit.ras -Gbrown+b -Gred+f -Dx0/0+w1c+n5 -pdf image

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`, :doc:`gmtlogo`
:doc:`legend`, :doc:`colorbar`
:doc:`plot`,
:manpage:`psconvert(1)`
