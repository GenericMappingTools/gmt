.. index:: ! plot3d

******
plot3d
******

.. only:: not man

    Plot lines, polygons, and symbols in 3-D

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt plot3d** [ *table* ] |-J|\ *parameters*
|-J|\ **z**\ \|\ **Z**\ *parameters*
|SYN_OPT-Rz|
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *dx*/*dy*\ [/*dz*] ]
[ |-G|\ *fill* ]
[ |-I|\ *intens* ] 
[ |-L|\ [**+b**\ \|\ **d**\ \|\ **D**][**+xl**\ \|\ **r**\ \|\ *x0*][**+yl**\ \|\ **r**\ \|\ *y0*][**+p**\ *pen*] ] 
[ |-N| ]
[ |-Q| ] 
[ |-S|\ [*symbol*][\ *size*\ [**unit**]][/*size_y*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][*attr*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-a| ] 
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

.. include:: plot3d_common.rst_

Examples
--------

To plot blue columns (width = 1.25 cm) at the positions listed in the
file heights.xyz on a 3-D projection of the space (0-10), (0-10),
(0-100), with tickmarks every 2, 2, and 10, viewing it from the
southeast at 30 degree elevation, use:

   ::

    gmt plot3d heights.xyz -R0/10/0/10/0/100 -Jx1.25c -Jz0.125c -So1.25c
              -Gblue -Bx2+lXLABEL -By2+lYLABEL -Bz10+lZLABEL -B+t"3-D PLOT" -p135/30
              -U+c -W -pdf heights

.. include:: plot3d_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`basemap`, :doc:`plot`
