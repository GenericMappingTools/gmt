.. index:: ! clip

******
clip
******

.. only:: not man

    Initialize or terminate polygonal clip paths

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt clip** [ *table* ] |-J|\ *parameters* |-C|\ [\ *n*]
|SYN_OPT-Rz|
[ |-A|\ [**m**\ \|\ **p**\ \|\ **x**\ \|\ **y**] ]
[ |SYN_OPT-B| ]
|-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-N| ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*] ]
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

Examples
--------

.. include:: explain_example.rst_

To see the effect of a simple clip path which result in some symbols
being partly visible or missing altogether, try

   ::

    gmt begin clip
      gmt clip -R0/6/0/6 -Jx2.5c -W1p,blue << EOF
    0 0
    5 1
    5 5
    EOF
      gmt plot @tut_data.txt -Gred -Sc2c 
      gmt psclip -C -B
    gmt end show

where we activate and deactivate the clip path.  Note we also draw the
outline of the clip path to make it clear what is being clipped.

See Also
--------

:doc:`gmt`, :doc:`grdmask`,
:doc:`basemap`, :doc:`mask`
