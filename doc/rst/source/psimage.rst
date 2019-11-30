.. index:: ! psimage
.. include:: module_core_purpose.rst_

*******
psimage
*******

|psimage_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt psimage** *imagefile*
[ |SYN_OPT-B| ]
[ |-D|\ *refpoint* ]
[ |-F|\ *box* ]
[ |-G|\ [*color*\ ][**+b**\ \|\ **+f**\ \|\ **+t**] ]
[ |-I| ]
[ |-J|\ *parameters* ]
[ |-K| ]
[ |-M| ]
[ |-O| ]
[ |-P| ]
[ |SYN_OPT-Rz| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: image_common.rst_

.. include:: common_classic.rst_

Examples
--------

.. include:: explain_example.rst_

To plot the image logo.jpg, scaling it be 1 inch wide (height is scaled
accordingly), and outline with a thin, blue pen, use

   ::

    gmt psimage logo.jpg -Dx0/0+w1i -F+pthin,blue > image.ps

To include an Encapsulated PostScript file tiger.eps with its upper
right corner 2 inch to the right and 1 inch up from the current
location, and have its width scaled to 3 inches, while keeping the
aspect ratio, use

   ::

    gmt psimage tiger.eps -Dx2i/1i+jTR+w3i > image.ps

To replicate the 1-bit remote raster image vader1.png, colorize it
(dark gray background and yellow foreground), and setting each of 6 by 12 tiles
to be 2.5 cm wide, use::

    gmt psimage @vader1.png -Gdarkgray+b -Gyellow+f -Dx0/0+w2.5c+n6/12 -P > image.ps

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`, :doc:`gmtlogo`
:doc:`pslegend`, :doc:`psscale`
:doc:`psxy`
