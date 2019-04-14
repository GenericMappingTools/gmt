.. index:: ! project

*******
project
*******

.. only:: not man

    project - Project data onto lines or great circles, generate tracks, or translate coordinates

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt project** [ *table* ] |-C|\ *cx*/*cy* [ |-A|\ *azimuth* ]
[ |-E|\ *bx*/*by* ] [ |-F|\ *flags* ]
[ |-G|\ *dist*\ [/*colat*][**+h**] ]
[ |-L|\ [**w**\ \|\ *l\_min*/*l\_max*] ]
[ |-N| ] [ |-Q| ] [ |-S| ]
[ |-T|\ *px*/*py* ]
[ |SYN_OPT-V| ]
[ |-W|\ *w\_min*/*w\_max* ]
[ |-Z|\ *major*/*minor*/*azimuth*\ [**+e**\ ] ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**project** reads arbitrary (*x*, *y*\ [,\ *z*]) data from standard input
[or *table* ] and writes to standard output any combination of (*x*,
*y*, *z*, *p*, *q*, *r*, *s*), where (*p*, *q*) are the coordinates in
the projection, (*r*, *s*) is the position in the (*x*, *y*) coordinate
system of the point on the profile (*q* = 0 path) closest to (*x*, *y*),
and *z* is all remaining columns in the input (beyond the required *x*
and *y* columns).

Alternatively, **project** may be used to generate (*r*, *s*, *p*)
triples at equal increments *dist* along a profile. In this case (
**-G** option), no input is read.

Projections are defined in any (but only) one of three ways:

(Definition 1) By a Center **-C** and an Azimuth **-A** in degrees
clockwise from North.

(Definition 2) By a Center **-C** and end point E of the projection path **-E**.

(Definition 3) By a Center **-C** and a roTation pole position **-T**.

To spherically project data along a great circle path, an oblique
coordinate system is created which has its equator along that path, and
the zero meridian through the Center. Then the oblique longitude
(*p*) corresponds to the distance from the Center
along the great circle, and the oblique latitude (*q*) corresponds to
the distance perpendicular to the great circle path. When moving in the
increasing (*p*) direction, (toward *B* or in the
*azimuth* direction), the positive (*q*) direction is to your left. If a
Pole has been specified, then the positive (*q*) direction is toward the
pole.

To specify an oblique projection, use the **-T** option to set the Pole.
Then the equator of the projection is already determined and the **-C**
option is used to locate the *p* = 0 meridian. The Center *cx/cy* will
be taken as a point through which the *p* = 0 meridian passes. If you do
not care to choose a particular point, use the South pole (*ox* = 0,
*oy* = -90).

Data can be selectively windowed by using the **-L** and **-W** options.
If **-W** is used, the projection Width is set to use only points with
*w\_min* < q < *w\_max*. If **-L** is set, then the Length is set to use
only those points with *l\_min* < p < *l\_max*. If the **-E** option has
been used to define the projection, then **-Lw** may be selected to
window the length of the projection to exactly the span from **O** to
**B**.

Flat Earth (Cartesian) coordinate transformations can also be made. Set
**-N** and remember that *azimuth* is clockwise from North (the *y*
axis), NOT the usual cartesian theta, which is counterclockwise from the
*x* axis. *azimuth* = 90 - theta.

No assumptions are made regarding the units for *x*, *y*, *r*, *s*, *p*,
*q*, *dist*, *l\_min*, *l\_max*, *w\_min*, *w\_max*. If **-Q** is
selected, map units are assumed and *x*, *y*, *r*, *s* must be in
degrees and *p*, *q*, *dist*, *l\_min*, *l\_max*, *w\_min*, *w\_max*
will be in km.

Calculations of specific great-circle and geodesic distances or for
back-azimuths or azimuths are better done using :doc:`mapproject`.

**project** is CASE SENSITIVE. Use UPPER CASE for all one-letter
designators which begin optional arguments. Use lower case for the
xyzpqrs letters in **F**\ *flags*. 

Required Arguments
------------------

.. _-C:

**-C**\ *cx*/*cy*
    *cx/cy* sets the origin of the projection, in Definition 1 or 2. If
    Definition 3 is used (**-T**), then *cx/cy* are the coordinates of a
    point through which the oblique zero meridian (*p* = 0) should pass.
    The *cx/cy* is not required to be 90 degrees from the pole.

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

**-A**\ *azimuth*
    *azimuth* defines the azimuth of the projection (Definition 1).

.. _-E:

**-E**\ *bx*/*by*
   *bx/by* defines the end point of the projection path (Definition 2).

.. _-F:

**-F**\ *flags*
    Specify your desired output using any combination of **xyzpqrs**, in
    any order [Default is **xyzpqrs**]. Do not space between the letters.
    Use lower case. The output will be ASCII (or binary, see **-bo**)
    columns of values corresponding to your *flags*. The **z** flag is
    special and refers to all numerical columns beyond the leading **x** and **y** in
    your input record.  If output format is ASCII then **z** also includes any
    trailing text (which is placed at the end of the record regardless
    of the order of **z** in *flags*). Note: If **-G** is selected, then the
    output order is hardwired to be **rsp** and **-F** is not allowed.

.. _-G:

**-G**\ *dist*\ [/*colat*][**+h**]
    Generate mode. No input is read. Create (*r*, *s*, *p*) output
    points every *dist* units of *p*. See **-Q** option. Alternatively,
    append **/**\ *colat* for a small circle instead [Default is a
    colatitude of 90, i.e., a great circle]. Use **-C** and **-E** to
    generate a circle that goes through the center and end point. Note,
    in this case the center and end point cannot be farther apart than
    2\*\|\ *colat*\ \|. Finally, if you append **+h** the we will report
    the position of the pole as part of the segment header [no header].

.. _-L:

**-L**\ [**w**\ \|\ *l\_min*/*l\_max*]
    Length controls. Project only those points whose *p* coordinate is
    within *l\_min* < *p* < *l\_max*. If **-E** has been set, then you
    may alternatively use **-Lw** to stay within the distance from **C** to **E**.

.. _-N:

**-N**
    Flat Earth. Make a Cartesian coordinate transformation in the plane.
    [Default uses spherical trigonometry.]

.. _-Q:

**-Q**
    Map type units, i.e., project assumes *x*, *y*, *r*, *s* are in
    degrees while *p*, *q*, *dist*, *l\_min*, *l\_max*, *w\_min*,
    *w\_max* are in km. If **-Q** is not set, then all these are assumed
    to be in the same units.

.. _-S:

**-S**
    Sort the output into increasing *p* order. Useful when projecting
    random data into a sequential profile.

.. _-T:

**-T**\ *px*/*py*
    *px/py* sets the position of the rotation pole of the projection.
    (Definition 3). 

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *w\_min*/*w\_max*
    Width controls. Project only those points whose *q* coordinate is
    within *w\_min* < *q* < *w\_max*. 

**-Z**\  *major*/*minor*/*azimuth*\ [**+e**\ ] ]
    Used in conjunction with **-C** (sets its center) and **-G** (sets the
    distance increment) to create the coordinates of an ellipse
    with *major* and *minor* axes given in km (unless **-N** is given) and the *azimuth* of the
    major axis in degrees.  Append **+e** to adjust the increment set via
    **-G** so that the the ellipse has equal distance increments [Default
    uses the given increment and closes the ellipse].


.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is given by **-F** or **-G**]. 
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

.. include:: explain_-s.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Examples
--------

To generate points every 10km along a great circle from 10N,50W to 30N,10W:

   ::

    gmt project -C-50/10 -E-10/30 -G10 -Q > great_circle_points.xyp

(Note that great_circle_points.xyp could now be used as input for :doc:`grdtrack`, etc. ).

To generate points every 1 degree along a great circle from 30N,10W with
azimuth 30 and covering a full 360, try:

   ::

    gmt project -C10W/30N -A30 -G1 -L-180/180 > great_circle.txt

To generate points every 10km along a small circle of colatitude 60 from 10N,50W to 30N,10W:

   ::

    gmt project -C-50/10 -E-10/30 -G10/60 -Q > small_circle_points.xyp

To create a partial small circle of colatitude 80 about a pole at
40E,85N, with extent of 45 degrees to either side of the meridian
defined by the great circle from the pole to a point 15E,15N, try

   ::

    gmt project -C15/15 -T40/85 -G1/80 -L-45/45 > some_circle.xyp

To generate points approximately every 10km along a an ellipse centered on (30W,70N) with
major axis of 1500 km with azimuth of 30 degree and a minor axis of 600 km, try

   ::

    gmt project -C-30/70 -G10 -Z1500/600/30+e > ellipse.xyp

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
new origin is at 5,3 and the new *x* axis (*p*) makes
an angle of 20 degrees with the old *x* axis, use:

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

    gmt grd2xyz pacific_topo.nc | project -T-75/68 -C0/-90 -Fxyq | xyz2grd -Retc -Ietc -Cflow.nc

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
:doc:`grdproject`,
:doc:`grdtrack`
