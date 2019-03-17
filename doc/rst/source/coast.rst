.. index:: ! coast

*****
coast
*****

.. only:: not man

    Plot continents, shorelines, rivers, and borders on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt coast** |-J|\ *parameters*
|SYN_OPT-R|
[ |-A|\ *area* ]
[ |SYN_OPT-B| ]
[ |-C|\ [**l**\ \|\ **r**/]\ *fill* ]
[ |-D|\ *resolution*\ [**+f**] ]
[ |-E|\ *dcw* ]
[ |-F|\ *box* ]
[ |-G|\ *fill*\ \|\ **c** ]
[ |-I|\ *river*\ [/\ *pen*] ]
[ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-L|\ *scalebar* ]
[ |-M| ]
[ |-N|\ *border*\ [/*pen*] ]
[ |-Q| ]
[ |-S|\ *fill*\ \|\ **c** ]
[ |-T|\ *rose* ]
[ |-T|\ *mag_rose* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*level*/]\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: coast_common.rst_

Examples
--------

To plot a green Africa with white outline on blue background, with
permanent major rivers in thick blue pen, additional major rivers in
thin blue pen, and national borders as dashed lines on a Mercator map at
scale 0.1 inch/degree, use

   ::

    gmt coast -R-30/30/-40/40 -Jm0.1i -B5 -I1/1p,blue -N1/0.25p,-
                -I2/0.25p,blue -W0.25p,white -Ggreen -Sblue -pdf africa

To plot Iceland using the lava pattern (# 28) at 100 dots per inch, on a
Mercator map at scale 1 cm/degree, run

   ::

    gmt coast -RIS+r1 -Jm1c -B -Wthin -Gp28+r100 -pdf iceland

To initiate a clip path for Africa so that the subsequent colorimage of
gridded topography is only seen over land, using a Mercator map at scale
0.1 inch/degree, use

   ::

    gmt begin
    gmt coast -R-30/30/-40/40 -Jm0.1i -B5 -Gc
    gmt grdimage etopo5.nc -Ccolors.cpt
    gmt coast -Q
    gmt end

To plot Great Britain, Italy, and France in blue with a red outline and
Spain, Portugal and Greece in yellow (no outline), and pick up the plot
domain form the extents of these countries, use

   ::

    gmt coast -JM6i -Baf -EGB,IT,FR+gblue+p0.25p,red -EES,PT,GR+gyellow -pdf map

To extract a high-resolution coastline data table for Iceland to be used
in your analysis, try

   ::

    gmt pscoast -RIS -Dh -W -M > iceland.txt

**coast** will first look for coastline files in directory
**$GMT_SHAREDIR**/coast If the desired file is not found, it will look
for the file **$GMT_SHAREDIR**/coastline.conf. This file may contain
any number of records that each holds the full pathname of an
alternative directory. Comment lines (#) and blank lines are allowed.
The desired file is then sought for in the alternate directories.

.. include:: explain_gshhs.rst_

.. include:: coast_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`grdlandmask`,
:doc:`basemap`
