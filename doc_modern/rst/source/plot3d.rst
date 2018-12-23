.. index:: ! plot3d

******
plot3d
******

.. only:: not man

    Plot lines, polygons, and symbols in 3-D

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt plot3d** [ *table* ] |-J|\ *parameters*
|-J|\ **z**\ \|\ **Z**\ *parameters*
|SYN_OPT-Rz|
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *dx*/*dy*\ [/*dz*] ]
[ |-G|\ *fill* ]
[ |-I|\ *intens* ] 
[ |-L|\ [**+b**\ \|\ **d**\ \|\ **D**][**+xl**\ \|\ **r**\ \|\ *x0*][**+yl**\ \|\ **r**\ \|\ *y0*][**+p**\ *pen*] ] 
[ |-N| ]
[ |-Q| ] 
[ |-S|\ [*symbol*][\ *size*\ [**unit**]][/*size_y*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][*attr*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-a| ] 
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**plot3d** reads (x,y,z) triplets from *files* [or standard input] and
will plot lines, polygons, or symbols
at those locations in 3-D. If a symbol is selected and no symbol size
given, then **plot3d** will interpret the fourth column of the input data
as symbol size. Symbols whose *size* is <= 0 are skipped. If no symbols
are specified then the symbol code (see **-S** below) must be present as
last column in the input. If **-S** is not used, a line connecting the
data points will be drawn instead. To explicitly close polygons, use
**-L**. Select a fill with **-G**. If **-G** is set, **-W** will control
whether the polygon outline is drawn or not. If a symbol is selected,
**-G** and **-W** determines the fill and outline/no outline,
respectively.

Required Arguments
------------------

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. include:: explain_-Jz.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-B:

.. include:: explain_-B.rst_

.. _-C:

**-C**\ *cpt*
    Give a CPT or specify **-C**\ *color1,color2*\ [*,color3*\ ,...]
    to build a linear continuous CPT from those colors automatically.  
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).
    If **-S** is set, let symbol fill color be
    determined by the t-value in the fourth column. Additional fields are
    shifted over by one column (optional size would be in 5th rather than
    4th field, etc.). An exception to this rule is for multi-band 3-D
    columns where each band gets its color from each slice in the CPT.
    If **-S** is not set, then **plot3d** expects the user
    to supply a multisegment file (where each segment header contains a
    **-Z**\ *val* string. The *val* will control the color of the line or
    polygon (if **-L** is set) via the CPT.

.. _-D:

**-D**\ *dx*/*dy*\ [/*dz*]
    Offset the plot symbol or line locations by the given amounts
    *dx/dy*\ [*dz*\ ] [Default is no offset].

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Select color or pattern for filling of symbols or polygons [Default is no fill].
    Note that **plot3d** will search for **-G** and **-W** strings in all the
    segment headers and let any values thus found over-ride the command line settings.

.. _-I:

**-I**\ *intens*
    Use the supplied *intens* value (nominally in the -1 to + 1 range) to
    modulate the fill color by simulating illumination [none]. 

.. _-L:

**-L**\ [**+b**\ \|\ **d**\ \|\ **D**][**+xl**\ \|\ **r**\ \|\ *x0*][**+yl**\ \|\ **r**\ \|\ *y0*][**+p**\ *pen*]
    Force closed polygons.  Alternatively, append modifiers to build a polygon from a line segment.
    Append **+d** to build symmetrical envelope around y(x) using deviations dy(x) given in extra column 4.
    Append **+D** to build asymmetrical envelope around y(x) using deviations dy1(x) and dy2(x) from extra columns 4-5.
    Append **+b** to build asymmetrical envelope around y(x) using bounds yl(x) and yh(x) from extra columns 4-5.
    Append **+xl**\ \|\ **r**\ \|\ *x0* to connect first and last point to anchor points at either xmin, xmax, or x0, or
    append **+yb**\ \|\ **t**\ \|\ *y0* to connect first and last point to anchor points at either ymin, ymax, or y0.
    Polygon may be painted (**-G**) and optionally outlined by adding **+p**\ *pen* [no outline].
    All constructed polygons are assumed to have a constant z value.

.. _-N:

**-N**\ [**c**\ \|\ **r**]
    Do NOT clip symbols that fall outside map border [Default plots points
    whose coordinates are strictly inside the map border only]. The option does not apply to lines and polygons
    which are always clipped to the map region. For periodic (360-longitude)
    maps we must plot all symbols twice in case they are clipped by the repeating
    boundary. The **-N** will turn off clipping and not plot repeating symbols.
    Use **-Nr** to turn off clipping but retain the plotting of such repeating symbols, or
    use **-Nc** to retain clipping but turn off plotting of repeating symbols.

.. _-Q:

**-Q**
    Turn off the automatic sorting of items based on their distance from the
    viewer. The default is to sort the items so that items in the foreground
    are plotted after items in the background. 

.. _-S:

.. include:: explain_symbols2.rst_

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    Set pen attributes for lines or the outline of symbols [Defaults:
    width = default, color = black, style = solid]. If the modifier **+cl**
    is appended then the color of the line are taken from the CPT (see
    **-C**). If instead modifier **+cf** is appended then the color from the cpt
    file is applied to symbol fill.  Use just **+c** for both effects.

.. _-X:

.. include:: explain_-XY.rst_

.. include:: explain_-aspatial.rst_

.. |Add_-bi| replace:: [Default is the required number of columns given the chosen settings]. 
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| replace:: The **-g** option is ignored if **-S** is set. 
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_vectors.rst_

Examples
--------

To plot blue columns (width = 1.25 cm) at the positions listed in the
file heights.xyz on a 3-D projection of the space (0-10), (0-10),
(0-100), with tickmarks every 2, 2, and 10, viewing it from the
southeast at 30 degree elevation, use:

   ::

    gmt plot3d heights.xyz -R0/10/0/10/0/100 -Jx1.25c -Jz0.125c -So1.25c
              -Gblue -Bx2+lXLABEL -By2+lYLABEL -Bz10+lZLABEL -B+t"3-D PLOT" -p135/30
              -U+c -W -pdf heights

Segment Header Parsing
----------------------

Segment header records may contain one of more of the following options:

**-G**\ *fill* 
    Use the new *fill* and turn filling on
**-G-** 
    Turn filling off
**-G** 
    Revert to default fill (none if not set on command line)
**-W**\ *pen* 
    Use the new *pen* and turn outline on
**-W** 
    Revert to default pen :ref:`MAP_DEFAULT_PEN <MAP_DEFAULT_PEN>`
    (if not set on command line)
**-W-** 
    Turn outline off
**-Z**\ *zval* 
    Obtain fill via cpt lookup using z-value *zval*
**-Z**\ *NaN* 
    Get the NaN color from the CPT

Custom Symbols
--------------

**plot3d** allows users to define and plot their own custom symbols. This
is done by encoding the symbol using our custom symbol macro code
described in Appendix N. Put all the macro codes for your new symbol in
a file whose extension must be .def; you may then address the symbol
without giving the extension (e.g., the symbol file tsunami.def is used
by specifying **-Sk**\ *tsunami/size*. The definition file can contain
any number of plot code records, as well as blank lines and comment
lines (starting with #). **plot3d** will look for the definition files
in (1) the current directory, (2) the ~/.gmt directory,
and (3) the **$GMT_SHAREDIR**/custom directory, in that
order. Freeform polygons (made up of straight line segments and arcs of
circles) can be designed - these polygons can be painted and filled with
a pattern. Other standard geometric symbols can also be used. See Appendix
:ref:`App-custom_symbols` for macro definitions.

Bugs
----

No hidden line removal is employed for polygons and lines. Symbols,
however, are first sorted according to their distance from the viewpoint
so that nearby symbols will overprint more distant ones should they
project to the same x,y position.

**plot3d** cannot handle filling of polygons that contain the south or
north pole. For such a polygon, make a copy and split it into two and
make each explicitly contain the polar point. The two polygons will
combine to give the desired effect when filled; to draw outline use the
original polygon.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`basemap`, :doc:`plot`
