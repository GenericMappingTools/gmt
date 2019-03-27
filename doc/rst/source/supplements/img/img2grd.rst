.. index:: ! img2grd

*******
img2grd
*******

.. only:: not man

    Extract subset of img file in Mercator or Geographic format

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt img2grd** *imgfile* |-G|\ *grdfile*
|SYN_OPT-R|
[ |-C| ]
[ |-D|\ [*minlat/maxlat*] ] [ |-E| ] [ |-I|\ *inc* ]
[ |-M| ] [ |-N|\ *navg* ] [ |-S|\ [*scale*] ]
[ |-T|\ *type* ]
[ |SYN_OPT-V| ]
[ |-W|\ *maxlon* ]
[ |SYN_OPT-n| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**img2grd** reads an img format file, extracts a subset, and writes it
to a grid file. The **-M** option dictates whether or not the Spherical
Mercator projection of the img file is preserved or if a Geographic grid
should be written by undoing the Mercator projection. If geographic grid
is selected you can also request a resampling onto the exact **-R** given.

Required Arguments
------------------

*imgfile*
    A Mercator img format file such as the marine gravity or seafloor
    topography fields estimated from satellite altimeter data by
    Sandwell and Smith. If the user has set an environment variable
    **$GMT_DATADIR**, then **img2grd** will try to find *imgfile* in
    **$GMT_DATADIR**; else it will try to open *imgfile* directly.

.. _-G:

**-G**\ *grdfile*
    *grdfile* is the name of the output grid file.

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

Optional Arguments
------------------

.. _-C:

**-C**
    Set the x and y Mercator coordinates relative to projection center
    [Default is relative to lower left corner of grid]. Requires **-M**.

.. _-D:

**-D**\ [*minlat/maxlat*\ ]
    Use the extended latitude range -80.738/+80.738. Alternatively,
    append *minlat/maxlat* as the latitude extent of the input img file.
    [Default is -72.006/72.006]. Not usually required since we can
    determine the extent from inspection of the file size.

.. _-E:

**-E**
    Can be used when **-M** is not set to force the final grid to have
    the exact same region as requested with **-R**. By default, the
    final region is a direct projection of the original Mercator region
    and will typically extend slightly beyond the requested latitude
    range, and furthermore the grid increment in latitude does not match
    the longitude increment. However, the extra resampling introduces
    small interpolation errors and should only be used if the output
    grid must match the requested region and have x_inc = y_inc. In
    this case the region set by **-R** must be given in multiples of the
    increment (.e.g, **-R**\ 0/45/45/72).

.. _-I:

**-I**
    Indicate *inc* as the width of an input img pixel in minutes of
    longitude [Default is 2]. Append **m** [Default] or **s** to
    indicate unit.  Not usually required since we can
    determine the pixel size from inspection of the size.

.. _-M:

**-M**
    Output a Spherical Mercator grid [Default is a geographic lon/lat
    grid]. The Spherical Mercator projection of the img file is
    preserved, so that the region **-R** set by the user is modified
    slightly; the modified region corresponds to the edges of pixels [or
    groups of *navg* pixels]. The grid file header is set so that the x
    and y axis lengths represent distance from the west and south edges
    of the image, measured in user default units, with **-Jm**\ 1 and
    the adjusted **-R**. By setting the default **PROJ_ ELLIPSOID** =
    Sphere, the user can make overlays with the adjusted **-R** so that
    they match. See **EXAMPLES** below. The adjusted **-R** is also
    written in the grid header remark, so it can be found later. See
    **-C** to set coordinates relative to projection center.

.. _-N:

**-N**\ *navg*
    Average the values in the input img pixels into *navg* by *navg*
    squares, and create one output pixel for each such square. If used
    with **-T**\ *3* it will report an average constraint between 0 and
    1. If used with **-T**\ *2* the output will be average data value or
    NaN according to whether average constraint is > 0.5. *navg* must
    evenly divide into the dimensions of the imgfile in pixels. [Default
    *1* does no averaging].

.. _-S:

**-S**\ [*scale*\ ]
    Multiply the img file values by *scale* before storing in grid file.
    [Default is 1.0]. For recent img files: img topo files are stored in
    (corrected) meters [**-S**\ 1]; free-air gravity files in mGal\*10
    [**-S**\ 0.1 to get mGal]; vertical deflection files in
    micro-radians\*10 [**-S**\ 0.1 to get micro-radians], vertical gravity
    gradient files in Eotvos\*10 [**-S**\ 0.1 to get Eotvos, or
    **-S**\ 0.01 to get mGal/km]). If no *scale* is given we try to
    determine the scale by examining the file name for clues.

.. _-T:

**-T**\ *type*
    *type* handles the encoding of constraint information. *type* = 0
    indicates that no such information is encoded in the img file (used
    for pre-1995 versions of the gravity data) and gets all data. *type*
    > 0 indicates that constraint information is encoded (1995 and later
    (current) versions of the img files) so that one may produce a grid
    file as follows: **-T**\ *1* gets data values at all points,
    **-T**\ *2* gets data values at constrained points and NaN at
    interpolated points; **-T**\ *3* gets 1 at constrained points and 0
    at interpolated points [Default is 1].

.. _-V:

.. |Add_-V| replace:: Particularly recommended here, as it is
    helpful to see how the coordinates are adjusted.
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**\ *maxlon*
    Indicate *maxlon* as the maximum longitude extent of the input img
    file. Versions since 1995 have had *maxlon* = 360.0, while some
    earlier files had *maxlon* = 390.0. [Default is 360.0].

.. include:: ../../explain_-n.rst_

.. include:: ../../explain_help.rst_

Geographic Examples
-------------------

