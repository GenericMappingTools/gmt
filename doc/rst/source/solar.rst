.. index:: ! solar

*****
solar
*****

.. only:: not man

    Calculate and plot the day-night terminator

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt solar** 
[ |SYN_OPT-B| ]
[ |-C| ]
[ |-G|\ *fill*\ \|\ **c** ]
[ |-I|\ [*lon/lat*][**+d**\ *date*][**+z**\ *TZ*] ]
[ |-J|\ *parameters* ]
[ |-M| ]
[ |-N| ]
[ |-Q| ]
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

Examples
--------

Print current Sun position and Sunrise, Sunset times at:

   ::

    gmt solar -I-7.93/37.079+d2016-02-04T10:01:00

Plot the day-night and civil twilight 

   ::

    gmt begin
    gmt coast -Rd -W0.1p -JQ0/14c -Ba -BWSen -Dl -A1000
    gmt solar -W1p -Tdc
    gmt end

Set up a clip path overlay based on the day/night terminator: 

   ::

    gmt solar -Gc -Tc


.. include:: solar_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`clip`, :doc:`coast`, :doc:`plot`
