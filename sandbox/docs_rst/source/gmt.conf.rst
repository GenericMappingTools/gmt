********
gmt.conf
********


gmt.conf - Configuration for **GMT**

`Description <#toc1>`_
----------------------

The following is a list of the parameters that are user-definable in
**GMT**. The parameter names are always given in UPPER CASE. The
parameter values are case-insensitive unless otherwise noted. The system
defaults are given in brackets [ for SI (and US) ]. Those marked **\***
can be set on the command line as well (the corresponding option is
given in parentheses). Note that default distances and lengths below are
given in both cm or inch; the chosen default depends on your choice of
default unit (see **PROJ\_LENGTH\_UNIT**). You can explicitly specify
the unit used for distances and lengths by appending **c** (cm), **i**
(inch), or **p** (points). When no unit is indicated the value will be
assumed to be in the unit set by **PROJ\_LENGTH\_UNIT**. Several
parameters take only **true** or **false**. Finally, most of these
parameters can be changed on-the-fly via the **--PARAMETER**\ =\ *VALUE*
option to any **GMT** program. However, a few are static and are only
read via the **gmt.conf** file; these are labeled (static).

`Common Specifications <#toc2>`_
--------------------------------

The full explanation for how to specify pens, pattern fills, colors, and
fonts can be found in the **gmt** man page.

**COLOR\_BACKGROUND**
    Color used for the background of images (i.e., when z < lowest
    colortable entry) [black].
**COLOR\_FOREGROUND**
    Color used for the foreground of images (i.e., when z > highest
    colortable entry) [white].
**COLOR\_HSV\_MAX\_SATURATION**
    Maximum saturation (0-1) assigned for most positive intensity value
    [0.1].
**COLOR\_HSV\_MIN\_SATURATION**
    Minimum saturation (0-1) assigned for most negative intensity value
    [1.0].
**COLOR\_HSV\_MAX\_VALUE**
    Maximum value (0-1) assigned for most positive intensity value
    [1.0].
**COLOR\_HSV\_MIN\_VALUE**
    Minimum value (0-1) assigned for most negative intensity value
    [0.3].
**COLOR\_MODEL**
    Selects in which color space a color palette should be interpolated.
    By default, color interpolation takes place directly on the RGB
    values which can produce some unexpected hues, whereas interpolation
    directly on the HSV values better preserves those hues. The choices
    are: **none** (default: use whatever the COLOR\_MODEL setting in the
    color palette file demands), **rgb** (force interpolation in RGB),
    **hsv** (force interpolation in HSV), **cmyk** (assumes colors are
    in CMYK but interpolates in RGB).
**COLOR\_NAN**
    Color used for the non-defined areas of images (i.e., where z ==
    NaN) [127.5].
**FONT**
    Sets the default for all fonts, except FONT\_LOGO. This setting is
    not included in the **gmt.conf** file.
**FONT\_ANNOT\_PRIMARY**
    Font used for primary annotations, etc. [12p,Helvetica,black]. When
    **+** is prepended, scale fonts, offsets and ticklengths relative to
    **FONT\_ANNOT\_PRIMARY**.
**FONT\_ANNOT\_SECONDARY**
    Font to use for time axis secondary annotations
    [14p,Helvetica,black].
**FONT\_LABEL**
    Font to use when plotting labels below axes [16p,Helvetica,black].
**FONT\_LOGO**
    Font to use for text plotted as part of the GMT time logo
    [8p,Helvetica,black].
**FONT\_TITLE**
    Font to use when plotting titles over graphs [24p,Helvetica,black].
**FORMAT\_CLOCK\_IN**
    Formatting template that indicates how an input clock string is
    formatted. This template is then used to guide the reading of clock
    strings in data fields. To properly decode 12-hour clocks, append am
    or pm (or upper case) to match your data records. As examples, try
    hh:mm, hh:mm:ssAM, etc. [hh:mm:ss].
**FORMAT\_CLOCK\_MAP**
    Formatting template that indicates how an output clock string is to
    be plotted. This template is then used to guide the formatting of
    clock strings in plot annotations. See **FORMAT\_CLOCK\_OUT** for
    details. [hh:mm:ss].
**FORMAT\_CLOCK\_OUT**
    Formatting template that indicates how an output clock string is to
    be formatted. This template is then used to guide the writing of
    clock strings in data fields. To use a floating point format for the
    smallest unit (e.g. seconds), append .xxx, where the number of x
    indicates the desired precision. If no floating point is indicated
    then the smallest specified unit will be rounded off to nearest
    integer. For 12-hour clocks, append am, AM, a.m., or A.M. (**GMT**
    will replace a\|A with p\|P for pm). If your template starts with a
    leading hyphen (-) then each integer item (y,m,d) will be printed
    without leading zeros (default uses fixed width formats). As
    examples, try hh:mm, hh.mm.ss, hh:mm:ss.xxxx, hha.m., etc.
    [hh:mm:ss].
