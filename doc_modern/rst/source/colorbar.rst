.. index:: ! colorbar

********
colorbar
********

.. only:: not man

    Plot a gray or color scale-bar on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt colorbar** |-D|\ *refpoint*
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-F|\ *panel* ]
[ |-G|\ *zlo*\ /\ *zhi* ]
[ |-I|\ [*max\_intens*\ \|\ *low_i*/*high_i*] ]
[ |-J|\ *parameters* ]
[ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-L|\ [**i**][*gap*] ]
[ |-M| ]
[ |-N|\ [**p**\ \|\ *dpi* ]]
[ |-Q| ]
[ |SYN_OPT-R| ]
[ |-S| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *scale* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ *zfile* ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**colorbar** plots gray scales or color scales on maps. Both horizontal
and vertical scales are supported. For CPTs with gradational
colors (i.e., the lower and upper boundary of an interval have different
colors) **colorbar** will interpolate to give a continuous scale.
Variations in intensity due to shading/illumination may be displayed by
setting the option **-I**. Colors may be spaced according to a linear
scale, all be equal size, or by providing a file with individual tile
widths. The font used for the annotations along the scale and optional
units is specified by :ref:`FONT_ANNOT_PRIMARY <FONT_ANNOT_PRIMARY>`.
If a label is requested, it is plotted with :ref:`FONT_LABEL <FONT_LABEL>`.

Required Arguments
------------------

.. _-D:

**-D**\ [**g**\ \|\ **j**\ \|\ **J**\ \|\ **n**\ \|\ **x**]\ *refpoint*\ [\ **+w**\ *length*\ [/\ *width*\ ]]\ [**+e**\ [**b**\ \|\ **f**][*length*]][**+h**\ \|\ **v**\ ][**+j**\ *justify*]\ [**+m**\ [**a**\ \|\ **c**\ \|\ **l**\ \|\ **u**]][**+n**\ [*txt*]][**+o**\ *dx*\ [/*dy*]]
    Defines the reference point on the map for the color scale using one of four coordinate systems:
    (1) Use **-Dg** for map (user) coordinates, (2) use **-Dj** or **-DJ** for setting *refpoint* via
    a 2-char justification code that refers to the (invisible) map domain rectangle,
    (3) use **-Dn** for normalized (0-1) coordinates, or (4) use **-Dx** for plot coordinates
    (inches, cm, etc.).  All but **-Dx** requires both **-R** and **-J** to be specified.
    For **-Dj** or **-DJ** with codes TC, BC, ML, MR (i.e., centered on one of the map sides) we
    pre-calculate all further settings.  Specifically, the *length* is set to 80% of the map side,
    horizontal or vertical depends on the side, the offset is MAP_LABEL_OFFSET for **Dj** with an
    extra offset MAP_FRAME_WIDTH for **DJ**, and annotations are placed on the side of the scale facing
    away from the map frame.
    However, you can override any of these with these modifiers:
    Append **+w** followed by the *length* and *width* of the color bar.  If *width* is not
    specified then it is set to 4% of the given *length*.
    Give a negative *length* to reverse the scale bar. Append **+h** to get a
    horizontal scale [Default is vertical (**+v**)].
    By default, the anchor point on the scale is assumed to be the bottom left corner (BL), but this
    can be changed by appending **+j** followed by a 2-char justification code *justify* (see :doc:`text`).
    Note: If **-Dj** is used then *justify* defaults to the same as *refpoint*,
    if **-DJ** is used then *justify* defaults to the mirror opposite of *refpoint*. Consequently,
    **-DJ** is used to place a scale outside the map frame while **-Dj** is used to place it inside the frame.
    Finally, add **+o** to offset the color scale by *dx*/*dy* away from the *refpoint* point in
    the direction implied by *justify* (or the direction implied by **-Dj** or **-DJ**).
    Add sidebar triangles for back- and/or foreground
    colors with **+e**. Append **f** (foreground) or **b** (background) for only one sidebar triangle [Default
    gives both]. Optionally, append triangle height [Default is half the
    barwidth].
    Move text to opposite side with **+m**\ [**a**\ \|\ **c**\ \|\ **l**\ \|\ **u**].
    Horizontal scale bars: Move annotations and labels above the scale bar [Default is below];
    the unit remains on the left.
    Vertical scale bars: Move annotations and labels to the left of the scale bar [Default is to the right];
    the unit remains below.
    Append one or more of **a**, **l** or **u** to control which of the annotations, label, and
    unit that will be moved to the opposite side. Append **c** if you want to print a
    vertical label as a column of characters (does not work with special characters).
    Append **+n** to plot a rectangle with the NaN color at
    the start of the bar, append *text* to change label from NaN.

Optional Arguments
------------------

.. _-B:

**-B**\ [**p**\ \|\ **s**]\ *parameters*
    Set annotation, tick, and gridline interval for the colorbar. The
    x-axis label will plot beneath a horizontal bar (or vertically to
    the right of a vertical bar), except when using the **+m** modifier of the **-D** option. As an
    option, use the y-axis label to plot the data unit to the right of a
    horizontal bar (and above a vertical bar). When using **-Ba** or
    **-Baf** annotation and/or minor tick intervals are chosen
    automatically. If **-B** is omitted, or no annotation intervals are
    provided, the default is to annotate every color level based on the
    numerical entries in the CPT (which may be overridden by ULB
    flags in the CPT). To specify custom text annotations for
    intervals, you must append ;\ *annotation* to each z-slice in the CPT.

.. _-C:

**-C**\ *cpt*
    *cpt* is the CPT to be used. By default all
    color changes are annotated. To use a subset, add an extra column to
    the CPT with a L, U, or B to annotate Lower, Upper, or Both
    color segment boundaries (but see **-B**). If not given, **colorbar**
    will read stdin. Like :doc:`grdview`, **colorbar** can understand
    pattern specifications in the CPT. For CPTs where the
    *z* range is in meters, it may be useful to change to another unit
    when plotting.  To do so, append **+U**\ *unit* to the file name.
    Likewise, if the CPT uses another unit than meter and you wish
    to plot the CPT versus meters, append **+u**\ *unit*.
    If a GMT master dynamic CPT is given instead then its *z*-range will
    be set to its default range (if it has one) before plotting.

.. _-F:

**-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*]]]
    Without further options, draws a rectangular border around the scale using
    **MAP\_FRAME\_PEN**; specify a different pen with **+p**\ *pen*.
    Add **+g**\ *fill* to fill the scale panel [no fill].
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
    [4\ **p**/-4\ **p**] and *shade* sets the fill style to use for shading [gray50].

