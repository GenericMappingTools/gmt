.. index:: ! gmtlogo
.. include:: module_core_purpose.rst_

*******
gmtlogo
*******

|gmtlogo_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmtlogo** [ |-D|\ [**g**\|\ **j**\|\ **J**\|\ **n**\|\ **x**]\ *refpoint*\ **+w**\ *width*\ [**+j**\ *justify*]\ [**+o**\ *dx*\ [/*dy*]] ]
[ |-F|\ [**+c**\ *clearances*][**+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][**+p**\ [*pen*]][**+r**\ [*radius*]][**+s**\ [[*dx*/*dy*/][*shade*]]] ]
[ |-J|\ *parameters* ]
[ |-K| ]
[ |-O| ] [ |-P| ]
[ |SYN_OPT-Rz| ]
[ |-S|\ [**l**\|\ **n**\|\ **u**] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: gmtlogo_common.rst_

.. include:: common_classic.rst_

Examples
--------

To plot the GMT logo of a 144-point width as a stand-alone plot, use::

    gmt logo -P > logo.ps

To append a GMT logo overlay in the upper right corner of the current map, but
scaled up to be 6 cm wide and offset by 0.25 cm from the border, try::

    gmt logo -O -K -R -J -DjTR+o0.25c+w6c >> bigmap.ps

Notes
-----

To instead plot the GMT QR code that links to www.generic-mapping-tools.org, just plot the
custom symbols **QR** or **QR_transparent** in :doc:`psxy`.

See Also
--------

:doc:`gmt`, :doc:`pslegend`,
:doc:`psimage`, :doc:`psscale`, :doc:`psxy`
