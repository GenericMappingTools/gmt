.. index:: ! mask
.. include:: module_core_purpose.rst_

******
mask
******

|mask_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt mask** [ *table* ]
|SYN_OPT-I|
|-J|\ *parameters*
|SYN_OPT-R|
[ |SYN_OPT-B| ]
[ |-C| ]
[ |-D|\ *dumpfile* ]
[ |-F|\ [**l**\ \|\ **r**] ]
[ |-G|\ *fill* ] [ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-L|\ *nodegrid*\ [**+i**\ \|\ **o**\ ] ]
[ |-N| ]
[ |-Q|\ *cut* ]
[ |-S|\ *search\_radius*\ [*unit*] ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: mask_common.rst_

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To make a mask that has a 5 degree radius around points spaced every 10 degree
along the prime meridian, and just paint those areas yellow we try::

    gmt begin mask
      gmt math -T-90/90/10 -N2/1 0 = | gmt mask -Gyellow -I30m -R-75/75/-90/90 -JQ0/7i \
        -S5d -T -Bafg10 -BWSne+t"Mask for points with r = 5 degrees"
    gmt end show

To make an overlay that will mask out the regions of a
contour map where there is no control data using clip polygons, use:

   ::

    gmt mask africa_grav.xyg -R20/40/20/40 -I5m -JM10i -pdf mask

We do it again, but this time we wish to save the clipping polygons to
file all_pols.txt:

   ::

    gmt mask africa_grav.xyg -R20/40/20/40 -I5m -Dall_pols.txt

A repeat of the first example but this time we use white tiling:

   ::

    gmt mask africa_grav.xyg -R20/40/20/40 -I5m -JM10i -T -Gwhite -pdf mask

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`grdmask`, :doc:`surface`,
:doc:`basemap`, :doc:`clip`
