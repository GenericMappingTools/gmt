.. index:: ! grdimage
.. include:: module_core_purpose.rst_

********
grdimage
********

|grdimage_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdimage** *grid* \| *image*
|-J|\ *parameters*
[ |-A|\ *out_img*\ [**=**\ *driver*] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ [**r**] ]
[ |-E|\ [**i**\|\ *dpi*] ]
[ |-G|\ *color*\ [**+b**\|\ **f**] ]
[ |-I|\ [*intensfile*\|\ *intensity*\|\ *modifiers*] ]
[ |-M| ]
[ |-N| ]
[ |-Q|\ [*color*][**+z**\ *value*] ]
[ |SYN_OPT-Rz| ]
[ |-T|\ [**+o**\ [*pen*]][**+s**] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

**grdimage** reads a 2-D grid file and produces a gray-shaded (or
colored) map by building a rectangular image and assigning pixels
a gray-shade (or color) based on the z-value and the CPT file.
Optionally, illumination may be added by providing a file with
intensities in the (-1,+1) range or instructions to derive intensities
from the input data grid. Values outside this range will be
clipped. Such intensity files can be created from the grid using
:doc:`grdgradient` and, optionally, modified by :doc:`grdmath` or
:doc:`grdhisteq`. Alternatively , pass *image* which can be an image
file (geo-referenced or not).
In this case the image can optionally be illuminated with the
file provided via the |-I| option. Here, if image has no coordinates
then those of the intensity file will be used.

When using map projections, the grid is first resampled on a new
rectangular grid with the same dimensions. Higher resolution images can
be obtained by using the |-E| option. To obtain the resampled value
(and hence shade or color) of each map pixel, its location is inversely
projected back onto the input grid after which a value is interpolated
between the surrounding input grid values. By default bi-cubic
interpolation is used. Aliasing is avoided by also forward projecting
the input grid nodes. If two or more nodes are projected onto the same
pixel, their average will dominate in the calculation of the pixel
value. Interpolation and aliasing is controlled with the **-n** option.

The |-R| option can be used to select a map region larger or smaller
than that implied by the extent of the grid.

Required Arguments
------------------

*grid* \| *image*
    2-D gridded data set or image to be plotted (see :ref:`Grid File Formats
    <grd_inout_full>`).

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Optional Arguments
------------------

.. _-A:

**-A**\ *out_img*\ [**=**\ *driver*]
    Save an image in a raster format instead of PostScript. Append *out_img* to select
    the image file name and extension. If the extension is one of .bmp, .gif, .jpg, .png, or .tif
    then no driver information is required. For other output formats you must append the required
    GDAL driver. The *driver* is the driver code name used by GDAL; see your GDAL installation's
    documentation for available drivers. Append a **+c**\ *options* string where *options* is a list of
    one or more concatenated number of GDAL **-co** options. For example, to write a GeoPDF with the
    TerraGo format use *=PDF+cGEO_ENCODING=OGC_BP*. Notes: (1) If a tiff file (.tif) is selected
    then we will write a GeoTiff image if the GMT projection syntax translates into a PROJ syntax,
    otherwise a plain tiff file is produced. (2) Any vector elements will be lost.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

.. include:: use_cpt_grd.rst_

.. _-D:

**-D**\ [**r**]
    GMT will automatically detect standard image files (Geotiff, TIFF,
    JPG, PNG, GIF, etc.) and will read those via GDAL.  For very
    obscure image formats you may need to explicitly set |-D|, which
    specifies that the grid is in fact an image file to be read via
    GDAL. Append **r** to assign the region specified by |-R| to the image.
    For example, if you have used **-Rd** then the image will be
    assigned a global domain. This mode allows you to project a raw image
    (an image without referencing coordinates).

.. _-E:

**-E**\ [**i**\|\ *dpi*]
    Sets the resolution of the projected grid that will be created if a
    map projection other than Linear or Mercator was selected [100]. By
    default, the projected grid will be of the same size (rows and
    columns) as the input file. Specify **i** to use the PostScript
    image operator to interpolate the image at the device resolution.

.. _-G:

**-G**\ *color*\ [**+b**\|\ **f**]
    This option only applies when a resulting 1-bit image otherwise would
    consist of only two colors: black (0) and white (255). If so, this
    option will instead use the image as a transparent mask and paint
    the mask with the given *color*.  Append **+b** to paint the background
    pixels (1) or **+f** for the foreground pixels [Default].

.. _-I:

**-I**\ [*intensfile*\|\ *intensity*\|\ *modifiers*]
    Gives the name of a grid file with intensities in the (-1,+1) range,
    or a constant intensity to apply everywhere (affects the ambient light).
    Alternatively, derive an intensity grid from the input data grid *grid*
    via a call to :doc:`grdgradient`; append **+a**\ *azimuth*, **+n**\ *args*,
    and **+m**\ *ambient* to specify azimuth, intensity, and ambient arguments
    for that module, or just give **+d** to select the
    default arguments (**+a**\ -45\ **+nt**\ 1\ **+m**\ 0). If you want a more
    specific intensity scenario then run :doc:`grdgradient` separately first.
    If we should derive intensities from another file than *grid*, specify the
    file with suitable modifiers [Default is no illumination].  **Note**: If
    the input data represent an *image* then an *intensfile* or constant *intensity*
    must be provided.

.. _-M:

**-M**
    Force conversion to monochrome image using the (old-style television) YIQ
    transformation. Cannot be used with |-Q|.

.. _-N:

**-N**
    Do not clip the image at the map boundary (only relevant for
    non-rectangular maps).

.. _-Q:

**-Q**\ [**+z**\ *value*][*color*]
    Make grid nodes with NaN values transparent, using the color-masking
    feature in PostScript Level 3 (the PS device must support PS Level 3).
    If the input is a grid, use **+z** to select another grid value than NaN.
    If input is instead an image, append an alternate color to select another
    pixel value to be transparent [Default is black].

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-Rz| replace:: You may ask for a larger
    *w/e/s/n* region to have more room between the image and the axes. A
    smaller region than specified in the grid file will result in a
    subset of the grid [Default is the region given by the grid file].
.. include:: explain_-Rz.rst_

.. _-t:

**-T**\ [**+o**\ [*pen*]][**+s**]
    Plot a data grid without any interpolation. This involves converting each
    node-centered bin into a polygon which is then painted separately.
    Append **+s** to skip nodes with z = NaN. This option is suitable for
    categorical data where interpolating between values is meaningless
    and a categorical CPT has been provided via |-C|.
    Optionally, append **+o** to draw the tile outlines, and specify a
    custom pen if the default pen is not to your liking.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_-n.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_core.rst_

.. include:: explain_help.rst_

.. module_common_ends

.. module_note_begins

Imaging Grids With Nans
-----------------------

Be aware that if your input grid contains patches of NaNs, these patches
can become larger as a consequence of the resampling that must take
place with most map projections. Because **grdimage** uses the
PostScript colorimage operator, for most non-linear projections we
must resample your grid onto an equidistant rectangular lattice. If you
find that the NaN areas are not treated adequately, consider (a) use a
linear projection, or (b) use **-T+s** instead to plot graticule polygons.

.. include:: explain_grdresample.rst_

Imaging Categorical Grids
-------------------------

Geographical categorical grids have values at the nodes that should not
be interpolated (i.e., categories 4 and 5 should never be averaged to give
a new category 4.5).  However, imaging such grids using map projections
requires a resampling onto an equidistant Cartesian lattice that usually
will result in such blending.  We do not know if a grid is categorical but
if the CPT provided via |-C| is categorical we will override any **-n** setting you
have chosen (perhaps implicitly) with **-nn+a** that turns *on* nearest neighbor
gridding and turns *off* anti-aliasing.  Alternatively, use |-T|
instead to plot individual polygons centered on each node.

Image formats recognized
------------------------

We automatically recognize image formats via their magic bytes.  For formats
that could contain either an image or a data set (e.g., geotiff) we determine
which case it is and act accordingly.  If your favorite image format is not
automatically detected then please let us know its magic bytes so we can add it.

.. include:: macos_preview_issue.rst_

.. module_note_ends

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

For a quick-and-dirty illuminated color map of the data in the remote file
@AK_gulf_grav.nc, try::

    gmt grdimage @AK_gulf_grav.nc -I+d -B -pdf quick

To gray-shade the file AK_gulf_grav.nc on a Lambert map at 1.5 cm/degree
along the standard parallels 18 and 24, centered on (142W, 55N), try::

    gmt begin alaska_gray
      gmt grd2cpt -Cgray @AK_gulf_grav.nc
      gmt grdimage @AK_gulf_grav.nc -Jl142W/55N/18/24/1.5c -B
    gmt end show

To create an illuminated color plot of the gridded data set
image.nc, using the intensities provided by the file intens.nc, and
color levels in the file colors.cpt, with linear scaling at 10
inch/x-unit, tickmarks every 5 units::

    gmt grdimage image.nc -Jx10i -Ccolors.cpt -Iintens.nc -B5 -pdf image

To create a sinusoidal projection of a remotely located Jessica Rabbit::

    gmt grdimage -JI15c -Rd http://larryfire.files.wordpress.com/2009/07/untooned_jessicarabbit.jpg -pdf jess

.. include:: cpt_notes.rst_

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`grd2kml`,
:doc:`grdcontour`,
:doc:`grdview`,
:doc:`grdgradient`,
:doc:`grdhisteq`
