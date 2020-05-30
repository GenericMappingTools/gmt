.. index:: ! grdconvert
.. include:: module_core_purpose.rst_

***********
grdconvert
***********

|grdconvert_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdconvert** *ingrdfile* |-G|\ *outgrdfile*
[ |-N| ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-Z|\ [**+s**\ *factor*][**+o**\ *offset*] ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdconvert** reads a grid file in one format and writes it out using
another format. As an option the user may select a subset of the data to
be written and to specify scaling, translation, and NaN-value.

Required Arguments
------------------

*ingrdfile*\ [=id[**+s**\ *scale*][**+o**\ *offset*][**+n**\ *invalid*]]
    The grid file to be read. Append format =\ *id* code if not a
    standard COARDS-compliant netCDF grid file. If =\ *id* is set (see
    below), you may optionally append any of **+s**\ *scale*, **+o**\ *offset*,
    and **+n**\ *invalid*. The first two options will scale the data
    and then offset them with the specified amounts after reading
    while the latter lets you supply a value that represents an invalid
    grid entry, i.e., 'Not-a-Number' (for floating-point grids this is
    unnecessary since the IEEE NaN is used; however integers need a
    value which means no data available). When *id=gd*, the file will
    be read using the GDAL library, which will take care to detect the
    format of the file being read. This mechanism is actually used
    automatically when the file format is not one of those that GMT
    recognize. However, sometimes the guessing may fail, so adding
    *id=gd* forces a read via GDAL.
    See Section :ref:`grid-file-format` of the GMT Technical Reference and Cookbook for more information.

.. _-G:

**-G**\ *outgrdfile*\ [=id[**+s**\ *scale*][**+o**\ *offset*][**+n**\ *invalid*]][*:driver*\ [/*datatype*]]]
    The grid file to be written. Append format =\ *id* code if not a
    standard COARDS-compliant netCDF grid file. If =\ *id* is set (see
    below), you may optionally append  any of **+s**\ *scale*,
    **+o**\ *offset*, and **+n**\ *invalid*.  These modifiers are
    particularly practical when storing the data as integers, by
    first removing an offset and then scaling down the values.
    Since the scale and offset are applied in reverse order when
    reading, this does not affect the data values (except for
    round-offs).  The **+n** modifier let you append a value
    that represents 'Not-a-Number' (for floating-point grids this is
    unnecessary since the IEEE NaN is used; however integers need a
    value which means no data available). You may specify **+s**\ *a*
    for auto-adjusting the scale and/or offset of packed integer grids
    (=\ *id*\ **+s**\ *a* is a shorthand for =\ *id*\ **+s**\ *a*\ **+o**\ *a*).
    When *id*\ =\ *gd*, the file will be saved using the
    GDAL library. Append the format *:driver* and optionally the output
    *datatype*. The driver names are those used by GDAL itself (e.g.,
    netCDF, GTiFF, etc.), and the data type is one of
    *u8*\|\ *u16*\|\ *i16*\|\ *u32*\|\ *i32*\|\ *float32*,
    where 'i' and 'u' denote signed and unsigned integers respectively.
    The default type is *float32*. Note also that both driver names and
    data types are case insensitive.
    See Section :ref:`grid-file-format` of the GMT Technical Reference and Cookbook for more information.

    Consider setting :term:`IO_NC4_DEFLATION_LEVEL`
    to reduce file size and to further increase read/write performance.
    Especially when working with subsets of global grids, masks, and grids with
    repeating grid values, the improvement is usually significant.

Optional Arguments
------------------

.. _-N:

**-N**
    Suppress the writing of the GMT header structure. This is useful
    when you want to write a native grid to be used by external tools
    that do not recognize GMT headers. It
    only applies to native grids and is ignored for netCDF output.

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-Z:

**-Z**\ [**+s**\ *factor*][**+o**\ *offset*]
    Use to subtract *offset* from the data and then multiply the results by
    *factor* before writing the output file [1/0]. **Note**: This
    *changes* the values in the grid.  In contrast, while options to supply
    a scale and offset via the **+s** and **+o** modifiers in a file
    name also adjust the data accordingly they also set the scale and
    offset in the metadata, so upon reading the new file you recover
    the original range.  Typically, those options are used to enable
    packing of data via the use of an integer format (see table).

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

Format Identifier
-----------------

By default, grids will be written as floating point data stored in
binary files using the netCDF format and meta-data structure. This
format is conform the COARDS conventions. GMT versions prior to 4.1
produced netCDF files that did not conform to these conventions.
Although these files are still supported, their use is deprecated. To
write other than floating point COARDS-compliant netCDF files, append
the =\ *id* suffix to the filename *outgrdfile*.

When reading files, **grdconvert** and other GMT programs will try
to automatically recognize the type of the input grid file. If this
fails you may append the =\ *id* suffix to the filename *ingrdfile*.

+----------+---------------------------------------------------------------+
| ID       | Explanation                                                   |
+----------+---------------------------------------------------------------+
| **nb**   | GMT netCDF format (8-bit integer, COARDS, CF-1.5)             |
+----------+---------------------------------------------------------------+
| **ns**   | GMT netCDF format (16-bit integer, COARDS, CF-1.5)            |
+----------+---------------------------------------------------------------+
| **ni**   | GMT netCDF format (32-bit integer, COARDS, CF-1.5)            |
+----------+---------------------------------------------------------------+
| **nf**   | GMT netCDF format (32-bit float, COARDS, CF-1.5)              |
+----------+---------------------------------------------------------------+
| **nd**   | GMT netCDF format (64-bit float, COARDS, CF-1.5)              |
+----------+---------------------------------------------------------------+
| **cb**   | GMT netCDF format (8-bit integer, deprecated)                 |
+----------+---------------------------------------------------------------+
| **cs**   | GMT netCDF format (16-bit integer, deprecated)                |
+----------+---------------------------------------------------------------+
| **ci**   | GMT netCDF format (32-bit integer, deprecated)                |
+----------+---------------------------------------------------------------+
| **cf**   | GMT netCDF format (32-bit float, deprecated)                  |
+----------+---------------------------------------------------------------+
| **cd**   | GMT netCDF format (64-bit float, deprecated)                  |
+----------+---------------------------------------------------------------+
| **bm**   | GMT native, C-binary format (bit-mask)                        |
+----------+---------------------------------------------------------------+
| **bb**   | GMT native, C-binary format (8-bit integer)                   |
+----------+---------------------------------------------------------------+
| **bs**   | GMT native, C-binary format (16-bit integer)                  |
+----------+---------------------------------------------------------------+
| **bi**   | GMT native, C-binary format (32-bit integer)                  |
+----------+---------------------------------------------------------------+
| **bf**   | GMT native, C-binary format (32-bit float)                    |
+----------+---------------------------------------------------------------+
| **bd**   | GMT native, C-binary format (64-bit float)                    |
+----------+---------------------------------------------------------------+
| **rb**   | SUN rasterfile format (8-bit standard)                        |
+----------+---------------------------------------------------------------+
| **rf**   | GEODAS grid format GRD98 (NCEI)                               |
+----------+---------------------------------------------------------------+
| **sf**   | Golden Software Surfer format 6 (32-bit float)                |
+----------+---------------------------------------------------------------+
| **sd**   | Golden Software Surfer format 7 (64-bit float, read-only)     |
+----------+---------------------------------------------------------------+
| **af**   | Atlantic Geoscience Center format AGC (32-bit float)          |
+----------+---------------------------------------------------------------+
| **ei**   | ESRI Arc/Info ASCII Grid Interchange format (ASCII integer)   |
+----------+---------------------------------------------------------------+
| **ef**   | ESRI Arc/Info ASCII Grid Interchange format (ASCII float)     |
+----------+---------------------------------------------------------------+
| **gd**   | Import/export through GDAL                                    |
+----------+---------------------------------------------------------------+

GMT Standard Netcdf Files
-------------------------

The standard format used for grdfiles is based on netCDF and conforms to
the COARDS conventions. Files written in this format can be read by
numerous third-party programs and are platform-independent. Some
disk-space can be saved by storing the data as bytes or shorts in stead
of integers. Use the *scale* and *offset* parameters to make this work
without loss of data range or significance. For more details, see
:ref:`GMT File Formats` and :ref:`Write-grids-images`.

**Multi-variable grid files**

By default, GMT programs will read the first 2-dimensional grid
contained in a COARDS-compliant netCDF file. Alternatively, use
*ingrdfile*\ **?**\ *varname* (ahead of any optional suffix **=**\ *id*)
to specify the requested variable *varname*. Since **?** has special
meaning as a wildcard, escape this meaning by placing the full filename
and suffix between quotes.

**Multi-dimensional grids**

To extract one *layer* or *level* from a 3-dimensional grid stored in a
COARDS-compliant netCDF file, append both the name of the variable and
the index associated with the layer (starting at zero) in the form:
*ingrdfile*\ **?**\ *varname*\ **[**\ *layer*\ **]**. Alternatively,
specify the value associated with that layer using parentheses in stead
of brackets:
*ingridfile*\ **?**\ *varname*\ **(**\ *layer*\ **)**.

In a similar way layers can be extracted from 4- or even 5-dimensional
grids. For example, if a grid has the dimensions (parameter, time,
depth, latitude, longitude), a map can be selected by using:
*ingridfile*\ **?**\ *varname*\ **(**\ *parameter*,\ *time*,\ *depth*\ **)**.

Since question marks, brackets and parentheses have special meanings on
the command line, escape these meanings by placing the full filename and
suffix between quotes.

Native Binary Files
-------------------

For binary native GMT files the size of the GMT grid header block
is *hsize* = 892 bytes, and the total size of the file is *hsize* + *nx*
\* *ny* \* *item_size*, where *item_size* is the size in bytes of each
element (1, 2, 4). Bit grids are stored using 4-byte integers, each
holding 32 bits, so for these files the size equation is modified by
using ceil (*nx* / 32) \* 4 instead of *nx*. Note that these files are
platform-dependent. Files written on Little Endian machines (e.g., PCs)
can not be read on Big Endian machines (e.g., most workstations). Also
note that it is not possible for GMT to determine uniquely if a
4-byte grid is float or int; in such cases it is best to use the *=ID*
mechanism to specify the file format. In all cases a native grid is
considered to be signed (i.e., there are no provision for unsigned short
ints or unsigned bytes). For header and grid details, see :ref:`GMT File Formats`.

.. include:: explain_float.rst_

Examples
--------

.. include:: explain_example.rst_

To extract the second layer from a 3-dimensional grid named temp from a
COARDS-compliant netCDF file climate.nc:

   ::

    gmt grdconvert climate.nc?temp[1] -Gtemp.nc -V

To create a 4-byte native floating point grid from the COARDS-compliant
remote netCDF file AFR.nc:

   ::

    gmt grdconvert @AFR.nc -GAFR_bin.b4=bf -V

To make a 2-byte short integer file, scale it by 10, subtract 32000,
setting NaNs to -9999, do

   ::

    gmt grdconvert values.nc -Gshorts.i2=bs+s10+o-32000+n-9999 -V

To create a Sun standard 8-bit rasterfile for a subset of the data file
image.nc, assuming the range in image.nc is 0-1 and we need 0-255, run

   ::

    gmt grdconvert image.nc -R-60/-40/-40/-30 -Gimage.ras8=rb+s255 -V

See Also
--------

:doc:`gmt.conf`,
:doc:`gmt`,
:doc:`grdmath`
