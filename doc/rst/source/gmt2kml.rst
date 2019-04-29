.. index:: ! gmt2kml

*******
gmt2kml
*******

.. only:: not man

    Convert GMT data tables to KML files for Google Earth

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt 2kml** [ *table* ]
[ |-A|\ **a**\ \|\ **g**\ \|\ **s**\ [*alt*\ \|\ **x**\ *scale*] ]
[ |-C|\ *cpt* ] [ |-D|\ *descriptfile* ]
[ |-E|\ [**+e**\ ][**+s**\ ] ]
[ |-F|\ **e**\ \|\ **s**\ \|\ **t**\ \|\ **l**\ \|\ **p**\ \|\ **w** ]
[ |-G|\ [*color*\ ]\ [**+f**\ \|\ **+n**\ ] ]
[ |-I|\ *icon* ] [ **-K**]
[ |-L|\ *col1:name1*,\ *col2:name2*,... ]
[ |-N|\ [**t**\ \|\ *col* \ \|\ *name\_template*\ \|\ *name*] ]
[ **-O**]
[ |-Q|\ **a**\ \|\ **i**\ *az* ]
[ |-Q|\ **s**\ *scale*\ [*unit*\ ] ]
[ |-R|\ **e**\ \|\ *w/e/s/n* ]
[ |-S|\ **c**\ \|\ **n**\ *scale*] ]
[ |-T|\ *title*\ [/*foldername*] ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][*attr*] ]
[ |-Z|\ *args* ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]
[ > *output.kml* ]

|No-spaces|

Description
-----------

**2kml** reads one or more GMT table file and converts them to a
single output file using Google Earth's KML format. Data may represent
points, lines, polygons, or wiggles, and you may specify additional attributes
such as title, altitude mode, colors, pen widths, transparency, regions,
and data descriptions. You may also extend the feature down to ground
level (assuming it is above it) and use custom icons for point symbols.
Finally, there are controls on visibility depending on level of detail
settings, altitude, regions, including the status upon loading into
Google Earth as well as fading depending on zoom.

The input files should contain the following columns:

*lon* *lat* [ *alt* ] [ *timestart* [ *timestop* ] ]

where *lon* and *lat* are required for all features, *alt* is optional
for all features (see also **-A** and **-C**), and *timestart* and
*timestop* apply to events and timespan features.   For wiggles,
the *alt* column is required but is expected to represent an along-track
data anomaly such as gravity, magnetics, etc.  These values will be
scaled to yield distances from the line in degrees.

Required Arguments
------------------

None.

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

**-A**\ **a**\ \|\ **g**\ \|\ **s**\ [*alt*\ \|\ **x**\ *scale*]
    Select one of three altitude modes recognized by Google Earth that
    determines the altitude (in m) of the feature: **a** absolute
    altitude, **g** altitude relative to sea surface or ground, **s**
    altitude relative to seafloor or ground. To plot the features at a
    fixed altitude, append an altitude *alt* (in m). Use 0 to clamp the
    features to the chosen reference surface. Append **x**\ *scale* to
    scale the altitude from the input file by that factor. If no value
    is appended, the altitude (in m) is read from the 3rd column of the
    input file. [By default the features are clamped to the sea surface or ground].

.. _-C:

**-C**\ *cpt*
    Use the CPT for assigning colors to the symbol, event, or
    timespan icons, based on the value in the 3rd column of the input
    file. For lines or polygons we examine the segment header for
    -Z<value> statements and obtain the color via the cpt lookup. Note
    only discrete colors are possible.

.. _-D:

**-D**\ *descriptfile*
    File with HTML snippets that will be included as part of the main
    description content for the KML file [no description]. See SEGMENT
    INFORMATION below for feature-specific descriptions.

.. _-E:

**-E**\ [**+e**\ ][**+s**\ ]
    Control how lines and polygons are rendered in Google Earth.
    Append **+e** to extrude the feature down to ground level [no extrusion].
    Append **+s** to connect points with straight lines (which may intersect
    the Earth's surface and be invisible) [tessellate onto surface].

.. _-F:

**-F**\ **e**\ \|\ **s**\ \|\ **t**\ \|\ **l**\ \|\ **p**\ \|\ **w**
    Sets the feature type. Choose from points (**e**\ vent,
    **s**\ ymbol, or **t**\ imespan), **l**\ ine, **p**\ olygon, or
    **w**\ iggle [symbol]. The first two columns of the input file should contain
    (*lon*, *lat*). When altitude or value is required (i.e., no
    *altitude* value was given with **-A**, or **-C** is set), the third
    column needs to contain the *altitude* (in m) or *value*. The event
    (**-Fe**) is a symbol that should only be active at a particular
    *time*, given in the next column. Timespan (**-Ft**) is a symbol
    that should only be active during a particular time period indicated
    by the next two columns (*timestart*, *timestop*). Use NaN to
    indicate unbounded time limits. If used, times should be in ISO
    format yyyy-mm-ddThh:mm:ss[.xxx] or in GMT relative time format
    (see **-f**).  For wiggles, the data anomaly is required to be
    in the 3rd input column.  If you also need to plot the track itself
    then do that separately with **-Fl**.

.. _-G:

**-G**\ [*color*\ ]\ [**+f**\ \|\ **+n**\ ]
    Sets *color* for fill (modifier **+f** [Default]) or label font (modifier **+n**).
    Fill sets infill color for symbols, extrusions, polygons and positive anomaly
    wiggles [Default is light orange at 75% transparency].  Alternatively,
    use **-G+f** to turn off such infill.
    Text labels: Specify *color* for the font [Default is white]. Alternatively,
    use **-G+n** to instead disable the labels.

.. _-I:

**-I**\ *icon*
    Specify the URL to an alternative icon that should be used for the
    symbol [Default is a Google Earth circle]. If the URL starts with +
    then we will prepend
    `http://maps.google.com/mapfiles/kml/ <http://maps.google.com/mapfiles/kml/>`_
    to the name. To turn off icons entirely (e.g., when just wanting a
    text label), use **-I**-. [Default is a local icon with no directory path].

.. _-K:

**-K**
    Allow more KML code to be appended to the output later [finalize the KML file].

**-L**\ *name1*,\ *name2*,...
    Extended data given. Append one or more column names separated by
    commas. We will expect the listed data columns to exist in the input
    immediately following the data coordinates required for the selected
    feature set by **-F**, and they will be encoded
    in the KML file as Extended Data sets, whose attributes will be
    available in a Google Earth balloon when the item is selected.
    The data file must have enough data columns and trialing text to
    accommodate the number of columns requested.  If the number of extended
    data is one larger than the number of available numerical columns then
    the entire trailing text is set as the last extended data column.
    Otherwise, the trailing text is split into individual words and
    set as separate extended columns.

.. _-N:

**-N**\ [**t**\ \|\ *col* \ \|\ *name\_template*\ \|\ *name*]
    By default, if segment headers contain a **-L**"label string" then
    we use that for the name of the KML feature (polygon, line segment
    or set of symbols). Default names for these segments are "Line %d"
    and "Point Set %d", depending on the feature, where %d is a sequence
    number of line segments within a file. Each point within a line
    segment will be named after the line segment plus a sequence number.
    Default is simply "Point %d".
    Alternatively, select one of these options: (1) append *col* to
    supply individual symbol labels as the string formatted from the *col*
    data column, (2) append **t** to let individual symbol labels
    be the trailing text of each record (3) append a string that may
    include %d or a similar integer format to assign unique name IDs for
    each feature, with the segment number (for lines and polygons) or
    point number (symbols) appearing where %d is placed, (4) give no
    arguments to turn symbol labeling off; line segments will still be
    named. Also note that options (2) is not available unless input is an ASCII file.

.. _-O:

**-O**
    Append KML code to an existing KML file [initialize a new KML file].

.. _-Q:

**-Qa**\ \|\ **i**\ *azimuth*
    Option in support of wiggle plots (requires **-Fw**). You may
    control which directions the positive wiggles will tend to point
    to with **-Qa**.  The provided azimuth defines a half-circle
    centered on the selected azimuth [0] where positive anomalies
    will plot.  If outside then switch by 180 degrees.  Alternatively,
    use **-Qi** to set a fixed direction with no further variation.

**-Qs**\ *scale*\ [*unit*\ ]
    Required setting for wiggle plots (i.e., it requires **-Fw**).
    Sets a wiggle scale in *z*-data units per the user's units (given
    via the trailing unit taken from d|m|s|e|f|k|M|n|u [e]). This scale
    is then inverted to yield degrees per user z-unit and used to
    convert wiggle anomalies to map distances and positions.

.. _-R:

**-Re**\ \|\ *w/e/s/n*
    Issue a single Region tag. Append *w/e/s/n* to set a particular
    region (will ignore points outside the region), or append **e** to
    determine and use the exact domain of the data (single file only)
    [no region tags issued].

.. _-S:

**-S**\ **c**\ \|\ **n**\ *scale*]
    Scale icons or labels. Here, **-Sc** sets a scale for the symbol
    icon, whereas **-Sn** sets a scale for the name labels [1 for both].

.. _-T:

**-T**\ *title*\ [/*foldername*]
    Sets the document title [default is unset]. Optionally, append
    /*FolderName*; this allows you, with **-O**, **-K**, to group
    features into folders within the KML document. [The default folder
    name is "*Name* Features", where *Name* is Point, Event, Timespan,
    Line, Polygon or Wiggle].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    Set pen attributes for lines, wiggles or polygon outlines. Append pen
    attributes to use [Defaults: width = default, color = black, style =
    solid]. If the modifier **+cl** is appended then the color of the line
    are taken from the CPT (see **-C**). If instead modifier **+cf** is
    appended then the color from the cpt file is applied to symbol fill.
    Use just **+c** for both effects.  Note that for KML the pen width is
    given in (fractional) pixels and not in points (1/72 inch).

.. _-Z:

**-Z**\ *args*
    Set one or more attributes of the Document and Region tags. Append
    **+a**\ *alt\_min/alt\_max* to specify limits on visibility based on
    altitude. Append **+l**\ *lod\_min/lod\_max* to specify limits on
    visibility based on Level Of Detail, where *lod\_max* == -1 means it
    is visible to infinite size. Append **+f**\ *fade\_min/fade\_max* to
    fade in and out over a ramp [abrupt]. Append **+v** to make a
    feature *not* visible when loaded [visible]. Append **+o** to open a
    folder or document in the sidebar when loaded [closed]. 

.. |Add_-bi| replace:: [Default is 2 or more input columns, depending on settings]. 
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_


Examples
--------

To convert a file with point locations (lon, lat) into a KML file with
red circle symbols, try

   ::

    gmt 2kml mypoints.txt -Gred+f -Fs > mypoints.kml

To convert a multisegment file with lines (lon, lat) separated by
segment headers that contain a **-L**\ labelstring with the feature
name, selecting a thick white pen, and title the document, try

   ::

    gmt 2kml mylines.txt -Wthick,white -Fl -T"Lines from here to there" > mylines.kml

To convert a multisegment file with polygons (lon, lat) separated by
segment headers that contain a **-L**\ labelstring with the feature
name, selecting a thick black pen and semi-transparent yellow fill,
giving a title to the document, and prescribing a particular region
limit, try

   ::

    gmt 2kml mypolygons.txt -Gyellow@50+f -Fp -T"My polygons" -R30/90/-20/40 > mypolygons.kml

To convert a file with point locations (lon, lat, time) into a KML file
with green circle symbols that will go active at the specified time and
stay active going forward, try

   ::

    awk '{print $1, $2, $3, "NaN"}' mypoints.txt | gmt 2kml -Ggreen+f -Ft > mytimepoints.kml

To extract contours and labels every 10 units from the grid temp.nc and
plot them in KML, using red lines at 75% transparency and red labels (no
transparency), try

   ::

    gmt grdcontour temp.nc -Jx1id -A10+tlabel.txt -C10 -Dcontours.txt
    gmt 2kml    contours.txt -Fl -W1p,red@75 -K > contours.kml
    gmt 2kml    -O -Nt -Fs -Sn2 -Gred@0+n label.txt -I- >> contours.kml

To instead plot the contours as lines with colors taken from the cpt
file contours.cpt, try

   ::

    gmt 2kml contours.txt -Fl -Ccontours.cpt > contours.kml

To plot magnetic anomalies as wiggles along track, with positive
wiggles painted orange and the wiggle line drawn with a black pen
of width 2p, scaling the magnetic anomalies (in nTesla) so that
50 nT equals 1 nm on the map, and place the wiggles 50m above the
sea surface, use

   ::

    gmt 2kml magnetics_lon_lat_mag.txt -Fw -Gorange+f -W2p -Ag50 -Qs50n > wiggles.kml

Limitations
-----------

Google Earth has trouble displaying filled polygons across the Dateline.
For now you must manually break any polygon crossing the dateline into a
west and east polygon and plot them separately.  Google Earth also has
other less obvious limitations on file size or line length.  These do
not seem to be documented.  If features do not show and you are not
getting an error, try to reduce the size of the file by splitting
things up.

Making Kmz Files
----------------

Using the KMZ format is preferred as it takes less space. KMZ is simply
a KML file and any data files, icons, or images referenced by the KML,
contained in a zip archive. One way to organize large data sets is to
split them into groups called Folders. A Document can contain any number
of folders. Using scripts you can create a composite KML file using the
**-K**, **-O** options just like you do with GMT plots. See **-T** for
switching between folders and documents.  The gmt_shell_scripts.sh
contains function gmt_build_kmz that can assist in building a KMZ file
from any number of KML files (and optionally images they may refer to).

Kml Hierarchy
-------------

GMT stores the different features in hierarchical folders by feature
type (when using **-O**, **-K** or **-T/**\ *foldername*), by input file
(if not standard input), and by line segment (using the name from the
segment header, or **-N**). This makes it more easy in Google Earth to
switch on or off parts of the contents of the Document. The following is
a crude example:

[ KML header information; not present if **-O** was used ]

<Document><name>GMT Data Document</name>

<Folder><name>Point Features</name>

<!--This level of folder is inserted only when using -O, -K>

<Folder><name>file1.dat</name>

<!--One folder for each input file (not when standard input)>

<Folder><name>Point Set 0</name>

<!--One folder per line segment>

<!--Points from the first line segment in file file1.dat go here>

<Folder><name>Point Set 1</name>

<!--Points from the second line segment in file file1.dat go here>

</Folder>

</Folder>

<Folder><name>Line Features</name>

<Folder><name>file1.dat</name>

<!--One folder for each input file (not when standard input)>

<Placemark><name>Line 0</name>

<!--Here goes the first line segment>

</Placemark>

<Placemark><name>Line 1</name>

<!--Here goes the second line segment>

</Placemark>

</Folder>

<Folder>

</Document>

[ KML trailer information; not present if **-K** was used ]

Segment Information
-------------------

**2kml** will scan the segment headers for substrings of the form
**-L**"*some label*\ " [also see **-N** discussion] and **-T**"*some
text description*\ ". If present, these are parsed to supply name and
description tags, respectively, for the current feature.

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

:doc:`gmt` ,
:doc:`gmt.conf`,
:doc:`gmt_shell_functions.sh`,
:doc:`grd2kml`,
:doc:`img2google <supplements/img/img2google>`,
:doc:`kml2gmt` ,
:doc:`psconvert`
