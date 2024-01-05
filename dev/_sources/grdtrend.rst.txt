.. index:: ! grdtrend
.. include:: module_core_purpose.rst_

********
grdtrend
********

|grdtrend_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdtrend** *ingrid* |-N|\ *n_model*\ [**+r**][**+x**\|\ **y**]
[ |-D|\ *diff.nc* ]
[ |SYN_OPT-R| ]
[ |-T|\ *trend.nc* ]
[ |SYN_OPT-V| ]
[ |-W|\ *weight.nc* ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdtrend** reads a 2-D grid file and fits a low-order polynomial trend
to these data by [optionally weighted] least-squares. The trend surface
is defined by:

.. math::
    m_1 + m_2x + m_3y + m_4xy + m_5x^2 + m_6y^2 + m_7x^3 + m_8x^2y + m_9xy^2 + m_{10}y^3.

The user must specify **-N**\ *n_model*, the number of model parameters
to use; thus, **-N**\ *3* fits a bilinear trend, **-N**\ *6* a quadratic
surface, and so on. Optionally, append **+r** to the |-N| option to
perform a robust fit. In this case, the program will iteratively
reweight the data based on a robust scale estimate, in order to converge
to a solution insensitive to outliers. This may be handy when separating
a "regional" field from a "residual" which should have non-zero mean,
such as a local mountain on a regional surface.
Optionally, you may choose to fit a trend that varies only along the *x* or *y* axis,
in which case you select an *n_model* from 1 (constant) to 4 (cubic).

If data file has values set to NaN, these will be ignored during
fitting; if output files are written, these will also have NaN in the
same locations.

Required Arguments
------------------

.. |Add_ingrid| replace:: 2-D gridded data file.
.. include:: explain_grd_inout.rst_
    :start-after: ingrid-syntax-begins
    :end-before: ingrid-syntax-ends

.. _-N:

**-N**\ *n_model*\ [**+r**][**+x**\|\ **y**]
    *n_model* sets the ID of the highest model parameters to fit.
    Append **+r** for robust fit.  As an option, append either **+x** or **+y** to only
    fit a model that depends on *x* or *y* terms, respectively. This means we either fit
    :math:`m_1 + m_2x + m_3x^2 + m_4x^3` or :math:`m_1 + m_2y + m_3y^2 + m_4y^3`.
    Note that *n_model* may only be 1-4 for the one-dimensional fits but may be 1-10
    for the two-dimensional surface fits.

Optional Arguments
------------------

.. _-D:

**-D**\ *diff.nc*
    Write the difference (input data - trend) to the file *diff.nc*.

.. |Add_-R| replace:: Using the |-R| option will select a subsection of the input grid. If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-T:

**-T**\ *trend.nc*
    Write the fitted trend to the file *trend.nc*.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

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
equivalent to the :math:`m_j` in the equation described above. The description
above is to allow the user to match |-N| with the order of the
polynomial surface. See :doc:`grdmath` if you need to evaluate the trend
using the reported coefficients.

Examples
--------

.. include:: explain_example.rst_

To remove a planar trend from the remote grid earth_relief_05m for the region
around Hawaii and write the result to hawaii_residual.nc::

    gmt grdtrend @earth_relief_05m -R180/240/10/40 -N3 -Dhawaii_residual.nc

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
