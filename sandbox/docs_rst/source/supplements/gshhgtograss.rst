**************
gshhgtograss
**************

gshhg2grass - Extracting GSHHG and WDBII data in GRASS-compatible ASCII
format

`Synopsis <#toc1>`_
-------------------

**gshhg2grass** **-i** gshhs\_[f\|h\|i\|l\|c].b [ **-x**\ *minx* ]
[**-X**\ *maxx* ] [ **-y**\ *miny* ] [ **-Y**\ *maxy* ]

`Description <#toc2>`_
----------------------

**gshhg2grass** reads the binary coastline (GSHHG) and and translates it
into an ASCII format suitable for import into GRASS. It automatically
handles byte-swabbing between different architectures.

`Required Arguments <#toc3>`_
-----------------------------

*gshhs\_[f\|h\|i\|l\|c].b*
    One of the GSHHG binary data file as distributed with the GSHHG data
    supplement. Any of the 5 standard resolutions (full, high,
    intermediate, low, crude) can be used. The resulting files are
    called dig\_[ascii\|att\|cats].gshhs\_[f\|h\|i\|l\|c].

`Optional Arguments <#toc4>`_
-----------------------------

**-x**\ *minx*
    Specify a minimum (west) longitude.
**-X**\ *maxx*
    Specify a maximum (east) longitude.
**-y**\ *miny*
    Specify a minimum (south) latitude.
**-Y**\ *maxy*
    Specify a maximum (north) latitude.

`Examples <#toc5>`_
-------------------

To convert the full GSHHG data set , try

**gshhg2grass** *gshhs\_f.b*

`Bugs <#toc6>`_
---------------

Not updated to handle the WDBII line data (borders or rivers).

`Author <#toc7>`_
-----------------

Original version by Simon Cox (simon@ned.dem.csiro.au) with some
maintenance by Paul Wessel (pwessel@hawaii.edu).

`See Also <#toc8>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ , `*gshhs*\ (1) <gshhs.html>`_
`*gshhs\_dp*\ (1) <gshhs_dp.html>`_
