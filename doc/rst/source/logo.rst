.. index:: ! gmtlogo

****
logo
****

.. only:: not man

    Place the GMT graphics logo on a map

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt logo** [ |-D|\ [**g**\ \|\ **j**\ \|\ **J**\ \|\ **n**\ \|\ **x**]\ *refpoint*\ **+w**\ *width*\ [**+j**\ *justify*]\ [**+o**\ *dx*\ [/*dy*]] ]
[ |-F|\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]] ]
[ |-J|\ *parameters* ] [ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |SYN_OPT-Rz| ]
[ |-S|\ [\ **l**\ \|\ **n**\ \|\ **u**\ ] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: gmtlogo_common.rst_

Examples
--------

To plot the GMT logo of a 2 inch width as a stand-alone pdf plot, use

   ::

    gmt logo -Dx0/0+w2i -pdf logo

To append a GMT logo overlay in the upper right corner of the current map, but
scaled up to be 3 inches wide and offset by 0.1 inches from the border, try

   ::

    gmt logo -DjTR+o0.1i/0.1i+w3i


Notes
-----

To instead plot the GMT QR code that links to www.generic-mapping-tools.org, just plot the
custom symbols **QR** or **QR_transparent** in :doc:`psxy`.

See Also
--------

:doc:`gmt`, :doc:`legend`,
:doc:`image`, :doc:`colorbar`, :doc:`plot`
