.. index:: ! rose

******
rose
******

.. only:: not man

    Plot a polar histogram (rose, sector, windrose diagrams)

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt rose** [ *table* ] [ |-A|\ *sector_width*\ [**+r**] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D| ]
[ |-E|\ **m**\ \|\ [**+w**\ ]\ *mode_file* ]
[ |-F| ]
[ |-G|\ *fill* ] [ |-I| ]
[ |-J|\ **X**\ *diameter* ]
[ |-L|\ [\ *wlabel*\ ,\ *elabel*\ ,\ *slabel*\ ,\ *nlabel*\ ] ]
[ |-M|\ *parameters* ]
[ |-Q|\ *alpha* ]
[ |-R|\ *r0*/*r1*/*az_0*/*az_1* ]
[ |-S| ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [**v**]\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ **u**\ \|\ *scale* ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**rose** reads (length, azimuth) pairs from *file* [or standard input]
and plot a windrose diagram.  Add **-i**\ 0 if your file only has azimuth values.
Optionally (with **-A**), polar histograms may be drawn (sector diagram
or rose diagram). Options include full circle and half circle plots. The
outline of the windrose is drawn with the same color as **MAP\_DEFAULT\_PEN**. 

Required Arguments
------------------

None.

Optional Arguments
------------------

.. |Add_intables| replace:: If a file with only
    azimuths are given, use **-i** to indicate the single column with
    azimuths; then all lengths are set to unity (see **-Zu** to set
    actual lengths to unity as well).
.. include:: explain_intables.rst_

.. _-A:

**-A**\ *sector_width*\ [**+r**]
    Gives the sector width in degrees for sector and rose diagram.
    [Default 0 means windrose diagram]. Append **+r** to draw rose
    diagram instead of sector diagram. 

.. _-B:

.. include:: explain_-B.rst_
|
|   Remember that "x" here is
|   radial distance and "y" is azimuth. The ylabel may be used to plot a figure caption.
|   The scale bar length is determined by the radial gridline spacing.

.. _-C:

**-C**\ *cpt*
    Give a CPT. The r-value for each sector is used to
    look-up the sector color.  Cannot be used with a rose diagram.

.. _-D:

**-D**
    Shift sectors so that they are centered on the bin interval (e.g.,
    first sector is centered on 0 degrees).

.. _-E:

**-E**\ **m**\ \|\ [**+w**\ ]\ *mode_file*
    Plot vectors showing the principal directions given in the *mode_file*
    file. Alternatively, specify **-Em** to compute and plot mean direction. See
    **-M** to control the vector attributes.  Finally, to instead save the
    computed mean direction and other statistics, use [**m**\ ]\ **+w**\ *mode_file*.
    The eight items saved to a single record are: 
    *mean_az, mean_r, mean_resultant, max_r, scaled_mean_r, length_sum, n, sign@alpha*,
    where the last term is 0 or 1 depending on whether the mean resultant is significant
    at the level of confidence set via **-Q**.

.. _-F:

**-F**
    Do not draw the scale length bar [Default plots scale in lower right corner]

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Selects shade, color or pattern for filling the sectors [Default is no fill].

.. _-I:

**-I**
    Inquire. Computes statistics needed to specify a useful **-R**. No
    plot is generated.  The following statistics are written to stdout:
    *n*, *mean az*, *mean r*, *mean resultant length*, *max bin sum*,
    *scaled mean*, and *linear length sum*.

.. _-J:

**-JX**\ *diameter*\ [*unit*]
    Sets the diameter of the rose diagram.  Only this form of the projection machinery
    is supported.  If not given we default to 6 inches (or 7.5 cm).

.. _-L:

**-L**\ [\ *wlabel*\ ,\ *elabel*\ ,\ *slabel*\ ,\ *nlabel*]
    Specify labels for the 0, 90, 180, and 270 degree marks. For
    full-circle plot the default is WEST,EAST,SOUTH,NORTH and for
    half-circle the default is 90W,90E,-,0. A - in any entry disables
    that label. Use **-L** with no argument to disable all four labels.
    Note that the GMT_LANGUAGE setting will affect the words used.

.. _-M:

**-M**\ *parameters*
    Used with **-E** to modify vector parameters. For vector heads,
    append vector head *size* [Default is 0, i.e., a line]. See VECTOR
    ATTRIBUTES for specifying additional attributes.  If **-E** is not
    given and the current plot mode is to draw a windrose diagram then
    using **-M** will add vector heads to all individual directions
    using the supplied attributes.

.. _-Q:

**-Q**\ [*alpha*\ ]
    Sets the confidence level used to determine if the mean resultant
    is significant (i.e., Lord Rayleigh test for uniformity) [0.05].
    Note: The critical values are approximated [Berens, 2009] and
    requires at least 10 points; the critical resultants are accurate
    to at least 3 significant digits.  For smaller data sets you
    should consult exact statistical tables.

.. _-R:

**-R**\ *r0*/*r1*/*az\_0*/*az\_1*
    Specifies the 'region' of interest in (r,azimuth) space. r0 is 0, r1
    is max length in units. For azimuth, specify either -90/90 or 0/180
    for half circle plot or 0/360 for full circle.

.. _-S:

**-S**
    Normalize input radii (or bin counts if **-A** is used) by the largest
    value so all radii (or bin counts) range from 0 to 1.

.. _-T:

**-T**
    Specifies that the input data are orientation data (i.e., have a 180 degree
    ambiguity) instead of true 0-360 degree directions [Default]. We
    compensate by counting each record twice: First as *azimuth* and second
    as *azimuth + 180*.  Ignored if range is given as -90/90 or 0/180.

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *pen*
    Set pen attributes for sector outline or rose plot. [Default is no
    outline]. Use **-Wv**\ *pen* to change pen used to draw vector
    (requires **-E**) [Default is same as sector outline]. 

.. _-X:

.. include:: explain_-XY.rst_

.. _-Z:

**-Z**\ **u**\ \|\ *scale*
    Multiply the data radii by *scale*. E.g., use **-Z**\ 0.001 to
    convert your data from m to km. To exclude the radii from
    consideration, set them all to unity with **-Zu** [Default is no
    scaling].

**-:**
    Input file has (azimuth,radius) pairs rather than the expected
    (radius,azimuth). 

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_vectors.rst_

Examples
--------

To plot a half circle rose diagram of the data in the file
fault_segments.az_r (containing pairs of (azimuth, length in meters),
using a 10 degree bin sector width, on a circle of diameter = 6 inch, grid
going out to radius = 150 km in steps of 25 km with a 30 degree sector
interval, radial direction annotated every 50 km, using a light blue
shading outlined by a solid red pen (width = 0.75 points), draw the mean
azimuth, and shown in Portrait orientation, use:

   ::

    gmt rose fault_segments.az_r -R0/150/-90/90 -Bx50g25+l"Fault length"
               -Byg30 -B+t"Rose diagram" -JX6i -A10+r -Glightblue
               -W0.75p,red -Z0.001 -Em -T -: -pdf half_rose

To plot a full circle wind rose diagram of the data in the file
lines.r_az, on a circle of diameter = 10 cm, grid going out to radius =
500 units in steps of 100 with a 45 degree sector interval, using a
solid pen (width = 0.5 point, and shown in landscape [Default]
orientation with a timestamp and command line plotted, use:

   ::

    gmt rose lines.az_r -R0/500/0/360 -JX10c -Bxg100 -Byg45 -B+t"Windrose diagram" -W0.5p -U+c -pdf rose

Redo the same plot but this time add orange vector heads to each direction (with nominal head size
0.5 cm but this will be reduced linearly for lengths less than 1 cm) and save the plot, use:

   ::

    gmt rose lines.az_r -R0/500/0/360 -JX10c -Bxg100 -Byg45 -B+t"Windrose diagram" -M0.5c+e+gorange+n1c -W0.5p -U+c -pdf rose

Bugs
----

No default radial scale and grid settings for polar histograms. User
must run **gmt rose -I** to find max length in binned data set.

References
----------

Berens, P., 2009, CircStat: A MATLAB Toolbox for Circular Statistics, *J. Stat. Software, 31(10)*\ , 1-21.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`, :doc:`histogram`
