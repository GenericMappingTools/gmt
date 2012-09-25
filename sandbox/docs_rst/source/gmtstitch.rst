*********
gmtstitch
*********

gmtstitch - Join individual lines whose end points match within
tolerance

`Synopsis <#toc1>`_
-------------------

**gmtstitch** [ *table* ] [ **-C**\ [*closed*\ ] ] [
**-D**\ [*template*\ ] ] [ **-L**\ [*linkfile*\ ] ] [
**-Q**\ [*template*\ ] ] [ **-T**\ *cutoff*\ [*unit*\ ][/\ *nn\_dist*] ]
[ **-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-bo**\ [*ncols*\ ][*type*\ ] ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo*
] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**gmtstitch** reads standard input or one or more data files, which may
be multisegment files, and examines the coordinates of the end points of
all line segments. If a pair of end points are identical or closer to
each other than the specified separation tolerance then the two line
segments are joined into a single segment. The process repeats until all
the remaining endpoints no longer pass the tolerance test; the resulting
segments are then written out to standard output or specified output
file. If it is not clear what the separation tolerance should be then
use **-L** to get a list of all separation distances and analyze them to
determine a suitable cutoff.

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
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-C**\ [*closed*\ ]
    Write all the closed polygons to *closed* [gmtstitch\_closed.txt]
    and all other segments as they are to stdout. No stitching takes
    place. Use **-T**\ *cutoff* to set a minimum separation [0], and if
    *cutoff* is > 0 then we also explicitly close the polygons on
    output.
**-D**\ [*template*\ ]
    For multiple segment data, dump each segment to a separate output
    file [Default writes a single multiple segment file]. Append a
    format template for the individual file names; this template
    **must** contain a C format specifier that can format an integer
    argument (the segment number); this is usually %d but could be %08d
    which gives leading zeros, etc. Optionally, it may also contain the
    format %c *before* the integer; this will then be replaced by C
    (closed) or O (open) to indicate segment type. [Default is
    gmtstitch\_segment\_%d.txt]. Note that segment headers will be
    written in either case. For composite segments, a generic segment
    header will be written and the segment headers of individual pieces
    will be written out as comments to make it possible to identify
    where the stitched pieces came from.
**-L**\ [*linkfile*\ ]
    Writes the link information to the specified file
    [gmtstitch\_link.txt]. For each segment we write the original
    segment id, and for the beginning and end point of the segment we
    report the id of the closest segment, whether it is the beginning
    (B) or end (E) point that is closest, and the distance between those
    points in units determined by **-T**.
**-Q**\ [*template*\ ]
    Used with **-D** to a list file with the names of the individual
    output files. Optionally, append a filename template for the
    individual file names; this template **may** contain a C format
    specifier that can format an character (C or O for closed or open,
    respectively). [Default is gmtstitch\_list.txt].
**-T**\ *cutoff*\ [*unit*\ ][/\ *nn\_dist*]
    Specifies the separation tolerance in the data coordinate units [0];
    append distance unit (see UNITS). If two lines has end-points that
    are closer than this cutoff they will be joined. Optionally, append
    /*nn\_dist* which adds the requirement that a link will only be made
    if the second closest connection exceeds the *nn\_dist*. The latter
    distance must be given in the same units as *cutoff*.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
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

`Examples <#toc8>`_
-------------------

To combine the digitized segment lines segment\_\*.txt (whose
coordinates are in cm) into as few complete lines as possible, assuming
the end points slop could be up to 0.1 mm, run

gmtstitch segment\_\*.txt -Tf0.1 > new\_segments.txt

To combine the digitized segments in the multisegment file my\_lines.txt
(whose coordinates are in lon,lat) into as few complete lines as
possible, assuming the end points slop could be up to 150 m, and write
the complete segments to separate files called Map\_segment\_0001.dat,
Map\_segment\_0002.dat, etc., run

gmtstitch my\_lines.txt -T150e -DMap\_segment\_%04d.dat

`Bugs <#toc9>`_
---------------

The line connection does not work if a line only has a single point.
However, gmtstitch will correctly add the point to the nearest segment.
Running gmtstitch again on the new set of lines will eventually connect
all close lines.

`See Also <#toc10>`_
--------------------

`*gmt*\ (1) <gmt.html>`_ , `*mapproject*\ (1) <mapproject.html>`_
