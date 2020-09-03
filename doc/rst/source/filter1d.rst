.. index:: ! filter1d
.. include:: module_core_purpose.rst_

********
filter1d
********

|filter1d_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt filter1d** [ *table* ] |-F|\ *type<width>*\ [*modifier*]
[ |-D|\ *increment* ] [ |-E| ]
[ |-L|\ *lack\_width* ] [ |-N|\ *t\_col* ] [ |-Q|\ *q\_factor* ]
[ |-S|\ *symmetry\_factor* ]
[ |-T|\ [*min/max*\ /]\ *inc*\ [**+e**\|\ **a**\|\ **n**] \|\ |-T|\ *file*\|\ *list* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-j| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**filter1d** is a general time domain filter for multiple column time
series data. The user specifies which column is the time (i.e., the
independent variable). (See **-N** option below). The fastest operation
occurs when the input time series are equally spaced and have no gaps or
outliers and the special options are not needed. **filter1d** has
options **-L**, **-Q**, and **-S** for unevenly sampled data with gaps.
For spatial series there is an option to compute along-track distances
and use that as the independent variable for filtering.

Required Arguments
------------------

.. _-F:

**-F**\ **type**\ *width*\ [*modifier*]
    Sets the filter **type**. Choose among convolution and non-convolution
    filters. Append the filter code followed by the full filter
    *width* in same units as time column. By default we
    perform low-pass filtering; append **+h** to select high-pass filtering.
    Some filters allow for optional arguments and a modifier. Available convolution
    filter types are:

    (**b**) Boxcar: All weights are equal.

    (**c**) Cosine Arch: Weights follow a cosine arch curve.

    (**g**) Gaussian: Weights are given by the Gaussian function.

    (**f**) Custom: Instead of *width* give name of a one-column file
    with your own weight coefficients.

    Non-convolution filter types are:

    (**m**) Median: Returns median value.

    (**p**) Maximum likelihood probability (a mode estimator): Return
    modal value. If more than one mode is found we return their average
    value. Append **+l** or **+u** if you rather want
    to return the lowermost or uppermost of the modal values.

    (**l**) Lower: Return the minimum of all values.

    (**L**) Lower: Return minimum of all positive values only.

    (**u**) Upper: Return maximum of all values.

    (**U**) Upper: Return maximum of all negative values only.

    Upper case type **B**, **C**, **G**, **M**, **P**, **F** will use
    robust filter versions: i.e., replace outliers (2.5 L1 scale off
    median, using 1.4826 \* median absolute deviation [MAD]) with median during filtering.

    In the case of **L**\|\ **U** it is possible that no data passes
    the initial sign test; in that case the filter will return 0.0.
    Apart from custom coefficients (**f**), the other filters may accept variable
    filter widths by passing *width* as a two-column time-series file with filter widths
    in the second column.  The filter-width file does not need to be co-registered with
    the data as we obtain the required filter width at each output location via
    interpolation.  For multi-segment data files the filter file must either have
    the same number of segments or just a single segment to be used for all data
    segments.


Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-D:

**-D**\ *increment*
    *increment* is used when series is NOT equidistantly sampled. Then
    *increment* will be the abscissae resolution, i.e., all abscissae
    will be rounded off to a multiple of *increment*. Alternatively,
    resample data with :doc:`sample1d`.

.. _-E:

**-E**
    Include Ends of time series in output. Default loses half the filter-width of data at each end.

.. _-L:

**-L**\ *lack_width*
    Checks for Lack of data condition. If input data has a gap exceeding
    *width* then no output will be given at that point [Default does not check Lack].

.. _-N:

**-N**\ *t_col*
    Indicates which column contains the independent variable (time). The
    left-most column is # 0, the right-most is # (*n_cols* - 1).  [Default is 0].

.. _-Q:

**-Q**\ *q_factor*
    Assess Quality of output value by checking mean weight in
    convolution. Enter *q_factor* between 0 and 1. If mean weight <
    *q_factor*, output is suppressed at this point [Default does not check Quality].

.. _-S:

**-S**\ *symmetry_factor*
    Checks symmetry of data about window center. Enter a factor between
    0 and 1. If ( (abs(n_left - n_right)) / (n_left + n_right) ) >
    *factor*, then no output will be given at this point [Default does
    not check Symmetry].

.. _-T:

**-T**\ [*min/max*\ /]\ *inc*\ [**+e**\|\ **a**\|\ **n**] \|\ |-T|\ *file*\|\ *list*
    Make evenly spaced time-steps from *min* to *max* by *inc* [Default uses input times].
    For details on array creation, see `Generate 1D Array`_.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_distcalc.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-q.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

.. include:: explain_array.rst_

Examples
--------

.. include:: explain_example.rst_

To filter the remote CO2 data set in the file MaunaLoa_CO2.txt (year, CO2)
with a 5 year Gaussian filter, try

   ::

    gmt filter1d @MaunaLoa_CO2.txt -Fg5 > CO2_trend.txt

Data along track often have uneven sampling and gaps which we do not
want to interpolate using :doc:`sample1d`. To find the median depth in a 50
km window every 25 km along the track of cruise v3312, stored in
v3312.dt, checking for gaps of 10km and asymmetry of 0.3:

   ::

    gmt filter1d v3312.dt -FM50 -T0/100000/25 -L10 -S0.3 > v3312_filt.dt

To smooth a noisy geospatial track using a Gaussian filter of full-width 100 km
and not shorten the track, and add the distances to the file, use

   ::

    gmt filter1d track.txt -Tk+a -E -Fg200 > smooth_track.txt

See Also
--------

:doc:`gmt` ,
:doc:`sample1d` ,
:doc:`splitxyz`
