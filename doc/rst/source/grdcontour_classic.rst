.. index:: ! grdcontour

**********
grdcontour
**********

.. only:: not man

    grdcontour - Make contour map using a grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**grdcontour** *grid*
|-J|\ *parameters*
[ |-A|\ [**-**\ *contours*][*labelinfo*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *contours* ]
[ |-D|\ *template* ]
[ |-F|\ [**l**\ \|\ **r**] ]
[ |-G|\ [**d**\ \|\ **f**\ \|\ **n**\ \|\ **l**\ \|\ **L**\ \|\ **x**\ \|\ **X**]\ *params* ]
[ |-J|\ **z**\ \|\ **Z**\ *parameters* ] [ |-K| ]
[ |-L|\ *low/high*\ \|\ **n**\ \|\ **N**\ \|\ **P**\ \|\ **p** ]
[ |-N|\ [*cpt*] ]
[ |-O| ] [ |-P| ]
[ |-Q|\ [*cut*\ [*unit*]][\ **+z**] ]
[ |SYN_OPT-Rz| ]
[ |-S|\ *smoothfactor* ]
[ |-T|\ [**h**\ \|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][\ **+l**\ [*labels*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*type*]\ *pen* ][**+c**\ [**l**\ \|\ **f**]]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ [**+s**\ *factor*][**+o**\ *shift*][**+p**] ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-do| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ **-ho**\ [*n*] ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: grdcontour_common.rst_

.. include:: common_classic.rst_

.. include:: grdcontour_notes.rst_

Examples
--------

To contour the file hawaii_grav.nc every 25 mGal on a Mercator map at
0.5 inch/degree, annotate every 50 mGal (using fontsize = 10p), using 1
degree tickmarks, and draw 30 minute gridlines:

   ::

    gmt grdcontour hawaii_grav.nc -Jm0.5i -C25 -A50+f10p -B1g30m > hawaii_grav.ps

To do the same map but only draw the 50 and 150 and annotate the 100 contour:

   ::

    gmt grdcontour hawaii_grav.nc -Jm0.5i -C50.150 -A100,+f10p -B1g30m > hawaii_grav.ps

To contour the file image.nc using the levels in the file cont.txt on a
linear projection at 0.1 cm/x-unit and 50 cm/y-unit, using 20 (x) and
0.1 (y) tickmarks, smooth the contours a bit, use "RMS Misfit" as
plot-title, use a thick red pen for annotated contours, and a thin,
dashed, blue pen for the rest, and send the output to the default printer:

   ::

    gmt grdcontour image.nc -Jx0.1c/50.0c -Ccont.txt -S4 -Bx20 -By0.1 \
               -B+t"RMS Misfit" -Wathick,red -Wcthinnest,blue,- | lp

The labeling of local highs and lows may plot outside the innermost
contour since only the mean value of the contour coordinates is used to
position the label.

To save the smoothed 100-m contour lines in topo.nc and separate them
into two multisegment files: contours_C.txt for closed and
contours_O.txt for open contours, try

   ::

    gmt grdcontour topo.nc -C100 -S4 -Dcontours_%c.txt

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`psbasemap`,
:doc:`grdimage`, :doc:`grdview`,
:doc:`pscontour`
