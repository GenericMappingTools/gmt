.. index:: ! grdcontour

**********
grdcontour
**********

.. only:: not man

    Make contour map using a grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdcontour** *grid*
|-J|\ *parameters* [ |-A|\ [**-**\ \|\ [+]\ *annot_int*][*labelinfo*] ]
[ |SYN_OPT-B| ]
[ |-C|\ [+]\ *cont_int*\ \|\ *cpt* ]
[ |-D|\ *template* ]
[ |-F|\ [**l**\ \|\ **r**] ]
[ |-G|\ [**d**\ \|\ **f**\ \|\ **n**\ \|\ **l**\ \|\ **L**\ \|\ **x**\ \|\ **X**]\ *params* ]
[ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-L|\ *low/high*\ \|\ **n**\ \|\ **N**\ \|\ **P**\ \|\ **p** ]
[ |-N|\ [*cpt*] ]
[ |-Q|\ [*cut*\ [*unit*]][\ **+z**] ]
[ |SYN_OPT-Rz| ]
[ |-S|\ *smoothfactor* ]
[ |-T|\ [**h**\ \|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][\ **+l**\ [*labels*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*type*]\ *pen* ][**+c**\ [**l**\ \|\ **f**]]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ [**+s**\ *factor*][**+o**\ *shift*][**+p**] ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-do| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ **-ho**\ [*n*] ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdcontour** reads a 2-D grid file and produces a contour map by
tracing each contour through the grid. Various options that affect the plotting
are available. Alternatively, the *x, y, z* positions of the contour lines
may be saved to one or more output files (or stdout) and no plot is produced.

Required Arguments
------------------

*grid*
    2-D gridded data set to be contoured. (See GRID FILE FORMATS below).

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

Optional Arguments
------------------

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
        plot a single annotated contour and another single non-annotated contour,
        as in '... -A+10 -C+5' that plots an annotated 10 contour and an non-annotated 5 contour.
        If **-A** is set and **-C** is not, then the contour interval is set
        equal to the specified annotation interval. Note to specify a negative
        value you must still prepend the +, as in '... -C+-10'.

    If a file is given and **-T** is set, then only contours marked with
    upper case C or A will have tick-marks. In all cases the contour
    values have the same units as the grid.  Finally, if neither **-C**
    nor **-A** are set then we auto-compute suitable contour and annotation
    intervals from the data range, yielding 10-20 contours.

.. _-D:

**-D**\ *template*
    Dump contours as data line segments; no plotting takes place.
    Append filename template which may contain C-format specifiers.
    If no filename template is given we write all lines to stdout.
    If filename has no specifiers then we write all lines to a single file.
    If a float format (e.g., %6.2f) is found we substitute the contour z-value.
    If an integer format (e.g., %06d) is found we substitute a running segment count.
    If an char format (%c) is found we substitute C or O for closed and open contours.
    The 1-3 specifiers may be combined and appear in any order to produce the
    the desired number of output files (e.g., just %c gives two files, just %f would.
    separate segments into one file per contour level, and %d would write all segments.
    to individual files; see manual page for more examples.

.. _-F:

**-F**\ [**l**\ \|\ **r**]
    Force dumped contours to be oriented so that higher z-values are to the
    left (**-Fl** [Default]) or right (**-Fr**) as we move along the contour
    [Default is arbitrary orientation]. Requires **-D**.

.. _-G:

**-G**\ [**d**\ \|\ **f**\ \|\ **n**\ \|\ **l**\ \|\ **L**\ \|\ **x**\ \|\ **X**]\ *params*

.. include:: explain_contlabel.rst_

.. include:: explain_-Jz.rst_

.. _-L:

**-L**\ *low/high*\ \|\ **n**\ \|\ **N**\ \|\ **P**\ \|\ **p**
    Limit range: Do not draw contours for data values below *low* or
    above *high*.  Alternatively, limit contours to negative (**n**)
    or positive (**p**) contours.  Use upper case **N** or **P** to
    include the zero contour.

.. _-N:

**-N**\ [*cpt*]
    Fill the area between contours using the discrete color table given by *cpt*.
    Then, **-C** and **-A** can be used as well to control the contour lines and
    annotations.  If no *cpt* is appended then a discrete color table must be given
    via **-C** instead.

.. _-Q:

**-Q**\ [*cut*\ [*unit*]][\ **+z**]
    Do not draw contours with less than *cut* number of points [Draw all contours].
    Alternatively, give instead a minimum contour length in distance units
    (see UNITS for available units and how distances are computed),
    including **c** (Cartesian distances using user coordinates) or **C** for plot
    length units in current plot units after projecting the coordinates.
    Optionally, append **z** to exclude the zero contour.

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| replace:: [Default is region defined in the grid file].
.. include:: explain_-Rz.rst_

.. _-S:

**-S**\ *smoothfactor*
    Used to resample the contour lines at roughly every
    (gridbox_size/*smoothfactor*) interval.

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

.. _-Z:

**-Z**\ [**+s**\ *factor*][**+o**\ *shift*][**+p**]
    Use to subtract *shift* from the data and multiply the results by
    *factor* before contouring starts [1/0]. (Numbers in **-A**, **-C**,
    **-L** refer to values after this scaling has occurred.) Append
    **+p** to indicate that this grid file contains z-values that are
    periodic in 360 degrees (e.g., phase data, angular distributions)
    and that special precautions must be taken when determining
    0-contours.

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bo.rst_

.. |Add_-do| unicode:: 0x20 .. just an invisible code
.. include:: explain_-do.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

.. include:: explain_grd_inout_short.rst_

Notes
-----

The angle of a contour is computed as an average over *n* points along the contour.
If you obtain poor angles you can play with two variables: Change *n* via the **+w**
modifier to **-A**, and/or resample the contour via **-S**.  For a fixed *n* the
**-S** will localize the calculation, while the opposite is true if you increase *n*
for a constant **-S**.

Examples
--------

To contour the file hawaii_grav.nc every 25 mGal on a Mercator map at
0.5 inch/degree, annotate every 50 mGal (using fontsize = 10p), using 1
degree tickmarks, and draw 30 minute gridlines:

   ::

    gmt grdcontour hawaii_grav.nc -Jm0.5i -C25 -A50+f10p -B1g30m -pdf hawaii_grav

To contour the file image.nc using the levels in the file cont.txt on a
linear projection at 0.1 cm/x-unit and 50 cm/y-unit, using 20 (x) and
0.1 (y) tickmarks, smooth the contours a bit, use "RMS Misfit" as
plot-title, use a thick red pen for annotated contours, and a thin,
dashed, blue pen for the rest, and send the output to the default printer:

   ::

    gmt grdcontour image.nc -Jx0.1c/50.0c -Ccont.txt -S4 -Bx20 -By0.1
               -B+t"RMS Misfit" -Wathick,red -Wcthinnest,blue,- | lp

The labeling of local highs and lows may plot outside the innermost
contour since only the mean value of the contour coordinates is used to
position the label.

To save the smoothed 100-m contour lines in topo.nc and separate them
into two multisegment files: contours_C.txt for closed and
contours_O.txt for open contours, try

   ::

    gmt grdcontour topo.nc -C100 -S4 -Dcontours_%c.txt

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`basemap`,
:doc:`grdimage`, :doc:`grdview`,
:doc:`contour`
