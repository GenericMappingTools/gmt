.. index:: ! grdview

*******
grdview
*******

.. only:: not man

    Create 3-D perspective image or surface mesh from a grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdview** *reliefgrid* |-J|\ *parameters*
[ |SYN_OPT-B| ]
[ |-C|\ [*cpt*]]
[ |-G|\ *drapegrid* \| |-G|\ *grd_r* |-G|\ *grd_g* |-G|\ *grd_b* ]
[ |-I|\ [*intensgrid*\ \|\ *intensity*\ \|\ *modifiers*] ]
[ **-Jz**\ \|\ **Z**\ *parameters* ]
[ |-N|\ *level*\ [**+g**\ *fill*] ]
[ |-Q|\ *args*\ [**+m**] ]
[ |SYN_OPT-Rz| ]
[ |-S|\ *smooth* ]
[ |-T|\ [\ **+o**\ [*pen*]][**+s**] ]
[ |SYN_OPT-U| ]
[ |-W|\ **c|m|f**\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: grdview_common.rst_

Examples
--------

To make a mesh plot from the file hawaii_grav.nc and drawing the
contours given in the CPT hawaii.cpt on a Lambert map at
1.5 cm/degree along the standard parallels 18 and 24, with vertical
scale 20 mgal/cm, and looking at the surface from SW at 30 degree
elevation, run

   ::

    gmt grdview hawaii_grav.nc -Jl18/24/1.5c -Chawaii.cpt
                -Jz0.05c -Qm -N-100 -p225/30 -Wc -pdf hawaii_grav_image

To create a illuminated color perspective plot of the gridded data set
image.nc, using the CPT color.cpt, with linear scaling at
10 cm/x-unit and tickmarks every 5 units, with intensities provided by
the file intens.nc, and looking from the SE, use

   ::

    gmt grdview image.nc -Jx10c -Ccolor.cpt -Qs -p135/30 -Iintens.nc -pdf image3D

To make the same plot using the rastering option with dpi = 50, use

   ::

    gmt grdview image.nc -Jx10c -Ccolor.cpt -Qi50 -p135/30 -Iintens.nc -pdf image3D

To create a color PostScript perspective plot of the gridded data set
magnetics.nc, using the CPT mag_intens.cpt, draped over
the relief given by the file topography.nc, with Mercator map width of 6
inch and tickmarks every 1 degree, with intensities provided by the file
topo_intens.nc, and looking from the SE, run

   ::

    gmt grdview topography.nc -JM6i -Gmagnetics.nc -Cmag_intens.cpt
                -Qs -p140/30 -Itopo_intens.nc -pdf draped3D

.. include:: grdview_notes.rst_

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`,
:doc:`grdcontour`,
:doc:`grdimage`,
:doc:`grdsample`,
:doc:`nearneighbor`,
:doc:`basemap`,
:doc:`contour`, :doc:`text`,
:doc:`surface`
