***********
pshistogram
***********

pshistogram - Calculate and plot histograms

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**pshistogram** [ *table* ] **-Jx**\ \|\ **X**\ *parameters*
**-W**\ *bin\_width* [ **-A** ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *cptfile* ] [
**-F** ] [ **-G**\ *fill* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [
**-I**\ [**o**\ \|\ **O**] ] [ **-K** ] [ **-L**\ *pen* ] 
[ **-N**\ [*mode*][**+p**\ *pen*] ] [ **-O** ] [**-P** ] [ **-Q** ] [
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] ] [ **-S** ]
[ **-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [
**-V**\ [*level*\ ] ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-Z**\ *type* ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [
**-c**\ *copies* ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**pshistogram** reads *file* [or standard input] and examines the first
data column (or use **-i**) to calculate histogram parameters based on
the bin-width provided. Using these parameters, scaling, and optional
range parameters it will generate *PostScript* code that plots a
histogram. A cumulative histogram may also be specified. 

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

**-Jx**
    *xscale[/yscale]* (Linear scale(s) in distance unit/data unit).
**-W**\ *bin\_width*
    Sets the bin width used for histogram calculations.

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

**-A**
    Plot the histogram horizontally from x = 0 [Default is vertically
    from y = 0]. 

.. include:: explain_-B.rst_

**-C**\ *cptfile*
    Give a color palette file. The mid x-value for each bar is used to
    look-up the bar color.
**-F**
    Center bin on each value. [Default is left edge].
**-G**\ *fill*
    Select filling of bars [Default is no fill].
**-I**\ [**o**\ \|\ **O**]
    Inquire about min/max x and y after binning. The *xmin xmax ymin
    ymax* is output; no plotting is done. Append **o** to output an
    ASCII table of the resulting x,y data instead. Upper case **O** will
    output all x,y bin data even when y == 0. 

.. include:: explain_-Jz.rst_

.. include:: explain_-K.rst_

**-L**\ *pen*
    Draw bar outline using the specified pen thickness. [Default is no outline]. 

**-N**\ [*mode*][**+p**\ *pen*]
    Draw the equivalent normal distribution; append desired pen [0.5p,black].
    The *mode* selects which central location and scale to use:
 
    * 0 = mean and standard deviation [Default];
    * 1 = median and L1 scale;
    * 2 = LMS mode and scale.

    The **-N** option may be repeated to draw several of these curves.

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-Q**
    Draw a cumulative histogram. 

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| replace:: If not given, **pshistogram** will automatically find reasonable values for the region.
.. include:: explain_-Rz.rst_

**-S**
    Draws a stairs-step diagram which does not include the internal bars
    of the default histogram. 

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-XY.rst_

**-Z**\ *type*
    Choose between 6 types of histograms: 

    * 0 = counts [Default] 
    * 1 = frequency\_percent 
    * 2 = log (1.0 + count) 
    * 3 = log (1.0 + frequency\_percent) 
    * 4 = log10 (1.0 + count) 
    * 5 = log10 (1.0 + frequency\_percent). 

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. include:: explain_-c.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

`Examples <#toc6>`_
-------------------

To draw a histogram of the data v3206.t containing seafloor depths,
using a 250 meter bin width, center bars, and draw bar outline, use:

    pshistogram v3206.t -JXh -W250 -F -LP0.5p -V > plot.ps

If you know the distribution of your data, you may explicitly specify
range and scales. E.g., to plot a histogram of the y-values (2nd column)
in the file errors.xy using a 1 meter bin width, plot from -10 to +10
meters @ 0.75 cm/m, annotate every 2 m and 100 counts, and use black
bars, run:

    pshistogram errors.xy -W1 -R-10/10/0/0 **-Jx**\ LENBD(c)/0.01\ **c**
    -B2:Error:/100:Counts: -Gblack -i1 -V > plot.ps

Since no y-range was specified, pshistogram will calculate ymax in even
increments of 100.

`Bugs <#toc7>`_
---------------

The **-W** option does not yet work properly with time series data
(e.g., **-f**\ 0T). Thus, such variable intervals as months and years
are not calculated. Instead, specify your interval in the same units as
the current setting of **TIME\_UNIT**.

`See Also <#toc8>`_
-------------------

`gmt <gmt.html>`_, `gmtcolors <gmtcolors.html>`_,
`psbasemap <psbasemap.html>`_, `psrose <psrose.html>`_,
`psxy <psxy.html>`_
