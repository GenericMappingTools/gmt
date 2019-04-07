.. index:: ! pscontour


*********
pscontour
*********

.. only:: not man

    Contour table data by direct triangulation [method]

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**pscontour** [ *table* ] |-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A|\ [**-**\ *contours*][*labelinfo*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *contours* ]
[ |-D|\ [*template*] ] [ |-E|\ *indexfile* ]
[ |-G|\ [**d**\ \|\ **f**\ \|\ **n**\ \|\ **l**\ \|\ **L**\ \|\ **x**\ \|\ **X**]\ *params* ]
[ |-I| ] [ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-K| ]
[ |-L|\ *pen* ] [ |-N| ]
[ |-O| ]
[ |-P| ]
[ |-Q|\ [*cut*\ [*unit*]][\ **+z**] ]
[ |-S|\ [\ **p**\ \|\ **t**] ]
[ |-T|\ [**h**\ \|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][\ **+l**\ [*labels*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*type*]\ *pen* ][**+c**\ [**l**\ \|\ **f**]]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: contour_common.rst_

.. include:: common_classic.rst_

Examples
--------

To make a raw contour plot from the file topo.xyz and drawing the
contours (pen = 2) given in the CPT topo.cpt on a Lambert
map at 0.5 inch/degree along the standard parallels 18 and 24, use

   ::

    gmt pscontour topo.xyz -R320/330/20/30 -Jl18/24/0.5i -Ctopo.cpt -W0.5p > topo.ps


To use the same data but only contour the values 250 and 700, use

   ::

    gmt pscontour topo.xyz -R320/330/20/30 -Jl18/24/0.5i -C250,700 -W0.5p > topo.ps

To create a color plot of the numerical temperature
solution obtained on a triangular mesh whose node coordinates and
temperatures are stored in temp.xyz and mesh arrangement is given by the
file mesh.ijk, using the colors in temp.cpt, run

   ::

    gmt pscontour temp.xyz -R0/150/0/100 -Jx0.1i -Ctemp.cpt -G -W0.25p > temp.ps

To save the triangulated 100-m contour lines in topo.txt and separate
them into multisegment files (one for each contour level), try

   ::

    gmt pscontour topo.txt -C100 -Dcontours_%.0f.txt

.. include:: contour_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`grdcontour`,
:doc:`grdimage`,
:doc:`nearneighbor`,
:doc:`psbasemap`, :doc:`psscale`,
:doc:`surface`,
:doc:`triangulate`
