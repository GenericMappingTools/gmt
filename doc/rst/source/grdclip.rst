.. index:: ! grdclip

*******
grdclip
*******

.. only:: not man

    Clip the range of grid values

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdclip** *ingrid* |-G|\ *outgrid*
[ |SYN_OPT-R| ]
[ |-S|\ **a**\ *high/above* ]
[ |-S|\ **b**\ *low/below* ] [ |-S|\ **i**\ *low/high/between* ] [ |-S|\ **r**\ *old/new* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdclip** will set values < *low* to *below* and/or values > *high* to
*above*. You can also specify one or more intervals where all values should be
set to IT(between), or replace individual values.  Such operations are useful
when you want all of a continent or an ocean to fall
into one color or gray shade in image processing, when clipping of the
range of data values is required, or for reclassification of data values.
*above*, *below*, *between*, *old* and *new* can be any number or even NaN
(Not a Number). You must choose at least one of the **-S** options. Use
**-R** to only extract a subset of the *ingrid* file. 

Required Arguments
------------------

*ingrid*
    The input 2-D binary grid file.

.. _-G:

**-G**\ *outgrid*
    *outgrid* is the modified output grid file.

Optional Arguments
------------------

.. _-R:

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of *ingrid* grid. If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted.
.. include:: explain_-R.rst_

.. _-S:

**-Sa**\ *high/above*
    Set all data[i] > *high* to *above*.
**-Sb**\ *low/below*
    Set all data[i] < *low* to *below*. 
**-Si**\ *low/high/between*
    Set all data[i] >= *low* and <= *high* to *between*.
    Repeat the option for as many intervals as are needed.
**-Sr**\ *old/new*
    Set all data[i] == *old* to *new*.  This is mostly useful when
    your data are known to be integer values.  Repeat the option
    for as many replacements as are needed.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

Examples
--------

To set all values > 70 to NaN and all values < 0 to 0 in file data.nc:

   ::

    gmt grdclip data.nc -Gnew_data.nc -Sa70/NaN -Sb0/0 -V

To reclassify all values in the 25-30 range to 99, those in 35-39 to 55,
exchange 17 for 11 and all values < 10 to 0 in file classes.nc, try

   ::

    gmt grdclip classes.nc -Gnew_classes.nc -Si25/30/99 -Si35/39/55 -Sr17/11 -Sb10/0 -V

See Also
--------

:doc:`gmt`,
:doc:`grdfill`,
:doc:`grdlandmask`,
:doc:`grdmask`, :doc:`grdmath`,
:doc:`grd2xyz`, :doc:`xyz2grd`
