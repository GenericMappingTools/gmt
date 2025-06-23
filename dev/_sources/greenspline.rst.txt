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
|-G|\ *grdfile*
[ |-A|\ *gradfile*\ **+f**\ **0**\|\ **1**\|\ **2**\|\ **3**\|\ **4**\|\ **5** ]
[ |-C|\ [[**n**\|\ **r**\|\ **v**]\ *value*\ [%]][**+c**][**+f**\ *file*][**+i**][**+n**] ]
[ |SYN_OPT-D3| ]
[ |-E|\ [*misfitfile*][**+r**\ *reportfile*] ]
[ |-I|\ *xinc*\ [/*yinc*\ [/*zinc*]] ]
[ |-L|\ [**t**][**r**] ]
[ |-N|\ *nodefile* ]
[ |-Q|\ [*az*\|\ *x/y/z*] ]
[ |-R|\ *xmin*/*xmax*\ [/*ymin*/*ymax*\ [/*zmin*/*zmax*]] ]
[ |-S|\ **c**\|\ **l**\|\ **p**\|\ **q**\|\ **r**\|\ **t**\ [*tension*\ [/*scale*]][**+e**\ *limit*][**+n**\ *odd*] ]
[ |-T|\ *maskgrid* ]
[ |SYN_OPT-V| ]
[ |-W|\ [**w**]]
[ |-Z|\ *mode* ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**greenspline** uses the Green's function :math:`g(\mathbf{x}; \mathbf{x}')` for the
chosen spline and geometry to interpolate data at regular [or arbitrary]
output locations. Choose between minimum curvature, regularized, or
continuous curvature splines in tension for either 1-D, 2-D, or 3-D
Cartesian coordinates or spherical surface coordinates. Mathematically, the solution is composed as

.. math::

    w(\mathbf{x}) = T(\mathbf{x}) + \sum_{j=1}^{n} \alpha_j g(\mathbf{x}; \mathbf{x}_j),

where :math:`\mathbf{x}` is the output location, :math:`n` is the number of points,
:math:`T(\mathbf{x})` is a trend function, and :math:`\alpha_j` are the *n*
unknown weights we must solve for. Typically, :math:`T(\mathbf{x})` is a linear
or planar trend (Cartesian geometries) or mean value (spherical surface) and a
least-squares solution is determined and removed from the data, yielding data
residuals (:math:`\Delta w_i = w_i - T(\mathbf{x}_i)`); these are then
normalized for numerical stability. The unknown coefficients :math:`\alpha_j`
are determined by requiring the solution to fit the observed residual data exactly:

.. math::

    \Delta w(\mathbf{x}_i) = \sum_{j=1}^{n} \alpha_j g(\mathbf{x}_i; \mathbf{x}_j), \quad i = 1,n


yielding a :math:`n \times n` linear system to be solved for the coefficients.

If there are also *m* observed constraints on the gradient *s* of the curve or surface (|-A|) then we must
add additional *m* unknown coefficients and use the gradient of the Green's functions to satisfy
the *m* extra constraints:

.. math::

    s(\mathbf{x}_k) = \nabla w(\mathbf{x}_k) = \sum_{j=1}^{n} \alpha_j \nabla g(\mathbf{x}_k; \mathbf{x}_j) \mathbf{n}_k, \quad k = 1,m

where the gradient of the Green's functions is dotted with the unit vector of the observed gradient.

Finally, away from the data constraints the Green's function must satisfy

.. math::

    \nabla^2  \left [ \nabla^2 - p^2 \right ] g(\mathbf{x}; \mathbf{x}') = \delta (\mathbf{x} - \mathbf{x}'),


where :math:`\nabla^2` is the Laplacian operator, :math:`\delta` is the
Dirac Delta function, and :math:`p` is the tension (if desired). This
solution yields an exact interpolation of the supplied data points.
Alternatively, you may choose to perform a singular value decomposition
(SVD) and eliminate the contribution from the smallest eigenvalues; this
approach yields an approximate solution. Trends and normalization scales are restored
when evaluating the output.

Required Arguments
------------------

*table*
    The name of one or more ASCII [or binary, see
    **-bi**] files holding the **x**, *w* data
    points. If no file is given then we read standard input instead.

.. _-G:

**-G**\ *grdfile*
    Name of resulting output file. (1) If options |-R|, |-I|, and
    possibly **-r** are set we produce an equidistant output table. This
    will be written to standard output unless |-G| is specified. **Note**: For 2-D
    grids the |-G| option is required. (2) If option |-T| is
    selected then |-G| is required and the output file is a 2-D binary
    grid file. Applies to 2-D interpolation only. (3) For 3-D cubes
    the |-G| option is optional.  If set, it can be the name of a 3-D
    cube file or a filename template with a floating-point C-format identifier
    in it so that each layer is written to a 2-D grid file; otherwise
    we write (*x, y, z, w*) records to standard output. (4) If |-N| is
    selected then the output is an ASCII (or binary; see
    **-bo**) table; if |-G| is not given then
    this table is written to standard output. Ignored if |-C| or
    **-C**\ 0 is given.

Optional Arguments
------------------

.. _-A:

**-A**\ *gradfile*\ **+f**\ **0**\|\ **1**\|\ **2**\|\ **3**\|\ **4**\|\ **5**
    The solution will partly be constrained by surface gradients
    :math:`\mathbf{v} = v \hat{\mathbf{n}}`, where :math:`v` is the gradient
    magnitude and :math:`\hat{\mathbf{n}}` its unit vector direction.
    The gradient direction may be specified either by Cartesian components
    (either unit vector :math:`\hat{\mathbf{n}}` and magnitude :math:`v` separately
    or gradient components :math:`\mathbf{v}` directly) or
    angles w.r.t. the coordinate axes. Append name of ASCII file with
    the surface gradients.  Use modifier **+f** to select one of five input
    formats:

    - **0** - For 1-D data there is no direction, just gradient magnitude (slope) so
      the input format is *x*, :math:`v` (1-D data set).
    - **1** - Records contain *x*, *y*, *azimuth*, :math:`v` (*azimuth* in degrees is
      measured clockwise from the vertical (north) [Default] (2-D data set).
    - **2** - Records contain *x*, *y*, :math:`v`, *azimuth* (*azimuth* in degrees is
      measured clockwise from the vertical (north);  2-D data set).
    - **3** - Records contain **x**, *direction(s)*, :math:`v` (*direction(s)* in degrees
      are measured counter-clockwise from the horizontal, and for 3-D the vertical axis 
      (2-D or 3-D data set).
    - **4** - Records contain **x**, :math:`\mathbf{v}` (2-D or 3-D data set).
    - **5** - Records contain **x**, :math:`\hat{\mathbf{n}}`, :math:`v` (2-D or 3-D data set).
    
    **Note**: The slope constraints must not be at the same locations as the
    data constraints. That scenario has not yet been implemented.

.. _-C:

**-C**\ [[**n**\|\ **r**\|\ **v**]\ *value*\ [%]][**+c**][**+f**\ *file*][**+i**][**+n**]
    Find an approximate surface fit: Solve the linear system for the
    spline coefficients by SVD and eliminate the contribution from smaller
    eigenvalues [Default uses Gauss-Jordan elimination to solve the linear system
    and fit the data exactly (unless |-W| is used)]. Append a directive and *value*
    to determine which eigenvalues to keep:

    - **n** - Retain only the *value* numbers largest eigenvalues [all]. Optionally,
      append % to indicate *value* is given in percentage.
    - **r** - Retain those eigenvalues whose ratio to the largest eigenvalue is less than
      *value* [Default, with *value* = 0].
    - **v** - Retain the eigenvalues needed to ensure the model prediction variance fraction
      is at least *value*. Optionally, append % to indicate *value* is given in percentage.

    Several optional modifiers are available:

    - **+c** - Produce the cumulative sum of these contributions, one grid per eigenvalue (2-D only).
    - **+f** - Append *file* to save the eigenvalues to the specified file for further analysis.
    - **+n** - If given then **+f**\ *file* is required and execution will
      stop after saving the eigenvalues, i.e., no surface output is produced. 
    - **+i** - Produce the incremental sum of these contributions, one grid per eigenvalue (2-D only).
        
    **Notes**: (1) Modifiers **++c** and **+i** require a file name with a suitable extension
    to be given via |-G| (we automatically insert "_cum_###" or "_inc_###" before the
    extension, using a fixed integer format for the eigenvalue number, starting at 0).
    (2) Use both modifiers to write both types of intermediate grids.

.. _-D:

.. include:: explain_-D_cap.rst_

.. _-E:

**-E**\ [*misfitfile*][**+r**\ *reportfile*]
    Evaluate the spline exactly at the input data locations and report
    statistics of the misfit (mean, standard deviation, and rms).  Optionally,
    append a filename and we will write the data table, augmented by
    two extra columns holding the spline estimate and the misfit. Alternatively,
    if |-C| is used and history is computed (via one or more of modifiers **+c**
    and **+i**), then we will instead write a table with eigenvalue number,
    eigenvalue, percent of model variance explained, and rms misfit.  If |-W|
    is used we also append :math:`\chi^2`. Optionally append this modifier:

    - **+r** - Write misfit and variance evaluation statistics to *reportfile*.
      Output order is *Data Model Explained(%) N Mean Std.dev RMS*. If |-W| is
      used we add :math:`\chi^2` as a final 8th column. **Note**: If **+r** is
      not used then |-V|\ **i** will report the information via verbose messages.

.. _-I:

**-I**\ *xinc*\ [/*yinc*\ [/*zinc*]]
    Specify equidistant sampling intervals, on for each dimension, separated by slashes.

.. _-L:

**-L**\ [**t**][**r**]
    Specifically control how we detrend (i.e., adjust :math:`T(\mathbf{x})`)
    and normalize the data (and possibly gradients) prior to determining the
    solution coefficients.  The order of adjustments is always the same even if
    some steps may be deselected:

    - We always determine and then remove and restore the mean data value :math:`\bar{w}`.
    - We determine a linear least-squares trend (directive **t**) and remove this trend
      from residual data and slopes. **Note**: For spherical and 3-D interpolation the
      **t** directive is not available.
    - We determine the maximum absolute value of the minimum and maximum data residuals
      (directive **r**) and normalize the residual data and slopes by that range value.

    After evaluating the solution based on the residuals, we undo any normalization and
    detrending in reverse order.
    Use **-L** and specifically append one or both of these directives to override
    the default. If no directives are given then no detrending nor normalization will
    take place. 

.. _-N:

**-N**\ *nodefile*
    ASCII file with coordinates of desired output locations **x** in the
    first column(s). The resulting *w* values are appended to each
    record and written to the file given in |-G| [or standard output if not
    specified]; see **-bo** for binary output
    instead. This option eliminates the need to specify options |-R|,
    |-I|, and **-r**.

.. _-Q:

**-Q**\ [*az*\|\ *x/y/z*]
    Rather than evaluate the solution *w*\ (**x**), take the first derivative of
    a 1-D solution.  For 2-D, select directional derivative in
    the *az* azimuth and return the magnitude of this derivative
    instead. For a 3-D interpolation, specify the three components of the
    desired vector direction (the vector will be normalized before use).

.. _-R:

**-R**\ *xmin*/*xmax*\ [/*ymin*/*ymax*\ [/*zmin*/*zmax*]]
    Specify the domain for an equidistant lattice where output
    predictions are required. Requires |-I| and optionally **-r**.

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

**-S**\ **c**\|\ **l**\|\ **p**\|\ **q**\|\ **r**\|\ **t**\ [*tension*\ [/*scale*]][**+e**\ *limit*][**+n**\ *odd*]
    Select one of six different splines. Some are 1-D, 2-D, or 3-D Cartesian splines
    (see |-Z| for discussion). Note that all *tension* values are expected to be
    normalized tension in the range 0 < *tension* < 1. Choose among these directives:

    - **c** - Minimum curvature spline [*Sandwell*, 1987] (1-D, 2-D, or 3-D Cartesian spline).
    - **l** - Linear or bilinear spline; these produce output that do
      not exceed the range of the given data (1-D or 2-D Cartesian spline).
    - **p** - Minimum curvature spline [*Parker*, 1994] (spherical surface splines and implies **-Z**).
    - **q** - Continuous curvature spline in tension [*Wessel and Becker*, 2008]; append *tension*. The
      :math:`g(\mathbf{x}; \mathbf{x}')` for the last method is slower to compute (a series solution),
      so we pre-calculate values and use cubic spline interpolation lookup instead (spherical surface
      spline and implies **-Z**).
    - **r** - Regularized spline in tension [*Mitasova and Mitas*, 1993]; again,
      append *tension* and optional *scale* (2-D or 3-D spline).
    - **t** - Continuous curvature spline in tension [*Wessel and Bercovici*, 1998];
      append *tension*\ [/*scale*] with *tension* in the 0-1 range and optionally
      supply a length *scale* [Default is the average grid spacing] (1-D, 2-D, or 3-D Cartesian spline).

    **Note**: Directive **q** may take two optional modifiers:
  
    - **+e** - The finite Legendre sum has a truncation error [1e-6]; you can lower that by
      appending *limit* at the expense of longer run-time.
    - **+n** - Change how many  points to use in the spline setup by appending *odd* [10001]
      (must be an odd integer).

.. _-T:

**-T**\ *maskgrid*
    For 2-D interpolation only. Only evaluate the solution at the nodes
    in the *maskgrid* that are not equal to NaN. This option eliminates
    the need to specify options |-R|, |-I|, and **-r**.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [**w**]
   Data one-sigma uncertainties are provided in the last column.
   We then compute weights that are inversely proportional to the uncertainties squared.
   Append **w** if weights are given instead of uncertainties and then they will be used
   as is (no squaring).  This results in a weighted least squares fit.  Note that this
   only has an effect if |-C| is used.  [Default uses no weights or uncertainties].

.. _-Z:

**-Z**\ *mode*
    Sets the distance mode that determines how we calculate distances
    between data points. Select *mode* 0 for Cartesian 1-D spline
    interpolation: **-Z**\ 0 means (*x*) in user units, Cartesian
    distances. Select *mode* 1-3 for Cartesian 2-D surface spline
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

.. include:: explain_-w.rst_

.. include:: explain_core.rst_

.. include:: explain_help.rst_

1-D Examples
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

2-D Examples
------------

To make a uniform grid using the minimum curvature spline for the same
Cartesian data set from Table 5.11 in Davis (1986) that is used in the
GMT Technical Reference example 16, try::

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
using output file based on the main grid name (such as contribution_cum_033.nc), try::

    gmt greenspline @Table_5_11.txt -R0/6.5/-0.2/6.5 -I0.1 -Gcontribution.nc -Sc -Z1 -C+c

Finally, to use Cartesian minimum curvature splines in recovering a
surface where the input data represent a single surface value (pt.txt) and the
remaining constraints specify only the surface slope and direction
(slopes.txt), use::

    gmt greenspline pt.txt -R-3.2/3.2/-3.2/3.2 -I0.1 -Sc -V -Z1 -Aslopes.txt+f1 -Gslopes.nc

3-D Examples
------------

To create a uniform 3-D Cartesian grid table based on the data in
Table 5.23 in Davis (1986) that contains *x*,\ *y*,\ *z* locations and
a measure of uranium oxide concentrations (in percent), try::

    gmt greenspline @Table_5_23.txt -R5/40/-5/10/5/16 -I0.25 -Sr0.85 -V -Z5 > 3D_UO2.txt

To instead write the results as a series of 2-D layer grids called layer_*z*.grd, try::

    gmt greenspline @Table_5_23.txt -R5/40/-5/10/5/16 -I0.25 -Sr0.85 -V -Z5 -G3D_UO2_%g.grd

Finally, to write the result to a 3-D netCDF grid, try::

    gmt greenspline @Table_5_23.txt -R5/40/-5/10/5/16 -I0.25 -Sr0.85 -V -Z5 -G3D_UO2.nc

2-D Spherical Surface Examples
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
   **greenspline**. Pre-processing your data with :doc:`blockmean`,
   :doc:`blockmedian`, or :doc:`blockmode` is recommended to avoid aliasing and
   may also control the size of *n*. For information, if *n* = 1024 then
   only 8 Mb memory is needed, but for *n* = 10240 we need 800 Mb. Note
   that **greenspline** is fully 64-bit compliant if compiled as such.
   For spherical data you may consider decimating using :doc:`gmtspatial`
   nearest neighbor reduction.

#. The inversion for coefficients can become numerically unstable when
   data neighbors are very close compared to the overall span of the data.
   You can remedy this by preprocessing the data, e.g., by averaging
   closely spaced neighbors. Alternatively, you can improve stability by
   using the SVD solution and discard information associated with the
   smallest eigenvalues (see |-C|).

#. The series solution implemented for **-Sq** was developed by
   Robert L. Parker, Scripps Institution of Oceanography, which we
   gratefully acknowledge.

#. If you need to fit a certain 1-D spline through your data
   points you may wish to consider :doc:`sample1d` instead.
   It will offer traditional splines with standard boundary conditions
   (such as the natural cubic spline, which sets the curvatures at the ends
   to zero).  In contrast, **greenspline**\ 's 1-D spline, as is explained in
   note 1, does *not* specify boundary conditions at the end of the data domain.

#. It may be difficult to know how many eigenvalues are needed for a suitable
   approximate fit.  The |-C| modifiers allow you to explore this further
   by creating solutions for all cutoff selections and estimate model variance
   and data misfit as a function of how many eigenvalues are used.  The large
   set of such solutions can be animated so it is easier to explore the changes
   between solutions and to make a good selection for the |-C| directive values.
   See the animations for one or more examples of this exploration.

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

Deprecations
------------

- 6.3.0: Replace **+m** and **+M** modifiers for |-C|. `#5714 <https://github.com/GenericMappingTools/gmt/pull/5714>`_
- 6.3.0: Use **+n** instead of negative value for |-C| to set dry-run. `#5725 <https://github.com/GenericMappingTools/gmt/pull/5725/>`_

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
tension: a Green's function approach, *Math. Geol.*, **30**, 77-93,
https://doi.org/10.1023/A:1021713421882.

Wessel, P., and J. M. Becker, 2008, Interpolation using a generalized
Green's function for a spherical surface spline in tension, *Geophys. J.
Int*, **174**, 21-28, https://doi.org/10.1111/j.1365-246X.2008.03829.x.

Wessel, P., 2009, A general-purpose Green's function interpolator,
*Computers & Geosciences*, **35**, 1247-1254, https://doi.org/10.1016/j.cageo.2008.08.012.

See Also
--------

:doc:`gmt`, :doc:`gmtmath`,
:doc:`nearneighbor`, :doc:`plot`,
:doc:`sample1d`,
:doc:`sphtriangulate`,
:doc:`surface`,
:doc:`triangulate`, :doc:`xyz2grd`
