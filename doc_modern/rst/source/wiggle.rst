.. index:: ! wiggle

********
wiggle
********

.. only:: not man

    Plot z = f(x,y) anomalies along tracks

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt wiggle** [ *table* ] |-J|\ *parameters* |SYN_OPT-Rz| |-Z|\ *scale*\ [*units*]
[ |-A|\ [\ *azimuth*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *center* ]
[ |-D|\ *refpoint* ]
[ |-F|\ *panel* ]
[ |-G|\ *fill*\ [**+n**\ ][**+p**\ ] ]
[ |-I|\ *fix_az* ]
[ |-T|\ *pen* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
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

**wiggle** reads (*x*,\ *y*,\ *z*) triplets from files [or standard
input] and plots z as a function of distance along track. This means
that two consecutive (*x*,\ *y*) points define the local distance axis,
and the local *z* axis is then perpendicular to the distance axis,
forming a right-handed coordinate system. The
user may set a preferred positive anomaly plot direction, and if the
positive normal is outside the plus/minus 90 degree window around the
preferred direction, then 180 degrees are added to the direction. Either
the positive or the negative wiggle may be shaded.

Required Arguments
------------------

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

.. _-Z:

**-Z**\ *scale*\ [*units*]
    Gives anomaly scale in data-units/distance-unit, where distance-unit is the currently chosen unit specified
    by PROJ_LENGTH_UNIT.  Alternatively, append a distance-unit among the other choices (c\|i\|p).

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

**-A**\ [*azimuth*]
    Sets the preferred positive azimuth. Positive wiggles will
    "gravitate" towards that direction, i.e., azimuths of the
    normal direction to the track will be flipped into the
    -90/+90 degree window centered on *azimuth* and that defines
    the positive wiggle side.  If no azimuth is given the no
    preferred azimuth is enforced.  Default is **-A**\ 0.

.. _-B:

.. include:: explain_-B.rst_

.. _-C:

**-C**\ *center*
    Subtract *center* from the data set before plotting [0].

.. _-D:

**-D**\ [**g**\ \|\ **j**\ \|\ **J**\ \|\ **n**\ \|\ **x**]\ *refpoint*\ \ **+w**\ *length*\ [**+j**\ *justify*]\ [**+a**\ **l**\ \|\ **r**\ ]\ [**+o**\ *dx*\ [/*dy*]]\ [**+l**\ [*label*]]
    Defines the reference point on the map for the vertical scale bar using one of four coordinate systems:
    (1) Use **-Dg** for map (user) coordinates, (2) use **-Dj** or **-DJ** for setting *refpoint* via
    a 2-char justification code that refers to the (invisible) map domain rectangle,
    (3) use **-Dn** for normalized (0-1) coordinates, or (4) use **-Dx** for plot coordinates
    (inches, cm, etc.).  All but **-Dx** requires both **-R** and **-J** to be specified.
    Append **+w** followed by the *length* or the scale bar in data (*z) units.
    By default, the anchor point on the scale is assumed to be the middle left corner (ML), but this
    can be changed by appending **+j** followed by a 2-char justification code *justify* (see :doc:`text`).
    Note: If **-Dj** is used then *justify* defaults to the same as *refpoint*,
    if **-DJ** is used then *justify* defaults to the mirror opposite of *refpoint*. Consequently,
    **-DJ** is used to place a scale outside the map frame while **-Dj** is used to place it inside the frame.
    Add **+o** to offset the vertical scale bar by *dx*/*dy* away from the *refpoint* point in
    the direction implied by *justify* (or the direction implied by **-Dj** or **-DJ**).
    Move scale label to the left side with **+al** [Default is to the right of the scale].
    Append **+l** to set the *z* unit label that is used in the scale label [no unit].
    The **FONT\_ANNOT\_PRIMARY** is used for the font setting.

.. _-F:

**-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*]]]
    Without further options, draws a rectangular border around the vertical scale bar using
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

**-G**\ *fill*\ [**+n**\ ][**+p**\ ] :ref:`(more ...) <-Gfill_attrib>`
    Set fill shade, color or pattern for positive and/or negative
    wiggles [Default is no fill]. Optionally, append **+p** to fill
    positive areas (this is the default behavior). Append **+n** to fill
    negative areas. Append **+n+p** to fill both positive and negative
    areas with the same fill.

.. _-I:

**-I**\ *fix_az*
    Set a fixed azimuth projection for wiggles [Default uses track
    azimuth, but see **-A**]. With this option, the calculated
    track-normal azimuths are overridden by *fixed_az*.

.. include:: explain_-Jz.rst_

.. _-T:

**-T**\ *pen*
    Draw track [Default is no track]. Append pen attributes to use
    [Defaults: width = 0.25p, color = black, style = solid]. 

.. _-U:

.. include:: explain_-U.rst_

.. _-:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *pen*
    Specify outline pen attributes [Default is no outline].

.. |Add_-bi| replace:: [Default is 3 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

Examples
--------

To plot the magnetic anomaly stored in the file track.xym along track @
500 nTesla/cm (after removing a mean value of 32000 nTesla), using a
15-cm-wide Polar Stereographic map ticked every 5 degrees in Portrait
mode, with positive anomalies in red on a blue track of width 0.25
points, use

   ::

    gmt wiggle track.xym -R-20/10/-80/-60 -JS0/90/15c -Z500 -B5
                 -C32000 -Gred -T0.25p,blue -DjRM+w1000+lnT -V -pdf track_xym

and the positive anomalies will in general point in the north direction.
We used **-D** to place a vertical scale bar indicating a 1000 nT anomaly.
To instead enforce a fixed azimuth of 45 for the positive wiggles, we add **-I**
and obtain

   ::

    gmt wiggle track.xym -R-20/10/-80/-60 -JS0/90/15c -Z1000 -B5
              -C32000 -Gred -I45 -T0.25p,blue -DjRM+w1000+lnT -V -pdf track_xym

Bugs
----

Sometimes the (x,y) coordinates are not printed with enough significant
digits, so the local perpendicular to the track swings around a lot. To
see if this is the problem, you should do this:

   ::

    awk '{ if (NR > 1) print atan2(y-$1, x-$2); y=$1; x=$2; }' yourdata.xyz | more

(note that output is in radians; on some machines you need "nawk" to do
this). Then if these numbers jump around a lot, you may do this:

   ::

    awk '{ print NR, $0 }' yourdata.xyz | filter1d -Fb5 -N4/0
    --FORMAT_FLOAT_OUT=%.12g > smoothed.xyz

which performs a 5-point boxcar filter, and plot this data set instead.

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`filter1d`,
:doc:`basemap`,
:doc:`splitxyz`
