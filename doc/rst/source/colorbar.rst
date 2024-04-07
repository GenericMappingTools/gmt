.. index:: ! colorbar
.. include:: module_core_purpose.rst_

********
colorbar
********

|colorbar_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt colorbar**
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *refpoint* ]
[ |-F|\ *panel* ]
[ |-G|\ *zlo*\ /\ *zhi* ]
[ |-I|\ [*max\_intens*\|\ *low_i*/*high_i*] ]
[ |-J|\ *parameters* ]
[ |-J|\ **z**\|\ **Z**\ *parameters* ]
[ |-L|\ [**i**\|\ **I**][*gap*] ]
[ |-M| ]
[ |-N|\ [**p**\|\ *dpi* ]]
[ |-Q| ]
[ |SYN_OPT-R| ]
[ |-S|\ [**+a**\ *angle*][**+c**\|\ **n**\ ][**+r**][**+s**][**+x**\ *label*][**+y**\ *unit*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *scale* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ *widthfile* ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Plots gray scales or color scales on maps. Both horizontal
and vertical scales are supported. For color palette tables (CPTs) with gradational
colors (i.e., the lower and upper boundary of an interval have different
colors) we will interpolate to give a continuous scale.
Variations in intensity due to shading/illumination may be displayed by
setting the option |-I|. Colors may be spaced according to a linear
scale, all be equal size, or by providing a file with individual tile
widths. The font used for the annotations along the scale is specified by
:term:`FONT_ANNOT_PRIMARY` while any unit placed at the side of the
bar is controlled by :term:`FONT_ANNOT_SECONDARY`. If a label along the bar is requested, it is plotted with
:term:`FONT_LABEL`. But mind you that the modern mode theme scales down the size of annotations and labels
based on the colorbar length for improved legibility. So, in modern mode scripts it might look that these
parameter settings are not working correctly. If you may want to increase font sizes to compensate the
scaling effect the solution is to use the online ``--FONT_ANNOT_PRIMARY=...`` construct.
For a full overview of CPTs, see the Technical Reference section on :ref:`Color palette tables <CPT_section>`.

.. figure:: /_images/GMT_colorbar.*
   :width: 500 px
   :align: center

   Example of a horizontal color bar placed below a geographic map.

Required Arguments
------------------

None.

Optional Arguments
------------------

.. _-B:

**-B**\ [**p**\|\ **s**]\ *parameters*
    Set annotation, tick, and gridline interval for the bar. The
    x-axis label will plot beneath a horizontal bar (or vertically to
    the right of a vertical bar), except when using the **+m** modifier of the |-D| option. As an
    option, use the y-axis label to plot the data unit to the right of a
    horizontal bar (and above a vertical bar). If |-B| is omitted, or no annotation intervals are
    provided (classic mode only), the default is to annotate every color level based on the
    numerical entries in the CPT (which may be overridden by ULB
    flags in the CPT). The exception to this rule is for CPT files that were scaled to fit the range
    of a grid exactly and thus have arbitrary color levels; these will trigger an automatic **-Baf** setting.
    To specify custom text annotations for
    intervals, you must append ;\ *annotation* to each z-slice in the CPT. **Note**: The |-B|
    option relies on the |-R| and |-J| settings of the :ref:`given hierarchical level <reference/features:GMT Modern Mode Hierarchical Levels>`
    to plot correctly. For standard |-B| operations, |Add_-B_links|

.. _-C:

**-C**\ [*cpt*]
    *cpt* is the CPT to be used. If no *cpt* is appended or no |-C| is given
    then we use the current CPT (modern mode only).  In classic mode, if no |-C|
    is given then we read standard input.  By default all
    color changes are annotated. To use a subset, add an extra column to
    the CPT with a L, U, or B to annotate Lower, Upper, or Both
    color segment boundaries (but see |-B|). Like :doc:`grdview`, we can understand
    pattern specifications in the CPT. For CPTs where the
    *z* range is in meters, it may be useful to change to another unit
    when plotting.  To do so, append **+U**\ *unit* to the file name.
    Likewise, if the CPT uses another unit than meter and you wish
    to plot the CPT versus meters, append **+u**\ *unit*.
    If a GMT master dynamic CPT is given instead then its *z*-range will
    be set to its default range (if it has one) before plotting.

.. _-D:

**-D**\ [**g**\|\ **j**\|\ **J**\|\ **n**\|\ **x**]\ *refpoint*\ [**+w**\ *length*\ [/\ *width*]]\ [**+e**\ [**b**\|\ **f**][*length*]][**+h**\|\ **v**][**+j**\ *justify*]\ [**+m**\ [**a**\|\ **c**\|\ **l**\|\ **u**]][**+n**\ [*txt*]][**+o**\ *dx*\ [/*dy*]][**+r**]
    Defines the reference point on the map for the color scale using one of four coordinate systems:

    .. include:: explain_refpoint.rst_

    For **-Dj** or **-DJ** with codes TC, BC, ML, MR (i.e., centered on one of the map sides) we
    pre-calculate all further settings.  Specifically, the *length* is set to 80% of the map side,
    horizontal or vertical depends on the side, the offset is :term:`MAP_LABEL_OFFSET` for **Dj** with an
    extra offset :term:`MAP_FRAME_WIDTH` for **DJ**, and annotations are placed on the side of the scale facing
    away from the map frame.  If not given, the default argument is JBC (Place color bar centered beneath current plot).
    However, you can override any of these with these modifiers:

  - **+w** followed by the *length* and *width* of the color bar sets bar dimensions.  If *width* is not
    specified then it is set to 4% of the given *length*. If *length* is not given then it defaults
    to 80% of the corresponding map side dimension.  If either *length* or *width* end with % then
    those percentages are used instead to set the dimensions, where *width* is defined as a
    percentage of the bar *length*.
  - **+e** adds sidebar triangles for back- and/or foreground colors. Append **f** (foreground) or **b** 
    (background) for only one sidebar triangle [Default gives both]. Optionally, append triangle height [Default is half the barwidth].
    The back and/or foreground colors are taken from your **B** and **F** colors in your CPT.  If none then the system default
    colors for **B** and **F** are used instead (:term:`COLOR_BACKGROUND` and :term:`COLOR_FOREGROUND`).
  - **+h** selects a horizontal scale [Default is vertical (**+v**)].
  - **+j** sets the anchor point. By default, the anchor point on the scale is assumed to be the bottom left corner (BL),
    but this can be changed by appending **+j** followed by a 2-char justification code *justify* (see :doc:`text`).
    **Note**: If **-Dj** is used then *justify* defaults to the same as *refpoint*,
    if **-DJ** is used then *justify* defaults to the mirror opposite of *refpoint*. Consequently,
    **-DJ** is used to place a scale outside the map frame while **-Dj** is used to place it inside the frame.
  - **+m** will move text to opposite side as per arguments [**a**\|\ **c**\|\ **l**\|\ **u**].
    Horizontal scale bars: Move annotations and labels above the scale bar [Default is below];
    the unit remains on the left. Vertical scale bars: Move annotations and labels to the left of the scale bar [Default is to the right];
    the unit remains below. Append one or more of **a**, **l** or **u** to control which of the annotations, label, and
    unit that will be moved to the opposite side. Append **c** if you want to print a
    vertical label as a column of characters (does not work with special characters).
  - **+n** plots a rectangle with the NaN color (via the **N** entry in your cpt (or :term:`COLOR_NAN` if no such entry)
    at the start of the bar, append *text* to change label from NaN. To place it at the end of the bar, use **+N** instead.
  - **+r** will reverse the positive direction of the bar.

.. _-F:

**-F**\ [**+c**\ *clearances*][**+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][**+p**\ [*pen*]][**+r**\ [*radius*]][**+s**\ [[*dx*/*dy*/][*shade*]]]
    Without further options, draws a rectangular border around the bar using :term:`MAP_FRAME_PEN`. The following
    modifiers can be appended to |-F|, with additional explanation and examples provided in the :ref:`Background-panel`
    cookbook section:

    .. include:: explain_-F_box.rst_

