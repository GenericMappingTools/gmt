.. index:: ! clip

******
clip
******

.. only:: not man

    Initialize or terminate polygonal clip paths

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt clip** [ *table* ] |-J|\ *parameters* |-C|\ [\ *n*]
|SYN_OPT-Rz|
[ |-A|\ [**m**\ \|\ **p**\ \|\ **x**\ \|\ **y**] ]
[ |SYN_OPT-B| ]
|-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-N| ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**clip** reads (x,y) file(s) [or standard input] and draws polygons
that are activated as clipping paths. Several files may be read to
create complex paths consisting of several non-connecting segments. Only
marks that are subsequently drawn inside the clipping path will be
shown. To determine what is inside or outside the clipping path,
**clip** uses the even-odd rule. When a ray drawn from any point,
regardless of direction, crosses the clipping path segments an odd
number of times, the point is inside the clipping path. If the number is
even, the point is outside. The **-N** option, reverses the sense of
what is the inside and outside of the paths by plotting a clipping path
along the map boundary. After subsequent plotting, which will be clipped
against these paths, the clipping may be deactivated by running
**clip** a second time with the **-C** option only. 

Required Arguments
------------------

.. _-C:

**-C**\ [\|\ *n*]
    Mark end of existing clip path(s). No input file will be processed.
    No projection information is needed unless **-B** has been selected
    as well. With no arguments we terminate all active clipping paths.
    Experts may restrict the termination to just *n* of the active
    clipping path by passing that as the argument.
    Remember to supply
    **-X** and **-Y** settings if you have moved since the clip started.

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

**-A**\ [**m**\ \|\ **p**\ \|\ **x**\ \|\ **y**]
    By default, geographic line segments are connected as great circle arcs. To connect them as
    straight lines, use the **-A** flag. Alternatively, add **m** to connect
    the line by first following a meridian, then a parallel. Or append **p**
    to start following a parallel, then a meridian. (This can be practical
    to connect lines along parallels, for example). 
    For Cartesian data, points are simply connected, unless you append
    **x** or **y** to construct stair-case paths whose first move is along 
    *x* or *y*, respectively.

.. _-B:

.. include:: explain_-B.rst_

.. include:: explain_-Jz.rst_

.. _-N:

**-N**
    Invert the sense of what is inside and outside. For example, when
    using a single path, this means that only points outside that path
    will be shown. Cannot be used together with **-B**. 

.. _-T:

**-T**
    Rather than read any input files, simply turn on clipping for the
    current map region. Basically, **-T** is a convenient way to run
    **clip** with the arguments **-N** /dev/null (or, under Windows,
    **-N** NUL). Cannot be used together with **-B**. 

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-X:

.. include:: explain_-XY.rst_

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

Examples
--------

To set up a complex clip
area to which all subsequent plotting will be confined, run:

   ::

    gmt clip my_region.xy -R0/40/0/40 -Jm0.3i -K > clip_mask_on.ps

To deactivate the clipping in an existing plotfile, run:

   ::

    gmt clip -C -O >> complex_plot.ps

See Also
--------

:doc:`gmt`, :doc:`grdmask`,
:doc:`basemap`, :doc:`mask`
