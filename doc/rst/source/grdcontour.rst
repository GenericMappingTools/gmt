.. index:: ! grdcontour
.. include:: module_core_purpose.rst_

**********
grdcontour
**********

|grdcontour_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdcontour** *ingrid*
|-J|\ *parameters* [ |-A|\ [**n**\|\ *contours*][*labelinfo*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *contours*\|\ *cpt* ]
[ |-D|\ *template* ]
[ |-F|\ [**l**\|\ **r**] ]
[ |-G|\ [**d**\|\ **f**\|\ **n**\|\ **l**\|\ **L**\|\ **x**\|\ **X**]\ *params* ]
[ |-L|\ *low/high*\|\ **n**\|\ **N**\|\ **P**\|\ **p** ]
[ |-N|\ [*cpt*] ]
[ |-Q|\ [*n*][**+z**] ]
[ |SYN_OPT-Rz| ]
[ |-S|\ *smoothfactor* ]
[ |-T|\ [**h**\|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][**+l**\ [*labels*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*type*]\ *pen*\ [**+c**\ [**l**\|\ **f**]] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ [**+o**\ *shift*][**+p**][**+s**\ *factor*] ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-do| ]
[ |SYN_OPT-f| ]
[ **-ho**\ [*n*] ]
[ |SYN_OPT-l| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

**grdcontour** reads a 2-D grid file and produces a contour map by
tracing each contour through the grid. Various options that affect the plotting
are available. Alternatively, the *x, y, z* positions of the contour lines
may be saved to one or more output files (or standard output) and no plot is produced.

Required Arguments
------------------

.. |Add_ingrid| replace:: 2-D gridded data set to be contoured.
.. include:: explain_grd_inout.rst_
    :start-after: ingrid-syntax-begins
    :end-before: ingrid-syntax-ends

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

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
        *contour-level* [[*angle*] **C**\|\ **c**\|\ **A**\|\ **a** [*pen*]],
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

    If a file is given and |-T| is set, then only inner-most contours marked with
    upper case **C** or **A** will have tick-marks. In all cases the contour
    values have the same units as the grid.  Finally, if neither |-C|
    nor |-A| are set then we auto-compute suitable contour and annotation
    intervals from the data range, yielding approximately 10-20 contours.

.. _-D:

**-D**\ *template*
    Dump contours as data line segments.
    No plotting takes place.
    Append filename template which may contain C language
    `printf <https://en.wikipedia.org/wiki/Printf_format_string>`__ format specifiers.
    If no filename template is given we write all lines to standard output.
    If filename has no specifiers then we write all lines to a single file.

    * If a float format (e.g., %6.2f) is found we substitute the contour z-value.
    * If an integer format (e.g., %06d) is found we substitute a running segment count.
    * If an char format (%c) is found we substitute C or O for closed and open contours.

    The 1-3 specifiers may be combined and appear in any order to produce the
    the desired number of output files.

    E.g., just %c gives two files, just %f would
    separate segments into one file per contour level, and %d would write all segments
    to individual files. See `printf(3) <https://linux.die.net/man/3/printf>`__ for more examples.

.. _-F:

**-F**\ [**l**\|\ **r**]
    Force dumped contours to be oriented so that higher z-values are to the
    left (**-Fl** [Default]) or right (**-Fr**) as we move along the contour
    [Default is arbitrary orientation]. Requires |-D|.

.. _-G:

**-G**\ [**d**\|\ **f**\|\ **n**\|\ **l**\|\ **L**\|\ **x**\|\ **X**]\ *params*

.. include:: explain_contlabel.rst_

.. _-L:

**-L**\ *low/high*\|\ **n**\|\ **N**\|\ **P**\|\ **p**
    Limit range: Do not draw contours for data values below *low* or
    above *high*.  Alternatively, limit contours to negative (**n**)
    or positive (**p**) contours.  Use upper case **N** or **P** to
    include the zero contour.

.. _-N:

**-N**\ [*cpt*]
    Fill the area between contours using the discrete color table given by *cpt*.
    Then, |-C| and |-A| can be used as well to control the contour lines and
    annotations.  If no *cpt* is appended then a discrete color table must be given
    via |-C| instead.

.. _-Q:

**-Q**\ [*n*][**+z**]
    Do not draw contours with less than *n* number of points [Draw all contours].
    Alternatively, give instead a minimum contour length in distance units
    (see `Units`_ for available units and how distances are computed),
    including **c** (Cartesian distances using user coordinates) or **C** for plot
    length units in current plot units after projecting the coordinates.
    Optionally, append **+z** to exclude the zero contour.

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-Rz| replace:: [Default is region defined in the grid file].
.. include:: explain_-Rz.rst_

.. _-S:

**-S**\ *smoothfactor*
    Used to resample the contour lines at roughly every
    (gridbox_size/*smoothfactor*) interval.

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
    [and annotations].  **Note**: The labeling of local highs and lows may plot sometimes
    outside the innermost contour since only the mean value of the contour coordinates
    is used to position the label.

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

.. _-Z:

**-Z**\ [**+o**\ *shift*][**+p**][**+s**\ *factor*]
    Use to subtract *shift* from the data and multiply the results by
    *factor* before contouring starts [+o0+s1]. (Numbers in |-A|, |-C|,
    |-L| refer to values after this scaling has occurred.) Append
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

.. |Add_-l| replace:: Normally, the annotated contour is selected for the legend. You can select the regular contour instead, or both of them, by considering the *label* to be of the format [*annotcontlabel*][/*contlabel*].  If either label contains a slash (/) character then use | as the separator for the two labels instead.
.. include:: explain_-l.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

.. module_common_ends

.. module_note_begins

Notes
-----

The angle of a contour is computed as an average over *n* points along the contour.
If you obtain poor angles you can play with two variables: Change *n* via the **+w**
modifier to |-A|, and/or resample the contour via |-S|.  For a fixed *n* the
**-S** will localize the calculation, while the opposite is true if you increase *n*
for a constant |-S|.

.. module_note_ends

.. include:: auto_legend_info.rst_

Examples
--------

.. include:: oneliner_info.rst_

To contour the remote file AK_gulf_grav.nc every 25 mGal on a Mercator map at
0.5 inch/degree, annotate every 50 mGal (using font size = 10p), using 1
degree tick marks, and draw 30 minute gridlines::

    gmt grdcontour @AK_gulf_grav.nc -JM16c -C25 -A50+f10p -B -pdf alaska_grav1

To do the same map but only draw the 50 and 150 and annotate the 100 contour::

    gmt grdcontour @AK_gulf_grav.nc -JM16c -C50,150 -A100,+f10p -B -pdf alaska_grav2

To contour the Alaska gravity data every 10 mGal with labels every 50 mGal, smooth
the contours a bit, use "Gravity Anomalies" as plot-title, use a thick red pen for
the annotated contours and a thin, dashed, blue pen for the rest, try::

    gmt grdcontour @AK_gulf_grav.nc -C10 -A50 -S4 -B -B+t"Gravity Anomalies" -Wathick,red -Wcthinnest,blue,- -pdf alaska_grav3

Same, but this time we want all negative contours to be blue and positive to be red, with
the zero-contour black::

    gmt begin alaska_grav4
      grdcontour @AK_gulf_grav.nc -C10 -A50 -S4 -B -B+t"Gravity Anomalies" -Ln -Wathick,blue -Wcthinnest,blue,-
      grdcontour @AK_gulf_grav.nc -C10 -A50 -S4 -Lp -Wathick,red -Wcthinnest,red,-
      grdcontour @AK_gulf_grav.nc -A0, -S4
    gmt end show

To save the smoothed 50-mGal contour lines in AK_gulf_grav.nc and separate them
into two multisegment files: contours_C.txt for closed and
contours_O.txt for open contours, try::

    gmt grdcontour @AK_gulf_grav.nc -C150 -S4 -DAK_contours_%c.txt

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`basemap`,
:doc:`grdimage`,
:doc:`grdview`,
:doc:`contour`
