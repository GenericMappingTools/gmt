.. index:: ! image
.. include:: module_core_purpose.rst_

*****
image
*****

|image_purpose|

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

.. include:: oneliner_info.rst_

To plot the remote image needle.jpg, scaling it be 7 cm wide (height is scaled
accordingly), use::

    gmt image @needle.jpg -Dx0/0+w7c -pdf plot

To plot the same file but reversing the bands, use::

    gmt image @needle.jpg+b2,1,0 -Dx0/0+w7c -pdf plot

To only plot its red band as gray shade, use::

    gmt image @needle.jpg+b0 -Dx0/0+w7c -pdf plot

To include an Encapsulated PostScript file gallo.eps with its upper
right corner 2 inch to the right and 1 inch up from the current
location, and have its width scaled to 3 inches, while keeping the
aspect ratio, use::

    gmt image @gallo.eps -Dx2i/1i+jTR+w3i -pdf image

To replicate the 1-bit remote raster image vader1.png, colorize it
(dark gray background and yellow foreground), and setting each of 6 by 12 tiles
to be 2.5 cm wide, use::

    gmt image @vader1.png -Gdarkgray+b -Gyellow+f -Dx0/0+w2.5c+n6/12 -pdf image

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`, :doc:`gmtlogo`
:doc:`legend`, :doc:`colorbar`
:doc:`plot`,
:doc:`psconvert`
