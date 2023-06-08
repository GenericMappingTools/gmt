.. index:: ! mask
.. include:: module_core_purpose.rst_

******
mask
******

|mask_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt mask** [ *table* ]
|SYN_OPT-I|
|-J|\ *parameters*
|SYN_OPT-R|
[ |SYN_OPT-B| ]
[ |-C| ]
[ |-D|\ *dumpfile* ]
[ |-F|\ [**l**\|\ **r**] ]
[ |-G|\ *fill* ]
[ |-Jz|\ \|\ **Z**\ *parameters* ]
[ |-L|\ *nodegrid*\ [**+i**\|\ **o**] ]
[ |-N| ]
[ |-Q|\ *cut* ]
[ |-S|\ *search\_radius* ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads a (*x*,\ *y*,\ *z*) file [or standard input] and uses
this information to find out which grid cells are reliable. Only grid
cells which have one or more data points are considered reliable. As an
option, you may specify a radius of influence. Then, all grid cells that
are within *radius* of a data point are considered reliable.
Furthermore, an option is provided to reverse the sense of the test.
Having found the reliable/not reliable points, the module will either
paint tiles to mask these nodes (with the |-T| switch), or use
contouring to create polygons that will clip out regions of no interest.
When clipping is initiated, it will stay in effect until turned off by a
second call to the module using the |-C| option.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-I:

.. include:: explain_-I.rst_

.. |Add_-J| replace:: [Not mandatory with |-D|]. |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-R| replace:: |Add_-R_auto_table|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**
    Mark end of existing clip path. No input file is needed. Implicitly
    sets |-O|. Also supply |-X| and |-Y| settings if you have
    moved since the clip started.

.. _-D:

**-D**\ *dumpfile*
    Dump the (x,y) coordinates of each clipping polygon to one or more
    output files (or standard output if *template* is not given). No plotting
    will take place. If *template* contains the C-format specifier %d
    (including modifications like %05d) then polygons will be written to
    different files; otherwise all polygons are written to the specified
    file (*template*). The files are ASCII unless
    **-bo** is used. See |-Q| to exclude small
    polygons from consideration.

.. _-F:

**-F**\ [**l**\|\ **r**]
    Force clip contours (polygons) to be oriented so that data points are to the
    left (**-Fl** [Default]) or right (**-Fr**) as we move along the perimeter
    [Default is arbitrary orientation]. Requires |-D|.

.. _-G:

**-G**\ *fill*
    Paint the clip polygons (or tiles) with a selected fill [Default is no fill].

.. _-Jz:

.. include:: explain_-Jz.rst_

.. _-L:

**-L**\ *nodegrid*\ [**+i**\|\ **o**]
    Save the internal grid with ones (data constraint) and zeros (no data) to
    the named *nodegrid* [no grid saved].  Append **+o** to convert the outside (no data)
    nodes to NaNs before writing the grid, while appending **+i** will instead convert
    the inside (data) flags to NaNs.

.. _-N:

**-N**
    Invert the sense of the test, i.e., clip regions where there is data coverage.

.. _-Q:

**-Q**\ *cut*
    Do not dump polygons with less than *cut* number of points [Dumps
    all polygons]. Only applicable if |-D| has been specified.

.. _-S:

**-S**\ *search\_radius*
    Sets radius of influence. Grid nodes within *radius* of a data point
    are considered reliable. [Default is 0, which means that only grid
    cells with data in them are reliable]. Append the distance unit (see `Units`_).

.. _-T:

**-T**
    Plot tiles instead of clip polygons. Use |-G| to set tile color or
    pattern. Cannot be used with |-D|.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-qi.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_-t.rst_

.. include:: explain_-w.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. module_common_ends

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To make a mask that has a 5 degree radius around points spaced every 10 degree
along the prime meridian, and just paint those areas yellow we try::

    gmt begin mask
      gmt math -T-90/90/10 -N2/1 0 = | gmt mask -Gyellow -I30m -R-75/75/-90/90 -JQ0/7i \
        -S5d -T -Bafg10 -BWSne+t"Mask for points with r = 5 degrees"
    gmt end show

To make an overlay that will mask out the regions of a
contour map where there is no control data using clip polygons, use:

::

  gmt mask africa_grav.xyg -R20/40/20/40 -I5m -JM10i -pdf mask

We do it again, but this time we wish to save the clipping polygons to
file all_pols.txt:

::

  gmt mask africa_grav.xyg -R20/40/20/40 -I5m -Dall_pols.txt

A repeat of the first example but this time we use white tiling:

::

  gmt mask africa_grav.xyg -R20/40/20/40 -I5m -JM10i -T -Gwhite -pdf mask

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`grdmask`, :doc:`surface`,
:doc:`basemap`, :doc:`clip`
