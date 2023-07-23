.. index:: ! meca
.. include:: ../module_supplements_purpose.rst_

******
meca
******

|meca_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt meca** [ *table* ]
|-J|\ *parameters*
|SYN_OPT-R|
|-S|\ *format*\ [*scale*][**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+l**][**+m**][**+o**\ *dx*\ [/*dy*]][**+s**\ *reference*]
[ |-A|\ [**+p**\ *pen*][**+s**\ *size*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *depmin*/*depmax* ]
[ |-E|\ *fill* ]
[ |-F|\ *mode*\ [*args*] ]
[ |-G|\ *fill* ]
[ |-H|\ [*scale*] ]
[ |-I|\ [*intens*] ]
[ |-L|\ [*pen*] ]
[ |-N| ]
[ |-T|\ *nplane*\ [/*pen*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-tv| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads data values from *files* [or standard input] and
plots focal mechanisms.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: /explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-R:

.. |Add_-Rgeo| replace:: |Add_-R_auto_table|
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

.. include:: explain_meca_-S.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [**+p**\ *pen*][**+s**\ *size*]
    Offsets focal mechanisms to the alternate longitudes, latitudes given
    in the last two columns of the input file before the (optional) text
    string. We will draw a line connecting the original and relocated
    beachball positions and optionally place a small circle at the original
    location.  Use **+s**\ *size* to set the diameter of the circle [no circle].
    The line pen defaults to that given via |-W| but can be overridden
    by using **+p**\ *pen* [0.25p].

.. |Add_-B| replace:: |Add_-B_links|
.. include:: ../../explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *cpt*
    Give a CPT and let compressive part color be
    determined by the z-value in the third column.

.. _-D:

**-D**\ *depmin/depmax*
    Plots events between depmin and depmax.

.. _-E:

**-E**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Selects filling of extensive quadrants. [Default is white].

.. _-F:

**-F**\ *mode*\ [*args*]
    Sets one or more attributes; repeatable. The various combinations are
**-Fa**\ [*size*\ [/*Psymbol*\ [*Tsymbol*]]]
    Computes and plots P and T axes with symbols. Optionally specify
    *size* and (separate) P and T axis symbols from the following:
    (**c**) circle, (**d**) diamond, (**h**) hexagon, (**i**) inverse
    triangle, (**p**) point, (**s**) square, (**t**) triangle, (**x**)
    cross. [Default: 6\ **p**/**cc**]
**-Fe**\ *fill*
    Sets the color or fill pattern for the T axis symbol. [Default as
    set by |-E|]
**-Fg**\ *fill*
    Sets the color or fill pattern for the P axis symbol. [Default as
    set by |-G|]
**-Fo**
    Use the **psvelomeca** input format without depth in the third column.
**-Fp**\ [*pen*]
    Draws the P axis outline using default pen (see |-W|), or sets pen attributes.
**-Fr**\ [*fill*]
    Draw a box behind the label (if any). [Default fill is white]
**-Ft**\ [*pen*]
    Draws the T axis outline using default pen (see |-W|), or sets pen
    attributes.
**-Fz**\ [*pen*]
    Overlay zero trace moment tensor using default pen (see |-W|), or
    sets pen attributes.

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Selects filling of focal mechanisms. By convention, the
    compressional quadrants of the focal mechanism beach balls are
    shaded. [Default is black].

.. _-H:

**-H**\ [*scale*]
    Scale symbol sizes and pen widths on a per-record basis using the *scale* read from the
    data set, given as the first column after the (optional) *size* column [no scaling].
    The symbol size is either provided by |-S| or via the input *size* column.  Alternatively,
    append a constant *scale* that should be used instead of reading a scale column.

.. _-I:

**-I**\ *intens*
    Use the supplied *intens* value (nominally in the -1 to +1 range) to
    modulate the compressional fill color by simulating illumination [none].
    If no intensity is provided we will instead read *intens* from an extra
    data column after the required input columns determined by |-S|.

.. _-L:

**-L**\ [*pen*]
    Draws the "beach ball" outline with *pen* attributes instead of with
    the default pen set by |-W|.

.. _-N:

**-N**
    Does **not** skip symbols that fall outside frame boundary specified
    by |-R| [Default plots symbols inside frame only].

.. _-T:

**-T**\ [*nplane*][**/**\ *pen*]
    Plots the nodal planes and outlines the bubble which is transparent.
    If *nplane* is

    *0*: both nodal planes are plotted;

    *1*: only the first nodal plane is plotted;

    *2*: only the second nodal plane is plotted.

    Append **/**\ *pen* to set the pen attributes for this feature.
    Default pen is as set by |-W|. [Default: 0].

    For double couple mechanisms, the |-T| option renders the beach ball transparent
    by drawing only the nodal planes and the circumference.
    For non-double couple mechanisms, **-T**\ *0* option overlays best double couple transparently.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: ../../explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *pen*
    Set pen attributes for all lines and the outline of symbols
    [Defaults: 0.25p,black,solid]. This
    setting applies to |-A|, |-L|, |-T|, **-Fp**, **-Ft**, and
    **-Fz**, unless overruled by options to those arguments.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: ../../explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_
.. include:: ../../explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-qi.rst_
.. include:: ../../explain_-tv_full.rst_
.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. include:: seis_extra_cols.rst_

.. module_common_ends

Examples
--------

.. include:: ../../oneliner_info.rst_

The following file should give a normal-faulting CMT mechanism::

    gmt meca -R239/240/34/35.2 -Jm4c -Sc2c -pdf test << END
    # lon lat depth str dip slip st dip slip mant exp plon plat
    239.384 34.556 12. 180 18 -88 0 72 -90 5.5 0 0 0
    END

.. include:: meca_notes.rst_

See Also
--------

:doc:`polar`,
:doc:`coupe`,
:doc:`gmt </gmt>`, :doc:`basemap </basemap>`, :doc:`plot </plot>`
