********
filter1d
********

filter1d - Do time domain filtering of 1-D data tables

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**filter1d** [ *table* ] **-F**\ *type<width>*\ [*mode*\ ] [
**-D**\ *increment* ] [ **-E** ] [ **-I**\ *ignore\_val* ] [
**-L**\ *lack\_width* ] [ **-N**\ *t\_col* ] [ **-Q**\ *q\_factor* ] [
**-S**\ *symmetry\_factor* ] [ **-T**\ *t\_min/t\_max/t\_inc*\ [**+**\ ]
] [ **-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ]

|No-spaces|

Description
-----------

**filter1d** is a general time domain filter for multiple column time
series data. The user specifies which column is the time (i.e., the
independent variable). (See **-N** option below). The fastest operation
occurs when the input time series are equally spaced and have no gaps or
outliers and the special options are not needed. **filter1d** has
options **-L**, **-Q**, and **-S** for unevenly sampled data with gaps.

Required Arguments
------------------

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

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

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
**-T**\ *t\_min/t\_max/t\_inc*\ [**+**\ ]
    Make evenly spaced time-steps from *t\_min* to *t\_max* by *t\_inc*
    [Default uses input times]. Append **+** to *t\_inc* if you are
    specifying the number of equidistant points instead. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Examples
--------

To filter the data set in the file cruise.gmtd containing evenly spaced
gravity, magnetics, topography, and distance (in m) with a 10 km
Gaussian filter, removing outliers, and output a filtered value every 2
km between 0 and 100 km:

    filter1d cruise.gmtd -T0/1.0e5/2000 -FG10000 -N3 -V > filtered\_cruise.gmtd

Data along track often have uneven sampling and gaps which we do not
want to interpolate using `sample1d <sample1d.html>`_. To find the median depth in a 50
km window every 25 km along the track of cruise v3312, stored in
v3312.dt, checking for gaps of 10km and asymmetry of 0.3:

    filter1d v3312.dt -FM50 -T0/100000/25 -L10 -S0.3 > v3312\_filt.dt

See Also
--------

`gmt5 <gmt5.html>`_ , `sample1d <sample1d.html>`_
