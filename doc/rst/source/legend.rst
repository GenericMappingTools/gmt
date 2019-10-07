.. index:: ! legend

********
legend
********

.. only:: not man

    Plot legends on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt legend** [ *specfile* ]
|-D|\ *refpoint* ]
[ |SYN_OPT-B| ]
[ |-C|\ *dx*/*dy* ]
[ |-F|\ *box* ]
[ |-J|\ *parameters* ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: legend_common.rst_

Examples
--------

To add an example of a legend to a Mercator plot (map.ps) with the given
specifications, use

   ::

     gmt legend -R-10/10/-10/10 -JM6i -F+gazure1 -Dx0.5i/0.5i+w5i/3.3i+jBL+l1.2
                -C0.1i/0.1i -B5f1 -pdf map << EOF
     # Legend test for legend
     # G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line.
     # H is header, L is label, S is symbol, T is paragraph text, M is map scale.
     #
     G -0.1i
     H 24p,Times-Roman My Map Legend
     D 0.2i 1p
     N 2
     V 0 1p
     S 0.1i c 0.15i p300/12 0.25p 0.3i This circle is hachured
     S 0.1i e 0.15i yellow 0.25p 0.3i This ellipse is yellow
     S 0.1i w 0.15i green 0.25p 0.3i This wedge is green
     S 0.1i f0.1i+l+t 0.25i blue 0.25p 0.3i This is a fault
     S 0.1i - 0.15i - 0.25p,- 0.3i A dashed contour
     S 0.1i v0.1i+a40+e 0.25i magenta 0.25p 0.3i This is a vector
     S 0.1i i 0.15i cyan 0.25p 0.3i This triangle is boring
     V 0 1p
     D 0.2i 1p
     N 1
     M 5 5 600+u f
     G 0.05i
     I SOEST_logo.ras 3i CT
     G 0.05i
     B colors.cpt 0.2i 0.2i
     G 0.05i
     L 9p,Times-Roman R Smith et al., @%5%J. Geophys. Res., 99@%%, 2000
     G 0.1i
     P
     T Let us just try some simple text that can go on a few lines.
     T There is no easy way to predetermine how many lines will be required,
     T so we may have to adjust the box height to get the right size box.
     EOF

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`, :doc:`gmtlogo`
:doc:`basemap`, :doc:`text`,
:doc:`plot`
