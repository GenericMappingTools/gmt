.. index:: ! psternary
.. include:: module_core_purpose.rst_

*********
psternary
*********

|psternary_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt psternary** [ *table* ]
[ **-JX**\ *width*\ [unit] ]
[ |-R|\ *amin/amax/bmin/bmax/cmin/cmax* ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-K| ]
[ |-L|\ *a*\ /*b*\ /*c* ]
[ |-M| ]
[ |-N| ]
[ |-O| ] [ |-P| ]
[ |-S|\ [*symbol*][\ *size*\ [**u**] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][*attr*] ]
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

.. include:: ternary_common.rst_

.. include:: common_classic.rst_

Examples
--------

.. include:: explain_example.rst_

To plot circles (diameter = 0.1 cm) on a 6-inch-wide ternary diagram at the positions listed
in the file ternary.txt, with default annotations and gridline spacings, using the
specified labeling, try::

    gmt makecpt -Cturbo -T0/80/10 > t.cpt
    gmt psternary @ternary.txt -R0/100/0/100/0/100 -JX6i -P -Sc0.1c -Ct.cpt -LWater/Air/Limestone \
        -Baafg+l"Water component"+u" %" -Bbafg+l"Air component"+u" %" -Bcagf+l"Limestone component"+u" %" \
        -B+givory+t"Example data from MATLAB Central" > map.ps

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`psbasemap`,
:doc:`psxy`,
:doc:`psxyz`
