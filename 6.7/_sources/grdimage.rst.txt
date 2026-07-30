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
[ |-A|\ *out_img*\ [**=**\ *driver*] ]
[ |SYN_OPT-B| ]
[ |-C|\ [*section*/]\ *master*\|\ *cpt*\|\ *color*\ :math:`_1`,\ *color*\ :math:`_2`\ [,\ *color*\ :math:`_3`\ ,...]\ [**+h**\ [*hinge*]][**+i**\ *dz*][**+u**\|\ **U**\ *unit*][**+s**\ *fname*] ]
[ |-D|\ [**r**] ]
[ |-E|\ [**i**\|\ *dpi*] ]
[ |-G|\ *color*\ [**+b**\|\ **f**] ]
[ |-I|\ [*file*\|\ *intens*\|\ **+a**\ *azimuth*][**+d**][**+m**\ *ambient*][**+n**\ *args*] ]
[ |-J|\ *parameters* ]
[ |-M| ]
[ |-N| ]
[ |-Q|\ [*color*][**+i**][**+t**][**+z**\ *value*] ]
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
intensities in the ±1 range or instructions to derive intensities
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
than that implied by the extent of the grid. Finally, |-A| allows the
creation of a direct output to a raster file instead of plotting via
PostScript.

Required Arguments
------------------

*grid* \| *image*
    2-D gridded data set or image to be plotted (see :ref:`Grid File Formats
    <grd_inout_full>`).

Optional Arguments
------------------

.. _-A:

**-A**\ *out_img*\ [**=**\ *driver*]
    Save an image in a raster format instead of PostScript. Append *out_img* to select
    the image file name and extension. If the extension is one of .bmp, .gif, .jp[e]g, .png, or .tif
    then no driver information is required. For other output formats you must append the required
    GDAL driver. The *driver* is the driver code name used by GDAL; see your GDAL installation's
    documentation for available drivers. Append a **+c**\ *options* string where *options* is a list of
    one or more concatenated number of GDAL **-co** options. For example, to write a GeoPDF with the
    TerraGo format use *=PDF+cGEO_ENCODING=OGC_BP*. Notes: (1) If a tiff file (.tif) is selected
    then we will write a GeoTIFF image if the GMT projection syntax translates into a PROJ syntax,
    otherwise a plain tiff file is produced. (2) Any vector elements will be lost. **Note**: The **-B**
    option is not compatible with |-A| since no PostScript output is allowed.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

.. include:: use_cpt_grd.rst_

.. _-D:

**-D**\ [**r**]
    GMT will automatically detect standard image files (GeoTIFF, TIFF,
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

.. include:: explain_intense.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-M:

**-M**
    Force conversion to monochrome image using the (old-style television) YIQ
    transformation. Cannot be used with |-Q|.

.. _-N:

**-N**
    Do not clip the image at the map boundary (only relevant for
    non-rectangular maps).

.. _-Q:

**-Q**\ [*color*][**+i**][**+t**][**+z**\ *value*]
    Handle transparency or opacity for grids or images. There are four general schemes:

    - Grid - Plain |-Q| will turn grid nodes with NaN values transparent in the image, using
      the color-masking feature in PostScript Level 3 (the PS device must support PS Level 3).
      Use modifier **+z**\ *value* to specify another grid value than NaN. Each pixel is now
      either opaque color or fully transparent.
    - RGB image - Append a *color* to identify pixels that should be turned transparent
      [Default is white]. Each pixel is then either opaque or transparent in the output image.
    - RGBA image with two *A* values (0, 255) - True transparent image requires an alpha
      channel that is either 0 or 255. Default turns any pixel with alpha = 0 transparent.
    - RGBA image with variable transparency - If we have an alpha channel with variable
      transparency between 0 and 255 on a per pixel basis then the *PostScript* image operator
      cannot create true variable pixel transparency *t*. Instead, each *r*, *g*, and *b* pixel
      values are converted by :math:`r' = t R + (1-t) r`, where *R* (and *G*, *B*) is the
      transparent color at full transparency [Default is white]. If *color* is given then it
      becomes the *R*, *B*, *G*  at full transparency. This (opaque) transformation is
      actually done by default (_i.e._, no need to set **-Q**). If, however, a true transparency
      is wished, then use the **+i** modifier. Such RGBA images will be approximated by *n_columns*
      times *n_rows* of tiny squares with variable color and transparency. Given this schema, the
      PositScript created file is much larger than if **+t** is not used and that is why this is
      not the default behavior. If *A* reflects opacity instead of transparency then you can use modifier
      **+i** to invert these numbers first. See `Limitations on transparency`_ for more discussion.
      **Note**: The **+i** modifier is not available for grids.

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

.. include:: explain_distunits.rst_

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

Imaging Categorical Images
--------------------------

If a 1-byte single layer image is given and the file has no color map then we will
interpret the byte values as categories and a categorical CPT is required via |-C|.
If no |-C| is given then we assume the image is a grayscale image with values in the
0-255 range.

Limitations on transparency
---------------------------

The PostScript imaging model does not support any form of transparency.  However, Adobe added
`pdfMark <https://opensource.adobe.com/dc-acrobat-sdk-docs/library/pdfmark/index.html>`_
which allows PostScript to specify transparency but only if activated when converting PostScript
or EPS to PDF with Adobe Distiller or GhostScript. Each graphic (e.g., polygon, line, text, image)
can have a specified transparency. Yet, for images this is very limited: We can choose a particular
characteristic of the image to mean transparency, e.g, a specific *r*\ /*g*\ /*b* color or an
*alpha* channel level (0-255). Thus, variable pixel-by-pixel transparency in a sophisticated RGBA
image (color + transparency) cannot be see-through for more than a single color.  Our
approximation for plotting transparent RGBA images is to simulate the transparency effect
on the color, but the image remains opaque (optionally apart from a single color via |-Q|).
Since polygons can have separate transparencies then we may simulate the image by squares symbols
that can have individualized color *and* transparency via (up to) 255 values in the alpha channel.

Image formats recognized
------------------------

We automatically recognize image formats via their magic bytes.  For formats
that could contain either an image or a data set (e.g., GeoTIFF) we determine
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
