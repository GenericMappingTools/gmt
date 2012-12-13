********
pslegend
********

pslegend - Plot legends on maps

`Synopsis <#toc1>`_
-------------------

**pslegend** [ *textfile* ]
**-D**\ [**x**\ ]\ *lon*/*lat*/*width*/*height*/*just*\ [/*dx*/*dy*] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *dx*/*dy* ] [
**-F**\ [**+r**\ [*radius*\ ]] ] [ **-G**\ *fill* ] [
**-J**\ *parameters* ] [ **-K** ] [ **-L**\ *spacing* ] [ **-O** ] [
**-P** ] [ **-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-c**\ *copies* ] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**pslegend** will make legends that can be overlaid on maps. It reads
specific legend-related information from an input file [or stdin].
Unless otherwise noted, annotations will be made using the primary
annotation font and size in effect (i.e., FONT\_ANNOT\_PRIMARY)

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

`Optional Arguments <#toc5>`_
-----------------------------

*textfile*

This file contains instruction for the layout of items in the legend.
Each legend item is described by a unique record. All records begin with
a unique character that is common to all records of the same kind. The
order of the legend items is implied by the order of the records. Ten
different record types are recognized, and the syntax for each of these
records are presented below:

    **#** *comment* Records starting with # and blank lines are skipped.
    **B** *cptname offset height* [ *optional arguments* ]
    The **B** record will plot a horizontal color bar, **psscale**-style
    in the middle, starting at *offset* from the left edge, and of the
    given *height*. You may add any additional **psscale** options from
    the list: **-A** **-B** **-I** **-L** **-M** **-N** **-S** **-Z**
    and **-p**. See the **psscale** man page for details.
    **C** *textcolor*
    The **C** record specifies the color with which the remaining text
    is to be printed. *textcolor* can be in the form *r/g/b*, *c/m/y/k*,
    or a named color. Use **-** to reset to default color.
    **D** *offset pen*
    The **D** record results in a horizontal line with specified *pen*
    across the legend with one quarter of the line spacing left blank
    above and below the line. Two gaps of *offset* units are left blank
    between the horizontal line and the left and right frame sides. If
    no pen is given we use MAP\_GRID\_PEN\_PRIMARY
    **G** *gap*
    The **G** record specifies a vertical gap of the given length. In
    addition to the standard units (**i**, **c**, **p**) you may use
    **l** for lines. A negative *gap* will move the current line upwards
    (thus closing a gap).
    **H** *fontsize*\ \|\ **-** *font*\ \|\ **-** header)
    The **H** record plots a centered text string using the specified
    font parameters. Use **-** to default to size and type of
    **FONT\_TITLE**.
    **I** *imagefile width justification*
    Place an EPS or Sun raster image in the legend justified relative to
    the current point. The image *width* determines the size of the
    image on the page.
    **L** *fontsize*\ \|\ **-** *font*\ \|\ **-** justification label)
    The **L** record plots a (L)eft, (C)entered, or (R)ight-justified
    text string within a column using the specified font parameters. Use
    **-** to default to the size and type of **FONT\_LABEL**.
    **M** *slon*\ \|\ **-** *slat length* **f**\ \|\ **p** [
    **-R**\ *w/e/s/n* **-J**\ *param* ]
    Place a map scale in the legend. Specify *slon slat*, the point on
    the map where the scale applies (*slon* is only meaningful for
    certain oblique projections. If not needed, you must specify **-**
    instead), *length*, the length of the scale in km (for other units
    append **e** (meter), **f** (foot), **M** (mile), **n** (nautical
    mile), or **u** (survey foot)), and **f** or **p** for fancy or
    plain scale. If the **-R** **-J** supplied to **pslegend** is
    different than the projection needed for the scale (or not given at
    all, i.e., with **-Dx**), supply the optional **-R** **-J** settings
    as well. Append **+l** to the *length* to select the default label
    which equals the distance unit (meter, feet, km, miles, nautical
    miles, survey feet) and is justified on top of the scale [t]. Change
    this by giving your own label (append **+l**\ *label*). Change label
    justification with **+j**\ *justification* (choose among l(eft),
    r(ight), `t(op) <t.op.html>`_ , and `b(ottom) <b.ottom.html>`_ ).
    Apply **+u** to append the unit to all distance annotations along
    the scale. If you want to place a rectangle behind the scale,
    specify suitable **+p**\ *pen* and/or **+f**\ *fill* parameters. All
    these )+)\ *modifiers* are appended to *length* to make a single
    string.
    **N** *ncolumns*
    Change the number of columns in the legend [1]. This only affects
    the printing of symbols (**S**) and labels (**L**). The number of
    columns stay in effect until **N** is used again.
    **S** *dx1 symbol size fill pen* [ *dx2 text* ]
    Plots the selected symbol with specified diameter, fill, and outline
    (see **psxy**). The symbol is centered at *dx1* from the left margin
    of the column, with the optional explanatory *text* starting *dx2*
    from the margin, printed with **FONT\_ANNOT\_PRIMARY**. Use **-** if
    no *fill* or outline (*pen*) is required. When plotting just a
    symbol, without text, *dx2* and *text* can be omitted. Two **psxy**
    symbols require special attention: front (**f**) and vector (**v**).
    You must prepend the length of the desired item to the rest of the
    symbol argument; this will be used internally to set the correct
    fault or vector length and will be stripped before passing the
    arguments to **psxy**.
    **T** *paragraph-text*
    One or more of these **T** records with *paragraph-text* printed
    with **FONT\_ANNOT\_PRIMARY**. To specify special positioning and
    typesetting arrangements, or to enter a paragraph break, use the
    optional **P** record.
    **V** *offset pen*
    The **V** record draws a vertical line between columns (if more than
    one) using the selected *pen* *offset* is analogous to the offset
    for the **D** records but in the vertical direction.
    **P** *paragraph-mode-header-for-pstext*
    Start a new text paragraph by specifying all the parameters needed
    (see **pstext -M** record description). Note that **pslegend** knows
    what all those values should be, so normally you can leave the
    entire record (after P) blank or leave it out all together. If you
    need to set at least one of the parameters directly, you must
    specify all and set the ones you want to leave at their default
    value to **-**.

**-D**\ [**x**\ ]\ *lon*/*lat*/*width*/*height*/*just*\ [/*dx*/*dy*]
    Positions the legend and specifies its size. The *just* is a 2-char
    justification string (see **pstext**) that relates the given
    position to a point on the rectangular legend box. If you want to
    specify the position in map plot units (i.e., inches or cm), use
    **-Dx**; in that case the **-R** and **-J** are optional (provided
    it is an overlay, i.e., we are using **-O**). Use to optional
    *dx*/*dy* to shift the legend by that amount in the direction
    implied by the justification.
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-C**\ *dx*/*dy*
    Sets the clearance between the legend frame and the internal items
    [4**p**/4**p**].
