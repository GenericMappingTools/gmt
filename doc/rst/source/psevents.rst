.. index:: ! psevents

********
psevents
********

.. only:: not man

    Construct event symbols for making movies

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt psevents** [ *table* ] |-J|\ *parameters* |SYN_OPT-Rz| |-S|\ *symbol*\ [*size*\ [*units*]]
[ |-A|\ *magnify*\ [**+c**\ *magnify2*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *duration*\ \|\ **t** ]
[ |-E|\ [**+r**\ *dt*][**+p**\ *dt*][**+d**\ *dt*][**+f**\ *dt*] ]
[ |-F|\ [*transparency*]\ [**+c**\ *transparency2*] ]
[ |-G|\ *color* ]
[ |-I|\ [*intensity*]\ [**+c**\ *intensity2*] ]
[ |-K| ]
[ |-O| ] [ **-P** ]
[ |-Q|\ *file* ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-U| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: wiggle_common.rst_

.. include:: common_classic.rst_

Examples
--------

To be added, such as

   ::

    gmt psevents track.xym -R-20/10/-80/-60 -JS0/90/15c -Z500 -B5 \
                 -C32000 -P -Gred -T0.25p,blue -DjRM+w1000+lnT -V > track_xym.ps

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`psxy`
