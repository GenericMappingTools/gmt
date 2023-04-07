.. index:: ! plot
.. include:: module_core_purpose.rst_

****
plot
****

|plot_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt plot** [ *table* ] |-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A|\ [**m**\|\ **p**\|\ **x**\|\ **y**\|\ **r**\|\ **t**] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *dx*/*dy* ]
[ |-E|\ [**x**\|\ **y**\|\ **X**\|\ **Y**][**+a**\|\ **A**][**+cl**\|\ **f**][**+n**][**+w**\ *width*\ [/*cap*]][**+p**\ *pen*] ]
[ |-F|\ [**c**\|\ **n**\|\ **r**][**a**\|\ **f**\|\ **s**\|\ **r**\|\ *refpoint*] ]
[ |-G|\ *fill*\|\ **+z** ]
[ |-H|\ [*scale*] ]
[ |-I|\ [*intens*] ]
[ |-L|\ [**+b**\|\ **d**\|\ **D**][**+xl**\|\ **r**\|\ *x0*][**+yb**\|\ **t**\|\ *y0*][**+p**\ *pen*] ]
[ |-N|\ [**c**\|\ **r**] ]
[ |-S|\ [*symbol*][*size*] ]
[ |-T| ]
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

Reads (*x*,\ *y*) pairs from *files* [or standard input] and will plot lines, polygons, or symbols at those locations on a map.
If a symbol is selected and no symbol size given, then it will interpret the third column of the input data as symbol size.
Symbols whose *size* is <= 0 are skipped. If no symbols are specified then the symbol code (see |-S| below) must be present as
last column in the input. If |-S| is not used, a line connecting the data points will be drawn instead. To explicitly close
polygons, use |-L|. Select a fill with |-G|. If |-G| is set, |-W| will control whether the polygon outline is drawn or not.
If a symbol is selected, |-G| and |-W| determines the fill and outline/no outline, respectively.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

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
    move is along *r* or *theta*, respectively.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *cpt*
    Give a CPT or specify **-C**\ *color1,color2*\ [*,color3*\ ,...]
    to build a linear continuous CPT from those colors automatically,
    where *z* starts at 0 and is incremented by one for each color.
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).
    If |-S| is set, let symbol fill color be
    determined by the *z*-value in the third column. Additional fields are
    shifted over by one column (optional *size* would be 4th rather than 3rd
    field, etc.).
    If |-S| is not set, then it expects the user to
    supply a multisegment file where each segment header contains a
    **-Z**\ *value* string. The *value* will control the color of the line or
    polygon (if |-L| is set) via the CPT.  Alternatively, see the |-Z|
    option for how to assign *z*-values. **Note**: If modern mode and no
    argument is given then we select the current CPT.

.. _-D:

**-D**\ *dx*/*dy*
    Offset the plot symbol or line locations by the given amounts *dx/dy*
    [Default is no offset]. If *dy* is not given it is set equal to *dx*.
    You may append dimensional units from **c**\ \|\ **i**\ \|\ **p** to each value.

.. _-E:

