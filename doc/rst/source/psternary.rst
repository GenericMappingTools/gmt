.. index:: ! psternary

*********
psternary
*********

.. only:: not man

    psternary - Plot data on ternary diagrams

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psternary** [ *table* ]
[ **-JX** \ *width*\ [unit] ]
[ |SYN_OPT-Rz| ]
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

.. include:: ternary_common.rst_

.. include:: common_classic.rst_

Examples
--------

To plot circles (diameter = 0.1 cm) on a 6-inch-wide ternary diagram at the positions listed
in the file ternary.txt, with default annotations and gridline spacings, using the
specified labeling, tru

   ::

    gmt psternary ternary.txt -R0/100/0/100/0/100 -JX6i -P -Xc -Baafg+l"Water component"+u" %" \
    -Bbafg+l"Air component"+u" %" -Bcagf+l"Limestone component"+u" %" \
    -B+givory+t"Example data from MATLAB Central" -Sc0.1c -Ct.cpt -Y2i -LWater/Air/Limestone > map.ps


See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`psbasemap`,
:doc:`psxy`,
:doc:`psxyz`
