.. index:: ! events

********
events
********

.. only:: not man

    Plot event symbols and labels for a moment in time

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt events** [ *table* ] |-J|\ *parameters* |SYN_OPT-Rz| |-S|\ *symbol*\ [*size*\ [*units*]]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ [**j**\ \|\ **J**]\ *dx*\ [/*dy*][\ **+v**\ [*pen*]] ]
[ |-E|\ **s**\ \|\ **t**\ [**+o**\ *dt*][**+r**\ *dt*][**+p**\ *dt*][**+d**\ *dt*][**+f**\ *dt*] ]
[ |-F|\ [**+a**\ *angle*][\ **+f**\ *font*][\ **+j**\ *justify*][\ **+r**\ [*first*]\ \|\ **+z**\ [*format*]] ] 
[ |-G|\ *color* ]
[ |-L|\ [*length*\ \|\ **t**\ ] ]
[ |-M|\ **i**\ \|\ **s**\ \|\ **t**\ [*val1*]\ [**+c**\ *val2*] ]
[ |-Q|\ *prefix* ]
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

    gmt events quakes.txt -R-20/10/-80/-60 -JS0/90/15c -Ms5 -B5 \
                 -Cseis.cpt -Es+r0.1+d0.1+f5 -T44.5 -V

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`plot`,
:doc:`movie`
