********
pswiggle
********

pswiggle - Plot z = f(x,y) anomalies along tracks

`Synopsis <#toc1>`_
-------------------

**pswiggle** [ *table* ] **-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
**-Z**\ *scale* [ **-A**\ *azimuth* ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *center* ] [
**-G**\ [**+**\ \|\ **-**\ \|\ **=**]\ *fill* ] [
**-Jz**\ \|\ **Z**\ *parameters* ] [ **-I**\ *fix\_az* ] [ **-K** ] [
**-O** ] [ **-P** ] [
**-S**\ [**x**\ ]\ *lon0*/*lat0*/*length*\ [/*units*] ] [ **-T**\ *pen*
] [ **-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [
**-V**\ [*level*\ ] ] [ **-W**\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [ **-c**\ *copies* ] [
**-f**\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**pswiggle** reads (*x*,\ *y*,\ *z*) triplets from files [or standard
input] and plots z as a function of distance along track. This means
that two consecutive (*x*,\ *y*) points define the local distance axis,
and the local *z* axis is then perpendicular to the distance axis. The
user may set a preferred positive anomaly plot direction, and if the
positive normal is outside the plus/minus 90 degree window around the
preferred direction, then 180 degrees are added to the direction. Either
the positive or the negative wiggle may be shaded. The resulting
*PostScript* code is written to standard output.

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
**-Z**\ *scale*
    Gives anomaly scale in data-units/distance-unit.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-A**\ *azimuth*
    Sets the preferred positive azimuth. Positive wiggles will
    "gravitate" towards that direction.
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-C**\ *center*
    Subtract *center* from the data set before plotting [0].
**-G**\ [**+**\ \|\ **-**\ \|\ **=**]\ *fill*
    Set fill shade, color or pattern for positive and/or negative
    wiggles [Default is no fill]. Optionally, prepend **+** to fill
    positive areas (this is the default behavior). Prepend **-** to fill
    negative areas. Prepend **=** to fill both positive and negative
    areas with the same fill.
**-I**\ *fix\_az*
    Set a fixed azimuth projection for wiggles [Default uses track
    azimuth, but see **-A**].
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-S**\ [**x**\ ]\ *lon0*/*lat0*/*length*\ [/*units*]
    Draws a simple vertical scale centered on *lon0/lat0*. Use **-Sx**
    to specify cartesian coordinates instead. *length* is in z units,
    append unit name for labeling. **FONT\_ANNOT\_PRIMARY** is used as
    font.
**-T**\ *pen*
    Draw track [Default is no track]. Append pen attributes to use
    [Defaults: width = 0.25p, color = black, style = solid].
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**\ *pen*
    Draw wiggle outline [Default is no outline]. Append pen attributes
    to use [Defaults: width = 0.25p, color = black, style = solid].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 3 input columns].
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
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

To plot the magnetic anomaly stored in the file track.xym along track @
1000 nTesla/cm (after removing a mean value of 32000 nTesla), using a
15-cm-wide Polar Stereographic map ticked every 5 degrees in Portrait
mode, with positive anomalies in red on a blue track of width 0.25
points, use

pswiggle track.xym -R-20/10/-80/-60 **-JS**\ 0/90/15\ **c** -Z1000 -B5
-C32000 -P -Gred -T0.25p,blue -S1000 -V > track\_xym.ps

`Bugs <#toc7>`_
---------------

Sometimes the (x,y) coordinates are not printed with enough significant
digits, so the local perpendicular to the track swings around a lot. To
see if this is the problem, you should do this:

awk ’{ if (NR > 1) print atan2(y-$1, x-$2); y=$1; x=$2; }’ yourdata.xyz
\| more

(note that output is in radians; on some machines you need "nawk" to do
this). Then if these numbers jump around a lot, you may do this:

awk ’{ print NR, $0 }’ yourdata.xyz \| filter1d -Fb5 -N4/0
--FORMAT\_FLOAT\_OUT=%.12g > smoothed.xyz

and plot this data set instead.

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*filter1d*\ (1) <filter1d.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_ ,
`*splitxyz*\ (1) <splitxyz.html>`_
