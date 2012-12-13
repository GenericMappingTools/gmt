****
psxy
****

psxy - Plot lines, polygons, and symbols on maps

`Synopsis <#toc1>`_
-------------------

**psxy** [ *table* ] **-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] [
**-A**\ [**m**\ \|\ **p**] ] [ **-B**\ [**p**\ \|\ **s**]\ *parameters*
] [ **-C**\ *cptfile* ] [ **-D**\ *dx*/*dy* ] [
**-E**\ [**x**\ \|\ **y**\ \|\ **X**\ \|\ **Y**][**n**\ ][*cap*\ ][/[\ **-**\ \|\ **+**]\ *pen*]
] [ **-G**\ *fill* ] [ **-I**\ *intens* ] [
**-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [ **-L** ] [ **-N** ] [
**-O** ] [ **-P** ] [
**-S**\ [*symbol*\ ][\ *size*\ \|\ **+s**\ *scaling*] ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [ **-W**\ [**-**\ \|\ **+**][*pen*\ ] ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-a**\ *col*\ =\ *name*\ [*...*\ ]] [
**-bi**\ [*ncols*\ ][*type*\ ] ] [ **-c**\ *copies* ] [
**-f**\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**psxy** reads (*x*,\ *y*) pairs from *files* [or standard input] and
generates *PostScript* code that will plot lines, polygons, or symbols
at those locations on a map. If a symbol is selected and no symbol size
given, then **psxy** will interpret the third column of the input data
as symbol size. Symbols whose *size* is <= 0 are skipped. If no symbols
are specified then the symbol code (see **-S** below) must be present as
last column in the input. If **-S** is not used, a line connecting the
data points will be drawn instead. To explicitly close polygons, use
**-L**. Select a fill with **-G**. If **-G** is set, **-W** will control
whether the polygon outline is drawn or not. If a symbol is selected,
**-G** and **-W** determines the fill and outline/no outline,
respectively. The *PostScript* code is written to standard output.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

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
then we read from standard input. Use **-T** to ignore all input files,
including standard input (see below).

**-A**\ [**m**\ \|\ **p**]

By default line segments are drawn as great circle arcs. To draw them as
straight lines, use the **-A** flag. Alternatively, add **m** to draw
the line by first following a meridian, then a parallel. Or append **p**
to start following a parallel, then a meridian. (This can be practical
to draw a lines along parallels, for example).

**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)

Set map boundary intervals.

**-C**\ *cptfile*

Give a color palette file. If **-S** is set, let symbol fill color be
determined by the z-value in the third column. Additional fields are
shifted over by one column (optional size would be 4th rather than 3rd
field, etc.). If **-S** is not set, then **psxy** expects the user to
supply a multisegment file where each segment header contains a
**-Z**\ *val* string. The *val* will control the color of the line or
polygon (if **-L** is set) via the cpt file.

**-D**\ *dx*/*dy*

Offset the plot symbol or line locations by the given amounts *dx/dy*
[Default is no offset]. If *dy* is not given it is set equal to *dx*.

**-E**\ [**x**\ \|\ **y**\ \|\ **X**\ \|\ **Y**][**n**\ ][*cap*\ ][/[\ **-**\ \|\ **+**]\ *pen*]

Draw error bars. Append **x** and/or **y** to indicate which bars you
want to draw (Default is both x and y). The x and/or y errors must be
stored in the columns after the (x,y) pair [or (x,y,size) triplet]. The
*cap* parameter indicates the length of the end-cap on the error bars
[7**p**]. Pen attributes for error bars may also be set [Defaults: width
= default, color = black, style = solid]. A leading **+** will use the
lookup color (via **-C**) for both symbol fill and error pen color,
while a leading **-** will set error pen color and turn off symbol fill.
If upper case **X** and/or **Y** is used we will instead draw
"box-and-whisker" (or "stem-and-leaf") symbols. The x (or y) coordinate
is then taken as the median value, and 4 more columns are expected to
contain the minimum (0% quantile), the 25% quantile, the 75% quantile,
and the maximum (100% quantile) values. The 25-75% box may be filled by
using **-G**. If **n** is appended to **X** (or **Y**) we draw a notched
"box-and-whisker" symbol where the notch width reflects the uncertainty
in the median. Then a 5th extra data column is expected to contain the
number of points in the distribution.

**-G**\ *fill*

Select color or pattern for filling of symbols or polygons [Default is
no fill].

Note that **psxy** will search for **-G** and **-W** strings in all the
segment headers and let any values thus found over-ride the command line
settings.

**-I**\ *intens*

Use the supplied *intens* value (nominally in the -1 to + 1 range) to
modulate the fill color by simulating illumination [none].

**-Jz**\ \|\ **Z**\ *parameters* (\*)

Set z-axis scaling; same syntax as **-Jx**.

**-K** (\*)

Do not finalize the *PostScript* plot.

**-L**

Force closed polygons: connect the endpoints of the line-segment(s) and
draw polygons. Also, in concert with **-C** and any **-Z** settings in
the headers will use the implied color for polygon fill [Default is
polygon pen color].

**-N**

Do NOT skip symbols that fall outside map border [Default plots points
inside border only]. The option does not apply to lines and polygons
which are always clipped to the map region.

**-O** (\*)

Append to existing *PostScript* plot.

**-P** (\*)

Select "Portrait" plot orientation.

**-S**\ [*symbol*\ ][\ *size*\ \|\ **+s**\ *scaling*]

Plot symbols. If present, *size* is symbol size in the unit set in
**gmt.conf** (unless **c**, **i**, or **p** is appended). If the symbol
code (see below) is not given it will be read from the last column in
the input data; this cannot be used in conjunction with
**-bi**\ [*ncols*\ ][*type*\ ]. Optionally, append **c**, **i**, or
**p** to indicate that the size information in the input data is in
units of cm, inch, or point, respectively [Default is
**PROJ\_LENGTH\_UNIT**]. Note: if you give both size and symbol via the
input file you must use **PROJ\_LENGTH\_UNIT** to indicate the units
used for the symbol size. If the symbol size is expected via the third
data column then you may convert those values to symbol sizes by
appending **+s**\ *scale*\ [/*origin*][**l**\ ] which will compute size
= (data - *origin*) \* *scale*, where *origin* defaults to 0. If **l**
is appended we take log10 of data and *origin* first [*origin* then
defaults to 1].

The uppercase symbols **A**, **C**, **D**, **G**, **H**, **I**, **N**,
**S**, **T** are normalized to have the same area as a circle with
diameter *size*, while the size of the corresponding lowercase symbols
refers to the diameter of a circumscribed circle.

Choose between these symbol codes:

**-S-**

x-dash (-). *size* is the length of a short horizontal (x-dir) line
segment.

**-S+**

plus (+). *size* is diameter of circumscribing circle.

**-Sa**

st\ **a**\ r. *size* is diameter of circumscribing circle.

**-Sb**

Vertical **b**\ ar extending from *base* to y. *size* is bar width.
Append **u** if *size* is in x-units [Default is plot-distance units].
By default, *base* = ymin. Append **b**\ [*base*\ ] to change this
value. If *base* is not appended then we read it from the last input
data column.

**-SB**

Horizontal **b**\ ar extending from *base* to x. *size* is bar width.
Append **u** if *size* is in y-units [Default is plot-distance units].
By default, *base* = xmin. Append **b**\ [*base*\ ] to change this
value. If *base* is not appended then we read it from the last input
data column.

**-Sc**

**c**\ ircle. *size* is diameter of circle.

**-Sd**

**d**\ iamond. *size* is diameter of circumscribing circle.

**-Se**

**e**\ llipse. Direction (in degrees counter-clockwise from horizontal),
major\_axis, and minor\_axis must be found in columns 3, 4, and 5.

**-SE**

Same as **-Se**, except azimuth (in degrees east of north) should be
given instead of direction. The azimuth will be mapped into an angle
based on the chosen map projection (**-Se** leaves the directions
unchanged.) Furthermore, the axes lengths must be given in km instead of
plot-distance units. An exception occurs for a linear projection in
which we assume the ellipse axes are given in the same units as **-R**.

**-Sf**

**f**\ ront.
**-Sf**\ *gap/size*\ [**+l**\ \|\ **+r**][**+b+c+f+s+t**\ ][\ **+o**\ *offset*].
Supply distance gap between symbols and symbol size. If *gap* is
negative, it is interpreted to mean the number of symbols along the
front instead. Append **+l** or BD+r) to plot symbols on the left or
right side of the front [Default is centered]. Append **+**\ *type* to
specify which symbol to plot: **b**\ ox, **c**\ ircle, **f**\ ault,
**s**\ lip, or **t**\ riangle. [Default is **f**\ ault]. Slip means
left-lateral or right-lateral strike-slip arrows (centered is not an
option). Append **+o**\ *offset* to offset the first symbol from the
beginning of the front by that amount [0]. Note: By placing **-Sf**
options in the segment header you can change the front types on a
segment-by-segment basis.

**-Sg**

octaBD(g)on. *size* is diameter of circumscribing circle.

**-Sh**

**h**\ exagon. *size* is diameter of circumscribing circle.

**-Si**

**i**\ nverted triangle. *size* is diameter of circumscribing circle.

**-Sj**

Rotated rectangle. Direction (in degrees counter-clockwise from
horizontal), x-dimension, and y-dimension must be found in columns 3, 4,
and 5.

**-SJ**

Same as **-Sj**, except azimuth (in degrees east of north) should be
given instead of direction. The azimuth will be mapped into an angle
based on the chosen map projection (**-Sj** leaves the directions
unchanged.) Furthermore, the dimensions must be given in km instead of
plot-distance units. An exception occurs for a linear projection in
which we assume the dimensions are given in the same units as **-R**.

**-Sk**

**k**\ ustom symbol. Append <name>/*size*, and we will look for a
definition file called <name>.def `in (1) <in.html>`_ the current
directory `or (2) <or.2.html>`_ in ~/.gmt `or (3) <or.html>`_ in
**$GMT\_SHAREDIR**/custom. The symbol as defined in that file is of size
1.0 by default; the appended *size* will scale symbol accordingly. Users
may add their own custom \*.def files; see CUSTOM SYMBOLS below.

**-Sl**

**l**\ etter or text string (less than 64 characters). Give size, and
append /*string* after the size. Note that the size is only approximate;
no individual scaling is done for different characters. Remember to
escape special characters like \*. Optionally, you may append %\ *font*
to select a particular font [Default is **FONT\_ANNOT\_PRIMARY**].

**-Sm**

**m**\ ath angle arc, optionally with one or two arrow heads [Default is
no arrow heads]. The *size* is the length of the vector head. Arc width
is set by **-W**. The radius of the arc and its start and stop
directions (in degrees counter-clockwise from horizontal) must be given
in columns 3-5. See VECTOR ATTRIBUTES for specifying attributes.

**-SM**

Same as **-Sm** but switches to straight angle symbol if angles subtend
90 degrees exactly.

**-Sn**

`peBD(n) <peBD.n.html>`_ tagon. *size* is diameter of circumscribing
circle.

**-Sp**

**p**\ oint. No size needs to be specified (1 pixel is used).

**-Sq**

**q**\ uoted line, i.e., lines with annotations such as contours. Append
[**d**\ \|\ **D**\ \|\ **f**\ \|\ **l**\ \|\ **L**\ \|\ **n**\ \|\ **x**\ \|\ **X**]\ *info*\ [:*labelinfo*].
The required argument controls the placement of labels along the quoted
lines. Choose among five controlling algorithms:

    **d**\ *dist*\ [**c**\ \|\ **i**\ \|\ **p**] or
    **D**\ *dist*\ [**d**\ \|\ **e**\ \|\ **f**\ \|\ **k**\ \|\ **m**\ \|\ **M**\ \|\ **n**\ \|\ **s**]
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
    **f**\ *ffile.d*
    Reads the ascii file *ffile.d* and places labels at locations in the
    file that matches locations along the quoted lines. Inexact matches
    and points outside the region are skipped.
    **l\|L**\ *line1*\ [,*line2*,...]
    Give *start* and *stop* coordinates for one or more comma-separated
    straight line segments. Labels will be placed where these lines
    intersect the quoted lines. The format of each *line* specification
    is *start/stop*, where *start* and *stop* are either a specified
    point *lon/lat* or a 2-character **XY** key that uses the
    justification format employed in **pstext** to indicate a point on
    the map, given as [LCR][BMT]. **L** will interpret the point pairs
    as defining great circles [Default is straight line].
    **n**\ *n\_label*
    Specifies the number of equidistant labels for quoted lines line
    [1]. Upper case **N** starts labeling exactly at the start of the
    line [Default centers them along the line]. **N**-1 places one
    justified label at start, while **N**\ +1 places one justified label
    at the end of quoted lines. Optionally, append
    /*min\_dist*\ [**c**\ \|\ **i**\ \|\ **p**] to enforce that a
    minimum distance separation between successive labels is enforced.
    **x\|X**\ *xfile.d*
    Reads the multisegment file *xfile.d* and places labels at the
    intersections between the quoted lines and the lines in *xfile.d*.
    **X** will resample the lines first along great-circle arcs.
    In addition, you may optionally append
    **+r**\ *radius*\ [**c**\ \|\ **i**\ \|\ **p**] to set a minimum
    label separation in the x-y plane [no limitation].

    The optional *labelinfo* controls the specifics of the label
    formatting and consists of a concatenated string made up of any of
    the following control arguments:

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

Note: By placing **-Sq** options in the segment header you can change
the quoted text attributes on a segment-by-segment basis.

**-Sr**
    **r**\ ectangle. No size needs to be specified, but the x- and
    y-dimensions must be found in columns 3 and 4.
**-SR**
    **R**\ ounded rectangle. No size needs to be specified, but the x-
    and y-dimensions and corner radius must be found in columns 3, 4,
    and 5.
**-Ss**
    **s**\ quare. *size* is diameter of circumscribing circle.
**-St**
    **t**\ riangle. *size* is diameter of circumscribing circle.
**-Sv**
    **v**\ ector. Direction (in degrees counter-clockwise from
    horizontal) and length must be found in columns 3 and 4. The *size*
    is the length of the vector head. Vector width is set by **-W**. See
    VECTOR ATTRIBUTES for specifying attributes.
**-SV**
    Same as **-Sv**, except azimuth (in degrees east of north) should be
    given instead of direction. The azimuth will be mapped into an angle
    based on the chosen map projection (**-Sv** leaves the directions
    unchanged.) See VECTOR ATTRIBUTES for specifying attributes.
**-Sw**
    pie **w**\ edge. Start and stop directions (in degrees
    counter-clockwise from horizontal) for pie slice must be found in
    columns 3 and 4.
**-SW**
    Same as **-Sw**, except azimuths (in degrees east of north) should
    be given instead of the two directions. The azimuths will be mapped
    into angles based on the chosen map projection (**-Sw** leaves the
    directions unchanged.)
**-Sx**
    cross (x). *size* is diameter of circumscribing circle.
**-Sy**
    y-dash (\|). *size* is the length of a short vertical (y-dir) line
    segment.
**-S=**
    geovector. Azimuth (in degrees east from north) and length (in km)
    must be found in columns 3 and 4. The *size* is the length of the
    vector head. Vector width is set by **-W**. See VECTOR ATTRIBUTES
    for specifying attributes.
**-T**
    Ignore all input files, including standard input. This is the same
    as specifying /dev/null (or NUL for Windows users) as input file.
    Use this to activate only the options that are not related to
    plotting of lines or symbols, such as **psxy** **-R** **-J** **-O**
    **-T** to terminate a sequence of **GMT** plotting commands without
    producing any plotting output.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**\ [**-**\ \|\ **+**][*pen*\ ]
    Set pen attributes for lines or the outline of symbols [Defaults:
    width = default, color = black, style = solid]. A leading **+** will
    use the lookup color (via **-C**) for both symbol fill and outline
    pen color, while a leading **-** will set outline pen color and turn
    off symbol fill.
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is the required number of columns
    given the chosen settings].
**-a**\ *col*\ =\ *name*\ [*...*\ ] (\*)
    Set aspatial column associations *col*\ =\ *name*.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks. The **-g** option is ignored if
    **-S** is set.
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

`Vector Attributes <#toc6>`_
----------------------------

Several modifiers may be appended to the vector-producing options to
specify the placement of vector heads, their shapes, and the
justification of the vector:

**+a**\ *angle* sets the angle of the vector head apex [30].

**+b** places a vector head at the beginning of the vector path [none].

**+e** places a vector head at the end of the vector path [none].

**+g**-\|\ *fill* turns off vector head fill (if -) or sets the vector
head fill [Default fill is used, which may be no fill].

**+l** draws half-arrows, using only the left side [both].

**+n**\ *norm* scales down vector attributes (pen thickness, head size)
with decreasing length, where vectors shorter than *norm* will have
their attributes scaled by length/\ *norm* [arrow attributes remains
invariant to length].

**+p**\ [-][*pen*\ ] sets the vector pen attributes. If *pen* has a
leading - then the head outline is not drawn. [Default pen is used, and
head outline is drawn]

**+r** draws half-arrows, using only the right side [both].

In addition, all but circular vectors may take these modifiers:

**+j**\ *just* determines how the input *x*,\ *y* point relates to the
vector. Choose from **b**\ eginning [default], **e**\ nd, or
**c**\ enter.

**+s** means the input *angle*, *length* is instead the *x*, *y*
coordinates of the vector end point.

`Examples <#toc7>`_
-------------------

To plot solid red circles (diameter = 0.25 cm) at the positions listed
in the file DSDP.xy on a Mercator map at 5 cm/degree of the area 150E to
154E, 18N to 23N, with tickmarks every 1 degree and gridlines every 15
minutes, use

**psxy** DSDP.xy **-R**\ 150/154/18/23 **-Jm**\ 5\ **c**
**-Sc**\ 0.25\ **c** OPR(G)red **-B**\ 1\ **g**\ 15\ **m** > map.ps

To plot the xyz values in the file quakes.xyzm as circles with size
given by the magnitude in the 4th column and color based on the depth in
the third using the color palette cpt on a linear map, use

**psxy** quakes.xyzm **-R**\ 0/1000/0/1000 **-JX**\ 6\ **i** **-Sc**
**-C**\ cpt **-B**\ 200 > map.ps

To plot the file trench.xy on a Mercator map, with white triangles with
sides 0.25 inch on the left side of the line, spaced every 0.8 inch, use

**psxy** trench.xy **-R**\ 150/200/20/50 **-Jm**\ 0.15\ **i**
**-Sf**\ 0.8\ **i**/0.1\ **ilt** **-G**\ white **-W** **-B**\ 10 >
map.ps

To plot the data in the file misc.d as symbols determined by the code in
the last column, and with size given by the magnitude in the 4th column,
and color based on the third column via the color palette cpt on a
linear map, use

**psxy** misc.d **-R**\ 0/100/-50/100 **-JX**\ 6\ **i** **-S**
**-C**\ cpt **-B**\ 20 > map.ps

`Segment Header Parsing <#toc8>`_
---------------------------------

Segment header records may contain one of more of the following options:

**-G**\ *fill*
    Use the new *fill* and turn filling on
**-G-**
    Turn filling off
**-G**
    Revert to default fill (none if not set on command line)
**-W**\ *pen*
    Use the new *pen* and turn outline on
**-W**
    Revert to default pen **MAP\_DEFAULT\_PEN** (if not set on command
    line)
**-W-**
    Turn outline off
**-Z**\ *zval*
    Obtain fill via cpt lookup using z-value *zval*
**-ZNaN**
    Get the NaN color from the cpt file

`Custom Symbols <#toc9>`_
-------------------------

**psxy** allows users to define and plot their own custom symbols. This
is done by encoding the symbol using our custom symbol macro code
described in Appendix N. Put all the macro codes for your new symbol in
a file whose extension must be .def; you may then address the symbol
without giving the extension (e.g., the symbol file tsunami.def is used
by specifying **-Sk**\ *tsunami/size*. The definition file can contain
any number of plot code records, as well as blank lines and comment
lines (starting with #). **psxy** will look for the definition files `in
(1) <in.html>`_ the current directory, (2) the ~/.gmt directory, `and
(3) <and.html>`_ the **$GMT\_SHAREDIR**/custom directory, in that order.
Freeform polygons (made up of straight line segments and arcs of
circles) can be designed - these polygons can be painted and filled with
a pattern. Other standard geometric symbols can also be used. See
Appendix N for macro definitions.

**psxy** cannot handle filling of polygons that contain the south or
north pole. For such a polygon, make a copy and split it into two and
make each explicitly contain the polar point. The two polygons will
combine to give the desired effect when filled; to draw outline use the
original polygon.

`See Also <#toc10>`_
--------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_ , `*psxyz*\ (1) <psxyz.html>`_
