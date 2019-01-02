.. index:: ! polar

*****
polar
*****

.. only:: not man

    Plot polarities on the inferior focal half-sphere on maps

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt polar** [ *table* ] |-D|\ *lon/lat* |-J|\ *parameters*
|SYN_OPT-R|
|-M|\ *size*
|-S|\ *<symbol><size>*
[ |SYN_OPT-B| ]
[ |-C|\ *lon*/*lat*\ [**+p**\ *pen*\ ][**+s**\ /*pointsize*] ]
[ |-E|\ *color* ]
[ |-F|\ *color* ]
[ |-G|\ *color* ]
[ |-L| ] [ |-N| ]
[ |-Q|\ *mode*\ [*args*] ]
[ |-T|\ *angle*/*form*/*justify*/*fontsize* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**polar** reads data values from *files* [or standard input] and
will plot stations on focal mechanisms
on a map.

Parameters are expected to be in the following columns:

    **1**,\ **2**,\ **3**:
        station\_code, azimuth, take-off angle
    **4**:
        polarity:

        - compression can be c,C,u,U,+

        - rarefaction can be d,D,r,R,-

        - not defined is anything else

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-J.rst_

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

.. _-D:

**-D**\ *longitude/latitude*
    Maps the bubble at given longitude and latitude point.

.. _-M:

**-M**\ *size*
    Sets the size of the beach ball to plot polarities in. *Size* is in
    default units (unless **c**, **i**, or **p** is appended).

.. _-S:

**-S**\ *<symbol_type><size>*
    Selects *symbol_type* and symbol *size*. Size is in default inits (unless
    **c**, **i**, or **p** is appended). Choose symbol type from
    st(*a*)r, (*c*)ircle, (*d*)iamond, (*h*)exagon, (*i*)nverted
    triangle, (*p*)oint, (*s*)quare, (*t*)riangle, (*x*)cross.

Optional Arguments
------------------

.. _-B:

.. include:: ../../explain_-B.rst_

.. _-C:

**-C**\ *lon*/*lat*\ [**+p**\ *pen*\ ][**+s**\ /*pointsize*]
    Offsets focal mechanisms to the latitude and longitude specified in
    the last two columns of the input file.  Optionally set the pen and
    symbol point size.

.. _-E:

**-E**\ *color*
    Selects filling of symbols for stations in extensive quadrants. Set
    the color [Default is 250]. If **-E**\ *color* is the same as
    **-F**\ *color*, use **-Qe** to outline.

.. _-F:

**-F**\ *color*
    Sets background color of the beach ball. Default is no fill.

.. _-G:

**-G**\ *color*
    Selects filling of symbols for stations in compressional quadrants.
    Set the color [Default is black].

.. _-N:

**-N**
    Does **not** skip symbols that fall outside map border [Default plots points inside border only].

.. _-Q:

**-Q**\ *mode*\ [*args*]
    Sets one or more attributes; repeatable. The various combinations are

    **-Qe**\ [pen]
        Outline symbols in extensive quadrants using *pen* or the default pen (see |-W|).

    **-Qf**\ [pen]
        Outline the beach ball using *pen* or the default pen (see |-W|).

    **-Qg**\ [pen]
        Outline symbols in compressional quadrants using *pen* or the default pen (see |-W|).

    **-Qh**
        Use special format derived from HYPO71 output

    **-Qs**\ *half-size*\ [**+v**\ *v_size*\ [*vecspecs*]]
        Plots S polarity azimuth. S polarity is in last column. Append **+v** to select a vector
        and append head size and any vector specifications.  If **+v** is given without arguments then we
	default to **+v**\ 0.3i+e+gblack [Default is a line segment].
        Give *half-size* in default units (unless **c**, **i**, or **p** is appended).
        See `Vector Attributes`_ for specifying additional attributes.

    **-Qt**\ *pen*
        Set pen color to write station code. Default uses the default pen (see |-W|).

.. _-Y:

**-T**\ *angle*/*form*/*justify*/*fontsize*
    To write station code; *fontsize* must be given in points [Default is 0.0/0/5/12].

.. _-U:

.. include:: ../../explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**\ [**-**\ \|\ **+**][*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    Set current pen attributes [Defaults: width = default, color = black, style = solid].

.. _-X:

.. include:: ../../explain_-XY.rst_
.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_-t.rst_

.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: ../../explain_vectors.rst_

Examples
--------

   ::

    gmt polar -R239/240/34/35.2 -JM8c -N -Sc0.4 -h1 -D39.5/34.5 -M5 -pdf test << END
    #stat azim ih pol
    0481 11 147 c
    6185 247 120 d
    0485 288 114 +
    0490 223 112 -
    0487 212 109 .
    END

or

   ::

    gmt polar -R239/240/34/35.2 -JM8c -N -Sc0.4 -h1 -D239.5/34.5 -M5 -pdf test <<END
    #Date Or. time stat azim ih
    910223 1 22 0481 11 147 ipu0
    910223 1 22 6185 247 120 ipd0
    910223 1 22 0485 288 114 epu0
    910223 1 22 0490 223 112 epd0
    910223 1 22 0487 212 109 epu0
    END

See Also
--------

:doc:`meca`,
:doc:`velo`,
:doc:`coupe`,
:doc:`gmt </gmt>`, :doc:`basemap </basemap>`, :doc:`plot </plot>`

References
----------

Bomford, G., Geodesy, 4th ed., Oxford University Press, 1980.

Aki, K. and P. Richards, Quantitative Seismology, Freeman, 1980.

Authors
-------

Genevieve Patau, `Laboratory of Seismogenesis <http://www.ipgp.fr/rech/sismogenese/>`,
Institut de Physique du Globe de Paris, Departement de Sismologie, Paris, France
