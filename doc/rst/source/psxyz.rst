.. index:: ! psxyz
.. include:: module_core_purpose.rst_

*****
psxyz
*****

|psxyz_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt psxyz** [ *table* ] |-J|\ *parameters*
|-J|\ **z**\|\ **Z**\ *parameters*
|SYN_OPT-Rz|
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *dx*/*dy*\ [/*dz*] ]
[ |-G|\ *fill* ]
[ |-I|\ [*intens*] ]
[ |-K| ]
[ |-L|\ [**+b**\|\ **d**\|\ **D**][**+xl**\|\ **r**\|\ *x0*][**+yl**\|\ **r**\|\ *y0*][**+p**\ *pen*] ]
[ |-N| ] [ |-O| ] [ |-P| ] [ |-Q| ]
[ |-S|\ [*symbol*][*size*\ [**unit**]][/*size_y*] ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][*attr*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ [**l**\|\ **f**]\ *value* ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: plot3d_common.rst_

.. include:: common_classic.rst_

Examples
--------

.. include:: explain_example.rst_

To plot blue columns (width = 1.25 cm) at the positions listed in the
file heights.xyz on a 3-D projection of the space (0-10), (0-10),
(0-100), with tickmarks every 2, 2, and 10, viewing it from the
southeast at 30 degree elevation, use:

   ::

    gmt psxyz heights.xyz -R0/10/0/10/0/100 -Jx1.25c -Jz0.125c -So1.25c \
              -Gblue -Bx2+lXLABEL -By2+lYLABEL -Bz10+lZLABEL -B+t"3-D PLOT" -p135/30 \
              -U+c -W -P > heights.ps

To plot a point with color and outline dictated by the *t.cpt* file for the *level*-value 65, try

   ::

    echo 175 30 0 | gmt psxyz -R150/200/20/50 -JX15c -Sc0.5c -Z65 -Ct.cpt > map.ps

.. include:: plot3d_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`psbasemap`, :doc:`psxy`