**FORMAT\_DATE\_IN**
    Formatting template that indicates how an input date string is
    formatted. This template is then used to guide the reading of date
    strings in data fields. You may specify either Gregorian calendar
    format or ISO week calendar format. Gregorian calendar: Use any
    combination of yyyy (or yy for 2-digit years; if so see
    **TIME\_Y2K\_OFFSET\_YEAR**), mm (or o for abbreviated month name in
    the current time language), and dd, with or without delimiters. For
    day-of-year data, use jjj instead of mm and/or dd. Examples can be
    ddmmyyyy, yy-mm-dd, dd-o-yyyy, yyyy/dd/mm, yyyy-jjj, etc. ISO
    Calendar: Expected template is yyyy[-]W[-]ww[-]d, where ww is ISO
    week and d is ISO week day. Either template must be consistent,
    e.g., you cannot specify months if you do not specify years.
    Examples are yyyyWwwd, yyyy-Www, etc. [yyyy-mm-dd].
**FORMAT\_DATE\_MAP**
    Formatting template that indicates how an output date string is to
    be plotted. This template is then used to guide the plotting of date
    strings in data fields. See **FORMAT\_DATE\_OUT** for details. In
    addition, you may use a single o instead of mm (to plot month name)
    and u instead of W[-]ww to plot "Week ##". Both of these text
    strings will be affected by the **TIME\_LANGUAGE**,
    **FORMAT\_TIME\_PRIMARY\_MAP** and **FORMAT\_TIME\_SECONDARY\_MAP**
    setting. [yyyy-mm-dd].
**FORMAT\_DATE\_OUT**
    Formatting template that indicates how an output date string is to
    be formatted. This template is then used to guide the writing of
    date strings in data fields. You may specify either Gregorian
    calendar format or ISO week calendar format. Gregorian calendar: Use
    any combination of yyyy (or yy for 2-digit years; if so see
    **TIME\_Y2K\_OFFSET\_YEAR**), mm (or o for abbreviated month name in
    the current time language), and dd, with or without delimiters. For
    day-of-year data, use jjj instead of mm and/or dd. As examples, try
    yy/mm/dd, yyyy=jjj, dd-o-yyyy, dd-mm-yy, yy-mm, etc. ISO Calendar:
    Expected template is yyyy[-]W[-]ww[-]d, where ww is ISO week and d
    is ISO week day. Either template must be consistant, e.g., you
    cannot specify months if you do not specify years. As examples, try
    yyyyWww, yy-W-ww-d, etc. If your template starts with a leading
    hyphen (-) then each integer item (y,m,d) will be printed without
    leading zeros (default uses fixed width formats) [yyyy-mm-dd].
**FORMAT\_GEO\_MAP**
    Formatting template that indicates how an output geographical
    coordinate is to be plotted. This template is then used to guide the
    plotting of geographical coordinates in data fields. See
    **FORMAT\_GEO\_OUT** for details. In addition, you can append A
    which plots the absolute value of the coordinate. The default is
    ddd:mm:ss. Not all items may be plotted as this depends on the
    annotation interval.
**FORMAT\_GEO\_OUT**
    Formatting template that indicates how an output geographical
    coordinate is to be formatted. This template is then used to guide
    the writing of geographical coordinates in data fields. The template
    is in general of the form [+\|-]D or [+\|-]ddd[:mm[:ss]][.xxx][F].
    By default, longitudes will be reported in the -180/+180 range. The
    various terms have the following purpose:

     +\ `` `` `` `` Output longitude in the 0 to 360 range [-180/+180]
     -`` `` `` `` Output longitude in the -360 to 0 range [-180/+180]
     D\ `` `` `` `` Use **FORMAT\_FLOAT\_OUT** for floating point degrees.
     ddd\ `` `` `` `` Fixed format integer degrees
     :`` `` `` `` delimiter used
     mm\ `` `` `` `` Fixed format integer arc minutes
     ss\ `` `` `` `` Fixed format integer arc seconds
     F\ `` `` `` `` Encode sign using WESN suffix
     G\ `` `` `` `` Same as F but with a leading space before suffix

    The default is D.

**FORMAT\_FLOAT\_MAP**
    Format (C language printf syntax) to be used when plotting double
    precision floating point numbers on maps. For geographic
    coordinates, see **FORMAT\_GEO\_MAP**. [%.12lg].
**FORMAT\_FLOAT\_OUT**
    Format (C language printf syntax) to be used when printing double
    precision floating point numbers to output files. For geographic
    coordinates, see **FORMAT\_GEO\_OUT**. [%.12lg]. To give some
    columns a separate format, supply one or more comma-separated
    *cols*:*format* specifications, where *cols* can be specific columns
    (e.g., 5 for 6th since 0 is the first) or a range of columns (e.g.,
    3-7). The last specification without column information will
    override the format for all other columns.
**FORMAT\_TIME\_LOGO**
    Defines the format of the time information in the UNIX time stamp.
    This format is parsed by the C function **strftime**, so that
    virtually any text can be used (even not containing any time
    information) [%Y %b %d %H:%M:%S].
**FORMAT\_TIME\_PRIMARY\_MAP**
    Controls how primary month-, week-, and weekday-names are formatted.
    Choose among full, abbreviated, and character. If the leading f, a,
    or c are replaced with F, A, and C the entire annotation will be in
    upper case [full].
**FORMAT\_TIME\_SECONDARY\_MAP**
    Controls how secondary month-, week-, and weekday-names are
    formatted. Choose among full, abbreviated, and character. If the
    leading f, a, or c are replaced with F, A, and C the entire
    annotation will be in upper case [full].
