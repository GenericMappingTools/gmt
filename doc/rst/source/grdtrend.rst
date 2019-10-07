.. index:: ! grdtrend

********
grdtrend
********

.. only:: not man

    Fit trend surface to grids and compute residuals

`Synopsis`
----------

.. include:: common_SYN_OPTs.rst_

**gmt grdtrend** *grdfile* |-N|\ *n\_model*\ [**+r**]
[ |-D|\ *diff.nc* ]
[ |SYN_OPT-R| ]
[ |-T|\ *trend.nc* ] [ |-W|\ *weight.nc* ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdtrend** reads a 2-D grid file and fits a low-order polynomial trend
to these data by [optionally weighted] least-squares. The trend surface
is defined by:

   m1 + m2\*x + m3\*y + m4\*x\*y + m5\*x\*x + m6\*y\*y + m7\*x\*x\*x +
   m8\*x\*x\*y + m9\*x\*y\*y + m10\*y\*y\*y.

The user must specify **-N**\ *n\_model*, the number of model parameters
to use; thus, **-N**\ *3* fits a bilinear trend, **-N**\ *6* a quadratic
surface, and so on. Optionally, append **+r** to the **-N** option to
perform a robust fit. In this case, the program will iteratively
reweight the data based on a robust scale estimate, in order to converge
to a solution insensitive to outliers. This may be handy when separating
a "regional" field from a "residual" which should have non-zero mean,
such as a local mountain on a regional surface.

If data file has values set to NaN, these will be ignored during
fitting; if output files are written, these will also have NaN in the
same locations. 

Required Arguments
------------------

*grdfile*
    The name of a 2-D binary grid file.

.. _-N:

**-N**\ *n\_model*\ [**+r**\ ]
    *n\_model* sets the number of model parameters to fit.
    Append **+r** for robust fit.

Optional Arguments
------------------

**-D**\ *diff.nc*
    Write the difference (input data - trend) to the file *diff.nc*. 

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of the input grid. If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted.
.. include:: explain_-R.rst_

.. _-T:

**-T**\ *trend.nc*
    Write the fitted trend to the file *trend.nc*. 

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *weight.nc*\ [**+s**]
    If *weight.nc* exists, it will be read and used to solve a weighted
    least-squares problem. [Default: Ordinary least-squares fit]. Append
    **+s** to instead read data uncertainties (one sigma) and create weights
    as 1/sigma^2.  If the robust option has been selected, the weights used
    in the robust fit will be written to *weight.nc*. 

.. include:: explain_help.rst_

Remarks
-------

The domain of x and y will be shifted and scaled to [-1, 1] and the
basis functions are built from Legendre polynomials. These have a
numerical advantage in the form of the matrix which must be inverted and
allow more accurate solutions. NOTE: The model parameters listed with
**-V** are Legendre polynomial coefficients; they are not numerically
equivalent to the m#s in the equation described above. The description
above is to allow the user to match **-N** with the order of the
polynomial surface. See :doc:`grdmath` if you need to evaluate the trend
using the reported coefficients. 

.. include:: explain_grd_inout_short.rst_

Examples
--------

To remove a planar trend from hawaii_topo.nc and write result in hawaii_residual.nc:

   ::

    gmt grdtrend hawaii_topo.nc -N3 -Dhawaii_residual.nc

To do a robust fit of a bicubic surface to hawaii_topo.nc, writing the
result in hawaii_trend.nc and the weights used in hawaii_weight.nc,
and reporting the progress:

   ::

    gmt grdtrend hawaii_topo.nc -N10+r -Thawaii_trend.nc -Whawaii_weight.nc -V

See Also
--------

:doc:`gmt`,
:doc:`grdfft`,
:doc:`grdfilter`,
:doc:`grdmath`
