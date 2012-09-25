*********
dimfilter
*********

dimfilter - Directional filtering of 2-D gridded files in the space (or
time) domain

`Synopsis <#toc1>`_
-------------------

**dimfilter** *input\_file.nc* **-D**\ *distance\_flag*
**-F**\ *<filtertype><width>*\ [*mode*\ ] **-G**\ *output\_file.nc*
**-N**\ *<filtertype><n\_sectors>* [ **-Q**\ *cols* ] [
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
] [ **-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-T** ] [
**-V**\ [*level*\ ] ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

`Description <#toc2>`_
----------------------

**dimfilter** will filter a *.nc* file in the space (or time) domain by
dividing the given filter circle into *n\_sectors*, applying one of the
selected primary convolution or non-convolution filters to each sector,
and choosing the final outcome according to the selected secondary
filter. It computes distances using Cartesian or Spherical geometries.
The output *.nc* file can optionally be generated as a subregion of the
input and/or with a new **-I**\ ncrement. In this way, one may have
"extra space" in the input data so that there will be no edge effects
for the output grid. If the filter is low-pass, then the output may be
less frequently sampled than the input. **-Q** is for the error analysis
mode and only requires the total number of columns in the input file,
which contains the filtered depths. Finally, one should know that
**dimfilter** will not produce a smooth output as other spatial filters
do because it returns a minimum median out of *N* medians of *N*
sectors. The output can be rought unless the input data is noise-free.
Thus, an additional filtering (e.g., Gaussian via **grdfilter**) of the
DiM-filtered data is generally recommended.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*input\_file.nc*
    The data grid to be filtered.
**-D**\ *distance\_flag*
    Distance *flag* tells how grid (x,y) relates to filter *width*, as
    follows:

    *flag* = 0: grid (x,y) same units as *width*, Cartesian distances.
     *flag* = 1: grid (x,y) in degrees, *width* in kilometers, Cartesian
    distances.
     *flag* = 2: grid (x,y) in degrees, *width* in km, dx scaled by
    cos(middle y), Cartesian distances.

    The above options are fastest because they allow weight matrix to be
    computed only once. The next three options are slower because they
    recompute weights for each latitude.

    *flag* = 3: grid (x,y) in degrees, *width* in km, dx scaled by
    cosine(y), Cartesian distance calculation.
     *flag* = 4: grid (x,y) in degrees, *width* in km, Spherical
    distance calculation.

**-F**\ *<filtertype><width>*\ [*mode*\ ]
    Sets the primary filter type. Choose among convolution and
    non-convolution filters. Append the filter code followed by the full
    diameter *width*. Available convolution filters are:
     (**b**) Boxcar: All weights are equal.
     (**c**) Cosine Arch: Weights follow a cosine arch curve.
     (**g**) Gaussian: Weights are given by the Gaussian function.
     Non-convolution filters are:
     (**m**) Median: Returns median value.
     (**p**) Maximum likelihood probability (a mode estimator): Return
    modal value. If more than one mode is found we return their average
    value. Append - or + to the filter width if you rather want to
    return the smallest or largest of the modal values.
**-N**\ *<filtertype><n\_sectors>*
    Sets the secondary filter type and the number of bow-tie sectors.
    *n\_sectors* must be integer and larger than 0. When *n\_sectors* is
    set to 1, the secondary filter is not effective. Available secondary
    filters are:
     (**l**) Lower: Return the minimum of all filtered values.
     (**u**) Upper: Return the maximum of all filtered values.
     (**a**) Average: Return the mean of all filtered values.
     (**m**) Median: Return the median of all filtered values.
     (**p**) Mode: Return the mode of all filtered values.
**-G**\ *output\_file.nc*
    *output\_file.nc* is the output of the filter.

`Optional Arguments <#toc5>`_
-----------------------------

**-I**
    *x\_inc* [and optionally *y\_inc*] is the output Increment. Append
    **m** to indicate minutes, or **c** to indicate seconds. If the new
    *x\_inc*, *y\_inc* are NOT integer multiples of the old ones (in the
    input data), filtering will be considerably slower. [Default: Same
    as input.]
**-R**
    *west*, *east*, *south*, and *north* defines the Region of the
    output points. [Default: Same as input.]
**-T**
    Toggle the node registration for the output grid so as to become the
    opposite of the input grid [Default gives the same registration as
    the input grid].
**-Q**\ *cols*
    *cols* is the total number of columns in the input text table file.
    For this mode, it expects to read depths consisted of several
    columns. Each column represents a filtered grid with a filter width,
    which can be obtained by ’grd2xyz -Z’. The outcome will be median,
    MAD, and mean. So, the column with the medians is used to generate
    the regional component and the column with the MADs is used to
    conduct the error analysis.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Grid File Formats <#toc6>`_
----------------------------

By default **GMT** writes out grid as single precision floats in a
COARDS-complaint netCDF file format. However, **GMT** is able to produce
grid files in many other commonly used grid file formats and also
facilitates so called "packing" of grids, writing out floating point
data as 1- or 2-byte integers. To specify the precision, scale and
offset, the user should add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. When reading grids, the format is generally automatically
recognized. If not, the same suffix can be added to input grid file
names. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20
of the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. The **?**\ *varname* suffix can also be used for output
grids to specify a variable name different from the default: "z". See
`**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20 of the GMT
Technical Reference and Cookbook for more information, particularly on
how to read splices of 3-, 4-, or 5-dimensional grids.

`Geographical And Time Coordinates <#toc7>`_
--------------------------------------------

When the output grid type is netCDF, the coordinates will be labeled
"longitude", "latitude", or "time" based on the attributes of the input
data or grid (if any) or on the **-f** or **-R** options. For example,
both **-f0x** **-f1t** and **-R**\ 90w/90e/0t/3t will result in a
longitude/time grid. When the x, y, or z coordinate is time, it will be
stored in the grid as relative time since epoch as specified by
**TIME\_UNIT** and **TIME\_EPOCH** in the **gmt.conf** file or on the
command line. In addition, the **unit** attribute of the time variable
will indicate both this unit and epoch.

`Examples <#toc8>`_
-------------------

Suppose that north\_pacific\_dbdb5.nc is a file of 5 minute bathymetry
from 140E to 260E and 0N to 50N, and you want to find the medians of
values within a 300km radius (600km full width) of the output points,
which you choose to be from 150E to 250E and 10N to 40N, and you want
the output values every 0.5 degree. To prevent the medians from being
biased by the sloping plane, you want to divide the filter circle into 6
sectors and to choose the lowest value among 6 medians. Using spherical
distance calculations, you need:

**dimfilter** north\_pacific\_dbdb5.nc **-G**\ filtered\_pacific.nc
**-Fm**\ 600 **-D**\ 4 **-N**\ l6 **-R**\ 150/250/10/40 **-I**\ 0.5
**-V**

Suppose that cape\_verde.nc is a file of 0.5 minute bathymetry from 32W
to 15W and 8N to 25N, and you want to remove small-length-scale features
in order to define a swell in an area extending from 27.5W to 20.5W and
12.5N to 19.5N, and you want the output value every 2 minute. Using
cartesian distance calculations, you need:

**dimfilter** cape\_verde.nc **-G**\ t.nc **-Fm**\ 220 \\fB-Nl8
**-D**\ 2 **-R**-27.5/-20.5/12.5/19.5 **-I**\ 2m **-V**
 **grdfilter** t.nc **-G**\ cape\_swell.nc **--Fg**\ 50 **-D**\ 2 **-V**

Suppose that you found a range of filter widths for a given area, and
you filtered the given bathymetric data using the range of filter widths
(e.g., *f100.nc f110.nc f120.nc f130.nc*), and you want to define a
regional trend using the range of filter widths, and you want to obtain
median absolute deviation (MAD) estimates at each data point. Then, you
will need to do:

**grd2xyz** f100.nc **-Z** > f100.d
 **grd2xyz** f110.nc **-Z** > f110.d
 **grd2xyz** f120.nc **-Z** > f120.d
 **grd2xyz** f130.nc **-Z** > f130.d
 **paste** f100.d f110.d f120.d f130.d > depths.d
 **dimfilter** depths.d **-Q**\ 4 > output.z

`Limitations <#toc9>`_
----------------------

When working with geographic (lat, lon) grids, all three convolution
filters (boxcar, cosine arch, and gaussian) will properly normalize the
filter weights for the variation in gridbox size with latitude, and
correctly determine which nodes are needed for the convolution when the
filter "circle" crosses a periodic (0-360) boundary or contains a
geographic pole. However, the spatial filters, such as median and mode
filters, do not use weights and thus should only be used on Cartesian
grids (or at very low latitudes) only. If you want to apply such spatial
filters you should project your data to an equal-area projection and run
dimfilter on the resulting Cartesian grid.

`Script Template <#toc10>`_
---------------------------

The dim.template.sh is a skeleton shell script that can be used to set
up a complete DiM analysis, including the MAD analysis.

`Reference <#toc11>`_
---------------------

Kim, S.-S., and Wessel, P. (2008), Directional Median Filtering for
Regional-Residual Separation of Bathymetry, *Geochem. Geophys.
Geosyst.*, **9**, Q03005, doi:10.1029/2007GC001850.

`See Also <#toc12>`_
--------------------

`*GMT*\ (1) <GMT.html>`_ , `*grdfilter*\ (1) <grdfilter.html>`_
