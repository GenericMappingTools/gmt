.. index:: ! pslegend

********
pslegend
********

.. only:: not man

    pslegend - Plot legends on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**pslegend** [ *textfile* ]
**-D**\ [**x**\ ]\ *lon*/*lat*/*width*\ [/*height*]/\ *just*\ [/*dx*/*dy*]
[ **-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *dx*/*dy* ] [
**-F**\ [\ **+g**\ *fill*][**+i**\ [[*gap*/]*pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]] ] [
**-J**\ *parameters* ] [ **-K** ] [ **-L**\ *spacing* ] [ **-O** ] [
**-P** ] [ **-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-c**\ *copies* ] [
**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

|No-spaces|

Description
-----------

**pslegend** will make legends that can be overlaid on maps. It reads
specific legend-related information from an input file [or stdin].
Unless otherwise noted, annotations will be made using the primary
annotation font and size in effect (i.e., FONT\_ANNOT\_PRIMARY) 

`Required Arguments <#toc4>`_
-----------------------------

**-D**\ [**x**\ ]\ *lon*/*lat*/*width*\ [/*height*]/\ *just*\ [/*dx*/*dy*]
    Positions the legend and specifies its size. The *just* is a 2-char
    justification string (see :doc:`pstext`) that relates the given
    position to a point on the rectangular legend box. If you want to
    specify the position in map plot units (i.e., inches or cm), use
    **-Dx**; in that case the **-R** and **-J** are optional (provided
    it is an overlay, i.e., we are using **-O**). Use to optional
    *dx*/*dy* to shift the legend by that amount in the direction
    implied by the justification. If *height* is zero or not given then
    we estimate *height* based the expected vertical extent of the items
    to be placed. 

`Optional Arguments <#toc5>`_
-----------------------------

.. include:: explain_-B.rst_

**-C**\ *dx*/*dy*
    Sets the clearance between the legend frame and the internal items
    [4\ **p**/4\ **p**].
**-F**\ [\ **+g**\ *fill*][**+i**\ [[*gap*/]*pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]]
    Without further options, draws a rectangular border around the legend using
    **MAP\_FRAME\_PEN**; specify a different pen with **+p**\ *pen*.
    Add **+g**\ *fill* to fill the legend box [no fill]
    Append **+i** to draw a secondary, inner border as well. We use a
    *gap* between borders of 2\ **p** and the **MAP\_DEFAULTS\_PEN**
    unless other values are specified. Append **+r** to draw rounded
    rectangular borders instead, with a 6\ **p** corner radius. You can
    override this radius by appending another value. Finally, append
    **+s** to draw an offset background shaded region. Here, *dx*/*dy*
    indicates the shift relative to the foreground frame
    [4\ **p**/-4\ **p**] and *shade* sets the fill style to use for shading.

.. include:: explain_-J.rst_

.. include:: explain_-K.rst_

**-L**\ *spacing*
    Sets the linespacing factor in units of the current annotation font size [1.1]. 

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-XY.rst_

.. include:: explain_-c.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

`Pslegend Codes <#toc6>`_
-------------------------

*textfile*
    This file contains instruction for the layout of items in the
    legend. Each legend item is described by a unique record. All
    records begin with a unique character that is common to all records
    of the same kind. The order of the legend items is implied by the
    order of the records. Ten different record types are recognized, and
    the syntax for each of these records are presented below:
**#** *comment*
    Records starting with # and blank lines are skipped.
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
    **l** for lines. A negative *gap* will move the current line upwards (thus closing a gap).
**H** *fontsize*\ \|\ **-** *font*\ \|\ **-** header)
    The **H** record plots a centered text string using the specified
    font parameters. Use **-** to default to size and type of **FONT\_TITLE**.
**I** *imagefile width justification*
    Place an EPS or Sun raster image in the legend justified relative to
    the current point. The image *width* determines the size of the image on the page.
