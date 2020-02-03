.. index:: ! grdcontour
.. include:: module_core_purpose.rst_

**********
grdcontour
**********

|grdcontour_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdcontour** *grid*
|-J|\ *parameters* [ |-A|\ [**-**\|\ *contours*][*labelinfo*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *contours*\|\ *cpt* ]
[ |-D|\ *template* ]
[ |-F|\ [**l**\|\ **r**] ]
[ |-G|\ [**d**\|\ **f**\|\ **n**\|\ **l**\|\ **L**\|\ **x**\|\ **X**]\ *params* ]
[ |-L|\ *low/high*\|\ **n**\|\ **N**\|\ **P**\|\ **p** ]
[ |-N|\ [*cpt*] ]
[ |-Q|\ [*cut*][**+z**] ]
[ |SYN_OPT-Rz| ]
[ |-S|\ *smoothfactor* ]
[ |-T|\ [**h**\|\ **l**][**+a**][**+d**\ *gap*\ [/*length*]][**+l**\ [*labels*]] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*type*]\ *pen*\ [**+c**\ [**l**\|\ **f**]] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ [**+s**\ *factor*][**+o**\ *shift*][**+p**] ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-do| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ **-ho**\ [*n*] ]
[ |SYN_OPT-l| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: grdcontour_common.rst_

.. include:: grdcontour_notes.rst_

.. include:: auto_legend_info.rst_

Examples
--------

.. include:: oneliner_info.rst_

To contour the remote file AK_gulf_grav.nc every 25 mGal on a Mercator map at
0.5 inch/degree, annotate every 50 mGal (using font size = 10p), using 1
degree tick marks, and draw 30 minute gridlines::

    gmt grdcontour @AK_gulf_grav.nc -JM16c -C25 -A50+f10p -B -pdf alaska_grav1

To do the same map but only draw the 50 and 150 and annotate the 100 contour::

    gmt grdcontour @AK_gulf_grav.nc -JM16c -C50,150 -A100,+f10p -B -pdf alaska_grav2

To contour the Alaska gravity data every 10 mGal with labels every 50 mGal, smooth
the contours a bit, use "Gravity Anomalies" as plot-title, use a thick red pen for
the annotated contours and a thin, dashed, blue pen for the rest, try::

    gmt grdcontour @AK_gulf_grav.nc -C10 -A50 -S4 -B -B+t"Gravity Anomalies" -Wathick,red -Wcthinnest,blue,- -pdf alaska_grav3

Same, but this time we want all negative contours to be blue and positive to be red, with
the zero-contour black::

    gmt begin alaska_grav4
      grdcontour @AK_gulf_grav.nc -C10 -A50 -S4 -B -B+t"Gravity Anomalies" -Ln -Wathick,blue -Wcthinnest,blue,-
      grdcontour @AK_gulf_grav.nc -C10 -A50 -S4 -Lp -Wathick,red -Wcthinnest,red,-
      grdcontour @AK_gulf_grav.nc -A0, -S4
    gmt end show

To save the smoothed 50-mGal contour lines in AK_gulf_grav.nc and separate them
into two multisegment files: contours_C.txt for closed and
contours_O.txt for open contours, try::

    gmt grdcontour @AK_gulf_grav.nc -C150 -S4 -DAK_contours_%c.txt

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`basemap`,
:doc:`grdimage`,
:doc:`grdview`,
:doc:`contour`
