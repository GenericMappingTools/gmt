.. index:: ! events

********
events
********

.. only:: not man

    Construct event symbols for making movies

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt events** [ *table* ] |-J|\ *parameters* |SYN_OPT-Rz| |-S|\ *symbol*\ [*size*\ [*units*]]
[ |-A|\ *magnify*\ [**+c**\ *magnify2*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *duration*\ \|\ **t**\ ] ]
[ |-E|\ [**+r**\ *dt*][**+p**\ *dt*][**+d**\ *dt*][**+f**\ *dt*] ]
[ |-F|\ [*transparency*]\ [**+c**\ *transparency2*] ]
[ |-G|\ *color* ]
[ |-I|\ [*intensity*]\ [**+c**\ *intensity2*] ]
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

.. include:: events_common.rst_

Examples
--------

To be added, such as

   ::

    gmt events track.xym -R-20/10/-80/-60 -JS0/90/15c -Z500 -B5
                 -C32000 -Gred -T0.25p,blue -DjRM+w1000+lnT -V -pdf track_xym

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`plot`,
:doc:`movie`