**GMT\_FFT**
    Determines which Fast Fourier Transform should be used among those
    that have been configured during installation. Choose from auto
    (pick the most suitable for the task among available algorithms),
    brenner (GMT’s version of Normal Brenner’s Fortran multi-dimensional
    FFT), fftw (The Fastest Fourier Transform in the West), accelerate
    (Use the Accelerate Framework under OS X), fftpack (use Paul N.
    Swarztrauber FFT pack translated from Fortran) or perflib (Sun
    Performance Library under Solaris) [auto].
**GMT\_HISTORY**
    If true, passes the history of past common command options via the
    hidden .gmtcommands file [true].
**GMT\_INTERPOLANT**
    Determines if linear (linear), Akima’s spline (akima), natural cubic
    spline (cubic) or no interpolation (none) should be used for 1-D
    interpolations in various programs [akima].
**GMT\_TRIANGULATE**
    Determines if we use the **Watson** [Default] or **Shewchuk**
    algorithm (if configured during installation) for triangulation.
    Note that Shewchuk is required for operations involving Voronoi
    constructions.
**GMT\_VERBOSE**
    (**\* -V**) Determines the level of verbosity used by **GMT**
    programs. Choose among 5 levels; each level adds to the verbosity of
    the lower levels: 0 = silence, 1 = fatal errors, 2 = warnings and
    progress reports, 3 = extensive progress reports, 4 = debugging
    messages [1].
**IO\_COL\_SEPARATOR**
    This setting determines what character will separate ASCII output
    data columns written by **GMT**. Choose from tab, space, comma, and
    none [tab].
**IO\_GRIDFILE\_FORMAT**
    Default file format for grids, with optional scale, offset and
    invalid value, written as *ff*/*scale*/*offset*/*invalid*. The
    2-letter format indicator can be one of [**bcnsr**\ ][**bsifd**\ ].
    The first letter indicates native **GMT** binary, old format netCDF,
    COARDS-compliant netCDF, Surfer format or Sun Raster format. The
    second letter stands for byte, short, int, float and double,
    respectively. When /*invalid* is omitted the appropriate value for
    the given format is used (NaN or largest negative). When
    /*scale*/*offset* is omitted, /1.0/0.0 is used. [nf].
**IO\_GRIDFILE\_SHORTHAND**
    If true, all grid file names are examined to see if they use the
    file extension shorthand discussed in Section 4.17 of the **GMT**
    Technical Reference and Cookbook. If false, no filename expansion is
    done [false].
**IO\_HEADER**
    (**\* -H**) Specifies whether input/output ASCII files have header
    record(s) or not [false].
**IO\_LONLAT\_TOGGLE**
    (**\* -:**) Set if the first two columns of input and output files
    contain (latitude,longitude) or (y,x) rather than the expected
    (longitude,latitude) or (x,y). false means we have (x,y) both on
    input and output. true means both input and output should be (y,x).
    IN means only input has (y,x), while OUT means only output should be
    (y,x). [false].
**IO\_N\_HEADER\_RECS**
    Specifies how many header records to expect if **-h** is used [1].
**IO\_NAN\_RECORDS**
    Determines what happens when input records containing NaNs for *x*
    or *y* (and in some cases *z*) are read. Choose between **skip**,
    which will simply report how many bad records were skipped, and
    **pass** [Default], which will pass these records on to the calling
    programs. For most programs this will result in output records with
    NaNs as well, but some will interpret these NaN records to indicate
    gaps in a series; programs may then use that information to detect
    segmentation (if applicable).
**IO\_SEGMENT\_MARKER**
    This holds the character we expect to indicate a segment header in
    an incoming ASCII data or text table [>]. If this marker should be
    different for output then append another character for the output
    segment marker. The two characters must be separated by a comma. Two
    marker characters have special meaning: B means "blank line" and
    will treat blank lines as initiating a new segment, whereas N means
    "NaN record" and will treat records with all NaNs as initiating a
    new segment. If you choose B or N for the output marker then the
    normal GMT segment header is replaced by a blank or NaN record,
    respectively, and no segment header information is written. To use B
    or N as regular segment markers you must escape them with a leading
    backslash.
**MAP\_ANNOT\_MIN\_ANGLE**
    If the angle between the map boundary and the annotation baseline is
    less than this minimum value (in degrees), the annotation is not
    plotted (this may occur for certain oblique projections.) Give a
    value in the range 0-90. [20]
**MAP\_ANNOT\_MIN\_SPACING**
    If an annotation would be plotted less than this minimum distance
    from its closest neighbor, the annotation is not plotted (this may
    occur for certain oblique projections.) [0p]
