.. index:: ! image
.. include:: module_core_purpose.rst_

*****
image
*****

|image_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt image** *imagefile*
[ |SYN_OPT-B| ]
[ |-D|\ *refpoint* ]
[ |-F|\ *box* ]
[ |-G|\ [*color*][**+b**\|\ **f**\|\ **t**] ]
[ |-I| ]
[ |-J|\ *parameters* ]
[ |-J|\ **z**\|\ **Z**\ *parameters* ]
[ |-M| ]
[ |SYN_OPT-Rz| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. module_common_begins

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

Optional Arguments
------------------

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-D:

**-D**\ [**g**\|\ **j**\|\ **J**\|\ **n**\|\ **x**]\ *refpoint*\ **+r**\ *dpi*\ **+w**\ [**-**]\ *width*\ [/*height*]\ [**+j**\ *justify*]\ [**+n**\ *nx*\ [/*ny*] ]\ [**+o**\ *dx*\ [/*dy*]]
    Sets reference point on the map for the image using one of four coordinate systems:

    .. include:: explain_refpoint.rst_

    By default, the anchor point on the scale is assumed to be the bottom left corner (BL), but this
    can be changed by appending **+j** followed by a 2-char justification code *justify* (see :doc:`text`).
    **Note**: If **-Dj** is used then *justify* defaults to the same as *refpoint*,
    if **-DJ** is used then *justify* defaults to the mirror opposite of *refpoint*.
    Specify image size in one of two ways:
    Use **+r**\ *dpi* to set the dpi of the image in dots per inch, or use
    **+w**\ [**-**]\ *width*\ [/*height*] to
    set the width (and height) of the image in plot coordinates
    (inches, cm, etc.). If *height* (or *width*) is set to 0, then the original aspect
    ratio of the image is maintained. If *width* (or *height*) is negative we use the
    absolute value and interpolate image to the device resolution using
    the PostScript image operator. If neither dimensions nor *dpi* are set then we
    revert to the default dpi [:term:`GMT_GRAPHICS_DPU`].  Optionally, use
    **+n**\ *nx*\ [/*ny*] to replicate the image *nx* times horizontally and *ny* times
    vertically. If *ny* is omitted, it will be identical to *nx* [Default is 1/1].

.. _-F:

**-F**\ [**+c**\ *clearances*][**+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][**+p**\ [*pen*]][**+r**\ [*radius*]]\
[**+s**\ [[*dx*/*dy*/][*shade*]]]

    Without further options, draws a rectangular border around the image using :term:`MAP_FRAME_PEN`. The following
    modifiers can be appended to |-F|, with additional explanation and examples provided in the :ref:`Background-panel`
    cookbook section:

    .. include:: explain_-F_box.rst_

.. _-G:

**-G**\ [*color*][**+b**\|\ **f**\|\ **t**]
    Change certain pixel values to another color or make them transparent.
    For 1-bit images you can specify an alternate *color* for the background (**+b**)
    or the foreground (**+f**) pixels, or give no color to make those pixels
    transparent. Alternatively, for color images you can select a single *color*
    that should be made transparent instead  (**+t**). This option may be repeated with different settings.

.. _-I:

**-I**
   Invert 1-bit image before plotting. This is what is done when you
   use **-GP** to invert patterns in other GMT plotting programs.  Ignored
   if used with color images.

.. |Add_-J| replace:: (Used only with **-p**)
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-M:

**-M**
    Convert color image to monochrome grayshades using the (television)
    YIQ-transformation.

.. |Add_-R| replace:: (Used only with **-p**) |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_perspective| replace:: (Requires |-R| and |-J| for proper functioning).
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

Notes
-----

The |-G| and |-I| options are for raster images only. They have
no effect when placing Encapsulated *PostScript* files.

.. module_common_ends

Examples
--------

.. include:: oneliner_info.rst_

To plot the remote image needle.jpg, scaling it be 7 cm wide (height is scaled
accordingly), use::

    gmt image @needle.jpg -Dx0/0+w7c -pdf plot

To plot the same file but reversing the bands, use::

    gmt image @needle.jpg+b2,1,0 -Dx0/0+w7c -pdf plot

To only plot its red band as gray shade, use::

    gmt image @needle.jpg+b0 -Dx0/0+w7c -pdf plot

To include an Encapsulated PostScript file gallo.eps with its upper
right corner 2 inch to the right and 1 inch up from the current
location, and have its width scaled to 3 inches, while keeping the
aspect ratio, use::

    gmt image @gallo.eps -Dx2i/1i+jTR+w3i -pdf image

To replicate the 1-bit remote raster image vader1.png, colorize it
(dark gray background and yellow foreground), and setting each of 6 by 12 tiles
to be 2.5 cm wide, use::

    gmt image @vader1.png -Gdarkgray+b -Gyellow+f -Dx0/0+w2.5c+n6/12 -pdf image

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`, :doc:`gmtlogo`
:doc:`legend`, :doc:`colorbar`
:doc:`plot`,
:doc:`psconvert`
