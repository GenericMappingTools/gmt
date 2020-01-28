.. index:: ! grdvector
.. include:: module_core_purpose.rst_

*********
grdvector
*********

|grdvector_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdvector** *compx.nc* *compy.nc* **-J**\ *parameters* [ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-I|\ [**x**]\ *dx*\ [/*dy*] ]
[ |-N| ] [ |-Q|\ *parameters* ]
[ |SYN_OPT-R| ]
[ |-S|\ [**i**\|\ **l**]\ *scale*\ [*unit*] ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: grdvector_common.rst_

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To draw the vector field given by the files r.nc and theta.nc on a
linear plot with scale 5 cm per data unit, using vector rather than
stick plot, scale vector magnitudes so that 10 units equal 1 inch, and
center vectors on the node locations, run

   ::

    gmt grdvector r.nc theta.nc -Jx5c -A -Q0.1i+e+jc -S10i -pdf gradient

To plot a geographic data sets given the files comp_x.nc and comp_y.nc,
using a length scale of 200 km per data unit and only plot every 3rd node in either direction, try

   ::

    gmt grdvector comp_x.nc comp_y.nc -Ix3 -JH0/20c -Q0.1i+e+jc -S200k -pdf globe

.. include:: grdvector_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`grdcontour`, :doc:`plot`
