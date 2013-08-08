.. index:: ! kml2gmt

*******
kml2gmt
*******

.. only:: not man

    kml2gmt - Extract GMT table data from Google Earth KML files

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**kml2gmt** [ *kmlfiles* ] [ |SYN_OPT-V| ] [ **-Z** ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**kml2gmt** reads a Google Earth KML file and outputs a GMT table file.
Only KML files that contain points, lines, or polygons can be processed.
This is a bare-bones operation that aims to extract coordinates and
possibly the name and description tags of each feature. The main use
intended is to capture coordinates modified in Google Earth and then
reinsert the modified data into the original GMT data file. For a more
complete reformatting, consider using **ogr2ogr -f** "GMT" somefile.gmt
somefile.kml. 

Required Arguments
------------------

None.

Optional Arguments
------------------

*kmlfiles*
    Name of one or more KML files to work on. If not are given, then
    standard input is read.
**-Z**
    Output the altitude coordinates as GMT z coordinates [Default will
    output just longitude and latitude]. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bo.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

Examples
--------

To extract the lon,lat values from the KML file google.kml, try

    gmt kml2gmt google.kml -V > google.txt

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`img2google <supplements/img/img2google>`,
:doc:`ps2raster`, :doc:`gmt2kml`
