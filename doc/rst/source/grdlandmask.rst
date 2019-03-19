.. index:: ! grdlandmask

***********
grdlandmask
***********

.. only:: not man

    grdlandmask - Create a "wet-dry" mask grid from shoreline data base

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdlandmask** |-G|\ *mask_grd_file*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ *min\_area*\ [/*min\_level*/*max\_level*][\ **+ag**\ \|\ **i**\ \|\ **s** \|\ **S**][\ **+r**\ \|\ **l**][\ **p**\ *percent*] ]
[ |-D|\ *resolution*\ [**+f**] ]
[ |-E|\ [*bordervalues*] ]
[ |-N|\ *maskvalues* ]
[ |-V|\ [*level*] ] [ |SYN_OPT-r| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdlandmask** reads the selected shoreline database and uses that
information to decide which nodes in the specified grid are over land or
over water. The nodes defined by the selected region and lattice spacing
will be set according to one of two criteria: (1) land vs water, or
(2) the more detailed (hierarchical) ocean vs land vs lake
vs island vs pond. The resulting mask may be used in subsequent
operations involving :doc:`grdmath` to mask out data from land [or water] areas. 

Required Arguments
------------------

.. _-G:

**-G**\ *mask_grd_file*]
    Name of resulting output mask grid file. (See GRID FILE FORMATS below). 

.. _-I:

.. include:: explain_-I.rst_

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

Optional Arguments
------------------

.. _-A:

.. |Add_-A| unicode:: 0x20 .. just an invisible code
.. include:: explain_-A.rst_

.. _-D:

**-D**\ *resolution*\ [**+f**]
    Selects the resolution of the data set to use ((**f**)ull,
    (**h**)igh, (**i**)ntermediate, (**l**)ow, or (**c**)rude). The
    resolution drops off by ~80% between data sets. [Default is **l**].
    Append **+f** to automatically select a lower resolution should the
    one requested not be available [abort if not found].
    Alternatively, choose (**a**)uto to automatically select the best
    resolution given the chosen region.  Note that
    because the coastlines differ in details a node in a mask file using
    one resolution is not guaranteed to remain inside [or outside] when
    a different resolution is selected.

.. _-E:

**-E**\ [*bordervalues*]
    Nodes that fall exactly on a polygon boundary should be
    considered to be outside the polygon [Default considers them to be inside].
    Alternatively, append either the four values *cborder/lborder/iborder/pborder*
    or just the single value *bordervalue* (for the case when they should all be the same value).
    This turns on the line-tracking mode.  Now, after setting the mask values
    specified via **-N** we trace the lines and change the node values for all
    cells traversed by a line to the corresponding border value.  Here, *cborder*
    is used for cells traversed by the coastline, *lborder* for cells traversed
    by a lake outline, *iborder* for islands-in-lakes outlines, and *pborder* for
    ponds-in-islands-in-lakes outlines [Default is no line tracing].

.. _-N:

**-N**\ *maskvalues*
    Sets the values that will be assigned to nodes. Values can be any
    number, including the textstring NaN. Also select **-E** to let nodes
    exactly on feature boundaries be considered outside [Default is
    inside]. Specify this information using 1 of 2 formats:

    **-N**\ *wet/dry*.

    **-N**\ *ocean/land/lake/island/pond*.

    [Default is 0/1/0/1/0 (i.e., 0/1)]. 

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_core.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_output.rst_

Notes
-----

A grid produced by grdlandmask is a *categorical* dataset.  As such,
one has to be careful not to interpolate it with standard methods,
such as splines.  However, if you make a map of this grid using
a map projection the grid will be reprojected to yield a rectangular
matrix in the projected coordinates.  This interpolation is done
using splines by default and thus may yield artifacts in your map.
We recommend you use :doc:`grdimage` **-nn** to instead use a nearest
neighbor interpolation for such cases.

Examples
--------

To set all nodes on land to NaN, and nodes over water to 1, using the
high resolution data set, do

   ::

    gmt grdlandmask -R-60/-40/-40/-30 -Dh -I5m -N1/NaN -Gland_mask.nc -V

To make a 1x1 degree global grid with the hierarchical levels of the
nodes based on the low resolution data:

   ::

    gmt grdlandmask -R0/360/-90/90 -Dl -I1 -N0/1/2/3/4 -Glevels.nc -V
 
.. include:: explain_gshhs.rst_

See Also
--------

:doc:`gmt`, :doc:`grdmath`,
:doc:`grdclip`, :doc:`mask`,
:doc:`clip`, :doc:`coast`
