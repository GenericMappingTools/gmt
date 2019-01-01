.. index:: ! contour


*********
contour
*********

.. only:: not man

    Contour table data by direct triangulation [method]

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt contour** [ *table* ] |-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A|\ [**-**\ \|\ [+]\ *annot\_int*][*labelinfo*] ]
[ |SYN_OPT-B| ]
[ |-C|\ [+]\ *cont_int* ]
[ |-D|\ [*template*] ] [ |-E|\ *indexfile* ]
[ |-G|\ [**d**\ \|\ **f**\ \|\ **n**\ \|\ **l**\ \|\ **L**\ \|\ **x**\ \|\ **X**]\ *params* ]
[ |-I| ] [ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-L|\ *pen* ] [ |-N| ]
[ |-Q|\ [*cut*\ [*unit*]][\ **+z**] ]
[ |-S|\ [\ *p*\ \|\ *t*] ]
[ |-T|\ [**h**\ \|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][\ **+l**\ [*labels*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*type*]\ *pen* ][**+c**\ [**l**\ \|\ **f**]]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**contour** reads an ASCII [or binary] *table* and produces a raw
contour plot by triangulation. By default, the optimal Delaunay
triangulation is performed (using either Shewchuk's [1996] or Watson's
[1982] method as selected during GMT installation; type **contour**
**-** to see which method is selected), but the user may optionally
provide a second file with network information, such as a triangular
mesh used for finite element modeling. In addition to contours, the area
between contours may be painted according to the CPT.
Alternatively, the *x, y, z* positions of the contour lines may be saved to
one or more output files (or stdout) and no plot is produced.

Required Arguments
------------------

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

**-A**\ [**-**\ \|\ [+]\ *annot_int*][*labelinfo*]
    *annot_int* is annotation interval in data units; it is ignored if
    contour levels are given in a file. [Default is no annotations]. Append
    **-** to disable all annotations implied by **-C**. Alternatively prepend
    + to the annotation interval to plot that as a single contour. The optional
    *labelinfo* controls the specifics of the label formatting and consists
    of a concatenated string made up of any of the following control arguments:

.. include:: explain_clabelinfo.rst_

.. _-B:

.. include:: explain_-B.rst_

.. _-C:

**-C**\ [+]\ *cont_int*
    The contours to be drawn may be specified in one of three possible ways:

    (1) If *cont_int* has the suffix ".cpt" and can be opened as a
        file, it is assumed to be a CPT. The color
        boundaries are then used as contour levels. If the CPT has
        annotation flags in the last column then those contours will be
        annotated. By default all contours are labeled; use **-A-** to
        disable all annotations.

    (2) If *cont_int* is a file but not a CPT, it is expected to
        contain contour levels in column 1 and a
        C(ontour) OR A(nnotate) in
        col 2. The levels marked C (or c) are contoured, the levels marked A
        (or a) are contoured and annotated. Optionally, a third column may
        be present and contain the fixed annotation angle for this contour
        level.

    (3) If no file is found, then *cont_int* is interpreted as a
        constant contour interval. However, if prepended with the + sign the
        *cont_int* is taken as meaning draw that single contour. The **-A**
        option offers the same possibility so they may be used together to
        plot only one annotated and one non-annotated contour.
        If **-A** is set and **-C** is not, then the contour interval is set
        equal to the specified annotation interval. Note to specify a negative
        value you must still prepend the +, as in '... -C+-10'.

    If a file is given and **-T** is set, then only contours marked with
    upper case C or A will have tick-marks. In all cases the contour
    values have the same units as the file.  Finally, if neither **-C**
    nor **-A** are set then we auto-compute suitable contour and annotation
    intervals from the data range, yielding 10-20 contours.

.. _-D:

**-D**\ [*template*]

.. include:: explain_contdump.rst_

.. _-E:

**-E**\ *indexfile*
    Give name of file with network information. Each record must contain
    triplets of node numbers for a triangle [Default computes these
    using Delaunay triangulation (see :doc:`triangulate`)].

.. _-G:

**-G**

.. include:: explain_contlabel.rst_

.. _-I:

**-I**
    Color the triangles using the CPT.

.. include:: explain_-Jz.rst_

.. _-L:

**-L**\ *pen* :ref:`(more ...) <set-pens>`
    Draw the underlying triangular mesh using the specified pen
    attributes [Default is no mesh].

.. _-N:

**-N**
    Do NOT clip contours or image at the boundaries [Default will clip
    to fit inside region **-R**].

.. _-Q:

**-Q**\ [*cut*\ [*unit*]][\ **+z**]
    Do not draw contours with less than *cut* number of points [Draw all contours].
    Alternatively, give instead a minimum contour length in distance units
    (see UNITS for available units and how distances are computed),
    including **c** (Cartesian distances using user coordinates) or **C** for plot
    length units in current plot units after projecting the coordinates.
    Optionally, append **z** to exclude the zero contour.

.. _-S:

**-S**\ [\ *p*\ \|\ *t*]
    Skip all input *xyz* points that fall outside the region [Default
    uses all the data in the triangulation].  Alternatively, use **-St**
    to skip triangles whose three vertices are all outside the region.
    **-S** with no modifier is interpreted as **-Sp**.

.. _-T:

**-T**\ [**h**\ \|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][\ **+l**\ [*labels*]]
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
    labels, separate the low and hight label strings with a comma (e.g.,
    **+l**\ *lo*,\ *hi*). If a file is given by **-C** and **-T** is set,
    then only contours marked with upper case C or A will have tick marks
    [and annotations].

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [*type*]\ *pen*\ [**+c**\ [**l**\ \|\ **f**]] :ref:`(more ...) <set-pens>`
    *type*, if present, can be **a** for annotated contours or **c** for
    regular contours [Default]. The *pen* sets the attributes for the
    particular line. Default pen for annotated contours: 0.75p,black.
    Regular contours use pen 0.25p,black. Normally, all contours are drawn
    with a fixed color determined by the pen setting. If the modifier **+cl** is appended
    then the color of the contour lines are taken from the CPT (see
    **-C**). If instead modifier **+cf** is appended then the color from the cpt
    file is applied to the contour annotations.  Select **+c** for both effects.

.. _-X:

.. include:: explain_-XY.rst_

.. |Add_-bi| replace:: [Default is 3 input columns]. Use 4-byte integer triplets for node ids (**-E**).
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 3 output columns].
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_colon.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

Examples
--------

To make a raw contour plot from the file topo.xyz and drawing the
contours (pen = 2) given in the CPT topo.cpt on a Lambert
map at 0.5 inch/degree along the standard parallels 18 and 24, use

   ::

    gmt contour topo.xyz -R320/330/20/30 -Jl18/24/0.5i -Ctopo.cpt -W0.5p -pdf topo

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

References
----------

Watson, D. F., 1982, Acord: Automatic contouring of raw data, *Comp. &
Geosci.*, **8**, 97-101.

Shewchuk, J. R., 1996, Triangle: Engineering a 2D Quality Mesh Generator
and Delaunay Triangulator, First Workshop on Applied Computational
Geometry (Philadelphia, PA), 124-133, ACM, May 1996.

http://www.cs.cmu.edu/~quake/triangle.html
