.. index:: ! trend1d

*******
trend1d
*******

.. only:: not man

    trend1d - Fit a [weighted] [robust] polynomial [or Fourier] model for y = f(x) to xy[w] data

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**trend1d** [ *table* ] **-F**\ **xymrw\|p**
**-N**\ [**f**]\ *n\_model*\ [**r**] [ *xy[w]file* ]
[ **-C**\ *condition\_number* ] [ **-I**\ [*confidence\_level*] ]
[ |SYN_OPT-V| ] [ **-W** ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**trend1d** reads x,y [and w] values from the first two [three] columns
on standard input [or *file*] and fits a regression model y = f(x) + e
by [weighted] least squares. The functional form of f(x) may be chosen
as polynomial or Fourier, and the fit may be made robust by iterative
reweighting of the data. The user may also search for the number of
terms in f(x) which significantly reduce the variance in y. 

Required Arguments
------------------

**-F**\ **xymrw\|p**
    Specify up to five letters from the set {**x y m r w**\ } in any
    order to create columns of ASCII [or binary] output. **x** = x,
    **y** = y, **m** = model f(x), **r** = residual y - **m**, **w** =
    weight used in fitting. Alternatively choose **-F**\ **p** (i.e., no
    other of the 5 letters) to output only the model coefficients
    (Polynomial).
**-N**\ [**f**\ ]\ *n\_model*\ [**r**\ ]
    Specify the number of terms in the model, *n\_model*, whether to fit
    a Fourier (**-Nf**) or polynomial [Default] model, and append **r**
    to do a robust fit. E.g., a robust quadratic model is
    **-N**\ *3*\ **r**.

Optional Arguments
------------------

*table*
    One or more ASCII [or binary, see **-bi**]
    files containing x,y [w] values in the first 2 [3] columns. If no
    files are specified, **trend1d** will read from standard input.
**-C**\ *condition\_number*
    Set the maximum allowed condition number for the matrix solution.
    **trend1d** fits a damped least squares model, retaining only that
    part of the eigenvalue spectrum such that the ratio of the largest
    eigenvalue to the smallest eigenvalue is *condition\_#*. [Default:
    *condition\_#* = 1.0e06. ].
**-I**\ [*confidence\_level*\ ]
    Iteratively increase the number of model parameters, starting at
    one, until *n\_model* is reached or the reduction in variance of the
    model is not significant at the *confidence\_level* level. You may
    set **-I** only, without an attached number; in this case the fit
    will be iterative with a default confidence level of 0.51. Or choose
    your own level between 0 and 1. See remarks section. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-W**
    Weights are supplied in input column 3. Do a weighted least squares
    fit [or start with these weights when doing the iterative robust
    fit]. [Default reads only the first 2 columns.] 

.. |Add_-bi| replace:: [Default is 2 (or 3 if **-W** is set) columns]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 1-5 columns as given by **-F**]. 
.. include:: explain_-bo.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Remarks
-------

If a Fourier model is selected, the domain of x will be shifted and
scaled to [-pi, pi] and the basis functions used will be 1, cos(x),
sin(x), cos(2x), sin(2x), ... If a
polynomial model is selected, the domain of x will be shifted and scaled
to [-1, 1] and the basis functions will be Chebyshev polynomials. These
have a numerical advantage in the form of the matrix which must be
inverted and allow more accurate solutions. The Chebyshev polynomial of
degree n has n+1 extrema in [-1, 1], at all of which its value is either
-1 or +1. Therefore the magnitude of the polynomial model coefficients
can be directly compared. NOTE: The stable model coefficients are
Chebyshev coefficients. The corresponding polynomial coefficients in a +
bx + cxx + ... are also given in Verbose mode but users must realize
that they are NOT stable beyond degree 7 or 8. See Numerical Recipes for
more discussion. For evaluating Chebyshev polynomials, see **gmtmath**.

The **-Nr** (robust) and **-I** (iterative) options evaluate the
significance of the improvement in model misfit Chi-Squared by an F
test. The default confidence limit is set at 0.51; it can be changed
with the **-I** option. The user may be surprised to find that in most
cases the reduction in variance achieved by increasing the number of
terms in a model is not significant at a very high degree of confidence.
For example, with 120 degrees of freedom, Chi-Squared must decrease by
26% or more to be significant at the 95% confidence level. If you want
to keep iterating as long as Chi-Squared is decreasing, set
*confidence_level* to zero.

A low confidence limit (such as the default value of 0.51) is needed to
make the robust method work. This method iteratively reweights the data
to reduce the influence of outliers. The weight is based on the Median
Absolute Deviation and a formula from Huber [1964], and is 95% efficient
when the model residuals have an outlier-free normal distribution. This
means that the influence of outliers is reduced only slightly at each
iteration; consequently the reduction in Chi-Squared is not very
significant. If the procedure needs a few iterations to successfully
attenuate their effect, the significance level of the F test must be
kept low.

Examples
--------

To remove a linear trend from data.xy by ordinary least squares, use:

   ::

    gmt trend1d data.xy -Fxr -N2 > detrended_data.xy

To make the above linear trend robust with respect to outliers, use:

   ::

    gmt trend1d data.xy -Fxr -N2r > detrended_data.xy

To find out how many terms (up to 20, say in a robust Fourier
interpolant are significant in fitting data.xy, use:

   ::

    gmt trend1d data.xy -Nf20r -I -V

See Also
--------

:doc:`gmt`,
:doc:`gmtmath`,
:doc:`grdtrend`,
:doc:`trend2d`

References
----------

Huber, P. J., 1964, Robust estimation of a location parameter, *Ann.
Math. Stat.*, **35**, 73-101.

Menke, W., 1989, Geophysical Data Analysis: Discrete Inverse Theory,
Revised Edition, Academic Press, San Diego.
