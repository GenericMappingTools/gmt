.. index:: ! grdvector
.. include:: module_core_purpose.rst_

*********
grdvector
*********

|grdvector_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdvector** *grid1* *grid2* |-J|\ *parameters*
[ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-I|\ [**x**]\ *dx*\ [/*dy*] ]
[ |-N| ] [ |-Q|\ *parameters* ]
[ |SYN_OPT-R| ]
[ |-S|\ [**i**\|\ **l**]\ *scale*\ [**+c**\ [[*slon*/]\ *slat*]][**+s**\ *refsize*] ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen*\ [**+c**] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-l| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

**grdvector** reads two 2-D grid files which represents the *x*\ - and
*y*\ -components, :math:`(x,y)`, of a vector field and produces a vector field plot by
drawing vectors with orientation and length according to the information
in the files. Alternatively, polar coordinate grids, :math:`(r,\theta)`, may be given
instead (see |-A| and |-Z|).

Required Arguments
------------------

*grid1*
    Contains the *x*\ -components of the vector field. (See :ref:`Grid File Formats
    <grd_inout_full>`).

*grid2*
    Contains the *y*\ -components of the vector field. (See :ref:`Grid File Formats
    <grd_inout_full>`).

Order is important.
For :math:`(x,y)`, *grid1* is expected to be the *x*\ -component, and *grid2* to be the *y*\ -component.
For :math:`(r,\theta)`, *grid1* is expected to be the magnitude (:math:`r`),
and *grid2* (:math:`\theta`), to be the azimuth (|-Z|) or direction (|-A|).

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Optional Arguments
------------------

.. _-A:

**-A**
    The grid files contain polar :math:`(r,\theta)` components (magnitude and direction)
    instead of Cartesian :math:`(x,y)` [Default is Cartesian components].
    If :math:`\theta` contains azimuth, see |-Z|.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

.. include:: use_cpt_grd.rst_

.. _-G:

**-G**\ *fill*
    Sets color or shade for vector interiors [Default is no fill].
    Alternatively, the fill may be set via |-Q|.

.. _-I:

**-I**\ [**x**]\ *dx*\ [/*dy*]
    Only plot vectors at nodes every *x\_inc*, *y\_inc* apart (must be
    multiples of original grid spacing). Append **m** for arc minutes or
    **s** for arc seconds.  Alternatively, use **-Ix** to specify the
    multiples *multx*\ [/*multy*] directly [Default plots every node].

.. _-N:

**-N**
    Do **not** clip vectors at map boundaries [Default will clip].

.. _-Q:

**-Q**\ *parameters*
    Modify vector parameters. For vector heads, append vector head
    *size* [Default is 0, i.e., stick-plot]. See `Vector Attributes`_ for
    specifying additional attributes.

.. |Add_-R| replace:: Specify a subset of the grid. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-S:

**-S**\ [**i**\|\ **l**]\ *scale*\ [**+c**\ [[*slon*/]\ *slat*]][**+s**\ *refsize*]
    Sets scale for vector plot lengths in data units per plot distance measurement unit.
    Append **c**, **i**, or **p** to indicate the desired plot distance measurement
    unit (cm, inch, or point); if no unit is given we use the default value that
    is controlled by :term:`PROJ_LENGTH_UNIT`.  Vector lengths converted via plot unit
    scaling will plot as straight Cartesian vectors and their lengths are not
    affected by map projections and coordinate locations.
    For geographic data you may alternatively give *scale* in data units per map distance
    unit (see `Units`_). Then, your vector magnitudes (in data units) are scaled to map
    *distances* in the given distance unit, and finally projected onto the Earth to give
    *plot* dimensions.  These are geo-vectors that follow great circle paths and their
    lengths may be affected by the map projection and their coordinates.  Finally, use
    **-Si** if it is simpler to give the reciprocal scale in plot length or distance units
    per data unit.  Alternatively, use **-Sl**\ *length* to set a fixed plot length for all
    vectors. To report the minimum, maximum, and mean data and plot vector lengths
    of all vectors plotted, use |-V|. If an automatic legend entry is desired via **-l**,
    one or two modifiers will be required:

    - **+c**\ [[*slon*/]\ *slat*] controls where on a geographic map a geovector's *refsize*
      length applies. The modifier is neither needed nor available when plotting Cartesian vectors.
      The length is calculated for latitude *slat* (optionally supply longitude *slon* for
      oblique projections [default is central meridian]). If **+c** is given with no arguments
      then we select the reference length origin to be the middle of the map.
    - **+s**\ *refsize* sets the desired reference vector magnitude in data units. E.g., for a
      reference length of 25 mm/yr for plate motions, use modifier **+s**\ 25 with a corresponding
      option **-l**\ "Velocity (25 mm/yr)".  If *refsize* is not specified we default to the *scale*
      given above.

.. _-T:

