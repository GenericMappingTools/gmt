.. index:: ! grd2kml
.. include:: module_core_purpose.rst_

*******
grd2kml
*******

|grd2kml_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grd2kml** *grid* 
[ |-C|\ *cpt* ]
[ |-E|\ *URL* ]
[ |-F|\ *filtercode* ]
[ |-H|\ *factor* ]
[ |-I|\ [*intensfile*\ \|\ *intensity*\ \|\ *modifiers*] ]
[ |-L|\ *tilesize* ]
[ |-N|\ *prefix* ]
[ |-Q| ]
[ |-T|\ *title* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grd2kml** reads a 2-D grid file and makes a *quadtree* of
PNG images and KML wrappers for Google Earth using the selected
tile size [256x256 pixels].  We downsample the grid depending on the
viewing level in the quadtree using a Gaussian filter, but other
filters can be selected as well.
Optionally, illumination may be added by providing a grid file with
intensities in the (-1,+1) range or by giving instructions to derive intensities
from the input data grid automatically (see **-I**). Values outside the (-1,+1) intensity range will be
clipped. Map colors are specified via a color palette lookup table.


Required Arguments
------------------

*grid*
    A 2-D gridded data set (See GRID FILE FORMATS below.)

Optional Arguments
------------------

.. _-C:

**-C**\ [*cpt* \|\ *master*\ [**+i**\ *zinc*] \|\ *color1,color2*\ [,\ *color3*\ ,...]]
    Name of the CPT (for *grd_z* only). Alternatively,
    supply the name of a GMT color master dynamic CPT [turbo, but geo
    for @earth_relief and srtm for @srtm_relief data] to
    automatically determine a continuous CPT from
    the grid's z-range; you may round up/down the z-range by adding **+i**\ *zinc*.
    Yet another option is to specify **-C**\ *color1*\ ,\ *color2*\ [,\ *color3*\ ,...]
    to build a linear continuous CPT from those colors automatically.
    In this case *color1* etc can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).  If no argument is given to **-C**
    then under modern mode we select the current CPT.

.. _-E:

**-E**\ *URL*
    Instead of hosting all files on your computer, you may prepend a remote site URL. Then,
    the top-level *prefix*\ .kml file will use this URL to find all other files it references.
    After building completes you must place the entire *prefix* directory at the remote
    location pointed to by the *URL* [local files only]. With this arrangement you can
    share the *prefix*\ .kml with others (say, via email or for download) and users can
    open the file in their Google Earth and access the remote files from your server as needed.

.. _-F:

**-F**\ *filtercode*

    Specifies the filter to use for the downsampling of the grid for more
    distant viewing.  Choose among **b**\ oxcar, **c**\ osine arch,
    **g**\ aussian, or **m**\ edian [Gaussian].  The filter width is set
    automatically depending on the level.

.. _-H:

**-H**\ *factor*
    Improve the quality of rasterization by passing the sub-pixel smoothing factor
    to psconvert (same as **-H** option in psconvert) [no sub-pixel smoothing].

.. _-I:

**-I**\ [*intensfile*\ \|\ *intensity*\ \|\ *modifiers*]
    Gives the name of a grid file with intensities in the (-1,+1) range,
    or a constant intensity to apply everywhere (affects the ambient light).
    Alternatively, derive an intensity grid from the input data grid *grid*
    via a call to :doc:`grdgradient`; append **+a**\ *azimuth* and **+n**\ *args*
    to specify azimuth and intensity arguments for that module or just give **+d**
    to select the default arguments (**+a**\ -45\ **+nt**\ 1). If you want a more
    specific intensity scenario then run :doc:`grdgradient` separately first.
    [Default is no illumination].


.. _-L:

**-L**\ *tilesize*
    Sets the fixed size of the image building blocks.  Must be an integer that
    is radix 2.  Typical values are 256 or 512 [256].

.. _-N:

**-N**\ *prefix*
    Sets a unique name prefixed used for the top-level KML filename *and* the
    directory where all referenced KML files and PNG images will be written [GMT_Quadtree].

.. _-Q:

**-Q**
    Make grid nodes with z = NaN transparent, using the color-masking
    feature in PostScript Level 3 (the PS device must support PS Level 3).

.. _-T:

**-T**\ *title*
    Sets the title of the top-level document (i.e., its description).

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

Notes
-----

The intensity grid can be created from the data grid using
:doc:`grdgradient` and, optionally, modified by :doc:`grdmath` or
:doc:`grdhisteq`.  Custom intensity grids built with several different
illumination angles can be combined with :doc:`grdmath`.  For a single
illumination angle the automatic illumination can be used instead.

Examples
--------

.. include:: explain_example.rst_

To make a quadtree image representation of the large topography grid file ellice_basin.nc, using
the default tile size, supply automatic shading based on the topography, and use the larger 512x512 tiles,
supplying a suitable title, try

   ::

    gmt grd2kml ellice_basin.nc -I+d -Nellice -L512 -T"Ellice Basin Bathymetry"

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`gmt2kml`,
:doc:`grdgradient`,
:doc:`grdhisteq`,
:doc:`grdimage`,
:doc:`grdmath`,
:doc:`kml2gmt`,
:doc:`psconvert`
