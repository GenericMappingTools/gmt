.. index:: ! coupe
.. include:: ../module_supplements_purpose.rst_

*******
coupe
*******

|coupe_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt coupe**
[ *table* ]
|-J|\ *parameters*
|SYN_OPT-R|
|-A|\ **a**\|\ **b**\|\ **c**\|\ **d**\ *params*\ [**+c**\ [**n**\|\ **t**]][**+d**\ *dip*][**+r**\ [**a**\|\ **e**\|\ *dx*]][**+w**\ *width*][**+z**\ [**s**]\ **a**\|\ **e**\|\ *dz*\|\ *min*/*max*]
|-S|\ *format*\ [*scale*][**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+l**][**+m**][**+o**\ *dx*\ [/*dy*]][**+s**\ *reference*]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-E|\ *fill* ]
[ |-F|\ *mode*\ [*args*] ]
[ |-G|\ *fill* ]
[ |-H|\ [*scale*] ]
[ |-I|\ [*intens*] ]
[ |-L|\ [*pen*] ]
[ |-N| ]
[ |-Q| ]
[ |-T|\ *nplane*\ [/*pen*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-tv| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: coupe_common.rst_

Examples
--------

The following will plot a cross section of three focal mechanisms::

    gmt coupe << END -Sa1c -Aa111/33/119/33+d90+w500+z0/50+r -Q \
		-JX15c/-8c -Bxaf+l"Distance (km)" -Byaf+l"Depth (km)" -BWSen -png test
    112 32 25  30  90   0  4  Strike-slip
    115 34 15  30  60  90  5  Reverse
    118 32 45  30  60 -90  6  Normal
    END

.. include:: meca_notes.rst_

See Also
--------

:doc:`meca`,
:doc:`polar`,
:doc:`gmt </gmt>`, :doc:`basemap </basemap>`,
:doc:`plot </plot>`
