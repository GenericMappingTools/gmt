.. index:: ! polespotter

***********
polespotter
***********

.. only:: not man

    polespotter - Find rotation poles given FZs and abyssal hills

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**polespotter**
[ |SYN_OPT-I| ]
[ |-G|\ *polegrid* ]
[ |-A|\ *abyssalhills* ]
[ |-D|\ *spacing* ]
[ |-F|\ *fracturezones* ]
[ |-L| ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-W|\ **a**\ \|\ **f**\ *weight* ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**polespotter** reads line segments (abyssal hill fabric lineaments
and/or fracture zone lineaments) and computes great circles for each
that are expected to intersect at potential rotation poles.  The assumption
is that abyssal hill lines are meridians and fracture zones are parallels
with respect to the rotation pole.  Line density may be computed and returned
via a grid, and the great circle lines may be returned via stdout.

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

.. _-F:

**-F**\ *fracturezones*
    File with multiple segments of fracture zone lineaments. These
    are assumed to reflect small circles about the
    rotation pole in effect when the seafloor was formed.

.. _-G:

**-G**\ *CVAgrid*
    Specify name for output polesearch grid file.  We will accumulate
    great circle line density for the grid.  Each bin that is crossed
    by a great circle is incremented by 1, multiplied by cos(latitude),
    the length of the fracture zone or abyssal line segment used to
    define the great circle, and any overall weight set via **-W**.

.. _-I:

.. include:: ../../explain_-I.rst_

.. _-L:

**-L**
    Dump all great circles produced to standard output [no output].

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

.. _-W:

**-W**
    Provide different weighting for the circles defined by **a**\ byssal
    hills versus **f**\ racture zones.  Give **-Wa**\ *weight* to set the
    former [1] and **-Wf**\ *weight* for the latter [1].

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

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: explain_geodetic.rst_

Examples
--------

To create a polespotting image from the abyssal hill and fracture zone fabric
(lon,lat) data in the files hills.txt and fractures.txt, on a 1x1 degree grid
for the norhern hemisphere, sampling the great circles every 10 km, and also
dump the great circles to standard output, try

   ::

    gmt polespotter -Ahills.txt -Ffractures.txt -D10 -Gpoles.nc -R0/360/0/90 -I1 -V > lines.txt

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
:doc:`originator`
