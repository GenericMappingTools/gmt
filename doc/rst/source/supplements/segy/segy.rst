.. index:: ! segy

******
segy
******

.. only:: not man

    segy - Plot a SEGY file on a map

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt segy** *SEGYfile* |-J|\ *parameters*
|SYN_OPT-R|
|-D|\ *deviation*
|-F|\ [*color*] |-W|
[ |-C|\ *clip* ]
[ |-E|\ *error* ] [ |-I| ] [ |-L|\ *nsamp* ]
[ |-M|\ *ntrace* ] [ |-N| ]
[ |-Q|\ *<mode><value>* ]
[ |-S|\ *header* ]
[ |-T|\ *filename* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: segy_common.rst_

Examples
--------

To plot the SEGY file wa1.segy with normalized traces plotted at true
offset locations, clipped at ±3 and with wiggle trace and positive
variable area shading in black, use

   ::

    gmt segy wa1.segy -JX5i/-5i -R0/100/0/10 -D1 -C3 -N -So -W -Fblack -pdf segy

To plot the SEGY file wa1.segy with traces plotted at true cdp\*0.1,
clipped at ±3, with bias -1 and negative variable area shaded red, use

   ::

    gmt segy wa1.segy -JX5i/-5i -R0/100/0/10 -D1 -C3 -Sc -Qx0.1 -Fred -Qb-1 -I -pdf segy

See Also
--------

:doc:`gmt </gmt>`,
:doc:`segyz`,
:doc:`segy2grd`
