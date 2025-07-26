.. index:: ! psbarb
.. include:: ../module_supplements_purpose.rst_

******
psbarb
******

.. only:: not man

|psbarb_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**psbarb**
[ *table* ]
|-J|\ *parameters* |-J|\ **z**\ \|\ **Z**\ *parameters*
|SYN_OPT-Rz|
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *dx*/*dy*\ [/*dz*] ]
[ |-G|\ *fill* ]
[ |-I|\ *intens* ]
[ |-K| ]
[ |-N| ]
[ |-O| ] [ |-P| ]
[ |-Q|\ *parameters* ]
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
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]

.. include:: barb.rst
    :start-after: .. module_common_begins
    :end-before: .. module_common_ends

.. include:: ../../common_classic.rst_

Examples
--------

.. include:: ../../explain_example.rst_

To plot blue columns (width = 1.25 cm) at the positions listed in the
file heights.xyz on a 3-D projection of the space (0-10), (0-10),
(0-100), with tickmarks every 2, 2, and 10, viewing it from the
southeast at 30 degree elevation, use::

 gmt psbarb heights.xyz -R0/10/0/10/0/100 -Jx1.25c -Jz0.125c -So1.25c -Gblue \
          -Bx2+lXLABEL -By2+lYLABEL -Bz10+lZLABEL -B+t"3-D PLOT" -p135/30 -Uc -W -P > heights.ps

Segment Header Parsing
----------------------

Segment header records may contain one of more of the following options:

**-G**\ *fill*
    Use the new *fill* and turn filling on.
**-G-**
    Turn filling off.
**-G**
    Revert to default fill (none if not set on command line).
**-W**\ *pen*
    Use the new *pen* and turn outline on.
**-W**
    Revert to default pen :term:`MAP_DEFAULT_PEN <MAP_DEFAULT_PEN>`
    (if not set on command line).
**-W-**
    Turn outline off
**-Z**\ *zval*
    Obtain fill via cpt lookup using z-value *zval*.
**-Z**\ *NaN*
    Get the NaN color from the CPT.

See Also
--------

:doc:`gmt.conf </gmt.conf>`, :doc:`gmt </gmt>`,
:doc:`gmtcolors </gmtcolors>`,
:doc:`grdbarb`, :doc:`psxyz </psxyz>`
