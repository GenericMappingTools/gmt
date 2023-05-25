.. index:: ! coast
.. include:: module_core_purpose.rst_

*****
coast
*****

|coast_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt coast** |-J|\ *parameters*
|SYN_OPT-R|
[ |SYN_OPT-Area| ]
[ |SYN_OPT-B| ]
[ |-C|\ *fill*\ [**+l**\|\ **r**] ]
[ |-D|\ *resolution*\ [**+f**] ]
[ |-E|\ *dcw* ]
[ |-F|\ *box* ]
[ |-G|\ [*fill*] ]
[ |-I|\ *river*\ [/\ *pen*] ]
[ |-J|\ **z**\|\ **Z**\ *parameters* ]
[ |-L|\ *scalebar* ]
[ |-M| ]
[ |-N|\ *border*\ [/*pen*] ]
[ |-Q| ]
[ |-S|\ [*fill*] ]
[ |-T|\ *rose* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [[*level*/]\ *pen*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Plots grayshaded, colored, or textured land-masses [or
water-masses] on maps and [optionally] draws coastlines, rivers, and
political boundaries. Alternatively, it can (1) issue clip
paths that will contain all land or all water areas, or
(2) dump the data to an ASCII table. The data files come
in 5 different resolutions: (**f**)ull, (**h**)igh, (**i**)ntermediate,
(**l**)ow, and (**c**)rude. The full resolution files amount to more
than 55 Mb of data and provide great detail; for maps of larger
geographical extent it is more economical to use one of the other
resolutions. If the user selects to paint the land-areas and does not
specify fill of water-areas then the latter will be transparent (i.e.,
earlier graphics drawn in those areas will not be overwritten).
Likewise, if the water-areas are painted and no land fill is set then
the land-areas will be transparent. A map projection must be supplied.

Required Arguments
------------------

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-R:

.. |Add_-Rgeo| replace:: Not required when |-E| is used.
.. include:: explain_-Rgeo.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. _-A:

.. |Add_-A| unicode:: 0x20 .. just an invisible code
.. include:: explain_-A.rst_

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *fill*\ [**+l**\|\ **+r**] :ref:`(more ...) <-Gfill_attrib>`
    Set the shade, color, or pattern for lakes and river-lakes [Default
    is the fill chosen for "wet" areas (|-S|)]. Optionally, specify
    separate fills by appending **+l** for lakes or **+r** for
    river-lakes, repeating the |-C| option as needed.

.. _-D:

**-D**\ *resolution*\ [**+f**]
    Select the resolution of the data set to use ((**f**)ull,
    (**h**)igh, (**i**)ntermediate, (**l**)ow, and (**c**)rude). The
    resolution drops off by 80% between data sets.
    Append **+f** to automatically select a lower resolution should the one
    requested not be available [abort if not found].
    Alternatively, choose (**a**)uto to automatically select the best
    resolution given the chosen map scale. [Default is **l** in classic mode
    and **a** in modern mode].

.. _-E:

**-E**\ *code1,code2,...*\ [**+l**\|\ **L**\|\ **n**][**+c**\|\ **C**][**+e**][**+g**\ *fill*][**+p**\ *pen*][**+r**][**+R**][**+z**]
    Select painting, clipping or dumping country polygons from the Digital Chart of the World (DCW).
    This is another dataset independent of GSHHG and hence the |-A| and |-D| options do not apply. The following codes
    are supported:

    - Append one or more comma-separated countries using either the
      `2-character ISO 3166-1 alpha-2 convention <https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2>`_
      (e.g., NO for Norway) or the full country name (e.g., Norway). Append .\ *state* to a country code to select a
      state of a country (if available), e.g., US.TX for Texas.
    - Append =\ *continent* to specify a continent, using either the full names or the abbreviations AF (Africa),
      AN (Antarctica), AS (Asia), EU (Europe), OC (Oceania), NA (North America), or SA (South America).
    - To specify a collection or named region, give either the code or the full name.

    The following modifiers are supported:

    - **+l** to just list the countries and their codes (no data extraction or plotting takes place).
    - **+L** to see states/territories for Argentina, Australia, Brazil, Canada, China, India, Norway, Russia and the US.
    - **+l**\|\ **+L** to **-E**\ =\ *continent* or **-E**\ *code* to only list countries in that continent or country;
      repeat if more than one continent or country is requested.
    - **+n** to list the named :ref:`DCW collections <dcw-collections>` or regions (**-E**\ *code*\ **+n** will list
      collections that contains the listed codes). All names are case-insensitive.
    - **+c** to set up an inside clip path based on your selection.
    - **+C** to set up an outside (area between selection and map boundary) clip path based on your selection.
    - **+p**\ *pen* to draw polygon outlines [Default is no outline].
    - **+e** to adjust the region boundaries to be multiples of the steps indicated by *inc*, *xinc*/*yinc*, or
      *winc*/*einc*/*sinc*/*ninc*, while ensuring that the bounding box is adjusted by at least 0.25 times the
      increment [Default is no adjustment], where *inc* can be positive to expand the region or negative to shrink
      the region.
    - **+g**\ *fill* to fill polygons [Default is no fill].
    - **+r** to adjust the region boundaries to be multiples of the steps indicated by *inc*, *xinc*/*yinc*, or
      *winc*/*einc*/*sinc*/*ninc* [Default is no adjustment]. For example, **-R**\ *FR*\ **+r**\ 1 will select the
      national bounding box of France rounded to nearest integer degree, where *inc* can be positive to expand the
      region or negative to shrink the region.
    - **+R** to adjust the region by adding the amounts specified by *inc*, *xinc*/*yinc*, or
      *winc*/*einc*/*sinc*/*ninc* [Default is no extension], where *inc* can be positive to expand the
      region or negative to shrink the region.
    - **+z** to place the country code in the segment headers via **-Z**\ *code* settings (for use with |-M|).

    One of **+c**\|\ **C**\|\ **g**\|\ **p** must be specified unless |-M| is in effect, in which case only one
    |-E| option can be given. Otherwise, you may repeat |-E| to give different groups of items their own pen/fill
    settings. If neither |-J| nor |-M| are set then we just print the **-R**\ *wesn* string.

.. _-F:

**-F**\ [**l**\|\ **t**][**+c**\ *clearances*][**+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][**+p**\ [*pen*]]\
[**+r**\ [*radius*]][**+s**\ [[*dx*/*dy*/][*shade*]]]

    Without further options, draws a rectangular border around any map scale (|-L|) or map rose (|-T|) using
    :term:`MAP_FRAME_PEN`. Used in combination with |-L| or |-T|.  Append **l** for map scale or **t** for map rose to
    specify which plot embellisment the |-F| parameters should be applied to [default uses the same panel parameters for
    all selected map embellishments]. The following modifiers can be appended to |-F|, with additional explanation and
    examples provided in the :ref:`Background-panel` cookbook section:

    .. include:: explain_-F_box.rst_

.. _-G:

**-G**\ [*fill*] :ref:`(more ...) <-Gfill_attrib>`
    Select filling or clipping of "dry" areas. Append the shade, color,
    or pattern; or give no argument for clipping [Default is no fill].

.. _-I:

**-I**\ *river*\ [/*pen*]
    Draw rivers. Specify the type of rivers and [optionally] append pen
    attributes [Default pen: width = 0.25p, color = black, style =
    solid].

    Choose from the list of river types below; repeat option |-I| as
    often as necessary.

    - 0 = Double-lined rivers (river-lakes)
    - 1 = Permanent major rivers
    - 2 = Additional major rivers
    - 3 = Additional rivers
    - 4 = Minor rivers
    - 5 = Intermittent rivers - major
    - 6 = Intermittent rivers - additional
    - 7 = Intermittent rivers - minor
    - 8 = Major canals
    - 9 = Minor canals
    - 10 = Irrigation canals

    You can also choose from several preconfigured river groups:

    - a = All rivers and canals (0-10)
    - A = All rivers and canals except river-lakes (1-10)
    - r = All permanent rivers (0-4)
    - R = All permanent rivers except river-lakes (1-4)
    - i = All intermittent rivers (5-7)
    - c = All canals (8-10)

.. _-L:

.. include:: explain_-L_scale.rst_

.. _-M:

**-M**
    Dump a single multisegment ASCII (or binary, see
    **-bo**) file to standard output. No plotting
    occurs. Specify one of |-E|, |-I|, |-N| or |-W|.
    **Note**: If |-M| is used with |-E| then |-R| or the **+r** modifier
    to |-E| are not required as we automatically determine the region
    given the selected geographic entities.  If using |-W| and you want
    just certain levels (1-4) then use the full syntax **-W**\ *level*/\ *pen*
    and repeat for each level (pen is not used but required to parse the level correctly).

.. _-N:

**-N**\ *border*\ [/*pen*]
    Draw political boundaries. Specify the type of boundary and
    [optionally] append pen attributes [Default pen: width = 0.25p,
    color = black, style = solid].

    Choose from the list of boundaries below. Repeat option |-N| as
    often as necessary.

    - 1 = National boundaries
    - 2 = State boundaries within the Americas
    - 3 = Marine boundaries
    - a = All boundaries (1-3)

.. _-Q:

**-Q**
    Mark end of existing clip path. No projection information is needed.

.. _-S:

**-S**\ [*fill*] :ref:`(more ...) <-Gfill_attrib>`
    Select filling or clipping of "wet" areas. Append the shade, color,
    or pattern; or give no argument for clipping [Default is no fill].

.. _-T:

..  include:: explain_-T_rose.rst_

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [[*level*/]\ *pen*] :ref:`(more ...) <set-pens>`
    Draw shorelines [Default is no shorelines]. Append pen attributes
    [Defaults: width = 0.25p, color = black, style = solid] which
    apply to all four levels. To set the pen for each level differently,
    prepend *level*/, where *level* is 1-4 and represent coastline,
    lakeshore, island-in-lake shore, and lake-in-island-in-lake shore.
    Repeat |-W| as needed. When specific level pens are set, those not
    listed will not be drawn [Default draws all levels; but see |-A|].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. module_common_ends

.. _dcw-collections:

DCW Collections
---------------

The |-E| and |-R| options can be expanded to take the user's own custom collections
and named regions.  Users can create a dcw.conf file and place it in their
GMT user directory (typically ~/.gmt).  The format of the file is the same
as the dcw-collections.txt file distributed with DCW 2.1.0 or later::

    # ~/.gmt/dcw.conf
    # Arbitrary comments and blank lines anywhere

    # The France-Italian union (2042-45) of gallery example 34.
    tag: FRIT Franco-Italian Union
    list: FR,IT
    # Stay away from those dangerous eels!
    tag: SARG Sargasso Sea
    region: 70W/40W/20N/35N

* Each *tag:* record must be immediately followed by either a *list:* (e.g. *list: FR,IT*, no space between codes)
  or *region:* (e.g. *region: 70W/40W/20N/35N*) record.
* All tags should be at least 3 characters long.
  Either the *tag* or the *name* (if available) can be used to make selections in |-R| or |-E|.
  Use quotes if *name* consists of more than one word (e.g. *"Franco-Italian Union"*).
* The **-E+n** option wil list the contents of the collection distributed with DCW as well
  as any contents in ~/.gmt/dcw.conf. The latter file is consulted first and can be used to
  override same-name tag selections in the system DCW file.

Examples
--------

.. include:: oneliner_info.rst_

To plot a green Africa with white outline on blue background, with
permanent major rivers in thick blue pen, additional major rivers in
thin blue pen, and national borders as dashed lines on a Mercator map at
scale 0.1 inch/degree, use::

    gmt coast -R-30/30/-40/40 -Jm0.1i -B5 -I1/1p,blue -N1/0.25p,- \
                -I2/0.25p,blue -W0.25p,white -Ggreen -Sblue -pdf africa

To plot Iceland using the lava pattern (# 28) at 100 dots per inch, on a
Mercator map at scale 1 cm/degree, run::

    gmt coast -RIS+r1 -Jm1c -B -Wthin -Gp28+r100 -pdf iceland

To initiate a clip path for Africa so that the subsequent colorimage of
gridded topography is only seen over land, using a Mercator map at scale
0.1 inch/degree, use::

    gmt begin
      gmt coast -R-30/30/-40/40 -Jm0.1i -B -G
      gmt grdimage @earth_relief_05m
      gmt coast -Q
    gmt end show

To plot Great Britain, Italy, and France in blue with a red outline and
Spain, Portugal and Greece in yellow (no outline), and pick up the plot
domain from the extents of these countries, use::

    gmt coast -JM6i -Baf -EGB,IT,FR+gblue+p0.25p,red -EES,PT,GR+gyellow -pdf map

To extract a high-resolution coastline data table for Iceland to be used
in your analysis, try::

    gmt coast -RIS -Dh -W -M > iceland.txt

To lay down a clip path around France that will clip your later plotting
until you end the clipping with clip -C, try::

    gmt coast -R-10/10/40/52 -JM15c -E+c

**coast** will first look for coastline files in directory
**$GMT_SHAREDIR**/coast If the desired file is not found, it will look
for the file **$GMT_SHAREDIR**/coastline.conf. This file may contain
any number of records that each holds the full pathname of an
alternative directory. Comment lines (#) and blank lines are allowed.
The desired file is then sought for in the alternate directories.

.. include:: explain_gshhg.rst_

.. module_note_begins

Bugs
----

The options to fill (|-C| |-G| |-S|) may not always work if the
Azimuthal equidistant projection is chosen (**-Je**\|\ **E**). If the
antipole of the projection is in the oceans it will most likely work. If
not, try to avoid using projection center coordinates that are even
multiples of the coastline bin size (1, 2, 5, 10, and 20 degrees for
**f**, **h**, **i**, **l**, **c**, respectively). This projection is not
supported for clipping.

The political borders are for the most part 1970s-style but have been
updated to reflect more recent border rearrangements in Europe and
elsewhere. Let us know if you find something out of date.

The full-resolution coastlines are also from a digitizing effort in the
1970-80s and it is difficult to assess the accuracy. Users who zoom in
close enough may find that the GSHHG coastline is not matching other
data, e.g., satellite images, more recent coastline data, etc. We are
aware of such mismatches but cannot undertake band-aid solutions each
time this occurs.

Some users will not be satisfied with what they find for
the Antarctic shoreline. In Antarctica, the boundary between ice and
ocean varies seasonally and inter-annually. There are some areas of
permanent shelf ice. In addition to these time-varying ice-ocean
boundaries, there are also shelf ice grounding lines where ice goes from
floating on the sea to sitting on land, and lines delimiting areas of
rock outcrop. For consistency's sake, we have used the World Vector
Shoreline throughout the world, as described in
`The Global Self-consistent, Hierarchical, High-resolution Geography Database (GSHHG) <https://github.com/GenericMappingTools/gshhg-gmt#readme>`_.
Users who need specific boundaries in Antarctica
should get the Antarctic Digital Database, prepared by the British
Antarctic Survey, Scott Polar Research Institute, World Conservation
Monitoring Centre, under the auspices of the Scientific Committee on
Antarctic Research. This data base contains various kinds of limiting
lines for Antarctica and is available on CD-ROM. It is published by the
Scientific Committee on Antarctic Research, Scott Polar Research
Institute, Lensfield Road, Cambridge CB2 1ER, United Kingdom.

.. module_note_ends

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`grdlandmask`,
:doc:`basemap`
