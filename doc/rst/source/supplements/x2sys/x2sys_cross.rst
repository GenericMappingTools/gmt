.. index:: ! x2sys_cross

***********
x2sys_cross
***********

.. only:: not man

    x2sys_cross - Calculate crossovers between track data files

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_cross** *track(s)* |-T|\ *TAG* [ |-A|\ *combi.lis* ]
[ |-C|\ [*runtimes*] ]
[ |-D|\ [**S**\ \|\ **N**\ ] ]
[ |-I|\ **l**\ \|\ **a**\ \|\ **c** ]
[ |-Q|\ **e**\ \|\ **i** ]
[ |-S|\ **l**\ \|\ **u**\ \|\ **h**\ *speed* ]
[ |SYN_OPT-V| ]
[ |-W|\ *size* ] [ |-Z| ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-do| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**x2sys_cross** is used to determine all intersections between
("external cross-overs") or within ("internal cross-overs") tracks
(Cartesian or geographic), and report the time, position, distance along
track, heading and speed along each track segment, and the crossover
error (COE) and mean values for all observables. The names of the tracks
are passed on the command line. By default, **x2sys_cross** will look
for both external and internal COEs. As an option, you may choose to
project all data using one of the map-projections prior to calculating
the COE.

Required Arguments
------------------

.. include:: explain_track.rst_
.. include:: explain_tag.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ *combi.lis*
    Only process the pair-combinations found in the file *combi.lis*
    [Default process all possible combinations among the specified
    files]. The file *combi.lis* created by :doc:`x2sys_get` -L option

.. _-C:

**-C**\ [*runtimes*]
    Compute and append the processing run-time for each pair to the
    progress message. Append a filename to save these run-times to file.
    The idea here is to use the knowledge of run-times to split the main
    process in a number of sub-processes that can each be launched in a
    different processor of your multi-core machine. See the MATLAB
    function *split_file4coes.m* that lives in the x2sys supplement source code.

.. _-D:

**-D**\ [**S**\ \|\ **N**\ ]
    Control how geographic coordinates are handled (Cartesian data are unaffected).
    By default, we determine if the data are closer to one pole than the other, and
    then we use a cylindrical polar conversion to avoid problems with longitude jumps.
    You can turn this off entirely with **-D** and then the calculations uses the
    original data (we have protections against longitude jumps).  However, you can
    force the selection of the pole for the projection by appending **S** or **N**
    for the south or north pole, respectively.  The conversion is used because the
    algorithm used to find crossovers are inherently a Cartesian algorithm that can
    run into trouble with data that has large longitudinal range at higher latitudes.

.. _-I:

**-Il**\ \|\ **a**\ \|\ **c**
    Sets the interpolation mode for estimating values at the crossover. Choose among:

    **l** Linear interpolation [Default].

    **a** Akima spline interpolation.

    **c** Cubic spline interpolation.

.. _-Q:

**-Qe**\ \|\ **i**
    Append **e** for external COEs only, and **i** for internal COEs
    only [Default is all COEs].

.. |Add_-Rgeo| replace:: For Cartesian
    data just give *xmin/xmax/ymin/ymax*. This option limits the COEs to those that fall inside the specified domain.
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-Sl**\ \|\ **u**\ \|\ **h**\ *speed*
    Defines window of track speeds. If speeds are outside this window we do not calculate a COE. Specify

    **-Sl** sets lower speed [Default is 0].

    **-Su** sets upper speed [Default is Infinity].

    **-Sh** does not limit the speed but sets a lower speed below which headings
    will not be computed (i.e., set to NaN) [Default calculates
    headings regardless of speed].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**\ *size*
    Give the maximum number of data points on either side of the
    crossover to use in the spline interpolation [3].

.. _-Z:

**-Z**
    Report the values of each track at the crossover [Default reports
    the crossover value and the mean value].

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-bo.rst_

.. |Add_-do| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-do.rst_

.. include:: ../../explain_help.rst_

Remarks
-------

The COEs found are printed out to standard output in ASCII format
(unless **-bo** is set). When ASCII is chosen,
the output format depends on whether or not old-style XOVER output
(**-L**) has been selected [See the GMT4 x_over man page for more
details]. If ASCII, then the first record contains the name of the tag
used, the second records specifies the exact command line used for this
run, and the third record contains the names of each column. These three
records are encoded as table headers and start with #.  For each
track pair, there will be a segment header record containing the two
file names and their start/stop/dist information (start/stop is absolute
time or NaN if unavailable while dist is the total track length),
whereas subsequent records have the data for each COE encountered. The
fields written out are x, y, time along track #1 and #2, distance along
track #1 and #2, heading along track #1 and #2, velocity along track #1
and #2, and then pairs of columns for each selected observable. These
are either pairs of (COE, average value) for each data type (or
track-values #1 and #2; see **-Z**). It is recommended that the Akima
spline is used instead of the natural cubic spline, since it is less
sensitive to outliers that tend to introduce wild oscillations in the
interpolation.

Sign Convention
---------------

If track_a and track_b are passed on the command line, then the COE
value is Value (track_a) - Value (track_b).

Precision And Format
--------------------

The output format of individual columns are controlled by
:ref:`FORMAT_FLOAT_OUT <FORMAT_FLOAT_OUT>` except for geographic coordinates
(:ref:`FORMAT_GEO_OUT <FORMAT_GEO_OUT>`) and absolute calendar time
(:ref:`FORMAT_DATE_OUT <FORMAT_DATE_OUT>`, :ref:`FORMAT_CLOCK_OUT <FORMAT_CLOCK_OUT>`).
Make sure these are set to give you enough significant digits to achieve the desired precision.

Examples
--------

To compute all internal crossovers in the gmt-formatted file c2104.gmt,
and using the tag GMT, try

   ::

    gmt x2sys_cross c2104.gmt -TGMT > c2104.txt

To find the crossover locations with bathymetry between the two MGD77
files A13232.mgd77 and A99938.mgd77, using the MGD77 tag, try

   ::

    gmt x2sys_cross A13232.mgd77 A99938.mgd77 -Qe -TMGD77 > crossovers.txt

References
----------

Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys
package. *Computers and Geosciences*, **36**, 348-354.

Wessel, P. (1989), XOVER: A cross-over error detector for track data,
*Computers and Geosciences*, **15**\ (3), 333-346.

See Also
--------

:doc:`gmt </gmt>`, :doc:`x2sys_binlist`,
:doc:`x2sys_init`,
:doc:`x2sys_datalist`,
:doc:`x2sys_get`,
:doc:`x2sys_list`,
:doc:`x2sys_put`,
:doc:`x2sys_report`,
:doc:`x2sys_solve`,
:manpage:`x_over`
