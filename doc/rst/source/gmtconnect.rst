.. index:: ! gmtconnect
.. include:: module_core_purpose.rst_

*******
connect
*******

|gmtconnect_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt connect** [ *table* ] [ |-C|\ [*closed*] ]
[ |-D|\ [*template*] ] [ |-L|\ [*linkfile*] ]
[ |-Q|\ [*template*] ] [ |-T|\ [*cutoff*\ [**+s**\ *sdist*]] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**connect** reads standard input or one or more data files, which may
be multisegment files, and examines the coordinates of the end points of
all line segments. If a pair of end points are identical or closer to
each other than the specified separation *cutoff* then the two line
segments are joined into a single segment. The process repeats until all
the remaining endpoints no longer pass the tolerance test; the resulting
segments are then written out to standard output or specified output
file. If it is not clear what the separation tolerance should be then
use |-L| to get a list of all separation distances and analyze them to
determine a suitable cutoff.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

Optional Arguments
------------------

.. _-C:

**-C**\ [*closed*]
    Write all the already-closed polygons to file *closed* [gmtconnect_closed.txt]
    and all open segments to standard output. No connection will take
    place. Use **-T**\ *cutoff* to set a minimum separation [0], and then
    any existing polygon whose first and last point are separated by less
    that *cutoff* will be considered to be closed.  Note that if
    *cutoff* is > 0 then we explicitly close all polygons on output.

.. _-D:

**-D**\ [*template*]
    For multiple segment data, dump each segment to a separate output
    file [Default writes a single multiple segment file]. Append a
    format template for the individual file names; this template
    **must** contain a C language `printf <https://en.wikipedia.org/wiki/Printf_format_string>`_
    format specifier that can format an integer
    argument (the segment number); this is usually %d but could be %08d
    for leading zeros, etc. Optionally, it may also contain the
    format %c *before* the integer; this will then be replaced by C
    (closed) or O (open) to indicate segment type. [Default is
    gmtconnect_segment_%d.txt]. Note that segment headers will be
    written in either case. For composite segments, a generic segment
    header will be written and the segment headers of individual pieces
    will be written out as comments to make it possible to identify
    where the connected pieces came from.

.. _-L:

**-L**\ [*linkfile*]
    Writes link information to the specified file
    [gmtconnect_link.txt]. For each segment we write the original
    segment id, and for the beginning and end point of the segment we
    report the id of the nearest segment, whether it is the beginning
    (B) or end (E) point that is closest, and the distance between those
    points in units determined by |-T|.

.. _-Q:

**-Q**\ [*template*]
    Used with |-D| to write a list file with the names of the individual
    output files. Optionally, append a filename template for the
    individual file names; this template **may** contain a C format
    specifier that can format an character (C or O for closed or open,
    respectively). [Default is gmtconnect_list.txt].

.. _-T:

**-T**\ [*cutoff*\ [**+s**\ *sdist*]]
    Specifies the separation tolerance in the data coordinate units [0]; for geographic data
    append distance unit (see `Units`_). If two lines have end-points that
    are closer than *cutoff* they will be joined. Optionally, append
    **+s**\ *sdist* which adds the extra requirement that a link will only be made
    if the *second* closest connection exceeds the *sdist* distance. The latter
    distance must be given in the same units as *cutoff*.  However, if |-T| is
    no arguments then we close every line segment or polygon *regardless* of
    the gap distance between the first and last point in the segment.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: explain_-aspatial.rst_

.. |Add_-bi| replace:: [Default is 2 input columns].
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

.. include:: explain_-ocols.rst_

.. include:: explain_-q.rst_

.. include:: explain_-s.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

Examples
--------

.. include:: explain_example.rst_

To combine the digitized segment lines segment\_\*.txt (whose Cartesian
coordinates are in cm) into as few complete lines as possible, assuming
the end points' digitization error could be up to 0.1 mm, run::

    gmt connect segment_*.txt -T0.1 > new_segments.txt

To combine the digitized segments in the multisegment file my_lines.txt
(whose coordinates are in *lon, lat*) into as few complete lines as
possible, assuming the end points digitization error could be up to 150 m, and write
the complete segments to individual files called Map_segment_0001.txt,
Map_segment_0002.txt, etc., run::

    gmt connect my_lines.txt -T150e -DMap_segment_%04d.txt

Bugs
----

The line connection does not work if a line only has a single point.
However, **connect** will correctly add the point to the nearest segment.
Running **connect** again on the new set of lines will eventually connect
all lines that satisfy the criteria given in |-T|.

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`gmtsimplify`,
:doc:`gmtspatial`,
:doc:`mapproject`
