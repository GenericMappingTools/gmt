.. index:: ! grdimage

********
grdimage
********

.. only:: not man

    grdimage - Project grids or images and plot them on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**grdimage** *grd_z* \| *grd_r grd_g grd_b*
[ **-A**\ *out_img*\ **=**\ *driver* ] [ **-C**\ *cptfile* ]
[ **-D**\ [**r**\ ] ] **-J**\ *parameters*
[ |SYN_OPT-U| ]
[ **-G**\ [**f**\ \|\ **b**]\ *color* ] [ **-I**\ *intensfile*\ \|\ *intensity* ]
[ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [ **-M** ] [ **-N** ]
[ **-O** ] [ **-P** ] [ **-Q** ]
[ |SYN_OPT-Rz| ]
[ **-T** ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-p| ]
[ **-tr** ]

|No-spaces|

Description
-----------

**grdimage** reads one 2-D grid file and produces a gray-shaded (or
colored) map by plotting rectangles centered on each grid node and
assigning them a gray-shade (or color) based on the z-value.
Alternatively, **grdimage** reads three 2-D grid files with the red,
green, and blue components directly (all must be in the 0-255 range).
Optionally, illumination may be added by providing a file with
intensities in the (-1,+1) range. Values outside this range will be
clipped. Such intensity files can be created from the grid using
:doc:`grdgradient` and, optionally, modified by :doc:`grdmath` or
:doc:`grdhisteq`. Yet as a third alternative available when GMT is build
with GDAL support the grd_z file can be an image referenced or not
(than see **-Dr**). In this case the images can be illuminated with the
file provided via the **-I** option. Here if image has no coordinates
those of the intensity file will be used.

When using map projections, the grid is first resampled on a new
rectangular grid with the same dimensions. Higher resolution images can
be obtained by using the **-E** option. To obtain the resampled value
(and hence shade or color) of each map pixel, its location is inversely
projected back onto the input grid after which a value is interpolated
between the surrounding input grid values. By default bi-cubic
interpolation is used. Aliasing is avoided by also forward projecting
the input grid nodes. If two or more nodes are projected onto the same
pixel, their average will dominate in the calculation of the pixel
value. Interpolation and aliasing is controlled with the **-n** option.

The **-R** option can be used to select a map region larger or smaller
than that implied by the extent of the grid.

A (color) PostScript file is output. 

Required Arguments
------------------

*grd_z* \| *grd_r grd_g grd_b*
    2-D gridded data set (or red, green, blue grids) to be imaged (See
    GRID FILE FORMATS below.) 

.. include:: explain_-J.rst_

Optional Arguments
------------------

**-A**\ *out\_img*\ **=**\ *driver*
    With GDAL aware versions: save image in a raster format instead of
    PostScript. Append *out\_img*\ **=**\ *driver* to select the file
    name and image format. The *driver* is the driver code name used by
    GDAL. For example, **-A**\ img.tif=GTiff will write a GeoTiff image
    if the subset of GMT syntax projections that is currently possible
    to translate into the PROJ4 syntax allows it, or a plain tiff file
    otherwise. Note: any vector elements are lost. 

.. include:: explain_-B.rst_

**-C**\ *cptfile*
    Name of the color palette table (for *grd\_z* only). Alternatively,
    supply the name of a GMT color master CPT [rainbow] and let
    **grdimage** automatically determine a 16-level continuous CPT from
    the grid's z-range.
**-D**\ [**r**]
    Specifies that the grid supplied is an image file to be read via
    GDAL. Obviously this option will work only with GMT versions
    built with GDAL support. The image can be indexed or true color
    (RGB) and can be an URL of a remotely located file. That is **-D**
    `http://www.somewhere.com/image.jpg <http://www.somewhere.com/image.jpg>`_
    is a valid file syntax. Note, however, that to use it this way you
    must not be blocked by a proxy. If you are, chances are good that it
    can work by setting the environmental variable
    *http_proxy* with the value 'your_proxy:port' Append
    **r** to use the region specified by **-R** to apply to the image.
    For example, if you have used **-Rd** then the image will be
    assigned the limits of a global domain. The interest of this mode is
    that you can project a raw image (an image without referencing
    coordinates).
**-Ei**\ [\|*dpi*]
    Sets the resolution of the projected grid that will be created if a
    map projection other than Linear or Mercator was selected [100]. By
    default, the projected grid will be of the same size (rows and
    columns) as the input file. Specify **i** to use the PostScript
    image operator to interpolate the image at the device resolution.
**-G**\ [**f**\ \|\ **b**]\ *color*
    This option only applies when the resulting image otherwise would
    consist of only two colors: black (0) and white (255). If so, this
    option will instead use the image as a transparent mask and paint
    the mask (or its inverse, with **-Gb**) with the given color
    combination.
**-I**\ *intensfile*\ \|\ *intensity*
    Gives the name of a grid file with intensities in the (-1,+1) range,
    or a constant intensity to apply everywhere.
    [Default is no illumination]. 

.. include:: explain_-Jz.rst_

.. include:: explain_-K.rst_

**-M**
    Force conversion to monochrome image using the (television) YIQ
    transformation. Cannot be used with **-Q**.
**-N**
    Do not clip the image at the map boundary (only relevant for
    non-rectangular maps). 

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-Q**
    Make grid nodes with z = NaN transparent, using the colormasking
    feature in PostScript Level 3 (the PS device must support PS Level 3). 

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| replace:: You may ask for a larger
    *w/e/s/n* region to have more room between the image and the axes. A
    smaller region than specified in the grid file will result in a
    subset of the grid [Default is the region given by the grid file].
.. include:: explain_-Rz.rst_
    

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-XY.rst_

.. include:: explain_-c.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_-n.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout.rst_


Imaging Grids With Nans
-----------------------

Be aware that if your input grid contains patches of NaNs, these patches
can become larger as a consequence of the resampling that must take
place with most map projections. Because **grdimage** uses the
PostScript colorimage operator, for most non-linear projections we
must resample your grid onto an equidistant rectangular lattice. If you
find that the NaN areas are not treated adequately, consider (a) use a
linear projection, or (b) use :doc:`grdview` **-Ts** instead.

.. include:: explain_grdresample.rst_

Examples
--------

For a quick-and-dirty color map of the data in the file stuff.nc, with
the maximum map dimension limited to be 6 inches, try

   ::

    gmt grdimage stuff.nc -JX6i+ > quick.ps

To gray-shade the file hawaii_grav.nc with shades given in shades.cpt
on a Lambert map at 1.5 cm/degree along the standard parallels 18 and
24, and using 1 degree tickmarks:

   ::

    gmt grdimage hawaii_grav.nc -Jl18/24/1.5c -Cshades.cpt -B1 > hawaii_grav_image.ps

To create an illuminated color PostScript plot of the gridded data set
image.nc, using the intensities provided by the file intens.nc, and
color levels in the file colors.cpt, with linear scaling at 10
inch/x-unit, tickmarks every 5 units:

   ::

    gmt grdimage image.nc -Jx10i -Ccolors.cpt -Iintens.nc -B5 > image.ps

To create an false color PostScript plot from the three grid files
red.nc, green.nc, and blue.nc, with linear scaling at 10 inch/x-unit,
tickmarks every 5 units:

   ::

    gmt grdimage red.nc green.nc blue.nc -Jx10i -B5 > rgbimage.ps

When GDAL support is built in: To create a sinusoidal projection of a
remotely located Jessica Rabbit

   ::

    gmt grdimage -JI15c -Rd -Dr
        http://larryfire.files.wordpress.com/2009/07/untooned_jessicarabbit.jpg \
        -P > jess.ps

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`grd2rgb`,
:doc:`grdcontour`,
:doc:`grdview`,
:doc:`grdgradient`,
:doc:`grdhisteq`
