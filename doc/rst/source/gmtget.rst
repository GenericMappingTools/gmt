.. index:: ! gmtget
.. include:: module_core_purpose.rst_

******
gmtget
******

|gmtget_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt get**
[ *PARAMETER1* *PARAMETER2* *PARAMETER3* ... ]
[ |-D|\ *selection* ]
[ |-G|\ *defaultsfile* ]
[ |-I|\ *inc*\ [**m**\|\ **s**] ]
[ |-L| ]
[ |-N| ]
[ |-Q| ]
[ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

Normally, **gmt get** will list the value of one or more specified GMT default parameters.
Alternatively (with **-D**), it will instead download selected sets remote data from the
current GMT data server.

Optional Arguments
------------------

PARAMETER
    Provide one or several parameters of interest. The current value of
    those parameters will be written to *stdout*. For a complete listing
    of available parameters and their meaning, see the :doc:`gmt.conf` man page.

.. _-D:

**-D**\ *selection*
    Download one or more directories of data from the GMT server.  Here,
    *selection* may be *cache* (get the entire cache directory with the files
    used in the GMT examples and tests), *data* (the entire data directory
    on the server), or *all* (both *cache* and *data*).  You can further limit
    the *data* download by appending =\ *planet* or =\ *datasetlist*.  Consult
    the :doc:`/datasets` documentation to see what data sets are currently
    available from the server as this may change over time.

.. _-G:

**-G**\ *defaultsfile*
    Name of specific :doc:`gmt.conf` file to read [Default looks first in
    current directory, then in your home directory, then in ~/.gmt and
    finally in the system defaults].


.. _-I:

**-I**\ *inc*\ [**m**\|\ **s**]
    In conjunction with **-D**, limit the download of grids to those with grid
    spacing equal to or larger than *inc* [no limit].

.. _-L:

**-L**
    Return the values of the parameters on separate lines [Default
    returns all selected parameter values on one line separated by
    spaces].

.. _-N:

**-N**
    Used in conjunction with **-D** and disables the otherwise automatic conversion
    of downloaded compressed JP2000 tiles to locally (compressed) netCDF grids.
    This speeds up the total data request and defers the conversion to when the
    tile is requested by a module.

.. _-Q:

**-Q**
    Can be used in conjunction with **-D** (and **-I**) to provide a listing of
    available datasets (no downloading takes place). The output is one record per
    dataset giving the information as *planet group dataset size nitems remark*.  For datasets
    that are tiled, the *size* is set to N/A (tile sizes vary but are usually just
    a few Mb each) and *nitems* indicates the number of tiles.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help.rst_

Examples
--------

To download all the Earth gridded products, leaving tiles as JP2000 files, try::

    gmt get -Ddata=earth -N

To download just the Earth masks and day image data, with a cutoff at 1x1 arc minutes, try::

    gmt get -Ddata=earth_mask,earth_day -I1m

To download the entire cache directory contents, try::

    gmt get -Dcache

To list the value of the parameter :term:`PS_COMMENTS`::

    gmt get PS_COMMENTS

To get both the values of the parameter
:term:`MAP_GRID_CROSS_SIZE_PRIMARY` and :term:`MAP_GRID_CROSS_SIZE_SECONDARY` on one line, try::

    gmt get MAP_GRID_CROSS_SIZE_PRIMARY MAP_GRID_CROSS_SIZE_SECONDARY

Downloading Data
----------------

Be aware that the GMT data server hosts many tens of Gb of data sets.  The **-D** option
may require considerable free space on your local computer.  Data and cache directories
may be removed by using the :doc:`clear` module.

See Also
--------

:doc:`clear`,
:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`gmtdefaults`,
:doc:`gmtset`
