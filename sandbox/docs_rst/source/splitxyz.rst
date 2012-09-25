********
splitxyz
********

splitxyz - Split xyz[dh] data tables into individual segments

`Synopsis <#toc1>`_
-------------------

**splitxyz** [ *table* ] **-C**\ *course\_change* [
**-A**\ *azimuth*/*tolerance* ] [ **-D**\ *minimum\_distance* ] [
**-F**\ *xy\_filter*/*z\_filter* ] [ **-N**\ *template* ] [
**-Q**\ *flags* ] [ **-S** ] [ **-V**\ [*level*\ ] ] [ **-Z** ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**splitxyz** reads a series of (x,y[,z]) records [or optionally
(x,y,z,d,h); see **-S** option] from standard input [or *xyz[dh]file*]
and splits this into separate lists of (x,y[,z]) series, such that each
series has a nearly constant azimuth through the x,y plane. There are
options to choose only those series which have a certain orientation, to
set a minimum length for series, and to high- or low-pass filter the z
values and/or the x,y values. **splitxyz** is a useful filter between
data extraction and **pswiggle** plotting, and can also be used to
divide a large x,y,z dataset into segments. The output is always in the
ASCII format; input may be ASCII or binary (see
**-bi**\ [*ncols*\ ][*type*\ ]).

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-C**\ *course\_change*
    Terminate a segment when a course change exceeding *course\_change*
    degrees of heading is detected.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII [or binary, see **-bi**\ [*ncols*\ ][*type*\ ]]
    files with 3 (or 2, see **-Z**) [or 5] columns holding (x,y,z[,d,h])
    data values. To use (x,y,z,d,h) input, sorted so that d is
    non-decreasing, specify the **-S** option; default expects (x,y,z)
    only. If no files are specified, **splitxyz** will read from
    standard input.
**-A**\ *azimuth*/*tolerance*
    Write out only those segments which are within +/- *tolerance*
    degrees of *azimuth* in heading, measured clockwise from North, [0 -
    360]. [Default writes all acceptable segments, regardless of
    orientation].
**-D**\ *minimum\_distance*
    Do not write a segment out unless it is at least *minimum\_distance*
    units long [0]
**-F**\ *xy\_filter*/*z\_filter*
    Filter the z values and/or the x,y values, assuming these are
    functions of d coordinate. *xy\_filter* and *z\_filter* are filter
    widths in distance units. If a filter width is zero, the filtering
    is not performed. The absolute value of the width is the full width
    of a cosine-arch low-pass filter. If the width is positive, the data
    are low-pass filtered; if negative, the data are high-pass filtered
    by subtracting the low-pass value from the observed value. If
    *z\_filter* is non-zero, the entire series of input z values is
    filtered before any segmentation is performed, so that the only edge
    effects in the filtering will happen at the beginning and end of the
    complete data stream. If *xy\_filter* is non-zero, the data is first
    divided into segments and then the x,y values of each segment are
    filtered separately. This may introduce edge effects at the ends of
    each segment, but prevents a low-pass x,y filter from rounding off
    the corners of track segments. [Default = no filtering].
**-N**\ *template*
    Write each segment to a separate output file [Default writes a
    multiple segment file to stdout]. Append a format template for the
    individual file names; this template **must** contain a C format
    specifier that can format an integer argument (the running segment
    number across all tables); this is usually %d but could be %08d
    which gives leading zeros, etc. [Default is
    splitxyz\_segment\_%d.{txt\|bin}, depending on
    **-bo**\ [*ncols*\ ][*type*\ ]]. Alternatively, give a template with
    two C format specifiers and we will supply the table number and the
    segment number within the table to build the file name.
**-Q**\ *flags*
    Specify your desired output using any combination of *xyzdh*, in any
    order. Do not space between the letters. Use lower case. The output
    will be ASCII (or binary, see **-bo**\ [*ncols*\ ][*type*\ ])
    columns of values corresponding to *xyzdh* [Default is
    **-Q**\ *xyzdh* (**-Q**\ *xydh* if **-Z** is set)].
**-S**
    Both d and h are supplied. In this case, input contains x,y,z,d,h.
    [Default expects (x,y,z) input, and d,h are computed from delta x,
    delta y. Use **-fg** to indicate map data; then x,y are assumed to
    be in degrees of longitude, latitude, distances are considered to be
    in kilometers, and angles are actually azimuths. Otherwise,
    distances are Cartesian in same units as x,y and angles are
    counter-clockwise from horizontal].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-Z**
    Data have x,y only (no z-column).
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2, 3, or 5 input columns as set by
    **-S**, **-Z**].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is 1-5 output columns as set by
    **-Q**].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
    Do not let a segment have a gap exceeding *gap*; instead, split it
    into two segments. [Default ignores gaps].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s). Not used with binary data.
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
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

`Distance Calculations <#toc7>`_
--------------------------------

The type of input data is dictated by the **-f** option. If **-fg** is
given then x,y are in degrees of longitude, latitude, distances are in
kilometers, and angles are azimuths. Otherwise, distances are Cartesian
in same units as x,y and angles are counter-clockwise from horizontal.

`Examples <#toc8>`_
-------------------

Suppose you want to make a wiggle plot of magnetic anomalies on segments
oriented approximately east-west from a cruise called cag71 in the
region **-R**\ 300/315/12/20. You want to use a 100km low-pass filter to
smooth the tracks and a 500km high-pass filter to detrend the magnetic
anomalies. Try this:

gmtlist cag71 -R300/315/12/20 -Fxyzdh \| splitxyz -A90/15 -F100/-500
-D100 -S -V -fg \| pswiggle -R300/315/12/20 -Jm0.6 -Ba5f1:.cag71: -T1
-W0.75p -Ggray -Z200 > cag71\_wiggles.ps

MGD-77 users: For this application we recommend that you extract d, h
from **mgd77list** rather than have **splitxyz** compute them
separately.

Suppose you have been given a binary, double-precision file containing
lat, lon, gravity values from a survey, and you want to split it into
profiles named *survey*\ \_\ *###.txt* (when gap exceeds 100 km). Try
this:

splitxyz survey.bin -Nsurvey\_%03d.txt -V -gd100k -D100 -: -fg -bi3d

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*mgd77list*\ (1) <mgd77list.html>`_ ,
`*pswiggle*\ (1) <pswiggle.html>`_
