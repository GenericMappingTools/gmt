.. index:: ! psxy

****
psxy
****

.. only:: not man

    psxy - Plot lines, polygons, and symbols on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psxy** [ *table* ] **-J**\ *parameters*
|SYN_OPT-Rz|
[ **-A**\ [**m**\ \|\ **p**] ] 
[ |SYN_OPT-B| ]
[ **-C**\ *cptfile* ] [ **-D**\ *dx*/*dy* ]
[ **-E**\ [**x**\ [**+**]\ \|\ **y**\ [**+**]\ \|\ **X**\ \|\ **Y**][**n**][*cap*][/[\ **-**\ \|\ **+**]\ *pen*] ] 
[ **-G**\ *fill* ] [ **-I**\ *intens* ] 
[ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ]
[ **-L**\ [**+b**\ \|\ **d**\ \|\ **D**][**+xl**\ \|\ **r**\ \|\ *x0*][**+yl**\ \|\ **r**\ \|\ *y0*][**+p**\ *pen*] ] 
[ **-N**\ [**c**\ \|\ **r**] ] 
[ **-O** ] [ **-P** ] 
[ **-S**\ [*symbol*][\ *size*\ [**u**] ] [ **-T** ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ **-W**\ [**-**\ \|\ **+**][*pen*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-a| ] 
[ |SYN_OPT-bi| ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**psxy** reads (*x*,\ *y*) pairs from *files* [or standard input] and
generates PostScript code that will plot lines, polygons, or symbols
at those locations on a map. If a symbol is selected and no symbol size
given, then **psxy** will interpret the third column of the input data
as symbol size. Symbols whose *size* is <= 0 are skipped. If no symbols
are specified then the symbol code (see **-S** below) must be present as
last column in the input. If **-S** is not used, a line connecting the
data points will be drawn instead. To explicitly close polygons, use
**-L**. Select a fill with **-G**. If **-G** is set, **-W** will control
whether the polygon outline is drawn or not. If a symbol is selected,
**-G** and **-W** determines the fill and outline/no outline,
respectively. The PostScript code is written to standard output. 

Required Arguments
------------------

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. |Add_intables| replace:: Use **-T** to ignore all input files, including standard input (see below).
.. include:: explain_intables.rst_

**-A**\ [**m**\ \|\ **p**]
    By default line segments are drawn as great circle arcs. To draw them as
    straight lines, use the **-A** flag. Alternatively, add **m** to draw
    the line by first following a meridian, then a parallel. Or append **p**
    to start following a parallel, then a meridian. (This can be practical
    to draw a line along parallels, for example). 

.. include:: explain_-B.rst_

**-C**\ *cptfile*
    Give a color palette file or specify -Ccolor1,color2[,color3,...]
    to build a linear continuous cpt from those colors automatically.  
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).
    If **-S** is set, let symbol fill color be
    determined by the z-value in the third column. Additional fields are
    shifted over by one column (optional size would be 4th rather than 3rd
    field, etc.). If **-S** is not set, then **psxy** expects the user to
    supply a multisegment file where each segment header contains a
    **-Z**\ *val* string. The *val* will control the color of the line or
    polygon (if **-L** is set) via the cpt file.

**-D**\ *dx*/*dy*
    Offset the plot symbol or line locations by the given amounts *dx/dy*
    [Default is no offset]. If *dy* is not given it is set equal to *dx*.

**-E**\ [**x**\ [**+**]\ \|\ **y**\ [**+**]\ \|\ **X**\ \|\ **Y**][**n**][*cap*][/[\ **-**\ \|\ **+**]\ *pen*]
    Draw symmetrical error bars. Append **x** and/or **y** to indicate which bars you
    want to draw (Default is both x and y). The x and/or y errors must be
    stored in the columns after the (x,y) pair [or (x,y,size) triplet]. If
    a **+** is appended after **x** and/or **y** then we will draw asymmetrical
    error bar; these requires two rather than one extra data column.  The
    *cap* parameter indicates the length of the end-cap on the error bars
    [7\ **p**]. Pen attributes for error bars may also be set [Defaults: width
    = default, color = black, style = solid]. A leading **+** will use the
    lookup color (via **-C**) for both symbol fill and error pen color,
    while a leading **-** will set error pen color and turn off symbol fill.
    If upper case **X** and/or **Y** is used we will instead draw
    "box-and-whisker" (or "stem-and-leaf") symbols. The x (or y) coordinate
    is then taken as the median value, and 4 more columns are expected to
    contain the minimum (0% quantile), the 25% quantile, the 75% quantile,
    and the maximum (100% quantile) values. The 25-75% box may be filled by
    using **-G**. If **n** is appended to **X** (or **Y**) we draw a notched
    "box-and-whisker" symbol where the notch width reflects the uncertainty
    in the median. Then a 5th extra data column is expected to contain the
    number of points in the distribution.

**-G**\ *fill*
    Select color or pattern for filling of symbols or polygons [Default is no fill].
    Note that **psxy** will search for **-G** and **-W** strings in all the
    segment headers and let any values thus found over-ride the command line settings.

**-I**\ *intens*
    Use the supplied *intens* value (nominally in the -1 to + 1 range) to
    modulate the fill color by simulating illumination [none]. 

.. include:: explain_-Jz.rst_ 

.. include:: explain_-K.rst_

.. include:: PSXY_OPTs_EX.rst_

**-L**\ [**+b**\ \|\ **d**\ \|\ **D**][**+xl**\ \|\ **r**\ \|\ *x0*][**+yl**\ \|\ **r**\ \|\ *y0*][**+p**\ *pen*] |loc_OPT-L|
    Force closed polygons.  Alternatively, append modifiers to build a polygon from a line segment.
    Append **+d** to build symmetrical envelope around y(x) using deviations dy(x) given in extra column 3.
    Append **+D** to build asymmetrical envelope around y(x) using deviations dy1(x) and dy2(x) from extra columns 3-4.
    Append **+b** to build asymmetrical envelope around y(x) using bounds yl(x) and yh(x) from extra columns 3-4.
    Append **+xl**\ \|\ **r**\ \|\ *x0* to connect first and last point to anchor points at either xmin, xmax, or x0, or
    append **+yb**\ \|\ **t**\ \|\ *y0* to connect first and last point to anchor points at either ymin, ymax, or y0.
    Polygon may be painted (**-G**) and optionally outlined by adding **+p**\ *pen* [no outline].

**-N**\ [**c**\ \|\ **r**]
    Do NOT clip symbols that fall outside map border [Default plots points
    whose coordinates are strictly inside the map border only]. The option does not apply to lines and polygons
    which are always clipped to the map region. For periodic (360-longitude)
    maps we must plot all symbols twice in case they are clipped by the repeating
    boundary. The **-N** will turn off clipping and not plot repeating symbols.
    Use **-Nr** to turn off clipping but retain the plotting of such repeating symbols, or
    use **-Nc** to retain clipping but turn off plotting of repeating symbols.

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

.. include:: explain_symbols.rst_

**-T**
    Ignore all input files, including standard input. This is the same
    as specifying /dev/null (or NUL for Windows users) as input file.
    Use this to activate only the options that are not related to
    plotting of lines or symbols, such as **psxy** **-R** **-J** **-O**
    **-T** to terminate a sequence of GMT plotting commands without
    producing any plotting output. 

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-W**\ [**-**\ \|\ **+**][*pen*\ ]
    Set pen attributes for lines or the outline of symbols [Defaults:
    width = default, color = black, style = solid]. A leading **+** will
    use the lookup color (via **-C**) for both symbol fill and outline
    pen color, while a leading **-** will set outline pen color and turn
    off symbol fill. 

.. include:: explain_-XY.rst_

.. |Add_-bi| replace:: [Default is the required number of columns given the chosen settings]. 
.. include:: explain_-bi.rst_

.. include:: explain_-aspatial.rst_

.. include:: explain_-c.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| replace:: The **-g** option is ignored if **-S** is set. 
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_colon.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_vectors.rst_

Examples
--------

To plot solid red circles (diameter = 0.25 cm) at the positions listed
in the file DSDP.xy on a Mercator map at 5 cm/degree of the area 150E to
154E, 18N to 23N, with tickmarks every 1 degree and gridlines every 15
minutes, use

   ::

    gmt psxy DSDP.xy R150/154/18/23 -Jm5c -Sc0.25c -Gred -B1g15m > map.ps

To plot the xyz values in the file quakes.xyzm as circles with size
given by the magnitude in the 4th column and color based on the depth in
the third using the color palette cpt on a linear map, use

   ::

    gmt psxy quakes.xyzm -R0/1000/0/1000 -JX6i -Sc -Ccpt -B200 > map.ps

To plot the file trench.xy on a Mercator map, with white triangles with
sides 0.25 inch on the left side of the line, spaced every 0.8 inch, use

   ::

    gmt psxy trench.xy -R150/200/20/50 -Jm0.15i -Sf0.8i/0.1i+l+t -Gwhite -W -B10 > map.ps

To plot the data in the file misc.d as symbols determined by the code in
the last column, and with size given by the magnitude in the 4th column,
and color based on the third column via the color palette cpt on a
linear map, use

   ::

    gmt psxy misc.d -R0/100/-50/100 -JX6i -S -Ccpt -B20 > map.ps

If you need to place vectors on a plot you can choose among
straight Cartesian vectors, math circular vectors, or geo-vectors (these
form small or great circles on the Earth).  These can have optional heads at either
end, and heads may be the traditional arrow, a circle, or a terminal cross-line.
To place a few vectors with
a circle at the start location and an arrow head at the end, try

   ::

    gmt psxy -R0/50/-50/50 -JX6i -Sv0.15i+bc+ea -Gyellow -W0.5p -Baf << EOF > map.ps
    10 10 45 2i
    30 -20 0 1.5i
    EOF

To plot vectors (red vector heads, solid stem) from the file data.txt that contains
record of the form lon, lat, dx, dy, where dx, dy are the Cartesian
vector components given in user units, and these user units should be converted
to cm given the scale 3.60, try

   ::

    gmt psxy -R20/40/-20/0 -JM6i -Sv0.15i+e+z3.6c -Gred -W0.25p -Baf data.txt > map.ps

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
    Get the NaN color from the cpt file

Custom Symbols
--------------

**psxy** allows users to define and plot their own custom symbols. This
is done by encoding the symbol using our custom symbol macro code
described in Appendix N. Put all the macro codes for your new symbol in
a file whose extension must be .def; you may then address the symbol
without giving the extension (e.g., the symbol file tsunami.def is used
by specifying **-Sk**\ *tsunami/size*. The definition file can contain
any number of plot code records, as well as blank lines and comment
lines (starting with #). **psxy** will look for the definition files in
(1) the current directory, (2) the ~/.gmt directory, and
(3) the **$GMT_SHAREDIR**/custom directory, in that order.
Freeform polygons (made up of straight line segments and arcs of
circles) can be designed - these polygons can be painted and filled with
a pattern. Other standard geometric symbols can also be used. See Appendix
:ref:`App-custom_symbols` for macro definitions.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`psbasemap`, :doc:`psxyz`