**-E**\ [**x**\|\ **y**\|\ **X**\|\ **Y**][**+a**\|\ **A**][**+cl**\|\ **f**][**+n**][**+w**\ *width*\ [/*cap*]][**+p**\ *pen*]
    Draw error bars. Append **x** and/or **y** to indicate which bars you
    want to draw [Default is both x and y]. The x and/or y errors must be
    stored in the columns after the (x,y) pair [or (x,y,z) triplet]. If
    **+a** is appended then we will draw asymmetrical error bars [Default
    is symmetrical error bars]; these requires
    two rather than one extra data column, with the two signed deviations.
    Use **+A** to read the low and high bounds rather than signed deviations.
    If upper case **X** and/or **Y** are used we will instead draw
    "box-and-whisker" (or "stem-and-leaf") symbols. The x (or y) coordinate
    is then taken as the median value, and four more columns are expected to
    contain the minimum (0% quantile), the 25% quantile, the 75% quantile,
    and the maximum (100% quantile) values. The 25-75% box may be filled by
    using |-G|. If **+n** is appended the we draw a notched
    "box-and-whisker" symbol where the notch width reflects the uncertainty
    in the median. This symbol requires a 5th extra data column to contain the
    number of points in the distribution.  The **+w** modifier sets the
    *width* that indicates the length of the end-cap on the error bars
    [7\ **p**]. For box-and-whisker symbols it sets both the default box width and whisker cap length [7\ **p**].
    Append both *width*\ /*cap* to set separate width and cap dimensions for such symbols.
    Pen attributes for error bars may also be set via **+p**\ *pen*.
    [Defaults: width = 0.25p, color = black, style = solid]. When |-C| is
    used we can control how the look-up color is applied to our symbol.
    Append **+cf** to use it to fill the symbol, while **+cl** will just
    set the error pen color and turn off symbol fill.  Giving **+c** will
    set both color items.  **Note**: The error bars are placed behind symbols
    except for the large vertical and horizontal bar symbols (**-Sb**\|\ **B**)
    where they are plotted on top to avoid the lower bounds being obscured.

.. _-F:

**-F**\ [**c**\|\ **n**\|\ **p**][**a**\|\ **f**\|\ **s**\|\ **r**\|\ *refpoint*]
    Alter the way points are connected (by specifying a *scheme*) and data are grouped (by specifying a *method*).
    Append one of three line connection schemes:
    **c**\ : Draw continuous line segments for each group [Default].
    **p**\ : Draw line segments from a reference point reset for each group.
    **n**\ : Draw networks of line segments between all points in each group.
    Optionally, append the one of four segmentation methods to define the group:
    **a**\ : Ignore all segment headers, i.e., let all points belong to a single group,
    and set group reference point to the very first point of the first file.
    **f**\ : Consider all data in each file to be a single separate group and
    reset the group reference point to the first point of each group.
    **s**\ : Segment headers are honored so each segment is a group; the group
    reference point is reset to the first point of each incoming segment [Default].
    **r**\ : Same as **s**, but the group reference point is reset after
    each record to the previous point (this method is only available with the **-Fp** scheme).
    Instead of the codes **a**\|\ **f**\|\ **s**\|\ **r** you may append
    the coordinates of a *refpoint* which will serve as a fixed external
    reference point for all groups.

.. _-G:

**-G**\ *fill*\|\ **+z** :ref:`(more ...) <-Gfill_attrib>`
    Select color or pattern for filling of symbols or polygons [Default is no fill].
    Note that this module will search for |-G| and |-W| strings in all the
    segment headers and let any values thus found over-ride the command line settings.
    If |-Z| is set, use **-G+z** to assign fill color via **-C**\ *cpt* and the
    *z*-values obtained. Finally, if *fill* = *auto*\ [*-segment*] or *auto-table* then
    we will cycle through the fill colors implied by :term:`COLOR_SET` and change on a per-segment
    or per-table basis.  Any *transparency* setting is unchanged.

.. _-H:

**-H**\ [*scale*]
    Scale symbol sizes and pen widths on a per-record basis using the *scale* read from the
    data set, given as the first column after the (optional) *z* and *size* columns [no scaling].
    The symbol size is either provided by |-S| or via the input *size* column.  Alternatively,
    append a constant *scale* that should be used instead of reading a scale column.

.. _-I:

**-I**\ *intens*
    Use the supplied *intens* value (nominally in the -1 to +1 range) to
    modulate the fill color by simulating illumination [none]. If no intensity
    is provided we will instead read *intens* from the first data column after
    the symbol parameters (if given).

.. _-L:

**-L**\ [**+b**\|\ **d**\|\ **D**][**+xl**\|\ **r**\|\ *x0*][**+yb**\|\ **t**\|\ *y0*][**+p**\ *pen*] |ex_OPT-L|
    Force closed polygons.  Alternatively, append modifiers to build a polygon from a line segment.
    Append **+d** to build symmetrical envelope around y(x) using deviations dy(x) given in extra column 3.
    Append **+D** to build asymmetrical envelope around y(x) using deviations dy1(x) and dy2(x) from extra columns 3-4.
    Append **+b** to build asymmetrical envelope around y(x) using bounds yl(x) and yh(x) from extra columns 3-4.
    Append **+xl**\|\ **r**\|\ *x0* to connect first and last point to anchor points at either *xmin*, *xmax*, or *x0*, or
    append **+yb**\|\ **t**\|\ *y0* to connect first and last point to anchor points at either *ymin*, *ymax*, or *y0*.
    Polygon may be painted (|-G|) and optionally outlined by adding **+p**\ *pen* [no outline].
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

.. _-S:

.. include:: explain_symbols.rst_

.. _-T:

**-T**
    Ignore all input files.  If |-B| is not used then **-R -J** are not required.
    Typically used to move plot origin via |-X| and |-Y|.

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
    You can also append one or more additional line attribute modifiers:
    **+o**\ *offset*\ *unit* will start and stop drawing the line the given distance offsets
    from the end point.  Append unit *unit* from **c**\|\ **i**\|\ **p** to
    indicate plot distance on the map or append map distance units instead (see below)
    [Cartesian distances];
    **+s** will draw the line using a Bezier spline [linear spline];
    **+v**\ *vspecs* will place a vector head at the ends of the lines.  You can
    use **+vb** and **+ve** to specify separate vector specs at each end [shared specs].
    Because **+v** may take additional modifiers it must necessarily be given
    at the end of the pen specification.
    See the `Vector Attributes`_ for more information.
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

.. include:: explain_-qi.rst_

.. include:: explain_colon.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-tv_full.rst_

.. include:: explain_-w.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_vectors.rst_

.. module_common_ends

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To plot solid red circles (diameter = 0.2 cm) at the positions listed
in the remote file DSDP.txt on a Mercator map at 0.3 cm/degree of the area 100E to
160E, 20S to 30N, with automatic tick-marks and gridlines::

    gmt plot @DSDP.txt -R100/160/-20/30 -Jm0.3c -Sc0.2c -Gred -Bafg -pdf map

To plot the xyz values in the file quakes.xyzm as circles with size
given by the magnitude in the 4th column and color based on the depth in
the third using the CPT rgb.cpt on a linear map::

    gmt plot quakes.xyzm -R0/1000/0/1000 -JX6i -Sc -Crgb -B200 -pdf map

To plot the file trench.txt on a Mercator map, with white triangles with
sides 0.25 inch on the left side of the line, spaced every 0.8 inch::

    gmt plot trench.txt -R150/200/20/50 -Jm0.15i -Sf0.8i/0.1i+l+t -Gwhite -W -B10 -pdf map

To plot a circle with color dictated by the *t.cpt* file for the *z*-value 65::

    echo 175 30 | gmt plot -R150/200/20/50 -JM15c -B -Sc0.5c -Z65 -Ct.cpt -G+z -pdf map

To plot the data in the file misc.txt as symbols determined by the code in
the last column, and with size given by the magnitude in the 4th column,
and color based on the third column via the CPT chrome on a
linear mape::

    gmt plot misc.txt -R0/100/-50/100 -JX6i -S -Cchrome -B20 -pdf map

If you need to place vectors on a plot you can choose among
straight Cartesian vectors, math circular vectors, or geo-vectors (these
form small or great circles on the Earth).  These can have optional heads at either
end, and heads may be the traditional arrow, a circle, or a terminal cross-line.
To place a few vectors with
a circle at the start location and an arrow head at the end::

    gmt plot -R0/50/-50/50 -JX6i -Sv0.15i+bc+ea -Gyellow -W0.5p -Baf -pdf map << EOF
    10 10 45 2i
    30 -20 0 1.5i
    EOF

To plot vectors (red vector heads, solid stem) from the file data.txt that contains
record of the form lon, lat, dx, dy, where dx, dy are the Cartesian
vector components given in user units, and these user units should be converted
to cm given the scale 3.60::

    gmt plot -R20/40/-20/0 -JM6i -Sv0.15i+e+z3.6c -Gred -W0.25p -Baf data.txt -pdf map

.. include:: plot_notes.rst_

.. include:: auto_legend_info.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`basemap`, :doc:`plot3d`

.. ------------------------------------- Examples per option -------------------

.. |ex_OPT-L| raw:: html

   <a href="#openModal">Example</a>
   <div id="openModal" class="modalDialog">
    <div>
        <a href="#close" title="Close" class="close">X</a>
        <h2>-L example</h2>
        <p>
        cat << EOF > t.txt</br>
        1 1</br>
        2 3</br>
        3 2</br>
        4 4</br>
        EOF</br>
        gmt begin filler pdf</br>
          gmt plot -R0/5/0/5 -JX3i -B t.txt -Gred -W2p -L+yb</br>
          gmt plot -B t.txt -Gred -W2p -L+yt -X3.25i</br>
          gmt plot -B t.txt -Gred -W2p -L+xl -X-3.25i -Y3.25i</br>
          gmt plot -B t.txt -Gred -W2p -L+xr -X3.25i</br>
          gmt plot -B t.txt -Gred -W2p -L+y4 -X-3.25i -Y3.25i</br>
          gmt plot -B t.txt -Gred -W2p -L+x4.5 -X3.25i</br>
        gmt end show</br>
        </p>
    </div>
   </div>
