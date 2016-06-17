.. index:: ! gmt2kml

*******
gmt2kml
*******

.. only:: not man

    gmt2kml - Convert GMT data tables to KML files for Google Earth

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt2kml** [ *table* ]
[ |-A|\ **a**\ \|\ **g**\ \|\ **s**\ [*alt*\ \|\ **x**\ *scale*] ]
[ |-C|\ *cpt* ] [ |-D|\ *descriptfile* ] [ |-E|\ [*altitude*] ]
[ |-F|\ **e**\ \|\ **s**\ \|\ **t**\ \|\ **l**\ \|\ **p** ]
[ |-G|\ **f\|n**\ **-**\ \|\ *fill* ]
[ |-I|\ *icon* ] [ **-K**]
[ |-L|\ *col1:name1*,\ *col2:name2*,... ]
[ |-N|\ [+\|*name\_template*\ \|\ *name*] ] [ **-O**]
[ |-R|\ **a**\ \|\ *w/e/s/n* ]
[ |-S|\ **c**\ \|\ **n**\ *scale*] ]
[ |-T|\ *title*\ [/*foldername*] ]
[ |SYN_OPT-V| ]
[ |-W|\ [**-**\ \|\ **+**]\ *pen* ]
[ |-Z|\ *args* ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]
[ > *output.kml* ]

|No-spaces|

Description
-----------

**gmt2kml** reads one or more GMT table file and converts them to a
single output file using Google Earth's KML format. Data may represent
points, lines, or polygons, and you may specify additional attributes
such as title, altitude mode, colors, pen widths, transparency, regions,
and data descriptions. You may also extend the feature down to ground
level (assuming it is above it) and use custom icons for point symbols.

The input files should contain the following columns:

*lon* *lat* [ *alt* ] [ *timestart* [ *timestop* ] ]

where *lon* and *lat* are required for all features, *alt* is optional
for all features (see also **-A** and **-C**), and *timestart* and
*timestop* apply to events and timespan features. 

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

**-E**\ [*altitude*]
    Extrude feature down to ground level [no extrusion].

.. _-F:

**-F**\ **e**\ \|\ **s**\ \|\ **t**\ \|\ **l**\ \|\ **p**
    Sets the feature type. Choose from points (**e**\ vent,
    **s**\ ymbol, or **t**\ imespan), **l**\ ine, or **p**\ olygon
    [symbol]. The first two columns of the input file should contain
    (*lon*, *lat*). When altitude or value is required (i.e., no
    *altitude* value was given with **-A**, or **-C** is set), the third
    column needs to contain the *altitude* (in m) or *value*. The event
    (**-Fe**) is a symbol that should only be active at a particular
    *time*, given in the next column. Timespan (**-Ft**) is a symbol
    that should only be active during a particular time period indicated
    by the next two columns (*timestart*, *timestop*). Use NaN to
    indicate unbounded time limits. If used, times should be in ISO
    format yyyy-mm-ddThh:mm:ss[.xxx] or in GMT relative time format (see **-f**).

.. _-G:

**-G**\ **f\|n**\ *fill*
    Set fill color for symbols, extrusions and polygons (**-Gf**)
    [Default is light orange at 75% transparency] or text labels
    (**-Gn**) [Default is white]. Optionally, use **-Gf-** to turn off
    polygon fill, and **-Gn-** to disable labels.

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

.. _-L:

**-L**\ *name1*,\ *name2*,...
    Extended data given. Append one or more column names separated by
    commas. We will expect the listed data columns to exist in the input
    immediately following the data coordinates and they will be encoded
    in the KML file as Extended Data sets, whose attributes will be
    available in the Google Earth balloon when the item is selected.
    This option is not available unless input is an ASCII file.

.. _-N:

**-N**\ [-\|+\|\ *name\_template*\ \|\ *name*]
    By default, if segment headers contain a **-L**"label string" then
    we use that for the name of the KML feature (polygon, line segment
    or set of symbols). Default names for these segments are "Line %d"
    and "Point Set %d", depending on the feature, where %d is a sequence
    number of line segments within a file. Each point within a line
    segment will be named after the line segment plus a sequence number.
    Default is simply "Point %d".
    Alternatively, select one of these options: (1) append **-** to
    supply individual symbol labels (single word) via the field
    immediately following the data coordinates, (2) append **+** to
    supply individual symbol labels as the rest the end of the data
    record following the data coordinates, (3) append a string that may
    include %d or a similar integer format to assign unique name IDs for
    each feature, with the segment number (for lines and polygons) or
    point number (symbols) appearing where %d is placed, (4) give no
    arguments to turn symbol labeling off; line segments will still be
    named. Note: if **-N-** is used with **-L** then the label must
    appear before the extended data columns.  Also note that options
    (1) and (2) are not available unless input is an ASCII file.

.. _-O:

**-O**
    Appended KML code to an existing KML file [initialize a new KML file].

.. _-R:

**-Ra**\ \|\ *w/e/s/n*
    Issue a single Region tag. Append *w/e/s/n* to set a particular
    region (will ignore points outside the region), or append **a** to
    determine and use the actual domain of the data (single file only)
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
    Line, or Polygon].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**-**\ \|\ **+**]\ *pen*
    Set pen attributes for lines or polygon outlines. Append pen
    attributes to use [Defaults: width = default, color = black, style =
    solid]. If **-C** is given you may optionally use **-W-** to apply
    the cpt color to the polygon outline only (fill determined by
    **-G**) or **-W+** to use the cpt color for both polygon fill and
    outline. Note that for KML the pen width is given as integer pixel
    widths so you must specify pen width as *n*\ **p**, where *n* is an
    integer.

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

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

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

    gmt2kml mypoints.txt -Gfred -Fs > mypoints.kml

To convert a multisegment file with lines (lon, lat) separated by
segment headers that contain a **-L**\ labelstring with the feature
name, selecting a thick white pen, and title the document, try

   ::

    gmt2kml mylines.txt -Wthick,white -Fl -T"Lines from here to there" > mylines.kml

To convert a multisegment file with polygons (lon, lat) separated by
segment headers that contain a **-L**\ labelstring with the feature
name, selecting a thick black pen and semi-transparent yellow fill,
giving a title to the document, and prescribing a particular region
limit, try

   ::

    gmt gmt2kml mypolygons.txt -Gfyellow@50 -Fp -T"My polygons" -R30/90/-20/40 > mypolygons.kml

To convert a file with point locations (lon, lat, time) into a KML file
with green circle symbols that will go active at the specified time and
stay active going forward, try

   ::

    awk '{print $1, $2, $3, "NaN"}' mypoints.txt | gmt gmt2kml -Gfgreen -Ft > mytimepoints.kml

To extract contours and labels every 10 units from the grid temp.nc and
plot them in KML, using red lines at 75% transparency and red labels (no
transparency), try

   ::

    gmt grdcontour temp.nc -Jx1id -A10+tlabel.txt -C10 -Dcontours.txt
    gmt gmt2kml    contours.txt -Fl -W1p,red@75 -K > contours.kml
    gmt gmt2kml    -O -N+ -Fs -Sn2 -Gnred@0 label.txt -I- >> contours.kml

To instead plot the contours as lines with colors taken from the cpt
file contours.cpt, try

   ::

    gmt gmt2kml contours.txt -Fl -Ccontours.cpt > contours.kml

Limitations
-----------

Google Earth has trouble displaying filled polygons across the Dateline.
For now you must manually break any polygon crossing the dateline into a
west and east polygon and plot them separately.

Making Kmz Files
----------------

Using the KMZ format is preferred as it takes less space. KMZ is simply
a KML file and any data files, icons, or images referenced by the KML,
contained in a zip archive. One way to organize large data sets is to
split them into groups called Folders. A Document can contain any number
of folders. Using scripts you can create a composite KML file using the
**-K**, **-O** options just like you do with GMT plots. See **-T** for
switching between folders and documents.

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

**gmt2kml** will scan the segment headers for substrings of the form
**-L**"*some label*\ " [also see **-N** discussion] and **-T**"*some
text description*\ ". If present, these are parsed to supply name and
description tags, respectively, for the current feature.

See Also
--------

:doc:`gmt` , :doc:`gmt.conf`,
:doc:`img2google <supplements/img/img2google>`,
:doc:`kml2gmt` , :doc:`psconvert`