**MAP\_ANNOT\_OBLIQUE**
    This integer is a sum of 6 bit flags (most of which only are
    relevant for oblique projections): If bit 1 is `set
    (1) <set.1.html>`_ , annotations will occur wherever a gridline
    crosses the map boundaries, else longitudes will be annotated on the
    lower and upper boundaries only, and latitudes will be annotated on
    the left and right boundaries only. If bit 2 is `set
    (2) <set.2.html>`_ , then longitude annotations will be plotted
    horizontally. If bit 3 is `set (4) <set.4.html>`_ , then latitude
    annotations will be plotted horizontally. If bit 4 is `set
    (8) <set.8.html>`_ , then oblique tickmarks are extended to give a
    projection equal to the specified tick length. If bit 5 is set (16),
    tickmarks will be drawn normal to the border regardless of gridline
    angle. If bit 6 is set (32), then latitude annotations will be
    plotted parallel to the border. To set a combination of these, add
    up the values in parentheses. [1].
**MAP\_ANNOT\_OFFSET\_PRIMARY**
    Distance from end of tickmark to start of annotation [5p].
**MAP\_ANNOT\_OFFSET\_SECONDARY**
    Distance from base of primary annotation to the top of the secondary
    annotation [5p] (Only applies to time axes with both primary and
    secondary annotations).
**MAP\_ANNOT\_ORTHO**
    Determines which axes will get their annotations (for linear
    projections) plotted orthogonally to the axes. Combine any **w**,
    **e**, **s**, **n**, **z** (uppercase allowed as well). [we].
**MAP\_DEFAULT\_PEN**
    Sets the default of all pens related to **-W** options. Prepend
    **+** to overrule the color of the parameters
    **MAP\_GRID\_PEN\_PRIMARY**, **MAP\_GRID\_PEN\_SECONDARY**,
    **MAP\_FRAME\_PEN**, **MAP\_TICK\_PEN\_PRIMARY**, and
    **MAP\_TICK\_PEN\_SECONDARY** by the color of **MAP\_DEFAULT\_PEN**
    [default,black].
**MAP\_DEGREE\_SYMBOL**
    Determines what symbol is used to plot the degree symbol on
    geographic map annotations. Choose between ring, degree, colon, or
    none [ring].
**MAP\_FRAME\_AXES**
    Sets which axes to draw and annotate. Combine any uppercase **W**,
    **E**, **S**, **N**, **Z** to draw and annotate west, east, south,
    north and/or vertical (perspective view only) axis. Use lower case
    to draw the axis only, but not annotate. Add an optional **+** to
    draw a cube of axes in perspective view. [WESN].
**MAP\_FRAME\_PEN**
    Pen attributes used to draw plain map frame [thicker,black].
**MAP\_FRAME\_TYPE**
    Choose between **inside**, **plain** and **fancy** (thick boundary,
    alternating black/white frame; append **+** for rounded corners)
    [fancy]. For some map projections (e.g., Oblique Mercator), plain is
    the only option even if fancy is set as default. In general, fancy
    only applies to situations where the projected x and y directions
    parallel the lon and lat directions (e.g., rectangular projections,
    polar projections). For situations where all boundary ticks and
    annotations must be inside the maps (e.g., for preparing geotiffs),
    chose **inside**.
**MAP\_FRAME\_WIDTH**
    Width (> 0) of map borders for fancy map frame [5p].
**MAP\_GRID\_CROSS\_SIZE\_PRIMARY**
    Size (>= 0) of grid cross at lon-lat intersections. 0 means draw
    continuous gridlines instead [0p].
**MAP\_GRID\_CROSS\_SIZE\_SECONDARY**
    Size (>= 0) of grid cross at secondary lon-lat intersections. 0
    means draw continuous gridlines instead [0p].
**MAP\_GRID\_PEN\_PRIMARY**
    Pen attributes used to draw primary grid lines in dpi units or
    points (append p) [default,black].
**MAP\_GRID\_PEN\_SECONDARY**
    Pen attributes used to draw secondary grid lines in dpi units or
    points (append p) [thinner,black].
**MAP\_LABEL\_OFFSET**
    Distance from base of axis annotations to the top of the axis label
    [8p].
**MAP\_LINE\_STEP**
    Determines the maximum length (> 0) of individual straight
    line-segments when drawing arcuate lines [0.75p]
**MAP\_LOGO**
    (**\* -U**) Specifies if a GMT logo with system timestamp should be
    plotted at the lower left corner of the plot [false].
**MAP\_LOGO\_POS**
    (**\* -U**) Sets the justification and the position of the
    logo/timestamp box relative to the current plots lower left corner
    of the plot [BL/-54p/-54p].
**MAP\_ORIGIN\_X**
    (**\* -X**) Sets the x-coordinate of the origin on the paper for a
    new plot [1i]. For an overlay, the default offset is 0i.
**MAP\_ORIGIN\_Y**
    (**\* -Y**) Sets the y-coordinate of the origin on the paper for a
    new plot [1i]. For an overlay, the default offset is 0i.
**MAP\_POLAR\_CAP**
    Controls the appearance of gridlines near the poles for all
    azimuthal projections and a few others in which the geographic poles
    are plotted as points (Lambert Conic, Hammer, Mollweide, Sinusoidal,
    and van der Grinten). Specify either none (in which case there is no
    special handling) or *pc\_lat*/*pc\_dlon*. In that case, normal
    gridlines are only drawn between the latitudes
    -*pc\_lat*/+*pc\_lat*, and above those latitudes the gridlines are
    spaced at the (presumably coarser) *pc\_dlon* interval; the two
    domains are separated by a small circle drawn at the *pc\_lat*
    latitude [85/90]. Note for r-theta (polar) projection where r = 0 is
    at the center of the plot the meaning of the cap is reversed, i.e.,
    the default 85/90 will draw a r = 5 radius circle at the center of
    the map with less frequent radial lines there.
