.. index:: ! psrose

******
psrose
******

.. only:: not man

    psrose - Plot a polar histogram (rose, sector, windrose diagrams)

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psrose** [ *table* ] [ |-A|\ *sector_width*\ [**+r**] ]
[ |SYN_OPT-B| ]
[ |-C|\ **m**\ \|\ [**+w**\ ]\ *mode_file* ]
[ |-D| ]
[ |-F| ]
[ |-G|\ *fill* ] [ |-I| ]
[ |-J|\ **X**\ *diameter* ]
[ |-K| ]
[ |-L|\ [\ *wlabel*\ ,\ *elabel*\ ,\ *slabel*\ ,\ *nlabel*\ ] ]
[ |-M|\ *parameters* ]
[ |-O| ] [ |-P| ]
[ |-Q|\ *alpha* ]
[ |-R|\ *r0*/*r1*/*az_0*/*az_1* ]
[ |-S| ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [**v**]\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ **u**\ \|\ *scale* ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: rose_common.rst_

.. include:: common_classic.rst_

Examples
--------

To plot a half circle rose diagram of the data in the file
fault_segments.az_r (containing pairs of (azimuth, length in meters),
using a 10 degree bin sector width, on a circle of diameter = 6 inch, grid
going out to radius = 150 km in steps of 25 km with a 30 degree sector
interval, radial direction annotated every 50 km, using a light blue
shading outlined by a solid red pen (width = 0.75 points), draw the mean
azimuth, and shown in Portrait orientation, use:

   ::

    gmt psrose fault_segments.az_r -R0/150/-90/90 -Bx50g25+l"Fault length"
               -Byg30 -B+t"Rose diagram" -JX6i -A10+r -Glightblue
               -W0.75p,red -Z0.001 -Cm -P -T -: > half_rose.ps

To plot a full circle wind rose diagram of the data in the file
lines.r_az, on a circle of diameter = 10 cm, grid going out to radius =
500 units in steps of 100 with a 45 degree sector interval, using a
solid pen (width = 0.5 point, and shown in landscape [Default]
orientation with UNIX timestamp and command line plotted, use:

   ::

    gmt psrose lines.az_r -R0/500/0/360 -JX10c -Bxg100 -Byg45 -B+t"Windrose diagram" -W0.5p -U+c | lpr

Redo the same plot but this time add orange vector heads to each direction (with nominal head size
0.5 cm but this will be reduced linearly for lengths less than 1 cm) and save the plot, use:

   ::

    gmt psrose lines.az_r -R0/500/0/360 -JX10c -Bxg100 -Byg45 -B+t"Windrose diagram" -M0.5c+e+gorange+n1c -W0.5p -U+c > rose.ps

.. include:: rose_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`, :doc:`pshistogram`
