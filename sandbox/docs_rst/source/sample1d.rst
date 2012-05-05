********
sample1d
********


sample1d - Resample 1-D table data using splines

`Synopsis <#toc1>`_
-------------------

**sample1d** [ *table* ] [ **-A**\ **m**\ \|\ **p** ] [
**-Fl**\ \|\ **a**\ \|\ **c**\ \|\ **n** ] [ **-I**\ *inc*\ [*unit*\ ] ]
[ **-N**\ *knotfile* ] [ **-S**\ *start*\ [/*stop*] ] [ **-T**\ *col* ]
[ **-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ]

`Description <#toc2>`_
----------------------

**sample1d** reads a multi-column ASCII [or binary] data set from file
[or standard input] and interpolates the timeseries/profile at locations
where the user needs the values. The user must provide the column number
of the independent (monotonically increasing **or** decreasing)
variable. Equidistant or arbitrary sampling can be selected. All columns
are resampled based on the new sampling interval. Several interpolation
schemes are available. Extrapolation outside the range of the input data
is not supported.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

None.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    This is one or more ASCII [of binary, see
    **-bi**\ [*ncol*\ ][**t**\ ]] files with one column containing the
    independent variable (which must be monotonically in/de-creasing)
    and the remaining columns holding other data values. If no file is
    provided, **sample1d** reads from standard input.
**-A**\ **m**\ \|\ **p**
    For spherical surface resampling (see **-I**) we resample along
    great circle arcs. Alternatively, use **-Am** to resample by first
    following a meridian, then a parallel. Or use **-Ap** to start
    following a parallel, then a meridian. (This can be practical for
    resampling lines along parallels, for example).
**-Fl**\ \|\ **a**\ \|\ **c**\ \|\ **n**
    Choose from **l** (Linear), **a** (Akima spline), **c** (natural
    cubic spline), and **n** (no interpolation: nearest point) [Default
    is **-Fa**]. You may change the default interpolant; see
    **GMT\_INTERPOLANT** in your **gmt.conf** file.
**-I**\ *inc*\ [*unit*\ ]
    *inc* defines the sampling interval [Default is the separation
    between the first and second abscissa point in the *infile*]. Append
    a distance unit (see UNITS) to indicate that the first two columns
    contain longitude, latitude and you wish to resample this path using
    spherical segments with a nominal spacing of *inc* in the chosen
    units. See **-Am**\ \|\ **p** to only sample along meridians and
    parallels.
**-N**\ *knotfile*
    *knotfile* is an optional ASCII file with the x locations where the
    data set will be resampled in the first column. Note: if **-H** is
    selected it applies to both *infile* and *knotfile*.
**-S**\ *start*
    For equidistant sampling, *start* indicates the location of the
    first output value. [Default is the smallest even multiple of *inc*
    inside the range of *infile*]. Optionally, append /*stop* to
    indicate the location of the last output value [Default is the
    largest even multiple of *inc* inside the range of *infile*].
**-T**\ *col*
    Sets the column number of the independent variable [Default is 0 (first)].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input. [Default is 2 (or at least the number of
    columns implied by **-T**)].
**-bo**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary output. [Default is same as input].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ] (\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] (\*)
    Select input columns.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Units <#toc6>`_
----------------

For map distance units, append *unit* **d** for arc degrees, **m** for
arc minutes, and **s** for arc seconds, or **e** for meters [Default],
**f** for feet, **k** for km, **M** for statute miles, and **n** for
nautical miles. By default we compute such distances using a spherical
approximation with great circles. Prepend **-** to a distance (or the
unit is no distance is given) to perform "Flat Earth" calculations
(quicker but less accurate) or prepend **+** to perform exact geodesic
calculations (slower but more accurate).

`Ascii Format Precision <#toc7>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Calendar Time Sampling <#toc8>`_
---------------------------------

If the abscissa are calendar times then you must use the **-f** option
to indicate this. Furthermore, **-I** then expects an increment in the
current **TIME\_UNIT** units. There is not yet support for variable
intervals such as months.

`Examples <#toc9>`_
-------------------

To resample the file profiles.tdgmb, which contains
(time,distance,gravity,magnetics,bathymetry) records, at 1km equidistant
intervals using Akima’s spline, use

sample1d profiles.tdgmb -I1 -Fa -T1 > profiles\_equi\_d.tdgmb

To resample the file depths.dt at positions listed in the file
grav\_pos.dg, using a cubic spline for the interpolation, use

sample1d depths.dt -Ngrav\_pos.dg -Fc > new\_depths.dt

To resample the file track.txt which contains lon, lat, depth every 2
nautical miles, use

sample1d track.txt -I2n > new\_track.dt

`See Also <#toc10>`_
--------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*gmt.conf*\ (5) <gmt.conf.5.html>`_ ,
`*filter1d*\ (1) <filter1d.1.html>`_

