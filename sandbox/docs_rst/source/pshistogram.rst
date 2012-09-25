***********
pshistogram
***********

pshistogram - Calculate and plot histograms

`Synopsis <#toc1>`_
-------------------

**pshistogram** [ *table* ] **-Jx**\ \|\ **X**\ *parameters*
**-W**\ *bin\_width* [ **-A** ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *cptfile* ] [
**-F** ] [ **-G**\ *fill* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [
**-I**\ [**o**\ \|\ **O**] ] [ **-K** ] [ **-L**\ *pen* ] [ **-O** ] [
**-P** ] [ **-Q** ] [ **-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] ] [
**-S** ] [ **-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [
**-V**\ [*level*\ ] ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-Z**\ *type* ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [
**-c**\ *copies* ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**pshistogram** reads *file* [or standard input] and examines the first
data column (or use **-i**) to calculate histogram parameters based on
the bin-width provided. Using these parameters, scaling, and optional
range parameters it will generate *PostScript* code that plots a
histogram. A cumulative histogram may also be specified.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-Jx**
    *xscale[/yscale]* (Linear scale(s) in distance unit/data unit).
**-W**\ *bin\_width*
    Sets the bin width used for histogram calculations.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-A**
    Plot the histogram horizontally from x = 0 [Default is vertically
    from y = 0].
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-C**\ *cptfile*
    Give a color palette file. The mid x-value for each bar is used to
    look-up the bar color.
**-F**
    Center bin on each value. [Default is left edge].
**-G**\ *fill*
    Select filling of bars [Default is no fill].
**-I**\ [**o**\ \|\ **O**]
    Inquire about min/max x and y after binning. No plotting is done.
    Append **o** to output an ASCII table of the resulting x,y data to
    stdout. Alternatively, append **O** to output all x,y bin data even
    when y == 0.
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**\ *pen*
    Draw bar outline using the specified pen thickness. [Default is no
    outline].
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**
    Draw a cumulative histogram.
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*. If
    not given, **pshistogram** will automatically find reasonable values
    for the region.
**-S**
    Draws a stairs-step diagram which does not include the internal bars
    of the default histogram.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-Z**\ *type*
    Choose between 6 types of histograms: 0 = counts [Default], 1 =
    frequency\_percent, 2 = log (1.0 + count), 3 = log (1.0 +
    frequency\_percent), 4 = log10 (1.0 + count), 5 = log10 (1.0 +
    frequency\_percent).
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

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

`*gmt*\ (1) <gmt.html>`_ , `*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_ , `*psrose*\ (1) <psrose.html>`_ ,
`*psxy*\ (1) <psxy.html>`_
