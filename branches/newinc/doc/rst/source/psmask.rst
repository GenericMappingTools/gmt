.. index:: ! psmask

******
psmask
******

.. only:: not man

    psmask - Use data tables to clip or mask map areas with no coverage

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psmask** [ *table* ]
|SYN_OPT-I|
**-J**\ *parameters*
|SYN_OPT-R|
[ |SYN_OPT-B| ]
[ **-D**\ *dumpfile* ]
[ **-G**\ *fill* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ]
[ **-N** ] [ **-O** ] [ **-P** ] [ **-Q**\ *cut* ]
[ **-S**\ *search\_radius*\ [*unit*] ] [ **-T** ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ **-r** ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]

**psmask** **-C** [ **-K** ] [ **-O** ]

|No-spaces|

Description
-----------

**psmask** reads a (*x*,\ *y*,\ *z*) file [or standard input] and uses
this information to find out which grid cells are reliable. Only grid
cells which have one or more data points are considered reliable. As an
option, you may specify a radius of influence. Then, all grid cells that
are within *radius* of a data point are considered reliable.
Furthermore, an option is provided to reverse the sense of the test.
Having found the reliable/not reliable points, **psmask** will either
paint tiles to mask these nodes (with the **-T** switch), or use
contouring to create polygons that will clip out regions of no interest.
When clipping is initiated, it will stay in effect until turned off by a
second call to **psmask** using the **-C** option. 

Required
--------

.. include:: explain_-I.rst_

.. include:: explain_-J.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. include:: explain_-B.rst_

**-C**
    Mark end of existing clip path. No input file is needed. Implicitly
    sets **-O**. Also supply **-X** and **-Y** settings if you have
    moved since the clip started.
**-D**\ *dumpfile*
    Dump the (x,y) coordinates of each clipping polygon to one or more
    output files (or *stdout* if *template* is not given). No plotting
    will take place. If *template* contains the C-format specifier %d
    (including modifications like %05d) then polygons will be written to
    different files; otherwise all polygons are written to the specified
    file (*template*). The files are ASCII unless
    **-bo** is used. See **-Q** to exclude small
    polygons from consideration. 

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

**-G**\ *fill*
    Paint the clip polygons (or tiles) with a selected fill [Default is no fill]. 

.. include:: explain_-Jz.rst_

.. include:: explain_-K.rst_

**-N**
    Invert the sense of the test, i.e., clip regions where there is data coverage. 

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-Q**
    Do not dump polygons with less than *cut* number of points [Dumps
    all polygons]. Only applicable if **-D** has been specified.
**-S**\ *search\_radius*\ [*unit*\ ]
    Sets radius of influence. Grid nodes within *radius* of a data point
    are considered reliable. [Default is 0, which means that only grid
    cells with data in them are reliable]. Append the distance unit (see
    UNITS).
**-T**
    Plot tiles instead of clip polygons. Use **-G** to set tile color or
    pattern. Cannot be used with **-D**. 

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-XY.rst_

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. include:: explain_-c.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: explain_-h.rst_
    
.. include:: explain_-icols.rst_

.. include:: explain_perspective.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_-t.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

Examples
--------

To make an overlay PostScript file that will mask out the regions of a
contour map where there is no control data using clip polygons, use:

   ::

    gmt psmask africa_grav.xyg -R20/40/20/40 -I5m -JM10i -O -K > mask.ps

We do it again, but this time we wish to save the clipping polygons to
file all_pols.txt:

   ::

    gmt psmask africa_grav.xyg -R20/40/20/40 -I5m -Dall_pols.txt

A repeat of the first example but this time we use white tiling:

   ::

    gmt psmask africa_grav.xyg -R20/40/20/40 -I5m -JM10i -T -O -K -Gwhite > mask.ps

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`grdmask`, :doc:`surface`,
:doc:`psbasemap`, :doc:`psclip`
