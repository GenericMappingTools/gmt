.. index:: ! histogram

***********
histogram
***********

.. only:: not man

    Calculate and plot histograms

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt histogram** [ *table* ] |-J|\ **x**\ \|\ **X**\ *parameters*
|-T|\ [\ *min/max*\ /]\ *inc*\ [**n**] \|\ |-T|\ *file*\ \|\ *list*
[ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ [**+b**][**+f**\ *font*][**+o**\ *off*][**+r**] ]
[ |-F| ]
[ |-G|\ *fill* ] [ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-I|\ [**o**\ \|\ **O**] ]
[ |-L|\ **l**\ \|\ **h**\ \|\ **b**] ]
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
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**histogram** reads *file* [or standard input] and examines the first
data column (or one set by **-i**) to calculate histogram parameters based on
the bin-width provided. Using these parameters, scaling, and optional
range parameters it will plot the histogram. A cumulative histogram may also be specified. 

Required Arguments
------------------

.. _-J:

**-Jx**
    *xscale[/yscale]* (Linear scale(s) in distance unit/data unit).

.. _-T:

**-T**\ [\ *min/max*\ /]\ *inc*\ [**n**] \|\ |-T|\ *file*\ \|\ *list*
    Make evenly spaced array of bin boundaries from *min* to *max* by *inc*.
    If *min/max* are not given then we default to the range in **-R**.
    For details on array creation, see `Generate 1D Array`_.

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

**-A**
    Plot the histogram horizontally from x = 0 [Default is vertically
    from y = 0].  The plot dimensions remain the same, but the two
    axes are flipped.

.. _-B:

.. include:: explain_-B.rst_

.. _-C:

**-C**\ *cpt*
    Give a CPT. The mid x-value for each bar is used to
    look-up the bar color.

.. _-D:

**-D**\ [**+b**][**+f**\ *font*][**+o**\ *off*][**+r**]
    Annotate each bar with the count it represents.  Append any of the
    following modifiers: Use **+b** to place the labels beneath the bars
    instead of above; use **+f** to change to another font than the default
    annotation font; use **+o** to change the offset between bar and label [6p];
    use **+r** to rotate the labels from horizontal to vertical.

.. _-F:

**-F**
    Center bin on each value. [Default is left edge].

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Select filling of bars [Default is no fill].

.. _-I:

**-I**\ [**o**\ \|\ **O**]
    Inquire about min/max x and y after binning. The *xmin xmax ymin
    ymax* is output; no plotting is done. Append **o** to output an
    ASCII table of the resulting x,y data instead. Upper case **O** will
    output all x,y bin data even when y == 0. 

.. include:: explain_-Jz.rst_

.. _-L:

**-Ll**\ \|\ **h**\ \|\ **b**
    The modifiers specify the handling of extreme values that fall outside the range
    set by **-T**.  By default these values are ignored.  Append **b** to let
    these values be included in the first or last bins.  To only include
    extreme values below first bin into the first bin, use **l**, and to
    only include extreme values above the last bin into that last bin, use
    **h**.

.. _-N:

**-N**\ [*mode*][**+p**\ *pen*]
    Draw the equivalent normal distribution; append desired pen [0.5p,black].
    The *mode* selects which central location and scale to use:
 
    * 0 = mean and standard deviation [Default];
    * 1 = median and L1 scale (1.4826 \* median absolute deviation; MAD);
    * 2 = LMS mode and scale.

    The **-N** option may be repeated to draw several of these curves.

.. _-Q:

**-Q**\ **r**
    Draw a cumulative histogram. Append **r** to instead compute the
    reverse cumulative histogram.

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| replace:: If not given, **histogram** will automatically find reasonable values for the region.
.. include:: explain_-Rz.rst_

.. _-S:

**-S**
    Draws a stairs-step diagram which does not include the internal bars
    of the default histogram. 

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *pen*
    Draw bar outline (or stair-case curve) using the specified pen thickness. [Default is no outline]. 

.. _-X:

.. include:: explain_-XY.rst_

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

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_array.rst_

Examples
--------

To draw a histogram of the data v3206.t containing seafloor depths,
using a 250 meter bin width, center bars, and draw bar outline, use:

   ::

    gmt histogram v3206.t -JXh -T250 -F -W0.5p -V -pdf plot

If you know the distribution of your data, you may explicitly specify
range and scales. E.g., to plot a histogram of the y-values (2nd column)
in the file errors.xy using a 1 meter bin width, plot from -10 to +10
meters @ 0.75 cm/m, annotate every 2 m and 100 counts, and use black
bars, run:

   ::

    gmt histogram errors.xy -T1 -R-10/10/0/0 -Jxc/0.01c
                  -Bx2+lError -By100+lCounts -Gblack -i1 -V -pdf plot

Since no y-range was specified, **histogram** will calculate *ymax* in even
increments of 100.

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`basemap`, :doc:`rose`,
:doc:`plot`
