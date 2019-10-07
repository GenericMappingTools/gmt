.. index:: ! wiggle

********
wiggle
********

.. only:: not man

    Plot z = f(x,y) anomalies along tracks

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt wiggle** [ *table* ] |-J|\ *parameters* |SYN_OPT-Rz| |-Z|\ *scale*\ [*units*]
[ |-A|\ [\ *azimuth*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *center* ]
[ |-D|\ *refpoint* ]
[ |-F|\ *panel* ]
[ |-G|\ *fill*\ [**+n**\ ][**+p**\ ] ]
[ |-I|\ *fix_az* ]
[ |-T|\ *pen* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: wiggle_common.rst_

Examples
--------

To plot the magnetic anomaly stored in the file track.xym along track @
500 nTesla/cm (after removing a mean value of 32000 nTesla), using a
15-cm-wide Polar Stereographic map ticked every 5 degrees in Portrait
mode, with positive anomalies in red on a blue track of width 0.25
points, use

   ::

    gmt wiggle track.xym -R-20/10/-80/-60 -JS0/90/15c -Z500 -B5
                 -C32000 -Gred -T0.25p,blue -DjRM+w1000+lnT -V -pdf track_xym

and the positive anomalies will in general point in the north direction.
We used **-D** to place a vertical scale bar indicating a 1000 nT anomaly.
To instead enforce a fixed azimuth of 45 for the positive wiggles, we add **-I**
and obtain

   ::

    gmt wiggle track.xym -R-20/10/-80/-60 -JS0/90/15c -Z1000 -B5
              -C32000 -Gred -I45 -T0.25p,blue -DjRM+w1000+lnT -V -pdf track_xym

Bugs
----

Sometimes the (x,y) coordinates are not printed with enough significant
digits, so the local perpendicular to the track swings around a lot. To
see if this is the problem, you should do this:

   ::

    awk '{ if (NR > 1) print atan2(y-$1, x-$2); y=$1; x=$2; }' yourdata.xyz | more

(note that output is in radians; on some machines you need "nawk" to do
this). Then if these numbers jump around a lot, you may do this:

   ::

    awk '{ print NR, $0 }' yourdata.xyz | filter1d -Fb5 -N4/0
    --FORMAT_FLOAT_OUT=%.12g > smoothed.xyz

which performs a 5-point boxcar filter, and plot this data set instead.

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`filter1d`,
:doc:`basemap`,
:doc:`splitxyz`
