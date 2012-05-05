******
psrose
******


psrose - Plot a polar histogram (rose, sector, windrose diagrams)

`Synopsis <#toc1>`_
-------------------

**psrose** [ *table* ] [ **-A**\ *sector\_width*\ [**r**\ ] ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ [*mode\_file*\ ] ]
[ **-D** ] [ **-I** ] [ **-G**\ *fill* ] [ **-I** ] [ **-K** ] [
**-L**\ [*wlabel*/*elabel*/*slabel*/*nlabel*] ] [ **-M**\ *parameters* ]
[ **-O** ] [ **-P** ] [ **-R**\ *r0*/*r1*/*az\_0*/*az\_1* ] [
**-S**\ *radial\_scale*\ [**n**\ ] ] [ **-T** ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [ **-W**\ [**v**\ ]\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-Z**\ *scale* ] [ **-bi**\ [*ncol*\ ][**t**\ ] ] [
**-c**\ *copies* ] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**psrose** reads (length,azimuth) pairs from *file* [or standard input]
and generates *PostScript* code that will plot a windrose diagram.
Optionally (with **-A**), polar histograms may be drawn (sector diagram
or rose diagram). Options include full circle and half circle plots. The
*PostScript* code is written to standard output. The outline of the
windrose is drawn with the same color as **MAP\_DEFAULT\_PEN**.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

None.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncol*\ ][**t**\ ]) data
    table file(s) holding a number of data columns. If no tables are
    given then we read from standard input.
**-A**\ *sector\_width*\ [**r**\ ]
    Gives the sector width in degrees for sector and rose diagram.
    [Default 0 means windrose diagram]. Append **r** to draw rose
    diagram instead of sector diagram.
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals. Remember that "x" here is radial
    distance and "y" is azimuth. The ylabel may be used to plot a figure
    caption.
**-C**\ [*mode\_file*\ ]
    Plot vectors showing the principal directions given in the *modes*
    file. If no file is given, compute and plot mean direction. See
    **-M** to control vector attributes.
**-D**
    Shift sectors so that they are centered on the bin interval (e.g.,
    first sector is centered on 0 degrees).
**-F**
    Do not draw the scale length bar [Default plots scale in lower right
    corner]
**-G**\ *fill*
    Selects shade, color or pattern for filling the sectors [Default is
    no fill].
**-I**
    Inquire. Computes statistics needed to specify useful **-R**. No
    plot is generated.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**\ [*wlabel*/*elabel*/*slabel*/*nlabel*]
    Specify labels for the 0, 90, 180, and 270 degree marks. For
    full-circle plot the default is WEST/EAST/SOUTH/NORTH and for
    half-circle the default is 90W/90E/-/0. A - in any entry disables
    that label. Use **-L** with no argument to disable all four labels
**-M**\ *parameters*
    Used with **-C** to modify vector parameters. For vector heads,
    append vector head *size* [Default is 0, i.e., a line]. See VECTOR
    ATTRIBUTES for specifying additional attributes.
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-R**\ *r0*/*r1*/*az\_0*/*az\_1*
    Specifies the ’region’ of interest in (r,azimuth) space. r0 is 0, r1
    is max length in units. For azimuth, specify -90/90 for half circle
    plot or 0/360 for full circle.
**-S**\ *radial\_scale*\ [**n**\ ]
    Specifies radius of circle. Append **n** to normalize input radii to
    go from 0 to 1.
**-T**
    Specifies that the input data is orientation data (has a 180 degree
    ambiguity) instead of true 0-360 degree directions [Default].
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**\ *pen*
    Set pen attributes for sector outline or rose plot. [Default is no
    outline]. Use **-Wv**\ *pen* to change pen used to draw vector
    (requires **-C**) [Default is same as sector outline].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]] (\*)
    Shift plot origin.
**-Z**\ *scale*
    Multiply the data radii by *scale*. E.g., use **-Z**\ 0.001 to
    convert your data from m to km [Default is no scaling].
**-:**
    Input file has (azimuth,radius) pairs rather than the expected
    (radius,azimuth).
**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] (\*)
    Select input columns.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*] (\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

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

To plot a half circle rose diagram of the data in the file
fault\_segments.az\_r (containing pairs of (azimuth, length in meters),
using a 10 degree bin sector width, on a circle of radius = 3 inch, grid
going out to radius = 150 km in steps of 25 km with a 30 degree sector
interval, radial direction annotated every 50 km, using a light blue
shading outlined by a solid red pen (width = 0.75 points), draw the mean
azimuth, and shown in Portrait orientation, use:

psrose fault\_segments.az\_r -R0/150/-90/90 -B50g25:"Fault
length":/g30:."Rose diagram": **-S**\ 3\ **i** -A10r -Glightblue
-W0.75p,red -Z0.001 -C -P -T -: \| lpr

To plot a full circle wind rose diagram of the data in the file
lines.r\_az, on a circle of radius = 5 cm, grid going out to radius =
500 units in steps of 100 with a 45 degree sector interval, using a
solid pen (width = 0.5 point, and shown in landscape [Default]
orientation with UNIX timestamp and command line plotted, use:

psrose lines.az\_r -R0/500/0/360 **-S**\ 5\ **c** -Bg100/g45:."Windrose
diagram": -W0.5p -Uc \| lpr

`Bugs <#toc8>`_
---------------

No default radial scale and grid settings for polar histograms. User
must run **psrose** **-I** to find max length in binned data set.

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*gmt.conf*\ (5) <gmt.conf.5.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.5.html>`_ ,
`*pshistogram*\ (1) <pshistogram.1.html>`_

