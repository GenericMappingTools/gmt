.. index:: ! mgd77track

**********
mgd77track
**********

.. only:: not man

    mgd77track - Plot track-line map of MGD77 cruises

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77track** *NGDC-ids*
|SYN_OPT-R|
|-J|\ *parameters*
[ |-A|\ [**c**][*size*][**+i**\ *spacing*] ]
[ |SYN_OPT-B| ]
[ |-C|\ **f**\ \|\ **g**\ \|\ **e** ] [ **-Da**\ *startdate* ] 
[ |-D|\ **b**\ *stopdate* ]
[ |-F| ]
[ |-G|\ **d**\ \|\ **t**\ \|\ **n**\ *gap* ]
[ |-I|\ *ignore* ]
[ |-L|\ *trackticks* ]
[ |-S|\ **a**\ *startdist*\ [**u**] ]
[ |-S|\ **b**\ *stopdist*\ [**u**] ]
[ |-T|\ **T**\ \|\ **t**\ \|\ **d**\ *ms*,\ *mc*,\ *mfs*,\ *mf*,\ *mfc* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [**-**\ \|\ **+**][*pen*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mgd77track** reads NGDC MGD77 cruises and creates PostScript code
that will plot one or more ship tracks on a map using the specified
projection. The PostScript code is written to standard output.

Required Arguments
------------------

.. include:: explain_ncid.rst_

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-J.rst_
 
.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [**c**][*size*][**+i**\ *spacing*]
    Append **c** to annotate using the MGD77 cruise ID [Default uses the
    filename prefix]. Optional *size* is the font size in points. The
    leg annotation font is controlled by :ref:`FONT_LABEL <FONT_LABEL>`. By default,
    each leg is annotated every time it enters the map region.
    Alternatively, append **+i**\ *spacing* to place this label every
    *spacing* units apart along the track. Append one of the units **k**
    (km), **n** (nautical mile), **d** (day), or **h** (hour).

.. _-B:

.. include:: ../../explain_-B.rst_

.. _-C:

**-C**\ **f**\ \|\ **g**\ \|\ **e**
    Select procedure for along-track distance calculation:
        **f** Flat Earth distances.
        **g** Great circle distances [Default].
        **e** Geodesic distances on current GMT ellipsoid.

.. _-D:

**-Da**\ *startdate*
    Do not plot data collected before *startdate*
    (yyyy-mm-dd\ **T**\ [hh:mm:ss]) [Default is first day].
**-Db**\ *stopdate*
    Do not plot data collected after *stopdate*
    (yyyy-mm-dd\ **T**\ [hh:mm:ss]). [Default is last day].

.. _-F:

**-F**
    Do not apply the error bit flags if present in a MGD77+ file
    [Default will apply these flags upon reading the data].

.. _-G:

**-G**\ **d**\ \|\ **t**\ \|\ **n**\ *gap*
    Let successive point separations exceeding **d**\ *gap* (km) or
    **t**\ *gap* (minutes) indicate a break in the track where we should
    not draw a line [no gaps recognized]. Repeat to use both types of
    gap checking. The **n**\ *N* form is used to plot only one every other *N* points.
    This is useful to reduce plot file size bat cannot be used (will be ignored)
    with the other two gap types.

.. _-I:

**-I**\ *ignore*
    Ignore certain data file formats from consideration. Append
    **a\|c\|m\|t** to ignore MGD77 ASCII, MGD77+ netCDF, MGD77T ASCII, or plain table
    files, respectively. The option may be repeated to ignore more than
    one format. [Default ignores none].

.. _-L:

**-L**\ *trackticks*
    To put time/distance log-marks on the track. E.g.
    **a**\ 500\ **ka**\ 24\ **ht**\ 6\ **h** means (**a**)nnotate every
    500 km (**k**) and 24 **h**\ (ours), with
    (**t**)ickmarks every 500 km and 6 hours. Alternatively you may use
    the modifiers **d** (days) and **n** (nautical miles).

.. _-S:

**-Sa**\ *startdist*\ [**u**]
    Do not plot data that are less than *startdist* meter along track
    from port of departure. Append **k** for km, **m** for miles, or
    **n** for nautical miles [Default is 0 meters].
**-Sb**\ *stopdist*\ [**u**]
    Do not plot data that are more than *stopdist* meter along track
    from port of departure. Append **k** for km, **m** for miles, or
    **n** for nautical miles [Default is end of track].

.. _-T:

**-TT**\ \|\ **t**\ \|\ **d**\ *ms*,\ *mc*,\ *mfs*,\ *mf*,\ *mfc*
    Controls the attributes of the three kinds of markers (**T** for the
    first time marker in a new day, **t** for additional time markers in
    the same day, and **d** for distance markers). For each of these you
    can specify the 5 comma-separated attributes *markersize*,
    *markercolor*, *markerfontsize*, *markerfont*, and
    *markerfontcolor*. Repeat the **-T** option for each marker type.

.. _-U:

.. include:: ../../explain_-U.rst_

.. _-W:

**-W**\ [**-**\ \|\ **+**][*pen*]
    Append *pen* used for the trackline. [Defaults:
    width = default, color = black, style = solid]. A leading **+** will
    use the lookup color (via **-C**) for both symbol fill and outline
    pen color, while a leading **-** will set outline pen color and turn
    off symbol fill. 

.. _-X:

.. include:: ../../explain_-XY.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_
 
.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_
 
.. include:: ../../explain_-t.rst_
    
.. include:: ../../explain_help.rst_

Examples
--------

To generate a Mercator map of the track of the cruise 01010007 in the
area 70W to 20E, 40S to 20N, using a Mercator scale of 0.1inch/degree,
label the tracks with 10 points characters, annotate the boundaries
every 10 degrees, draw gridlines every 5 degrees, and mark the track
every day and 1000 km, with ticks every 6 hours and 250 km, and create
a PDF map, enter the following command:

   ::

    gmt mgd77track 01010007 -R70W/20E/40S/20N -Jm0.1 -B10g5 -A10 \
                   -La1da1000kf6hf250k -pdf map

See Also
--------

:doc:`mgd77info`,
:doc:`basemap </basemap>`,
:doc:`mgd77list`

References
----------

The Marine Geophysical Data Exchange Format - MGD77, see
`<http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt>`_
