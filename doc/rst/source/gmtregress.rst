.. index:: ! gmtregress

**********
gmtregress
**********

.. only:: not man

    gmtregress - Linear regression of 1-D data sets

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmtregress** [ *table* ] [ **-A**\ *min*\ /*max*\ /*inc* ] [ **-C**\ *level* ]
[ **Ex**\ \|\ **y**\ \|\ **o**\ \|\ **r** ]
[ **-F**\ *flags* ] [ **-N**\ *norm* ]
[ **-T**\ *min*\ /*max*\ /*inc* \| **T**\ *n* ] [ **-W**\ [**w**]\ [**x**]\ [**y**]\ [**r**] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]

|No-spaces|

Description
-----------

**gmtregress** reads one or more data tables [or *stdin*]
and determines the best linear regression for each segment using the chosen parameters.
The user may specify the output components.  By default, the model will be evaluated at the
input points, but alternatively you can specify an equidistant range over which to evaluate
the model.  Instead of determining the best fit we can perform a scan of all possible regression lines
(for a range of slope angles) and examine how the chosen misfit measure varies with the slope.

Required Arguments
------------------

None

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

**-A**\ *min*\ /*max*\ /*inc*
    Examine all possible regression lines with slope angles between *min* and *max*,
    using steps of *inc* degrees [-90/+90/1].  For each slope the optimum intercept
    is determined based on your **-E** and **-N** settings.  
    We report the four columns *angle*, *E*, *slope*, *intercept*, for
    each input segment; the best model parameters of these are written into the segment header.

**-C**\ *level*
    Set the confidence level (in %) to use for calculation of confidence bands on the regression [95].
    This is only used if **F** includes the column **c**.

**Ex**\ \|\ **y**\ \|\ **o**\ \|\ **r**
    Type of linear regression, i.e., select the type of misfit we should calculate.
    Choose from **x** (regress *x* on *y*; i.e., the misfit is measured horizontally from data point to line),
    **y** (regress *y* on *x*; i.e., the misfit is measured vertically), **o** (orthogonal regression;
    i.e., the misfit is measured from data point to nearest point on the line), or **r** (Reduced Major
    Axis regression; i.e., the misfit is the product of both vertical and horizontal misfits) [**y**].

**-F**\ *flags*
    Append a combination of the columns you wish returned; the output order will match the order specified here.  Choose from
    **x** (observed *x*), **y** (observed *y*), **m** (model prediction), **r** (residual = data minus model),
    **c** (symmetrical confidence interval on the regression; see **-C**
    for the level), and **w** (Reweighted Least Squares weights) [**xymrcw**].

**-N**\ *norm*
    Selects the norm to use for the misfit calculation.  Choose among **1** (L-1 measure; the mean of the
    absolute residuals), **2** (Least-squares; the mean of the squared residuals), or
    **r** (LMS; The least median of the squared residuals) [**2**].  Traditional regression uses L-2
    while L-1 and in particular LMS are more robust in how they handle outliers.

**-T**\ *min*\ /*max*\ /*inc* \| **T**\ *n*
    Solve for the regression parameters and then evaluate the model at the equidistant points implied by the arguments.  If
    **-T**\ *n* is given instead we will reset *min* and *max* to the extreme *x*-values for each segment and determine *inc*
    so that there are exactly *n* output values for each segment.  To skip the model evaluation, simply provide **-T**\ 0.

**-W**\ [**w**]\ [**x**]\ [**y**]\ [**r**]
    Specifies weighted regression and which weights are provided.
    Append **x** if giving 1-sigma uncertainties in *x*, **y** if giving 1-sigma uncertainties in *y*, and
    **r** if giving correlations between *x* and *y* pairs, in the order these columns appear in the input.
    Giving both **xx** and **y** (and optionally **r**) implies an orthogonal regression.
    We will convert uncertainties in *x* and *y* to weights via 1/sigma.
    Use **-Ww** if the we should consider the input columns to have weights instead.

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-aspatial.rst_

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_


Examples
--------

To do a standard least-squares regression on the data points.txt and return the
x, y, model prediction, and 99% confidence intervals, try 

   ::

    gmt gmtregress points.txt -Fxymc -C99 > points_regressed.txt

To examine how the orthogonal LMS misfits vary with angle between 0 and 90
in steps of 0.2 degrees for the same file, try 

   ::

    gmtregress points.txt -A0/90/0.2 -Eo -Nr > points_analysis.txt


References
----------

Draper, N. R., and H. Smith, 1998, *Applied regression analysis*, 3rd ed., 736 pp.,
John Wiley and Sons, New York.

Rousseeuw, P. J., and A. M. Leroy, 1987, *Robust regression and outlier detection*,
329 pp., John Wiley and Sons, New York.

York, D., N. M. Evensen, M. L. Martinez, and J. De Basebe Delgado, 2004, Unified
equations for the slope, intercept, and standard errors of the best straight line,
*Am. J. Phys.*, 72(3), 367-375.

See Also
--------

:doc:`gmt`,
:doc:`trend1d`,
:doc:`trend2d`
