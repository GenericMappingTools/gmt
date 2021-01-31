.. index:: ! greenspline
.. include:: module_core_purpose.rst_

***********
greenspline
***********

|greenspline_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt greenspline** [ *table* ]
[ |-A|\ *gradfile*\ **+f**\ **1**\|\ **2**\|\ **3**\|\ **4**\|\ **5** ]
[ |-C|\ [**n**]\ *value*\ [%][**+f**\ *file*][**+m**\|\ **M**] ]
[ |-D|\ [**+x**\ *xname*][**+y**\ *yname*][**+z**\ *zname*][**+v**\ *vname*][**+s**\ *scale*][**+o**\ *offset*][**+n**\ *invalid*][**+t**\ *title*][**+r**\ *remark*] ]
[ |-E|\ [*misfitfile*] ]
[ |-G|\ *grdfile* ]
[ |-I|\ *xinc*\ [/*yinc*\ [/*zinc*]] ]
[ |-L| ]
[ |-N|\ *nodefile* ]
[ |-Q|\ *az*\|\ *x/y/z* ]
[ **-R**\ *xmin*/*xmax*\ [/*ymin*/*ymax*\ [/*zmin*/*zmax*]] ]
[ |-S|\ **c\|t\|l\|r\|p\|q**\ [*pars*] ] [ |-T|\ *maskgrid* ]
[ |SYN_OPT-V| ]
[ |-W|\ [**w**]]
[ |-Z|\ *mode* ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**greenspline** uses the Green's function G(\ **x**; **x'**) for the
chosen spline and geometry to interpolate data at regular [or arbitrary]
output locations. Mathematically, the solution is composed as
*w*\ (**x**) = sum {*c*\ (*i*) G(\ **x'**; **x**\ (*i*))}, for *i* = 1,
*n*, the number of data points {**x**\ (*i*), *w*\ (*i*)}. Once the *n*
coefficients *c*\ (*i*) have been found the sum can be evaluated at any
output point **x**. Choose between minimum curvature, regularized, or
continuous curvature splines in tension for either 1-D, 2-D, or 3-D
Cartesian coordinates or spherical surface coordinates. After first
removing a linear or planar trend (Cartesian geometries) or mean value
(spherical surface) and normalizing these residuals, the least-squares
matrix solution for the spline coefficients *c*\ (*i*) is found by
solving the *n* by *n* linear system *w*\ (*j*) = sum-over-*i*
{*c*\ (*i*) G(\ **x**\ (*j*); **x**\ (*i*))}, for *j* = 1, *n*; this
solution yields an exact interpolation of the supplied data points.
Alternatively, you may choose to perform a singular value decomposition
(SVD) and eliminate the contribution from the smallest eigenvalues; this
approach yields an approximate solution. Trends and scales are restored
when evaluating the output.

Required Arguments
------------------

*table*
    The name of one or more ASCII [or binary, see
    **-bi**] files holding the **x**, *w* data
    points. If no file is given then we read standard input instead.

Optional Arguments
------------------

.. _-A:

**-A**\ *gradfile*\ **+f**\ **1**\|\ **2**\|\ **3**\|\ **4**\|\ **5**
    The solution will partly be constrained by surface gradients **v** =
    *v*\ \*\ **n**, where *v* is the gradient magnitude and **n** its
    unit vector direction. The gradient direction may be specified
    either by Cartesian components (either unit vector **n** and
    magnitude *v* separately or gradient components **v** directly) or
    angles w.r.t. the coordinate axes. Append name of ASCII file with
    the surface gradients.  Use **+f** to select one of five input
    formats: **0**: For 1-D data there is no direction, just gradient
    magnitude (slope) so the input format is *x*, *gradient*. Options
    1-2 are for 2-D data sets: **1**: records contain *x*, *y*,
    *azimuth*, *gradient* (*azimuth* in degrees is measured clockwise
    from the vertical (north) [Default]). **2**: records contain *x*,
    *y*, *gradient*, *azimuth* (*azimuth* in degrees is measured
    clockwise from the vertical (north)). Options 3-5 are for either 2-D
    or 3-D data: **3**: records contain **x**, *direction(s)*, *v*
    (*direction(s)* in degrees are measured counter-clockwise from the
    horizontal (and for 3-D the vertical axis). **4**: records contain
    **x**, **v**. **5**: records contain **x**, **n**, *v*.

.. _-C:

**-C**\ [**n**]\ *value*\ [%][**+f**\ *file*][**+m**\|\ **M**]
    Find an approximate surface fit: Solve the linear system for the
    spline coefficients by SVD and eliminate the contribution from all
    eigenvalues whose ratio to the largest eigenvalue is less than *value*
    [Default uses Gauss-Jordan elimination to solve the linear system
    and fit the data exactly]. Optionally, append **+f**\ *file* to save the
    eigenvalues to the specified file for further analysis.
    If a negative *value* is given then **+f**\ *file* is required and
    execution will stop after saving the eigenvalues, i.e., no surface
    output is produced.  Specify **-Cn** to retain only the *value* largest
    eigenvalues; append % if *value* is the percentage of eigenvalues
    to use instead.  The two last modifiers (**+m**\|\ **M**) are only
    available for 2-D gridding and can be used to write intermediate grids,
    one per eigenvalue, and thus require a file name template with a C-format
    integer specification to be given via **-G**.  The **+m** modifier will
    write the contributions to the grid for each eigenvalue, while **+M**
    will instead produce the cumulative sum of these contributions.

.. _-D:

.. include:: explain_-D_cap.rst_

.. _-E:

**-E**\ [*misfitfile*]
    Evaluate the spline exactly at the input data locations and report
    statistics of the misfit (mean, standard deviation, and rms).  Optionally,
    append a filename and we will write the data table, augmented by
    two extra columns holding the spline estimate and the misfit.

.. _-G:

**-G**\ *grdfile*
    Name of resulting output file. (1) If options **-R**, **-I**, and
    possibly **-r** are set we produce an equidistant output table. This
    will be written to stdout unless **-G** is specified. **Note**: For 2-D
    grids the **-G** option is required. (2) If option **-T** is
    selected then **-G** is required and the output file is a 2-D binary
    grid file. Applies to 2-D interpolation only. (3) For 3-D cubes
    the **-G** option is optional.  If set, it can be the name of a 3-D
    cube file or a filename template with a floating-point C-format identifier
    in it so that each layer is written to a 2-D grid file; otherwise
    we write (*x, y, z, w*) records to stdout. (4) If **-N** is
    selected then the output is an ASCII (or binary; see
    **-bo**) table; if **-G** is not given then
    this table is written to standard output. Ignored if **-C** or
    **-C**\ 0 is given.

.. _-I:

**-I**\ *xinc*\ [/*yinc*\ [/*zinc*]]
    Specify equidistant sampling intervals, on for each dimension, separated by slashes.

.. _-L:

**-L**
    Do *not* remove a linear (1-D) or planer (2-D) trend when **-Z**
    selects mode 0-3 [For those Cartesian cases a least-squares line or
    plane is modeled and removed, then restored after fitting a spline
    to the residuals]. However, in mixed cases with both data values and
    gradients, or for spherical surface data, only the mean data value
    is removed (and later and restored).

.. _-N:

**-N**\ *nodefile*
    ASCII file with coordinates of desired output locations **x** in the
    first column(s). The resulting *w* values are appended to each
    record and written to the file given in **-G** [or stdout if not
    specified]; see **-bo** for binary output
    instead. This option eliminates the need to specify options **-R**,
    **-I**, and **-r**.

.. _-Q:

**-Q**\ *az*\|\ *x/y/z*
    Rather than evaluate the surface, take the directional derivative in
    the *az* azimuth and return the magnitude of this derivative
    instead. For 3-D interpolation, specify the three components of the
    desired vector direction (the vector will be normalized before use).

.. _-R:

**-R**\ *xmin*/*xmax*\ [/*ymin*/*ymax*\ [/*zmin*/*zmax*]]
    Specify the domain for an equidistant lattice where output
    predictions are required. Requires **-I** and optionally **-r**.

    *1-D:* Give *xmin/xmax*, the minimum and maximum *x* coordinates.

    *2-D:* Give *xmin/xmax/ymin/ymax*, the minimum and maximum *x* and
    *y* coordinates. These may be Cartesian or geographical. If
    geographical, then *west*, *east*, *south*, and *north* specify the
    Region of interest, and you may specify them in decimal degrees or
    in [±]dd:mm[:ss.xxx][**W**\|\ **E**\|\ **S**\|\ **N**]
    format. The two shorthands **-Rg**
    and **-Rd** stand for global domain (0/360 and -180/+180 in
    longitude respectively, with -90/+90 in latitude).

    *3-D:* Give *xmin/xmax/ymin/ymax/zmin/zmax*, the minimum and maximum
    *x*, *y* and *z* coordinates. See the 2-D section if your horizontal
    coordinates are geographical; note the shorthands **-Rg** and
    **-Rd** cannot be used if a 3-D domain is specified.

.. _-S:

**-S**\ **c\|t\|l\|r\|p\|q**\ [*pars*]
    Select one of six different splines. The first two are used for
    1-D, 2-D, or 3-D Cartesian splines (see **-Z** for discussion). Note
    that all tension values are expected to be normalized tension in the
    range 0 < *t* < 1: (**c**) Minimum curvature spline [*Sandwell*,
    1987], (**t**) Continuous curvature spline in tension [*Wessel and
    Bercovici*, 1998]; append *tension*\ [/*scale*] with *tension* in
    the 0-1 range and optionally supply a length scale [Default is the
    average grid spacing]. The next is a 1-D or 2-D spline: (**l**)
    Linear (1-D) or Bilinear (2-D) spline; these produce output that do
    not exceed the range of the given data.  The next is a 2-D or 3-D spline: (**r**)
    Regularized spline in tension [*Mitasova and Mitas*, 1993]; again,
    append *tension* and optional *scale*. The last two are spherical
    surface splines and both imply **-Z**\ 4: (**p**) Minimum
    curvature spline [*Parker*, 1994], (**q**) Continuous curvature
    spline in tension [*Wessel and Becker*, 2008]; append *tension*. The
    G(\ **x'**; **x'**) for the last method is slower to compute (a series solution) so we
    pre-calculate values and use cubic spline interpolation lookup instead.
    Optionally append **+n**\ *N* (an odd integer) to change how many
    points to use in the spline setup [10001].  The finite Legendre sum has
    a truncation error [1e-6]; you can lower that by appending **+e**\ *limit*
    at the expense of longer run-time.

.. _-T:

**-T**\ *maskgrid*
    For 2-D interpolation only. Only evaluate the solution at the nodes
    in the *maskgrid* that are not equal to NaN. This option eliminates
    the need to specify options **-R**, **-I**, and **-r**.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**w**]
   Data one-sigma uncertainties are provided in the last column.
   We then compute weights that are inversely proportional to the uncertainties squared.
   Append **w** if weights are given instead of uncertainties and then they will be used
   as is (no squaring).  This results in a weighted least squares fit.  Note that this
   only has an effect if **-C** is used.  [Default uses no weights or uncertainties].

.. _-Z:

**-Z**\ *mode*
    Sets the distance flag that determines how we calculate distances
    between data points. Select *mode* 0 for Cartesian 1-D spline
    interpolation: **-Z**\ 0 means (*x*) in user units, Cartesian
    distances, Select *mode* 1-3 for Cartesian 2-D surface spline
    interpolation: **-Z**\ 1 means (*x*,\ *y*) in user units, Cartesian
    distances, **-Z**\ 2 for (*x*,\ *y*) in degrees, Flat Earth
    distances, and **-Z**\ 3 for (*x*,\ *y*) in degrees, Spherical
    distances in km. Then, if :term:`PROJ_ELLIPSOID` is spherical, we
    compute great circle arcs, otherwise geodesics. Option *mode* = 4
    applies to spherical surface spline interpolation only: **-Z**\ 4
    for (*x*,\ *y*) in degrees, use cosine of great circle (or geodesic)
    arcs. Select *mode* 5 for Cartesian 3-D surface spline
    interpolation: **-Z**\ 5 means (*x*,\ *y*,\ *z*) in user units,
    Cartesian distances.

.. |Add_-bi| replace:: [Default is 2-4 input
   columns (**x**,\ *w*); the number depends on the chosen dimension].
.. include:: explain_-bi.rst_

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-q.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_core.rst_

.. include:: explain_help.rst_

1-d Examples
------------

To resample the *x*,\ *y* Gaussian random data created by :doc:`gmtmath`
and stored in 1D.txt, requesting output every 0.1 step from 0 to 10, and
using a minimum cubic spline, try::

    gmt begin 1D
      gmt math -T0/10/1 0 1 NRAND = 1D.txt
      gmt plot -R0/10/-5/5 -JX6i/3i -B -Sc0.1 -Gblack 1D.txt
      gmt greenspline 1D.txt -R0/10 -I0.1 -Sc | gmt plot -Wthin
    gmt end show

To apply a spline in tension instead, using a tension of 0.7, try::

    gmt begin 1Dt
      gmt plot -R0/10/-5/5 -JX6i/3i -B -Sc0.1 -Gblack 1D.txt
      gmt greenspline 1D.txt -R0/10 -I0.1 -St0.7 | gmt plot -Wthin
    gmt end show

2-d Examples
------------

To make a uniform grid using the minimum curvature spline for the same
Cartesian data set from Table 5.11 in Davis (1986) that is used in the
GMT Technical Reference and Cookbook example 16, try::

    gmt begin 2D
      gmt greenspline @Table_5_11.txt -R0/6.5/-0.2/6.5 -I0.1 -Sc -V -Z1 -GS1987.nc
      gmt plot -R0/6.5/-0.2/6.5 -JX6i -B -Sc0.1 -Gblack @Table_5_11.txt
      gmt grdcontour -C25 -A50 S1987.nc
    gmt end show

To use Cartesian splines in tension but only evaluate the solution where
the input mask grid is not NaN, try::

    gmt greenspline @Table_5_11.txt -Tmask.nc -St0.5 -V -Z1 -GWB1998.nc

To use Cartesian generalized splines in tension and return the magnitude
of the surface slope in the NW direction, try::

    gmt greenspline @Table_5_11.txt -R0/6.5/-0.2/6.5 -I0.1 -Sr0.95 -V -Z1 -Q-45 -Gslopes.nc

To use Cartesian cubic splines and evaluate the cumulative solution as a function of eigenvalue,
using the output template with three digits for the eigenvalue, try::

    gmt greenspline @Table_5_11.txt -R0/6.5/-0.2/6.5 -I0.1 -Gcontribution_%3.3d.nc -Sc -Z1 -C+M

Finally, to use Cartesian minimum curvature splines in recovering a
surface where the input data is a single surface value (pt.txt) and the
remaining constraints specify only the surface slope and direction
(slopes.txt), use::

    gmt greenspline pt.txt -R-3.2/3.2/-3.2/3.2 -I0.1 -Sc -V -Z1 -Aslopes.txt+f1 -Gslopes.nc

3-d Examples
------------

To create a uniform 3-D Cartesian grid table based on the data in
Table 5.23 in Davis (1986) that contains *x*,\ *y*,\ *z* locations and
a measure of uranium oxide concentrations (in percent), try::

    gmt greenspline @Table_5_23.txt -R5/40/-5/10/5/16 -I0.25 -Sr0.85 -V -Z5 > 3D_UO2.txt

To instead write the results as a series of 2-D layer grids called layer_*z*.grd, try::

    gmt greenspline @Table_5_23.txt -R5/40/-5/10/5/16 -I0.25 -Sr0.85 -V -Z5 -G3D_UO2_%g.grd

Finally, to write the result to a 3-D netCDF grid, try::

    gmt greenspline @Table_5_23.txt -R5/40/-5/10/5/16 -I0.25 -Sr0.85 -V -Z5 -G3D_UO2.nc

2-d Spherical Surface Examples
------------------------------

To recreate Parker's [1994] example on a global 1x1 degree grid,
assuming the data are in the remote file mag_obs_1990.txt, try::

    gmt greenspline -V -Rg -Sp -Z3 -I1 -GP1994.nc @mag_obs_1990.txt

To do the same problem but applying tension of 0.85, use::

    gmt greenspline -V -Rg -Sq0.85 -Z3 -I1 -GWB2008.nc @mag_obs_1990.txt

Considerations
--------------

#. For the Cartesian cases we use the free-space Green functions, hence
   no boundary conditions are applied at the edges of the specified domain.
   For most applications this is fine as the region typically is
   arbitrarily set to reflect the extent of your data. However, if your
   application requires particular boundary conditions then you may
   consider using :doc:`surface` instead.

#. In all cases, the solution is obtained by inverting a *n* x *n*
   double precision matrix for the Green function coefficients, where *n*
   is the number of data constraints. Hence, your computer's memory may
   place restrictions on how large data sets you can process with
   **greenspline**. Pre-processing your data with doc:`blockmean`,
   doc:`blockmedian`, or doc:`blockmode` is recommended to avoid aliasing and
   may also control the size of *n*. For information, if *n* = 1024 then
   only 8 Mb memory is needed, but for *n* = 10240 we need 800 Mb. Note
   that **greenspline** is fully 64-bit compliant if compiled as such.
   For spherical data you may consider decimating using doc:`gmtspatial`
   nearest neighbor reduction.

#. The inversion for coefficients can become numerically unstable when
   data neighbors are very close compared to the overall span of the data.
   You can remedy this by pre-processing the data, e.g., by averaging
   closely spaced neighbors. Alternatively, you can improve stability by
   using the SVD solution and discard information associated with the
   smallest eigenvalues (see **-C**).

#. The series solution implemented for **-Sq** was developed by
   Robert L. Parker, Scripps Institution of Oceanography, which we
   gratefully acknowledge.

#. If you need to fit a certain 1-D spline through your data
   points you may wish to consider :doc:`sample1d` instead.
   It will offer traditional splines with standard boundary conditions
   (such as the natural cubic spline, which sets the curvatures at the ends
   to zero).  In contrast, **greenspline**\ 's 1-D spline, as is explained in
   note 1, does *not* specify boundary conditions at the end of the data domain.

Tension
-------

Tension is generally used to suppress spurious oscillations caused by
the minimum curvature requirement, in particular when rapid gradient
changes are present in the data. The proper amount of tension can only
be determined by experimentation. Generally, very smooth data (such as
potential fields) do not require much, if any tension, while rougher
data (such as topography) will typically interpolate better with
moderate tension. Make sure you try a range of values before choosing
your final result. **Note**: The regularized spline in tension is only
stable for a finite range of *scale* values; you must experiment to find
the valid range and a useful setting. For more information on tension
see the references below.

References
----------

Davis, J. C., 1986, *Statistics and Data Analysis in Geology*, 2nd
Edition, 646 pp., Wiley, New York,

Mitasova, H., and L. Mitas, 1993, Interpolation by regularized spline
with tension: I. Theory and implementation, *Math. Geol.*, **25**,
641-655.

Parker, R. L., 1994, *Geophysical Inverse Theory*, 386 pp., Princeton
Univ. Press, Princeton, N.J.

Sandwell, D. T., 1987, Biharmonic spline interpolation of Geos-3 and
Seasat altimeter data, *Geophys. Res. Lett.*, **14**, 139-142.

Wessel, P., and D. Bercovici, 1998, Interpolation with splines in
tension: a Green's function approach, *Math. Geol.*, **30**, 77-93.

Wessel, P., and J. M. Becker, 2008, Interpolation using a generalized
Green's function for a spherical surface spline in tension, *Geophys. J.
Int*, **174**, 21-28.

Wessel, P., 2009, A general-purpose Green's function interpolator,
*Computers & Geosciences*, **35**, 1247-1254, doi:10.1016/j.cageo.2008.08.012.

See Also
--------

:doc:`gmt`, :doc:`gmtmath`,
:doc:`nearneighbor`, :doc:`plot`,
:doc:`sample1d`,
:doc:`sphtriangulate`,
:doc:`surface`,
:doc:`triangulate`, :doc:`xyz2grd`