.. _-G:

**-G**\ *zlo*\ /\ *zhi*
    Truncate the incoming CPT so that the lowest and highest z-levels
    are to *zlo* and *zhi*.  If one of these equal NaN then
    we leave that end of the CPT alone.  The truncation takes place
    before the plotting.

.. _-I:

**-I**\ [*max_intens*\ \|\ *low\_i*/*high_i*]
    Add illumination effects. Optionally, set the range of intensities
    from - to + *max\_intens*. If not specified, 1 is used.
    Alternatively, append *low/high* intensities to specify an
    asymmetric range [Default is no illumination].

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. include:: explain_-Jz.rst_

.. _-L:

**-L**\ [**i**][*gap*]
    Gives equal-sized color rectangles. Default scales rectangles
    according to the z-range in the CPT (Also see **-Z**). If set,
    any equal interval annotation set with **-B** will be ignored. If
    *gap* is appended and the CPT is discrete we will center each
    annotation on each rectangle, using the lower boundary z-value for
    the annotation. If **i** is prepended we annotate the interval range
    instead. If **-I** is used then each rectangle will have its
    constant color modified by the specified intensity.

.. _-M:

**-M**
    Force a monochrome graybar using the (television) YIQ transformation.

.. _-N:

**-N**\ [**p**\ \|\ *dpi*]
    Controls how the color scale should be encoded graphically.
    To preferentially draw color rectangles (e.g., for discrete colors), append **p**.
    Otherwise we will preferentially draw images (e.g., for continuous colors).
    Optionally append effective dots-per-inch for rasterization of color scales [600].

.. _-Q:

**-Q**
    Select logarithmic scale and power of ten annotations. All z-values
    in the CPT will be converted to p = log10(z) and only integer p
    values will be annotated using the 10^p format [Default is linear scale].

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

.. _-S:

**-S**
    Do not separate different color intervals with black grid lines.

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *scale*
    Multiply all *z*-values in the CPT by the provided *scale*.
    By default the CPT is used as is.

.. _-X:

.. include:: explain_-XY.rst_

.. _-Z:

**-Z**\ *zfile*
    File with colorbar-width per color entry. By default, width of entry
    is scaled to color range, i.e., z = 0-100 gives twice the width as z
    = 100-150 (Also see **-L**).

.. |Add_perspective| replace:: (Required **-R** and **-J** for proper functioning).
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

Examples
--------

To plot a a horizontal color scale (12 cm long; 0.5 cm wide) at the reference point (8,1)
(paper coordinates) with justification at top center and automatic annotation interval, do

   ::

    gmt makecpt -T-200/1000/100 -Crainbow > t.cpt
    gmt colorbar -Ct.cpt -Dx8c/1c+w12c/0.5c+jTC+h -Bxaf+l"topography" -By+lkm > map.ps


To append a vertical color scale (7.5 cm long; 1.25 cm wide) to the
right of a plot that is 6 inch wide and 4 inch high, using illumination,
and show back- and foreground colors, and annotating every 5 units, we
provide the reference point and select the left-mid anchor point via

   ::

    gmt colorbar -Dx6.5i+jLM/2i+w7.5c/1.25c+e -O -Ccolors.cpt -I -Bx5+lBATHYMETRY -By+lm >> map.ps

To overlay a horizontal color scale (4 inches long and default width) above a
Mercator map produced by a previous call, ensuring a 2 cm offset from the map frame, use

   ::

    gmt colorbar -DjCT+w4i+o0/2c+h -O -Ccolors.cpt -Baf -R -J >> map.ps

Notes
-----

#. When the CPT is discrete and no illumination is specified, the
   color bar will be painted using polygons. For all other cases we must
   paint with an image. Some color printers may give slightly different
   colors for the two methods given identical RGB values.
#. For cyclic (wrapping) color tables the cyclic symbol is plotted to the right
   of the color bar.  If annotations are specified there then we place the cyclic
   symbol at the left, unless **+n** was used in which case we center of the color bar instead.

See Also
--------

:doc:`gmt`, :doc:`makecpt`
:doc:`gmtlogo`, :doc:`grd2cpt`
:doc:`image`, :doc:`legend`
