.. index:: ! dimfilter

*********
dimfilter
*********

.. only:: not man

    dimfilter - Directional filtering of 2-D gridded files in the space (or time) domain

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt dimfilter** *input_file.nc*
|-D|\ *distance_flag*
|-F|\ **x**\ *width*\ [*modifier*]
|-G|\ *output_file.nc*
|-N|\ **x**\ *sectors*\ [*modifier*]
[ |-L| ]
[ |-Q| ]
[ |SYN_OPT-I| ]
[ |SYN_OPT-R| ]
[ |-T| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**dimfilter** will filter a *.nc* file in the space (or time) domain by
dividing the given filter circle into *n\_sectors*, applying one of the
selected primary convolution or non-convolution filters to each sector,
and choosing the final outcome according to the selected secondary
filter. It computes distances using Cartesian or Spherical geometries.
The output *.nc* file can optionally be generated as a subregion of the
input and/or with a new **-I**\ ncrement. In this way, one may have
"extra space" in the input data so that there will be no edge effects
for the output grid. If the filter is low-pass, then the output may be
less frequently sampled than the input. The **-Q** option is for the error analysis
mode and expects the input file to contains the filtered depths. Finally, one should know that
**dimfilter** will not produce a smooth output as other spatial filters
do because it returns a minimum median out of *N* medians of *N*
sectors. The output can be rough unless the input data is noise-free.
Thus, an additional filtering (e.g., Gaussian via :doc:`grdfilter`) of the
DiM-filtered data is generally recommended.

Required Arguments
------------------

*input\_file.nc*
    The data grid to be filtered.

.. _-D:

**-D**\ *distance\_flag*
    Distance *flag* tells how grid (x,y) relates to filter *width*, as follows:

    *flag* = 0: grid (x,y) same units as *width*, Cartesian distances.
    *flag* = 1: grid (x,y) in degrees, *width* in kilometers, Cartesian distances.
    *flag* = 2: grid (x,y) in degrees, *width* in km, dx scaled by
    cos(middle y), Cartesian distances.

    The above options are fastest because they allow weight matrix to be
    computed only once. The next three options are slower because they
    recompute weights for each latitude.

    *flag* = 3: grid (x,y) in degrees, *width* in km, dx scaled by
    cosine(y), Cartesian distance calculation.

    *flag* = 4: grid (x,y) in degrees, *width* in km, Spherical distance
    calculation.

.. _-F:

**-F**\ **x**\ *width*\ [*modifier*]
    Sets the primary filter type. Choose among convolution and
    non-convolution filters. Append the filter code **x** followed by the full
    diameter *width*. Available convolution filters are:

    (**b**) Boxcar: All weights are equal.

    (**c**) Cosine Arch: Weights follow a cosine arch curve.

    (**g**) Gaussian: Weights are given by the Gaussian function.

    Non-convolution filters are:

    (**m**) Median: Returns median value.

    (**p**) Maximum likelihood probability (a mode estimator): Return
    modal value. If more than one mode is found we return their average
    value. Append **+l** or **+h** to the filter width if you rather want to
    return the smallest or largest of each sector's modal values.

.. _-N:

**-N**\ **x**\ *sectors*\ [*modifier*]
    Sets the secondary filter type **x** and the number of bow-tie sectors.
    *sectors* must be integer and larger than 0. When *sectors* is
    set to 1, the secondary filter is not effective. Available secondary
    filters **x** are:

    (**l**) Lower: Return the minimum of all filtered values.

    (**u**) Upper: Return the maximum of all filtered values.

    (**a**) Average: Return the mean of all filtered values.

    (**m**) Median: Return the median of all filtered values.

    (**p**) Mode: Return the mode of all filtered values:
    If more than one mode is found we return their average
    value. Append **+l** or **+h** to the sectors if you rather want to
    return the smallest or largest of the modal values.

.. _-G:

**-G**\ *output_file.nc*
    *output_file.nc* is the output of the filter.

Optional Arguments
------------------

.. _-I:

**-I**
    *x_inc* [and optionally *y_inc*] is the output Increment. Append
    **m** to indicate minutes, or **c** to indicate seconds. If the new
    *x_inc*, *y_inc* are NOT integer multiples of the old ones (in the
    input data), filtering will be considerably slower. [Default: Same
    as input.]

.. _-L:

**-L**
    This option is used by itself to write the dim.template.sh bash script
    to standard output.  No other options can be used in combination.

.. _-R:

**-R**
    *west*, *east*, *south*, and *north* defines the Region of the
    output points. [Default: Same as input.]

.. _-T:

**-T**
    Toggle the node registration for the output grid so as to become the
    opposite of the input grid [Default gives the same registration as
    the input grid].

.. _-Q:

**-Q**
    For this mode, it expects to read depths consisted of several
    columns. Each column represents a filtered grid with a filter width,
    which can be obtained by **grd2xyz -Z**. The outcome will be median,
    MAD, and mean. So, the column with the medians is used to generate
    the regional component and the column with the MADs is used to
    conduct the error analysis.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_
.. include:: explain_grd_inout_short.rst_
.. include:: explain_grd_coord.rst_

Examples
--------

Suppose that north_pacific_dbdb5.nc is a file of 5 minute bathymetry
from 140E to 260E and 0N to 50N, and you want to find the medians of
values within a 300km radius (600km full width) of the output points,
which you choose to be from 150E to 250E and 10N to 40N, and you want
the output values every 0.5 degree. To prevent the medians from being
biased by the sloping plane, you want to divide the filter circle into 6
sectors and to choose the lowest value among 6 medians. Using spherical
distance calculations, you need:

   ::

    gmt dimfilter north_pacific_dbdb5.nc -Gfiltered_pacific.nc -Fm600 -D4 \
        -Nl6 -R150/250/10/40 -I0.5 -V

Suppose that cape_verde.nc is a file of 0.5 minute bathymetry from 32W
to 15W and 8N to 25N, and you want to remove small-length-scale features
in order to define a swell in an area extending from 27.5W to 20.5W and
12.5N to 19.5N, and you want the output value every 2 minute. Using
cartesian distance calculations, you need:

   ::

    gmt dimfilter cape_verde.nc -Gt.nc -Fm220 -Nl8 -D2 -R-27.5/-20.5/12.5/19.5 -I2m -V
    gmt grdfilter t.nc -Gcape_swell.nc -Fg50 -D2 -V

Suppose that you found a range of filter widths for a given area, and
you filtered the given bathymetric data using the range of filter widths
(e.g., *f100.nc f110.nc f120.nc f130.nc*), and you want to define a
regional trend using the range of filter widths, and you want to obtain
median absolute deviation (MAD) estimates at each data point. Then, you
will need to do:

   ::

    gmt grd2xyz f100.nc -Z > f100.txt
    gmt grd2xyz f110.nc -Z > f110.txt
    gmt grd2xyz f120.nc -Z > f120.txt
    gmt grd2xyz f130.nc -Z > f130.txt
    paste f100.txt f110.txt f120.txt f130.txt > depths.txt
    gmt dimfilter depths.txt -Q > output.z

Limitations
-----------

When working with geographic (lat, lon) grids, all three convolution
filters (boxcar, cosine arch, and gaussian) will properly normalize the
filter weights for the variation in gridbox size with latitude, and
correctly determine which nodes are needed for the convolution when the
filter "circle" crosses a periodic (0-360) boundary or contains a
geographic pole. However, the spatial filters, such as median and mode
filters, do not use weights and thus should only be used on Cartesian
grids (or at very low latitudes) only. If you want to apply such spatial
filters you should project your data to an equal-area projection and run
**dimfilter** on the resulting Cartesian grid.

Script Template
---------------

The dim.template.sh is a skeleton shell script that can be used to set
up a complete DiM analysis, including the MAD analysis.  It is obtained
via the **-L** option.

Reference
---------

Kim, S.-S., and Wessel, P. (2008), Directional Median Filtering for
Regional-Residual Separation of Bathymetry, *Geochem. Geophys.
Geosyst.*, **9**, Q03005, doi:10.1029/2007GC001850.

See Also
--------

:doc:`gmt`, :doc:`grdfilter`
