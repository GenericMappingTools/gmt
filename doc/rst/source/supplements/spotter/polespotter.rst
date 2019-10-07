.. index:: ! polespotter

***********
polespotter
***********

.. only:: not man

    polespotter - Find rotation poles given FZs and abyssal hills

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt polespotter**
[ |-A|\ *abyssalhills* ]
[ |-D|\ *spacing* ]
[ |-E|\ **a**\ \|\ **f**\ *sigma* ]
[ |-F|\ *fracturezones* ]
[ |-G|\ *grid* ]
[ |SYN_OPT-I| ]
[ |-N| ]
[ |SYN_OPT-R| ]
[ |-S|\ **l**\ \|\ **p**\ \|\ **s**\ [*modifiers*] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**polespotter** reads line segments (abyssal hill fabric lineaments
and/or fracture zone lineaments) and performs one of three types of
scans.  In spot mode it computes great circles for each
individual segment along these lines.  The bisector great circles to
fracture zones and the great circle extensions of abyssal hills
are expected to intersect at potential rotation poles.  The assumption
is that abyssal hill lines are meridians and fracture zones are parallels
with respect to the rotation pole.  Line density may be computed and returned
via a grid, the great circle lines may be returned via stdout, and the
intersections of the great circles may be saved to file.  In line mode
it will determine which line segments are compatible with a given trial
pole, while in pole mode it will compute chi-squared misfits for all the
poles defined by the grid.

Optional Arguments
------------------

.. _-A:

**-A**\ *abyssalhills*
    File with multiple segments of abyssal hill lineaments. These
    are assumed to reflect the great circle direction towards the
    rotation pole in effect when the seafloor was formed.

.. _-D:

**-D**\ *step*
    Sets the line increment for all great circles produced, in km [5].
    Actual spacing will be adjusted to give an integer number of steps
    along the full circle.

.. _-E:

**-E**
    Provide different 1-sigma angular uncertainty (in degrees) in the orientation
    of **a**\ byssal hills or **f**\ racture zones.  Give **-Ea**\ *sigma* to set the
    former [1] and **-Ef**\ *sigma* for the latter [1].  These *sigma* values are
    then used to form weights = 1/*sigma*.

.. _-F:

**-F**\ *fracturezones*
    File with multiple segments of fracture zone lineaments. These
    are assumed to reflect small circles about the
    rotation pole in effect when the seafloor was formed.

.. _-G:

**-G**\ *grid*
    Specify name for output grid.  For spot mode we will accumulate
    great circle line density for the grid.  Each bin that is crossed
    by a great circle is incremented by 1, multiplied by cos(latitude),
    the length of the fracture zone or abyssal line segment used to
    define the great circle, and any overall weight set via **-E**.
    In pole mode we return the chi-squared misfit surface.  Not used
    in line mode.

.. _-I:

.. include:: ../../explain_-I.rst_

.. _-N:

**-N**
    Normalize the grid so max value equals 1 [no normalization].

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-Sl**\ \|\ **p**\ \|\ **s**\ [*modifiers*]
    Set the scan mode for this run.  Choose from **l**\ ines, **p**\ oles, or
    **s**\ pots.  Depending on the mode there may be required and optional arguments.

**-Sl**\ *plon*\ /*plat*\ [**+m**\ ]

    Line mode means we accept a *plon*\ /*plat* trial pole location
    and determine how compatible each data segment is with the predictions of
    small circles (for fracture zones) and meridians (for abyssal hills).  By
    default we report summary statistics (*chi2*, *table*, *segment*, *type*)
    for each line segment. Append **+m** to instead report the misfit information
    (*mlon*, *mlat*, *del_angle*, *chi2*, *table*, *segment*, *type*) for each
    mid-point along all multi-point line segments.  The information is written 
    to standard output.

**-Sp**
    Pole mode means we search for all poles on the given grid and determine the
    weighted chi-square misfit to all given line constraints.  This mode requires
    **-G**, **-R**, **-I** (and optionally **-r**).

**-Ss**\ [**+c**\ *xfile*][**+l**]
    Spot mode means we compute bisectors to fracture zones and meridians along abyssal hills
    and determine intersections of all these great circles.  You can append any
    of two modifiers: **+l** will dump all great circles produced to standard
    output [no output], and **+c**\ *xfile* will compute the intersections of
    all great circles and write the locations
    to *xfile*.  This output has 5 columns: *lon*, *lat*, *weight*, *cos*, *type*,
    where *weight* is the combined length weight from the two generating
    line segments, *cos* is the cosine of the angle between the intersecting
    lines, and *type* is either 0 (AH intersect AH), 1 (AH intersect FZ), or
    2 (FZ intersect FZ), where AH means an abyssal hill great circle and FZ
    means a bisector great circle to a fracture zone.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. |Add_-bi| replace:: [Default is 5 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_../../explain_-V.rst_

.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_-ocols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: explain_geodetic.rst_

Notes
-----

#. Each input line is expected to contain two or more points, and each consecutive
   pairs of points define a great circle line segment.  For fracture zones,
   these points should be digitized often enough so that the great circle between
   then can approximate the small circle.
#. All line segments are given equal angular uncertainty [1, unless changed by **-E**\ ].  However,
   individual line segments can override this weight by adding a **-D**\ *sigma*
   argument in the segment headers (in degrees).

Examples
--------

To create a polespotting image from the abyssal hill and fracture zone fabric
(lon,lat) data in the files hills.txt and fractures.txt, on a 1x1 degree grid
for the northern hemisphere, sampling the great circles every 10 km, and also
dump the great circles to standard output, try

   ::

    gmt polespotter -Ahills.txt -Ffractures.txt -D10 -Gpoles.nc -R0/360/0/90 -I1 -V -Ss+l > lines.txt

This file can then be plotted with :doc:`grdimage </grdimage>`.

See Also
--------

:doc:`gmt </gmt>`, :doc:`grdimage </grdimage>`,
:doc:`grdrotater`,
:doc:`grdspotter`,
:doc:`project </project>`,
:doc:`mapproject </mapproject>`,
:doc:`backtracker`,
:doc:`gmtpmodeler`,
:doc:`gmtvector </gmtvector>`,
:doc:`grdpmodeler`,
:doc:`grdrotater`,
:doc:`originater`
