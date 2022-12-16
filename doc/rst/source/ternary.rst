.. index:: ! ternary
.. include:: module_core_purpose.rst_

*******
ternary
*******

|ternary_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt ternary** [ *table* ]
[ |-J|\ **X**\ [-]\ *width* ]
[ |-R|\ *amin*\ /*amax*\ /*bmin*\ /*bmax*\ /*cmin*\ /*cmax* ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-L|\ *a*\ /*b*\ /*c* ]
[ |-M| ]
[ |-N| ]
[ |-S|\ [*symbol*][*size*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][*attr*] ]
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
[ |SYN_OPT-s| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads (*a*,\ *b*,\ *c*\ [,\ *z*]) records from *table* [or standard input] and
plots symbols at those locations on a ternary diagram. If a symbol is selected
and no symbol size given, then we will interpret the fourth column of the input data
as symbol size. Symbols whose *size* is <= 0 are skipped. If no symbols
are specified then the symbol code (see |-S| below) must be present as
last column in the input.  If |-S| is not specified then we instead plot
lines or polygons.

Required Arguments
------------------

Either |-M| (for dumping data) or |-R| and |-J| must be selected.

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

Optional Arguments
------------------

.. _-B:

**-B**\ [**a**\|\ **b**\|\ **c**]\ *args*
    For ternary diagrams the three sides are referred to as **a**, **b**, and **c**.  Thus,
    to give specific settings for one of these axis you must include the
    axis letter before the arguments.  If all axes have the same arguments
    then only give one option without the axis letter.  For more details,
    see the |-B| discussion in basemap.

.. _-C:

**-C**\ *cpt*
    Give a CPT or specify **-C**\ *color1,color2*\ [*,color3*\ ,...]
    to build a linear continuous CPT from those colors automatically.
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).
    If |-S| is set, let symbol fill color be
    determined by the z-value in the fourth column. Additional fields are
    shifted over by one column (optional size would be 5th rather than 4th
    field, etc.).  If modern mode and no argument is given then we select the current CPT.

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Select color or pattern for filling of symbols [Default is no fill].
    Note that we will search for |-G| and |-W| strings in all the
    segment headers and let any values thus found over-ride the command line settings.

.. _-J:

**-JX**\ [-]\ *width*
    The only valid projection is linear plot with specified ternary width.
    Use a negative *width* to indicate that positive axes directions be clock-wise
    [Default lets the *a, b, c* axes be positive in a counter-clockwise direction].

.. _-L:

**-L**\ *a*\ /*b*\ /*c*
    Set the labels for the three diagram vertices where the component is 100% [none].
    These are placed at a distance of three times the :term:`MAP_LABEL_OFFSET`
    setting from their respective corners.  To skip any one of them, specify that
    label as -.

.. _-M:

**-M**
    Do no plotting.  Instead, convert the input (*a*,\ *b*,\ *c*\ [,\ *z*]) records
    to Cartesian (*x*,\ *y*,\ [,\ *z*]) records, where *x, y* are normalized coordinates
    on the triangle (i.e., 0–1 in *x* and 0–sqrt(3)/2 in *y*\ ).

.. _-N:

**-N**
    Do **not** clip symbols that fall outside map border [Default plots points
    whose coordinates are strictly inside the map border only].

.. _-R:

**-R**\ *amin/amax/bmin/bmax/cmin/cmax*
    Give the min and max limits for each of the three axis **a**, **b**, and **c**.

.. _-S:

.. include:: explain_symbols_ternary.rst_

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    Set pen attributes for the outline of symbols.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-bi| replace:: [Default is the required number of columns given the chosen settings].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| replace:: The **-g** option is ignored if |-S| is set.
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-qi.rst_

.. include:: explain_-s.rst_

.. include:: explain_colon.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. module_common_ends

Examples
--------

To plot circles (diameter = 0.1 cm) on a 15-centimeter-wide ternary diagram at the positions listed
in the file ternary.txt, with default annotations and gridline spacings, using the
specified labeling, try::

    gmt begin map
    gmt makecpt -Cturbo -T0/80/10
    gmt ternary @ternary.txt -R0/100/0/100/0/100 -JX15c -Sc0.1c -C -LLimestone/Water/Air \
        -Baafg+l"Limestone component"+u" %" -Bbafg+l"Water component"+u" %" -Bcagf+l"Air component"+u" %" \
        -B+givory+t"Example data from MATLAB Central"
    gmt end show

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`basemap`,
:doc:`plot`,
:doc:`plot3d`
