.. index:: ! earthtide

*********
earthtide
*********

.. only:: not man

    earthtide - Compute grids or time-series of solid Earth tides

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt earthtide**
|-T|\ [\ *min/max*\ /]\ *inc*\ [**+n**] \|\ |-T|\ *file*\ \|\ *list*
|-G|\ *grdfile*
[ |-C|\ *components* ]
[ |SYN_OPT-I| ]
[ |-L|\ *lon/lat* ] 
[ |SYN_OPT-R| ]
[ |-S| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

Compute the three components of solid Earth tides as time-series or grids. Optionally compute also Sun and Moon position in lon,lat.
The output can be either in the form of a grid or as a table printed to stdout. The format of the table data is:
*time north east vertical* in units of meters.


Required Arguments
------------------

Specify either **-G**, **-S** or **-L**.

.. _-G:

**-G**\ *grdfile*
    Write one or more tide component directly to grids; no table data are written to standard output.
    If more than one component are specified via **-C** then *grdfile* must contain the format flag %s
    so that we can embed the component code in the file names (*n* for north; *e* for east and *v* for vertical).
    If only one component is selected with **-C** then no code is appended to grid name (and no need to 
    set the format flag %s). The grid(s) are computed at the time set by **-T**, if that option is used, or
    at the *now* time calculated in UTC from the computer clock.

.. _-S:

**-S**
    Output position of Sun and Moon in geographical coordinates plus distance
    in meters. Output is a Mx7 matrix where M is the number of times (set by **-T**)
    and columns are *time, sun_lon, sun_lat, sun_dist moon_lon, moon_lat, moon_dist*.

.. _-T:

**-T**\ [\ *min/max*\ /]\ *inc*\ [**+n**] \|\ |-T|\ *file*\ \|\ *list*
    Make evenly spaced time-steps from *min* to *max* by *inc*. Append **+n** to indicate *inc* is
    the number of *t*-values to produce over the range instead. Append a valid time unit
    (**d**\ \|\ **h**\ \|\ **|m**\ \|\ **|s**) to the increment. If only *min* is given then
    we use that date and time for the calculations.  If no **-T** is provided get
    current time in UTC from the computer clock. If no **-G** or **-S** are provided then **-T** is interpreted to mean compute
    a time-series at the location specified by **-L**, thus then **-L** becomes mandatory.
    When **-G** and **-T**, only first time T series is considered. Finally, dates may range from 1901 through 2099.

Optional Arguments
------------------

.. _-C:

**-C**\ *components*
    Select which component to write to individual grids. Requires **-G**.
    Append comma-separated codes for available components: **x** or **e** for the east component,
    **y** or **n** for the north component, and **z** or **v** for the vertical component.
    For example, **-Ce**\ ,\ **v**, will write two grids: one with east and the other with the vertical components.
    If **-G** is set but not **-C** then the default is to write the vertical component only.

.. _-I:

.. |Add_-I| replace:: Used only with **-G**. If not set, defaults to **-I**\ 30**m**
.. include:: ../../explain_-I.rst_

.. _-L:

**-L**\ *lon/lat*
    Geographical coordinate of the location where to compute a time-series. Coordinates are geodetic (ellipsoidal)
    latitude and longitude. GRS80 ellipsoid is used. (Which can be considered equivalent to the WGS84 ellipsoid at
    the sub-millimeter level.)


.. _-R:

.. |Add_-R| replace:: Used only with **-G**. If not set, defaults to **-Rd**
.. include:: ../../explain_-R.rst_


.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-bo.rst_

.. include:: ../../explain_-ocols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_


Examples
--------

To compute a global grid of the vertical component with a grid step of 30m at noon of 18 Jun 2018,
(note: we are using the defaults for **-R** and **-I**) try

::

    gmt earthtide -T2018-06-18T12:00:00 -Gsolid_tide_up.grd

To obtain a one day long time-series, starting at same date, at the -7W, 37N and 1 minute interval, try

::

    gmt earthtide -T2018-06-18T/2018-06-19T/1m -L-7/37 > solid_tide.dat


To get the Sun and Moon position in geographical coordinates at the *current* time

::

    gmt earthtide -S


Notes
-----

#. All times, both input and output, are in UTC.

References
----------

http://geodesyworld.github.io/SOFTS/solid.htm


See Also
--------

:doc:`gmt.conf </gmt.conf>`, :doc:`gmt </gmt>`,
