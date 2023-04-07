.. index:: ! plot3d
.. include:: module_core_purpose.rst_

******
plot3d
******

|plot3d_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt plot3d** [ *table* ] |-J|\ *parameters*
|-Jz|\ \|\ **Z**\ *parameters*
|SYN_OPT-Rz|
[ |-A|\ [**m**\|\ **p**\|\ **x**\|\ **y**\|\ **r**\|\ **t**] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *dx*/*dy*\ [/*dz*] ]
[ |-G|\ *fill*\|\ **+z** ]
[ |-H|\ [*scale*] ]
[ |-I|\ [*intens*] ]
[ |-L|\ [**+b**\|\ **d**\|\ **D**][**+xl**\|\ **r**\|\ *x0*][**+yl**\|\ **r**\|\ *y0*][**+p**\ *pen*] ]
[ |-N| ]
[ |-Q| ]
[ |-S|\ [*symbol*][*size*][/*size_y*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][*attr*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ *value*\|\ *file*]
[ |SYN_OPT-a| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-l| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-tv| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads (x,y,z) triplets from *files* [or standard input] and
will plot lines, polygons, or symbols
at those locations in 3-D. If a symbol is selected and no symbol size
given, then we will interpret the fourth column of the input data
as symbol size. Symbols whose *size* is <= 0 are skipped. If no symbols
are specified then the symbol code (see |-S| below) must be present as
last column in the input. If |-S| is not used, a line connecting the
data points will be drawn instead. To explicitly close polygons, use
|-L|. Select a fill with |-G|. If |-G| is set, |-W| will control
whether the polygon outline is drawn or not. If a symbol is selected,
**-G** and |-W| determines the fill and outline/no outline,
respectively.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Jz:

.. include:: explain_-Jz.rst_

.. |Add_-R| replace:: |Add_-R_auto_table|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [**m**\|\ **p**\|\ **x**\|\ **y**\|\ **r**\|\ **t**]
    By default, geographic line segments are drawn as great circle arcs by resampling
    coarse input data along such arcs. To disable this sampling and draw them as
    straight lines, use the |-A| flag.  Alternatively, add **m** to draw
    the line by first following a meridian, then a parallel. Or append **p**
    to start following a parallel, then a meridian. (This can be practical
    to draw a line along parallels, for example).  For Cartesian data, points
    are simply connected, unless you append **x** or **y** to draw stair-case
    curves that whose first move is along *x* or *y*, respectively. For polar
    projection, append **r** or **t** to draw stair-case curves that whose first
    move is along *r* or *theta*, respectively. **Note**:
    The |-A| option requires constant *z*-coordinates.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *cpt*
    Give a CPT or specify **-C**\ *color1,color2*\ [*,color3*\ ,...]
    to build a linear continuous CPT from those colors automatically.
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).
    If |-S| is set, let symbol fill color be
    determined by the *value* in the fourth column. Additional fields are
    shifted over by one column (optional *size* would be in 5th rather than
    4th field, etc.). An exception to this rule is for multi-band 3-D
    columns where each band gets its color from each slice in the CPT.
    If |-S| is not set, then it expects the user to
    supply a multisegment file where each segment header contains a
    **-Z**\ *value* string. The *value* will control the color of the line or
    polygon (if |-L| is set) via the CPT.  Alternatively, see the |-Z|
    option for how to assign *z*-values. **Note**: If modern mode and no
    argument is given then we select the current CPT.

.. _-D:

**-D**\ *dx*/*dy*\ [/*dz*]
    Offset the plot symbol or line locations by the given amounts
    *dx/dy*\ [*dz*] [Default is no offset].  You may append dimensional
    units from **c**\ \|\ **i**\ \|\ **p** to each value.

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Select color or pattern for filling of symbols or polygons [Default is no fill].
    Note that the module will search for |-G| and |-W| strings in all the
    segment headers and let any values thus found over-ride the command line settings.
    If |-Z| is set, use **-G+z** to assign fill color via **-C**\ *cpt* and the
    *z*-values obtained.  Finally, if *fill* = *auto*\ [*-segment*] or *auto-table* then
    we will cycle through the fill colors implied by :term:`COLOR_SET` and change on a per-segment
    or per-table basis.  Any *transparency* setting is unchanged.

.. _-H:

**-H**\ [*scale*]
    Scale symbol sizes and pen widths on a per-record basis using the *scale* read from the
    data set, given as the first column after the (optional) *w* and *size* columns [no scaling].
    The symbol size is either provided by |-S| or via the input *size* column.  Alternatively,
    append a constant *scale* that should be used instead of reading a scale column.

.. _-I:

**-I**\ *intens*
    Use the supplied *intens* value (nominally in the -1 to +1 range) to
    modulate the fill color by simulating illumination [none]. If no intensity
    is provided we will instead read *intens* from the first data column after
    the symbol parameters (if given).

.. _-L:

**-L**\ [**+b**\|\ **d**\|\ **D**][**+xl**\|\ **r**\|\ *x0*][**+yl**\|\ **r**\|\ *y0*][**+p**\ *pen*]
    Force closed polygons.  Alternatively, append modifiers to build a polygon from a line segment.
    Append **+d** to build symmetrical envelope around y(x) using deviations dy(x) given in extra column 4.
    Append **+D** to build asymmetrical envelope around y(x) using deviations dy1(x) and dy2(x) from extra columns 4-5.
    Append **+b** to build asymmetrical envelope around y(x) using bounds yl(x) and yh(x) from extra columns 4-5.
    Append **+xl**\|\ **r**\|\ *x0* to connect first and last point to anchor points at either *xmin*, *xmax*, or *x0*, or
    append **+yb**\|\ **t**\|\ *y0* to connect first and last point to anchor points at either *ymin*, *ymax*, or *y0*.
    Polygon may be painted (|-G|) and optionally outlined by adding **+p**\ *pen* [no outline].
    All constructed polygons are assumed to have a constant *z* value.
    **Note**: When option |-Z| is passed via segment headers you will need |-L| to ensure
    your segments are interpreted as polygons, else they are seen as lines.

.. _-N:

**-N**\ [**c**\|\ **r**]
    Do **not** clip symbols that fall outside map border [Default plots points
    whose coordinates are strictly inside the map border only]. For periodic (360-longitude)
    maps we must plot all symbols twice in case they are clipped by the repeating
    boundary. The |-N| will turn off clipping and not plot repeating symbols.
    Use **-Nr** to turn off clipping but retain the plotting of such repeating symbols, or
    use **-Nc** to retain clipping but turn off plotting of repeating symbols.
    **Note**: A plain |-N| may also be used with lines or polygons but note that this deactivates
    any consideration of periodicity (e.g., longitudes) and may have unintended consequences.

.. _-Q:

**-Q**
    Turn off the automatic sorting of items based on their distance from the
    viewer. The default is to sort the items so that items in the foreground
    are plotted after items in the background.

.. _-S:

.. include:: explain_symbols.rst_

.. include:: explain_3D_symbols.rst_

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    Set pen attributes for lines or the outline of symbols [Defaults:
    width = 0.25p, color = black, style = solid]. If the modifier **+cl**
    is appended then the color of the line are taken from the CPT (see
    |-C|). If instead modifier **+cf** is appended then the color from the cpt
    file is applied to symbol fill.  Use just **+c** for both effects.
    If |-Z| is set, then append **+z** to |-W| to assign pen color via **-C**\ *cpt* and the
    *z*-values obtained.  Finally, if pen *color* = *auto*\ [*-segment*] or *auto-table* then
    we will cycle through the pen colors implied by :term:`COLOR_SET` and change on a per-segment
    or per-table basis.  The *width*, *style*, or *transparency* settings are unchanged.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ *value*\|\ *file*
    Instead of specifying a symbol or polygon fill and outline color via |-G| and |-W|,
    give both a *value* via |-Z| and a color lookup table via |-C|.  Alternatively,
    give the name of a *file* with one z-value (read from the last column) for each polygon in the input data.
    To apply the color obtained to a fill, use **-G+z**; to apply it to the pen color, append **+z** to |-W|.

.. include:: explain_-aspatial.rst_

.. |Add_-bi| replace:: [Default is the required number of columns given the chosen settings].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| replace:: The **-g** option is ignored if |-S| is set.
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_-l| unicode:: 0x20 .. just an invisible code
.. include:: explain_-l.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-qi.rst_

.. include:: explain_-tv_full.rst_

.. include:: explain_-w.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_vectors.rst_

.. module_common_ends

.. include:: auto_legend_info.rst_

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To plot blue columns (width = 1.25 cm) at the positions listed in the
file heights.xyz on a 3-D projection of the space (0-10), (0-10),
(0-100), with tickmarks every 2, 2, and 10, viewing it from the
southeast at 30 degree elevation::

    gmt plot3d heights.xyz -R0/10/0/10/0/100 -Jx1.25c -Jz0.125c -So1.25c \
              -Gblue -Bx2+lXLABEL -By2+lYLABEL -Bz10+lZLABEL -B+t"3-D PLOT" -p135/30 \
              -U+c -W -pdf heights

To plot a point with color and outline dictated by the *t.cpt* file for the *level*-value 65::

    echo 175 30 0 | gmt plot3d -R150/200/20/50 -JM15c -B -Sc0.5c -Z65 -G+z -Ct.cpt -pdf map

.. include:: plot3d_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`basemap`, :doc:`plot`
