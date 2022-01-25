.. index:: ! gpsgridder
.. include:: ../module_supplements_purpose.rst_

**********
gpsgridder
**********

|gpsgridder_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt gpsgridder** [ *table* ]
|-G|\ *outgrid*
[ |-C|\ [[**n**\|\ **r**\|\ **v**]\ *value*\ [%]][**+c**][**+f**\ *file*][**+i**][**+n**] ]
[ |-E|\ [*misfitfile*] ]
[ |-F|\ [**d**\|\ **f**]\ *fudge* ]
[ |SYN_OPT-I| ]
[ |-L| ]
[ |-N|\ *nodefile* ]
[ |SYN_OPT-R| ]
[ |-S|\ *nu* ]
[ |-T|\ *maskgrid* ]
[ |SYN_OPT-V| ]
[ |-W|\ [**+s**\|\ **w**] ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**gpsgridder** grids 2-D vector data such as GPS velocities by using a coupled
model based on 2-D elasticity.  The degree of coupling can be tuned by adjusting
the effective Poisson's ratio, :math:`\nu`. The solution field can be tuned to extremes such
as incompressible (1), typical elastic (0.5) or even an unphysical value of -1
that basically removes the elastic coupling of vector interpolation.  Smoothing
is offered via the optional elimination of small eigenvalues.  The solutions
for the two component grids are evaluated as

.. math::

    u(\mathbf{x}) = \sum_{j=1}^{n} \alpha_j q(\mathbf{x}, \mathbf{x}_j) +  \beta_j w(\mathbf{x}, \mathbf{x}_j)\\
    v(\mathbf{x}) = \sum_{j=1}^{n} \alpha_j w(\mathbf{x}, \mathbf{x}_j) +  \beta_j p(\mathbf{x}, \mathbf{x}_j)

where the three 2-D elastic coupled Green's functions are given by

.. math::

    q(\mathbf{a}, \mathbf{b}) = (3 - \nu)\log r + (1 + \nu) \frac{y^2}{r^2}\\
    p(\mathbf{a}, \mathbf{b}) = (3 - \nu)\log r + (1 + \nu) \frac{x^2}{r^2}\\
    w(\mathbf{a}, \mathbf{b}) = -(1 + \nu) \frac{xy}{r^2}

Here, *r* is the radial distance between points **a** and **b** and *x* and *y* are the components of that
distance.  The body forces :math:`\alpha_j` and :math:`\beta_j` are obtained by evaluating the solution at the data
locations and inverting the square linear system that results; see *Sandwell and Wessel* [2016] and
*Haines et al.* [2015] for details.

Required Arguments
------------------

*table*
    table with GPS strain rates at discrete locations.  We expect the input
    format to be *x y u v* [ *du dv* ] (see **-W** to specify data uncertainties
    or weights).  If *lon lat* is given you must supply **-fg** and we will
    use a flat Earth approximation in the calculation of distances.

.. _-G:

.. |Add_outgrid| replace:: Name of resulting output grids(s). (1) If options |-R|, |-I|, and possibly **-r** are set
    we produce two equidistant output grids. In this case, we take *outgrid* and append "_u" and "_v" before the extension,
    respectively. (2) If option |-T| is selected then |-R|, |-I| cannot be given as the *maskgrid* determines the region
    and increments. The two output grid names are generated as under (1). (3) If |-N| is selected then the output is a
    single ASCII (or binary; see **-bo**) table written to *outfile*; if **-G** is not given then this table is written to
    standard output.
.. include:: /explain_grd_inout.rst_
    :start-after: outgrid-syntax-begins
    :end-before: outgrid-syntax-ends

Optional Arguments
------------------

.. _-C:

**-C**\ [[**n**\|\ **r**\|\ **v**]\ *value*\ [%]][**+c**][**+f**\ *file*][**+i**][**+n**]
    Find an approximate surface fit: Solve the linear system for the
    spline coefficients by SVD and eliminate the contribution from smaller
    eigenvalues [Default uses Gauss-Jordan elimination to solve the linear system
    and fit the data exactly (unless **-W** is used)]. Append a directive and *value*
    to determine which eigenvalues to keep: **n** will retain only the *value* largest
    eigenvalues [all], **r** [Default] will retain those eigenvalues whose ratio
    to the largest eigenvalue is less than *value* [0], while **v** will retain
    the eigenvalues needed to ensure the model prediction variance fraction is at
    least *value*. For **n** and **v** you may append % if *value* is given as a
    *percentage* of the total instead.  Several optional modifiers are available:
    Append **+f**\ *file* to save the eigenvalues to the specified file for further
    analysis. If **+n** is given then **+f**\ *file* is required and execution will
    stop after saving the eigenvalues, i.e., no surface output is produced.
    The two other modifiers (**+c** and **+i**) can be used to
    write intermediate grids, two (*u* and *v*) per eigenvalue, and we will
    automatically insert "_cum_###" or "_inc_###" before the file extension,
    using a fixed integer format for the eigenvalue number starting at 0.  The
    **+i** modifier will write the **i**\ ncremental contributions to the grids
    for each eigenvalue, while **+c** will instead produce the **c**\ umulative
    sum of these contributions. Use both modifiers to write both types of
    intermediate grids.

.. _-E:

**-E**\ [*misfitfile*]
    Evaluate the spline exactly at the input data locations and report
    statistics of the misfit (mean, standard deviation, and rms) for *u* and
    *v* separately and combined.  Optionally, append a filename and we will
    write the data table, augmented by two extra columns after each of the
    *u* and *v* columns holding the spline estimates and misfits. If **-W**
    is given we also add two more columns with :math:`\chi_u^2` and :math:`\chi_v^2`
    values. Alternatively, if **-C** is used and history is computed (via one
    or more of modifiers **+c** and **+i**), then we will instead write a table
    with eigenvalue number, eigenvalue, percent of model variance explained,
    and overall rms, rms_u, and rms_v misfits.  If **-W** is used we also append
    :math:`\chi^2`, :math:`\chi_u^2`, and :math:`\chi_v^2`.

.. _-F:

**-F**\ [**d**\|\ **f**]\ *fudge*\
    The Green's functions are proportional to :math:`r^{-2}` and :math:`\log(r)`
    and thus blow up for *r == 0*.  To prevent that we offer two fudging schemes:
    **-Fd**\ *del_radius* lets you add a constant offset to all radii
    and must be specified in the user units.  Alternatively, use
    **-Ff**\ *factor* which will compute *del_radius* from the product
    of the shortest inter-point distance and *factor* [0.01].

.. _-I:

.. include:: ../../explain_-I.rst_

.. _-L:

**-L**
    Leave trend alone.  Do *not* remove a planer (2-D) trend from the
    data before fitting the spline.  [Default removes least squares plane,
    fits normalized residuals, and restores plane].

.. _-N:

**-N**\ *nodefile*
    ASCII file with coordinates of desired output locations **x** in the
    first column(s). The resulting *w* values are appended to each
    record and written to the file given in **-G** [or standard output if not
    specified]; see **-bo** for binary output
    instead. This option eliminates the need to specify options **-R**,
    **-I**, and **-r**.

.. |Add_-R| replace:: |Add_-R_links|
.. include:: ../../explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-S:

**-S**\ *nu*
    Specify Poisson's ratio to use for this 2-D elastic sheet [0.5].
    **Note**: 1.0 is incompressible in a 2-D formulation while -1
    removes all coupling between the two directions.

.. _-T:

**-T**\ *maskgrid*
    Only evaluate the solutions at the nodes
    in the *maskgrid* that are not set to NaN. This option eliminates
    the need to specify options **-R**, **-I** (and **-r**).

.. _-W:

**-W**\ [**+s**\|\ **w**]
   One-sigma data uncertainties for *u* and *v* are provided in the last two columns.
   We then compute least squares weights that are inversely proportional to the square
   of the uncertainties [Default, or **+s**].  Instead, append **+w** if weights are
   given instead of uncertainties, in which case we just use the weights as provided
   (no squaring).  This results in a weighted least squares fit.  Note that **-W**
   only has an effect if **-C** is used [Default uses no weights or uncertainties].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

**-fg**
   Geographic grids (dimensions of longitude, latitude) will be converted to
   meters via a "Flat Earth" approximation using the current ellipsoid parameters.

.. |Add_-h| replace:: Not used with binary data.
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-qi.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_distunits.rst_

Notes on SVD solution
---------------------

It may be difficult to know how many eigenvalues are needed for a suitable
approximate fit.  The **-C** modifiers allow you to explore this further
by creating solutions for all cutoff selections and estimate model variance
and data misfit as a function of how many eigenvalues are used.  The large
set of such solutions can be animated so it is easier to explore the changes
between solutions and to make a good selection for the **-C** directive values.
See the animations for one or more examples of this exploration.

Examples
--------

To compute the *u* and *v* strain rate grids from the GPS data set *gps.txt*,
containing *x y u v du dv*, on a 2x2 arc minute grid for California, and just
using about 25% of the largest eigenvalues, try::

    gmt gpsgridder gps.txt -R-125/-114/31/41 -I2m -fg -W -r -Cn25% -Ggps_strain_%s.nc -V

Deprecations
------------

- 6.3.0: Use **+n** instead of negative value for **-C** to set dry-run. `#5725 <https://github.com/GenericMappingTools/gmt/pull/5725/>`_

References
----------

Haines, A. J. et al., 2015, *Enhanced Surface Imaging of Crustal Deformation*, SpringerBriefs in Earth Sciences,
doi:10.1007/978-3-319-21578-5_2.

Sandwell, D. T. and P. Wessel, 2016, Interpolation of 2-D Vector Data Using Constraints from Elasticity,
*Geophys. Res. Lett., 43*, 10,703-10,709,
`https://dx.doi.org/10.1002/2016GL070340 <https://dx.doi.org/10.1002/2016GL070340>`_

See Also
--------

:doc:`gmt </gmt>`,
:doc:`greenspline </greenspline>`
:doc:`nearneighbor </nearneighbor>`,
:doc:`surface </surface>`
