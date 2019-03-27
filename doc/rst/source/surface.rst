.. index:: ! surface

*******
surface
*******

.. only:: not man

    Grid table data using adjustable tension continuous curvature splines

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt surface** [ *table* ] |-G|\ *outputfile.nc*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ *aspect_ratio*\ \|\ **m** ]
[ |-C|\ *convergence_limit*\ [%] ]
[ |-L|\ **l**\ *lower* ] [ **-Lu**\ *upper* ]
[ |-M|\ *max_radius*\ [**u**] ]
[ |-N|\ *max_iterations* ]
[ |-Q| ]
[ |-S|\ *search_radius*\ [**m**\ \|\ **s**] ]
[ |-T|\ [**i**\ \|\ **b**]\ *tension_factor* ]
[ |SYN_OPT-V| ]
[ |-Z|\ *over-relaxation_factor* ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**surface** reads randomly-spaced (x,y,z) triples from standard input
[or *table*] and produces a binary grid file of gridded values z(x,y) by
solving:

   (1 - T) \* L (L (z)) + T \* L (z) = 0

where T is a tension factor between 0 and 1, and L indicates the
Laplacian operator. T = 0 gives the "minimum curvature" solution which
is equivalent to SuperMISP and the ISM packages. Minimum curvature can
cause undesired oscillations and false local maxima or minima (See Smith
and Wessel, 1990), and you may wish to use T > 0 to suppress these
effects. Experience suggests T ~ 0.25 usually looks good for potential
field data and T should be larger (T ~ 0.35) for steep topography data.
T = 1 gives a harmonic surface (no maxima or minima are possible except
at control data points). It is recommended that the user pre-process the
data with :doc:`blockmean`, :doc:`blockmedian`, or :doc:`blockmode` to avoid
spatial aliasing and eliminate redundant data. You may impose lower
and/or upper bounds on the solution. These may be entered in the form of
a fixed value, a grid with values, or simply be the minimum/maximum
input data values. Natural boundary conditions are applied at the edges,
except for geographic data with 360-degree range where we apply periodic
boundary conditions in the longitude direction.

Required Arguments
------------------

.. _-G:

**-G**\ *outputfile.nc*
    Output file name. Output is a binary 2-D *.nc* file. Note that the
    smallest grid dimension must be at least 4. 

.. _-I:

.. include:: explain_-I.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

**-A**\ *aspect_ratio*\ \|\ **m**
    Aspect ratio. If desired, grid anisotropy can be added to the
    equations. Enter *aspect_ratio*, where dy = dx / *aspect_ratio*
    relates the grid dimensions. For geographic data, you may use
    **-Am** to set the aspect ratio to the cosine of the mean latitude
    [Default = 1 assumes isotropic grid.]

.. _-C:

**-C**\ *convergence_limit*\ [%]
    Convergence limit. Iteration is assumed to have converged when the
    maximum absolute change in any grid value is less than
    *convergence_limit*. (Units same as data z units). Alternatively,
    give limit in percentage of rms deviation by appending %.  [Default is
    scaled to 1e-4 of the root-mean-square deviation of the data
    from a best-fit (least-squares) plane.].
    This is the final convergence limit at the desired grid spacing; for
    intermediate (coarser) grids the effective convergence limit is divided
    by the grid spacing multiplier.

.. _-L:

**-Ll**\ *lower* and **-Lu**\ *upper*
    Impose limits on the output solution. **l**\ *lower* sets the lower
    bound. *lower* can be the name of a grid file with lower bound
    values, a fixed value, **d** to set to minimum input value, or **u**
    for unconstrained [Default]. **u**\ *upper* sets the upper bound and
    can be the name of a grid file with upper bound values, a fixed
    value, **d** to set to maximum input value, or **u** for
    unconstrained [Default]. Grid files used to set the limits may
    contain NaNs. In the presence of NaNs, the limit of a node masked
    with NaN is unconstrained.

.. _-M:

**-M**\ *max_radius*\ [**u**]
    After solving for the surface, apply a mask so that nodes farther
    than *max_radius* away from a data constraint is set to NaN [no masking].
    Append a distance unit (see UNITS) if needed.
    One can also select the nodes to mask by using the **-M**\ *n_cells*\ **c** form.
    Here *n_cells* means the number of cells around the node controlled by a data point. As an example
    **-M0c** means that only the cell where point lies is filled, **-M1c** keeps one cell
    beyond that (i.e. makes a 3x3 neighborhood), and so on.

.. _-N:

**-N**\ *max_iterations*
    Number of iterations. Iteration will cease when *convergence_limit*
    is reached or when number of iterations reaches *max_iterations*.
    This is the final iteration limit at the desired grid spacing; for
    intermediate (coarser) grids the effective iteration limit is scaled
    by the grid spacing multiplier [Default is 500].

.. _-Q:

**-Q**
    Suggest grid dimensions which have a highly composite greatest
    common factor. This allows surface to use several intermediate steps
    in the solution, yielding faster run times and better results. The
    sizes suggested by **-Q** can be achieved by altering **-R** and/or
    **-I**. You can recover the **-R** and **-I** you want later by
    using :doc:`grdsample` or :doc:`grdcut` on the output of **surface**.

.. _-S:

**-S**\ *search_radius*\ [**m**\ \|\ **s**]
    Search radius. Enter *search\_radius* in same units as x,y data;
    append **m** to indicate arc minutes or **s** for arc seconds. This
    is used to initialize the grid before the first iteration; it is not
    worth the time unless the grid lattice is prime and cannot have
    regional stages. [Default = 0.0 and no search is made.]

.. _-T:

**-T**\ [**i**\ \|\ **b**]\ *tension_factor*
    Tension factor[s]. These must be between 0 and 1. Tension may be
    used in the interior solution (above equation, where it suppresses
    spurious oscillations) and in the boundary conditions (where it
    tends to flatten the solution approaching the edges). Using zero for
    both values results in a minimum curvature surface with free edges,
    i.e., a natural bicubic spline. Use **-Ti**\ *tension_factor*
    to set interior tension, and **-Tb**\ *tension_factor* to set
    boundary tension. If you do not prepend **i** or **b**, both will be
    set to the same value. [Default = 0 for both gives minimum curvature
    solution.] 

.. _-V:

.. |Add_-V| replace:: 
    **-V3** will report the convergence after each iteration; 
    **-V** will report only after each regional grid is converged.
.. include:: explain_-V.rst_

.. _-Z:

**-Z**\ *over-relaxation_factor*
    Over-relaxation factor. This parameter is used to accelerate the
    convergence; it is a number between 1 and 2. A value of 1 iterates
    the equations exactly, and will always assure stable convergence.
    Larger values overestimate the incremental changes during
    convergence, and will reach a solution more rapidly but may become
    unstable. If you use a large value for this factor, it is a good
    idea to monitor each iteration with the **-Vl** option. [Default =
    1.4 converges quickly and is almost always stable.] 

.. include:: explain_-aspatial.rst_

.. |Add_-bi| replace:: [Default is 3 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: explain_-h.rst_
    
.. include:: explain_-icols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_float.rst_


Examples
--------

To grid 5 by 5 minute gravity block means from the ASCII data in
hawaii_5x5.xyg, using a *tension_factor* = 0.25, a
*convergence_limit* = 0.1 milligal, writing the result to a file called
hawaii_grd.nc, and monitoring each iteration, try:

   ::

    gmt surface hawaii_5x5.xyg -R198/208/18/25 -I5m -Ghawaii_grd.nc -T0.25 -C0.1 -Vl

Gridding Geographic Data: Boundary Conditions
---------------------------------------------

The surface finite difference algorithm is Cartesian at heart, hence the *ad hoc*
option to change the aspect ratio for a suitable mean latitude (**-A**). When
geographic data are supplied and the output grid has a 360 degree longitude range we will
impose periodic boundary conditions in longitude.  However, no equivalent geographic
boundary condition can be applied at the poles since the finite difference solution
will not be valid there (actual spacing between the nodes at the poles is zero).
If you attempt this type of gridding you will be severely warned but the calculations
will continue.  Because the result is a geographic grid, the GMT i/o machinery will
interfere and detect inconsistencies at the pole points and replace all values along
a pole with their mean value.  This will introduce further distortion into the
grid near the poles.  We recommend you instead consider spherical gridding for global
data sets; see :doc:`greenspline` (for modest data sets) or :doc:`sphinterpolate`.

Gridding Geographic Data: Setting Increments
--------------------------------------------

Specifying grid increments in distance units (meters, km, etc.) for geographic (lon, lat)
grids triggers a conversion from the given increment to the equivalent increment in degrees.
This is done differently for longitude and latitude and also depends on chosen ellipsoid,
but ultimately is a great-circle approximation. For latitude we divide your *y*-increment
with the number of you chosen unit per degree latitude, while for longitude we divide your
*x*-increment by the number of such units per degree along the mid-parallel in your region. The
resulting degree increments may therefore not exactly match the increments you entered explicitly.
Hence, there may be rounding off in ways you don't want and cannot easily control, resulting in prime grid
dimensions. You can handle the situation via **-Q** but with the never-ending decimals in some
increments that is still a challenge.  Another approach is to *not* grid geographic data
using length units as increments, due to the above conversion. It may be cleaner to specify
grid intervals in spherical degrees, minutes or seconds. That way you can control the grid
dimensions directly and avoid the round-off. Alternatively, if your region is far from Equator
and your are concerned about the difference in longitude and latitude increments in degrees
you could project all data to a local projection (e.g., UTM) to yield units of meters, and then
grid the projected data using meters as the final grid increment. Either approach avoids
"ugly" increments like 0.161697s and will let you specify intervals that are easily divisible
into the range. If increment choice is dictated by a need for a desired increment in meters
then the projection route will yield better results.

Bugs
----

**surface** will complain when more than one data point is found for any
node and suggest that you run :doc:`blockmean`, :doc:`blockmedian`, or
:doc:`blockmode` first. If you did run these decimators and still get this
message it usually means that your grid spacing is so small that you
need more decimals in the output format used. You may
specify more decimal places by editing the parameter
**FORMAT_FLOAT_OUT** in your :doc:`gmt.conf` file prior to running
the decimators or choose binary input and/or output using single or
double precision storage.

Note that only gridline registration is possible with **surface**. If
you need a pixel-registered grid you can resample a gridline registered
grid using :doc:`grdsample` **-T**.

See Also
--------

:doc:`blockmean`,
:doc:`blockmedian`,
:doc:`blockmode`,
:doc:`gmt`,
:doc:`grdcut`,
:doc:`grdsample`,
:doc:`greenspline`,
:doc:`nearneighbor`,
:doc:`triangulate`,
:doc:`sphinterpolate`

References
----------

Smith, W. H. F, and P. Wessel, 1990, Gridding with continuous curvature
splines in tension, *Geophysics*, 55, 293-305.