**-T**
    Means the azimuths of Cartesian data sets should be adjusted according to the
    signs of the scales in the x- and y-directions [Leave alone].  This option can
    be used to convert vector azimuths in cases when a negative scale is used in
    one of both directions (e.g., positive down).

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *pen*\ [**+c**\]
    Change the pen attributes used for vector outlines [Default: width =
    default, color = black, style = solid].
    If the modifier **+c** is appended then the color of the vector head
    and stem are taken from the CPT (see |-C|).

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**
    The :math:`\theta` grid provided contains azimuth (in degrees east of north)
    rather than direction (in degrees counter-clockwise from horizontal).
    Implies |-A|.

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-l| unicode:: 0x20 .. just an invisible code
.. include:: explain_-l.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_vectors.rst_

.. module_common_ends

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To draw the vector field given by the files r.nc and theta.nc on a
linear plot with scale 5 cm per data unit, using vector rather than
stick plot, scale vector magnitudes so that 10 units equal 1 inch, and
center vectors on the node locations, run

::

  gmt grdvector r.nc theta.nc -Jx5c -A -Q0.1i+e+jc -S10i -pdf gradient

To plot a geographic data sets given the files comp_x.nc and comp_y.nc,
using a length scale of 200 km per data unit and only plot every 3rd node in either direction, try

::

  gmt grdvector comp_x.nc comp_y.nc -Ix3 -JH0/20c -Q0.1i+e+jc -S200k -pdf globe

.. module_note_begins

Vector scaling and unit effects
-------------------------------

The scale given via |-S| may require some consideration. As explained in |-S|,
it is specified in data-units per plot or distance unit. The plot or distance unit
chosen will affect the type of vector you select. In all cases, we first compute
the magnitude *r* of the user's data vectors at each selected node from the *x* and *y*
components (unless you are passing :math:`(r,\theta)` grids directly with |-A| or |-Z|).  These
magnitudes are given in whatever data units they come with.  Let us pretend our data
grids record secular changes in the Earth's magnetic horizontal vector field in units
of nTesla/year, and that at a particular node the magnitude is 28 nTesla/year (in some
direction). If you specify the scale using plot distance units (**c**\|\ **i**\|\ **p**)
then you are selecting *Cartesian* vectors. Let us further pretend that you selected
**-S**\ 10c as your scale option.  That means you want 10 nTesla/year to equate to a
1 cm plot length. Internally, we convert this scale to a plot scale of 1/10  = 0.1 cm
per nTesla/year. Given our vector magnitude of 28 nTesla/year, we multiply it by our
plot scale and finally obtain a vector length of 2.8 cm, which is then plotted.
The user's data units do not enter of course, i.e., they always cancel [Likewise, if
we had used **-S**\ 25i (25 nTesla/year per inch) the plot scale would be (1/25) = 0.04
inch per nTesla/year and the vector plot lengths would be 28 * 0.04 inch = 1.12 inch].
If we now wished to plot a 10 nTesla/year reference vector in the map legend we would
plot one that is 10 times 0.1 cm = 1 cm long since the scale length is *constant*
regardless of map projection and location. A 10 nTesla/year vector will be 1 cm anywhere.

Let us contrast this behavior with what happens if we use a geographic distance unit
instead, say **-S**\ 0.5k (0.5 nTesla/year per km). Internally, this becomes a map scale of
2 km per nTesta/year. Given our node magnitude of 28 nTesla/year, the vector length will
be 28 x 2 km = 56 km. Again, the user's data unit do not enter. Now, that vector length
of 56 km must be projected onto the Earth, and because of map distortions, a 56 km vector
will be mapped to a length on the plot that is a function of the user's map projection,
the map scale, and possibly the location on the map. E.g., a 56 km vector due east at
Equator on a Mercator map would seem to equal ~0.5 degree longitude but at 60 north it
would be more like ~1 degree longitude. A consequence of this effect is that a user
who wants to add a 10 nTesla/year reference vector to a legend faces the same problem we do
when we wish to draw a 100 km map scale on a map: the plotted length usually will depend
on latitude and hence that reference scale is only useful around that latitude.

This brings us to the inverse scale option, **-Si**\ *length*.  This variant is useful
when providing the inverse of the scale is simpler. In the Cartesian case above, we
could instead give **-Si**\ 0.1c which would directly imply a plot scale of 0.1 cm per
nTesla/year. Likewise, for geographic distances we could give **-Si**\ 2k for 2 km per
nTesla/year scale as well. As the **-Si** argument increases, the plotted vector length
increases as well, while for plain |-S| the plot length decreases with increasing scale.

Notes
-----

Be aware that using |-I| may lead to aliasing unless
your grid is smoothly varying over the new length increments.
It is generally better to filter your grids and resample at a
larger grid increment and use these grids instead of the originals.

.. module_note_ends

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`grdcontour`, :doc:`plot`