**MAP\_SCALE\_HEIGHT**
    Sets the height (> 0) on the map of the map scale bars drawn by
    various programs [5p].
**MAP\_TICK\_LENGTH\_PRIMARY**
    The length of a primary major/minor tickmarks [5p/2.5p]. If only the
    first value is set, the second is assumed to be 50% of the first.
**MAP\_TICK\_LENGTH\_SECONDARY**
    The length of a secondary major/minor tickmarks [15p/3.75p]. If only
    the first value is set, the second is assumed to be 25% of the
    first.
**MAP\_TICK\_PEN\_PRIMARY**
    Pen attributes to be used for primary tickmarks in dpi units or
    points (append p) [thinner,black].
**MAP\_TICK\_PEN\_SECONDARY**
    Pen attributes to be used for secondary tickmarks in dpi units or
    points (append p) [thinner,black].
**MAP\_TITLE\_OFFSET**
    Distance from top of axis annotations (or axis label, if present) to
    base of plot title [14p].
**MAP\_VECTOR\_SHAPE**
    Determines the shape of the head of a vector. Normally (i.e., for
    vector\_shape = 0), the head will be triangular, but can be changed
    to an `arrow (1) <arrow.1.html>`_ or an open `V (2) <V.2.html>`_ .
    Intermediate settings give something in between. Negative values (up
    to -2) are allowed as well [0].
**PROJ\_ELLIPSOID**
    The (case sensitive) name of the ellipsoid used for the map
    projections [WGS-84]. Choose among:

    Airy: Applies to Great Britain (1830)
     Airy-Ireland: Applies to Ireland in 1965 (1830)
     Andrae: Applies to Denmark and Iceland (1876)
     APL4.9: Appl. Physics (1965)
     ATS77: Average Terrestrial System, Canada Maritime provinces (1977)
     Australian: Applies to Australia (1965)
     Bessel: Applies to Central Europe, Chile, Indonesia (1841)
     Bessel-Namibia: Same as Bessel-Schwazeck (1841)
     Bessel-NGO1948: Modified Bessel for NGO 1948 (1841)
     Bessel-Schwazeck: Applies to Namibia (1841)
     Clarke-1858: Clarke’s early ellipsoid (1858)
     Clarke-1866: Applies to North America, the Philippines (1866)
     Clarke-1866-Michigan: Modified Clarke-1866 for Michigan (1866)
     Clarke-1880: Applies to most of Africa, France (1880)
     Clarke-1880-Arc1950: Modified Clarke-1880 for Arc 1950 (1880)
     Clarke-1880-IGN: Modified Clarke-1880 for IGN (1880)
     Clarke-1880-Jamaica: Modified Clarke-1880 for Jamaica (1880)
     Clarke-1880-Merchich: Modified Clarke-1880 for Merchich (1880)
     Clarke-1880-Palestine: Modified Clarke-1880 for Palestine (1880)
     CPM : Comm. des Poids et Mesures, France (1799)
     Delambre: Applies to Belgium (1810)
     Engelis: Goddard Earth Models (1985)
     Everest-1830: India, Burma, Pakistan, Afghanistan, Thailand (1830)
     Everest-1830-Kalianpur: Modified Everest for Kalianpur (1956) (1830)
     Everest-1830-Kertau: Modified Everest for Kertau, Malaysia & Singapore (1830)
     Everest-1830-Pakistan: Modified Everest for Pakistan (1830)
     Everest-1830-Timbalai: Modified Everest for Timbalai, Sabah Sarawak (1830)
     Fischer-1960: Used by NASA for Mercury program (1960)
     Fischer-1960-SouthAsia: Same as Modified-Fischer-1960 (1960)
     Fischer-1968: Used by NASA for Mercury program (1968)
     FlatEarth: As Sphere, but implies fast "Flat Earth" distance calculations (1984)
     GRS-67: International Geodetic Reference System (1967)
     GRS-80: International Geodetic Reference System (1980)
     Hayford-1909: Same as the International 1924 (1909)
     Helmert-1906: Applies to Egypt (1906)
     Hough: Applies to the Marshall Islands (1960)
     Hughes-1980: Hughes Aircraft Company for DMSP SSM/I grid products (1980)
     IAG-75: International Association of Geodesy (1975)
     Indonesian: Applies to Indonesia (1974)
     International-1924: Worldwide use (1924)
     International-1967: Worldwide use (1967)
     Kaula: From satellite tracking (1961)
     Krassovsky: Used in the (now former) Soviet Union (1940)
     Lerch: For geoid modelling (1979)
     Maupertius: Really old ellipsoid used in France (1738)
     Mercury-1960: Same as Fischer-1960 (1960)
     MERIT-83: United States Naval Observatory (1983)
     Modified-Airy: Same as Airy-Ireland (1830)
     Modified-Fischer-1960: Applies to Singapore (1960)
     Modified-Mercury-1968: Same as Fischer-1968 (1968)
     NWL-10D: Naval Weapons Lab (Same as WGS-72) (1972)
     NWL-9D: Naval Weapons Lab (Same as WGS-66) (1966)
     OSU86F: Ohio State University (1986)
     OSU91A: Ohio State University (1991)
     Plessis: Old ellipsoid used in France (1817)
     SGS-85: Soviet Geodetic System (1985)
     South-American: Applies to South America (1969)
     Sphere: The mean radius in WGS-84 (for spherical/plate tectonics applications) (1984)
     Struve: Friedrich Georg Wilhelm Struve (1860)
     TOPEX: Used commonly for altimetry (1990)
     Walbeck: First least squares solution by Finnish astronomer (1819)
     War-Office: Developed by G. T. McCaw (1926)
     WGS-60: World Geodetic System (1960)
     WGS-66: World Geodetic System (1966)
     WGS-72: World Geodetic System (1972)
     WGS-84: World Geodetic System [Default] (1984)
     Moon: Moon (IAU2000) (2000)
     Mercury: Mercury (IAU2000) (2000)
     Venus: Venus (IAU2000) (2000)
     Mars: Mars (IAU2000) (2000)
     Jupiter: Jupiter (IAU2000) (2000)
     Saturn: Saturn (IAU2000) (2000)
     Uranus: Uranus (IAU2000) (2000)
     Neptune: Neptune (IAU2000) (2000)
     Pluto: Pluto (IAU2000) (2000)

    Note that for some global projections, **GMT** may use a spherical
    approximation of the ellipsoid chosen, setting the flattening to
    zero, and using a mean radius. A warning will be given when this
    happens. If a different ellipsoid name than those mentioned here is
    given, **GMT** will attempt to parse the name to extract the
    semi-major axis (*a* in m) and the flattening. Formats allowed are:

    `` `` `` `` *a*\ `` `` `` `` `` `` `` `` implies a zero flattening
     `` `` `` `` *a*,\ *inv\_f*\ `` `` `` `` where *inv\_f* is the
    inverse flattening
     `` `` `` `` *a*,\ **b=**\ *b*\ `` `` `` `` where *b* is the
    semi-minor axis (in m)
     `` `` `` `` *a*,\ **f=**\ *f*\ `` `` `` `` where *f* is the
    flattening

    This way a custom ellipsoid (e.g., those used for other planets) may
    be used. Further note that coordinate transformations in
    **mapproject** can also specify specific datums; see the
    **mapproject** man page for further details and how to view
    ellipsoid and datum parameters.

