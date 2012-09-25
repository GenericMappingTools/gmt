*******
project
*******

project - Project table data onto lines or great circles, generate
tracks, or translate coordinates

`Synopsis <#toc1>`_
-------------------

**project** [ *table* ] **-C**\ *cx*/*cy* [ **-A**\ *azimuth* ] [
**-E**\ *bx*/*by* ] [ **-F**\ *flags* ] [
**-G**\ *dist*\ [/*colat*][**+**\ ] ] [
**-L**\ [**w**\ ][\ *l\_min*/*l\_max*] ] [ **-N** ] [ **-Q** ] [ **-S**
] [ **-T**\ *px*/*py* ] [ **-V**\ [*level*\ ] ] [
**-W**\ *w\_min*/*w\_max* ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-s**\ [*cols*\ ][\ **a**\ \|\ **r**] ] [
**-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**project** reads arbitrary (*x*, *y*\ [,*z*]) data from standard input
[or *infile* ] and writes to standard output any combination of (*x*,
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

(Definition 2) By a Center **-C** and end point E of the projection path
**-E**.

(Definition 3) By a Center **-C** and a roTation pole position **-T**.

To spherically project data along a great circle path, an oblique
coordinate system is created which has its equator along that path, and
the zero meridian through the Center. Then the oblique `longitude
(*p*) <longitude.p.html>`_ corresponds to the distance from the Center
along the great circle, and the oblique latitude (*q*) corresponds to
the distance perpendicular to the great circle path. When moving in the
`increasing (*p*) <increasing.p.html>`_ direction, (toward *B* or in the
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
back-azimuths or azimuths are better done using **mapproject**.

**project** is CASE SENSITIVE. Use UPPER CASE for all one-letter
designators which begin optional arguments. Use lower case for the
xyzpqrs letters in **-flags**.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-C**\ *cx*/*cy*
    *cx/cy* sets the origin of the projection, in Definition 1 or 2. If
    Definition 3 is used (**-T**), then *cx/cy* are the coordinates of a
    point through which the oblique zero meridian (*p* = 0) should pass.
    The *cx/cy* is not required to be 90 degrees from the pole.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-A**\ *azimuth*
    *azimuth* defines the azimuth of the projection (Definition 1).
**-E**\ *bx*/*by*
    *bx/by* defines the end point of the projection path (Definition 2).
**-F**\ *flags*
    Specify your desired output using any combination of *xyzpqrs*, in
    any order. Do not space between the letters. Use lower case. The
    output will be ASCII (or binary, see **-bo**\ [*ncols*\ ][*type*\ ])
    columns of values corresponding to *xyzpqrs* [Default]. If both
    input and output are using ASCII format then the *z* data are
    treated as textstring(s). If the **-G** option is selected, the
    output will be *rsp*.
**-G**\ *dist*\ [/*colat*]
    Generate mode. No input is read. Create (*r*, *s*, *p*) output
    points every *dist* units of *p*. See **-Q** option. Alternatively,
    append **/**\ *colat* for a small circle instead [Default is a
    colatitude of 90, i.e., a great circle]. Use **-C** and **-E** to
    generate a circle that goes through the center and end point. Note,
    in this case the center and end point cannot be farther apart than
    2\*\|\ *colat*\ \|. Finally, if you append **+** the we will report
    the position of the pole as part of the segment header [no header].
**-L**\ [**w**\ ][\ *l\_min*/*l\_max*]
    Length controls. Project only those points whose *p* coordinate is
    within *l\_min* < *p* < *l\_max*. If **-E** has been set, then you
    may use **-Lw** to stay within the distance from **C** to **E**.
**-N**
    Flat Earth. Make a Cartesian coordinate transformation in the plane.
    [Default uses spherical trigonometry.]
**-Q**
    Map type units, i.e., project assumes *x*, *y*, *r*, *s* are in
    degrees while *p*, *q*, *dist*, *l\_min*, *l\_max*, *w\_min*,
    *w\_max* are in km. If **-Q** is not set, then all these are assumed
    to be in the same units.
**-S**
    Sort the output into increasing *p* order. Useful when projecting
    random data into a sequential profile.
**-T**\ *px*/*py*
    *px/py* sets the position of the rotation pole of the projection.
    (Definition 3).
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**\ *w\_min*/*w\_max*
    Width controls. Project only those points whose *q* coordinate is
    within *w\_min* < *q* < *w\_max*.
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is given by **-F** or **-G**].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-s**\ [*cols*\ ][\ **a**\ \|\ **r**] (\*)
    Set handling of NaN records.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Ascii Format Precision <#toc6>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Examples <#toc7>`_
-------------------

To generate points every 10km along a great circle from 10N,50W to
30N,10W:

project -C-50/10 -E-10/30 -G10 -Q > great\_circle\_points.xyp

(Note that great\_circle\_points.xyp could now be used as input for
**grdtrack**, etc. ).

To generate points every 10km along a small circle of colatitude 60 from
10N,50W to 30N,10W:

project -C-50/10 -E-10/30 -G10/60 -Q > small\_circle\_points.xyp

To create a partial small circle of colatitude 80 about a pole at
40E,85N, with extent of 45 degrees to either side of the meridian
defined by the great circle from the pole to a point 15E,15N, try

project -C15/15 -T40/85 -G1/80 -L-45/45 > some\_circle.xyp

To project the shiptrack gravity, magnetics, and bathymetry in
c2610.xygmb along a great circle through an origin at 30S, 30W, the
great circle having an azimuth of N20W at the origin, keeping only the
data from NE of the profile and within +/- 500 km of the origin, run:

project c2610.xygmb -C-30/-30 -A-20 -W-10000/0 -L-500/500 -Fpz -Q >
c2610\_projected.pgmb

(Note in this example that **-W**-10000/0 is used to admit any value
with a large negative *q* coordinate. This will take those points which
are on our right as we walk along the great circle path, or to the NE in
this example.)

To make a Cartesian coordinate transformation of mydata.xy so that the
new origin is at 5,3 and the new *x* `axis (*p*) <axis.p.html>`_ makes
an angle of 20 degrees with the old *x* axis, use:

project mydata.xy -C5/3 -A70 -Fpq > mydata.pq

To take data in the file pacific.lonlat and transform it into oblique
coordinates using a pole from the hotspot reference frame and placing
the oblique zero meridian (*p* = 0 line) through Tahiti, run:

project pacific.lonlat -T-75/68 -C-149:26/-17:37 -Fpq > pacific.pq

Suppose that pacific\_topo.nc is a grid file of bathymetry, and you want
to make a file of flowlines in the hotspot reference frame. If you run:

grd2xyz pacific\_topo.nc \| project -T-75/68 -C0/-90 -Fxyq \| xyz2grd
-Retc -Ietc -Cflow.nc

then flow.nc is a file in the same area as pacific\_topo.nc, but flow
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

`See Also <#toc8>`_
-------------------

`*fitcircle*\ (1) <fitcircle.html>`_ , `*gmt*\ (1) <gmt.html>`_ ,
`*mapproject*\ (1) <mapproject.html>`_ ,
`*grdproject*\ (1) <grdproject.html>`_
