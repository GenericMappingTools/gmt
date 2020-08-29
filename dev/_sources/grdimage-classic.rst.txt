.. index:: ! grdimage
.. include:: module_core_purpose.rst_

********
grdimage
********

|grdimage_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdimage** *grd_z* \| *img* \| *grd_r grd_g grd_b*
[ |-A|\ *out_img*\ [**=**\ *driver*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ [**r**] ]
[ |-E|\ [**i**\|\ *dpi*] ] |-J|\ *parameters*
[ |-G|\ *color*\ [**+b**\|\ **+f**] ]
[ |-I|\ [*intensfile*\|\ *intensity*\|\ *modifiers*] ]
[ |-J|\ **z**\|\ **-Z**\ *parameters* ]
[ |-K| ] [ |-M| ] [ |-N| ]
[ |-O| ] [ |-P| ] [ |-Q| ]
[ |SYN_OPT-Rz| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

.. include:: grdimage_common.rst_

.. include:: common_classic.rst_

.. include:: grdimage_notes.rst_

Examples
--------

.. include:: explain_example.rst_

For a quick-and-dirty illuminated color map of the data in the file stuff.nc, with
the maximum map dimension limited to be 6 inches, try

   ::

    gmt grdimage stuff.nc -JX6i+ -I+d -P > quick.ps

To gray-shade the file hawaii_grav.nc with shades given in shades.cpt
on a Lambert map at 1.5 cm/degree along the standard parallels 18 and
24, and using 1 degree tickmarks:

   ::

    gmt grdimage hawaii_grav.nc -Jl18/24/1.5c -Cshades.cpt -B1 -P > hawaii_grav_image.ps

To create an illuminated color PostScript plot of the gridded data set
image.nc, using the intensities provided by the file intens.nc, and
color levels in the file colors.cpt, with linear scaling at 10
inch/x-unit, tickmarks every 5 units:

   ::

    gmt grdimage image.nc -Jx10i -Ccolors.cpt -Iintens.nc -B5 -P > image.ps

To create an false color PostScript plot from the three grid files
red.nc, green.nc, and blue.nc, with linear scaling at 10 inch/x-unit,
tickmarks every 5 units:

   ::

    gmt grdimage red.nc green.nc blue.nc -Jx10i -B5 -P > rgbimage.ps

When GDAL support is built in: To create a sinusoidal projection of a
remotely located Jessica Rabbit

   ::

    gmt grdimage -JI15c -Rd http://larryfire.files.wordpress.com/2009/07/untooned_jessicarabbit.jpg -P > jess.ps

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`grd2kml`,
:doc:`grdcontour`,
:doc:`grdview`,
:doc:`grdgradient`,
:doc:`grdhisteq`
