********
pswiggle
********

pswiggle - Plot z = f(x,y) anomalies along tracks

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

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
**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
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

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

.. include:: explain_-J.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

**-Z**\ *scale*
    Gives anomaly scale in data-units/distance-unit.

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

**-A**\ *azimuth*
    Sets the preferred positive azimuth. Positive wiggles will
    "gravitate" towards that direction. 

.. include:: explain_-B.rst_

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

.. include:: explain_-Jz.rst_

.. include:: explain_-K.rst_

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-S**\ [**x**\ ]\ *lon0*/*lat0*/*length*\ [/*units*]
    Draws a simple vertical scale centered on *lon0/lat0*. Use **-Sx**
    to specify cartesian coordinates instead. *length* is in z units,
    append unit name for labeling. **FONT\_ANNOT\_PRIMARY** is used as
    font.
**-T**\ *pen*
    Draw track [Default is no track]. Append pen attributes to use
    [Defaults: width = 0.25p, color = black, style = solid]. 

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-W**\ *pen*

.. |Add_-bi| replace:: [Default is 3 input columns]. 
.. include:: explain_-bi.rst_

.. include:: explain_-c.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

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

    awk '{ if (NR > 1) print atan2(y-$1, x-$2); y=$1; x=$2; }' yourdata.xyz \| more

(note that output is in radians; on some machines you need "nawk" to do
this). Then if these numbers jump around a lot, you may do this:

    awk '{ print NR, $0 }’ yourdata.xyz \| filter1d -Fb5 -N4/0
    --FORMAT\_FLOAT\_OUT=%.12g > smoothed.xyz

and plot this data set instead.

`See Also <#toc8>`_
-------------------

`gmt <gmt.html>`_, `gmtcolors <gmtcolors.html>`_,
`filter1d <filter1d.html>`_,
`psbasemap <psbasemap.html>`_,
`splitxyz <splitxyz.html>`_
