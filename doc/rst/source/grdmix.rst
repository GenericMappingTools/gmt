.. index:: ! grdmix
.. include:: module_core_purpose.rst_

******
grdmix
******

|grdmix_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdmix** *file_a* [ *file_b* ] [ |-A|\ *weights* ] |-G|\ *outfile*
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdmix** will either use the weights grid, image, or constant to add a
transparency layer to the image given as *file_a*, or it will blend
the two *file_a* and *file_b* grids or images  using the *weights* into
*outfile*. Files *file_a*, *weights* and *file_b* must have the same 
dimensions. The *weights* file may instead just be a constant weight.

Required Arguments
------------------

*file_a*
    Grid or image. If *file_b* is not given then *file_a* must be an image.

*file_b*
    Grid or image to be blended with *file_a*.

.. _-G:

**-G**\ *outfile*
    The name for the output grid or image.

Optional Arguments
------------------

.. _-A:

**-A**\ *weights*
    A constant weight (0-1), or a grid (0-1) or image [0-255] with weights.
    When two input files are given, the weights are applied to *file_a* and
    (1-*weights*) are applied to *file_b*, then summed.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

Examples
--------

.. include:: explain_example.rst_

Suppose map1.png and map2.png are overlapping maps, but we wish to use the image
visible.png to blend them:

   ::

    gmt grdmix map1.png map2.png -Avisible.png -Gnewmap.png -V

See Also
--------

:doc:`gmt`, :doc:`grdblend`,
:doc:`grdclip`,
:doc:`grdcut`,
:doc:`grdinfo`,
:doc:`grdsample`
