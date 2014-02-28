.. index:: ! psscale

*******
psscale
*******

.. only:: not man

    psscale - Plot a gray or color scale-bar on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psscale** **-D**\ *xpos*/*ypos*/*length*/*width*\ [**h**]
[ **-A**\ [**a**\ \|\ **l**\ \|\ **c**] ]
[ |SYN_OPT-B| ]
[ **-C**\ *cpt\_file* ]
[ **-E**\ [**b**\ \|\ **f**][*length*][\ **+n**\ [*text*]] ]
[ **-G**\ *zlo*\ /\ *zhi* ]
[ **-I**\ [*max\_intens*\ \|\ *low\_i*/*high\_i*] ]
[ **-J**\ *parameters* ]
[ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ]
[ **-L**\ [**i**][*gap*] ] [ **-M** ] [ **-N**\ [**p**\ \|\ *dpi* ]] [ **-O** ]
[ **-P** ] [ **-Q** ]
[ |SYN_OPT-R| ]
[ **-S** ]
[ **-T**\ [**+p**\ *pen*][\ **+g**\ *fill*][\ **+l**\ \|\ **r**\ \|\ **b**\ \|\ **t**\ *off*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ **-Z**\ *zfile* ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]

|No-spaces|

Description
-----------

**psscale** plots gray scales or color scales on maps. Both horizontal
and vertical scales are supported. For cpt_files with gradational
colors (i.e., the lower and upper boundary of an interval have different
colors) **psscale** will interpolate to give a continuous scale.
Variations in intensity due to shading/illumination may be displayed by
setting the option **-I**. Colors may be spaced according to a linear
scale, all be equal size, or by providing a file with individual tile
widths. The font used for the annotations along the scale and optional
units is specified by :ref:`FONT_ANNOT_PRIMARY <FONT_ANNOT_PRIMARY>`. If a label is requested,
it is plotted with **FONT_LABEL** 

Required Arguments
------------------

**-D**\ *xpos*/*ypos*/*length*/*width*\ [**h**]
    Defines the position of the center/top (for horizontal scale) or
    center/left (for vertical scale) and the dimensions of the scale.
    Give a negative length to reverse the scalebar. Append *h* to get a
    horizontal scale [Default is vertical].

Optional Arguments
------------------

**-A**\ [**a**\ \|\ **l**\ \|\ **c**]
    Place annotations and labels above (instead of below) horizontal
    scalebars and to the left (instead of the right) of vertical
    scalebars. Append **a** or **l** to move only the annotations or the
    label to the other side. Append **c** if you want to print a
    vertical label as a column of characters (does not work with special characters).

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
    intervals, you must append ;\ *annotation* to each z-slice in the cpt file.

**-C**\ *cpt_file*
    *cpt\_file* is the color palette file to be used. By default all
    color changes are annotated. To use a subset, add an extra column to
    the cpt-file with a L, U, or B to annotate Lower, Upper, or Both
    color segment boundaries (but see **-B**). If not given, **psscale**
    will read stdin. Like **grdview**, **psscale** can understand
    pattern specifications in the cpt file. For CPT files where the
    *z* range is in meters, it may be useful to change to another unit
    when plotting.  To do so, append **+U**\ *unit* to the file name.
    Likewise, if the CPT file uses another unit than meter and you wish
    to plot the CPT versus meters, append **+u**\ *unit*.

**-E**\ [**b**\ \|\ **f**][*length*][\ **+n**\ [*text*]]
    Add sidebar triangles for **b**\ ack- and/or **f**\ oreground
    colors. Add **f** or **b** for only one sidebar triangle [Default
    gives both]. Optionally, append triangle height [Default is half the
    barwidth]. Finally, you can plot a rectangle with the NaN color at
    the start of the bar, labeled with *text* [NaN].

**-G**\ *zlo*\ /\ *zhi*
    Truncate the incoming CPT so that the lowest and highest z-levels
    are to *zlo* and *zhi*.  If one of these equal NaN then
    we leave that end of the CPT alone.  The truncation takes place
    before the plotting.

**-I**\ [*max\_intens*\ \|\ *low\_i*/*high\_i*]
    Add illumination effects. Optionally, set the range of intensities
    from - to + *max\_intens*. If not specified, 1 is used.
    Alternatively, append *low/high* intensities to specify an
    asymmetric range [Default is no illumination]. 

.. include:: explain_-J.rst_

.. include:: explain_-Jz.rst_

.. include:: explain_-K.rst_

**-L**\ [**i**][*gap*]
    Gives equal-sized color rectangles. Default scales rectangles
    according to the z-range in the cpt-file (Also see **-Z**). If set,
    any equal interval annotation set with **-B** will be ignored. If
    *gap* is appended and the cpt table is discrete we will center each
    annotation on each rectangle, using the lower boundary z-value for
    the annotation. If **i** is prepended we annotate the interval range
    instead. If **-I** is used then each rectangle will have its
    constant color modified by the specified intensity.

**-M**
    Force a monochrome graybar using the (television) YIQ transformation.

**-N**\ [**p**\ \|\ *dpi* ]
    Controls how the color scale is represented by the PostScript language.
    To preferentially draw color rectangles (e.g., for discrete colors), append **p**.
    Otherwise we will preferentially draw images (e.g., for continuous colors).
    Optionally append effective dots-per-inch for rasterization of color scales [600].

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-Q**
    Select logarithmic scale and power of ten annotations. All z-values
    in the cpt file will be converted to p = log10(z) and only integer p
    values will be annotated using the 10^p format [Default is linear
    scale]. 

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

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

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-XY.rst_

**-Z**\ *zfile*
    File with colorbar-width per color entry. By default, width of entry
    is scaled to color range, i.e., z = 0-100 gives twice the width as z
    = 100-150 (Also see **-L**). 

.. include:: explain_-c.rst_

.. |Add_perspective| replace:: (Required **-R** and **-J** for proper functioning). 
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

Examples
--------

To append a vertical color scale (7.5 cm long; 1.25 cm wide) to the
right of a plot that is 6 inch wide and 4 inch high, using illumination,
and show back- and foreground colors, and annotating every 5 units, use

   ::

    gmt psscale -D6.5i/2i/7.5c/1.25c -O -Ccolors.cpt -I -E -B5:BATHYMETRY:/:m: > map.ps

Notes
-----

When the cpt file is discrete and no illumination is specified, the
color bar will be painted using polygons. For all other cases we must
paint with an image. Some color printers may give slightly different
colors for the two methods given identical RGB values.

See Also
--------

:doc:`gmt`, :doc:`makecpt`, :doc:`grd2cpt`