**L** *fontsize*\ \|\ **-** *font*\ \|\ **-** justification label)
    The **L** record plots a (L)eft, (C)entered, or (R)ight-justified
    text string within a column using the specified font parameters. Use
    **-** to default to the size and type of **FONT\_LABEL**.
**M** *slon*\ \|\ **-** *slat length* **f**\ \|\ **p** [ **-R**\ *w/e/s/n* **-J**\ *param* ]
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
    r(ight), t(op) , and b(ottom)).
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
    (see :doc:`psxy`). The symbol is centered at *dx1* from the left margin
    of the column, with the optional explanatory *text* starting *dx2*
    from the margin, printed with **FONT\_ANNOT\_PRIMARY**. Use **-** if
    no *fill* or outline (*pen*) is required. When plotting just a
    symbol, without text, *dx2* and *text* can be omitted. Two **psxy**
    symbols may take special modifiers: front (**f**) and vector (**v**). 
    You can append modifiers to the symbol and affect how the fronts and
    vectors are presented (see :doc:`psxy` man page for modifiers).
    **pslegend** will determine default settings for all modifiers and
    secondary arguments if not provided.  A few other symbols (the rectangles,
    ellipse, wedge, mathangle) may take more than a single argument size.
    If just a single size if given then **pslegend** will provide reasonable
    arguments to plot the symbol  (See `Defaults`_).
    Alternatively, combine the required
    arguments into a single, comma-separated string and use that as the
    symbol size (again, see :doc:`psxy` for details on the arguments needed).
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


Defaults
--------

When attributes are not provided, or extended symbol information (for symbols taking more than just an overall size) are
not given as comma-separated quantities, **pslegend** will provide the following defaults:

Front: Front symbol is left-side (here, that means upper side) box, with dimensions 30% of the given symbol size.

Vector: Head size is 30% of given symbol size.

Ellipse: Minor axis is 65% of major axis (the symbol size), with an azimuth of 0 degrees.

Rectangle: Height is 65% of width (the symbol size).

Rotated rectangle: Same, with a rotation of 30 degrees.

Rounded rectangle: Same as rectangle, but with corner radius of 10% of width.

Mathangle: Angles are -10 and 45 degrees, with arrow head size 30% of symbol size.

Wedge: Angles are -30 and 30 degrees.

Examples
--------

To add an example of a legend to a Mercator plot (map.ps) with the given
specifications, use

    gmt pslegend -R-10/10/-10/10 -JM6i -F+gazure1 -Dx0.5i/0.5i/5i/3.3i/BL \
    -C0.1i/0.1i -L1.2 -B5f1 << EOF >> map.ps

    # Legend test for pslegend

    # G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line.

    # H is header, L is label, S is symbol, T is paragraph text, M is map scale.

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

    I SOEST_logo.ras 3i CT

    G 0.05i

    B colors.cpt 0.2i 0.2i

    G 0.05i L 9 4 R Smith et al., @%5%J. Geophys. Res., 99@%%, 2000

    G 0.1i

    P

    T Let us just try some simple text that can go on a few lines.

    T There is no easy way to predetermine how many lines will be required,

    T so we may have to adjust the box height to get the right size box.

    EOF

Note On Legend Height
---------------------

As **-D** suggests, leaving the *height* off forces a calculation of the
expected height. This is an exact calculation except in the case of
legends that place paragraph text. Here we simply do a first-order
estimate of how many typeset lines might appear. Without access to font
metrics this estimate will occasionally be off by 1 line. If so, note
the reported height (with **-V**) and specify a slightly larger or
smaller height in **-D**.

Windows Remarks
---------------

Note that under Windows, the percent sign (%) is a variable indicator
(like $ under Unix). To indicate a plain percentage sign in a batch
script you need to repeat it (%%); hence the font switching mechanism
(@%*font*\ % and @%%) may require twice the number of percent signs.
This only applies to text inside a script or that otherwise is processed
by DOS. Data files that are opened and read by **pslegend** do not need
such duplication.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`psbasemap`, :doc:`pstext`,
:doc:`psxy`
