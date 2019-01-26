.. index:: ! segyz

*******
segyz
*******

.. only:: not man

    segyz - Create imagemasked postscript from SEGY file

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt segyz** *SEGYfile* |-J|\ *parameters*
|-J|\ **z**\ \|\ **Z**\ *parameters*
|SYN_OPT-Rz|
|-D|\ *deviation* |-F|\ [*color*] **-W**
[ |-C|\ *clip* ]
[ |-I| ] [ |-L|\ *nsamp* ]
[ |-M|\ *ntrace* ]
[ |-N| ]
[ |-Q|\ *<mode><value>* ]
[ |-S|\ *header_x*/*header_y* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: segyz_common.rst_

Examples
--------

To plot the SEGY file wa1.segy with normalized traces plotted at true
offset locations, clipped at ±3 and with wiggle trace and positive
variable area shading in black, use

   ::

    gmt segyz wa1.segy -JX5i/-5i -D1 -Jz0.05i -E180/5 -R0/100/0/10/0/10 \
            -C3 -N -So -W -Fblack -pdf segy

.. include:: segyz_notes.rst_

See Also
--------

:doc:`gmt </gmt>`,
:doc:`segy`,
:doc:`segy2grd`
