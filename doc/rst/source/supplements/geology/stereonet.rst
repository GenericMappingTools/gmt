.. index:: ! stereonet
.. include:: ../module_supplements_purpose.rst_

*********
stereonet
*********

|stereonet_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt stereonet** [ *table* ]
[ |-J|\ **A**\|\ **S**\ [0/0/]\ *width* ]
[ |-A|\ [*annot*\ [/*tick*]] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-L|\ *pen* ]
[ |-M|\ [**c**\|\ **p**] ]
[ |SYN_OPT-R| ]
[ |-S|\ *symbol*\ [*size*] ]
[ |-T|\ [**d**\|\ **l**\|\ **p**][**+r**][**+u**] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-l| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

.. module_common_begins

Description
-----------

Reads pairs of angles in degrees (plus an optional *z*-value) from *table* [or standard
input] and plots them on a **stereonet**, the diagram that structural geologists use to
display the orientation of planes and lines.  Depending on |-T| the two angles are read
as the strike and dip of a plane (using the right-hand rule, so that the plane dips to
the right of the strike direction), as the dip direction and dip of a plane, or as the
trend and plunge of a line.

For each plane we can draw its **cyclographic trace**, i.e., the great circle where the
plane cuts the hemisphere (selected with |-W|), and the **pole** to the plane, i.e., the
point where the normal to the plane pierces the hemisphere (selected with |-S|).  For
each line we only draw the point where it pierces the hemisphere.  Instead of the pole,
|-T|\ **+r** lets you plot a lineation that lies *on* the plane (e.g., a slickenline),
given by its **rake** (also called *pitch*) in a third input column.

A stereonet is not a separate map projection: it is simply one hemisphere of a unit
sphere seen from above, centered on the nadir.  We therefore use one of the two standard
GMT azimuthal projections centered on 0/0, whose default 90-degree horizon is exactly
one hemisphere: |-J|\ **A** (Lambert azimuthal equal-area) gives the equal-area net that
is also known as a **Schmidt net**, while |-J|\ **S** (stereographic) gives the
equal-angle net that is also known as a **Wulff net** (see Figure
:ref:`Stereonets <GMT_stereonets>`).  With that setup north is up, so azimuth is measured
clockwise from the top of the plot, the center of the net is a vertical (90-degree plunge)
direction, and the perimeter is horizontal.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [*annot*\ [/*tick*]]
    Annotate the azimuth around the perimeter of the net every *annot* degrees, with tick
    marks every *tick* degrees [30/10].  If only *annot* is given then *tick* defaults to
    one third of *annot*.  Use **-A0** to leave the perimeter unannotated.

.. _-B:

.. include:: ../../explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

The gridlines requested via |-B| *are* the net: the meridians are the cyclographic traces
of planes striking N-S whose dips step by the grid interval, and the parallels are the
small circles of constant plunge.  Since the net has no meaningful longitude or latitude
annotations you will normally only ask for gridlines, and the two-level mesh that a
stereonet traditionally shows is obtained by giving both a primary and a secondary
interval, e.g., **-Bpg10 -Bsg30** [Default].  Add, e.g.,
``--MAP_GRID_PEN_PRIMARY=0.25p,gray`` to make the fine mesh recede into the background.

.. _-C:

**-C**\ *cpt*
    Give a CPT or specify **-C**\ *color1,color2*\ [*,color3*\ ,...] to build a linear
    continuous CPT from those colors automatically.  The color of each symbol and each
    cyclographic trace is then determined by the *z*-value expected in the third input
    column.  If no argument is given then we select the current CPT.

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Select color or pattern for filling the symbols [Default is no fill].

.. _-J:

**-JA**\|\ **S**\ [0/0/]\ *width*
    Select the type of net, where *width* is its diameter:

    - **A** - Lambert azimuthal equal-area, i.e., a Schmidt net [Default].
    - **S** - Stereographic (equal-angle), i.e., a Wulff net.

    Both projections must be centered on 0/0 and use the default 90-degree horizon, so you
    may skip the center and simply give **-JA**\ *width* or **-JS**\ *width*
    [**-JA**\ 15c].  Any other projection is an error.

.. _-L:

**-L**\ *pen* :ref:`(more ...) <-Wpen_attrib>`
    Set the pen used to outline the symbols selected with |-S|.

.. _-M:

**-M**\ [**c**\|\ **p**]
    Do no plotting.  Instead, convert the input angles to the longitude, latitude
    coordinates that this module would have plotted and write them to standard output, so
    that you can build a custom figure with :doc:`plot </plot>` or process the geometry
    with other modules.  Append **c** to write the cyclographic traces as a multiple
    segment data set, or **p** to write the poles (or their rake points if **-T+r** was
    set, or the lines, if **-Tl**) as a single segment [Default writes the traces for
    planes and the points for lines].

.. _-S:

**-S**\ *symbol*\ [*size*]
    Plot the pole to each plane (or the rake point if **-T+r** was set, or the line
    itself if **-Tl**) using this symbol; see :doc:`plot </plot>` for the available
    symbol codes [**-Sc**\ 0.15c].  Without |-S| no symbols are plotted for planes.

.. _-T:

**-T**\ [**d**\|\ **l**\|\ **p**][**+r**][**+u**]
    Select what the two input angles mean:

    - **d** - Planes given as dip direction and dip.
    - **l** - Lines given as trend and plunge.
    - **p** - Planes given as strike and dip, with the strike following the right-hand
      rule so that the plane dips to the right of the strike direction [Default].

    Optionally, append one or both modifiers:

    - **+r** - Expect a third column with the **rake** (also called *pitch*) of a
      lineation on the plane, e.g., a slickenline, in degrees measured from the strike
      azimuth: 0 is the strike azimuth itself, 90 is the down-dip direction, and 180 is
      the opposite end of the strike.  A negative rake is the common shorthand for a rake
      measured from that opposite end, and is folded into the 0-180 range (so -25 is read
      as 155).  Plot that point instead of the pole.  Not allowed with **-Tl**, since a
      line has no plane to measure the rake on.
    - **+u** - Plot the data on the upper hemisphere [Default is the lower hemisphere,
      which is the convention in structural geology].

.. |Add_-Rgeo| replace:: A stereonet always covers a full hemisphere, so you should not
    need this option [**-Rg**].
.. include:: ../../explain_-Rgeo.rst_

.. |Add_-U| replace:: |Add_-U_links|
.. include:: ../../explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: ../../explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *pen* :ref:`(more ...) <-Wpen_attrib>`
    Set the pen used to draw the cyclographic traces of the planes.  Ignored if **-Tl**,
    since a line has no trace, in which case the pen is used to outline the symbols
    instead.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: ../../explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-bi| replace:: [Default is 2 input columns, or 3 if |-C| is used].
.. include:: ../../explain_-bi.rst_

.. include:: ../../explain_-c.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-l.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-qi.rst_

.. include:: ../../explain_-s.rst_

.. include:: ../../explain_-t.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

Notes
-----

#. Repeat **-l** to label both items: the first label goes with the cyclographic traces and
   the second with the symbols.
#. To draw an empty net, give no input records, e.g., ``gmt stereonet -JA12c < /dev/null``.
#. Field notebooks often list a name before the two angles; use, e.g., **-i**\ 1,2 to skip
   such a leading column.
#. Dips (and plunges) must lie in the 0-90 range and rakes in the 0-180 range, as those are
   the only physically meaningful values.  An out-of-range angle is reported as an error
   rather than plotted, since it would otherwise project onto the far hemisphere and be
   silently clipped away.

.. module_common_ends

Examples
--------

.. include:: ../../explain_example.rst_

.. include:: ../../oneliner_info.rst_

To plot eight fault planes given as *name strike dip* on a 12-centimeter-wide Schmidt net,
drawing the cyclographic traces in red and the poles as blue crosses, try::

    cat << EOF > faults.txt
    FAULT_1  90 30
    FAULT_2 180 45
    FAULT_3 270 60
    FAULT_4   0 15
    FAULT_5  30 45
    FAULT_6 120 48
    FAULT_7 225 27
    FAULT_8 350 80
    EOF
    gmt begin faults
      gmt stereonet faults.txt -i1,2 -JA12c -W1p,red -Sx0.3c -L1p,blue \
        -l"Fault plane" -l"Pole" --MAP_GRID_PEN_PRIMARY=0.25p,gray
    gmt end show

To plot the poles to bedding, measured as *trend plunge*, as red circles on a 10-centimeter
Wulff net without the azimuth ring, try::

    gmt begin bedding
      gmt stereonet bedding.txt -JS10c -Tl -A0 -Sc0.2c -Gred
    gmt end show

To only convert the strike and dip of the faults into the longitudes and latitudes of their
poles, so that you can plot them yourself, try::

    gmt stereonet faults.txt -i1,2 -Mp > poles.txt

To plot fault-slip data, i.e., a fault plane together with the rake of its slickenline
(here strike 315, dip 30, rake 25 measured from the northwest end of the strike, hence a
rake of 180-25=155 in the 0-180 convention used by **-T+r**), try::

    echo 315 30 155 | gmt stereonet -JA10c -Tp+r -W1p,green -Sc0.2c -Ggreen

References
----------

Lisle, R. J., and P. R. Leyshon, 2004, *Stereographic Projection Techniques for Geologists
and Civil Engineers*, 2nd edition, Cambridge University Press.

See Also
--------

:doc:`gmt </gmt>`, :doc:`gmt.conf </gmt.conf>`,
:doc:`basemap </basemap>`,
:doc:`plot </plot>`,
:doc:`rose </rose>`,
:doc:`/supplements/seis/polar`