The **-M** option should be excluded if you need the output grid to be
in geographic coordinates. To extract data in the region
**-R**-40/40/-70/-30 from *world_grav.img.7.2* and reproject to yield
geographic coordinates, you can try

   ::

    img2grd world_grav.img.16.1 -Gmerc_grav.nc -R-40/40/-70/-30 -V

Because the latitude spacing in the img file is equidistant in Mercator
units, the resulting grid will not match the specified **-R** exactly,
and the latitude spacing will not equal the longitude spacing. If you
need an exact match with your **-R** and the same spacing in longitude
and latitude, use the **-E** option:

   ::

    img2grd world_grav.img.16.1 -Gmerc_grav.nc -R-40/40/-70/-30 -E -V

Mercator Examples
-----------------

Since the img files are in a Mercator projection, you should NOT extract
a geographic grid if your plan is to make a Mercator map. If you did
that you end of projecting and reprojection the grid, losing
short-wavelength detail. Better to use **-M** and plot the grid using a
linear projection with the same scale as the desired Mercator projection
(see GMT Example 29).
To extract data in the region **-R**-40/40/-70/-30 from
*world_grav.img.7.2*, run

   ::

    gmt img2grd -M world_grav.img.7.2 -Gmerc_grav.nc -R-40/40/-70/-30 -V

Note that the **-V** option tells us that the range was adjusted to
**-R**-40/40/-70.0004681551/-29.9945810754. For scripting purposes we
can extract this original region string using :doc:`grdinfo </grdinfo>` **-Ii**.
Furthermore, we can also use :doc:`grdinfo </grdinfo>`
to find that the grid file header shows its region to be
**-R**\ 0/80/0/67.9666667. This is the range of x,y we will get from a
Spherical Mercator projection using
**-R**-40/40/-70.0004681551/-29.9945810754 and **-Jm**\ 1. Thus, to take
ship.lonlatgrav and use it to sample the merc_grav.nc, we can do this:

   ::

    gmt set PROJ_ELLIPSOID Sphere

    gmt mapproject -R-40/40/-70.0004681551/-29.9945810754 -Jm1i ship.lonlatgrav |
              gmt grdtrack -Gmerc_grav.nc | gmt mapproject
              -R-40/40/-70.0004681551/-29.9945810754 -Jm1i -I > ship.lonlatgravsat

It is recommended to use the above method of projecting and unprojecting
the data in such an application, because then there is only one
interpolation step (in :doc:`grdtrack </grdtrack>`). If one first tries to convert the
grid file to lon,lat and then sample it, there are two interpolation
steps (in conversion and in sampling).

To make a lon,lat grid from the above grid we can use

   ::

    gmt grdproject merc_grav.nc -R-40/40/-70.0004681551/-29.9945810754 -Jm1i -I -D2m -Ggrav.nc

In some cases this will not be easy as the **-R** in the two coordinate
systems may not align well. When this happens, we can also use (in fact,
it may be always better to use)

   ::

    gmt grd2xyz merc_grav.nc | gmt mapproject
        -R-40/40/-70.0004681551/-29.994581075 -Jm1i -I |
        gmt surface -R-40/40/-70/70 -I2m -Ggrav.nc

To make a Mercator map of the above region, suppose our gmt.conf value
for :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>` is inch. Then since the above merc_grav.nc
file is projected with **-Jm**\ 1i it is 80 inches wide. We can make a
map 8 inches wide by using **-Jx**\ 0.1i on any map programs applied to
this grid (e.g., :doc:`grdcontour </grdcontour>`,
:doc:`grdimage </grdimage>`, :doc:`grdview </grdview>`), and then
for overlays which work in lon,lat (e.g., :doc:`plot </plot>`, :doc:`coast </coast>`) we can
use the above adjusted **-R** and **-Jm**\ 0.1 to get the two systems to
match up.

However, we can be smarter than this. Realizing that the input img file
had pixels 2.0 minutes wide (or checking the nx and ny with :doc:`grdinfo </grdinfo>`
merc_grav.nc) we realize that merc_grav.nc used the full resolution of
the img file and it has 2400 by 2039 pixels, and at 8 inches wide this
is 300 pixels per inch. We decide we do not need that many and we will be
satisfied with 100 pixels per inch, so we want to average the data into
3 by 3 squares. (If we want a contour plot we will probably choose to
average the data much more (e.g., 6 by 6) to get smooth contours.) Since
2039 isn't divisible by 3 we will get a different adjusted **-R** this time:

   ::

    gmt img2grd -M world_grav.img.7.2 -Gmerc_grav_2.nc -R-40/40/-70/-30 -N3 -V

This time we find the adjusted region is
**-R**-40/40/-70.023256525/-29.9368261101 and the output is 800 by 601
pixels, a better size for us. Now we can create an artificial
illumination file for this using :doc:`grdgradient </grdgradient>`:

   ::

    gmt grdgradient merc_grav_2.nc -Gillum.nc -A0/270 -Ne0.6

and if we also have a CPT called "grav.cpt" we can create a color
shaded relief map like this:

   ::

    gmt begin
    gmt grdimage merc_grav_2.nc -Iillum.nc -Cgrav.cpt -Jx0.1i
    gmt basemap -R-40/40/-70.023256525/-29.9368261101 -Jm0.1i -Ba10
    gmtend

Suppose you want to obtain only the constrained data values from an img
file, in lat/lon coordinates. Then run **img2grd** with the **-T**\ 2
option, use :doc:`grd2xyz </grd2xyz>` to dump the values, pipe through grep -v NaN to
eliminate NaNs, and pipe through :doc:`mapproject </mapproject>` with the inverse projection as above.

See Also
--------

:doc:`gmt </gmt>`
