.. index:: ! rose
.. include:: module_core_purpose.rst_

******
rose
******

|rose_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt rose** [ *table* ]
[ |-A|\ *sector_width*\ [**+r**] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D| ]
[ |-E|\ **m**\|\ [**+w**]\ *mode_file* ]
[ |-F| ]
[ |-G|\ *fill* ] [ |-I| ]
[ |-J|\ **X**\ *diameter* ]
[ |-L|\ [*wlabel*\ ,\ *elabel*\ ,\ *slabel*\ ,\ *nlabel*] ]
[ |-M|\ *parameters* ]
[ |-N|\ *mode*\ [**+p**\ *pen*] ]
[ |-Q|\ *alpha* ]
[ |-R|\ *r0*/*r1*/*az0*/*az1* ]
[ |-S|\ [**+a**] ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [**v**]\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ **u**\|\ *scale* ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

**rose** reads (length, azimuth) pairs from *file* [or standard input]
and plot a windrose diagram.  Add **-i**\ 0 if your file only has azimuth values.
Optionally (with |-A|), polar histograms may be drawn (sector diagram
or rose diagram). Options include full circle and half circle plots. The
outline of the windrose is drawn with the same color as :term:`MAP_DEFAULT_PEN`.

Required Arguments
------------------

.. |Add_intables| replace:: If a file with only
    azimuths are given, use **-i** to indicate the single column with
    azimuths; then all lengths are set to unity (see **-Zu** to set
    actual lengths to unity as well).
.. include:: explain_intables.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ *sector_width*\ [**+r**]
    Gives the sector width in degrees for sector and rose diagram.
    [Default 0 means windrose diagram]. Append **+r** to draw rose
    diagram instead of sector diagram.

.. |Add_-B| replace:: |Add_-B_links| Remember that "x" here is radial distance
   and "y" is azimuth. The y label may be used to plot a figure caption. The
   scale bar length is determined by the radial gridline spacing.
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *cpt*
    Give a CPT. The *r*-value for each sector is used to
    look-up the sector color.  Cannot be used with a rose diagram.
    If modern mode and no argument is given then we select the current CPT.

.. _-D:

**-D**
    Shift sectors so that they are centered on the bin interval (e.g.,
    first sector is centered on 0 degrees).

.. _-E:

**-E**\ **m**\|\ [**+w**]\ *mode_file*
    Plot vectors showing the principal directions given in the *mode_file*
    file. Alternatively, specify **-Em** to compute and plot mean direction. See
    |-M| to control the vector attributes.  Finally, to instead save the
    computed mean direction and other statistics, use [**m**]\ **+w**\ *mode_file*.
    The eight items saved to a single record are:
    *mean_az, mean_r, mean_resultant, max_r, scaled_mean_r, length_sum, n, sign@alpha*,
    where the last term is 0 or 1 depending on whether the mean resultant is significant
    at the level of confidence set via |-Q|.

.. _-F:

**-F**
    Do *not* draw the scale length bar [Default plots scale bar in lower right corner
    provided |-B| is used. We use :term:`MAP_TICK_PEN_PRIMARY` to draw the scale and
    label it with :term:`FONT_ANNOT_PRIMARY`].

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Selects shade, color or pattern for filling the sectors [Default is no fill].

.. _-I:

**-I**
    Inquire. Computes statistics needed to specify a useful |-R|. No
    plot is generated.  The following statistics are written to standard output:
    *n*, *mean az*, *mean r*, *mean resultant length*, *max bin sum*,
    *scaled mean*, and *linear length sum*. **Note**: You may use **-o**
    to select a subset from this record.

.. _-J:

**-JX**\ *diameter*
    Sets the diameter of the rose diagram.  Only this form of the projection machinery
    is supported for this module.  If not given, then we default to a diameter of 7.5 cm.

.. _-L:

**-L**\ [*wlabel*\ ,\ *elabel*\ ,\ *slabel*\ ,\ *nlabel*]
    Specify labels for the 0, 90, 180, and 270 degree marks. For
    full-circle plot the default is WEST,EAST,SOUTH,NORTH and for
    half-circle the default is 90W,90E,-,0. A - in any entry disables
    that label. Use |-L| with no argument to disable all four labels.
    Note that the :term:`GMT_LANGUAGE` setting will affect the words used.

.. _-M:

**-M**\ *parameters*
    Used with |-E| to modify vector parameters. For vector heads,
    append vector head *size* [Default is 0, i.e., a line]. See VECTOR
    ATTRIBUTES for specifying additional attributes.  If |-E| is not
    given and the current plot mode is to draw a windrose diagram then
    using |-M| will add vector heads to all individual directions
    using the supplied attributes.

.. _-N:

**-N**\ *mode*\ [**+p**\ *pen*]
    Draw the equivalent circular normal distribution, i.e., the *von Mises*
    distribution; append desired pen [0.25p,black].
    The *mode* selects which central location and scale to use:

    * 0 = mean and standard deviation;
    * 1 = median and L1 scale (1.4826 \* median absolute deviation; MAD);
    * 2 = LMS (least median of squares) mode and scale.

    **Note**: At the present time, only *mode* == 0 is supported.

.. _-Q:

**-Q**\ [*alpha*]
    Sets the confidence level used to determine if the mean resultant
    is significant (i.e., Lord Rayleigh test for uniformity) [0.05].
    **Note**: The critical values are approximated [Berens, 2009] and
    requires at least 10 points; the critical resultants are accurate
    to at least 3 significant digits.  For smaller data sets you
    should consult exact statistical tables.

.. _-R:

**-R**\ *r0*/*r1*/*az0*/*az1*
    Specifies the 'region' of interest in (*r*\ ,\ *azimuth*) space. Here, *r0* is 0, *r1*
    is max length in units. For azimuth, specify either -90/90 or 0/180
    for half circle plot or 0/360 for full circle.

.. _-S:

**-S**\ [**+a**]
    Normalize input radii (or bin counts if |-A| is used) by the largest
    value so all radii (or bin counts) range from 0 to 1.  Optionally,
    further normalize rose plots for area (i.e., take :math:`sqrt(r)` before
    plotting [Default is no normalizations].

.. _-T:

**-T**
    Specifies that the input data are orientation data (i.e., have a 180 degree
    ambiguity) instead of true 0-360 degree directions [Default]. We
    compensate by counting each record twice: First as *azimuth* and second
    as *azimuth + 180*.  Ignored if range is given as -90/90 or 0/180.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *pen*
    Set pen attributes for sector outline or rose plot. [Default is no
    outline]. Use **-Wv**\ *pen* to change pen used to draw vector
    (requires |-E|) [Default is same as sector outline].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ **u**\|\ *scale*
    Multiply the data radii by *scale*. E.g., use **-Z**\ 0.001 to
    convert your data from m to km. To exclude the radii from
    consideration, set them all to unity with **-Zu** [Default is no
    scaling].

**-:**
    Input file has (*azimuth,radius*) pairs rather than the expected
    (*radius,azimuth*).

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_
.. include:: explain_-ocols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-qi.rst_

.. include:: explain_-s.rst_

.. include:: explain_-t.rst_

.. include:: explain_-w.rst_

.. include:: explain_help.rst_

.. include:: explain_vectors.rst_

.. module_common_ends

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To plot a half circle rose diagram of the data in the remote data file
azimuth_lengths.txt (containing pairs of (azimuth, length in meters)),
using a 5 degree bin sector width, on a circle of diameter = 10 cm, using a light blue
shading, try::

    gmt rose @azimuth_lengths.txt -: -A5 -JX10c -F -L -Glightblue -R0/1/0/180 -Bxaf+l"Fault length" -Byg30 -S -pdf half_rose

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

.. module_note_begins

Bugs
----

No default radial scale and grid settings for polar histograms. Users
must run the module with |-I| to find max length in binned data set.

References
----------

Berens, P., 2009, CircStat: A MATLAB Toolbox for Circular Statistics, *J. Stat. Software, 31(10)*\ , 1-21.

.. module_note_ends

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`, :doc:`histogram`
