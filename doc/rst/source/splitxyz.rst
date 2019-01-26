.. index:: ! splitxyz

********
splitxyz
********

.. only:: not man

    Split xyz[dh] data tables into individual segments

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt splitxyz** [ *table* ] 
[ |-A|\ *azimuth*/*tolerance* ]
[ |-C|\ *course_change*]
[ |-D|\ *minimum_distance* ]
[ |-F|\ *xy\_filter*/*z\_filter* ]
[ |-N|\ *template* ]
[ |-Q|\ *flags* ]
[ |-S| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**splitxyz** reads a series of (x,y[,z]) records [or optionally
(x,y,z,d,h); see **-S** option] from standard input [or *xyz[dh]file*]
and splits this into separate lists of (x,y[,z]) series, such that each
series has a nearly constant azimuth through the x,y plane. There are
options to choose only those series which have a certain orientation, to
set a minimum length for series, and to high- or low-pass filter the z
values and/or the x,y values. **splitxyz** is a useful filter between
data extraction and :doc:`wiggle` plotting, and can also be used to
divide a large x,y[,z] dataset into segments. 

Required Arguments
------------------

none.

Optional Arguments
------------------

*table*
    One or more ASCII [or binary, see **-bi**]
    files with 2, 3, or 5 columns holding (x,y,[z[,d,h]])
    data values. To use (x,y,z,d,h) input, sorted so that d is
    non-decreasing, specify the **-S** option; default expects (x,y,z)
    only. If no files are specified, **splitxyz** will read from
    standard input.

.. _-A:

**-A**\ *azimuth*/*tolerance*
    Write out only those segments which are within ±\ *tolerance*
    degrees of *azimuth* in heading, measured clockwise from North, [0 -
    360]. [Default writes all acceptable segments, regardless of
    orientation].

.. _-C:

**-C**\ *course\_change*
    Terminate a segment when a course change exceeding *course\_change*
    degrees of heading is detected [ignore course changes].

.. _-D:

**-D**\ *minimum\_distance*
    Do not write a segment out unless it is at least *minimum\_distance*
    units long [0]

.. _-F:

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

.. _-N:

**-N**\ *template*
    Write each segment to a separate output file [Default writes a
    multiple segment file to stdout]. Append a format template for the
    individual file names; this template **must** contain a C format
    specifier that can format an integer argument (the running segment
    number across all tables); this is usually %d but could be %08d
    which gives leading zeros, etc. [Default is
    splitxyz\_segment\_%d.{txt\|bin}, depending on
    **-bo**]. Alternatively, give a template with
    two C format specifiers and we will supply the table number and the
    segment number within the table to build the file name.

.. _-Q:

**-Q**\ *flags*
    Specify your desired output using any combination of *xyzdh*, in any
    order. Do not space between the letters. Use lower case. The output
    will be ASCII (or binary, see **-bo**)
    columns of values corresponding to *xyzdh* [Default is
    **-Q**\ *xyzdh* (**-Q**\ *xydh* if only 2 input columns)].

.. _-S:

**-S**
    Both d and h are supplied. In this case, input contains x,y,z,d,h.
    [Default expects (x,y,z) input, and d,h are computed from delta x,
    delta y. Use **-fg** to indicate map data; then x,y are assumed to
    be in degrees of longitude, latitude, distances are considered to be
    in kilometers, and angles are actually azimuths. Otherwise,
    distances are Cartesian in same units as x,y and angles are
    counter-clockwise from horizontal].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2, 3, or 5 input columns as set by **-S**]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 1-5 output columns as set by **-Q**]. 
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| replace:: Do not let a segment have a gap exceeding *gap*; instead, split it into two segments. [Default ignores gaps]. 
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Distance Calculations
---------------------

The type of input data is dictated by the **-f** option. If **-fg** is
given then x,y are in degrees of longitude, latitude, distances are in
kilometers, and angles are azimuths. Otherwise, distances are Cartesian
in same units as x,y and angles are counter-clockwise from horizontal.

Examples
--------

Suppose you want to make a wiggle plot of magnetic anomalies on segments
oriented approximately east-west from a NGDC-supplied cruise called JA020015 in the
region **-R**\ 300/315/12/20. You want to use a 100 km low-pass filter to
smooth the tracks and a 500km high-pass filter to detrend the magnetic
anomalies. Try this:

   ::

    gmt mgd77list JA020015 -R300/315/12/20 -Flon,lat,mag,dist,azim | gmt splitxyz -A90/15 -F100/-500 \
        -D100 -S -V -fg | gmt wiggle -R300/315/12/20 -Jm0.6i -Baf -B+tJA020015 -T1 \
        -W0.75p -Ggray -Z200 -pdf JA020015_wiggles

MGD-77 users: For this application we recommend that you extract dist,azim
from :doc:`mgd77list <supplements/mgd77/mgd77list>` rather than have
**splitxyz** compute them separately.

Suppose you have been given a binary, double-precision file containing
lat, lon, gravity values from a survey, and you want to split it into
profiles named *survey*\ \_\ *###.txt* (when gap exceeds 100 km). Try this:

   ::

    gmt splitxyz survey.bin -Nsurvey_%03d.txt -V -gd100k -D100 -: -fg -bi3d

See Also
--------

:doc:`gmt`,
:doc:`filter1d`,
:doc:`mgd77list <supplements/mgd77/mgd77list>`,
:doc:`wiggle`