**PROJ\_LENGTH\_UNIT**
    Sets the unit length. Choose between **c**\ m, **i**\ nch, or
    **p**\ oint [c (or i)]. Note that, in **GMT**, one point is defined
    as 1/72 inch (the *PostScript* definition), while it is often
    defined as 1/72.27 inch in the typesetting industry. There is no
    universal definition.
**PROJ\_SCALE\_FACTOR**
    Changes the default map scale factor used for the Polar
    Stereographic [0.9996], UTM [0.9996], and Transverse Mercator [1]
    projections in order to minimize areal distortion. Provide a new
    scale-factor or leave as default.
**PS\_CHAR\_ENCODING**
    (static) Names the eight bit character set being used for text in
    files and in command line parameters. This allows **GMT** to ensure
    that the *PostScript* output generates the correct characters on the
    plot.. Choose from Standard, Standard+, ISOLatin1, ISOLatin1+, and
    ISO-8859-x (where x is in the ranges 1-10 or 13-15). See Appendix F
    for details [ISOLatin1+ (or Standard+)].
**PS\_COLOR\_MODEL**
    Determines whether *PostScript* output should use RGB, HSV, CMYK, or
    GRAY when specifying color [rgb]. Note if HSV is selected it does
    not apply to images which in that case uses RGB. When selecting
    GRAY, all colors will be converted to gray scale using YIQ
    (television) conversion.
**PS\_COMMENTS**
    (static) If true we will issue comments in the *PostScript* file
    that explain the logic of operations. These are useful if you need
    to edit the file and make changes; otherwise you can set it to false
    which yields a somewhat slimmer *PostScript* file [false].
**PS\_COPIES**
    (**\* -c**) Number of plot copies to make [1].
**PS\_IMAGE\_COMPRESS**
    (static) Determines if *PostScript* images are compressed using the
    Run-Length Encoding scheme (rle), Lempel-Ziv-Welch compression
    (lzw), or not at all (none) [lzw].
**PS\_LINE\_CAP**
    Determines how the ends of a line segment will be drawn. Choose
    among a *butt* cap (default) where there is no projection beyond the
    end of the path, a *round* cap where a semicircular arc with
    diameter equal to the linewidth is drawn around the end points, and
    *square* cap where a half square of size equal to the linewidth
    extends beyond the end of the path [butt].
**PS\_LINE\_JOIN**
    Determines what happens at kinks in line segments. Choose among a
    *miter* join where the outer edges of the strokes for the two
    segments are extended until they meet at an angle (as in a picture
    frame; if the angle is too acute, a bevel join is used instead, with
    threshold set by **PS\_MITER\_LIMIT**), *round* join where a
    circular arc is used to fill in the cracks at the kinks, and *bevel*
    join which is a miter join that is cut off so kinks are triangular
    in shape [miter].
