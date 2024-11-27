.. index:: ! mgd77track
.. include:: ../module_supplements_purpose.rst_

**********
mgd77track
**********

|mgd77track_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77track** *GEODAS-ids*
|SYN_OPT-R|
|-J|\ *parameters*
[ |-A|\ [**c**][*size*][**+i**\ *spacing*] ]
[ |SYN_OPT-B| ]
[ **-Da**\ *startdate* ]
[ |-D|\ **b**\ *stopdate* ]
[ |-F| ]
[ |-G|\ **d**\|\ **t**\|\ **n**\ *gap* ]
[ |-I|\ **a\|c\|m\|t** ]
[ |-L|\ *trackticks* ]
[ |-S|\ **a**\ *startdist* ]
[ |-S|\ **b**\ *stopdist* ]
[ |-T|\ **T**\|\ **t**\|\ **d**\ *ms*,\ *mc*,\ *mfs*,\ *mf*,\ *mfc* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

**mgd77track** reads NCEI MGD77 cruises and plots one or more ship tracks
on a map using the specified projection [*Wessel and Chandler*, 2007].

Required Arguments
------------------

.. include:: explain_ncid.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: /explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [**c**][*size*][**+i**\ *spacing*]
    Append **c** to annotate the track using the MGD77 cruise ID [Default uses the
    filename prefix]. Optional *size* is the font size in points. The
    leg annotation font is controlled by :term:`FONT_LABEL`. By default,
    each leg is annotated every time it enters the map region.
    Alternatively, append **+i**\ *spacing* to place this label every
    *spacing* units apart along the track. Append one of the units **k**
    (km), **n** (nautical mile), **d** (day), or **h** (hour).

.. |Add_-B| replace:: |Add_-B_links|
.. include:: ../../explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

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

**-G**\ **d**\|\ **t**\|\ **n**\ *gap*
    Let successive point separations exceeding **d**\ *gap* (km) or
    **t**\ *gap* (minutes) indicate a break in the track where we should
    not draw a line [no gaps recognized]. Repeat to use both types of
    gap checking. The **n**\ *N* form is used to plot only one every other *N* points.
    This is useful to reduce plot file size bat cannot be used (will be ignored)
    with the other two gap types.

.. _-I:

**-I**\ **a\|c\|m\|t**
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

**-Sa**\ *startdist*
    Do not plot data that are less than *startdist* meter along track
    from port of departure. Append **k** for km, **M** for miles, or
    **n** for nautical miles [Default is 0 meters].
**-Sb**\ *stopdist*
    Do not plot data that are more than *stopdist* meter along track
    from port of departure. Append **k** for km, **M** for miles, or
    **n** for nautical miles [Default is end of track].

.. _-T:

**-TT**\|\ **t**\|\ **d**\ *ms*,\ *mc*,\ *mfs*,\ *mf*,\ *mfc*
    Controls the attributes of the three kinds of markers (**T** for the
    first time marker in a new day, **t** for additional time markers in
    the same day, and **d** for distance markers). For each of these you
    can specify the 5 comma-separated attributes *markersize*,
    *markercolor*, *markerfontsize*, *markerfont*, and
    *markerfontcolor*. Repeat the |-T| option for each marker type.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: ../../explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*pen*]
    Append *pen* used for the trackline. [Defaults:
    width = 0.25p, color = black, style = solid].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: ../../explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-t.rst_

.. include:: ../../explain_help.rst_

.. module_common_ends

Examples
--------

.. include:: ../../oneliner_info.rst_

To generate a Mercator map of the track of the cruise 01010007 in the
area 70W to 20E, 40S to 20N, using a Mercator scale of 0.1inch/degree,
label the tracks with 10 points characters, annotate the boundaries
every 10 degrees, draw gridlines every 5 degrees, and mark the track
every day and 1000 km, with ticks every 6 hours and 250 km, and create
a PDF map, enter the following command:

::

  gmt mgd77track @01010007 -R70W/20E/40S/20N -Jm0.1 -B10g5 -A10 \
                 -La1da1000kf6hf250k -pdf map

.. module_note_begins

References
----------

The Marine Geophysical Data Exchange Format - MGD77, see
`<http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt>`_.

Wessel, P., and Chandler, M. T., 2007, The mgd77 supplement to the Generic Mapping Tools,
*Comp. Geosci.*, **33**\ (1), 62-75, https://doi.org/10.1016/j.cageo.2006.05.006.

.. module_note_ends

See Also
--------

:doc:`mgd77info`,
:doc:`basemap </basemap>`,
:doc:`mgd77list`
