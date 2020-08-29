.. index:: ! pspolar
.. include:: ../module_supplements_purpose.rst_

*******
pspolar
*******

|pspolar_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt pspolar** [ *table* ] |-D|\ *lon/lat* |-J|\ *parameters*
|SYN_OPT-R|
|-M|\ *size*\ [**+m**\ *mag*]
|-S|\ *<symbol><size>*
[ |SYN_OPT-B| ]
[ |-C|\ *lon*/*lat*\ [**+p**\ *pen*][**+s**\ *pointsize*] ]
[ |-E|\ *fill* ]
[ |-F|\ *fill* ]
[ |-G|\ *fill* ]
[ |-K| ] [ |-N| ]
[ |-O| ]
[ |-Q|\ *mode*\ [*args*] ]
[ |-T|\ [**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+o**\ *dx*\[/*dy*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: polar_common.rst_

.. include:: ../../common_classic.rst_

Examples
--------

::

    gmt pspolar -R239/240/34/35.2 -JM8c -N -Sc0.4 -D239.5/34.5 -M5 << END > test.ps
    #stat azim ih pol
    0481 11 147 c
    6185 247 120 d
    0485 288 114 +
    0490 223 112 -
    0487 212 109 .
    END

Use special format derived from HYPO71 output::

    gmt pspolar -R239/240/34/35.2 -JM8c -N -Sc0.4 -D239.5/34.5 -M5 -Qh <<END > test.ps
    #Date Or. time stat azim ih
    910223 1 22 0481 11 147 ipu0
    910223 1 22 6185 247 120 ipd0
    910223 1 22 0485 288 114 epu0
    910223 1 22 0490 223 112 epd0
    910223 1 22 0487 212 109 epu0
    END

.. include:: meca_notes.rst_

See Also
--------

:doc:`psmeca`,
:doc:`pscoupe`,
:doc:`gmt </gmt>`, :doc:`psbasemap </psbasemap>`, :doc:`psxy </psxy>`
