.. index:: ! surface

*******
surface
*******

.. only:: not man

    surface - Grid table data using adjustable tension continuous curvature splines

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**surface** [ *table* ] **-G**\ *outputfile.nc*
|SYN_OPT-I|
|SYN_OPT-R|
[ **-A**\ *aspect_ratio* ] [ **-C**\ *convergence_limit* ]
[ **-Ll**\ *lower* ] [ **-Lu**\ *upper* ] [ **-N**\ *max_iterations* ]
[ **-Q** ] [ **-S**\ *search_radius*\ [**m**\ \|\ **s**] ]
[ **-T**\ [**i**\ \|\ **b**] ]\ *tension_factor* [ **-V**\ [*level*] ]
[ **-Z**\ *over-relaxation_factor* ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]

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
data with **blockmean**, **blockmedian**, or **blockmode** to avoid
spatial aliasing and eliminate redundant data. You may impose lower
and/or upper bounds on the solution. These may be entered in the form of
a fixed value, a grid with values, or simply be the minimum/maximum
input data values. Natural boundary conditions are applied at the edges,
except for geographic data with 360-degree range where we apply periodic
boundary conditions in the longitude direction.

`Required Arguments <#toc4>`_
-----------------------------

**-G**\ *outputfile.nc*
    Output file name. Output is a binary 2-D *.nc* file. Note that the
    smallest grid dimension must be at least 4. 

.. include:: explain_-I.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

**-A**\ *aspect\_ratio*
    Aspect ratio. If desired, grid anisotropy can be added to the
    equations. Enter *aspect\_ratio*, where dy = dx / *aspect\_ratio*
    relates the grid dimensions. [Default = 1 assumes isotropic grid.]
**-C**\ *convergence\_limit*
    Convergence limit. Iteration is assumed to have converged when the
    maximum absolute change in any grid value is less than
    *convergence\_limit*. (Units same as data z units). [Default is
    scaled to 0.1 percent of typical gradient in input data.]
**-Ll**\ *lower* and **-Lu**\ *upper*
    Impose limits on the output solution. **l**\ *lower* sets the lower
    bound. *lower* can be the name of a grid file with lower bound
    values, a fixed value, **d** to set to minimum input value, or **u**
    for unconstrained [Default]. **u**\ *upper* sets the upper bound and
    can be the name of a grid file with upper bound values, a fixed
    value, **d** to set to maximum input value, or **u** for
    unconstrained [Default].
**-N**\ *max\_iterations*
    Number of iterations. Iteration will cease when *convergence\_limit*
    is reached or when number of iterations reaches *max\_iterations*.
    [Default is 250.]
**-Q**
    Suggest grid dimensions which have a highly composite greatest
    common factor. This allows surface to use several intermediate steps
    in the solution, yielding faster run times and better results. The
    sizes suggested by **-Q** can be achieved by altering **-R** and/or
    **-I**. You can recover the **-R** and **-I** you want later by
    using **grdsample** or **grdcut** on the output of **surface**.
**-S**\ *search\_radius*\ [**m**\ \|\ **s**]
    Search radius. Enter *search\_radius* in same units as x,y data;
    append **m** to indicate arc minutes or **s** for arc seconds. This
    is used to initialize the grid before the first iteration; it is not
    worth the time unless the grid lattice is prime and cannot have
    regional stages. [Default = 0.0 and no search is made.]
**-T**\ [**i**\ \|\ **b**] ]\ *tension\_factor*
    Tension factor[s]. These must be between 0 and 1. Tension may be
    used in the interior solution (above equation, where it suppresses
    spurious oscillations) and in the boundary conditions (where it
    tends to flatten the solution approaching the edges). Using zero for
    both values results in a minimum curvature surface with free edges,
    i.e., a natural bicubic spline. Use **-Ti**\ *tension\_factor*
    to set interior tension, and **-Tb**\ *tension\_factor* to set
    boundary tension. If you do not prepend **i** or **b**, both will be
    set to the same value. [Default = 0 for both gives minimum curvature
    solution.] 

.. |Add_-V| replace:: 
    **-V3** will report the convergence after each iteration; 
    **-V** will report only after each regional grid is converged.
.. include:: explain_-V.rst_

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

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: explain_-h.rst_
    
.. include:: explain_-icols.rst_

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

Bugs
----

**surface** will complain when more than one data point is found for any
node and suggest that you run **blockmean**, **blockmedian**, or
:doc:`blockmode` first. If you did run **blockm\*** and still get this
message it usually means that your grid spacing is so small that you
need more decimals in the output format used by **blockm\***. You may
specify more decimal places by editing the parameter
**FORMAT\_FLOAT\_OUT** in your :doc:`gmt.conf` file prior to running
**blockm\***, or choose binary input and/or output using single or
double precision storage.

Note that only gridline registration is possible with **surface**. If
you need a pixel-registered grid you can resample a gridline registered
grid using :doc:`grdsample` **-T**.

`See Also <#toc9>`_
-------------------

:doc:`blockmean`,
:doc:`blockmedian`,
:doc:`blockmode`,
:doc:`gmt`,
:doc:`greenspline`,
:doc:`nearneighbor`,
:doc:`triangulate`,
:doc:`sphtriangulate`

References
----------

Smith, W. H. F, and P. Wessel, 1990, Gridding with continuous curvature
splines in tension, *Geophysics*, 55, 293-305.
