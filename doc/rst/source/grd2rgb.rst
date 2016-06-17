.. index:: ! grd2rgb

*******
grd2rgb
*******

.. only:: not man

    grd2rgb - Write r/g/b grid files from a grid file, a raw RGB file, or SUN rasterfile

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**grd2rgb** *infile* |-G|\ *template* 
[ |-C|\ *cpt* ] [
|-I|\ *xinc*\ [**m**\ \|\ **s**][/\ *yinc*\ [**m**\ \|\ **s**]] ]
[ |-L|\ *layer* ]
|SYN_OPT-R|
|SYN_OPT-V|
[ |-W|\ *width*/*height*\ [/*n\_bytes*] ]
[ **-r** ]

|No-spaces|

Description
-----------

**grd2rgb** reads one of three types of input files: (1) A Sun 8-, 24-,
or 32-bit raster file; we the write out the red, green, and blue
components (0-255 range) to separate grid files. Since the raster file
header is limited you may use the **-R**, **-I**, **-r** options to set
a complete header record [Default is simply based on the number of rows
and columns]. (2) A binary 2-D grid file; we then convert the z-values
to red, green, blue via the provided CPT. Optionally, only write
out one of the r, g, b, layers. (3) A RGB or RGBA raw raster file. Since
raw rasterfiles have no header, you have to give the image dimensions
via the **-W** option.

Required Arguments
------------------

*infile*
    The (1) Sun raster file, (2) 2-D binary grid file, or (3) raw raster file to be converted.
**-G**\ *template*
    Provide an output name template for the three output grids. The
    template should be a regular grid file name except it must contain
    the string %c which on output will be replaced by r, g, or b. 

Optional Arguments
------------------

.. _-C:

**-C**\ *cpt*
    name of the color palette table (for 2-D binary input grid only). 

.. _-I:

..  include:: explain_-I.rst_

.. _-L:

**-L**\ *layer*
    Output only the specified layer (r, g, or b). [Default outputs all 3 layers]. 

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *width*/*height*\ [/*n\_bytes*]
    Sets the size of the raw raster file. By default an RGB file (which
    has 3 bytes/pixel) is assumed. For RGBA files use *n\_bytes* = 4.
    Use **-W** for guessing the image size of a RGB raw file, and
    **-W**\ *=/=/4* if the raw image is of the RGBA type. Notice that
    this might be a bit slow because the guessing algorithm makes uses
    of FFTs. 

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_help.rst_

Examples
--------

To use the color palette topo.cpt to create r, g, b component grids from
hawaii_grv.nc file, use

   ::

    gmt grd2rgb hawaii_grv.nc -Ctopo.cpt -Ghawaii_grv_%c.nc

To output the red component from the Sun raster radiation.ras file, use

   ::

    gmt grd2rgb radiation.ras -Lr -Gcomp_%c.nc

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`grdedit`, :doc:`grdimage`,
:doc:`grdmath` , :doc:`grdview`
