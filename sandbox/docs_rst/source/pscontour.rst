*********
pscontour
*********

pscontour - Contour table data by direct triangulation [method]

`Synopsis <#toc1>`_
-------------------

**pscontour** [ *table* ] **-C**\ *cptfile* **-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] [
**-A**\ [**-**\ ][*labelinfo*\ ] ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-D**\ [*template*\ ] ] [
**-G**\ [**d**\ \|\ **f**\ \|\ **n**\ \|\ **l**\ \|\ **L**\ \|\ **x**\ \|\ **X**]\ *params*
] [ **-I** ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-L**\ *pen* ] [ **-N** ] [ **-O** ] [ **-P** ] [ **-Q**\ *indexfile* ]
[ **-S** ] [ **-T**\ [**+\|-**\ ][*gap/length*\ ][\ **:**\ [*labels*\ ]]
] [ **-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [
**-V**\ [*level*\ ] ] [ **-W**\ [**+**\ ]\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-c**\ *copies* ] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**pscontour** reads an ASCII [or binary] xyz-file and produces a raw
contour plot by triangulation. By default, the optimal Delaunay
triangulation is performed (using either Shewchuk’s [1996] or Watson’s
[1982] method as selected during BD(GMT) installation; type **pscontour
-** to see which method is selected), but the user may optionally
provide a second file with network information, such as a triangular
mesh used for finite element modeling. In addition to contours, the area
between contours may be painted according to the color palette file.
Alternatively, the x/y/z positions of the contour lines may be saved to
one or more output files (or stdout) and no plot is produced.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-C**\ *cptfile*
    name of the color palette file. Must have discrete colors if you
    want to paint the surface (**-I**). Only contours that have
    annotation flags set will be annotated.
**-J**\ *parameters* (\*)
    Select map projection.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.

`Optional Arguments <#toc5>`_
-----------------------------

*table*

One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ]) data
table file(s) holding a number of data columns. If no tables are given
then we read from standard input.

**-A**\ [**-**\ ][*labelinfo*\ ]

