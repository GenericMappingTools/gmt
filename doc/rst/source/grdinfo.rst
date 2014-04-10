.. index:: ! grdinfo

*******
grdinfo
*******

.. only:: not man

    grdinfo - Extract information from grids

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**grdinfo** *grdfiles* [ **-C** ] [ **-F** ]
[ **-I**\ [*dx*\ [/*dy*]\|\ **-**\ \|\ **b**] ]
[ **-L**\ [**0**\ \|\ **1**\ \|\ **2**] ] [ **-M** ]
[ |SYN_OPT-R| ]
[ **-T**\ [**s**]\ *dz* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]

|No-spaces|

Description
-----------

**grdinfo** reads a 2-D binary grid file and reports metadata and
various statistics for the (*x*,\ *y*,\ *z*) data in the grid file(s).
The output information contains the minimum/maximum values for *x*, *y*,
and *z*, where the min/max of *z* occur, the *x*- and *y*-increments,
and the number of *x* and *y* nodes, and [optionally] the mean, standard
deviation, and/or the median, L1 scale of *z*, and number of nodes set
to NaN. We also report if the grid is pixel- or gridline-registered and
if it is a Cartesian or Geographic data set (based on metadata in the file). 

Required Arguments
------------------

*grdfile*
    The name of one or several 2-D grid files. (See GRID FILE FORMATS below.)

Optional Arguments
------------------

**-C**
    Formats the report using tab-separated fields on a single line. The
    output is *w e s n z0 z1 dx dy nx ny*\ [ *x0 y0 x1 y1* ] [ *med
    scale* ] [*mean std rms*\ ] [*n\_nan*\ ]. The data in brackets are
    output only if the corresponding options **-M**, **-L1**, **-L2**,
    and **-M** are used, respectively. If the **-I** option is used, the
    output format is instead *NF w e s n z0 z1*, where *NF* is the total
    number of grids read and *w e s n* are rounded off (see **-I**).
**-F**
    Report grid domain and x/y-increments in world mapping format
    [Default is generic]. Does not apply to the **-C** option.
**-I**\ [*dx*\ [/*dy*]\|\ **-**\ \|\ **b**]
    Report the min/max of the region to the nearest multiple of *dx* and
    *dy*, and output this in the form **-R**\ *w/e/s/n* (unless **-C**
    is set). To report the actual grid region, select **-I-**. If no
    argument is given then we report the grid increment in the form
    **-I**\ *xinc/yinc*. If **-Ib** is given we write each grid's
    bounding box polygon instead.
**-L**\ [**0** \| **1** \| **2**]
    **-L0**
        Report range of z after actually scanning the data, not just
        reporting what the header says.
    **-L1**
        Report median and L1 scale of z (L1 scale = 1.4826 \* Median
        Absolute Deviation (MAD)).
    **-L2**
        Report mean, standard deviation, and root-mean-square (rms) of z.
**-M**
    Find and report the location of min/max z-values, and count and
    report the number of nodes set to NaN, if any. 

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of the input grid(s). If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted.
.. include:: explain_-R.rst_

**-T**\ *dz*
    Determine min and max z-value, round off to multiples of *dz*, and
    report as the text string **-T**\ *zmin/zmax/dz* for use by
    **makecpt**. To get a symmetrical range about zero, using the max
    absolute multiple of *dz*, use **-Ts**\ *dz* instead. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout.rst_

Examples
--------

To obtain all the information about the data set in file hawaii\_topo.nc:

   ::

    gmt grdinfo -L1 -L2 -M hawaii_topo.nc

See Also
--------

:doc:`gmt`, :doc:`grd2cpt`,
:doc:`grd2xyz`, :doc:`grdedit`
