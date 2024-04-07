.. index:: ! barb
.. include:: ../module_supplements_purpose.rst_

****
barb
****

.. only:: not man

|barb_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**barb** [ *table* ]
|-J|\ *parameters* |-J|\ **z**\ \|\ **Z**\ *parameters*
|SYN_OPT-Rz|
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *dx*/*dy*\ [/*dz*] ]
[ |-G|\ *fill* ]
[ |-I|\ *intens* ]
[ |-N| ]
[ |-Q|\ *length*\ [**+a**\ *angle*][**+g**\ -\|\ *fill*][**+jb**\|\ **c**\|\ **e**][**+p**\ -\|\ *pen*][**+s**\ *scale*][**+w**\ *width*][**+z**] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][*attr*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]

|No-spaces|

.. module_common_begins

Description
-----------

**barb** reads (x, y, azimuth, speed) from *files* [or standard input]
and generates PostScript code that will plot wind barbs at those locations
on a map. If **-JZ|z** is set, then **barb** will interpret the third
column of the input data as z-values and plot wind barbs in 3-D.
If the wind barb length is not given with |-Q|, then **barb** will
interpret the third and fourth columns of the input data as barb length and
width, respectively.
Select a fill with |-G|. If |-G| is set, |-W| will control
whether the outline is drawn or not.

Required Arguments
------------------

.. _-J:

.. |Add_-J| replace:: |Add_-J_links|
.. include:: /explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: ../../explain_-Jz.rst_

.. _-R:

.. |Add_-Rgeo| replace:: |Add_-R_auto_table|
.. include:: ../../explain_-Rgeo.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rz.rst_

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. |Add_-B| replace:: |Add_-B_links|
.. include:: ../../explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *cpt*
    Give a CPT or specify **-C**\ *color1,color2*\ [*,color3*\ ,...]
    to build a linear continuous CPT from those colors automatically.
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ). Let fill color be
    determined by the t-value in the third (or fourth if **-Jz** is
    set) column. Additional fields are shifted over by one column
    (wind barb azimuth would be in 4th rather than 5th field, etc.).

.. _-D:

**-D**\ *dx*/*dy*\ [/*dz*]
    Offset the plot symbol or line locations by the given amounts
    *dx/dy*\ [*dz*\ ] [Default is no offset].

.. _-G:

**-G**\ *fill*
    Select color or pattern for filling of symbols or polygons [Default is no fill].
    Note that **barb** will search for |-G| and |-W| strings in all the
    segment headers and let any values thus found over-ride the command line settings.

.. _-I:

**-I**\ *intens*
    Use the supplied *intens* value (nominally in the ±1 range) to
    modulate the fill color by simulating illumination [none].

.. _-N:

**-N**\ [**c**\ \|\ **r**]
    Do NOT clip symbols that fall outside map border [Default plots points
    whose coordinates are strictly inside the map border only]. The option does not apply to lines and polygons
    which are always clipped to the map region. For periodic (360-longitude)
    maps we must plot all symbols twice in case they are clipped by the repeating
    boundary. The |-N| will turn off clipping and not plot repeating symbols.
    Use **-Nr** to turn off clipping but retain the plotting of such repeating symbols, or
    use **-Nc** to retain clipping but turn off plotting of repeating symbols.

.. _-Q:

**-Q**\ *length*\ [**+a**\ *angle*][**+g**\ -\|\ *fill*][**+jb**\|\ **c**\|\ **e**][**+p**\ -\|\ *pen*][**+s**\ *scale*][**+w**\ *width*][**+z**]
    Modify wind barb parameters. Append wind barb *length* [Default is 0.5c].
    Several modifiers may be appended to specify the placement of barbs, their shapes, and the
    justification of the wind barb. Below, left and right refers to the
    side of the wind barb line when viewed from the start point to the
    end point of the segment. Chose among these modifiers:

    - **+a** - Set the angle of the wind barb [120].
    - **+g** - Turn off *fill* (if -) or set the wind
      barb fill [Default fill is used, which may be no fill].
    - **+p** - Sets the wind barb pen attributes. If *pen* has a
      leading - then the outline is not drawn [Default pen is used, and
      outline is drawn].
    - **+j** - Determines how the input *x*,\ *y* point relates to the
      wind barb. Choose from **b**\ eginning [default], **e**\ nd, or **c**\ enter.
    - **+s** - Set the wind speed which corresponds to a long barb [default 5].
    - **+w** - Set the *width* of wind barbs.
    - **+z** - Input (u,v) wind components instead of (azimuth,speed).

.. |Add_-U| replace:: |Add_-U_links|
.. include:: ../../explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    Set pen attributes for lines or the outline of symbols [Defaults:
    width = default, color = black, style = solid]. If the modifier **+cl**
    is appended then the color of the line are taken from the CPT (see
    **-C**). If instead modifier **+cf** is appended then the color from the cpt
    file is applied to symbol fill.  Use just **+c** for both effects.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: ../../explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: ../../explain_-aspatial.rst_

.. |Add_-bi| replace:: [Default is the required number of columns given the chosen settings].
.. include:: ../../explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-t.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. module_common_ends

Examples
--------

.. include:: ../../explain_example.rst_

.. include:: ../../oneliner_info.rst_

To plot blue columns (width = 1.25 cm) at the positions listed in the
file heights.xyz on a 3-D projection of the space (0-10), (0-10),
(0-100), with tickmarks every 2, 2, and 10, viewing it from the
southeast at 30 degree elevation, use::

 gmt barb heights.xyz -R0/10/0/10/0/100 -Jx1.25c -Jz0.125c -So1.25c -Gblue \
          -Bx2+lXLABEL -By2+lYLABEL -Bz10+lZLABEL -B+t"3-D PLOT" -p135/30 -Uc -W -pdf heights

Segment Header Parsing
----------------------

Segment header records may contain one of more of the following options:

**-G**\ *fill*
    Use the new *fill* and turn filling on.
**-G-**
    Turn filling off.
**-G**
    Revert to default fill (none if not set on command line).
**-W**\ *pen*
    Use the new *pen* and turn outline on.
**-W**
    Revert to default pen :term:`MAP_DEFAULT_PEN <MAP_DEFAULT_PEN>`
    (if not set on command line).
**-W-**
    Turn outline off.
**-Z**\ *zval*
    Obtain fill via cpt lookup using z-value *zval*.
**-Z**\ *NaN*
    Get the NaN color from the CPT.

See Also
--------

:doc:`gmt.conf </gmt.conf>`, :doc:`gmt </gmt>`,
:doc:`gmtcolors </gmtcolors>`,
:doc:`grdbarb`, :doc:`plot3d </plot3d>`
