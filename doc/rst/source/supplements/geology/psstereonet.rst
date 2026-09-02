.. index:: ! psstereonet
.. include:: ../module_supplements_purpose.rst_

***********
psstereonet
***********

|psstereonet_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt psstereonet** [ *table* ]
[ |-J|\ **A**\|\ **S**\ [0/0/]\ *width* ]
[ |-A|\ [*annot*\ [/*tick*]] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-K| ]
[ |-L|\ *pen* ]
[ |-M|\ [**c**\|\ **p**] ]
[ |-O| ] [ |-P| ]
[ |-S|\ *symbol*\ [*size*] ]
[ |-T|\ [**d**\|\ **l**\|\ **p**][**+r**][**+u**] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
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
[ |SYN_OPT-qi| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

.. include:: stereonet.rst
    :start-after: .. module_common_begins
    :end-before: .. module_common_ends

.. include:: ../../common_classic.rst_

Examples
--------

.. include:: ../../explain_example.rst_

To plot eight fault planes given as *strike dip* on a 12-centimeter-wide Schmidt net,
drawing the cyclographic traces in red and the poles as blue crosses, try::

    gmt psstereonet faults.txt -JA12c -W1p,red -Sx0.3c -L1p,blue -P > faults.ps

See Also
--------

:doc:`gmt </gmt>`, :doc:`gmt.conf </gmt.conf>`,
:doc:`psbasemap </psbasemap>`,
:doc:`psxy </psxy>`,
:doc:`psrose </psrose>`,
:doc:`/supplements/seis/pspolar`
