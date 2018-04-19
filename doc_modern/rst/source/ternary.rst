.. index:: ! ternary

*********
ternary
*********

.. only:: not man

    Plot data on ternary diagrams

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt ternary** [ *table* ]
[ **-JX** \ *width*\ [unit] ]
[ |SYN_OPT-Rz| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-L|\ *a*\ /*b*\ /*c* ]
[ |-M| ]
[ |-N| ]
[ |-S|\ [*symbol*][\ *size*\ [**u**] ]
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

**ternary** reads (*a*,\ *b*,\ *c*\ [,\ *z*]) records from *files* [or standard input] and
plots symbols
at those locations on a ternary diagram. If a symbol is selected and no symbol size
given, then **ternary** will interpret the fourth column of the input data
as symbol size. Symbols whose *size* is <= 0 are skipped. If no symbols
are specified then the symbol code (see **-S** below) must be present as
last column in the input.

Optional Arguments
------------------

.. |Add_intables| replace:: Use **-T** to ignore all input files, including standard input (see below).
.. include:: explain_intables.rst_

.. _-B:

**-B**\ [**a**\ \|\ **b**\ \|\ **c**\ ]\ *args*
    For ternary diagrams the three sides are referred to as a, b, and c.  Thus,
    to give specific settings for one of these axis you must include the
    axis letter before the arguments.  If all axes have the same arguments
    then only give one option without the axis letter.  For more details,
    see the -B discussion in basemap.

.. _-C:

**-C**\ *cpt*
    Give a CPT or specify **-C**\ *color1,color2*\ [*,color3*\ ,...]
    to build a linear continuous CPT from those colors automatically.
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).
    If **-S** is set, let symbol fill color be
    determined by the z-value in the fourth column. Additional fields are
    shifted over by one column (optional size would be 5th rather than 4th
    field, etc.).

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Select color or pattern for filling of symbols [Default is no fill].
    Note that **ternary** will search for **-G** and **-W** strings in all the
    segment headers and let any values thus found over-ride the command line settings.

.. _-J:

**-JX**\ *width* \[*unit*]
    The only valid projection is linear plot with specified ternary width.

.. _-L:

**-L**\ *a*\ /*b*\ /*c*
    Set the labels for the three diagram vertices [none].
    These are placed a distance of 3 times the MAP_LABEL_OFFSET setting from
    their respective corners.

.. _-M:

**-M**
    Do no plotting.  Instead, convert the input (*a*,\ *b*,\ *c*\ [,*z*]) records
    to Cartesian (*x*,\ *y*,\ [,*z*]) records, where *x, y* are normalized coordinates
    on the triangle (i.e., 0-1 in *x*\ and 0-sqrt(3)/2 in *y*\ ).

.. _-N:

**-N**
    Do NOT clip symbols that fall outside map border [Default plots points
    whose coordinates are strictly inside the map border only].

.. _-R:

**-R**\ *amin/amax/bmin/bmax/cmin/cmax*
    Give the min and max limits for each of the three axis **a**, **b**, and **c**.

.. _-S:

.. include:: explain_symbols.rst_

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    Set pen attributes for the outline of symbols.

.. _-X:

.. include:: explain_-XY.rst_

.. |Add_-bi| replace:: [Default is the required number of columns given the chosen settings].
.. include:: explain_-bi.rst_

.. include:: explain_-aspatial.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| replace:: The **-g** option is ignored if **-S** is set.
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_colon.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_vectors.rst_

Examples
--------

To plot circles (diameter = 0.1 cm) on a 6-inch-wide ternary diagram at the positions listed
in the file ternary.txt, with default annotations and gridline spacings, using the
specified labeling, tru

   ::

    gmt ternary ternary.txt -R0/100/0/100/0/100 -JX6i -Xc -Baafg+l"Water component"+u" %"
        -Bbafg+l"Air component"+u" %" -Bcagf+l"Limestone component"+u" %"
        -B+givory+t"Example data from MATLAB Central" -Sc0.1c -Ct.cpt -Y2i -LWater/Air/Limestone -pdf map


See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`basemap`,
:doc:`plot`,
:doc:`plot3d`
