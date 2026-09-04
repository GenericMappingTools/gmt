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
[ |-J|\ **A**\|\ **S**\ *width* ]
[ |-A|\ [*annot*\ [/*tick*]] ]
[ |SYN_OPT-B| ]
[ |-G|\ *fill* ]
[ |-L|\ *pen* ]
[ |-S|\ *symbol*\ [*size*] ]
[ |-T|\ [**d**\|\ **l**\|\ **p**][**+u**] ]
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

Reads pairs of angles in degrees from *table* [or standard
input] and plots them on a **stereonet**, the diagram that structural geologists use to
display the orientation of planes and lines.  Depending on |-T| the two angles are read
as the strike and dip of a plane (using the right-hand rule, so that the plane dips to
the right of the strike direction), as the dip direction and dip of a plane, or as the
trend and plunge of a line.

For each plane we can draw its **cyclographic trace**, i.e., the great circle where the
plane cuts the hemisphere (selected with |-W|), and the **pole** to the plane, i.e., the
point where the normal to the plane pierces the hemisphere (selected with |-S|).  For
each line we only draw the point where it pierces the hemisphere.

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
    marks every *tick* degrees [30/10 if **-A** is given with no argument].  If only *annot*
    is given then *tick* defaults to one third of *annot*.  Without **-A** no azimuth ring
    is drawn at all; **-A0** is the same as omitting **-A**.

.. _-B:

.. include:: ../../explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

The gridlines requested via |-B| *are* the net: the meridians are the cyclographic traces
of planes striking N-S whose dips step by the grid interval, and the parallels are the
small circles of constant plunge.  Since the net has no meaningful longitude or latitude
annotations you will normally only ask for gridlines, and the two-level mesh that a
stereonet traditionally shows is obtained by giving both a primary and a secondary
interval, e.g., **-Bpg10 -Bsg30**.  Without |-B| no frame at all is drawn, not even the
perimeter of the net; give a bare **-B** for the classic two-level mesh [-Bpg10 -Bsg30].
A |-B| that only carries frame settings, such as a **-B+t**\ *title*, gets that same mesh,
so you can title a default net without spelling out the intervals.
Add, e.g., ``--MAP_GRID_PEN_PRIMARY=0.25p,gray`` to make the fine mesh recede into the
background.

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Select color or pattern for filling the symbols [Default is no fill].

.. _-J:

**-JA**\|\ **S**\ *width*
    Select the type of net, where *width* is its diameter:

    - **A** - Lambert azimuthal equal-area, i.e., a Schmidt net [Default].
    - **S** - Stereographic (equal-angle), i.e., a Wulff net.

    Both projections use the default 90-degree horizon.  The net is always centered on 0/0,
    so give the width only, without a center: any other projection, or an explicit center,
    is an error.  If |-J| is skipped we inherit the net of an earlier **stereonet** in the
    same figure, else we default to **-JA**\ 15c.

.. _-L:

**-L**\ *pen* :ref:`(more ...) <-Wpen_attrib>`
    Set the pen used to outline the symbols selected with |-S|.

.. _-S:

**-S**\ *symbol*\ [*size*]
    Plot the pole to each plane (or the line itself if **-Tl**) using this symbol; see
    :doc:`plot </plot>` for the available symbol codes [**-Sc**\ 0.15c].  Without |-S| no
    symbols are plotted for planes.

.. _-T:

**-T**\ [**d**\|\ **l**\|\ **p**][**+u**]
    Select what the two input angles mean:

    - **d** - Planes given as dip direction and dip.
    - **l** - Lines given as trend and plunge.
    - **p** - Planes given as strike and dip, with the strike following the right-hand
      rule so that the plane dips to the right of the strike direction [Default].

    Optionally, append modifier:

    - **+u** - Plot the data on the upper hemisphere [Default is the lower hemisphere,
      which is the convention in structural geology].

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
    since a line has no trace; use |-L| to outline the symbols instead.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: ../../explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-bi| replace:: [Default is 2 input columns].
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
#. To draw an empty net, give no input records, e.g., ``gmt stereonet -JA12c -B < /dev/null``.
   Without **-B** no frame at all is drawn, not even the perimeter of the net.
#. Field notebooks often list a name before the two angles; use, e.g., **-i**\ 1,2 to skip
   such a leading column.
#. Dips (and plunges) must lie in the 0-90 range, as that is the only physically meaningful
   value.  An out-of-range angle is reported as an error rather than plotted, since it would
   otherwise project onto the far hemisphere and be silently clipped away.
#. There is no **-R** option: a stereonet always covers a full hemisphere, and giving one
   is rejected as an error rather than silently ignored.

.. module_common_ends

Examples
--------

.. include:: ../../explain_example.rst_

.. include:: ../../oneliner_info.rst_

To plot eight fault planes given as *strike dip* on a 12-centimeter-wide Schmidt net,
drawing the cyclographic traces in red and the poles as blue crosses, try::

    cat << EOF > faults.txt
    90 30
    180 45
    270 60
    0 15
    30 45
    120 48
    225 27
    350 80
    EOF
    gmt begin faults
      gmt stereonet faults.txt -JA12c -B -W1p,red -Sx0.3c -L1p,blue \
        -l"Fault plane" -l"Pole" --MAP_GRID_PEN_PRIMARY=0.25p,gray
    gmt end show

To plot the poles to bedding, measured as *trend plunge*, as red circles on a 10-centimeter
Wulff net without the azimuth ring, try::

    gmt begin bedding
      gmt stereonet bedding.txt -JS10c -B -Tl -A0 -Sc0.2c -Gred
    gmt end show

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
