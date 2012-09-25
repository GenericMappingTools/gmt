*******
trend2d
*******

trend2d - Fit a [weighted] [robust] polynomial model for z = f(x,y) to
xyz[w] data

`Synopsis <#toc1>`_
-------------------

**trend2d** [ *table* ] **-F**\ **xyzmrw** **-N**\ *n\_model*\ [**r**\ ]
[ *xyz[w]file* ] [ **-C**\ *condition\_number* ] [
**-I**\ [*confidence\_level*\ ] ] [ **-V**\ [*level*\ ] ] [ **-W** ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**trend2d** reads x,y,z [and w] values from the first three [four]
columns on standard input [or *xyz[w]file*] and fits a regression model
z = f(x,y) + e by [weighted] least squares. The fit may be made robust
by iterative reweighting of the data. The user may also search for the
number of terms in f(x,y) which significantly reduce the variance in z.
n\_model may be in [1,10] to fit a model of the following form (similar
to grdtrend):

m1 + m2\*x + m3\*y + m4\*x\*y + m5\*x\*x + m6\*y\*y + m7\*x\*x\*x +
m8\*x\*x\*y + m9\*x\*y\*y + m10\*y\*y\*y.

The user must specify **-N**\ *n\_model*, the number of model parameters
to use; thus, **-N**\ *4* fits a bilinear trend, **-N**\ *6* a quadratic
surface, and so on. Optionally, append **r** to perform a robust fit. In
this case, the program will iteratively reweight the data based on a
robust scale estimate, in order to converge to a solution insensitive to
outliers. This may be handy when separating a "regional" field from a
"residual" which should have non-zero mean, such as a local mountain on
a regional surface.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-F**\ **xyzmrw**
    Specify up to six letters from the set {**x y z m r w**\ } in any
    order to create columns of ASCII [or binary] output. **x** = x,
    **y** = y, **z** = z, **m** = model f(x,y), **r** = residual z -
    **m**, **w** = weight used in fitting.
**-N**\ *n\_model*\ [**r**\ ]
    Specify the number of terms in the model, *n\_model*, and append
    **r** to do a robust fit. E.g., a robust bilinear model is
    **-N**\ *4*\ **r**.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII [or binary, see **-bi**\ [*ncols*\ ][*type*\ ]]
    files containing x,y,z [w] values in the first 3 [4] columns. If no
    files are specified, **trend2d** will read from standard input.
**-C**\ *condition\_number*
    Set the maximum allowed condition number for the matrix solution.
    **trend2d** fits a damped least squares model, retaining only that
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
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**
    Weights are supplied in input column 4. Do a weighted least squares
    fit [or start with these weights when doing the iterative robust
    fit]. [Default reads only the first 3 columns.]
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 3 (or 4 if **-W** is set) input
    columns].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is 1-6 columns as set by **-F**].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Remarks <#toc6>`_
------------------

The domain of x and y will be shifted and scaled to [-1, 1] and the
basis functions are built from Chebyshev polynomials. These have a
numerical advantage in the form of the matrix which must be inverted and
allow more accurate solutions. In many applications of **trend2d** the
user has data located approximately along a line in the x,y plane which
makes an angle with the x axis (such as data collected along a road or
ship track). In this case the accuracy could be improved by a rotation
of the x,y axes. **trend2d** does not search for such a rotation;
instead, it may find that the matrix problem has deficient rank.
However, the solution is computed using the generalized inverse and
should still work out OK. The user should check the results graphically
if **trend2d** shows deficient rank. NOTE: The model parameters listed
with **-V** are Chebyshev coefficients; they are not numerically
equivalent to the m#s in the equation described above. The description
above is to allow the user to match **-N** with the order of the
polynomial surface. For evaluating Chebyshev polynomials, see
**grdmath**.

The **-N**\ *n\_model*\ **r** (robust) and **-I** (iterative) options
evaluate the significance of the improvement in model misfit Chi-Squared
by an F test. The default confidence limit is set at 0.51; it can be
changed with the **-I** option. The user may be surprised to find that
in most cases the reduction in variance achieved by increasing the
number of terms in a model is not significant at a very high degree of
confidence. For example, with 120 degrees of freedom, Chi-Squared must
decrease by 26% or more to be significant at the 95% confidence level.
If you want to keep iterating as long as Chi-Squared is decreasing, set
*confidence\_level* to zero.

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

`Ascii Format Precision <#toc7>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Examples <#toc8>`_
-------------------

To remove a planar trend from data.xyz by ordinary least squares, use:

trend2d data.xyz -Fxyr -N2 > detrended\_data.xyz

To make the above planar trend robust with respect to outliers, use:

trend2d data.xzy -Fxyr -N2r > detrended\_data.xyz

To find out how many terms (up to 10 in a robust interpolant are
significant in fitting data.xyz, use:

trend2d data.xyz -N10r -I -V

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdmath*\ (1) <grdmath.html>`_ ,
`*grdtrend*\ (1) <grdtrend.html>`_ , `*trend1d*\ (1) <trend1d.html>`_

`References <#toc10>`_
----------------------

Huber, P. J., 1964, Robust estimation of a location parameter, *Ann.
Math. Stat.*, **35**, 73-101.

Menke, W., 1989, Geophysical Data Analysis: Discrete Inverse Theory,
Revised Edition, Academic Press, San Diego.
