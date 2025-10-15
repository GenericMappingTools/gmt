.. index:: ! solar
.. include:: module_core_purpose.rst_

*****
solar
*****

|solar_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt solar**
[ |SYN_OPT-B| ]
[ |-C| ]
[ |-G|\ [*fill*] ]
[ |-I|\ [*lon/lat*][**+d**\ *date*][**+z**\ *TZ*] ]
[ |-J|\ *parameters* ]
[ |-M| ]
[ |-N| ]
[ |SYN_OPT-R| ]
[ |-T|\ **dcna**\ [**+d**\ *date*][**+z**\ *TZ*]]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

**solar** calculates closed polygons for the day-night terminator and the civil, nautical and astronomical twilights
and either writes them to standard output or uses them for clipping or filling on maps.


Required Arguments
------------------

None.

Optional Arguments
------------------

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**
    Formats the report selected by |-I| using tab-separated fields on a single line. The
    output is Sun *Lon Lat Azimuth Elevation* in degrees, *Sunrise Sunset Noon* in decimal days,
    *day length* in minutes, *SolarElevationCorrected* corrected for the effect of refraction index
    and *Equation of time* in minutes. Note that if no position is provided in **-I**\ *lon/lat* the
    data after *Elevation* refers to the point (0,0).

.. _-G:

**-G**\ [*fill*] :ref:`(more ...) <-Gfill_attrib>`
    Select color or pattern for filling of terminators, or give no argument for clipping [Default is no fill or clipping].
    Deactivate clipping by appending the output of gmt :doc:`clip` |-C|.

.. _-I:

**-I**\ [*lon/lat*][**+d**\ *date*][**+z**\ *TZ*]
    Print current sun position as well as Azimuth and Elevation. Append *lon/lat* to print also the times of
    Sunrise, Sunset, Noon and length of the day.

    The default time is now. To select another time, use these two modifiers:
    
    - **+d** - Print *date* in ISO 8601 format, e.g, **+d**\ *2000-04-25T04:52*, to compute sun parameters
      for this date and time [Default is now].
    - **+z** - If necessary, append the time zone *TZ*. The time zone is given as an offset from UTC.
      Negative offsets look like *−03:00* or *−03*. Positive offsets look like *02:00* or *02*.

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-M:

**-M**
    Write terminator(s) as a multisegment ASCII (or binary, see **-b**\ *o*) polygons to standard output. No plotting occurs.

.. _-N:

**-N**
    Invert the sense of what is inside and outside the terminator.  Only
    used with clipping (|-G|) and cannot be used together with |-B|.

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-T:

**-Tdcna**\ [**+d**\ *date*][**+z**\ *TZ*]
    Plot (or dump; see **-M**\ ) one or more terminators by appending any of these directives:

    - **d** - Select the day-night terminator.
    - **c** - Select civil twilight.
    - **n** - Select nautical twilight.
    - **a** - Select astronomical twilight.

    The default time is now. To select another time, use these two modifiers:

    - **+d** - Append *date* in ISO format, e.g, **+d**\ *2000-04-25T12:15:00*
      to know where the day-night was at that date [Default is now].
    - **+z** - If necessary, append the time zone *TZ*. The time zone is given as an offset from UTC.
      Negative offsets look like *−03:00* or *−03*. Positive offsets look like *02:00* or *02*.

    Refer to https://en.wikipedia.org/wiki/Twilight for definitions of different twilights.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*pen*] :ref:`(more ...) <-Wpen_attrib>`
    Set pen attributes for lines or the outline of symbols [Defaults:
    width = 0.25p, color = black, style = solid].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bo.rst_

.. include:: explain_-ocols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. module_common_ends

Examples
--------

.. include:: explain_example.rst_

Print current Sun position and Sunrise, Sunset times at given date, time and time zone::

    gmt solar -I-7.93/37.079+d2016-02-04T10:01:00+z02:00

Plot the day-night and civil twilight::

    gmt begin
      gmt coast -Rd -W0.1p -JQ0/14c -B -BWSen -Dl -A1000
      gmt solar -W1p -Tdc
    gmt end show

Set up a clip path overlay based on the day-night terminator::

    gmt solar -G -Td

.. module_note_begins

References
----------

Code from the Excel Spreadsheets in https://gml.noaa.gov/grad/solcalc/calcdetails.html.

Notes
-----

Taken from the NOAA site *Data for Litigation* note.

    *The NOAA Solar Calculator is for research and recreational use only. NOAA cannot certify or authenticate sunrise, sunset or solar position data. The U.S. Government does not collect observations of astronomical data, and due to atmospheric conditions our calculated results may vary significantly from actual observed values.*

    *For further information, please see the U.S. Naval Observatory's page* `Astronomical Data Used for Litigation <https://aa.usno.navy.mil/faq/lawyers>`_

.. module_note_ends

See Also
--------

:doc:`gmt`, :doc:`clip`, :doc:`coast`, :doc:`plot`
