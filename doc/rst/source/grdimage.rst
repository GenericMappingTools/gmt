.. index:: ! grdimage
.. include:: module_core_purpose.rst_

********
grdimage
********

|grdimage_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdimage** *grid* \| *image*
|-J|\ *parameters*
[ |-A|\ *out_img*\ [**=**\ *driver*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ [**r**] ]
[ |-E|\ [**i**\|\ *dpi*] ]
[ |-G|\ *color*\ [**+b**\|\ **f**] ]
[ |-I|\ [*intensfile*\|\ *intensity*\|\ *modifiers*] ]
[ |-M| ]
[ |-N| ]
[ |-Q| ]
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

.. include:: grdimage_notes.rst_

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

For a quick-and-dirty illuminated color map of the data in the remote file
@AK_gulf_grav.nc, try::

    gmt grdimage @AK_gulf_grav.nc -I+d -B -pdf quick

To gray-shade the file AK_gulf_grav.nc on a Lambert map at 1.5 cm/degree
along the standard parallels 18 and 24, centered on (142W, 55N), try::

    gmt begin alaska_gray
      gmt grd2cpt -Cgray @AK_gulf_grav.nc
      grdimage @AK_gulf_grav.nc -Jl142W/55N/18/24/1.5c -B
    gmt end show

To create an illuminated color plot of the gridded data set
image.nc, using the intensities provided by the file intens.nc, and
color levels in the file colors.cpt, with linear scaling at 10
inch/x-unit, tickmarks every 5 units::

    gmt grdimage image.nc -Jx10i -Ccolors.cpt -Iintens.nc -B5 -pdf image

To create an false color plot from the three grid files
red.nc, green.nc, and blue.nc, with linear scaling at 10 inch/x-unit,
tickmarks every 5 units::

    gmt grdimage red.nc green.nc blue.nc -Jx10i -B5 -pdf rgbimage

When GDAL support is built in: To create a sinusoidal projection of a
remotely located Jessica Rabbit::

    gmt grdimage -JI15c -Rd http://larryfire.files.wordpress.com/2009/07/untooned_jessicarabbit.jpg -pdf jess

.. include:: cpt_notes.rst_

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`grd2kml`,
:doc:`grdcontour`,
:doc:`grdview`,
:doc:`grdgradient`,
:doc:`grdhisteq`