**PS\_MEDIA**
    Sets the physical format of the current plot paper [a4 (or letter)].
    The following formats (and their widths and heights in points) are
    recognized (Additional site-specific formats may be specified in the
    gmt\_custom\_media.conf file in **$GMT\_SHAREDIR**/conf or ~/.gmt;
    see that file for details):

    Media\ `` `` `` `` width\ `` `` `` `` height
     A0\ `` `` `` `` 2380\ `` `` `` `` 3368
     A1\ `` `` `` `` 1684\ `` `` `` `` 2380
     A2\ `` `` `` `` 1190\ `` `` `` `` 1684
     A3\ `` `` `` `` 842\ `` `` `` `` 1190
     A4\ `` `` `` `` 595\ `` `` `` `` 842
     A5\ `` `` `` `` 421\ `` `` `` `` 595
     A6\ `` `` `` `` 297\ `` `` `` `` 421
     A7\ `` `` `` `` 210\ `` `` `` `` 297
     A8\ `` `` `` `` 148\ `` `` `` `` 210
     A9\ `` `` `` `` 105\ `` `` `` `` 148
     A10\ `` `` `` `` 74\ `` `` `` `` 105
     B0\ `` `` `` `` 2836\ `` `` `` `` 4008
     B1\ `` `` `` `` 2004\ `` `` `` `` 2836
     B2\ `` `` `` `` 1418\ `` `` `` `` 2004
     B3\ `` `` `` `` 1002\ `` `` `` `` 1418
     B4\ `` `` `` `` 709\ `` `` `` `` 1002
     B5\ `` `` `` `` 501\ `` `` `` `` 709
     archA\ `` `` `` `` 648\ `` `` `` `` 864
     archB\ `` `` `` `` 864\ `` `` `` `` 1296
     archC\ `` `` `` `` 1296\ `` `` `` `` 1728
     archD\ `` `` `` `` 1728\ `` `` `` `` 2592
     archE\ `` `` `` `` 2592\ `` `` `` `` 3456
     flsa\ `` `` `` `` 612\ `` `` `` `` 936
     halfletter\ `` `` `` `` 396\ `` `` `` `` 612
     statement\ `` `` `` `` 396\ `` `` `` `` 612
     note\ `` `` `` `` 540\ `` `` `` `` 720
     letter\ `` `` `` `` 612\ `` `` `` `` 792
     legal\ `` `` `` `` 612\ `` `` `` `` 1008
     11x17\ `` `` `` `` 792\ `` `` `` `` 1224
     tabloid\ `` `` `` `` 792\ `` `` `` `` 1224
     ledger\ `` `` `` `` 1224\ `` `` `` `` 792

    For a completely custom format (e.g., for large format plotters) you
    may also specify WxH, where W and H are in points unless you append
    a unit to each dimension (**c**, **i**, **m** or **p** [Default]).

**PS\_MITER\_LIMIT**
    Sets the threshold angle in degrees (integer in 0-180 range) used
    for mitered joins only. When the angle between joining line segments
    is smaller than the threshold the corner will be bevelled instead of
    mitered. The default threshold is 35 degrees. Setting the threshold
    angle to 0 implies the *PostScript* default of about 11 degrees.
    Setting the threshold angle to 180 causes all joins to be beveled.
**PS\_PAGE\_COLOR**
    Sets the color of the imaging background, i.e., the paper [white].
**PS\_PAGE\_ORIENTATION**
    (**\* -P**) Sets the orientation of the page. Choose portrait or
    landscape [landscape].
**PS\_SCALE\_X**
    Global x-scale (> 0) to apply to plot-coordinates before plotting.
    Normally used to shrink the entire output down to fit a specific
    height/width [1.0].
**PS\_SCALE\_Y**
    Global y-scale (> 0) to apply to plot-coordinates before plotting.
    Normally used to shrink the entire output down to fit a specific
    height/width [1.0].
**PS\_TRANSPARENCY**
    Sets the transparency mode to use when preparing PS for rendering to
    PDF. Choose from Color, ColorBurn, ColorDodge, Darken, Difference,
    Exclusion, HardLight, Hue, Lighten, Luminosity, Multiply, Normal,
    Overlay, Saturation, SoftLight, and Screen [Normal].
**TIME\_EPOCH**
    Specifies the value of the calendar and clock at the origin (zero
    point) of relative time units (see **TIME\_UNIT**). It is a string
    of the form yyyy-mm-ddT[hh:mm:ss] (Gregorian) or
    yyyy-Www-ddT[hh:mm:ss] (ISO) Default is 2000-01-01T12:00:00, the
    epoch of the J2000 system.
**TIME\_INTERVAL\_FRACTION**
    Determines if partial intervals at the start and end of an axis
    should be annotated. If the range of the partial interval exceeds
    the specified fraction of the normal interval stride we will place
    the annotation centered on the partial interval [0.5].
