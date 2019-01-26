.. index:: ! pssolar

*******
pssolar
*******

.. only:: not man

    pssolar - Calculate and plot the day-night terminator

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**pssolar** 
[ |SYN_OPT-B| ]
[ |-C| ]
[ |-G|\ *fill*\ \|\ **c** ]
[ |-I|\ [*lon/lat*][**+d**\ *date*][**+z**\ *TZ*] ]
[ |-J|\ *parameters* ]
[ |-K| ]
[ |-M| ]
[ |-N| ]
[ |-O| ] [|-P| ] [ |-Q| ]
[ |SYN_OPT-R| ]
[ |-T|\ **dcna**\ [**+d**\ *date*][**+z**\ *TZ*]]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: solar_common.rst_

.. include:: common_classic.rst_

Examples
--------

Print current Sun position and Sunrise, Sunset times at:

   ::

    gmt pssolar -I-7.93/37.079+d2016-02-04T10:01:00

Plot the day-night and civil twilight 

   ::

    gmt pscoast -Rd -W0.1p -JQ0/14c -Ba -BWSen -Dl -A1000 -P -K > terminator.ps

    gmt pssolar -R -J -W1p -Tdc -O >> terminator.ps

Set up a clip path overlay based on the day/night terminator: 

   ::

    gmt pssolar -R -J -Gc -Tc -O -K >> someplot.ps


.. include:: solar_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`psclip`, :doc:`pscoast`, :doc:`psxy`
