.. index:: ! clip
.. include:: module_core_purpose.rst_

******
clip
******

|clip_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt clip** [ *table* ] |-J|\ *parameters* |-C|\ [*n*]
|SYN_OPT-Rz|
[ |-A|\ [**m**\|\ **p**\|\ **x**\|\ **y**\|\ **r**\|\ **t**] ]
[ |SYN_OPT-B| ]
|-J|\ **z**\|\ **Z**\ *parameters* ]
[ |-N| ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*] ]
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
[ |SYN_OPT-qi| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads (x,y) file(s) [or standard input] and draws polygons
that are activated as clipping paths. Several files may be read to
create complex paths consisting of several non-connecting segments. Only
marks that are subsequently drawn inside the clipping path will be
shown. To determine what is inside or outside the clipping path,
we use the even-odd rule. When a ray drawn from any point,
regardless of direction, crosses the clipping path segments an odd
number of times, the point is inside the clipping path. If the number is
even, the point is outside. The **-N** option, reverses the sense of
what is the inside and outside of the paths by plotting a clipping path
along the map boundary. After subsequent plotting, which will be clipped
against these paths, the clipping may be deactivated by running
the module a second time with the **-C** option only.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-C:

**-C**\ [*n*]
    Mark end of existing clip path(s). No input file will be processed.
    No projection information is needed unless **-B** has been selected
    as well. With no arguments we terminate all active clipping paths.
    Experts may restrict the termination to just *n* of the active
    clipping path by passing that as the argument.

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-R| replace:: |Add_-R_auto_table|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [**m**\|\ **p**\|\ **x**\|\ **y**\|\ **r**\|\ **t**]
    By default, geographic line segments are connected as great circle arcs. To connect them as
    straight lines, use the **-A** flag. Alternatively, add **m** to connect
    the line by first following a meridian, then a parallel. Or append **p**
    to start following a parallel, then a meridian. (This can be practical
    to connect lines along parallels, for example).
    For Cartesian data, points are simply connected, unless you append
    **x** or **y** to construct stair-case paths whose first move is along
    *x* or *y*, respectively. For polar projection, append **r** or **t** to
    draw stair-case curves that whose first move is along *r* or *theta*, respectively.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-N:

**-N**
    Invert the sense of what is inside and outside. For example, when
    using a single path, this means that only points outside that path
    will be shown. Cannot be used together with **-B**.

.. _-T:

**-T**
    Rather than read any input files, simply turn on clipping for the
    current map region. Basically, **-T** is a convenient way to run
    the module with the arguments **-N** /dev/null (or, under Windows,
    **-N** NUL). Cannot be used together with **-B**.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *pen*
    Draw outline of clip path using given pen attributes before clipping is initiated [Default is no outline].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

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

.. include:: explain_-qi.rst_

.. include:: explain_-t.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. module_common_ends

Examples
--------

To see the effect of a simple clip path which result in some symbols
being partly visible or missing altogether, try::

    gmt begin clip
      gmt clip -R0/6/0/6 -Jx2.5c -W1p,blue << EOF
    0 0
    5 1
    5 5
    EOF
      gmt plot @tut_data.txt -Gred -Sc2c
      gmt clip -C -B
    gmt end show

where we activate and deactivate the clip path.  Note we also draw the
outline of the clip path to make it clear what is being clipped.

See Also
--------

:doc:`gmt`, :doc:`grdmask`,
:doc:`basemap`, :doc:`mask`
