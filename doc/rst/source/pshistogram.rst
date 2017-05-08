.. index:: ! pshistogram

***********
pshistogram
***********

.. only:: not man

    pshistogram - Calculate and plot histograms

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**pshistogram** [ *table* ] |-J|\ **x**\ \|\ **X**\ *parameters*
|-W|\ *bin_width*\ [**+l**\ \|\ **h**\ \|\ **b**]
[ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ [**+b**][**+f**\ *font*][**+o**\ *off*][**+r**] ]
[ |-F| ]
[ |-G|\ *fill* ] [ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-I|\ [**o**\ \|\ **O**] ]
[ |-K| ] [ |-L|\ *pen* ] 
[ |-N|\ [*mode*][**+p**\ *pen*] ]
[ |-O| ] [|-P| ] [ |-Q| ]
[ |SYN_OPT-R| ]
[ |-S| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
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

|No-spaces|

Description
-----------

**pshistogram** reads *file* [or standard input] and examines the first
data column (or use **-i**) to calculate histogram parameters based on
the bin-width provided. Using these parameters, scaling, and optional
range parameters it will generate PostScript code that plots a
histogram. A cumulative histogram may also be specified. 

Required Arguments
------------------

.. _-J:

**-Jx**
    *xscale[/yscale]* (Linear scale(s) in distance unit/data unit).

.. _-W:

**-W**\ *bin_width*\ [**+l**\ \|\ **h**\ \|\ **b**]
    Sets the bin width used for histogram calculations.
    The modifiers specify the handling of extreme values that fall outside the range
    set by **-R**.  By default these values are ignored.  Use **+b** to let
    these values be included in the first or last bins.  To only include
    extreme values below first bin into the first bin, use **+l**, and to
    only include extreme values above the last bin into that last bin, use
    **+h**.

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

**-G**\ *fill*
    Select filling of bars [Default is no fill].

.. _-I:

**-I**\ [**o**\ \|\ **O**]
    Inquire about min/max x and y after binning. The *xmin xmax ymin
    ymax* is output; no plotting is done. Append **o** to output an
    ASCII table of the resulting x,y data instead. Upper case **O** will
    output all x,y bin data even when y == 0. 

.. include:: explain_-Jz.rst_

.. _-K:

.. include:: explain_-K.rst_

.. _-L:

**-L**\ *pen*
    Draw bar outline using the specified pen thickness. [Default is no outline]. 

.. _-N:

**-N**\ [*mode*][**+p**\ *pen*]
    Draw the equivalent normal distribution; append desired pen [0.5p,black].
    The *mode* selects which central location and scale to use:
 
    * 0 = mean and standard deviation [Default];
    * 1 = median and L1 scale;
    * 2 = LMS mode and scale.

    The **-N** option may be repeated to draw several of these curves.

.. _-O:

.. include:: explain_-O.rst_

.. _-P:

.. include:: explain_-P.rst_

.. _-Q:

**-Q**
    Draw a cumulative histogram. 

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| replace:: If not given, **pshistogram** will automatically find reasonable values for the region.
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

Examples
--------

To draw a histogram of the data v3206.t containing seafloor depths,
using a 250 meter bin width, center bars, and draw bar outline, use:

   ::

    gmt pshistogram v3206.t -JXh -W250 -F -LP0.5p -V > plot.ps

If you know the distribution of your data, you may explicitly specify
range and scales. E.g., to plot a histogram of the y-values (2nd column)
in the file errors.xy using a 1 meter bin width, plot from -10 to +10
meters @ 0.75 cm/m, annotate every 2 m and 100 counts, and use black
bars, run:

   ::

    gmt pshistogram errors.xy -W1 -R-10/10/0/0 -Jxc/0.01c \
                    -Bx2+lError -By100+lCounts -Gblack -i1 -V > plot.ps

Since no y-range was specified, **pshistogram** will calculate *ymax* in even
increments of 100.

Bugs
----

The **-W** option does not yet work properly with time series data
(e.g., **-f**\ 0T). Thus, such variable intervals as months and years
are not calculated. Instead, specify your interval in the same units as
the current setting of **TIME_UNIT**.

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`psbasemap`, :doc:`psrose`,
:doc:`psxy`
