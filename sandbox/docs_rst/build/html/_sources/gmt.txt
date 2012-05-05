***
gmt
***


gmt - The Generic Mapping Tools data processing and display software
package

`Introduction <#toc1>`_
-----------------------

**GMT** is a collection of public-domain Unix tools that allows you to
manipulate x,y and x,y,z data sets (filtering, trend fitting, gridding,
projecting, etc.) and produce *PostScript* illustrations ranging from
simple x-y plots, via contour maps, to artificially illuminated surfaces
and 3-D perspective views in black/white or full color. Linear, log10,
and power scaling is supported in addition to over 30 common map
projections. The processing and display routines within **GMT** are
completely general and will handle any (x,y) or (x,y,z) data as input.

`Synopsis <#toc2>`_
-------------------

**gmt** is a wrapper program that can start any of the programs:

**gmt** module module-options

where module is the name of a **GMT** program and the options are those
that pertain to that particular program.

`Gmt Overview <#toc3>`_
-----------------------

The following is a summary of all the programs supplied with **GMT** and
a very short description of their purpose. Detailed information about
each program can be found in the separate manual pages.

**blockmean**\ `` `` `` `` `` `` `` `` L2 (x,y,z) data filter/decimator
 **blockmedian**\ `` `` `` `` L1 (x,y,z) data filter/decimator
 **blockmode**\ `` `` `` `` `` `` `` `` Mode (x,y,z) data filter/decimator
 **filter1d**\ `` `` `` `` `` `` `` `` Filter 1-D data sets (time series)
 **fitcircle**\ `` `` `` `` `` `` `` `` Finds the best-fitting great circle to a set of points
 **gmt2kml**\ `` `` `` `` `` `` `` `` Convert GMT data tables to KML files for Google Earth
 **grd2rgb**\ `` `` `` `` `` `` `` `` Convert Sun rasterfile or grid to r, g, b grids
 **gmtconvert**\ `` `` `` `` Convert between ASCII and binary 1-D tables
 **gmtdp**\ `` `` `` `` `` `` `` `` Line reduction using the Douglas-Peucker algorithm
 **gmtdefaults**\ `` `` `` `` List the current default settings
 **gmtget**\ `` `` `` `` `` `` `` `` Get individual default parameters
 **gmtmath**\ `` `` `` `` `` `` `` `` Mathematical operations on data tables
 **gmtset**\ `` `` `` `` `` `` `` `` Set individual default parameters
 **gmtselect**\ `` `` `` `` `` `` `` `` Extract data subsets based on spatial criteria
 **gmtspatial**\ `` `` `` `` Geospatial operations on lines and polygons
 **gmtstitch**\ `` `` `` `` `` `` `` `` Join individual lines whose end points match within tolerance
 **gmtvector**\ `` `` `` `` `` `` `` `` Basic vector manipulation in 2-D and 3-D
 **gmtwhich**\ `` `` `` `` `` `` `` `` Find full path to specified files
 **grdfilter**\ `` `` `` `` `` `` `` `` Filter 2-D data sets in the space domain
 **grd2cpt**\ `` `` `` `` `` `` `` `` Make a color palette table from grid files
 **grd2xyz**\ `` `` `` `` `` `` `` `` Conversion from 2-D grid file to table data
 **grdblend**\ `` `` `` `` `` `` `` `` Blend several partially over-lapping grid files onto one grid
 **grdclip**\ `` `` `` `` `` `` `` `` Limit the z-range in gridded data
 **grdcontour**\ `` `` `` `` Contouring of 2-D gridded data
 **grdcut**\ `` `` `` `` `` `` `` `` Cut a sub-region from a grid file
 **grdedit**\ `` `` `` `` `` `` `` `` Modify header information in a 2-D grid file
 **grdfft**\ `` `` `` `` `` `` `` `` Operate on grid files in the wavenumber (or frequency) domain
 **grdgradient**\ `` `` `` `` Compute directional gradient from grid files
 **grdhisteq**\ `` `` `` `` `` `` `` `` Histogram equalization for grid files
 **grdimage**\ `` `` `` `` `` `` `` `` Produce images from 2-D gridded data
 **grdinfo**\ `` `` `` `` `` `` `` `` Get information about grid files
 **grdlandmask**\ `` `` `` `` Create mask grid file from shoreline data base
 **grdmask**\ `` `` `` `` `` `` `` `` Reset nodes outside a clip path to a constant
 **grdmath**\ `` `` `` `` `` `` `` `` Mathematical operations on grid files
 **grdpaste**\ `` `` `` `` `` `` `` `` Paste together grid files along a common edge
 **grdproject**\ `` `` `` `` Project gridded data onto a new coordinate system
 **grdreformat**\ `` `` `` `` Converting between different grid file formats
 **grdsample**\ `` `` `` `` `` `` `` `` Resample a 2-D gridded data set onto a new grid
 **grdtrend**\ `` `` `` `` `` `` `` `` Fits polynomial trends to grid files
 **grdtrack**\ `` `` `` `` `` `` `` `` Sampling of 2-D data set along 1-D track
 **grdvector**\ `` `` `` `` `` `` `` `` Plot vector fields from grid files
 **grdview**\ `` `` `` `` `` `` `` `` 3-D perspective imaging of 2-D gridded data
 **grdvolume**\ `` `` `` `` `` `` `` `` Volume calculations from 2-D gridded data
 **greenspline**\ `` `` `` `` Interpolation using Green’s functions for splines in 1-3 dimensions
 **kml2gmt**\ `` `` `` `` `` `` `` `` Extract GMT data from a Google Earth KML file
 **makecpt**\ `` `` `` `` `` `` `` `` Make color palette tables
 **mapproject**\ `` `` `` `` Forward or inverse map projections of table data
 **minmax**\ `` `` `` `` `` `` `` `` Find extreme values in data tables
 **nearneighbor**\ `` `` `` `` Nearest-neighbor gridding scheme
 **project**\ `` `` `` `` `` `` `` `` Project data onto lines/great circles
 **ps2raster**\ `` `` `` `` `` `` `` `` Crop and convert *PostScript* files to raster images, EPS, and PDF
 **psbasemap**\ `` `` `` `` `` `` `` `` Create a basemap plot
 **psclip**\ `` `` `` `` `` `` `` `` Use polygon files to define clipping paths
 **pscoast**\ `` `` `` `` `` `` `` `` Plot coastlines and filled continents on maps
 **pscontour**\ `` `` `` `` `` `` `` `` Contour xyz-data by triangulation
 **pshistogram**\ `` `` `` `` Plot a histogram
 **psimage**\ `` `` `` `` `` `` `` `` Plot images (EPS or Sun raster files) on maps
 **pslegend**\ `` `` `` `` `` `` `` `` Plot legend on maps
 **psmask**\ `` `` `` `` `` `` `` `` Create overlay to mask out regions on maps
 **psrose**\ `` `` `` `` `` `` `` `` Plot sector or rose diagrams
 **psscale**\ `` `` `` `` `` `` `` `` Plot gray scale or color scale on maps
 **pstext**\ `` `` `` `` `` `` `` `` Plot text strings on maps
 **pswiggle**\ `` `` `` `` `` `` `` `` Draw time-series along track on maps
 **psxy**\ `` `` `` `` `` `` `` `` `` `` `` `` Plot symbols, polygons, and lines on maps
 **psxyz**\ `` `` `` `` `` `` `` `` Plot symbols, polygons, and lines in 3-D
 **sample1d**\ `` `` `` `` `` `` `` `` Resampling of 1-D table data sets
 **spectrum1d**\ `` `` `` `` Compute various spectral estimates from time-series
 **splitxyz**\ `` `` `` `` `` `` `` `` Split xyz-files into several segments
 **surface**\ `` `` `` `` `` `` `` `` A continuous curvature gridding algorithm
 **trend1d**\ `` `` `` `` `` `` `` `` Fits polynomial or Fourier trends to y = f(x) data
 **trend2d**\ `` `` `` `` `` `` `` `` Fits polynomial trends to z = f(x,y) data
 **triangulate**\ `` `` `` `` Perform optimal Delaunay triangulation and gridding
 **xyz2grd**\ `` `` `` `` `` `` `` `` Convert equidistant xyz data to a 2-D grid file

