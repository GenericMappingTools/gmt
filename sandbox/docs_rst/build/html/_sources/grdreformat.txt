***********
grdreformat
***********


grdreformat - Convert between different grid formats

`Synopsis <#toc1>`_
-------------------

**grdreformat** *ingrdfile*\ [*=id*\ [*/scale/offset*\ [*/NaNvalue*\ ]]]
*outgrdfile*\ [*=id*\ [*/scale/offset*\ [*/NaNvalue*\ ]]] [ **-N** ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [ **-V**\ [*level*\ ] ]

`Description <#toc2>`_
----------------------

**grdreformat** reads a grid file in one format and writes it out using
another format. As an option the user may select a subset of the data to
be written and to specify scaling, translation, and NaN-value.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc5>`_
-----------------------------

*ingrdfile*
    The grid file to be read. Append format =\ *id* code if not a
    standard COARDS-compliant netCDF grid file. If =\ *id* is set (see
    below), you may optionally append *scale* and *offset*. These
    options will scale the data and then offset them with the specified
    amounts after reading.
    If *scale* and *offset* are supplied you may also append a value
    that represents ’Not-a-Number’ (for floating-point grids this is
    unnecessary since the IEEE NaN is used; however integers need a
    value which means no data available.)
*outgrdfile*
    The grid file to be written. Append format =\ *id* code if not a
    standard COARDS-compliant netCDF grid file. If =\ *id* is set (see
    below), you may optionally append *scale* and *offset*. These
    options are particularly practical when storing the data as
    integers, first removing an offset and then scaling down the values.
    Since the scale and offset are applied in reverse order when
    reading, this does not affect the data values (except for
    round-offs).
    If *scale* and *offset* are supplied you may also append a value
    that represents ’Not-a-Number’ (for floating-point grids this is
    unnecessary since the IEEE NaN is used; however integers need a
    value which means no data available.)

`Optional Arguments <#toc6>`_
-----------------------------

**-N**
    Suppress the writing of the **GMT** header structure. This is useful
    when you want to write a native grid to be used by **grdraster**. It
    only applies to native grids and is ignored for netCDF output.
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Format Identifier <#toc7>`_
----------------------------

By default, grids will be written as floating point data stored in
binary files using the netCDF format and meta-data structure. This
format is conform the COARDS conventions. **GMT** versions prior to 4.1
produced netCDF files that did not conform to these conventions.
Although these files are still supported, their use is deprecated. To
write other than floating point COARDS-compliant netCDF files, append
the =\ *id* suffix to the filename *outgrdfile*.
When reading files, **grdreformat** and other **GMT** programs will
automatically recognize any type of netCDF grid file. These can be in
either COARDS-compliant or pre-4.1 format, and contain floating-point or
integer data. To read other types of grid files, append the =\ *id*
suffix to the filename *ingrdfile*.
**nb** GMT netCDF format (byte) (COARDS-compliant)
**ns** GMT netCDF format (short) (COARDS-compliant)
**ni** GMT netCDF format (int) (COARDS-compliant)
**nf** GMT netCDF format (float) (COARDS-compliant)
**nd** GMT netCDF format (double) (COARDS-compliant)
**cb** GMT netCDF format (byte) (deprecated)
**cs** GMT netCDF format (short) (deprecated)
**ci** GMT netCDF format (int) (deprecated)
**cf** GMT netCDF format (float) (deprecated)
**cd** GMT netCDF format (double) (deprecated)
**bm** GMT native, C-binary format (bit-mask)
**bb** GMT native, C-binary format (byte)
**bs** GMT native, C-binary format (short)
**bi** GMT native, C-binary format (int)
**bf** GMT native, C-binary format (float)
**bd** GMT native, C-binary format (double)
**rb** SUN rasterfile format (8-bit standard)
**rf** GEODAS grid format GRD98 (NGDC)
**sf** Golden Software Surfer format 6 (float)
**sd** Golden Software Surfer format 7 (double, read-only)
**af** Atlantic Geoscience Center format AGC (float)
**ei** ESRI Arc/Info ASCII Grid Interchange format (integer)
**ef** ESRI Arc/Info ASCII Grid Interchange format (float)
**gd** Import through GDAL (convert to float)

`Gmt Standard Netcdf Files <#toc8>`_
------------------------------------

The standard format used for grdfiles is based on netCDF and conforms to
the COARDS conventions. Files written in this format can be read by
numerous third-party programs and are platform-independent. Some
disk-space can be saved by storing the data as bytes or shorts in stead
of integers. Use the *scale* and *offset* parameters to make this work
without loss of data range or significance. For more details, see
Appendix B.

**Multi-variable grid files**
By default, **GMT** programs will read the first 2-dimensional grid
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
*ingridfile*\ **?\ `*varname*\ **(**\ *level*\ **)** <varname.level.html>`_
.
In a similar way layers can be extracted from 4- or even 5-dimensional
grids. For example, if a grid has the dimensions (parameter, time,
depth, latitude, longitude), a map can be selected by using:
*ingridfile*\ **?**\ *varname*\ **(**\ *parameter*,\ *time*,\ *depth*\ **)**.
Since question marks, brackets and parentheses have special meanings on
the command line, escape these meanings by placing the full filename and
suffix between quotes.**

`Native Binary Files <#toc9>`_
------------------------------

For binary native **GMT** files the size of the **GMT** grdheader block
is *hsize* = 892 bytes, and the total size of the file is *hsize* + *nx*
\* *ny* \* *item\_size*, where *item\_size* is the size in bytes of each
element (1, 2, 4). Bit grids are stored using 4-byte integers, each
holding 32 bits, so for these files the size equation is modified by
using ceil (*nx* / 32) \* 4 instead of *nx*. Note that these files are
platform-dependent. Files written on Little Endian machines (e.g. PCs)
can not be read on Big Endian machines (e.g. most workstations). Also
note that it is not possible for **GMT** to determine uniquely if a
4-byte grid is float or int; in such cases it is best to use the *=ID*
mechanism to specify the file format. In all cases a native grid is
considered to be signed (i.e., there are no provision for unsigned short
ints or unsigned bytes). For header and grid details, see Appendix B.

`Grid Values Precision <#toc10>`_
---------------------------------

Regardless of the precision of the input data, GMT programs that create
grid files will internally hold the grids in 4-byte floating point
arrays. This is done to conserve memory and furthermore most if not all
real data can be stored using 4-byte floating point values. Data with
higher precision (i.e., double precision values) will lose that
precision once GMT operates on the grid or writes out new grids. To
limit loss of precision when processing data you should always consider
normalizing the data prior to processing.

`Examples <#toc11>`_
--------------------

To extract the second layer from a 3-dimensional grid named temp from a
COARDS-compliant netCDF file climate.nc:

grdreformat climate.nc?temp[1] temp.nc -V

To create a 4-byte native floating point grid from the COARDS-compliant
netCDF file data.nc:

grdreformat data.nc ras\_data.b4=bf -V

To make a 2-byte short integer file, scale it by 10, subtract 32000,
setting NaNs to -9999, do

grdreformat values.nc shorts.i2=bs/10/-32000/-9999 -V

To create a Sun standard 8-bit rasterfile for a subset of the data file
image.nc, assuming the range in image.nc is 0-1 and we need 0-255, run

grdreformat image.nc -R-60/-40/-40/-30 image.ras8=rb/255/0 -V

To convert etopo2.nc to etopo2.i2 that can be used by **grdraster**, try

grdreformat etopo2.nc etopo2.i2=bs -N -V

`See Also <#toc12>`_
--------------------

`*gmt*\ <gmt.html>`_ , `*grdmath*\ <grdmath.html>`_

