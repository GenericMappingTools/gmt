*******
grdclip
*******

grdclip - Clip the range of grids

Synopsis
--------

**grdclip** *ingrid* **-G**\ *outgrid* [
**-R**\ *west*/*east*/*south*/*north*\ [**r**] ] [ **-Sa**\ *high/above* ]
[ **-Sb**\ *low/below* ] [ **-Si**\ *low/high/between* ] [ **-V**\ [*level*] ]

Description
-----------

**grdclip** will set values < *low* to *below* and/or values > *high* to
*above*. Useful when you want all of a continent or an ocean to fall
into one color or gray shade in image processing, or clipping of the
range of data values is required. *above/below* can be any number or NaN
(Not a Number). You must choose at least one of **-Sa** or **-Sb**. Use
**-R** to only extract a subset of the *ingrid* file. 

.. include:: explain_commonitems.rst_

Required Arguments
------------------

*ingrid*
    The input 2-D binary grid file.
**-G**\ *outgrid*
    *outgrid* is the modified output grid file.

Optional Arguments
------------------

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of *ingrid* grid. If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted.
.. include:: explain_-R.rst_

**-Sa**\ *high/above*
    Set all data[i] > *high* to *above*.
**-Sb**\ *low/below*
    Set all data[i] < *low* to *below*. 
**-Si**\ *low/high/below*
    Set all data[i] >= *low* and <= *high* to *between*. 

.. |Add_-V| unicode:: 0x0C .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout.rst_

Examples
--------

To set all values > 70 to NaN and all values < 0 to 0 in file data.nc:

grdclip data.nc -Gnew\_data.nc -Sa70/NaN -Sb0/0 -V

See Also
--------

`gmt <gmt.html>`_ , `grdlandmask <grdlandmask.html>`_ ,
`grdmask <grdmask.html>`_ , `grdmath <grdmath.html>`_ ,
`grd2xyz <grd2xyz.html>`_ , `xyz2grd <xyz2grd.html>`_
