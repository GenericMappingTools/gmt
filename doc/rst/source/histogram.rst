.. index:: ! histogram
.. include:: module_core_purpose.rst_

***********
histogram
***********

|histogram_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt histogram** [ *table* ]
|-T|\ [*min/max*\ /]\ *inc*\ [**+i**\|\ **n**] \| |-T|\ *file*\|\ *list*
[ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt*\ [**+b**] ]
[ |-D|\ [**+b**][**+f**\ *font*][**+o**\ *off*][**+r**] ]
[ |-E|\ *width*\ [**+o**\ *offset*] ]
[ |-F| ]
[ |-G|\ *fill* ]
[ |-I|\ [**o**\|\ **O**] ]
[ |-J|\ **x**\|\ **X**\ *parameters* ]
[ |-Jz|\ \|\ **Z**\ *parameters* ]
[ |-L|\ **l**\|\ **h**\|\ **b** ]
[ |-N|\ [*mode*][**+p**\ *pen*] ]
[ |-Q|\ **r** ]
[ |SYN_OPT-R| ]
[ |-S| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ [*type*][**+w**] ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-l| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads *file* [or standard input] and examines the first
data column (or one set by **-i**) to calculate histogram parameters based on
the bin-width provided. Using these parameters, scaling, and optional
range parameters it will plot the histogram. A cumulative histogram may also be specified.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-J:

**-Jx**
    *xscale*\ [/*yscale*] (Linear scale(s) in distance unit/data unit), or
    **-JX** with *width*\ [/*height*] dimensions.  **Note**: Optional in modern mode.

.. _-T:

**-T**\ [*min/max*\ /]\ *inc*\ [**+n**] \|\ **-T**\ *file*\|\ *list*
    Make evenly spaced array of bin boundaries from *min* to *max* by *inc*.
    If *min/max* are not given then we default to the range in |-R|.
    For details on array creation, see `Generate 1-D Array`_. **Note**: If
    *inc* is given with a trailing time unit then it takes precedence over
    the current setting of :term:`TIME_UNIT`; otherwise that setting determines
    the units used for the bin widths.

Optional Arguments
------------------

.. _-A:

**-A**
    Plot the histogram horizontally from x = 0 [Default is vertically
    from y = 0].  The plot dimensions remain the same, but the two
    axes are flipped.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *cpt*\ [**+b**]
    Give a CPT. The mid-coordinate for each bar is used to look up
    the bar color.  Alternatively, append **+b** to use the *bin* value
    as the look-up value, unless |-Z| involves percentages,
    in which case the look-up value is the *percentage* computed. If
    we are in modern mode and no *cpt* is given then we select the
    current CPT.

.. _-D:

**-D**\ [**+b**][**+f**\ *font*][**+o**\ *off*][**+r**]
    Annotate each bar with the count it represents.  Append any of the
    following modifiers: Use **+b** to place the labels beneath the bars
    instead of above; use **+f** to change to another font than the default
    annotation font; use **+o** to change the offset between bar and label [6p];
    use **+r** to rotate the labels from horizontal to vertical.

.. _-E:

**-E**\ *width*\ [**+o**\ *offset*]
    Use an alternative histogram bar width than the default set via |-T|,
    and optionally shift all bars by an *offset*.  Here *width* is either
    an alternative width in data units, or the user may append a valid plot
    dimension unit (**c**\|\ **i**\|\ **p**) for a fixed dimension instead.
    Optionally, all bins may be shifted along the axis by *offset*. As for
    *width*, it may be given in data units of plot dimension units by appending
    the relevant unit.

.. _-F:

**-F**
    Center bin on each value. [Default is left edge].

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Select filling of bars [Default is no fill].

.. _-I:

**-I**\ [**o**\|\ **O**]
    Inquire about min/max x and y after binning. The *xmin xmax ymin
    ymax* is output; no plotting is done. Append **o** to output an
    ASCII table of the resulting x,y data instead. Upper case **O** will
    output all x,y bin data even when y == 0. **Note**: You may use **-o**
    to select a subset from this record.

.. _-Jz:

.. include:: explain_-Jz.rst_

.. _-L:

**-Ll**\|\ **h**\|\ **b**
    The modifiers specify the handling of extreme values that fall outside the range
    set by |-T|.  By default these values are ignored.  Append **b** to let
    these values be included in the first or last bins.  To only include
    extreme values below first bin into the first bin, use **l**, and to
    only include extreme values above the last bin into that last bin, use
    **h**.

.. _-N:

**-N**\ [*mode*][**+p**\ *pen*]
    Draw the equivalent normal distribution; append desired pen [0.25p,black].
    The *mode* selects which central location and scale to use:

    * 0 = mean and standard deviation [Default];
    * 1 = median and L1 scale (1.4826 \* median absolute deviation; MAD);
    * 2 = LMS (least median of squares) mode and scale.

    The |-N| option may be repeated to draw several of these curves.
    **Note**: If **-w** is used then only *mode* = 0 is available and we will
    determine the parameters of the circular von Mises distribution instead.

.. _-Q:

**-Q**\ **r**
    Draw a cumulative histogram. Append **r** to instead compute the
    reverse cumulative histogram.  Cannot be used with **-w**.

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-Rz| replace:: If not given, we will automatically find reasonable values for the region.
.. include:: explain_-Rz.rst_

.. _-S:

**-S**
    Draws a stairs-step diagram which does not include the internal bars
    of the default histogram.

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
    Draw bar outline (or stair-case curve) using the specified pen thickness. [Default is no outline].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ [*type*][**+w**]
    Choose between 6 types of histograms:

    * 0 = counts [Default]
    * 1 = frequency_percent
    * 2 = log (1.0 + count)
    * 3 = log (1.0 + frequency_percent)
    * 4 = log10 (1.0 + count)
    * 5 = log10 (1.0 + frequency_percent).

    To use weights provided as a second data column instead of pure counts,
    append **+w**.

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_-l| replace:: Symbol is a rectangle with width-to-height ratio of 3:2.  Use **+S**\ *width*\ [/*height*] to overwrite with custom width and optionally height.
.. include:: explain_-l.rst_

.. include:: explain_-ocols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-qi.rst_

.. include:: explain_-s.rst_

.. include:: explain_-t.rst_

.. include:: explain_-w.rst_

.. include:: explain_help.rst_

.. include:: explain_array.rst_

.. module_common_ends

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To draw a histogram of the remote data v3206_06.txt containing seafloor depths,
using a 250 meter bin width, center bars, and draw bar outline, use:

::

  gmt histogram  @v3206_06.txt -F -T250 -W0.25p -B -pdf plot

If you know the distribution of your data, you may explicitly specify
range and scales. E.g., to plot a histogram of the y-values (2nd column)
in the file errors.xy using a 1 meter bin width, plot from -10 to +10
meters @ 0.75 cm/m and 0.01c/count in y, annotate every 2 m and 100 counts,
and use black bars, run:

::

  gmt begin plot
    gmt histogram errors.xy -T1 -R-10/10/0/0 -Jx0.75c/0.01c -Bx2+lError -By100+lCounts -Gblack -i1
  gmt end show

Since no y-range was specified, **histogram** will calculate *ymax* in even
increments of 100.

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`basemap`, :doc:`rose`,
:doc:`plot`
