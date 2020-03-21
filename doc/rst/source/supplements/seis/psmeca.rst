.. index:: ! psmeca
.. include:: ../module_supplements_purpose.rst_

******
psmeca
******

|psmeca_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt psmeca** [ *table* ] |-J|\ *parameters* |SYN_OPT-R|
|-S|\ *<format><scale>*\ [**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+o**\ *dx*\ [/*dy*]]
[ |SYN_OPT-B| ]
[ |-C|\ [*pen*][**+s**\ *size*] ] [ |-D|\ *depmin*/*depmax* ]
[ |-E|\ *fill*]
[ |-F|\ *mode*\ [*args*] ] [ |-G|\ *fill*] [ |-K| ] [ |-L|\ [*pen*] ]
[ |-M| ]
[ |-N| ] [ |-O| ] [ |-P| ]
[ |-T|\ *nplane*\ [*pen*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ *cpt*]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: meca_common.rst_

.. include:: ../../common_classic.rst_

Examples
--------

The following file should give a normal-faulting CMT mechanism::

    gmt psmeca -R239/240/34/35.2 -Jm4c -Sc2c << END > test.ps
    # lon lat depth str dip slip st dip slip mant exp plon plat
    239.384 34.556 12. 180 18 -88 0 72 -90 5.5 0 0 0
    END

.. include:: meca_notes.rst_

See Also
--------

:doc:`pspolar`,
:doc:`pscoupe`,
:doc:`gmt </gmt>`, :doc:`psbasemap </psbasemap>`, :doc:`psxy </psxy>`
