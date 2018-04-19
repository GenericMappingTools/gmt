.. index:: ! grdsample

*********
grdsample
*********

.. only:: not man

    grdsample - Resample a grid onto a new lattice

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdsample** *in_grdfile* |-G|\ *out_grdfile*
[ |SYN_OPT-I| ]
[ |SYN_OPT-R| ]
[ |-T| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdsample** reads a grid file and interpolates it to create a new grid
file with either: a different registration (**-r** or **-T**); or, a new
grid-spacing or number of nodes (**-I**), and perhaps also a new
sub-region (**-R**). A bicubic [Default], bilinear, B-spline or
nearest-neighbor interpolation is used; see **-n** for settings. Note
that using **-R** only is equivalent to :doc:`grdcut` or :doc:`grdedit` **-S**.
**grdsample** safely creates a fine mesh from a coarse one; the converse
may suffer aliasing unless the data are filtered using :doc:`grdfft` or :doc:`grdfilter`.

When **-R** is omitted, the output grid will cover the same region as
the input grid. When **-I** is omitted, the grid spacing of the output
grid will be the same as the input grid. Either **-r** or **-T** can be
used to change the grid registration. When omitted, the output grid will
have the same registration as the input grid. 

Required Arguments
------------------

*in_grdfile*
    The name of the input 2-D binary grid file. (See GRID FILE FORMAT below.)

.. _-G:

**-G**\ *out_grdfile*
    The name of the output grid file. (See GRID FILE FORMAT below.)

Optional Arguments
------------------

.. _-I:

.. include:: explain_-I.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-T:

**-T**
    Translate between grid and pixel registration; if the input is
    grid-registered, the output will be pixel-registered and vice-versa.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_-n.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_core.rst_

.. include:: explain_help.rst_

.. include:: explain_float.rst_

.. include:: explain_grd_inout_short.rst_

.. include:: explain_grdresample2.rst_

Hints
-----

If an interpolation point is not on a node of the input grid, then a NaN
at any node in the neighborhood surrounding the point will yield an
interpolated NaN. Bicubic interpolation [default] yields continuous
first derivatives but requires a neighborhood of 4 nodes by 4 nodes.
Bilinear interpolation [**-n**] uses only a 2 by 2 neighborhood, but
yields only zero-order continuity. Use bicubic when smoothness is
important. Use bilinear to minimize the propagation of NaNs.

Notes
-----

As an alternative to bicubic spline, linear spline or nearest neighbor interpolation one can
instead send the entire dataset through :doc:`surface` for re-gridding.  This approach allows
more control on aspects such as tension but it also leads to a solution that
is not likely to have fully converged.  The general approach would be
something like

   ::

    gmt grd2xyz old.grd | gmt surface -Rold.grd -Inewinc -Gnew.grd [other options]

For moderate data set one could also achieve an exact solution with :doc:`greenspline`,
such as

::

 gmt grd2xyz old.grd | gmt greenspline -Rold.grd -Inewinc -Gnew.grd [other options]

Examples
--------

To resample the 5 x 5 minute grid in hawaii_5by5_topo.nc onto a 1 minute grid:

   ::

    gmt grdsample hawaii_5by5_topo.nc -I1m -Ghawaii_1by1_topo.nc

To translate the gridline-registered file surface.nc to pixel
registration while keeping the same region and grid interval:

   ::

    gmt grdsample surface.nc -T -Gpixel.nc

See Also
--------

:doc:`gmt`,
:doc:`grdedit`,
:doc:`grdfft`,
:doc:`grdfilter`,
:doc:`greenspline`,
:doc:`surface`
