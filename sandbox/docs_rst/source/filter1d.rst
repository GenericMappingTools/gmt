********
filter1d
********

filter1d - Do time domain filtering of 1-D data tables

`Synopsis <#toc1>`_
-------------------

**filter1d** [ *table* ] **-F**\ *type<width>*\ [*mode*\ ] [
**-D**\ *increment* ] [ **-E** ] [ **-I**\ *ignore\_val* ] [
**-L**\ *lack\_width* ] [ **-N**\ *t\_col* ] [ **-Q**\ *q\_factor* ] [
**-S**\ *symmetry\_factor* ] [ **-T**\ *start/stop/int* ] [
**-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ]

`Description <#toc2>`_
----------------------

**filter1d** is a general time domain filter for multiple column time
series data. The user specifies which column is the time (i.e., the
independent variable). (See **-N** option below). The fastest operation
occurs when the input time series are equally spaced and have no gaps or
outliers and the special options are not needed. **filter1d** has
options **-L**, **-Q**, and **-S** for unevenly sampled data with gaps.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-F**\ *type<width>*\ [*mode*\ ]
    Sets the filter *type*. Choose among convolution and non-convolution
    filters. Append the filter code followed by the full filter
    *<width>* in same units as time column. Available convolution
    filters are:

    (**b**) Boxcar: All weights are equal.

    (**c**) Cosine Arch: Weights follow a cosine arch curve.

    (**g**) Gaussian: Weights are given by the Gaussian function.

    (**f**) Custom: Instead of *width* give name of a one-column file
    with your own weight coefficients.

    Non-convolution filters are:

    (**m**) Median: Returns median value.

    (**p**) Maximum likelihood probability (a mode estimator): Return
    modal value. If more than one mode is found we return their average
    value. Append - or + to the filter width if you rather want to
    return the smallest or largest of the modal values.

    (**l**) Lower: Return the minimum of all values.

    (**L**) Lower: Return minimum of all positive values only.

    (**u**) Upper: Return maximum of all values.

    (**U**) Upper: Return maximum or all negative values only.

    Upper case type **B**, **C**, **G**, **M**, **P**, **F** will use
    robust filter versions: i.e., replace outliers (2.5 L1 scale off
    median) with median during filtering.

    In the case of **L**\ \|\ **U** it is possible that no data passes
    the initial sign test; in that case the filter will return 0.0.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-D**\ *increment*
    *increment* is used when series is NOT equidistantly sampled. Then
    *increment* will be the abscissae resolution, i.e., all abscissae
    will be rounded off to a multiple of *increment*. Alternatively,
    resample data with **sample1d**.
**-E**
    Include Ends of time series in output. Default loses half the
    filter-width of data at each end.
**-I**\ *ignore\_val*
    To ignore values; If an input value equals *ignore\_val* it will be
    set to NaN.
**-L**\ *lack\_width*
    Checks for Lack of data condition. If input data has a gap exceeding
    *width* then no output will be given at that point [Default does not
    check Lack].
**-N**\ *t\_col*
    Indicates which column contains the independent variable (time). The
    left-most column is # 0, the right-most is # (*n\_cols* - 1).
    [Default is 0].
**-Q**\ *q\_factor*
    assess Quality of output value by checking mean weight in
    convolution. Enter *q\_factor* between 0 and 1. If mean weight <
    *q\_factor*, output is suppressed at this point [Default does not
    check Quality].
**-S**\ *symmetry\_factor*
    Checks symmetry of data about window center. Enter a factor between
    0 and 1. If ( (abs(n\_left - n\_right)) / (n\_left + n\_right) ) >
    *factor*, then no output will be given at this point [Default does
    not check Symmetry].
**-T**\ *start/stop/int*
    Make evenly spaced time-steps from *start* to *stop* by *int*
    [Default uses input times].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input.
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is same as input].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Ascii Format Precision <#toc6>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Examples <#toc7>`_
-------------------

To filter the data set in the file cruise.gmtd containing evenly spaced
gravity, magnetics, topography, and distance (in m) with a 10 km
Gaussian filter, removing outliers, and output a filtered value every 2
km between 0 and 100 km:

filter1d cruise.gmtd -T0/1.0e5/2000 -FG10000 -N3 -V >
filtered\_cruise.gmtd

Data along track often have uneven sampling and gaps which we do not
want to interpolate using **sample1d**. To find the median depth in a 50
km window every 25 km along the track of cruise v3312, stored in
v3312.dt, checking for gaps of 10km and asymmetry of 0.3:

filter1d v3312.dt -FM50 -T0/100000/25 -L10 -S0.3 > v3312\_filt.dt

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*sample1d*\ (1) <sample1d.html>`_