.. _-G:

**-G**\ *zlo*\ /\ *zhi*
    Truncate the incoming CPT so that the lowest and highest z-levels
    are to *zlo* and *zhi*.  If one of these equal NaN then
    we leave that end of the CPT alone.  The truncation takes place
    before the plotting.

.. _-I:

**-I**\ [*max_intens*\|\ *low\_i*/*high_i*]
    Add illumination effects. Optionally, set the range of intensities
    from -*max\_intens* to +\ *max\_intens*. If not specified, 1 is used.
    Alternatively, append *low/high* intensities to specify an
    asymmetric range [Default is no illumination].

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-L:

**-L**\ [**i**\|\ **I**][*gap*]

    Gives equal-sized color rectangles. Default scales rectangles
    according to the z-range in the CPT (Also see |-Z|). If
    *gap* is appended and the CPT is discrete we will center each
    annotation on each rectangle, using the lower boundary z-value for
    the annotation. If **i** is prepended we annotate the interval range
    instead, and if **I** is used instead then we include the background
    and foreground values in the label (e.g, "< 12"). If |-I| is used
    then each rectangle will have its constant color modified by the
    specified intensity.  **Note**: For categorical CPTs we default to
    activating |-L| with a *gap* such that the sum of all the gaps equal
    15% of the bar width.  You may chose no gaps by giving |-L| only or
    explicitly set *gap = 0*.

.. _-M:

**-M**
    Force a monochrome graybar using the (television) YIQ transformation.

.. _-N:

**-N**\ [**p**\|\ *dpi*]
    Controls how the color scale should be encoded graphically.
    To preferentially draw color rectangles (e.g., for discrete colors), append **p**.
    Otherwise we will preferentially draw images (e.g., for continuous colors).
    Optionally append effective dots-per-inch for rasterization of color scales [600].

.. _-Q:

**-Q**
    Select logarithmic scale and power of ten annotations. All z-values
    in the CPT will be converted to p = :math:`\log_{10}(z)` and only integer p
    values will be annotated using the 10^p format [Default is linear scale].

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

.. _-S:

**-S**\ [**+a**\ *angle*][**+c**\|\ **n**\ ][**+r**][**+s**][**+x**\ *label*][**+y**\ *unit*]
    Control various aspects of color bar appearance when |-B| is *not* used (**Note**: If |-L|
    is used then |-B| cannot be used). The following modifiers are available:

    - **+a** Place annotations at the given *angle* [default is no slanting].
    - **+c** Use custom labels if given in the CPT as annotations.
    - **+n** Use numerical labels instead [Default]. Only one of **+c** and **+n** can be set.
    - **+r** Only annotate lower and upper limits in the CPT [Default follows CPT boundaries].
    - **+s** Skip drawing gridlines separating different color intervals [Default draws gridlines].
    - **+x** Place a bar label via **+x**\ *label*.
    - **+y** Place a bar unit via **+y**\ *unit*.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *scale*
    Multiply all *z*-values in the CPT by the provided *scale*.
    By default the CPT is used as is.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ *widthfile*
    File with bar-width per color entry. By default, width of entry
    is scaled to color range, i.e., z = 0-100 gives twice the width as z
    = 100-150 (Also see |-L|). **Note**: The widths may be in plot distance
    units or given as relative fractions and will be automatically scaled
    so that the sum of the widths equals the requested bar length.

.. |Add_perspective| replace:: (Required |-R| and |-J| for proper functioning).
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: macos_preview_issue.rst_

.. module_common_ends

Examples
--------

.. include:: oneliner_info.rst_

To plot a horizontal color scale (12 cm long; 0.5 cm wide) at the reference point (8,1)
(paper coordinates) with justification at top center and automatic annotation interval, do

::

  gmt begin map
    gmt makecpt -T-200/1000/100 -Crainbow
    gmt colorbar -Dx8c/1c+w12c/0.5c+jTC+h -Bxaf+l"topography" -By+lkm
  gmt end show


To append a vertical color scale (7.5 cm long; 1.25 cm wide) to the
right of a plot that is 6 inch wide and 4 inch high, using illumination,
and show back- and foreground colors, and annotating every 5 units, we
provide the reference point and select the left-mid anchor point via

::

  gmt colorbar -Dx6.5i+jLM/2i+w7.5c/1.25c+e -Ccolors.cpt -I -Bx5+lBATHYMETRY -By+lm

To overlay a horizontal color scale (4 inches long and default width) above a
Mercator map produced by a previous call, ensuring a 2 cm offset from the map frame, use

::

  gmt colorbar -DjCT+w4i+o0/2c+h -Ccolors.cpt -Baf

.. module_note_begins

Notes
-----

#. When the CPT is discrete and no illumination is specified, the
   color bar will be painted using polygons. For all other cases we must
   paint with an image. Some color printers may give slightly different
   colors for the two methods given identical RGB values.  See option |-N|
   for affecting these decisions.  Also note that for years now, Apple's
   Preview insists on smoothing deliberately course CPT color images to a blur.
   Use another PDF viewer if this bothers you.
#. For cyclic (wrapping) color tables the cyclic symbol is plotted to the right
   of the color bar.  If annotations are specified there then we place the cyclic
   symbol at the left, unless **+n** or **+N** were used in which case we center of the color bar instead.
#. Discrete CPTs may have transparency applied to all or some individual slices.
   Continuous CPTs may have transparency applied to all slices, but not just some.

.. module_note_ends

See Also
--------

:doc:`gmt`, :doc:`makecpt`
:doc:`gmtlogo`, :doc:`grd2cpt`
:doc:`image`, :doc:`legend`
