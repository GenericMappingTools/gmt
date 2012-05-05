********
grdimage
********


grdimage - Project grids or images and plot them on maps

`Synopsis <#toc1>`_
-------------------

**grdimage** *grd\_z* \| *grd\_r grd\_g grd\_b* [
**-A**\ *out\_img*\ **=**\ *driver* ] **-C**\ *cptfile* [
**-D**\ [**r**\ ] ] **-J**\ *parameters* [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-Ei**\ [\|*dpi*] ] [
**-G**\ [**f**\ \|\ **b**]\ *color* ] [ **-I**\ *intensfile* ] [
**-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [ **-M** ] [ **-N** ] [
**-O** ] [ **-P** ] [ **-Q** ] [
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] ] [
**-T** ] [ **-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [
**-V**\ [*level*\ ] ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-c**\ *copies* ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-tr** ]

`Description <#toc2>`_
----------------------

**grdimage** reads one 2-D grid file and produces a gray-shaded (or
colored) map by plotting rectangles centered on each grid node and
assigning them a gray-shade (or color) based on the z-value.
Alternatively, **grdimage** reads three 2-D grid files with the red,
green, and blue components directly (all must be in the 0-255 range).
Optionally, illumination may be added by providing a file with
intensities in the (-1,+1) range. Values outside this range will be
clipped. Such intensity files can be created from the grid using
**grdgradient** and, optionally, modified by **grdmath** or
**grdhisteq**. Yet as a third alternative available when GMT is build
with GDAL support the grd\_z file can be an image referenced or not
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
A (color) *PostScript* file is output.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*grd\_z* \| *grd\_r grd\_g grd\_b*
    2-D gridded data set (or red, green, blue grids) to be imaged (See
    GRID FILE FORMATS below.)
**-C**\ *cptfile*
    name of the color palette table (for *grd\_z* only).
**-J**\ *parameters* (\*)
    Select map projection.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ *out\_img*\ **=**\ *driver*
    With GDAL aware versions: save image in a raster format instead of
    *PostScript*. Append *out\_img*\ **=**\ *driver* to select the file
    name and image format. The *driver* is the driver code name used by
    GDAL. For example, **-A**\ img.tif=GTiff will write a GeoTiff image
    if the subset of GMT syntax projections that is currently possible
    to translate into the PROJ4 syntax allows it, or a plain tiff file
    otherwise. Note: any vector elements are lost.
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-D**\ [**r**\ ]
    Specifies that the grid supplied is an image file to be read via
    GDAL. Obviously this option will work only with **GMT** versions
    built with GDAL support. The image can be indexed or true color
    (RGB) and can be an URL of a remotely located file. That is **-D**
    `http://www.somewhere.com/image.jpg <http://www.somewhere.com/image.jpg>`_
    is a valid file syntax. Note, however, that to use it this way you
    must not be blocked by a proxy. If you are, chances are good that it
    can work by setting the environmental variable
    `http\_proxy <http_proxy>`_ with the value ’your\_proxy:port’ Append
    **r** to use the region specified by **-R** to apply to the image.
    For example, if you have used **-Rd** then the image will be
    assigned the limits of a global domain. The interest of this mode is
    that you can project a raw image (an image without referencing
    coordinates).
**-Ei**\ [\|*dpi*]
    Sets the resolution of the projected grid that will be created if a
    map projection other than Linear or Mercator was selected [100]. By
    default, the projected grid will be of the same size (rows and
    columns) as the input file. Specify **i** to use the *PostScript*
    image operator to interpolate the image at the device resolution.
**-G**\ [**f**\ \|\ **b**]\ *color*
    This option only applies when the resulting image otherwise would
    consist of only two colors: black (0) and white (255). If so, this
    option will instead use the image as a transparent mask and paint
    the mask (or its inverse, with **-Gb**) with the given color
    combination.
**-I**\ *intensfile*
    Gives the name of a grid file with intensities in the (-1,+1) range.
    [Default is no illumination].
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-M**
    Force conversion to monochrome image using the (television) YIQ
    transformation. Cannot be used with **-Q**.
**-N**
    Do not clip the image at the map boundary (only relevant for
    non-rectangular maps).
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**
    Make grid nodes with z = NaN transparent, using the colormasking
    feature in *PostScript* Level 3 (the PS device must support PS Level
    3).
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.
    You may ask for a larger *w/e/s/n* region to have more room between
    the image and the axes. A smaller region than specified in the grid
    file will result in a subset of the grid [Default is the region
    given by the grid file].
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]] (\*)
    Shift plot origin.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*] (\*)
    Select interpolation mode for grids.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*] (\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Grid File Formats <#toc6>`_
----------------------------

**GMT** is able to recognize many of the commonly used grid file
formats, as well as the precision, scale and offset of the values
contained in the grid file. When **GMT** needs a little help with that,
you can add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. See `**grdreformat**\ (1) <grdreformat.1.html>`_ and Section 4.17
of the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. See `**grdreformat**\ (1) <grdreformat.1.html>`_ and
Section 4.18 of the GMT Technical Reference and Cookbook for more
information, particularly on how to read splices of 3-, 4-, or
5-dimensional grids.

`Imaging Grids With Nans <#toc7>`_
----------------------------------

Be aware that if your input grid contains patches of NaNs, these patches
can become larger as a consequence of the resampling that must take
place with most map projections. Because **grdimage** uses the
*PostScript* colorimage operator, for most non-linear projections we
must resample your grid onto an equidistant rectangular lattice. If you
find that the NaN areas are not treated adequately, consider (a) use a
linear projection, or (b) use **grdview** **-Ts** instead.

`Examples <#toc8>`_
-------------------

To gray-shade the file hawaii\_grav.nc with shades given in shades.cpt
on a Lambert map at 1.5 cm/degree along the standard parallels 18 and
24, and using 1 degree tickmarks:

grdimage hawaii\_grav.nc **-Jl**\ 18/24/1.5\ **c** -Cshades.cpt -B1 >
hawaii\_grav\_image.ps

To create an illuminated color *PostScript* plot of the gridded data set
image.nc, using the intensities provided by the file intens.nc, and
color levels in the file colors.cpt, with linear scaling at 10
inch/x-unit, tickmarks every 5 units:

grdimage image.nc **-Jx**\ 10\ **i** -Ccolors.cpt -Iintens.nc -B5 >
image.ps

To create an false color *PostScript* plot from the three grid files
red.nc, green.nc, and blue.nc, with linear scaling at 10 inch/x-unit,
tickmarks every 5 units:

grdimage red.nc green.nc blue.nc **-Jx**\ 10\ **i** -B5 > rgbimage.ps

When GDAL support is built in: To create a sinusoidal projection of a
remotely located Jessica Rabbit

grdimage -JI15c -Rd -Dr
`http://larryfire.files.wordpress.com/2009/07/untooned\_jessicarabbit.jpg <http://larryfire.files.wordpress.com/2009/07/untooned_jessicarabbit.jpg>`_
-P > jess.ps

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*grd2rgb*\ (1) <grd2rgb.1.html>`_ ,
`*grdcontour*\ (1) <grdcontour.1.html>`_ ,
`*grdview*\ (1) <grdview.1.html>`_ ,
`*grdgradient*\ (1) <grdgradient.1.html>`_ ,
`*grdhisteq*\ (1) <grdhisteq.1.html>`_

