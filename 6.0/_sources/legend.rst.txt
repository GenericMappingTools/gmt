.. index:: ! legend
.. include:: module_core_purpose.rst_

********
legend
********

|legend_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt legend** [ *specfile* ]
|-D|\ *refpoint*
[ |SYN_OPT-B| ]
[ |-C|\ *dx*/*dy* ]
[ |-F|\ *box* ]
[ |-J|\ *parameters* ]
[ |SYN_OPT-R| ]
[ |-S|\ *scale* ]
[ |-T|\ *file* ]
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

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To add an example of a legend to a Mercator plot (map.ps) with the given
specifications, use::

    gmt begin legend
    gmt makecpt -Cpanoply -T-8/8/1 -H > tt.cpt
    gmt set FONT_ANNOT_PRIMARY 12p
    gmt legend -R0/10/0/10 -JM6i -Dx0.5i/0.5i+w5i+jBL+l1.2 -C0.1i/0.1i -F+p+gazure1+r -B5f1 << EOF
    # Legend test for gmt pslegend
    # G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line,
    # H is ps=legend.ps
    #
    G -0.1i
    H 24p,Times-Roman My Map Legend
    D 0.2i 1p
    N 2
    V 0 1p
    S 0.1i c 0.15i p300/12 0.25p 0.3i This circle is hachured
    S 0.1i e 0.15i yellow 0.25p 0.3i This ellipse is yellow
    S 0.1i w 0.15i green 0.25p 0.3i This wedge is green
    S 0.1i f 0.25i blue 0.25p 0.3i This is a fault
    S 0.1i - 0.15i - 0.25p,- 0.3i A contour
    S 0.1i v 0.25i magenta 0.5p 0.3i This is a vector
    S 0.1i i 0.15i cyan 0.25p 0.3i This triangle is boring
    D 0.2i 1p
    V 0 1p
    N 1
    M 5 5 600+u+f
    G 0.05i
    I @SOEST_block4.png 3i CT
    G 0.05i
    B tt.cpt 0.2i 0.2i -B0
    G 0.05i
    L 9p,Times-Roman R Smith et al., @%5%J. Geophys. Res., 99@%%, 2000
    G 0.1i
    T Let us just try some simple text that can go on a few lines.
    T There is no easy way to predetermine how many lines may be required
    T so we may have to adjust the height to get the right size box.
    EOF
    rm -f tt.cpt
    gmt end show


Auto-legends
------------

In modern mode, some modules can access the **-l** option and build the legend
*specfile* from individual entries for each command.  The **-l** option takes an optional
label and allows optional modifiers **+d**, **+g**, **+n**, **+h**, **+j**, **+l**, and **+v**
that follow the corresponding uppercase legend codes discussed above.  In addition,
there is **+f** to set header (H) or label (L) font,  **+s** to set the symbol size (or line length)
to use for the current entry, and **+x** to scale all symbols uniformly (see **-S** above).
Some defaults are hardwired: We draw a white rectangular panel with
a 1 point black outline offset from the justification point (**+j**) by 0.2 cm.  To use
different settings you must call **legend** explicitly before :doc:`end` does so
implicitly instead.  Note: With an explicit call to **legend** you can also use **-T** to
save the auto-generate *specfile*, make modification to it, and then pass that to **legend**
directly.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`, :doc:`gmtlogo`
:doc:`basemap`, :doc:`text`,
:doc:`plot`
