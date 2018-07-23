.. index:: ! gpsgridder

**********
gpsgridder
**********

.. only:: not man

    gpsgridder - Interpolate GPS strain vectors using Green's functions for elastic deformation

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt gpsgridder** [ *table* ]
|-G|\ *outfile*
[ |SYN_OPT-I| ]
[ |SYN_OPT-R| ]
[ |-C|\ [**n**\ ]\ *value*\ [**+f**\ *file*] ]
[ |-E|\ [*misfitfile*] ]
[ |-F|\ [**d**\ \|\ **f**]\ *fudge*\ ]
[ |-L| ]
[ |-N|\ *nodefile* ]
[ |-S|\ *nu* ]
[ |-T|\ *maskgrid* ]
[ |SYN_OPT-V| ]
[ |-W|\ [**w**]]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**gpsgridder** grids 2-D vector data such as GPS velocities by using a coupled
model based on 2-D elasticity.  The degree of coupling can be tuned by adjusting
the effective Poisson's ratio. The solution field can be tuned to extremes such
as incompressible (1), typical elastic (0.5) or even an unphysical value of -1
that basically removes the elastic coupling of vector interpolation.  Smoothing
is offered via the optional elimination of small eigenvalues.

Required Arguments
------------------

*table*
    table with GPS strain rates at discrete locations.  We expect the input
    format to be *x y u v* [ *du dv* ] (see **-W** to specify data uncertainties
    or weights).  If *lon lat* is given you must supply **-fg** and we will
    use a flat Earth approximation in the calculation of distances.

.. _-G:

**-G**\ *outfile*
    Name of resulting output file. (1) If options **-R**, **-I**, and
    possibly **-r** are set we produce two equidistant output grids. In
    this case, *outfile* must be a name template containing the C format
    specifier %s, which will be replaced with u and v, respectively.
    (2) If option **-T** is selected then **-R**, **-I** cannot be given
    as the *maskgrid* determines the region and increments. Again, the
    *outfile* must be a name template for the two output grids.
    (3) If **-N** is selected then the output is a single ASCII (or binary; see
    **-bo**) table written to *outfile*; if **-G** is not given then
    this table is written to standard output. The **-G** option is ignored
    if **-C** or **-C**\ 0 is given.

Optional Arguments
------------------

.. _-C:

**-C**\ [**n**\ ]\ *value*\ [**+f**\ *file*]
    Find an approximate surface fit: Solve the linear system for the
    spline coefficients by SVD and eliminate the contribution from all
    eigenvalues whose ratio to the largest eigenvalue is less than *value*
    [Default uses Gauss-Jordan elimination to solve the linear system
    and fit the data exactly]. Optionally, append **+f**\ *file* to save the
    eigenvalue ratios to the specified file for further analysis.
    If a negative *value* is given then **+f**\ *file* is required and
    execution will stop after saving the eigenvalues, i.e., no surface
    output is produced.  Specify **-Cn**\ *value* to retain only the *value* largest eigenvalues.
    Note: 1/4 of the total number of data constraints is a good starting point
    for further experiments.

.. _-E:

**E**\ [*misfitfile*]

    Evaluate the spline exactly at the input data locations and report
    statistics of the misfit (mean, standard deviation, and rms) for *u* and
    *v* separately and combined.  Optionally, append a filename and we will
    write the data table, augmented by two extra columns after each of the
    *u* and *v* columns holding the spline estimates and misfits.  If **-W**
    is given we also add two more columns with the chi^2 values.

.. _-F:

|-F|\ [**d**\ \|\ **f**]\ *fudge*\
    The Green's functions are proportional to terms like 1/r^2 and log(r)
    and thus blow up for r == 0.  To prevent that we offer two fudging schemes:
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
    record and written to the file given in **-G** [or stdout if not
    specified]; see **-bo** for binary output
    instead. This option eliminates the need to specify options **-R**,
    **-I**, and **-r**.

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

.. _-S:

**-S**\ *nu*
    Specify Poisson's ratio to use for this 2-D elastic sheet [0.5].
    Note: 1.0 is incompressible in a 2-D formulation while -1
    removes all coupling between the two directions.

.. _-T:

**-T**\ *maskgrid*
    Only evaluate the solutions at the nodes
    in the *maskgrid* that are not set to NaN. This option eliminates
    the need to specify options **-R**, **-I** (and **-r**). 

.. _-W:

**-W**\ [**w**]
   One-sigma data uncertainties for *u* and *v* are provided in the last two columns.
   We then compute least squares weights that are inversely proportional to the square
   of the uncertainties.  Append **w** if weights are given instead of uncertainties,
   in which case we just use the weights as provided (no squaring).  This results in
   a weighted least squares fit.  Note that **-W** only has an effect if **-C** is used.
   [Default uses no weights or uncertainties].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

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

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_distunits.rst_

Examples
--------

To compute the *u* and *v* strain rate grids from the GPS data set *gps.txt*,
containing *x y u v du dv*, on a 2x2 arc minute grid for California, try

   ::

    gmt gpsgridder gps.txt -R-125/-114/31/41 -I2m -fg -W -r -Ggps_strain_%s.nc -V


References
----------

Haines, A. J. et al., 2015, *Enhanced Surface Imaging of Crustal Deformation*, SpringerBriefs in Earth Sciences,
doi:10.1007/978-3-319-21578-5_2.

Sandwell, D. T. and P. Wessel, 2016, Interpolation of 2-D Vector Data Using Constraints from Elasticity,
*Geophys. Res. Lett., 43*, 10,703-10,709, 
`http://dx.doi.org/10.1002/2016GL070340 <http://dx.doi.org/10.1002/2016GL070340>`_

See Also
--------

:doc:`gmt </gmt>`,
:doc:`greenspline </greenspline>`
:doc:`nearneighbor </nearneighbor>`,
:doc:`surface </surface>`
