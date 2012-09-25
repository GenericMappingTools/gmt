**********
grdcontour
**********

grdcontour - Make contour map using a grid

`Synopsis <#toc1>`_
-------------------

**grdcontour** *grid* **-C**\ *cont\_int*\ \|\ *cpt*
**-J**\ *parameters* [ **-A**\ [**-**\ \|\ *annot\_int*][*labelinfo*\ ]
] [ **-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-D**\ *template* ] [
**-F**\ [**l**\ \|\ **r**] ] [
**-G**\ [**d**\ \|\ **f**\ \|\ **n**\ \|\ **l**\ \|\ **L**\ \|\ **x**\ \|\ **X**]\ *params*
] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [ **-L**\ *low/high* ]
[ **-O** ] [ **-P** ] [ **-Q**\ *cut* ] [
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] ] [
**-S**\ *smoothfactor* ] [
**-T**\ [**+\|-**\ ][*gap/length*\ ][\ **:**\ [*labels*\ ]] ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [ **-W**\ [**+**\ ][*type*\ ]\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-Z**\ [*factor*\ [/*shift*]][**p**\ ] ] [
**-bo**\ [*ncols*\ ][*type*\ ] ] [ **-c**\ *copies* ] [ **-ho**\ [*n*\ ]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**grdcontour** reads a 2-D grid file and produces a contour map by
tracing each contour through the grid. *PostScript* code is generated
and sent to standard output. Various options that affect the plotting
are available. Alternatively, the x/y/z positions of the contour lines
may be saved to one or more output files (or stdout) and no plot is
produced.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*grid*
    2-D gridded data set to be contoured. (See GRID FILE FORMATS below).
**-C**\ *cont\_int*
    The contours to be drawn may be specified in one of three possible
    ways:

    (1) If *cont\_int* has the suffix ".cpt" and can be opened as a
    file, it is assumed to be a color palette table. The color
    boundaries are then used as contour levels. If the cpt-file has
    annotation flags in the last column then those contours will be
    annotated. By default all contours are labeled; use **-A-** to
    disable all annotations.

    (2) If *cont\_int* is a file but not a cpt-file, it is expected to
    contain contour levels in column 1 and a
    `C(ontour) <C.ontour.html>`_ OR `A(nnotate) <A.nnotate.html>`_ in
    col 2. The levels marked C (or c) are contoured, the levels marked A
    (or a) are contoured and annotated. Optionally, a third column may
    be present and contain the fixed annotation angle for this contour
    level.

    (3) If no file is found, then *cont\_int* is interpreted as a
    constant contour interval. If **-A** is set and **-C** is not, then
    the contour interval is set equal to the specified annotation
    interval.

    If a file is given and **-T** is set, then only contours marked with
    upper case C or A will have tickmarks. In all cases the contour
    values have the same units as the grid.

**-J**\ *parameters* (\*)
    Select map projection.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [**-**\ \|\ *annot\_int*][*labelinfo*\ ]

*annot\_int* is annotation interval in data units; it is ignored if
contour levels are given in a file. [Default is no annotations]. Append
**-** to disable all annotations implied by **-C**. The optional
*labelinfo* controls the specifics of the label formatting and consists
of a concatenated string made up of any of the following control
arguments:

    **+a**\ *angle*
    For annotations at a fixed angle, **+an** for line-normal, or
    **+ap** for line-parallel [Default]. By appending the **u** or **d**
    we get annotations whose top face the next upper or lower
    annotation, respectively.
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
    **+n**\ *dx*\ [/*dy*]
    Nudges the placement of labels by the specified amount (append
    **c**\ \|\ **i**\ \|\ **p** to specify the units). Increments are
    considered in the coordinate system defined by the orientation of
    the line; use **+N** to force increments in the plot x/y coordinates
    system [no nudging].
    **+o**
    Selects rounded rectangular text box [Default is rectangular]. Not
    applicable for curved text (**+v**) and only makes sense for opaque
    text boxes.
    **+p**\ [*pen*\ ]
    Draws the outline of text boxes [Default is no outline]; optionally
    specify pen for outline [Default is width = 0.25p, color = black,
    style = solid].
    **+r**\ *min\_rad*
    Will not place labels where the line’s radius of curvature is less
    than *min\_rad* [Default is 0].
    **+t**\ [*file*\ ]
    Saves contour label x, y, and text to *file* [Contour\_labels.txt].
    Use **+T** to save x, y, angle, text instead.
    **+u**\ *unit*
    Appends *unit* to all line labels. If *unit* starts with a leading
    hyphen (-) then there will be no space between label value and the
    unit. If no *unit* is appended we use the units listed in the grid
    file. [Default is no unit].
    **+v**
    Specifies curved labels following the path [Default is straight
    labels].
    **+w**
    Specifies how many (*x*,\ *y*) points will be used to estimate label
    angles [Default is 10].
    **+=**\ *prefix*
    Prepends *prefix* to all line labels. If *prefix* starts with a
    leading hyphen (-) then there will be no space between label value
    and the prefix. [Default is no prefix].

**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)

Set map boundary intervals.

**-D**\ *template*

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

**-F**\ [**l**\ \|\ **r**]

Force dumped contours to be oriented so that higher z-values are to the
left (**-Fl** [Default]) or right (**-Fr**) as we move along the contour
[Default is arbitrary orientation]. Requires **-D**.

**-G**

Controls the placement of labels along the contours. Choose among five
controlling algorithms:

    **-G**\ **d**\ *dist*\ [**c**\ \|\ **i**\ \|\ **p**] or
    **-G**\ **D**\ *dist*\ [**d**\ \|\ **e**\ \|\ **f**\ \|\ **k**\ \|\ **m**\ \|\ **M**\ \|\ **n**\ \|\ **s**]
    For lower case **d**, give distances between labels on the plot in
    your preferred measurement unit **c** (cm), **i** (inch), or **p**
    (points), while for upper case **D**, specify distances in map units
    and append the unit; choose among **e** (m), **f** (feet), **k**
    (km), **M** (mile), or **n** (nautical mile), and **d** (arc
    degree), **m** (arc minute), or **s** (arc second). [Default is
    10\ **c** or 4\ **i**]. As an option, you can append /*fraction*
    which is used to place the very first label for each contour when
    the cumulative along-contour distance equals *fraction \* dist*
    [0.25].
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
    as [LCR][BMT]. In addition, you may use Z+ and Z- which correspond
    to the locations of the global max and min locations in the grid,
    respectively. **-G**\ **L** will interpret the point pairs as
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

**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**\ *low/high*
    Limit range: Do not draw contours for data values below *low* or
    above *high*.
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**\ *cut*
    Do not draw contours with less than *cut* number of points [Draw all
    contours].
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.
    [Default is region defined in the grid file].
**-S**\ *smoothfactor*
    Used to resample the contour lines at roughly every
    (gridbox\_size/*smoothfactor*) interval.
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
**-W**\ [**+**\ ][*type*\ ]\ *pen*
    *type*, if present, can be **a** for annotated contours or **c** for
    regular contours [Default]. *pen* sets the attributes for the
    particular line. Default pen for annotated contours: 0.75p,black.
    Regular contours use pen 0.25p,black. If the **+** flag is prepended
    then the color of the contour lines are taken from the cpt file (see
    **-C**). If the **-** flag is prepended then the color from the cpt
    file is applied both to the contours and the contour annotations.
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-Z**\ [*factor*\ [/*shift*]][**p**\ ]
    Use to subtract *shift* from the data and multiply the results by
    *factor* before contouring starts [1/0]. (Numbers in **-A**, **-C**,
    **-L** refer to values after this scaling has occurred.) Append
    **p** to indicate that this grid file contains z-values that are
    periodic in 360 degrees (e.g., phase data, angular distributions)
    and that special precautions must be taken when determining
    0-contours.
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
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

`Ascii Format Precision <#toc6>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`File Formats <#toc7>`_
-----------------------

**GMT** is able to recognize many of the commonly used grid file
formats, as well as the precision, scale and offset of the values
contained in the grid file. When **GMT** needs a little help with that,
you can add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.17 of
the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. See `**grdreformat**\ (1) <grdreformat.html>`_ and
Section 4.18 of the GMT Technical Reference and Cookbook for more
information, particularly on how to read splices of 3-, 4-, or
5-dimensional grids.

`Examples <#toc8>`_
-------------------

To contour the file hawaii\_grav.nc every 25 mGal on a Mercator map at
0.5 inch/degree, annotate every 50 mGal (using fontsize = 10p), using 1
degree tickmarks, and draw 30 minute gridlines:

grdcontour hawaii\_grav.nc -Jm0.5i -C25 -A50+f10p -B1g30m >
hawaii\_grav.ps

To contour the file image.nc using the levels in the file cont.d on a
linear projection at 0.1 cm/x-unit and 50 cm/y-unit, using 20 (x) and
0.1 (y) tickmarks, smooth the contours a bit, use "RMS Misfit" as
plot-title, use a thick red pen for annotated contours, and a thin,
dashed, blue pen for the rest, and send the output to the default
printer:

grdcontour image.nc -Jx0.1c/50.0c -Ccont.d -S4 -B20/0.1:."RMS
Misfit":-Wathick,red -Wcthinnest,blue,- \| lp

The labeling of local highs and lows may plot outside the innermost
contour since only the mean value of the contour coordinates is used to
position the label.

To save the smoothed 100-m contour lines in topo.nc and separate them
into two multisegment files: contours\_C.txt for closed and
contours\_O.txt for open contours, try

grdcontour topo.nc -C100 -S4 -Dcontours\_%c.txt

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_ ,
`*grdimage*\ (1) <grdimage.html>`_ , `*grdview*\ (1) <grdview.html>`_ ,
`*pscontour*\ (1) <pscontour.html>`_
