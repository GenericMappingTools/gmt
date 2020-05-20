.. index:: ! grdinfo
.. include:: module_core_purpose.rst_

*******
grdinfo
*******

|grdinfo_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdinfo** *grdfiles* [ |-C|\ [**n**\|\ **t**\] ]
[ |-D|\ [*xoff*\ [/*yoff*]][**+i**] ]
[ |-E|\ [**x**\|\ **y**][**+h**\|\ **H**\|\ **l**\|\ **L**] ]
[ |-F| ]
[ |-I|\ [*dx*\ [/*dy*]\|\ **b**\|\ **i**\|\ **r**] ]
[ |-L|\ [**0**\|\ **1**\|\ **2**\|\ **p**\|\ **a**] ] [ |-M| ]
[ |SYN_OPT-R| ]
[ |-T|\ [*dz*]\ [**+a**\ [*alpha*]]\ [**+s**] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdinfo** reads a 2-D binary grid file and reports metadata and
various statistics for the (*x*,\ *y*,\ *z*) data in the grid file(s).
The output information contains the minimum/maximum values for *x*, *y*,
and *z*, where the min/max of *z* occur, the *x*- and *y*-increments,
and the number of *x* and *y* nodes, and [optionally] the mean, standard
deviation, and/or the median, median absolute deviation (MAD) of *z*, and/or
the mode (Least Median of Squares; LMS), LMS scale of *z*, and number of nodes set
to NaN. We also report if the grid is pixel- or gridline-registered and
if it is a Cartesian or Geographic data set (based on metadata in the file).

Required Arguments
------------------

*grdfile*
    The name of one or several 2-D grid files. (See GRID FILE FORMATS below.)

Optional Arguments
------------------

.. _-C:

**-C**\ [**n**\|\ **t**\]
    Formats the report using tab-separated fields on a single line. The
    output is *name w e s n z0 z1 dx dy nx ny*\ [ *x0 y0 x1 y1* ] [ *med
    scale* ] [*mean std rms*] [*n\_nan*] *registration*. The data in brackets are
    output only if the corresponding options **-M**, **-L1**, **-L2**,
    and **-M** are used, respectively. Use **-Ct** to place file *name*
    at the end of the output record or **-Cn** to only output numerical
    columns.  The *registration* is either 0 (gridline) or 1 (pixel).
    If the **-I** option is used, the
    output format is instead *NF w e s n z0 z1*, where *NF* is the total
    number of grids read and *w e s n* are rounded off (see **-I**).

.. _-D:

**-D**\ [*xoff*\ [/*yoff*]][**+i**]
    Divide a single grid's domain (or the **-R** domain, if no grid given)
    into tiles of size *dx* times *dy* (set via **-I**).  You can specify
    overlap between tiles by appending *xoff*\ [/*yoff*].  If the single
    grid is given you may use the modifier **+i** to ignore tiles that
    have no data within each tile subregion.  Default output is text
    region strings.  Use **-C** to instead report four columns with
    *xmin xmax ymin ymax* per tile, or use **-Ct** to also have the
    region string appended as trailing text.

.. _-E:

**-E**\ [**x**\|\ **y**][**+h**\|\ **H**\|\ **l**\|\ **L**]
    Report the extreme values found on a per column (**-Ex**) or per
    row (**-Ey**) basis.  By default, we look for the global maxima (**+h**\|\ **H**)
    for each column.  Append **+l**\|\ **L** to look for minima instead.
    Upper case **+L** means we find the minimum of the positive values only, while
    upper case **+U** means we find the maximum of the negative values only [use all values].
    We only allow one input grid when **-E** is selected.

.. _-F:

**-F**
    Report grid domain and x/y-increments in world mapping format
    [Default is generic]. Does not apply to the **-C** option.

.. _-I:

**-I**\ [*dx*\ [/*dy*]\|\ **b**\|\ **i**\|\ **r**]
    Report the min/max of the region to the nearest multiple of *dx* and
    *dy*, and output this in the form **-R**\ *w/e/s/n* (unless **-C**
    is set). To report the actual grid region, select **-Ir**. For a
    grid produced by the img supplement (a Cartesian Mercator grid),
    the exact geographic region is given with **-Ii** (if not found
    then we return the actual grid region instead).  If no
    argument is given then we report the grid increment in the form
    **-I**\ *xinc*\ [/*yinc*]. If **-Ib** is given we write each grid's
    bounding box polygon instead.  Finally, if **-D** is in effect then
    *dx* and *dy* are the dimensions of the desired tiles.

.. _-L:

**-L**\ [**0**\|\ **1**\|\ **2**\|\ **p**\|\ **a**]
    **-L0**
        Report range of z after actually scanning the data, not just
        reporting what the header says.
    **-L1**
        Report median and L1 scale of *z* (L1 scale = 1.4826 \* Median
        Absolute Deviation (MAD)).
    **-L2**
        Report mean, standard deviation, and root-mean-square (rms) of *z*.
    **-Lp**
        Report mode (LMS) and LMS scale of *z*.
    **-La**
        All of the above.

    **Note**: If the grid is geographic then each node represents a physical
    area that decreases with increasing latitude.  We therefore report
    spherically weighted statistical estimates for such grids.

.. _-M:

**-M**
    Find and report the location of min/max z-values, and count and
    report the number of nodes set to NaN, if any.

.. _-R:

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of the input grid(s). If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted.
.. include:: explain_-R.rst_

.. _-T:

**-T**\ [*dz*]\ [**+a**\ [*alpha*]]\ [**+s**]
    Determine min and max z-value.  If *dz* is provided then we first round these
    values off to multiples of *dz*. To exclude the two tails of the distribution
    when determining the min and max you can add **+a** to set the *alpha*
    value (in percent [2]): We then sort the grid, exclude the data in the
    0.5*\ *alpha* and 100 - 0.5*\ *alpha* tails, and revise the min and max.
    To force a symmetrical range about zero, using minus/plus the max
    absolute value of the two extremes, append **+s**. We report the
    result via the text string **-T**\ *zmin/zmax* or **-T**\ *zmin/zmax/dz*
    (if *dz* was given) as expected by :doc:`makecpt`.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

Examples
--------

To obtain all the information about the remote data set in file earth_relief_10m::

    gmt grdinfo -L1 -L2 -M @earth_relief_10m

Get the grid spacing in earth_relief_10m::

    dx=`gmt grdinfo -Cn -o7 @earth_relief_10m`

See Also
--------

:doc:`gmt`, :doc:`grd2cpt`,
:doc:`grd2xyz`, :doc:`grdedit`
