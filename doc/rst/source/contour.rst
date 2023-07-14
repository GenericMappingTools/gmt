.. index:: ! contour
.. include:: module_core_purpose.rst_


*********
contour
*********

|contour_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt contour** [ *table* ]
|-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A|\ [**n**\|\ *contours*][*labelinfo*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *contours* ]
[ |-D|\ [*template*] ]
[ |-E|\ *indexfile*\ [**+b**] ]
[ |-G|\ [**d**\|\ **f**\|\ **n**\|\ **l**\|\ **L**\|\ **x**\|\ **X**]\ *params* ]
[ |-I| ]
[ |-J|\ **z**\|\ **Z**\ *parameters* ]
[ |-L|\ *pen* ]
[ |-N| ]
[ |-Q|\ [*n*][**+z**] ]
[ |-S|\ [*p*\|\ *t*] ]
[ |-T|\ [**h**\|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][**+l**\ [*labels*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*type*]\ *pen*\ [**+c**\ [**l**\|\ **f**]] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-l| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads an ASCII [or binary] *table* and produces a raw
contour plot by triangulation. By default, the optimal Delaunay
triangulation is performed (using either Shewchuk's [1996] or Watson's
[1982] method as selected during GMT installation; run
**gmt get GMT_TRIANGULATE** to see which method is selected), but the user may
optionally provide a second file with network information, such as a triangular
mesh used for finite element modeling. In addition to contours, the area
between contours may be painted according to the CPT.
Alternatively, the *x, y, z* positions of the contour lines may be saved to
one or more output files (or standard output) and no plot is produced.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

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

**-A**\ [**n**\|\ *contours*][*labelinfo*]
    *contours* is annotation interval in data units; it is ignored if
    contour levels are given in a file via |-C|. [Default is no annotations]. Prepend
    **n** to disable all annotations implied by |-C|. To just select a few specific
    contours give them as a comma-separated string; if only a single contour please add
    a trailing comma so it is seen as a list and not a contour interval. The optional
    *labelinfo* controls the specifics of the label formatting and consists
    of a concatenated string made up of any of the following control arguments:

.. include:: explain_clabelinfo.rst_

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *contours*
    The contours to be drawn may be specified in one of four possible ways:

    (1) If *contours* is a string with suffix ".cpt" and can be opened as a
        file, it is assumed to be a CPT. The color
        boundaries are then used as contour levels. If the CPT has
        annotation flags in the last column then those contours will be
        annotated. By default all contours are labeled; use **-An** to
        disable all annotations.

    (2) If *contours* is a file but not a CPT, it is expected to contain
        one record per contour, with information given in the order
        *contour-level* [*angle*] **C**\|\ **c**\|\ **A**\|\ **a** [*pen*],
        where items in brackets are optional.  The levels marked **C** (or **c**)
        are contoured, while the levels marked **A** (or **a**) are both contoured
        and annotated. If the annotation *angle* is present we will plot the label
        at that fixed angle [aligned with the contour].  Finally, a contour-
        specific *pen* may be present and will override the pen set by |-W|
        for this contour level only. **Note**: Please specify *pen* in proper
        format so it can be distinguished from a plain number like *angle*.
        If only *cont-level* columns are present then we set type to **C**.

    (3) If *contours* is a string with comma-separated values it is interpreted
        as those specific contours only.  To indicate a single specific contour
        you must append a trailing comma to separate it from a contour interval.
        The |-A| option offers the same list choice so they may be used together
        to plot only specific annotated and non-annotated contours.

    (4) If no argument is given in modern mode then we select the current CPT.

    (5) Otherwise, *contours* is interpreted as a constant contour interval.

    If a file is given and |-T| is set, then only contours marked with
    upper case C or A will have tick-marks. In all cases the contour
    values have the same units as the file.  Finally, if neither |-C|
    nor |-A| are set then we auto-compute suitable contour and annotation
    intervals from the data range, yielding approximately 10-20 contours.

.. _-D:

**-D**\ [*template*]

.. include:: explain_contdump.rst_

.. _-E:

**-E**\ *indexfile*\ [**+b**]
    Give name of file with network information. Each record must contain
    triplets of node numbers for a triangle [Default computes these
    using Delaunay triangulation (see :doc:`triangulate`)]. If the
    *indexfile* is binary and can be read the same way as the binary
    input *table* then you can append **+b** to spead up the reading
    [Default reads nodes as ASCII].

.. _-G:

**-G**

.. include:: explain_contlabel.rst_

.. _-I:

**-I**
    Color the triangles using the CPT.

.. _-L:

**-L**\ *pen* :ref:`(more ...) <set-pens>`
    Draw the underlying triangular mesh using the specified pen
    attributes [Default is no mesh].

.. _-N:

**-N**
    Do **not** clip contours or image at the boundaries [Default will clip
    to fit inside region |-R|].

.. _-Q:

**-Q**\ [*n*][**+z**]
    Do not draw contours with less than *n* number of points [Draw all contours].
    Alternatively, give instead a minimum contour length in distance units
    (see `Units`_ for available units and how distances are computed),
    including **c** (Cartesian distances using user coordinates) or **C** for plot
    length units in current plot units after projecting the coordinates.
    Optionally, append **z** to exclude the zero contour.

.. _-S:

**-S**\ [**p**\|\ **t**]
    Skip all input *xyz* points that fall outside the region [Default
    uses all the data in the triangulation].  Alternatively, use **-St**
    to skip triangles whose three vertices are all outside the region.
    |-S| with no modifier is interpreted as **-Sp**.

.. _-T:

**-T**\ [**h**\|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][**+l**\ [*labels*]]
    Will draw tick marks pointing in the downward direction every *gap*
    along the innermost closed contours only; append **+a** to tick all closed
    contours. Append **+d**\ *gap* and optionally tick
    mark *length* (append units as **c**, **i**, or **p**) or use defaults
    [15\ **p**/3\ **p**]. User may choose to tick only local highs or local
    lows by specifying **-Th** or **-Tl**, respectively. Append
    **+l**\ *labels* to annotate the centers of closed innermost contours
    (i.e., the local lows and highs). If no *labels* is appended we use -
    and + as the labels. Appending exactly two characters, e.g., **+l**\ *LH*,
    will plot the two characters (here, L and H) as labels. For more elaborate
    labels, separate the low and high label strings with a comma (e.g.,
    **+l**\ *lo*,\ *hi*). If a file is given by |-C| and |-T| is set,
    then only contours marked with upper case C or A will have tick marks
    [and annotations].

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*type*]\ *pen*\ [**+c**\ [**l**\|\ **f**]] :ref:`(more ...) <set-pens>`
    *type*, if present, can be **a** for annotated contours or **c** for
    regular contours [Default]. The *pen* sets the attributes for the
    particular line. Default pen for annotated contours: 0.75p,black.
    Regular contours use pen 0.25p,black. Normally, all contours are drawn
    with a fixed color determined by the pen setting. If the modifier **+cl** is appended
    then the color of the contour lines are taken from the CPT (see
    |-C|). If instead modifier **+cf** is appended then the color from the cpt
    file is applied to the contour annotations.  Select **+c** for both effects.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-bi| replace:: [Default is 3 input columns]. Use 4-byte integer triplets for node ids (|-E|).
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 3 output columns].
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_-l| replace:: Normally, the annotated contour is selected for the legend. You can select the regular contour instead, or both of them, by considering the *label* to be of the format [*annotcontlabel*][/*contlabel*].  If either label contains a slash (/) character then use | as the separator for the two labels instead.
.. include:: explain_-l.rst_

.. include:: explain_-qi.rst_

.. include:: explain_colon.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-s.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. module_common_ends

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To make a raw contour plot from the remote file Table_5.11.txt and draw the
contours every 25 and annotate every 50, using the default Cartesian projection, try

::

  gmt contour @Table_5_11.txt -Wthin -C25 -A50 -B -pdf map

To use the same data but only contour the values 750 and 800, use

::

  gmt contour @Table_5_11.txt -A750,800 -W0.5p -B -pdf map

To create a color plot of the numerical temperature
solution obtained on a triangular mesh whose node coordinates and
temperatures are stored in temp.xyz and mesh arrangement is given by the
file mesh.ijk, using the colors in temp.cpt, run

::

  gmt contour temp.xyz -R0/150/0/100 -Jx0.1i -Ctemp.cpt -G -W0.25p -pdf temp

To save the triangulated 100-m contour lines in topo.txt and separate
them into multisegment files (one for each contour level), try

::

  gmt contour topo.txt -C100 -Dcontours_%.0f.txt

.. include:: contour_notes.rst_

.. include:: auto_legend_info.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`grdcontour`,
:doc:`grdimage`,
:doc:`nearneighbor`,
:doc:`basemap`, :doc:`colorbar`,
:doc:`surface`,
:doc:`triangulate`
