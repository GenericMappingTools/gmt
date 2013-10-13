.. index:: ! psxyz

*****
psxyz
*****

.. only:: not man

    psxyz - Plot lines, polygons, and symbols in 3-D

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psxyz** [ *table* ] **-J**\ *parameters*
**-Jz**\ \|\ **Z**\ *parameters*
|SYN_OPT-Rz|
[ |SYN_OPT-B| ]
[ **-D**\ *dx*/*dy*\ [/*dz*] ] [ **-G**\ *fill* ] [ **-I**\ *intens* ] 
[ **-K** ] [ **-L** ] [ **-N** ] [ **-O** ] [ **-P** ] [ **-Q** ] 
[ **-S**\ [*symbol*][\ *size*\ [**unit**]][/*size_y*] ] 
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ **-W**\ [**-**\ \|\ **+**][*pen*] ] 
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-a| ] 
[ |SYN_OPT-bi| ]
[ |SYN_OPT-c| ]
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

**psxyz** reads (x,y,z) triplets from *files* [or standard input] and
generates PostScript code that will plot lines, polygons, or symbols
at those locations in 3-D. If a symbol is selected and no symbol size
given, then **psxyz** will interpret the fourth column of the input data
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

.. include:: explain_-J.rst_

.. include:: explain_-Jz.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. include:: explain_-B.rst_

**-C**\ *cptfile*
    Give a color palette file. If **-S** is set, let symbol fill color be
    determined by the t-value in the fourth column. Additional fields are
    shifted over by one column (optional size would be in 5th rather than
    4th field, etc.). If **-S** is not set, then **psxyz** expects the user
    to supply a multisegment file (where each segment header contains a
    **-Z**\ *val* string. The *val* will control the color of the line or
    polygon (if **-L** is set) via the cpt file.

**-D**\ *dx*/*dy*\ [/*dz*]
    Offset the plot symbol or line locations by the given amounts
    *dx/dy*\ [*dz*\ ] [Default is no offset].

**-G**\ *fill*
    Select color or pattern for filling of symbols or polygons [Default is no fill].
    Note that **psxyz** will search for **-G** and **-W** strings in all the
    segment headers and let any values thus found over-ride the command line settings.

**-I**\ *intens*
    Use the supplied *intens* value (nominally in the -1 to + 1 range) to
    modulate the fill color by simulating illumination [none]. 

.. include:: explain_-K.rst_

**-L**
    Force closed polygons: connect the endpoints of the line-segment(s) and
    draw polygons. Also, in concert with **-C** and any **-Z** settings in
    the headers will use the implied color for polygon fill [Default is
    polygon pen color]. **-N** Do NOT skip symbols that fall outside map
    border [Default plots points inside border only]. 

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-Q**
    Turn off the automatic sorting of items based on their distance from the
    viewer. The default is to sort the items so that items in the foreground
    are plotted after items in the background. 

.. include:: explain_symbols2.rst_

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

.. include:: explain_-aspatial.rst_

.. |Add_-bi| replace:: [Default is the required number of columns given the chosen settings]. 
.. include:: explain_-bi.rst_

.. include:: explain_-c.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| replace:: The **-g** option is ignored if **-S** is set. 
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*] (\*)
    Select perspective view.

.. include:: explain_-t.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_vectors.rst_

Examples
--------

To plot blue columns (width = 1.25 cm) at the positions listed in the
file heights.xyz on a 3-D projection of the space (0-10), (0-10),
(0-100), with tickmarks every 2, 2, and 10, viewing it from the
southeast at 30 degree elevation, use:

   ::

    gmt psxyz heights.xyz -R0/10/0/10/0/100 -Jx1.25c -Jz0.125c -So1.25c \
              -Gblue -B2:XLABEL:/2:YLABEL:/10:ZLABEL::."3-D PLOT":15 -p135/30 \
              -Uc -W -P > heights.ps

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
    Revert to default pen **MAP\_DEFAULT\_PENBD** (if not set on command line)
**-W-** 
    Turn outline off
**-Z**\ *zval* 
    Obtain fill via cpt lookup using z-value *zval*
**-Z**\ *NaN* 
    Get the NaN color from the cpt file

Custom Symbols
--------------

**psxyz** allows users to define and plot their own custom symbols. This
is done by encoding the symbol using our custom symbol macro code
described in Appendix N. Put all the macro codes for your new symbol in
a file whose extension must be .def; you may then address the symbol
without giving the extension (e.g., the symbol file tsunami.def is used
by specifying **-Sk**\ *tsunami/size*. The definition file can contain
any number of plot code records, as well as blank lines and comment
lines (starting with #). **psxyz** will look for the definition files
in (1) the current directory, (2) the ~/.gmt directory,
and (3) the **$GMT\_SHAREDIR**/custom directory, in that
order. Freeform polygons (made up of straight line segments and arcs of
circles) can be designed - these polygons can be painted and filled with
a pattern. Other standard geometric symbols can also be used. See
Appendix N for macro definitions.

Bugs
----

No hidden line removal is employed for polygons and lines. Symbols,
however, are first sorted according to their distance from the viewpoint
so that nearby symbols will overprint more distant ones should they
project to the same x,y position.

**psxyz** cannot handle filling of polygons that contain the south or
north pole. For such a polygon, make a copy and split it into two and
make each explicitly contain the polar point. The two polygons will
combine to give the desired effect when filled; to draw outline use the
original polygon.

The **-N** option does not adjust the BoundingBox information so you may
have to post-process the PostScript output with :doc:`ps2raster` **-A**
to obtain the correct BoundingBox.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`psbasemap`, :doc:`psxy`
