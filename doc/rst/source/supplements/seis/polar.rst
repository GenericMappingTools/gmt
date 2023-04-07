.. index:: ! polar
.. include:: ../module_supplements_purpose.rst_

*****
polar
*****

|polar_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt polar**
[ *table* ]
|-D|\ *lon/lat*
|-J|\ *parameters*
|SYN_OPT-R|
|-M|\ *size*\ [**+m**\ *mag*]
|-S|\ *<symbol><size>*
[ |SYN_OPT-B| ]
[ |-E|\ *fill* ]
[ |-F|\ *fill* ]
[ |-G|\ *fill* ]
[ |-N| ]
[ |-Q|\ *mode*\ [*args*] ]
[ |-T|\ [**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+o**\ *dx*\[/*dy*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Plot observations from a single earthquake observed at various stations
at different azimuths and distances on the lower hemisphere of the focal
sphere.  The focal sphere is typically plotted at the location of the earthquake,
specified via |-D|. Reads data values from *files* [or standard input].

Parameters are expected to be in the following columns:

    **1**,\ **2**,\ **3**:
        *station-code azimuth take-off-angle*
        (all three columns must contain numerical values)
    **4**:
        polarity:

        - compression can be c,C,u,U,+

        - rarefaction can be d,D,r,R,-

        - not defined is anything else

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. _-D:

**-D**\ *lon/lat*
    Centers the focal sphere at given longitude and latitude point on the map.

.. |Add_-J| replace:: |Add_-J_links|
.. include:: /explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-M:

**-M**\ *size*\ [**+m**\ *mag*]
    Sets the size of the focal sphere to plot polarities in. *size* is in
    default units (unless **c**, **i**, or **p** is appended).
    Optionally append **+m**\ *mag* to specify its magnitude,
    then focal sphere size is *mag* / 5.0 * *size*.

.. _-R:

.. |Add_-Rgeo| replace:: |Add_-R_auto_table|
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-S**\ *<symbol_type><size>*
    Selects *symbol_type* and symbol *size*. Size is in default units (unless
    **c**, **i**, or **p** is appended). Choose symbol type from
    st(*a*)r, (*c*)ircle, (*d*)iamond, (*h*)exagon, (*i*)nverted
    triangle, (*p*)oint, (*s*)quare, (*t*)riangle, (*x*)cross.

Optional Arguments
------------------

.. |Add_-B| replace:: |Add_-B_links|
.. include:: ../../explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-E:

**-E**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Selects filling of symbols for stations in extensive quadrants. Set
    the color [Default is 250]. If **-E**\ *fill* is the same as
    **-F**\ *fill*, use **-Qe** to outline.

.. _-F:

**-F**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Sets background color of the focal sphere. Default is no fill.

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
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
        Outline the focal sphere using *pen* or the default pen (see |-W|).

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

.. _-T:

**-T**\ [**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+o**\ *dx*\[/*dy*]]
    Write station code near symbols.

    Optionally append **+a**\ *angle* to change the text angle;
    **+f**\ *font* to set the font of the text;
    append **+j**\ *justify* to change the text location relative to the symbol;
    append **+o** to offset the text string by *dx*/*dy*.
    [Default to write station code above the symbol; the default font size is 12p]

.. |Add_-U| replace:: |Add_-U_links|
.. include:: ../../explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [**-**\|\ **+**][*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    Set current pen attributes [Default pen is 0.25p,black,solid].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: ../../explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_-qi.rst_
.. include:: ../../explain_-t.rst_

.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: ../../explain_vectors.rst_

.. module_common_ends

Examples
--------

.. include:: ../../oneliner_info.rst_

::

    gmt polar -R239/240/34/35.2 -JM8c -N -Sc0.4 -D239.5/34.5 -M5 -pdf test << END
    #stat azim ih pol
    0481 11 147 c
    6185 247 120 d
    0485 288 114 +
    0490 223 112 -
    0487 212 109 .
    END

Use special format derived from HYPO71 output::

    gmt polar -R239/240/34/35.2 -JM8c -N -Sc0.4 -D239:30E/34:30N -M5 -Qh -pdf test <<END
    #Date Or. time stat azim ih
    910223 1 22 0481 11 147 ipu0
    910223 1 22 6185 247 120 ipd0
    910223 1 22 0485 288 114 epu0
    910223 1 22 0490 223 112 epd0
    910223 1 22 0487 212 109 epu0
    END

.. include:: meca_notes.rst_

See Also
--------

:doc:`meca`,
:doc:`coupe`,
:doc:`gmt </gmt>`, :doc:`basemap </basemap>`, :doc:`plot </plot>`
