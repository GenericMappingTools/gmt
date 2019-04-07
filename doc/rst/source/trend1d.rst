.. index:: ! trend1d

*******
trend1d
*******

.. only:: not man

    Fit a [weighted] [robust] polynomial/Fourier model for y = f(x) to xy[w] data

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt trend1d** [ *table* ] |-F|\ **xymrw**\ \|\ **p**\ \|\ **P**\ \|\ **c** |-N|\ *params*
[ |-C|\ *condition_number* ]
[ |-I|\ [*confidence_level*] ]
[ |SYN_OPT-V| ]
[ |-W|\ [**+s**] ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**trend1d** reads x,y [and w] values from the first two [three] columns
on standard input [or *file*] and fits a regression model y = f(x) + e
by [weighted] least squares. The functional form of f(x) may be chosen
as polynomial or Fourier or a mix of the two, and the fit may be made robust by iterative
reweighting of the data. The user may also search for the number of
terms in f(x) which significantly reduce the variance in y. 

Required Arguments
------------------

.. _-F:

**-F**\ **xymrw**\ \|\ **p**\ \|\ **P**\ \|\ **c**
    Specify up to five letters from the set {**x y m r w**\ } in any
    order to create columns of ASCII [or binary] output. **x** = x,
    **y** = y, **m** = model f(x), **r** = residual y - **m**, **w** =
    weight used in fitting. Alternatively, choose just the single
    selection **p** to output a record with the polynomial model coefficients,
    **P** for the normalized polynomial model coefficients, or **c**
    for the normalized Chebyshev model coefficients.

.. _-N:

**-N**\ [**p**\ \|\ **P**\ \|\ **f**\ \|\ **F**\ \|\ **c**\ \|\ **C**\ \|\ **s**\ \|\ **S**\ \|\ **x**\ ]\ *n*\ [,...][**+l**\ *length*][**+o**\ *origin*][**+r**]
    Specify the components of the (possibly mixed) model.  Append
    one or more comma-separated model components.  Each component is
    of the form **T**\ *n*, where **T** indicates the basis function and
    *n* indicates the polynomial degree or how many terms in the Fourier series we want to include.  Choose
    **T** from **p** (polynomial with intercept and powers of x up to degree *n*), **P** (just the
    single term *x^n*), **f** (Fourier series with *n* terms),
    **c** (Cosine series with *n* terms), **s** (sine series with
    *n* terms), **F** (single Fourier component of order *n*),
    **C** (single cosine component of order *n*), and
    **S** (single sine component of order *n*).  By default the
    *x*-origin and fundamental period is set to the mid-point and data
    range, respectively.  Change this using the **+o**\ *origin* and
    **+l**\ *length* modifiers.  We normalize *x* before evaluating
    the basis functions.  Basically, the trigonometric bases all
    use the normalized x' = (2*pi*(x-\ *origin*\ )/*length*) while
    the polynomials use x' = 2*(x-x_mid)/(xmax - xmin) for stability. Finally, append **+r** for a robust
    solution [Default gives a least squares fit].  Use **-V** to see
    a plain-text representation of the y(x) model specified in **-N**.

Optional Arguments
------------------

*table*
    One or more ASCII [or binary, see **-bi**]
    files containing x,y [w] values in the first 2 [3] columns. If no
    files are specified, **trend1d** will read from standard input.

.. _-C:

**-C**\ *condition_number*
    Set the maximum allowed condition number for the matrix solution.
    **trend1d** fits a damped least squares model, retaining only that
    part of the eigenvalue spectrum such that the ratio of the largest
    eigenvalue to the smallest eigenvalue is *condition\_#*. [Default:
    *condition\_#* = 1.0e06. ].

.. _-I:

**-I**\ [*confidence_level*]
    Iteratively increase the number of model parameters, starting at
    one, until *n\_model* is reached or the reduction in variance of the
    model is not significant at the *confidence\_level* level. You may
    set **-I** only, without an attached number; in this case the fit
    will be iterative with a default confidence level of 0.51. Or choose
    your own level between 0 and 1. See remarks section.  Note that the
    model terms are added in the order they were given in **-N** so you
    should place the most important terms first.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**+s**]
    Weights are supplied in input column 3. Do a weighted least squares
    fit [or start with these weights when doing the iterative robust
    fit]. Append **+s** to instead read data uncertainties (one sigma)
    and create weights as 1/sigma^2 [Default reads only the first 2 columns].

.. |Add_-bi| replace:: [Default is 2 (or 3 if **-W** is set) columns]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 1-5 columns as given by **-F**]. 
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

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Remarks
-------

If a polynomial model is included, then the domain of x will be shifted and scaled
to [-1, 1] and the basis functions will be Chebyshev polynomials provided
the polygon is of full order (otherwise we stay with powers of x). The Chebyshev polynomials
have a numerical advantage in the form of the matrix which must be
inverted and allow more accurate solutions. The Chebyshev polynomial of
degree n has n+1 extrema in [-1, 1], at all of which its value is either
-1 or +1. Therefore the magnitude of the polynomial model coefficients
can be directly compared. NOTE: The stable model coefficients are
Chebyshev coefficients. The corresponding polynomial coefficients in a +
bx + cxx + ... are also given in Verbose mode but users must realize
that they are NOT stable beyond degree 7 or 8. See Numerical Recipes for
more discussion. For evaluating Chebyshev polynomials, see :doc:`gmtmath`.

The **-N**\ ...\ **+r** (robust) and **-I** (iterative) options evaluate the
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

    gmt trend1d data.xy -Fxr -Np1 > detrended_data.xy

To make the above linear trend robust with respect to outliers, use:

   ::

    gmt trend1d data.xy -Fxr -Np1+r > detrended_data.xy

To fit the model y(x) = a + bx^2 + c * cos(2*pi*3*(x/l) + d * sin(2*pi*3*(x/l), with l the fundamental period (here l = 15), try:

   ::

    gmt trend1d data.xy -Fxm -NP0,P2,F3+l15 > model.xy

To find out how many terms (up to 20, say in a robust Fourier
interpolant are significant in fitting data.xy, use:

   ::

    gmt trend1d data.xy -Nf20+r -I -V

See Also
--------

:doc:`gmt`,
:doc:`gmtmath`,
:doc:`gmtregress`,
:doc:`grdtrend`,
:doc:`trend2d`

References
----------

Huber, P. J., 1964, Robust estimation of a location parameter, *Ann.
Math. Stat.*, **35**, 73-101.

Menke, W., 1989, Geophysical Data Analysis: Discrete Inverse Theory,
Revised Edition, Academic Press, San Diego.
