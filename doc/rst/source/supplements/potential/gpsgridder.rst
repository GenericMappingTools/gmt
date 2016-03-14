.. index:: ! gpsgridder

**********
gpsgridder
**********

.. only:: not man

    gpsgridder - Interpolate GPS strain vectors using Green's functions for elastic deformation

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gpsgridder** [ *table* ]
|-G|\ *outfile*
[ |SYN_OPT-I| ]
[ |SYN_OPT-R| ]
[ |-C|\ [**n**\ \|\ **v**]\ *cut*\ [/*file*] ]
[ |-F|\ [**d**\ \|\ **f**]\ *fudge*\ ]
[ |-L| ]
[ |-N|\ *nodefile* ]
[ |-S|\ *nu* ]
[ |-T|\ *maskgrid* ]
[ |SYN_OPT-V| ]
[ |-W|\ **w**]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**gpsgridder** grids 2-D vector data such as GPS velocities by using a coupled
model based on 2-D elasticity.

Required Arguments
------------------

*table*
    table with GPS strain rates at discrete locations.  We expect the input
    format *x y u v* [ *du dv* ] (see **-W** to specify data uncertainties
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

**-C**\ [**n**\ \|\ **v**]\ *cut*\ [/*file*]
    Find an approximate surface fit: Solve the linear system for the
    spline coefficients by SVD and eliminate the contribution from all
    eigenvalues whose ratio to the largest eigenvalue is less than *cut*
    [Default uses Gauss-Jordan elimination to solve the linear system
    and fit the data exactly]. Optionally, append /*file* to save the
    eigenvalue ratios to the specified file for further analysis.
    Finally, if a negative *cut* is given then /*file* is required and
    execution will stop after saving the eigenvalues, i.e., no surface
    output is produced.  Specify **-Cv** to use the
    largest eigenvalues needed to explain *cut* % of the data variance.
    Alternatively, use **-Cn** to select the *cut* largest eigenvalues.
    If a *file* is given with **-Cv** then we save the eigenvalues instead
    of the ratios.

.. _-F:

|-F|\ [**d**\ \|\ **f**]\ *fudge*\
    The Green's functions are proportional to terms like 1/r^2 and log(r)
    and thus blow up for r == 0.  To prevent that we offer two schemes:
    **-Fd**\ *del_radius* lets you add a constant offset to all radii
    and must be specified in the user units.  Alternatively, use
    **-Ff**\ *factor* which will compute *del_radius* from the product
    of the shortest inter-point distance and $factor* [0.01].

.. _-I:

.. include:: ../../explain_-I.rst_

.. _-L:

**-L**
    Do *not* remove a linear (1-D) or planer (2-D) trend when **-D**
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

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

.. _-S:

**-S**\ *nu*
    Specify Poisson's ratio to use for this elastic sheet [0.25].

.. _-T:

**-T**\ *maskgrid*
    Only evaluate the solutions at the nodes
    in the *maskgrid* that are not set to NaN. This option eliminates
    the need to specify options **-R**, **-I** (and **-r**). 

.. _-W:

**-W**\ **w**
    Data one-sigma uncertainties for *u* and *v* are provided in the last two columns.
    Append **w** if weights are given instead of uncertainties [no weights or uncertainties].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

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


See Also
--------

:doc:`gmt </gmt>`,
:doc:`greenspline </greenspline>`
:doc:`nearneighbor </nearneighbor>`,
:doc:`surface </surface>`