Give - to disable all annotations. The optional *labelinfo* controls the
specifics of the label formatting and consists of a concatenated string
made up of any of the following control arguments:

    **+a**\ *angle*

    For annotations at a fixed angle, **+an** for line-normal, or
    **+ap** for line-parallel [Default].

    **+c**\ *dx*\ [/*dy*]

    Sets the clearance between label and optional text box. Append
    **c**\ \|\ **i**\ \|\ **p** to specify the unit or % to indicate a
    percentage of the label font size [15%].

    **+d**

    Turns on debug which will draw helper points and lines to illustrate
    the workings of the quoted line setup.

    **+e**

    Delay the plotting of the text. This is used to build a clip path
    based on the text, then lay down other overlays while that clip path
    is in effect, then turning of clipping with psclip **-Ct** which
    finally plots the original text.

    **+f**\ *font*

    Sets the desired font [Default **FONT\_ANNOT\_PRIMARY** with its
    size changed to 9p].

    **+g**\ [*color*\ ]

    Selects opaque text boxes [Default is transparent]; optionally
    specify the color [Default is **PS\_PAGE\_COLOR**].

    **+j**\ *just*

    Sets label justification [Default is MC]. Ignored when
    **-SqN**\ \|\ **n**\ +\|-1 is used.

    **+l**\ *label*

    Sets the constant label text.

    **+L**\ *flag*

    Sets the label text according to the specified flag:

        **+Lh**
        Take the label from the current segment header (first scan for
        an embedded **-L**\ *label* option, if not use the first word
        following the segment flag). For multiple-word labels, enclose
        entire label in double quotes.
        **+Ld**
        Take the Cartesian plot distances along the line as the label;
        append **c**\ \|\ **i**\ \|\ **p** as the unit [Default is
        **PROJ\_LENGTH\_UNIT**].
        **+LD**
        Calculate actual map distances; append
        **d\|e\|f\|k\|n\|M\|n\|s** as the unit [Default is
        **d**\ (egrees), unless label placement was based on map
        distances along the lines in which case we use the same unit
        specified for that algorithm]. Requires a map projection to be
        used.
        **+Lf**
        Use text after the 2nd column in the fixed label location file
        as the label. Requires the fixed label location setting.
        **+Lx**
        As **+Lh** but use the headers in the *xfile.d* instead.
        Requires the crossing file option.

    **+n**\ *dx*\ [/*dy*]
        Nudges the placement of labels by the specified amount (append
        **c**\ \|\ **i**\ \|\ **p** to specify the units). Increments
        are considered in the coordinate system defined by the
        orientation of the line; use **+N** to force increments in the
        plot x/y coordinates system [no nudging].
    **+o**
        Selects rounded rectangular text box [Default is rectangular].
        Not applicable for curved text (**+v**) and only makes sense for
        opaque text boxes.
    **+p**\ [*pen*\ ]
        Draws the outline of text boxes [Default is no outline];
        optionally specify pen for outline [Default is width = 0.25p,
        color = black, style = solid].
    **+r**\ *min\_rad*
        Will not place labels where the line’s radius of curvature is
        less than *min\_rad* [Default is 0].
    **+t**\ [*file*\ ]
        Saves line label x, y, and text to *file* [Line\_labels.txt].
        Use **+T** to save x, y, angle, text instead.
    **+u**\ *unit*
        Appends *unit* to all line labels. If *unit* starts with a
        leading hyphen (-) then there will be no space between label
        value and the unit. [Default is no unit].
    **+v**
        Specifies curved labels following the path [Default is straight
        labels].
    **+w**
        Specifies how many (*x*,\ *y*) points will be used to estimate
        label angles [Default is 10].
    **+=**\ *prefix*
        Prepends *prefix* to all line labels. If *prefix* starts with a
        leading hyphen (-) then there will be no space between label
        value and the prefix. [Default is no prefix].

**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)

Set map boundary intervals.

**-D**\ [*template*\ ]

Dump the (x,y,z) coordinates of each contour to one or more output files
(or *stdout* if *template* is not given). No plotting will take place.
If *template* contains one or more of the C-format specifiers %d, %f, %c
then line segments will be written to different files; otherwise all
lines are written to the specified file (*template*). The use of the
C-format specifiers controls how many files are created and how the
contours are organized. If the float format %f is present (standard
modifications to width and precision are allowed, e.g., %f7.3f), then
the filenames will contain the contour value and lines are thus
separated into files based on a common contour value. If the integer
format %d is present (including modifications like %05d), then all
contours are written to individual segment files; if any of the other
specifiers are present they just affect the file names. Finally, if the
character format %c is present it is replaced with the letters C (for
closed) or O (for open), reflecting the nature of each contour. Any
combination of one, two, or all three modifiers are valid, resulting in
different filenames and number of files. For instance, if %c appears by
itself, then only two files are created, separating the open from the
closed contours (assuming both kinds are present). If just %f is used,
then all segments for the same contour level will be written to the same
file, resulting in *N* multi-segment files. If both %f and %c were
combined then each contour level would be further subdivided into closed
and open contours. Any combination involving %d will result in one
individual file for each segment; %c, %f only modifies the file names.
The files are ASCII unless **-bo** is used.

**-G**

Controls the placement of labels along the contours. Choose among five
controlling algorithms:

    **-G**\ **d**\ *dist*\ [**c**\ \|\ **i**\ \|\ **p**] or
    **-G**\ **D**\ *dist*\ [**d**\ \|\ **e**\ \|\ **f**\ \|\ **k**\ \|\ **m**\ \|\ **M**\ \|\ **n**\ \|\ **s**]
    For lower case **d**, give distances between labels on the plot in
    your preferred measurement unit **c** (cm), **i** (inch), or **p**
    (points), while for upper case **D**, specify distances in map units
    and append the unit; choose among **e** (m), **f** (foot), **k**
    (km), **M** (mile), **n** (nautical mile) or **u** (US survey foot),
    and **d** (arc degree), **m** (arc minute), or **s** (arc second).
    [Default is 10\ **c** or 4\ **i**]. As an option, you can append
    /*fraction* which is used to place the very first label for each
    contour when the cumulative along-contour distance equals *fraction
    \* dist* [0.25].
    **-G**\ **f**\ *ffile.d*
    Reads the ascii file *ffile.d* and places labels at locations in the
    file that matches locations along the contours. Inexact matches and
    points outside the region are skipped.
    **-G**\ **l\|L**\ *line1*\ [,*line2*,...]
    Give *start* and *stop* coordinates for one or more comma-separated
    straight line segments. Labels will be placed where these lines
    intersect the contours. The format of each *line* specification is
    *start/stop*, where *start* and *stop* are either a specified point
    *lon/lat* or a 2-character **XY** key that uses the justification
    format employed in **pstext** to indicate a point on the map, given
    as [LCR][BMT]. **-G**\ **L** will interpret the point pairs as
    defining great circles [Default is straight line].
    **-G**\ **n**\ *n\_label*
    Specifies the number of equidistant labels for contours line [1].
    Upper case **-G** **N** starts labeling exactly at the start of the
    line [Default centers them along the line]. **-G** **N**-1 places
    one justified label at start, while **-G** **N**\ +1 places one
    justified label at the end of contours. Optionally, append
    /*min\_dist*\ [**c**\ \|\ **i**\ \|\ **p**] to enforce that a
    minimum distance separation between successive labels is enforced.
    **-G**\ **x\|X**\ *xfile.d*
    Reads the multisegment file *xfile.d* and places labels at the
    intersections between the contours and the lines in *xfile.d*.
    **-G** **X** will resample the lines first along great-circle arcs.
    In addition, you may optionally append
    **+r**\ *radius*\ [**c**\ \|\ **i**\ \|\ **p**] to set a minimum
    label separation in the x-y plane [no limitation].

**-I**
    Color the triangles using the color palette table.
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**\ *pen*
    Draw the underlying triangular mesh using the specified pen
    attributes [Default is no mesh].
**-N**
    Do NOT clip contours or image at the boundaries [Default will clip
    to fit inside region **-R**].
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**\ *indexfile*
    Give name of file with network information. Each record must contain
    triplets of node numbers for a triangle [Default computes these
    using Delaunay triangulation (see **triangulate**)].
**-S**
    Skip all input *xyz* points that fall outside the region [Default
    uses all the data in the triangulation].
**-T**\ [**+\|-**\ ][*gap/length*\ ][\ **:**\ [*labels*\ ]]
    Will draw tickmarks pointing in the downward direction every *gap*
    along the innermost closed contours. Append *gap* and tickmark
    length (append units as **c**, **i**, or **p**) or use defaults
    [15**p**/3**p**]. User may choose to tick only local highs or local
    lows by specifying **-T+** or **-T-**, respectively. Append
    **:**\ *labels* to annotate the centers of closed innermost contours
    (i.e, the local lows and highs). If no *labels* is appended we use -
    and + as the labels. Appending two characters, **:LH**, will plot
    the two characters (here, L and H) as labels. For more elaborate
    labels, separate the two label strings by a comma (e.g.,
    **:**\ *lo*,\ *hi*). If a file is given by **-C** and **-T** is set,
    then only contours marked with upper case C or A will have tickmarks
    [and annotation].
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**\ [**+**\ ]\ *pen*
    Select contouring and set contour pen attributes. If the **+** flag
    is prepended then the color of the contour lines are taken from the
    cpt file (see **-C**). If the **-** flag is prepended then the color
    from the cpt file is applied both to the contours and the contour
    annotations.
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 3 input columns]. Use 4-byte
    integer triplets for node ids (**-Q**).
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is 3 output columns].
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc6>`_
-------------------

To make a raw contour plot from the file topo.xyz and drawing the
contours (pen = 2) given in the color palette file topo.cpt on a Lambert
map at 0.5 inch/degree along the standard parallels 18 and 24, use

pscontour topo.xyz -R320/330/20/30 **-Jl**\ 18/24/0.5\ **i** -Ctopo.cpt
-W0.5p > topo.ps

To create a color *PostScript* plot of the numerical temperature
solution obtained on a triangular mesh whose node coordinates and
temperatures are stored in temp.xyz and mesh arrangement is given by the
file mesh.ijk, using the colors in temp.cpt, run

pscontour temp.xyz -R0/150/0/100 -Jx0.1i -Ctemp.cpt -G -W0.25p > temp.ps

`Bugs <#toc7>`_
---------------

Sometimes there will appear to be thin lines of the wrong color in the
image. This is a round-off problem which may be remedied by using a
higher value of **PS\_DPI** in the **gmt.conf** file.

To save the triangulated 100-m contour lines in topo.txt and separate
them into multisegment files (one for each contour level), try

pscontour topo.txt -C100 -Dcontours\_%.0f.txt

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*grdcontour*\ (1) <grdcontour.html>`_ ,
`*grdimage*\ (1) <grdimage.html>`_ ,
`*nearneighbor*\ (1) <nearneighbor.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_ , `*psscale*\ (1) <psscale.html>`_
, `*surface*\ (1) <surface.html>`_ ,
`*triangulate*\ (1) <triangulate.html>`_

`References <#toc9>`_
---------------------

Watson, D. F., 1982, Acord: Automatic contouring of raw data, *Comp. &
Geosci.*, **8**, 97-101.

Shewchuk, J. R., 1996, Triangle: Engineering a 2D Quality Mesh Generator
and Delaunay Triangulator, First Workshop on Applied Computational
Geometry (Philadelphia, PA), 124-133, ACM, May 1996.

www.cs.cmu.edu/~quake/triangle.html
