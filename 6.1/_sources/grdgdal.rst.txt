.. index:: ! grdgdal
.. include:: module_core_purpose.rst_

*******
grdgdal
*******

|grdgdal_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdgdal** *infile*
|-A|\ *prog*\ [**+m**\ *method*\ **+c**\ *cpt*]
|-G|\ *outfile* 
[ |-F|\ "*gd opts*"]
[ |-M|\ [**+r+w**]]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-g| ]
[ **-hi** ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

Wrapper module to run some of the GDAL programs (currently *gdalinfo*, *gdaldem*, *gdal_grid*, *gdal_translate*,
*gdal_rasterize*, and *gdalwarp*) from within GMT. Given that GMT and GDAL syntaxes are very different, this module
offers a minimal set of GMT style options but the bulk of them, which use the original GDAL syntax, are passed via
a **-F**\ "*gdal options*". So, users are requested to consult the GDAL programs documentation to learn how to use it.

When writing grids, an option is offered to write them either using the GMT or the GDAL machinery. Each one has
its pros and cons. For example, saving with GMT allows the access by external libraries (*e.g.* Matlab, Julia, Python)
but it requires to make a copy of the saving array. So saving with GDAL may be more efficient but looses the
goodies provided by GMT (but gain its own).

Required Arguments
------------------

*infile*
    Name of an ASCII [or binary, see **-bi**] or an OGR file for programs taking table data, or a 2-D grid file
    for the other cases. File format for grids is detected automatically, but for ingesting OGR formats **-M+r**
    must be used.

.. _-A:

**-A**\ *prog*\ [**+m**\ *method*\ **+c**\ *cpt*]
    Select which GDAL program to run (currently one of *info*, *dem*, *grid*, *rasterize*, *translate* or *warp*).
    When program is *dem* then please append **+m**\ *method* (pick one of *hillshade*, *color-relief*, *slope*, *TRI*, *TPI*
    or *roughness*) and, for *color-relief*, you also need to specify a colormap with **+c**\ *cpt_name*.

.. _-G:

**-G**\ *outfile*
    Output file name. *outfile* is the name of the output grid (or image) file. When saving images, the GDAL machinery is picked by default.

Optional Arguments
------------------

.. _-F:

**-F**\ "*gdal opts*"
    List of GDAL options for the selected program in -A wrapped in double quotes.

.. _-M:

**-M**\ [**+r+w**]
    Read and write files via GDAL. **-M** alone selects both reading and writing with GDAL.
    Option **-M+r** alone instructs the program to read with GDAL (and save with GMT). This option is needed when reading
    OGR vector data. **-M+w** indicates that the output will be saved with GDAL.

.. _-R:

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of the grid. If this subsection exceeds the
    boundaries of the grid, only the common region will be output.
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 3]. This option
    only applies to xyz input via GMT.
.. include:: explain_-bi.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-qi.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_


Examples
--------

To interpolate the x,y,z data at 0.05 increment in a VRT file using the nearest neighbor algorithm
and saving the result as a netCDF::

    gmt grdgdal lixo.vrt -Agrid -R0/10/0/10 -Gjunk.nc -I0.05 -F"-a nearest" -M+r

Now the same as above but saving the grid with GDAL and using the x,y,z point file directly::

    gmt grdgdal lixo.csv -Agrid -R0/10/0/10 -Gjunk.nc -I0.05 -F"-a nearest" -M+w

See Also
--------

:doc:`gmt.conf`,
:doc:`gmt`
