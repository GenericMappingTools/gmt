.. index:: ! contour
.. include:: module_core_purpose.rst_


*********
contour
*********

|contour_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt contour** [ *table* ] |-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A|\ [**n**\|\ *contours*][*labelinfo*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *contours* ]
[ |-D|\ [*template*] ] [ |-E|\ *indexfile* ]
[ |-G|\ [**d**\|\ **f**\|\ **n**\|\ **l**\|\ **L**\|\ **x**\|\ **X**]\ *params* ]
[ |-I| ] [ |-J|\ **z**\|\ **Z**\ *parameters* ]
[ |-L|\ *pen* ] [ |-N| ]
[ |-Q|\ [*cut*][**+z**] ]
[ |-S|\ [*p*\|\ *t*] ]
[ |-T|\ [**h**\|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][**+l**\ [*labels*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*type*]\ *pen*\ [**+c**\ [**l**\|\ **f**]] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-l| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: contour_common.rst_

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To make a raw contour plot from the remote file Table_5.11.txt and draw the
contours every 25 and annotate every 50, using the default Cartesian projection, try

   ::

    gmt contour @Table_5_11.txt -Wthin -C25 -A50 -B -pdf map

To use the same data but only contour the values 750 and 800, use

   ::

    gmt contour @Table_5_11.txt -A750,800 -W0.5p -B -pdf map

To create a color plot of the numerical temperature
solution obtained on a triangular mesh whose node coordinates and
temperatures are stored in temp.xyz and mesh arrangement is given by the
file mesh.ijk, using the colors in temp.cpt, run

   ::

    gmt contour temp.xyz -R0/150/0/100 -Jx0.1i -Ctemp.cpt -G -W0.25p -pdf temp

To save the triangulated 100-m contour lines in topo.txt and separate
them into multisegment files (one for each contour level), try

   ::

    gmt contour topo.txt -C100 -Dcontours_%.0f.txt

.. include:: contour_notes.rst_

.. include:: auto_legend_info.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`grdcontour`,
:doc:`grdimage`,
:doc:`nearneighbor`,
:doc:`basemap`, :doc:`colorbar`,
:doc:`surface`,
:doc:`triangulate`