**-F**\ [**+i**\ [[*gap*/]*pen*]][\ **+p**\ *pen*][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*fill*\ ]]]
    Draws a rectangular border around the legend using
    **MAP\_FRAME\_PEN**; specify a different pen with **+p**\ *pen*.
    Append **+i** to draw a secondary, inner border as well. We use a
    *gap* between borders of 2\ **p** and the **MAP\_DEFAULTS\_PEN**
    unless other values are specified. Append **+r** to draw rounded
    rectangular borders instead, with a 6\ **p** corner radius. You can
    override this radius by appending another value. Finally, append
    **+s** to draw an offset background shaded region. Here, *dx*/*dy*
    indicates the shift relative to the foreground frame
    [4**p**/-4\ **p**] and *fill* sets the shading to use [SHADE].
**-G**\ *fill*
    Select fill shade, color or pattern of the legend box [Default is no
    fill].
**-J**\ *parameters* (\*)
    Select map projection.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**\ *spacing*
    Sets the linespacing factor in units of the current annotation font
    size [1.1].
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
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

To add an example of a legend to a Mercator plot (map.ps) with the given
specifications, use

pslegend -R-10/10/-10/10 -JM6i -Gazure1 -Dx0.5i/0.5i/5i/3.3i/BL
-C0.1i/0.1i -L1.2 -Fr -B5f1 << EOF >> map.ps

# Legend test for pslegend

# G is vertical gap, V is vertical line, N sets # of columns, D draws
horizontal line.

# H is header, L is label, S is symbol, T is paragraph text, M is map
scale.

#

G -0.1i

H 24 Times-Roman My Map Legend

D 0.2i 1p

N 2

V 0 1p

S 0.1i c 0.15i p300/12 0.25p 0.3i This circle is hachured

S 0.1i e 0.15i 255/255/0 0.25p 0.3i This ellipse is yellow

S 0.1i w 0.15i 0/255/0 0.25p 0.3i This wedge is green

S 0.1i f 0.25i/-1/0.075ilb 0/0/255 0.25p 0.3i This is a fault

S 0.1i - 0.15i - 0.25tap 0.3i A contour

S 0.1i v 0.25i/0.02i/0.06i/0.05i 255/0/255 0.25p 0.3i This is a vector

S 0.1i i 0.15i 0/255/255 0.25p 0.3i This triangle is boring

V 0 1p

D 0.2i 1p

N 1

M 5 5 600+u f

G 0.05i

I SOEST\_logo.ras 3i CT

G 0.05i

B colors.cpt 0.2i 0.2i

G 0.05i L 9 4 R Smith et al., @%5%J. Geophys. Res., 99@%%, 2000

G 0.1i

P

T Let us just try some simple text that can go on a few lines.

T There is no easy way to predetermine how many lines will be required,

T so we may have to adjust the box height to get the right size box.

EOF

`Windows Remarks <#toc7>`_
--------------------------

Note that under Windows, the percent sign (%) is a variable indicator
(like $ under Unix). To indicate a plain percentage sign in a batch
script you need to repeat it (%%); hence the font switching mechanism
(@%*font*\ % and @%%) may require twice the number of percent signs.
This only applies to text inside a script or that otherwise is processed
by DOS. Data files that are opened and read by **pslegend** do not need
such duplication.

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_ , `*pstext*\ (1) <pstext.html>`_ ,
`*psxy*\ (1) <psxy.html>`_
