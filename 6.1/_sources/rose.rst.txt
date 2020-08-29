.. index:: ! rose
.. include:: module_core_purpose.rst_

******
rose
******

|rose_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt rose** [ *table* ] [ |-A|\ *sector_width*\ [**+r**] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D| ]
[ |-E|\ **m**\|\ [**+w**]\ *mode_file* ]
[ |-F| ]
[ |-G|\ *fill* ] [ |-I| ]
[ |-J|\ **X**\ *diameter* ]
[ |-L|\ [*wlabel*\ ,\ *elabel*\ ,\ *slabel*\ ,\ *nlabel*] ]
[ |-M|\ *parameters* ]
[ |-Q|\ *alpha* ]
[ |-R|\ *r0*/*r1*/*az0*/*az1* ]
[ |-S| ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [**v**]\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ **u**\|\ *scale* ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: rose_common.rst_

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To plot a half circle rose diagram of the data in the remote data file
azimuth_lengths.txt (containing pairs of (azimuth, length in meters)),
using a 5 degree bin sector width, on a circle of diameter = 10 cm, using a light blue
shading, try::

    gmt rose @azimuth_lengths.txt -: -A5 -JX10c -F -L -Glightblue -R0/1/0/180 -Bxaf+l"Fault length" -Byg30 -S -pdf half_rose

To plot a full circle wind rose diagram of the data in the file
lines.r_az, on a circle of diameter = 10 cm, grid going out to radius =
500 units in steps of 100 with a 45 degree sector interval, using a
solid pen (width = 0.5 point, and shown in landscape [Default]
orientation with a timestamp and command line plotted, use:

   ::

    gmt rose lines.az_r -R0/500/0/360 -JX10c -Bxg100 -Byg45 -B+t"Windrose diagram" -W0.5p -U+c -pdf rose

Redo the same plot but this time add orange vector heads to each direction (with nominal head size
0.5 cm but this will be reduced linearly for lengths less than 1 cm) and save the plot, use:

   ::

    gmt rose lines.az_r -R0/500/0/360 -JX10c -Bxg100 -Byg45 -B+t"Windrose diagram" -M0.5c+e+gorange+n1c -W0.5p -U+c -pdf rose

.. include:: rose_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`, :doc:`histogram`
