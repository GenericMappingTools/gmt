.. index:: ! project
.. include:: module_core_purpose.rst_

*******
project
*******

|project_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt project** [ *table* ]
|-C|\ *cx*/*cy*
[ |-A|\ *azimuth* ]
[ |-E|\ *bx*/*by* ]
[ |-F|\ *flags* ]
[ |-G|\ *dist*\ [*unit*][/*colat*][**+c**][**+h**][**+n**] ]
[ |-L|\ [**w**\|\ *lmin*/*lmax*] ]
[ |-N| ]
[ |-Q| ]
[ |-S| ]
[ |-T|\ *px*/*py* ]
[ |SYN_OPT-V| ]
[ |-W|\ *wmin*/*wmax* ]
[ |-Z|\ *major*\ [*unit*][/*minor*/*azimuth*][**+e**] ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**project** reads arbitrary (:math:`x`, :math:`y` [,\ *z*]) data from standard input
[or *table*] and writes to standard output any combination of (:math:`x, y`, *z*,
:math:`p, q, r, s`), where (:math:`p, q`) are the coordinates in
the projection, (:math:`r, s`) is the position in the (:math:`x, y`) coordinate
system of the point on the profile (:math:`q = 0` path) closest to (:math:`x, y`),
and *z* is all remaining columns in the input (beyond the required :math:`x`
and :math:`y` columns).

Alternatively, **project** may be used to generate (:math:`r,s,p`)
triplets at equal increments *dist* along a profile using |-G|. In this case, no input is read.

Projections are defined in one of three ways:

  1. By a center (*cx*/*cy*) using |-C| and an azimuth in degrees clockwise from North using |-A|.
  2. By a center (*cx*/*cy*) (e.g., start point) using |-C| and end point (*bx*/*by*) of the projection path using |-E|.
  3. By a center (*cx*/*cy*) using |-C| and a rotation pole position (*px*/*py*) using |-T| (not allowed when a
     Cartesian transformation is set by |-N|).

To spherically project data along a great circle path, an oblique coordinate
system is created which has its equator along that path, and the zero meridian
through *cx*/*cy*. Then the oblique longitude (:math:`p`) corresponds to the
distance from *cx*/*cy* along the great circle, and the oblique latitude (*q*)
corresponds to the distance perpendicular to the great circle path. When moving
in the increasing (:math:`p`) direction, (in the direction set by
|-A|\ *azimuth* ), the positive (:math:`q`) direction is to the left. If a pole
has been specified by |-T|, then the positive (*q*) direction is toward the
pole.

To specify an oblique projection, use the |-T| option to set the pole.
Then the equator of the projection is already determined and the |-C|
option is used to locate the :math:`p = 0` meridian. The center *cx/cy* will
be taken as a point through which the :math:`p = 0` meridian passes. If you do
not care to choose a particular point, use the South pole (*cx* = 0,
*cy* = -90).

Data can be selectively windowed by using the |-L| and |-W| options.
If |-W| is used, the projection width is set to use only points with
:math:`w_{min} < q < w_{max}`. If |-L| is set, then the length is set to use
only those points with :math:`l_{min} < p < l_{max}`. If the |-E| option has
been used to define the projection, then |-L|\ **w** may be selected to
window the length of the projection to exactly the span from the center (|-C|) to
to the endpoint (|-E|).

Flat Earth (Cartesian) coordinate transformations can also be made. Set
|-N| and remember that *azimuth* is clockwise from North (the :math:`y`
axis), **not** the usual cartesian theta, which is counterclockwise from the
:math:`x` axis. (i.e., :math:`azimuth = 90 - theta`).

No assumptions are made regarding the units for :math:`x, y, r, s, p, q`, *dist*,
:math:`l_{min}, l_{max}, w_{min}, w_{max}`. However, if |-Q| is
selected, map units are assumed and :math:`x, y, r, s`, must be in
degrees and :math:`p, q`, *dist*, :math:`l_{min}, l_{max}, w_{min}, w_{max}`
will be in km.

Calculations of specific great-circle and geodesic distances or for
back-azimuths or azimuths are better done using :doc:`mapproject` as
:doc:`project` is strictly spherical.


.. figure:: /_images/project_setup.*
   :width: 500 px
   :align: center

   Explanation of the coordinate system utilized by project.  The input point
   (red circle) is given in the original *x-y* (or *lon-lat*) coordinate system and is projected to
   the *p-q* coordinate system, defined by the center (**C**) and either the end-point
   (**E**) or azimuth (:math:`\alpha`), or for geographic data a rotation pole **T** (not shown).
   The blue point has projected coordinates (p,0) and is reported as (r,s) in the original
   coordinate system.  Options |-L| (limit range of *p*) and |-W| (limit range of *q*)
   can be used to exclude data outside the specified limits (light gray area).

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-C:

**-C**\ *cx*/*cy*
    Set the origin *cx*/*cy* of the projection when used with |-A| or |-E| or set the coordinates *cx*/*cy* of a point
    through which the oblique zero meridian (:math:`p = 0`) should pass when used with |-T|. *cx*/*cy* is not required
    to be 90 degrees from the pole set by |-T|.

Optional Arguments
------------------

.. _-A:

**-A**\ *azimuth*
    Set the *azimuth* of the projection. The *azimuth* is clockwise from North (the :math:`y` axis) regardless of
    whether spherical or Cartesian coordinate transformation is applied.

.. _-E:

**-E**\ *bx*/*by*
   Set the end point *bx/by* of the projection path.

.. _-F:

**-F**\ *flags*
    Specify the desired output using any combination of *xypqrsz* in any order, where (:math:`p, q`) are the
    coordinates in the projection, (:math:`r, s`) is the position in the (:math:`x, y`) coordinate system of the point
    on the profile (:math:`q = 0` path) closest to (:math:`x, y`), and *z* is all remaining columns in the input
    (beyond the required :math:`x` and :math:`y` columns). [Default is *xypqrsz*]. If output format is ASCII then
    *z* also includes any trailing text (which is placed at the end of the record regardless of the order of *z*
    in *flags*). Use lower case and do not add spaces between the letters. **Note**: If |-G| is selected, then the
    output order is set to be *rsp* and |-F| is not allowed.

.. _-G:

**-G**\ *dist*\ [*unit*][/*colat*][**+c**][**+h**][**+n**]
    Create (*r*, *s*, *p*) output points every *dist* units of *p*, assuming all units are the same unless
    :math:`x, y, r, s` are set to degrees using |-Q|. No input is read when |-G| is used. See `Units`_ for
    selecting geographic distance units [km]. The following directives and modifiers are supported:

    - Optionally, append /*colat* for a small circle instead [Default is a colatitude of 90, i.e., a great circle]. Note,
      when using |-C| and |-E| to generate a circle that goes through the center and end point, the center and end point
      cannot be farther apart than :math:`2|colat|`.
    - Optionally, append **+c** when using |-T| to calculate the colatitude that will lead to the small circle
      going through the center *cx*/*cy*.
    - Optionally, append **+h** to report the position of the pole as part of the segment header when using |-T|
      [Default is no header].
    - Optionally, append **+n** to indicate a desired number of points rather than an increment. Requires |-C| and |-E| or |-Z|
      so that a length can be computed.

.. _-L:

**-L**\ [**w**\|\ *lmin*/*lmax*]
    Specify length controls for the projected points. Project only those points whose *p* coordinate is
    within :math:`l_{min} < p < l_{max}`. If |-E| has been set, then you may alternatively use **-Lw** to stay within
    the distance from *cx*/*cy* to *bx*/*by*.

.. _-N:

**-N**
    Specify the Flat Earth case (i.e., Cartesian coordinate transformation in the plane).
    [Default uses spherical trigonometry.]

.. _-Q:

**-Q**
    Specify that  :math:`x`, :math:`y`, *r*, *s* are in degrees while *p*, *q*, *dist*, *lmin*, *lmax*, *wmin*,
    *wmax* are in km. If |-Q| is not set, then all these are assumed to be in the same units.

.. _-S:

**-S**
    Sort the output into increasing *p* order. Useful when projecting
    random data into a sequential profile.

.. _-T:

**-T**\ *px*/*py*
    Set the position of the rotation pole of the projection as *px/py*.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *wmin*/*wmax*
    Specify width controls for the projected points. Project only those points whose *q* coordinate is
    within :math:`w_{min} < q < w_{max}`.

.. _-Z:

**-Z**\ *major*\ [*unit*][/*minor*/*azimuth*][**+e**]
    Create the coordinates of an ellipse with *major* and *minor* axes given in km (unless |-N| is given for a
    Cartesian ellipse) and the *azimuth* of the major axis in degrees; used in conjunction with |-C| (sets its center)
    and |-G| (sets the distance increment). **Note**: For the Cartesian ellipse (which requires |-N|), we expect
    *direction* counter-clockwise from the horizontal instead of an *azimuth*. A geographic *major* may be specified
    in any desired unit [Default is km] by appending the unit (e.g., 3d for degrees); if so we assume the *minor* axis
    and the increment are also given in the same unit (see `Units`_).  For degenerate ellipses you can just supply a
    single *diameter* instead. The following modifiers are supported:

    - Append **+e** to adjust the increment set via |-G| so that the ellipse has equal distance increments [Default
      uses the given increment and closes the ellipse].

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is given by |-F| or |-G|].
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-q.rst_

.. include:: explain_-s.rst_

.. include:: explain_colon.rst_

Units
-----

For map distance unit, append *unit* **d** for arc degree, **m** for arc
minute, and **s** for arc second, or **e** for meter [Default unless stated otherwise], **f**
for foot, **k** for km, **M** for statute mile, **n** for nautical mile,
and **u** for US survey foot.

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Examples
--------

.. include:: explain_example.rst_

To project the remote data sets ship_03.txt (lon,lat,depth) onto a great circle specified by
the center (330,-18) and rotation pole (53,21) and sort the records on the projected distances along
that circle and only output the distance and the depths, try::

    gmt project @ship_03.txt -C330/-18 -T53/21 -S -Fpz -Q > ship_proj.txt

To generate points every 10 km along a great circle from 10N,50W to 30N,10W:

::

  gmt project -C-50/10 -E-10/30 -G10 -Q > great_circle_points.xyp

(Note that great_circle_points.xyp could now be used as input for :doc:`grdtrack`, etc. ).

To generate points every 1 degree along a great circle from 30N,10W with
azimuth 30 and covering a full 360, try:

::

  gmt project -C10W/30N -A30 -G1 -L-180/180 > great_circle.txt

To generate points every 10 km along a small circle of colatitude 60 from 10N,50W to 30N,10W:

::

  gmt project -C-50/10 -E-10/30 -G10/60 -Q > small_circle_points.xyp

To create a partial small circle of colatitude 80 about a pole at
40E,85N, with extent of 45 degrees to either side of the meridian
defined by the great circle from the pole to a point 15E,15N, try

::

  gmt project -C15/15 -T40/85 -G1/80 -L-45/45 > some_circle.xyp

To generate points approximately every 10 km along an ellipse centered on (30W,70N) with
major axis of 1500 km with azimuth of 30 degree and a minor axis of 600 km, try

::

  gmt project -C-30/70 -G10 -Z1500/600/30+e -Q > ellipse.xyp

To project the shiptrack gravity, magnetics, and bathymetry in
c2610.xygmb along a great circle through an origin at 30S, 30W, the
great circle having an azimuth of N20W at the origin, keeping only the
data from NE of the profile and within ±\ 500 km of the origin, run:

::

  gmt project c2610.xygmb -C-30/-30 -A-20 -W-10000/0 -L-500/500 -Fpz -Q > c2610_projected.pgmb

(Note in this example that **-W**-10000/0 is used to admit any value
with a large negative *q* coordinate. This will take those points which
are on our right as we walk along the great circle path, or to the NE in this example.)

To make a Cartesian coordinate transformation of mydata.xy so that the
new origin is at 5,3 and the new :math:`x` axis (*p*) makes
an angle of 20 degrees with the old :math:`x` axis, use:

::

  gmt project mydata.xy -C5/3 -A70 -Fpq > mydata.pq

To take data in the file pacific.lonlat and transform it into oblique
coordinates using a pole from the hotspot reference frame and placing
the oblique zero meridian (*p* = 0 line) through Tahiti, run:

::

  gmt project pacific.lonlat -T-75/68 -C-149:26/-17:37 -Fpq > pacific.pq

Suppose that pacific_topo.nc is a grid file of bathymetry, and you want
to make a file of flowlines in the hotspot reference frame. If you run:

::

  gmt grd2xyz pacific_topo.nc | gmt project -T-75/68 -C0/-90 -Fxyq | gmt xyz2grd -Retc -Ietc -Cflow.nc

then flow.nc is a file in the same area as pacific_topo.nc, but flow
contains the latitudes about the pole of the projection. You now can use
grdcontour on flow.nc to draw lines of constant oblique latitude, which
are flow lines in the hotspot frame.

If you have an arbitrarily rotation pole *px/py* and you would like to
draw an oblique small circle on a map, you will first need to make a
file with the oblique coordinates for the small circle (i.e., lon =
0-360, lat is constant), then create a file with two records: the north
pole (0/90) and the origin (0/0), and find what their oblique
coordinates are using your rotation pole. Now, use the projected North
pole and origin coordinates as the rotation pole and center,
respectively, and project your file as in the pacific example above.
This gives coordinates for an oblique small circle.

See Also
--------

:doc:`fitcircle`,
:doc:`gmt`,
:doc:`gmtvector`,
:doc:`grdtrack`,
:doc:`mapproject`,
:doc:`grdproject`
