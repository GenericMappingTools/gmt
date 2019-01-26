.. index:: ! psconvert

*********
psconvert
*********

.. only:: not man

    Convert [E]PS file(s) to other formats using GhostScript

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt psconvert** *psfile(s)*
[ |-A|\ *params* ]
[ |-C|\ *gs_option* ]
[ |-D|\ *outdir* ]
[ |-E|\ *resolution* ]
[ |-F|\ *out_name* ]
[ |-G|\ *ghost_path* ]
[ |-H|\ *factor*\ ]
[ |-I| ]
[ |-L|\ *listfile* ]
[ **-Mb**\ \|\ **f**\ *pslayer* ]
[ |-Q|\ [**g**\ \|\ **p**\ \|\ **t**][1\|2\|4] ]
[ |-S| ]
[ |-T|\ **b**\ \|\ **e**\ \|\ **E**\ \|\ **f**\ \|\ **F**\ \|\ **j**\ \|\ **g**\ \|\ **G**\ \|\ **m**\ \|\ **s**\ \|\ **t** ]
[ |SYN_OPT-V| ]
[ |-W|\ *params* ]
[ |-Z| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**psconvert** converts one or more PostScript files to other formats
(BMP, EPS, JPEG, PDF, PNG, PPM, SVG, TIFF) using GhostScript. Input file
names are read from the command line or from a file that lists them. The
size of the resulting images is determined by the BoundingBox (or
HiResBoundingBox, if present). As an option, a tight (HiRes)BoundingBox
may be computed first. As another option, it can compute ESRI type world
files used to reference, for instance, tif files and make them be
recognized as geotiff.  Note: If the PostScript file calls on any of
the Adobe PDF transparency extensions *and* PDF is not the selected output
format, then the file will first be converted to a temporary PDF file
(for the transparency to take effect) before converting the PDF to the
desired output format.

Required Arguments
------------------

*psfiles*
    Names of PostScript files to be converted. The output files will
    have the same name (unless **-F** is used) but with the conventional
    extension name associated to the raster format (e.g., .jpg for the
    jpeg format). Use **-D** to redirect the output to a different
    directory.

Optional Arguments
------------------

.. _-A:

**-A**\ [**+g**\ *paint*\ ][**+m**\ *margins*][**+n**][**+p**\ [\ *pen*\ ]][**+r**][**+s**\ [**m**]\ \|\ **S**\ *width*\ [**u**]/\ *height*\ [**u**]][**+u**]
    Adjust the BoundingBox and HiResBoundingBox to the minimum required
    by the image content. Append **+n** to leave the BoundingBoxes as they are
    (e.g., to override any automatic setting of **-A** by **-W**).
    Append **+u** to first remove any GMT-produced time-stamps.
    Optionally, append **+m** to specify extra margins to extend the bounding box.
    Give either one (uniform), two (x and y) or four (individual sides)
    margins; append unit [Default is set by :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>`].

    Use the **-A+s**\ *new_width* to resize the output image to exactly *new_width* units.
    The default is to use what is set by :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>`
    but you can append a new unit and/or impose different width and height. What happens
    here is that GhostScript will do the re-interpolation work and the final image will
    retain the DPI resolution set by **-E**.  Use **-A+sm** to set a maximum size and
    the new width are only imposed if the original figure width exceeds it. Append
    /\ *new_height* to also also impose a maximum height in addition to the width.
    Alternatively use **-A+S**\ *scale* to scale the image by a constant factor.

    Use the **-A+r** to round the HighRes BoundingBox instead of using the `ceil` function.
    This is going against Adobe Law but can be useful when creating very small images
    where the difference of one pixel might matter.
    If **-V** is used we also report the dimensions of the illustration.
    Use **-A+g**\ *paint* to paint the BoundingBox behind the illustration and
    use **-A+p**\ [\ *pen*] to draw the BoundingBox outline (append a pen or accept
    the default pen of 0.25p,black).

.. _-C:

**-C**\ *gs_option*
    Specify a single, custom option that will be passed on to
    GhostScript as is. Repeat to add several options [none].

.. _-D:

**-D**\ *outdir*
    Sets an alternative output directory (which must exist) [Default is
    the same directory as the PS files]. Use **-D.** to place the output
    in the current directory instead.

.. _-E:

**-E**\ *resolution*
    Set raster resolution in dpi [default = 720 for PDF, 300 for others].

.. _-F:

**-F**\ *out_name*
    Force the output file name. By default output names are constructed
    using the input names as base, which are appended with an
    appropriate extension. Use this option to provide a different name,
    but without extension. Extension is still determined automatically.

.. _-G:

**-G**\ *ghost_path*
    Full path to your GhostScript executable. NOTE: For Unix systems
    this is generally not necessary. Under Windows, the GhostScript path
    is now fetched from the registry. If this fails you can still add
    the GS path to system's path or give the full path here. (e.g.,
    **-G**\ c:\\programs\\gs\\gs9.02\\bin\\gswin64c). WARNING: because
    of the poor decision of embedding the bits on the gs exe name we
    cannot satisfy both the 32 and 64 bits GhostScript executable names.
    So in case of 'get from registry' failure the default name (when no
    **-G** is used) is the one of the 64 bits version, or gswin64c

.. _-H:

**-H**\ *factor*
    Given the finite dots-per-unit used to rasterize PostScript frames to rasters, the quantizing of features
    to discrete pixel will lead to rounding.  Some of this is mitigated by the anti-aliasing settings (**-Q**)
    which affect lines and text only.  The scale *factor* temporarily increases the effective dots-per-unit
    by *factor*, rasterizes the plot, then downsamples the image by the same factor at the end.  The larger
    the *factor*, the smoother the raster.  Because processing time increases with *factor* we suggest you
    try values in the 2-5 range.  Note that raster images can also suffer from quantizing when the original data
    have much higher resolution than your raster pixel dimensions.  The **-H** option may then be used to smooth
    the result to avoid aliasing [no downsampling].

.. _-I:

**-I**
    Enforce gray-shades by using ICC profiles.  GhostScript versions
    >= 9.00 change gray-shades by using ICC profiles.  GhostScript 9.05
    and above provide the '-dUseFastColor=true' option to prevent that
    and that is what **psconvert** does by default, unless option **-I** is
    set.  Note that for GhostScript >= 9.00 and < 9.05 the gray-shade
    shifting is applied to all but PDF format.  We have no solution to
    offer other than upgrade GhostScript.

.. _-L:

**-L**\ *listfile*
    The *listfile* is an ASCII file with the names of the PostScript
    files to be converted.

.. _-M:

**-Mb**\ \|\ **f**\ *pslayer*
    Sandwich the current *psfile* between an optional background (**-Mb**) and
    optional foreground (**-Mf**) Postscript plots.  These files are expected
    to be stand-alone plots that will align when stacked.

.. _-Q:

**-Q**\ [**g**\ \|\ **p**\ \|\ **t**][1\|2\|4]
    Set the anti-aliasing options for **g**\ raphics or **t**\ ext.
    Append the size of the subsample box (1, 2, or 4) [4]. For vector
    formats the default is no anti-aliasing (same as *bits* = 1).
    For any raster format the default setting is **-Qt4**, while transparent
    PNG also adds **-Qg2**.  These defaults may be overruled manually.
    Optionally, select **-Qp** to turn on generation of Geo PDFs (requires **-Tf** as well).

.. _-S:

**-S**
    Print to standard error the GhostScript command after it has been executed.
    This option also prevent all intermediate files from being removed.

.. _-T:

**-Tb**\ \|\ **e**\ \|\ **E**\ \|\ **f**\ \|\ **F**\ \|\ **j**\ \|\ **g**\ \|\ **G**\ \|\ **m**\ \|\ **s**\ \|\ **t**
    Sets the output format, where **b** means BMP, **e** means EPS,
    **E** means EPS with PageSize command, **f** means PDF, **F** means
    multi-page PDF, **j** means JPEG, **g** means PNG, **G** means
    transparent PNG (untouched regions are transparent), **m** means
    PPM, **s** means SVG, and **t** means TIFF [default is JPEG]. To **bjgt** you can
    append **-** in order to get a grayscale image. The EPS format can be
    combined with any of the other formats. For example, **-Tef**
    creates both an EPS and a PDF file. The **-TF** creates a multi-page
    PDF file from the list of input PS or PDF files. It requires the **-F** option.
    See also **NOTES** below.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**+g**][**+k**][**+t**\ *docname*][**+n**\ *layername*][**+o**\ *foldername*][**+a**\ *altmode*\ [*alt*]][**+l**\ *minLOD/maxLOD*][**+f**\ *minfade/maxfade*][**+u**\ *URL*]
    Write a ESRI type world file suitable to make (e.g) .tif files be
    recognized as geotiff by software that know how to do it. Be aware,
    however, that different results are obtained depending on the image
    contents and if the **-B** option has been used or not. The trouble
    with the **-B** option is that it creates a frame and very likely
    its annotations. That introduces pixels outside the map data extent,
    and therefore the map extents estimation will be wrong. To avoid
    this problem use *-*\ *-*\ MAP_FRAME_TYPE=inside option which plots all
    annotations and ticks inside the image and therefore does not
    compromise the coordinate computations. Pay attention also to the
    cases when the plot has any of the sides with whites only because
    than the algorithm will fail miserably as those whites will be eaten
    by the GhostScript. In that case you really must use **-B** or use a
    slightly off-white color.

    Together with **-V** it prints on screen the gdal_translate
    (gdal_translate is a command line tool from the GDAL package)
    command that reads the raster + world file and creates a true
    geotiff file. Use **-W+g** to do a system call to gdal_translate
    and create a geoTIFF image right away. The output file will have a
    .tiff extension.

    The world file naming follows the convention of jamming a 'w' in the
    file extension. So, if output is tif **-Tt** the world file is a
    .tfw, for jpeg we have a .jgw and so on. This option automatically
    sets **-A** **-P**.

    Use **-W+k** to create a minimalist KML file that allows loading the
    image in GoogleEarth. Note that for this option the image must be in
    geographical coordinates. If not, a warning is issued but the KML
    file is created anyway. Several modifier options are available to
    customize the KML file in the form of **+**\ *opt* strings. Append
    **+t**\ *title* to set the document title [GMT KML Document],
    **+n**\ *layername* to set the layer name, and
    **+a**\ */altmode*\ [*altitude*] to select one of 5 altitude modes
    recognized by Google Earth that determines the altitude (in m) of
    the image: **G** clamped to the ground, **g** append altitude
    relative to ground, **a** append absolute altitude, **s** append
    altitude relative to seafloor, and **S** clamp it to the seafloor.
    Control visibility of the layer with the **+l**\ *minLOD/maxLOD* and
    **+f**\ *minfade/maxfade* options. Finally, if you plan to leave the
    image itself on a server and only distribute the KML, use
    **+u**\ *URL* to prepend the URL to the image reference. If you are
    building a multi-component KML file then you can issue a KML snipped
    without the KML header and trailer by using the **+o**\ *foldername*
    modification; it will enclose the image and associated KML code
    within a KML folder of the specified name. See the KML documentation
    for further explanation
    (http://code.google.com/apis/kml/documentation/).
    Note: If any of your titles or names contain a plus symbol next to
    a letter it can be confused with an option modifier. Escape such
    plus signs by placing a backslash in front of it.  Alternatively,
    enclose the string in double quotes and then the entire **-W**
    argument in single-quotes (or vice versa).

    Further notes on the creation of georeferenced rasters.
    **psconvert** can create a georeferenced raster image with a world
    file OR uses GDAL to convert the GMT PostScript file to geotiff.
    GDAL uses Proj.4 for its projection library. To provide with the
    information it needs to do the georeferencing, GMT 4.5 embeds a
    comment near the start of the PostScript file defining the
    projection using Proj.4 syntax. Users with pre-GMT v4.5 PostScript
    files, or even non-GMT ps files, can provide the information
    **psconvert** requires by manually editing a line into the
    PostScript file, prefixed with %%PROJ.

    For example the command

   ::

        gmt coast -JM0/12c -R-10/-4/37/43 -W1 -Di -Bg30m --MAP_FRAME_TYPE=inside > cara.ps

    adds this comment line

   ::

        %%PROJ: merc -10.0 -4.0 37.0 43.0 -1113194.908 -445277.963
        4413389.889 5282821.824 +proj=merc +lon_0=0 +k=-1 +x_0=0 +y_0=0
        +a=6378137.0 +b=6356752.314245 +ellps=WGS84 +datum=WGS84 +units=m +no_defs

    where 'merc' is the keyword for the coordinate conversion; the 2 to
    5th elements contain the map limits, 6 to 9th the map limits in
    projected coordinates and the rest of the line has the regular proj4
    string for this projection.

**-Z**
    Remove the input PostScript file(s) after the conversion.
    The input file(s) will **not** be removed in case of failures.

.. include:: explain_help.rst_

Notes
-----

The conversion to raster images (BMP, JPEG, PNG, PPM or TIFF) inherently
results in loss of details that are available in the original
PostScript file. Choose a resolution that is large enough for the
application that the image will be used for. For web pages, smaller dpi
values suffice, for Word documents and PowerPoint presentations a higher
dpi value is recommended. **psconvert** uses the loss-less DEFLATE
compression technique when creating PDF and PNG files and LZW compression
for TIFF images.  For smaller dpi images, such as required for building
animations, the use of **-Qt**\ 4 and **-Qg**\ 4 may help sharpen text and lines,
as will the **-H** option.

EPS is a vector (not a raster) format. Therefore, the **-E** option has
no effect on the creation of EPS files. Using the option **-Te** will
remove setpagedevice commands from the PostScript file and will adjust the
BoundingBox when the **-A** option is used. Note the original and
required BoundingBox is limited to integer points, hence Adobe added the
optional HiResBoundingBox to add more precision in sizing. The **-A**
option calculates both and writes both to the EPS file and is subsequently
used in any rasterization, if requested. When the **-TE** option is used, a
new setpagedevice command is added that will indicate the actual pagesize for
the plot, similar to the BoundingBox. Note that when the command setpagedevice
exists in a PostScript file that is included in another document, this can wreak
havoc on the printing or viewing of the overall document. Hence, **-TE** should only
be used for "standalone" PostScript files.

Although PDF and SVG are also vector formats, the **-E** option has an effect on
the resolution of pattern fills and fonts that are stored as bitmaps in
the document. **psconvert** therefore uses a larger default resolution
when creating PDF and SVG files. **-E** also determines the resolution of the
boundingbox values used to indicate the size of the output PDF.
In order to obtain high-quality PDF or SVG files, the
*/prepress* options are in effect, allowing only loss-less DEFLATE
compression of raster images embedded in the PostScript file.

Although **psconvert** was developed as part of the GMT, it can be
used to convert PostScript files created by nearly any graphics
program. However, **-A+u** is GMT-specific.

The **ghostscript** program continues to be developed and occasionally its
developers make decisions that affect **psconvert**.  As of version 9.16 the
SVG device has been removed.  Fortunately, quality SVG graphics can be obtained
by first converting to PDF and then install and use the package **pdf2svg**.

See :ref:`include-gmt-graphics` of the **GMT Technical Reference and Cookbook** for more
information on how **psconvert** is used to produce graphics that can be
inserted into other documents (articles, presentations, posters, etc.).

The conversion to Geo PDFs have proven unstable and could create PDF files that could
not be opened.  We have therefore made this an optional setting that now requires
the **-Qp** option to activate, since most users are unaware of GeoPDFs anyway.

Examples
--------

To convert the file psfile.ps to PNG using a tight BoundingBox:

   ::

    gmt psconvert psfile.ps -A -Tg

To convert the file map.ps to PDF, extend the BoundingBox by 0.2 cm,
fill it with lightblue paint and draw outline with a thick pen:

   ::

    gmt psconvert map.ps -A+m0.2c+glightblue+pthick -Tf

To create a 5 cm PNG version at 300 dpi of our example_01.ps file

   ::

    gmt psconvert example_01.ps -A+s5c -Tg

To create a 3 pages PDF file from 3 individual PS files

   ::

    gmt psconvert -TF -Fabc a.ps b.ps c.ps

To create a simple linear map with :doc:`coast` and convert it to tif with a
.tfw the tight BoundingBox computation.

   ::

    gmt coast -JX12cd -R-10/-4/37/43 -W1 -Di -Bg30m -G200 --MAP_FRAME_TYPE=inside > cara.ps
    gmt psconvert cara.ps -Tt -W

To create a Mercator version of the above example and use GDAL to
produce a true geotiff file.

   ::

    gmt coast -JM0/12c -R-10/-4/37/43 -W1 -Di -Bg30m -G200 --MAP_FRAME_TYPE=inside > cara.ps
    gdalwarp -s_srs +proj=merc cara.tif carageo.tiff

To create a Polar Stereographic geotiff file of Patagonia

   ::

    gmt coast -JS-55/-60/15c -R-77/-55/-57.5/-48r -Di -Gred -Bg2 --MAP_FRAME_TYPE=inside > patagonia.ps
    gmt psconvert patagonia.ps -Tt -W+g -V

To create a simple KML file for use in Google Earth, try

   ::

    gmt grdimage lonlatgrid.nc -Jx1 -Ccolors.cpt -B0g2 --MAP_FRAME_TYPE=inside > tile.ps
    gmt psconvert tile.ps -Tg -W+k+t"my title"+l256/-1 -V

(These commands assume that GhostScript can be found in your system's path.)

GhostScript Options
-------------------

Most of the conversions done in **psconvert** are handled by
GhostScript. On most Unixes this program is available as **gs**; for
Windows there is a version called **gswin32c**. GhostScript accepts a
rich selection of command-line options that modify its behavior. Many of
these are set indirectly by the options available above. However,
hard-core usage may require some users to add additional options to
fine-tune the result. Use **-S** to examine the actual command used, and
add custom options via one or more instances of the **-C** option. For
instance, to turn on image interpolation for all images, improving image
quality for scaled images at the expense of speed, use
**-C**-dDOINTERPOLATE. See www.ghostscript.com for complete
documentation.

Making KMZ files
----------------

If you have made a series of KML files (which may depend on other items
like local PNG images), you can consolidate these into a single KMZ file
for saving space and for grouping related files together.  The bash function
**gmt_build_kmz** in the :doc:`gmt_shell_functions.sh` can be used to
do this.  You need to source gmt_shell_functions.sh first before you can
use it. 

See Also
--------

:doc:`gmt`,
:doc:`coast`

