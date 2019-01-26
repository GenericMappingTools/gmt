.. index:: ! image

*****
image
*****

.. only:: not man

    Place images or EPS files on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt image** *imagefile*
[ |SYN_OPT-B| ]
[ |-D|\ *refpoint* ]
[ |-F|\ *box* ]
[ |-G|\ [*color*\ ][**+b**\ \|\ **+f**\ \|\ **+t**] ]
[ |-I| ]
[ |-J|\ *parameters* ]
[ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-M| ]
[ |SYN_OPT-Rz| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: image_common.rst_

Examples
--------

To plot the image logo.jpg, scaling it be 1 inch wide (height is scaled
accordingly), and outline with a thin, blue pen, use

   ::

    gmt image logo.jpg -Dx0/0+w1i -F+pthin,blue -pdf image

To include an Encapsulated PostScript file tiger.eps with its upper
right corner 2 inch to the right and 1 inch up from the current
location, and have its width scaled to 3 inches, while keeping the
aspect ratio, use

   ::

    gmt image tiger.eps -Dx2i/1i+jTR+w3i -pdf image

To replicate the 1-bit raster image template 1_bit.ras, colorize it
(brown background and red foreground), and setting each of 5 by 5 tiles
to be 1 cm wide, use

   ::

    gmt image 1_bit.ras -Gbrown+b -Gred+f -Dx0/0+w1c+n5 -pdf image

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`, :doc:`gmtlogo`
:doc:`legend`, :doc:`colorbar`
:doc:`plot`,
:manpage:`psconvert(1)`
