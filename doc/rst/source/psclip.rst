.. index:: ! psclip

******
psclip
******

.. only:: not man

    psclip - Initialize or terminate polygonal clip paths

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psclip** [ *table* ] |-J|\ *parameters* |-C|\ [\ *n*]
|SYN_OPT-Rz|
[ |-A|\ [**m**\ \|\ **p**\ \|\ **x**\ \|\ **y**] ]
[ |SYN_OPT-B| ]
|-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-K| ] [ |-N| ] [ |-O| ]
[ |-P| ] [ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
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

.. include:: clip_common.rst_

.. include:: common_classic.rst_

Examples
--------

To set up a complex clip
area to which all subsequent plotting will be confined, run:

   ::

    gmt psclip my_region.xy -R0/40/0/40 -Jm0.3i -K > clip_mask_on.ps

To deactivate the clipping in an existing plotfile, run:

   ::

    gmt psclip -C -O >> complex_plot.ps

See Also
--------

:doc:`gmt`, :doc:`grdmask`,
:doc:`psbasemap`, :doc:`psmask`
