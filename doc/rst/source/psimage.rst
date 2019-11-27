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

To plot the remote image needle.jpg, scaling it be 7 cm wide (height is scaled
accordingly), use::

    gmt psimage @needle.jpg -Dx0/0+w7c > image.ps

To plot the same file but reversing the bands, use::

    gmt psimage @needle.jpg+b2,1,0 -Dx0/0+w7c > image.ps

To only plot its red band as gray shade, use::

    gmt psimage @needle.jpg+b0 -Dx0/0+w7c > image.ps

To include an Encapsulated PostScript file gallo.eps with its upper
right corner 2 inch to the right and 1 inch up from the current
location, and have its width scaled to 3 inches, while keeping the
aspect ratio, use::

    gmt psimage @gallo.eps -Dx2i/1i+jTR+w3i > image.ps

To replicate the 1-bit raster image template 1_bit.ras, colorize it
(brown background and red foreground), and setting each of 5 by 5 tiles
to be 1 cm wide, use::

    gmt psimage 1_bit.ras -Gbrown+b -Gred+f -Dx0/0+w1c+n5 > image.ps

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`, :doc:`gmtlogo`
:doc:`pslegend`, :doc:`psscale`
:doc:`psxy`