**TIME\_IS\_INTERVAL**
    Used when input calendar data should be truncated and adjusted to
    the middle of the relevant interval. In the following discussion,
    the unit **u** can be one of these time units: (**y** year, **o**
    month, **u** ISO week, **d** day, **h** hour, **m** minute, and
    **s** second). **TIME\_IS\_INTERVAL** can have any of the following
    three values: (1) OFF [Default]. No adjustment, time is decoded as
    given. (2) +\ *n*\ **u**. Activate interval adjustment for input by
    truncate to previous whole number of *n* units and then center time
    on the following interval. (3) -*n*\ **u**. Same, but center time on
    the previous interval. For example, with **TIME\_IS\_INTERVAL** =
    +1o, an input data string like 1999-12 will be interpreted to mean
    1999-12-15T12:00:00.0 (exactly middle of December), while if
    **TIME\_IS\_INTERVAL** = off then that date is interpreted to mean
    1999-12-01T00:00:00.0 (start of December) [off].
**TIME\_LANGUAGE**
    Language to use when plotting calendar items such as months and
    days. Select from:
     BR\ `` `` `` `` Brazilian Portuguese
     CN1\ `` `` `` `` Simplified Chinese
     CN2\ `` `` `` `` Traditional Chinese
     DE\ `` `` `` `` German
     DK\ `` `` `` `` Danish
     EH\ `` `` `` `` Basque
     ES\ `` `` `` `` Spanish
     FI\ `` `` `` `` Finnish
     FR\ `` `` `` `` French
     GR\ `` `` `` `` Greek
     HI\ `` `` `` `` Hawaiian
     HU\ `` `` `` `` Hungarian
     IE\ `` `` `` `` Irish
     IL\ `` `` `` `` Hebrew
     IS\ `` `` `` `` Icelandic
     IT\ `` `` `` `` Italian
     JP\ `` `` `` `` Japanese
     NL\ `` `` `` `` Dutch
     NO\ `` `` `` `` Norwegian
     PL\ `` `` `` `` Polish
     PT\ `` `` `` `` Portuguese
     RU\ `` `` `` `` Russian
     SE\ `` `` `` `` Swedish
     SG\ `` `` `` `` Scottish Gaelic
     TO\ `` `` `` `` Tongan
     TR\ `` `` `` `` Turkish
     UK\ `` `` `` `` British English
     US\ `` `` `` `` US English
    If your language is not supported, please examine the
    **$GMT\_SHAREDIR**/time/us.d file and make a similar file. Please
    submit it to the **GMT** Developers for official inclusion. Custom
    language files can be placed in directories **$GMT\_SHAREDIR**/time
    or ~/.gmt. Note: Some of these languages may require you to also
    change the **PS\_CHAR\_ENCODING** setting.
**TIME\_SYSTEM**
    Shorthand for a combination of **TIME\_EPOCH** and **TIME\_UNIT**,
    specifying which time epoch the relative time refers to and what the
    units are. Choose from one of the preset systems below (epoch and
    units are indicated):
     JD\ `` `` `` `` -4713-11-25T12:00:00\ `` `` `` `` d\ `` `` `` ``
    (Julian Date)
     MJD\ `` `` `` `` 1858-11-27T00:00:00\ `` `` `` `` d\ `` `` `` ``
    (Modified Julian Date)
     J2000\ `` `` `` `` 2000-01-01T12:00:00\ `` `` `` `` d\ `` `` `` ``
    (Astronomical time)
     S1985\ `` `` `` `` 1985-01-01T00:00:00\ `` `` `` `` s\ `` `` `` ``
    (Altimetric time)
     UNIX\ `` `` `` `` 1970-01-01T00:00:00\ `` `` `` `` s\ `` `` `` ``
    (UNIX time)
     RD0001\ `` `` `` `` 0001-01-01T00:00:00\ `` `` `` `` s
     RATA\ `` `` `` `` 0000-12-31T00:00:00\ `` `` `` `` d
     This parameter is not stored in the **gmt.conf** file but is
    translated to the respective values of **TIME\_EPOCH** and
    **TIME\_UNIT**.
**TIME\_EPOCH**
    Specifies the epoch for relative time [2000-01-01T12:00:00].
**TIME\_UNIT**
    Specifies the units of relative time data since epoch (see
    **TIME\_EPOCH**). Choose y (year - assumes all years are 365.2425
    days), o (month - assumes all months are of equal length y/12), d
    (day), h (hour), m (minute), or s (second) [d].
**TIME\_WEEK\_START**
    When weeks are indicated on time axes, this parameter determines the
    first day of the week for Gregorian calendars. (The ISO weekly
    calendar always begins weeks with Monday.) [Sunday].
**TIME\_Y2K\_OFFSET\_YEAR**
    When 2-digit years are used to represent 4-digit years (see various
    **FORMAT\_DATE**\ s), **TIME\_Y2K\_OFFSET\_YEAR** gives the first
    year in a 100-year sequence. For example, if
    **TIME\_Y2K\_OFFSET\_YEAR** is 1729, then numbers 29 through 99
    correspond to 1729 through 1799, while numbers 00 through 28
    correspond to 1800 through 1828. [1950].

`See Also <#toc3>`_
-------------------

`*gmt*\ <gmt.html>`_ , `*gmtdefaults*\ <gmtdefaults.html>`_
, `*gmtcolors*\ (5) <gmtcolors.5.html>`_ ,
`*gmtget*\ <gmtget.html>`_ , `*gmtset*\ <gmtset.html>`_

