*******
kml2gmt
*******


kml2gmt - Extract GMT table data from Google Earth KML files

`Synopsis <#toc1>`_
-------------------

**kml2gmt** [ *kmlfiles* ] [ **-V**\ [*level*\ ] ] [ **-Z** ] [
**-bo**\ [*ncol*\ ][**t**\ ] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**kml2gmt** reads a Google Earth KML file and outputs a GMT table file.
Only KML files that contain points, lines, or polygons can be processed.
This is a bare-bones operation that aims to extract coordinates and
possibly the name and description tags of each feature. The main use
intended is to capture coordinates modified in Google Earth and then
reinsert the modified data into the original GMT data file. For a more
complete reformatting, consider using **ogr2ogr -f** "GMT" somefile.gmt
somefile.kml.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

None.

`Optional Arguments <#toc5>`_
-----------------------------

*kmlfiles*
    Name of one or more KML files to work on. If not are given, then
    standard input is read.
**-Z**
    Output the altitude coordinates as GMT z coordinates [Default will
    output just longitude and latitude].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-bo**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary output.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Examples <#toc6>`_
-------------------

To extract the lon,lat values from the KML file google.kml, try

kml2gmt google.kml -V > google.txt

`See Also <#toc7>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*gmt.conf*\ (5) <gmt.conf.5.html>`_ ,
`*img2google*\ (1) <img2google.1.html>`_ ,
`*ps2raster*\ (1) <ps2raster.1.html>`_
`*gmt2kml*\ (1) <gmt2kml.1.html>`_

