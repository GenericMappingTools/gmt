.. index:: ! pssolar

*******
pssolar
*******

.. only:: not man

    pssolar - Calculate and plot the day-night terminator

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**pssolar** 
[ |SYN_OPT-B| ]
[ |-G|\ *fill* ]
[ |-I|\ [*lon/lat*][**+d**\ *<date>*][**+z**\ *<TZ>*] ]
[ |-J|\ *parameters* ]
[ |-K| ]
[ |-M| ]
[ |-O| ] [|-P| ] [ |-Q| ]
[ |SYN_OPT-R| ]
[ |-T|\ *<dcna>*\ [**+d**\ *<date>*][**+z**\ *<TZ>*]]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]

|No-spaces|

Description
-----------

**pssolar** Calculate and plot the day-night terminator and the civil, nautical and astronomical twilights.


Required Arguments
------------------

There are no required arguments but either **-I** or **-T** must be selected.

Optional Arguments
------------------

.. _-B:

.. include:: explain_-B.rst_

.. _-G:

**-G**\ *fill*
    Select color or pattern for filling of terminators [Default is no fill].

.. _-I:

**-I**\ [*lon/lat*][**+d**\ *<date>*][**+z**\ *<TZ>*]
    Print current sun position as well as Azimuth and Elevation. Append lon/lat to print also the times of
    Sunrise, Sunset, Noon and length of the day.
    Add +d<date> in ISO format, e.g, **+d**\ *2000-04-25*, to compute sun parameters
    for this date. If necessary, append time zone via **+z**\ *<TZ>*.

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. _-K:

.. include:: explain_-K.rst_

.. _-M:

**-M**
    Write terminator(s) as a multisegment ASCII (or binary, see **-b**\ *o*) file to standard output. No plotting occurs.

.. _-O:

.. include:: explain_-O.rst_

.. _-P:

.. include:: explain_-P.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-T:

**-T**\ *<dcna>*\ [**+d**\ *<date>*][**+z**\ *<TZ>*]
    Plot (or dump; see -M) one or more terminators defined via the *dcna* flags. Where:
    **d** means day/night terminator; **c** means civil twilight; **n** means nautical twilight;
    **a** means astronomical twilight.  Add +d<date> in ISO format, e.g, **+d**\ *2000-04-25T12:15:00*
    to know where the day-night was at that date. If necessary, append time zone via **+z**\ *<TZ>*.

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**-**\ \|\ **+**][*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    Set pen attributes for lines or the outline of symbols [Defaults:
    width = default, color = black, style = solid]. A leading **+** will
    use the lookup color (via **-C**) for both symbol fill and outline
    pen color, while a leading **-** will set outline pen color and turn
    off symbol fill.  You can also append one or more line attribute modifiers:
    **+o**\ *offset*\ **u** will start and stop drawing the line the given distance offsets
    from the end point.  Append unit **u** from **c**\ \|\ **i**\ \|\ **p** to
    indicate plot distance on the map or append map distance units instead (see below)
    [Cartesian distances];
    **+s** will draw the line using a *PostScript* Bezier spline [linear spline];
    **+v**\ *vspecs* will place a vector head at the ends of the lines.  You can
    use **+vb** and **+ve** to specify separate vector specs at each end [shared specs].
    See the :ref:`Vec_attributes` for more information.

.. _-X:

.. include:: explain_-XY.rst_

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bo.rst_

.. include:: explain_-c.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-t.rst_


Examples
--------

Print current Sun position and Sunrise, Sunset times at:

   ::

    gmt pssolar -I-7.93/37.079+d2016-02-04T10:01:00

Plot the day-night and civil twilight 

   ::

    gmt pscoast -Rd -W0.1 -JX14cd/0d -Ba -BWSen -Dl -A1000 -P -K > terminator.ps

    gmt pssolar -R -JX -W1 -Tdc -O >> terminator.ps


See Also
--------

:doc:`gmt`, :doc:`pscoast`, :doc:`psxy`