`The Common Gmt Options <#toc4>`_
---------------------------------

**-B**\ [**p**\ \|\ **s**]\ *parameters* **-J**\ *parameters*
**-Jz**\ \|\ **Z**\ *parameters* **-K** **-O** **-P**
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] **-V**\ [*level*\ ]
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
**-a**\ *col*\ =\ *name*\ [*...*\ ]
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] **-c**\ *copies*
**-f**\ [**i**\ \|\ **o**]\ *colinfo*
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
**-h**\ [**i**\ \|\ **o**][*n*\ ]
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
**-o**\ *cols*\ [,*...*]
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
**-r** **-s**\ [*cols*\ ][\ **a**\ \|\ **r**] **-t**\ [*transp*\ ]
**-:**\ [**i**\ \|\ **o**]

`Description <#toc5>`_
----------------------

These are all the common GMT options that remain the same for all GMT
programs. No space between the option flag and the associated arguments.

**-B**\ [**p**\ \|\ **s**]\ *parameters*

Set map boundary annotation and tickmark intervals. The format of
*tickinfo* is
[**p**\ \|\ **s**]\ *xinfo*\ [/*yinfo*\ [/*zinfo*]][\ **:.**"Title"**:**][\ **W**\ \|\ **w**][\ **E**\ \|\ **e**][\ **S**\ \|\ **s**][\ **N**\ \|\ **n**][\ **Z**\ \|\ **z**\ [**+**\ ]][\ **+g**\ *fill*].
The leading **p** [Default] or **s** selects the primary or secondary
annotation information. Each of the *?info* segments are textstrings of
the form *info*\ [**:**"Axis
label"**:**][\ **:=**"prefix"**:**][\ **:,**"unit label"**:**]. The
*info* string is made up of one or more concatenated substrings of the
form [**a**\ \|\ **f**\ \|\ **g**][\ *stride*\ [+-*phase*][*unit*\ ]].
The leading **a** is used to specify the annotation and major tick
spacing [Default], **f** for minor tick spacing, and **g** for gridline
spacing. *stride* is the desired stride interval. The optional *phase*
shifts the annotation interval by that amount (positive or negative).
The optional *unit* indicates the unit of the *stride* and can be any of
**Y** (year, plot with 4 digits), **y** (year, plot with 2 digits),
**O** (month, plot using **FORMAT\_DATE\_MAP**), **o** (month, plot with
2 digits), **U** (ISO week, plot using **FORMAT\_DATE\_MAP**), **u**
(ISO week, plot using 2 digits), **r** (Gregorian week, 7-day stride
from start of week **TIME\_WEEK\_START**), **K** (ISO weekday, plot name
of day), **D** (date, plot using **FORMAT\_DATE\_MAP**), **d** (day,
plot day of month 0-31 or year 1-366, via **FORMAT\_DATE\_MAP**), **R**
(day, same as **d**, aligned with **TIME\_WEEK\_START**), **H** (hour,
plot using **FORMAT\_CLOCK\_MAP**), **h** (hour, plot with 2 digits),
**M** (minute, plot using **FORMAT\_CLOCK\_MAP**), **m** (minute, plot
with 2 digits), **S** (second, plot using **FORMAT\_CLOCK\_MAP**), **s**
(second, plot with 2 digits). Note for geographic axes **m** and **s**
instead mean arc minutes and arc seconds. All entities that are
language-specific are under control by **TIME\_LANGUAGE**.
Alternatively, for linear maps, we can omit *stride*, thus setting
*xinfo*, *yinfo*, or *zinfo* to **a** plots annotations at automatically
determined intervals, **ag** plots both annotations and grid lines with
the same spacing, **afg** adds suitable minor tick intervals, **g**
plots grid lines with the same interval as if **-Bf** was used.
For custom annotations and intervals, let *xinfo*, *yinfo*, or *zinfo*
be **c**\ *intfile*, where *intfile* contains any number of records with
*coord* *type* [*label*\ ]. Here, *type* is one or more letters from
**a**\ \|\ **i**, **f**, and **g**. For **a**\ \|\ **i** you must supply
a *label* that will be plotted at the *coord* location. To specify
separate x and y ticks, separate the substrings that apply to the x and
y axes with a slash [/] (If a 3-D basemap is selected with **-p** and
**-Jz**, a third substring pertaining to the vertical axis may be
appended.) For linear/log/power projections (**-Jx**\ \|\ **X**): Labels
for each axis can be added by surrounding them with colons (**:**). If
the first character in the label is a period, then the label is used as
plot title; if it is a comma (**,**) then the label is appended to each
annotation; if it is an equal sign (**=**) the the prefix is prepended
to each annotation (start label/prefix with - to avoid space between
annotation and item); else it is the axis label. If the label consists
of more than one word, enclose the entire label in double quotes (e.g.,
**:**"my label"**:**). If you need to use a colon (:) as part of your
label you must specify it using its octal code (\\072). If you want to
plot just the map boundary and nothing else, **-B**\ 0 suffices.
By default, all 4 boundaries are plotted (referred to as **W**, **E**,
**S**, **N**). To change the default, append the code for only those
axes you want (e.g., **WS** for standard lower-left x- and y-axis
system). Upper case (e.g., **W**) means draw axis/tickmarks AND annotate
it, whereas lower case (e.g., **w**) will only draw axis/tickmarks. If
ONLY s is appended, separate it with a comma to avoid its interpretation
as the unit of second. If a 3-D basemap is selected with **-p** and
**-Jz**, append **Z** or **z** to control the appearance of the vertical
axis. Append **+** to draw the outline of the cube defined by **-R**.
Note that for 3-D views the title, if given, will be suppressed.
Finally, to paint the inside of the map region, append **+g**\ *fill*
[no fill].
For non-geographical projections: Give negative scale (in **-Jx**) or
axis length (in **-JX**) to change the direction of increasing
coordinates (i.e., to make the y-axis positive down). For log10 axes:
Annotations can be specified in one of three ways: (1) *stride* can be
1, 2, 3, or -*n*. Annotations will then occur at 1, 1-2-5, or
1-2-3-4-...-9, respectively; for -*n* we annotate every *n*\ ’t
magnitude. This option can also be used for the frame and grid
intervals. (2) An **l** is appended to the *tickinfo* string. Then,
log10 of the tick value is plotted at every integer log10 value. (3) A
**p** is appended to the *tickinfo* string. Then, annotations appear as
10 raised to log10 of the tick value. For power axes: Annotations can be
specified in one of two ways: (1) *stride* sets the regular annotation
interval. (2) A **p** is appended to the *tickinfo* string. Then, the
annotation interval is expected to be in transformed units, but the
annotation value will be plotted as untransformed units. E.g., if
*stride* = 1 and *power* = 0.5 (i.e., sqrt), then equidistant
annotations labeled 1-4-9... will appear.
These GMT parameters can affect the appearance of the map boundary:
**MAP\_ANNOT\_MIN\_ANGLE**, **MAP\_ANNOT\_MIN\_SPACING**,
**FONT\_ANNOT\_PRIMARY**, **FONT\_ANNOT\_SECONDARY**,
**MAP\_ANNOT\_OFFSET\_PRIMARY**, **MAP\_ANNOT\_OFFSET\_SECONDARY**,
**MAP\_ANNOT\_ORTHO**, **MAP\_FRAME\_AXES**, **MAP\_DEFAULT\_PEN**,
**MAP\_FRAME\_TYPE**, **FORMAT\_GEO\_MAP**, **MAP\_FRAME\_PEN**,
**MAP\_FRAME\_WIDTH**, **MAP\_GRID\_CROSS\_SIZE\_PRIMARY**,
**MAP\_GRID\_PEN\_PRIMARY**, **MAP\_GRID\_CROSS\_SIZE\_SECONDARY**,
**MAP\_GRID\_PEN\_SECONDARY**, **FONT\_TITLE**, **FONT\_LABEL**,
**MAP\_LINE\_STEP**, **MAP\_ANNOT\_OBLIQUE**, **FORMAT\_CLOCK\_MAP**,
**FORMAT\_DATE\_MAP**, **FORMAT\_TIME\_PRIMARY\_MAP**,
**FORMAT\_TIME\_SECONDARY\_MAP**, **TIME\_LANGUAGE**,
**TIME\_WEEK\_START**, **MAP\_TICK\_LENGTH**, and **MAP\_TICK\_PEN**;
see the **gmt.conf** man page for details.

**-J**\ *parameters*

Select map projection. The following character determines the
projection. If the character is upper case then the argument(s) supplied
as scale(s) is interpreted to be the map width (or axis lengths), else
the scale argument(s) is the map scale (see its definition for each
projection). UNIT is cm, inch, or point, depending on the
**PROJ\_LENGTH\_UNIT** setting in **gmt.conf**, but this can be
overridden on the command line by appending **c**, **i**, or **p** to
the *scale* or *width* values. Append **h**, **+**, or **-** to the
given *width* if you instead want to set map height, the maximum
dimension, or the minimum dimension, respectively [Default is **w** for
width].
In case the central meridian is an optional parameter and it is being
omitted, then the center of the longitude range given by the **-R**
option is used. The default standard parallel is the equator.
The ellipsoid used in the map projections is user-definable by editing
the **gmt.conf** file in your home directory. 73 commonly used
ellipsoids and spheroids are currently supported, and users may also
specify their own custum ellipsoid parameters [Default is WGS-84].
Several GMT parameters can affect the projection: **PROJ\_ELLIPSOID**,
**GMT\_INTERPOLANT**, **PROJ\_SCALE\_FACTOR**, and
**PROJ\_LENGTH\_UNIT**; see the **gmt.conf** man page for details.
Choose one of the following projections (The **E** or **C** after
projection names stands for Equal-Area and Conformal, respectively):

    **CYLINDRICAL PROJECTIONS:**

    **-Jc**\ *lon0/lat0/scale* or **-JC**\ *lon0/lat0/width* (Cassini).

    Give projection center *lon0/lat0* and *scale* (**1:**\ *xxxx* or
    UNIT/degree).

    **-Jcyl\_stere**/[*lon0/*\ [*lat0/*\ ]]\ *scale* or
    **-JCyl\_stere**/[*lon0/*\ [*lat0/*\ ]]\ *width* (Cylindrical
    Stereographic).

    Give central meridian *lon0* (optional), standard parallel *lat0*
    (optional), and *scale* along parallel (**1:**\ *xxxx* or
    UNIT/degree). The standard parallel is typically one of these (but
    can be any value):

            66.159467 - Miller’s modified Gall
             55 - Kamenetskiy’s First
             45 - Gall’s Stereographic
             30 - Bolshoi Sovietskii Atlas Mira or Kamenetskiy’s Second
             0 -
            Braun’s Cylindrical

    **-Jj**\ [*lon0/*\ ]\ *scale* or **-JJ**\ [*lon0/*\ ]\ *width*
    (Miller Cylindrical Projection).

    Give the central meridian *lon0* (optional) and *scale*
    (**1:**\ *xxxx* or UNIT/degree).

    **-Jm**\ [*lon0/*\ [*lat0/*\ ]]\ *scale* or
    **-JM**\ [*lon0/*\ [*lat0/*\ ]]\ *width*

    Give central meridian *lon0* (optional), standard parallel *lat0*
    (optional), and *scale* along parallel (**1:**\ *xxxx* or
    UNIT/degree).

    **-Jo**\ *parameters* (Oblique Mercator **[C]**).

    Specify one of:

        **-Jo**\ [**a**\ ]\ *lon0/lat0/azimuth/scale* or
        **-JO**\ [**a**\ ]\ *lon0/lat0/azimuth/width*
        Set projection center *lon0/lat0*, *azimuth* of oblique equator,
        and *scale*.
        **-Jo**\ [**b**\ ]\ *lon0/lat0/lon1/lat1/scale* or
        **-JO**\ [**b**\ ]\ *lon0/lat0/lon1/lat1/scale*
        Set projection center *lon0/lat0*, another point on the oblique
        equator *lon1/lat1*, and *scale*.
        **-Joc**\ *lon0/lat0/lonp/latp/scale* or
        **-JOc**\ *lon0/lat0/lonp/latp/scale*
        Set projection center *lon0/lat0*, pole of oblique projection
        *lonp/latp*, and *scale*.
        Give *scale* along oblique equator (**1:**\ *xxxx* or
        UNIT/degree).

    **-Jq**\ [*lon0/*\ [*lat0/*\ ]]\ *scale* or
    **-JQ**\ [*lon0/*\ [*lat0/*\ ]]\ *width* (Cylindrical Equidistant).

    Give the central meridian *lon0* (optional), standard parallel
    *lat0* (optional), and *scale* (**1:**\ *xxxx* or UNIT/degree). The
    standard parallel is typically one of these (but can be any value):

            61.7 - Grafarend and Niermann, minimum linear distortion
             50.5 - Ronald Miller Equirectangular
             43.5 - Ronald Miller, minimum continental distortion
             42 - Grafarend and Niermann
             37.5 - Ronald Miller, minimum overall distortion
             0 - Plate Carree, Simple Cylindrical, Plain/Plane Chart

    **-Jt**\ *lon0/*\ [*lat0/*\ ]\ *scale* or
    **-JT**\ *lon0/*\ [*lat0/*\ ]\ *width*

    Give the central meridian *lon0*, central parallel *lat0*
    (optional), and *scale* (**1:**\ *xxxx* or UNIT/degree).

    **-Ju**\ *zone/scale* or **-JU**\ *zone/width* (UTM - Universal
    Transverse Mercator **[C]**).

    Give the UTM zone (A,B,1-60[C-X],Y,Z)) and *scale* (**1:**\ *xxxx*
    or UNIT/degree).
    Zones: If C-X not given, prepend - or + to enforce southern or
    northern hemisphere conventions [northern if south > 0].

    **-Jy**\ [*lon0/*\ [*lat0/*\ ]]\ *scale* or
    **-JY**\ [*lon0/*\ [*lat0/*\ ]]\ *width* (Cylindrical Equal-Area
    **[E]**).

    Give the central meridian *lon0* (optional), standard parallel
    *lat0* (optional), and *scale* (**1:**\ *xxxx* or UNIT/degree). The
    standard parallel is typically one of these (but can be any value):

            50 - Balthasart
             45 - Gall-Peters
             37.0666 - Caster
             37.4 - Trystan Edwards
             37.5 - Hobo-Dyer
             30 - Behrman
             0 - Lambert (default)

    **CONIC PROJECTIONS:**

    **-Jb**\ *lon0/lat0/lat1/lat2/scale* or
    **-JB**\ *lon0/lat0/lat1/lat2/width* (Albers **[E]**).
        Give projection center *lon0/lat0*, two standard parallels
        *lat1/lat2*, and *scale* (**1:**\ *xxxx* or UNIT/degree).
    **-Jd**\ *lon0/lat0/lat1/lat2/scale* or
    **-JD**\ *lon0/lat0/lat1/lat2/width* (Conic Equidistant)
        Give projection center *lon0/lat0*, two standard parallels
        *lat1/lat2*, and *scale* (**1:**\ *xxxx* or UNIT/degree).
    **-Jl**\ *lon0/lat0/lat1/lat2/scale* or
    **-JL**\ *lon0/lat0/lat1/lat2/width* (Lambert **[C]**)
        Give origin *lon0/lat0*, two standard parallels *lat1/lat2*, and
        *scale* along these (**1:**\ *xxxx* or UNIT/degree).
    **-Jpoly**/[*lon0/*\ [*lat0/*\ ]]\ *scale* or
    **-JPoly**/[*lon0/*\ [*lat0/*\ ]]\ *width* ((American) Polyconic).
        Give the central meridian *lon0* (optional), reference parallel
        *lat0* (optional, default = equator), and *scale* along central
        meridian (**1:**\ *xxxx* or UNIT/degree).

    **AZIMUTHAL PROJECTIONS:**

    Except for polar aspects, **-R**\ w/e/s/n will be reset to **-Rg**.
    Use **-R**\ <...>\ **r** for smaller regions.

    **-Ja**\ *lon0/lat0*\ [*/horizon*\ ]\ */scale* or
    **-JA**\ *lon0/lat0*\ [*/horizon*\ ]\ */width* (Lambert **[E]**).
        *lon0/lat0* specifies the projection center. *horizon* specifies
        the max distance from projection center (in degrees, <= 180,
        default 90). Give *scale* as **1:**\ *xxxx* or *radius/lat*,
        where *radius* is distance in UNIT from origin to the oblique
        latitude *lat*.
    **-Je**\ *lon0/lat0*\ [*/horizon*\ ]\ */scale* or
    **-JE**\ *lon0/lat0*\ [*/horizon*\ ]\ */width* (Azimuthal
    Equidistant).
        *lon0/lat0* specifies the projection center. *horizon* specifies
        the max distance from projection center (in degrees, <= 180,
        default 180). Give *scale* as **1:**\ *xxxx* or *radius/lat*,
        where *radius* is distance in UNIT from origin to the oblique
        latitude *lat*.
    **-Jf**\ *lon0/lat0*\ [*/horizon*\ ]\ */scale* or
    **-JF**\ *lon0/lat0*\ [*/horizon*\ ]\ */width* (Gnomonic).
        *lon0/lat0* specifies the projection center. *horizon* specifies
        the max distance from projection center (in degrees, < 90,
        default 60). Give *scale* as **1:**\ *xxxx* or *radius/lat*,
        where *radius* is distance in UNIT from origin to the oblique
        latitude *lat*.
    **-Jg**\ *lon0/lat0*\ [*/horizon*\ ]\ */scale* or
    **-JG**\ *lon0/lat0*\ [*/horizon*\ ]\ */width* (Orthographic).
        *lon0/lat0* specifies the projection center. *horizon* specifies
        the max distance from projection center (in degrees, <= 90,
        default 90). Give *scale* as **1:**\ *xxxx* or *radius/lat*,
        where *radius* is distance in UNIT from origin to the oblique
        latitude *lat*.
    **-Jg**\ *lon0/lat0/altitude/azimuth/tilt/twist/Width/Height/scale*
    or
    **-JG**\ *lon0/lat0/altitude/azimuth/tilt/twist/Width/Height/width*
    (General Perspective).
        *lon0/lat0* specifies the projection center. *altitude* is the
        height (in km) of the viewpoint above local sea level. If
        *altitude* is less than 10, then it is the distance from the
        center of the earth to the viewpoint in earth radii. If
        *altitude* has a suffix **r** then it is the radius from the
        center of the earth in kilometers. *azimuth* is measured to the
        east of north of view. *tilt* is the upward tilt of the plane of
        projection. If *tilt* is negative, then the viewpoint is
        centered on the horizon. Further, specify the clockwise *twist*,
        *Width*, and *Height* of the viewpoint in degrees. Give *scale*
        as **1:**\ *xxxx* or *radius/lat*, where *radius* is distance in
        UNIT from origin to the oblique latitude *lat*.
    **-Js**\ *lon0/lat0*\ [*/horizon*\ ]\ */scale* or
    **-JS**\ *lon0/lat0*\ [*/horizon*\ ]\ */width* (General
    Stereographic **[C]**).
        *lon0/lat0* specifies the projection center. *horizon* specifies
        the max distance from projection center (in degrees, < 180,
        default 90). Give *scale* as **1:**\ *xxxx* (true at pole) or
        *lat0*/**1:**\ *xxxx* (true at standard parallel *lat0*) or
        *radius/lat* (*radius* in UNIT from origin to the oblique
        latitude *lat*). Note if **1:**\ *xxxx* is used then to specify
        *horizon* you must also specify the *lat0* as +-90 to avoid
        ambiguity.

    **MISCELLANEOUS PROJECTIONS:**

    **-Jh**\ [*lon0/*\ ]\ *scale* or **-JH**\ [*lon0/*\ ]\ *width*
    (Hammer **[E]**).
        Give the central meridian *lon0* (optional) and *scale* along
        equator (**1:**\ *xxxx* or UNIT/degree).
    **-Ji**\ [*lon0/*\ ]\ *scale* or **-JI**\ [*lon0/*\ ]\ *width*
    (Sinusoidal **[E]**).
        Give the central meridian *lon0* (optional) and *scale* along
        equator (**1:**\ *xxxx* or UNIT/degree).
    **-Jkf**\ [*lon0/*\ ]\ *scale* or **-JKf**\ [*lon0/*\ ]\ *width*
    (Eckert IV) **[E]**).
        Give the central meridian *lon0* (optional) and *scale* along
        equator (**1:**\ *xxxx* or UNIT/degree).
    **-Jk**\ [**s**\ ][*lon0/*\ ]\ *scale* or
    **-JK**\ [**s**\ ][*lon0/*\ ]\ *width* (Eckert VI) **[E]**).
        Give the central meridian *lon0* (optional) and *scale* along
        equator (**1:**\ *xxxx* or UNIT/degree).
    **-Jn**\ [*lon0/*\ ]\ *scale* or **-JN**\ [*lon0/*\ ]\ *width*
    (Robinson).
        Give the central meridian *lon0* (optional) and *scale* along
        equator (**1:**\ *xxxx* or UNIT/degree).
    **-Jr**\ [*lon0/*\ ]\ *scale* **-JR**\ [*lon0/*\ ]\ *width* (Winkel
    Tripel).
        Give the central meridian *lon0* (optional) and *scale* along
        equator (**1:**\ *xxxx* or UNIT/degree).
    **-Jv**\ [*lon0/*\ ]\ *scale* or **-JV**\ [*lon0/*\ ]\ *width* (Van
    der Grinten).
        Give the central meridian *lon0* (optional) and *scale* along
        equator (**1:**\ *xxxx* or UNIT/degree).
    **-Jw**\ [*lon0/*\ ]\ *scale* or **-JW**\ [*lon0/*\ ]\ *width*
    (Mollweide **[E]**).
        Give the central meridian *lon0* (optional) and *scale* along
        equator (**1:**\ *xxxx* or UNIT/degree).

    **NON-GEOGRAPHICAL PROJECTIONS:**

    **-Jp**\ [**a**\ ]\ *scale*\ [*/origin*\ ][\ **r**\ \|\ **z**] or
    **-JP**\ [**a**\ ]\ *width*\ [*/origin*\ ][\ **r**\ \|\ **z**]
    (Polar coordinates (theta,r))

    Optionally insert **a** after **-Jp** [ or **-JP**] for azimuths CW
    from North instead of directions CCW from East [Default]. Optionally
    append /*origin* in degrees to indicate an angular offset [0]).
    Finally, append **r** if r is elevations in degrees (requires s >= 0
    and n <= 90) or **z** if you want to annotate depth rather than
    radius [Default]. Give *scale* in UNIT/r-unit.

    **-Jx**\ *x-scale*\ [*/y-scale*\ ] or
    **-JX**\ *width*\ [*/height*\ ] (Linear, log, and power scaling)

    Give *x-scale* (**1:**\ *xxxx* or UNIT/x-unit) and/or *y-scale*
    (**1:**\ *xxxx* or UNIT/y-unit); or specify *width* and/or *height*
    in UNIT. *y-scale*\ =\ *x-scale* if not specified separately and
    using **1:**\ *xxxx* implies that x-unit and y-unit are in meters.
    Use negative scale(s) to reverse the direction of an axis (e.g., to
    have y be positive down). Set *height* or *width* to 0 to have it
    recomputed based on the implied scale of the other axis. Optionally,
    append to *x-scale*, *y-scale*, *width* or *height* one of the
    following:

        **d**
        Data are geographical coordinates (in degrees).
        **l**
        Take log10 of values before scaling.
        **p**\ *power*
        Raise values to *power* before scaling.
        **t**
        Input coordinates are time relative to **TIME\_EPOCH**.
        **T**
        Input coordinates are absolute time.
        Default axis lengths (see **gmt.conf**) can be invoked using
        **-JXh** (for landscape); **-JXv** (for portrait) will swap the
        x- and y-axis lengths. The default unit for this installation is
        either cm or inch, as defined in the file **share/gmt.conf**.
        However, you may change this by editing your **gmt.conf**
        file(s).

When **-J** is used without any further arguments, or just with the
projection type, the arguments of the last used **-J**, or the last used
**-J** with that projection type, will be used.

**-Jz**\ \|\ **Z**\ *parameters*
    Set z-axis scaling; same syntax as **-Jx**.
**-K**
    More *PostScript* code will be appended later [Default terminates
    the plot system]. Required for all but the last plot command when
    building multi-layer plots.
**-O**
    Selects Overlay plot mode [Default initializes a new plot system].
    Required for all but the first plot command when building
    multi-layer plots.
**-P**
    Select "Portrait" plot orientation [Default is "Landscape"; see
    **gmt.conf** or **gmtset** to change the **PS\_PAGE\_ORIENTATION**
    parameter, or supply --PS\_PAGE\_ORIENTATION=\ *orientation* on the
    command line].
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ]
    *xmin*, *xmax*, *ymin*, and *ymax* specify the region of interest.
    For geographic regions, these limits correspond to *west*, *east*,
    *south*, and *north* and you may specify them in decimal degrees or
    in [+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left
    and upper right map coordinates are given instead of w/e/s/n. The
    two shorthands **-Rg** and **-Rd** stand for global domain (0/360
    and -180/+180 in longitude respectively, with -90/+90 in latitude).
    Alternatively, specify the name of an existing grid file and the
    **-R** settings (and grid spacing, if applicable) are copied from
    the grid. When **-R** is used without any further arguments, the
    values from the last use of **-R** in a previous **GMT** command
    will be used.
    For calendar time coordinates you may either give (a) relative time
    (relative to the selected **TIME\_EPOCH** and in the selected
    **TIME\_UNIT**; append **t** to **-JX**\ \|\ **x**), or (b) absolute
    time of the form [*date*\ ]\ **T**\ [*clock*\ ] (append **T** to
    **-JX**\ \|\ **x**). At least one of *date* and *clock* must be
    present; the **T** is always required. The *date* string must be of
    the form [-]yyyy[-mm[-dd]] (Gregorian calendar) or yyyy[-Www[-d]]
    (ISO week calendar), while the *clock* string must be of the form
    hh:mm:ss[.xxx]. The use of delimiters and their type and positions
    must be exactly as indicated (however, input, output and plot
    formats are customizable; see **gmt.conf**).
     In case of perspective view (**-p**), a z-range (*zmin*, *zmax*)
    can be appended to indicate the third dimension. This needs to be
    done only when using the **-Jz** option, not when using only the
    **-p** option. In the latter case a perspective view of the plane is
    plotted, with no third dimension.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*]
    Draw Unix System time stamp on plot. By adding *just/dx/dy/*, the
    user may specify the justification of the stamp and where the stamp
    should fall on the page relative to lower left corner of the plot.
    For example, BL/0/0 will align the lower left corner of the time
    stamp with the lower left corner of the plot. Optionally, append a
    *label*, or **c** (which will plot the command string.). The **GMT**
    parameters **MAP\_LOGO**, **MAP\_LOGO\_POS**, and
    **FORMAT\_TIME\_LOGO** can affect the appearance; see the
    **gmt.conf** man page for details. The time string will be in the
    locale set by the environment variable **TZ** (generally local
    time).
**-V**\ [*level*\ ]
    Select verbose mode, which will send progress reports to *stderr*.
    Choose among 5 levels of verbosity; each level adds mode messages:
     **0** - Complete silence, not even fatal error messages are
    produced.
     **1** - Produce only fatal error messages (same as when **-V** is
    omitted).
     **2** - Produce also warnings and progress messages (same as **-V**
    only).
     **3** - Produce also detailed progress messages.
     **4** - Produce also debugging messages.
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
    Shift plot origin relative to the current origin by
    (*x-shift*,\ *y-shift*) and optionally append the length unit
    (**c**, **i**, or **p**). You can prepend **a** to shift the origin
    back to the original position after plotting, prepend **c** to
    center the plot on the center of the paper (optionally add shift),
    prepend **f** to shift the origin relative to the fixed lower left
    corner of the page, or prepend **r** [Default] to move the origin
    relative to its current location. If **-O** is used then the default
    (*x-shift*,\ *y-shift*) is (r0), otherwise it is (r1i). When **-X**
    or **-Y** are used without any further arguments, the values from
    the last use of that option in a previous **GMT** command will be
    used.
**-a**\ *col*\ =\ *name*\ [*...*\ ]
    Control how aspatial data are handled in GMT during input and
    output.
    Reading OGR/GMT-formatted files. To assign certain aspatial data
    items to GMT data columns, give one or more comma-separated
    associations *col*\ =\ *name*, where *name* is the name of an
    aspatial attribute field in a OGR/GMT file and whose value we wish
    to as data input for column *col*. In addition, to assign an
    aspatial value to non-column data, you may specify *col* as **D**
    for *distance*, **G** for *fill*, **I** for *ID*, **L** for *label*,
    **T** for *text*, **W** for *pen*, and **Z** for *value* [e.g., used
    to look-up color via a CPT].
    Write OGR/GMT-formatted files. Give one or more comma-separated
    associations *col*\ =\ *name*\ [:*type*], To write OGR/GMT-formatted
    files, give one or more comma-separated associations
    *col*\ =\ *name*\ [:*type*], with an optional data type from DOUBLE,
    FLOAT, INTEGER, CHAR, STRING, DATETIME, or LOGICAL [DOUBLE]. To
    extract information from GMT multisegment headers encoded in the
    **-D**\ *distance*, **-G**\ *fill*, **-I**\ *ID*, **-L**\ *label*,
    **-T**\ *text*, **-W**\ *pen*, or **-Z**\ *value* settings, specify
    *COL* as **D**, **G**, **I**, **L**, **T**, **W** or **Z**,
    respectively; type will be set automatically. Finally, you *must*
    append **+g**\ *geometry*, where *geometry* is either POINT, LINE,
    or POLY. Optionally, prepend **M** for multi-versions of these
    geometries. To force the clipping of features crossing the Dateline,
    use upper-case **+G** instead. See GMT Appendix Q for details of the
    OGR/GMT file format.
**-bi**\ [*ncol*\ ][**t**\ ]
    Select binary input. Append one or more comma-separated combinations
    of *n*\ *type*, where *type* is one of **c** (signed byte), **u**
    (unsigned byte), **h** (signed 2-byte int), **H** (unsigned 2-byte
    int), **i** (signed 4-byte int), **I** (unsigned 4-byte int), **l**
    (signed 8-byte int), **L** (unsigned 8-byte int), **f** (4-byte
    single-precision float), and **d** (8-byte double-precision float).
    Append **w** to any item to force byte-swapping. Alternatively,
    append **+L**\ \|\ **B** to indicate that the entire data file
    should be read as little- or big-endian, respectively. Here, *n* is
    the number of each item in your binary input file; the total number
    may exceeds the columns actually needed by the program. If no *n* is
    specified we assume that *type* applies to all columns and that *n*
    is implied by the expectation of the program. If the input file is
    netCDF, no **-b** is needed; simply append
    *var1*\ **/**\ *var2*\ **/**\ *...* to the filename to specify the
    variables to be read.
**-bo**\ [*ncol*\ ][**t**\ ]
    Select binary output. Append one or more comma-separated
    combinations of *n*\ *type*, where *type* is one of **c** (signed
    byte), **u** (unsigned byte), **h** (signed 2-byte int), **H**
    (unsigned 2-byte int), **i** (signed 4-byte int), **I** (unsigned
    4-byte int), **l** (signed 8-byte int), **L** (unsigned 8-byte int),
    **f** (4-byte single-precision float), and **d** (8-byte
    double-precision float). Append **w** to any item to force
    byte-swapping. Alternatively, append **+L**\ \|\ **B** to indicate
    that the entire data file should be written as little- or
    big-endian, respectively. Here, *n* is the number of each items in
    your binary input file; the total may exceeds the columns actually
    needed by the program. If no *n* is specified we assume that *type*
    applies to all columns and that *n* is implied by the default output
    of the program. NetCDF file output is not supported.
**-c**\ *copies*
    Specify number of plot copies. [Default is 1]. When used without
    argument, use the same number of copies and specified in the last
    **-c** in a previous **GMT** command.
**-f**\ [**i**\ \|\ **o**]\ *colinfo*
    Specify the data types of input and/or output columns (time or
    geographical data). Specify **i** or **o** to make this apply only
    to input or output [Default applies to both]. Give one or more
    columns (or column ranges) separated by commas, or use **-f**
    multiple times. Append **T** (absolute calendar time), **t**
    (relative time in chosen **TIME\_UNIT** since **TIME\_EPOCH**),
    **x** (longitude), **y** (latitude), or **f** (floating point) to
    each column or column range item. Shorthand
    **-f**\ [**i**\ \|\ **o**]\ **g** means
    **-f**\ [**i**\ \|\ **o**]0\ **x**,1\ **y** (geographic
    coordinates).
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
    Examine the spacing between consecutive data points in order to
    impose breaks in the line. Append **x**\ \|\ **X** or
    **y**\ \|\ **Y** to define a gap when there is a large enough change
    in the x or y coordinates, respectively, or **d**\ \|\ **D** for
    distance gaps; use upper case to calculate gaps from projected
    coordinates. For gap-testing on other columns use [*col*\ ]\ **z**;
    if *col* is not prepended the it defaults to 2 (i.e., 3rd column).
    Append [+\|-]\ *gap* and optionally a unit **u**. Regarding optional
    signs: -ve means previous minus current column value must exceed
    *gap* to be a gap, +ve means current minus previous column value
    must exceed *gap*, and no sign means the absolute value of the
    difference must exceed *gap*. For geographic data
    (**x**\ \|\ **y**\ \|\ **d**), the unit **u** may be arc
    **d**\ egree, **m**\ inute, or **s**\ econd, or m\ **e**\ ter
    [Default], **f**\ eet, **k**\ ilometer, **M**\ iles, or
    **n**\ autical miles. For projected data
    (**X**\ \|\ **Y**\ \|\ **D**), choose from **i**\ nch,
    **c**\ entimeter, or **p**\ oints [Default unit set by
    **PROJ\_LENGTH\_UNIT**]. Note: For **x**\ \|\ **y**\ \|\ **z** with
    time data the unit is instead controlled by **TIME\_UNIT**. Repeat
    the option to specify multiple criteria, of which any can be met to
    produce a line break. Issue an additional **-ga** to indicate that
    all criteria must be met instead.
**-h**\ [**i**\ \|\ **o**][*n*\ ]
    Input file(s) has header record(s). If used, the default number of
    header records is **IO\_N\_HEADER\_RECS** [1]. Use **-hi** if only
    input data should have header records [Default will write out header
    records if the input data have them]. Blank lines and lines starting
    with # are always skipped. Use **-ho** to force programs to write a
    header record with column names. If used with native binary data we
    interpret *n* to instead mean the number of bytes to skip on input
    or pad on output.
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
    Select specific data columns for input, in arbitrary order. Columns
    not listed will be skipped. Give columns (or column ranges)
    separated by commas [Default reads all columns in order]. To each
    column, optionally add any of the following, in this order: **l**
    takes the **log10** of the input values first; **s**\ *scale*,
    subsequently multiplies by a given scale factor; **o**\ *offset*,
    finally adds a given offset.
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
    Select grid interpolation mode by adding **b** for B-spline
    smoothing, **c** for bicubic interpolation, **l** for bilinear
    interpolation, or **n** for nearest-neighbor value (for example to
    plot categorical data). Optionally, append **+a** to switch off
    antialiasing (where supported). Append **+b**\ *BC* to override the
    boundary conditions used, adding **g** for geographic, **p** for
    periodic, or **n** for natural boundary conditions. For the latter
    two you may append **x** or **y** to specify just one direction,
    otherwise both are assumed. Add append **+t**\ *threshold* to
    control how close to nodes with NaNs the interpolation will go. A
    *threshold* of 1.0 requires all (4 or 16) nodes involved in
    interpolation to be non-NaN. 0.5 will interpolate about half way
    from a non-NaN value; 0.1 will go about 90% of the way, etc.
    [Default is bicubic interpolation with antialiasing and a threshold
    of 0.5, using geographic (if grid is known to be geographic) or
    natural boundary conditions].
**-o**\ *cols*\ [,*...*]
    Select specific data columns for output, in arbitrary order. Columns
    not listed will be skipped. Give columns (or column ranges)
    separated by commas [Default writes all columns in order].
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
    Selects perspective view and sets the azimuth and elevation of the
    viewpoint [180/90]. When **-p** is used in consort with **-Jz** or
    **-JZ**, a third value can be appended which indicates at which
    z-level all 2D material, like the plot frame, is plotted (in
    perspective). [Default is at the bottom of the z-axis]. Use **-px**
    or **-py** to plot against the "wall" x = level or y = level
    (default is at the bottom of the vertical axis). For frames used for
    animation, you may want to append **+** to fix the center of your
    data domain (or specify a particular world coordinate point with
    **+w**\ *lon0*/*lat*\ [/*z*]) which will project to the center of
    your page size (or specify the coordinates of the projected view
    point with **+v**\ *x0*/*y0*. When **-p** is used without any
    further arguments, the values from the last use of **-p** in a
    previous **GMT** command will be used.
**-r** (\*)
    Force pixel node registration [Default is gridline registration].
    (Node registrations are defined in **GMT** Technical Reference and
    Cookbook, Appendix B on grid file formats.)
**-s**\ [*cols*\ ][\ **a**\ \|\ **r**]
    Suppress output for records whose *z*-value equals NaN [Default
    outputs all records]. Append **a** to skip records where at least
    one field equal NaN. Append **r** to reverse the suppression, i.e.,
    only output the records whose *z*-value equals NaN. Alternatively,
    indicate a comma-separated list of all columns or column ranges to
    consider for this NaN test.
**-t**\ [*transp*\ ]
    Set PDF transparency level for an overlay, in 0-100 range. [Default
    is 0, i.e. opaque].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Specifying Color <#toc6>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~

*color*
    The *color* of lines, areas and patterns can be specified by a valid
    color name; by a gray shade (in the range 0-255); by a decimal color
    code (r/g/b, each in range 0-255; h-s-v, ranges 0-360, 0-1, 0-1; or
    c/m/y/k, each in range 0-1); or by a hexadecimal color code
    (#rrggbb, as used in HTML). For PDF transparency, append
    @\ *transparency* in the 0-100 range [0 or opaque]. See the
    **gmtcolors** manpage for more information and a full list of color
    names. See **GMT** Cookbook & Technical Reference Chapter 4 for more
    information.

`Specifying Fill <#toc7>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~

*fill*
    The attribute *fill* specifies the solid shade or solid *color* (see
    SPECIFYING COLOR above) or the pattern used for filling polygons.
    Patterns are specified as **p**\ *dpi/pattern*, where *pattern*
    gives the number of the built-in pattern (1-90) *or* the name of a
    Sun 1-, 8-, or 24-bit raster file. The *dpi* sets the resolution of
    the image. For 1-bit rasters: use **P**\ *dpi/pattern* for inverse
    video, or append **:F**\ *color*\ [**B**\ [*color*\ ]] to specify
    fore- and background colors (use *color* = - for transparency). See
    **GMT** Cookbook & Technical Reference Appendix E for information on
    individual patterns.

`Specifying Fonts <#toc8>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~

*font*
    The attributes of text fonts as defined by *font* is a comma
    delimited list of *size*, *fonttype* and *fill*, each of which is
    optional. *size* is the font size (usually in points) but **c** or
    **i** can be added to indicate other units. *fonttype* is the name
    (case sensitive!) of the font or its equivalent numerical ID (e.g.,
    Helvetica-Bold or 1). *fill* specifies the gray shade, color or
    pattern of the text (see SPECIFYING FILL above). Optionally, you may
    append **=**\ *pen* to the *fill* value in order to draw the text
    outline; if used you may optionally skip the text fill by setting it
    to **-**. If any of the attributes is omitted their default or
    previous setting will be retained.

    The 35 available fonts are:

     0\ `` `` `` `` Helvetica
     1\ `` `` `` `` Helvetica-Bold
     2\ `` `` `` `` Helvetica-Oblique
     3\ `` `` `` `` Helvetica-BoldOblique
     4\ `` `` `` `` Times-Roman
     5\ `` `` `` `` Times-Bold
     6\ `` `` `` `` Times-Italic
     7\ `` `` `` `` Times-BoldItalic
     8\ `` `` `` `` Courier
     9\ `` `` `` `` Courier-Bold
     10\ `` `` `` `` Courier-Oblique
     11\ `` `` `` `` Courier-BoldOblique
     12\ `` `` `` `` Symbol
     13\ `` `` `` `` AvantGarde-Book
     14\ `` `` `` `` AvantGarde-BookOblique
     15\ `` `` `` `` AvantGarde-Demi
     16\ `` `` `` `` AvantGarde-DemiOblique
     17\ `` `` `` `` Bookman-Demi
     18\ `` `` `` `` Bookman-DemiItalic
     19\ `` `` `` `` Bookman-Light
     20\ `` `` `` `` Bookman-LightItalic
     21\ `` `` `` `` Helvetica-Narrow
     22\ `` `` `` `` Helvetica-Narrow-Bold
     23\ `` `` `` `` Helvetica-Narrow-Oblique
     24\ `` `` `` `` Helvetica-Narrow-BoldOblique
     25\ `` `` `` `` NewCenturySchlbk-Roman
     26\ `` `` `` `` NewCenturySchlbk-Italic
     27\ `` `` `` `` NewCenturySchlbk-Bold
     28\ `` `` `` `` NewCenturySchlbk-BoldItalic
     29\ `` `` `` `` Palatino-Roman
     30\ `` `` `` `` Palatino-Italic
     31\ `` `` `` `` Palatino-Bold
     32\ `` `` `` `` Palatino-BoldItalic
     33\ `` `` `` `` ZapfChancery-MediumItalic
     34\ `` `` `` `` ZapfDingbats

`Specifying Pens <#toc9>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~

*pen*
    The attributes of lines and symbol outlines as defined by *pen* is a
    comma-delimited list of *width*, *color* and *style*, each of which
    is optional. *width* can be indicated as a measure (in **p**\ oints
    (this is the default), **c**\ entimeters, or **i**\ nches) or as
    **faint**, **default**, **thin**\ [**ner**\ \|\ **nest**],
    **thick**\ [**er**\ \|\ **est**], **fat**\ [**ter**\ \|\ **test**],
    or **obese**. *color* specifies a gray shade or color (see
    SPECIFYING COLOR above). *style* is a combination of dashes ‘-’ and
    dots ‘.’. If any of the attributes is omitted their default or
    previous setting will be retained. See **GMT** Cookbook & Technical
    Reference Chapter 4 for more information.

`Ascii Format Precision <#toc10>`_
----------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Grid File Formats <#toc11>`_
-----------------------------

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

`See Also <#toc12>`_
--------------------

Look up the individual man pages for more details and full syntax. Run
**gmt --help** to list all GMT programs and to show all installation
directories. For an explanation of the various **GMT** settings in this
man page (like **FORMAT\_FLOAT\_OUT**), see the man page of the GMT
configuration file **gmt.conf**. Information is also available on the
**GMT** home page gmt.soest.hawaii.edu

`References <#toc13>`_
----------------------

Wessel, P., W. H. F. Smith, R. Scharroo, and J. Luis, 2011, The Generic
Mapping Tools (GMT) version 5.0.0b Technical Reference & Cookbook,
SOEST/NOAA.
 Wessel, P., and W. H. F. Smith, 1998, New, Improved Version of Generic
Mapping Tools Released, EOS Trans., AGU, 79 (47), p. 579.
 Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic
Mapping Tools Released, EOS Trans., AGU, 76 (33), p. 329.
 Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic
Mapping Tools Released,
`http://www.agu.org/eos\_elec/95154e.html, <http://www.agu.org/eos_elec/95154e.html,>`_
Copyright 1995 by the American Geophysical Union.
 Wessel, P., and W. H. F. Smith, 1991, Free Software Helps Map and
Display Data, EOS Trans., AGU, 72 (41), p. 441.

