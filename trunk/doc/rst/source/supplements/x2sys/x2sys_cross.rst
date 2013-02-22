***********
x2sys_cross
***********

**x2sys\_cross** - Calculate crossovers between track data files

`Synopsis <#toc1>`_
-------------------

**x2sys\_cross** *track(s)* **-T**\ *TAG* [ **-A**\ *combi.lis* ] [
**-C**\ [*runtimes*\ ] ] [ **-Il**\ \|\ **a**\ \|\ **c** ] [
**-J**\ *parameters* ] [ **-Qe**\ \|\ **i** ] [
**-Sl**\ \|\ **u**\ \|\ **h**\ *speed* ] [ **-V**\ [*level*\ ] ] [
**-W**\ *size* ] [ **-Z** ] [ **-bo**\ [*ncols*\ ][*type*\ ] ]

`Description <#toc2>`_
----------------------

**x2sys\_cross** is used to determine all intersections between
("external cross-overs") or within ("internal cross-overs") tracks
(Cartesian or geographic), and report the time, position, distance along
track, heading and speed along each track segment, and the crossover
error (COE) and mean values for all observables. The names of the tracks
are passed on the command line. By default, **x2sys\_cross** will look
for both external and internal COEs. As an option, you may choose to
project all data using one of the map-projections prior to calculating
the COE.

.. include:: ../../explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

.. include:: explain_track.rst_
.. include:: explain_tag.rst_

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ *combi.lis*
    Only process the pair-combinations found in the file *combi.lis*
    [Default process all possible combinations among the specified
    files]. The file *combi.lis* created by x2sys\_get -L option
**-C**\ [*runtimes*\ ]
    Compute and append the processing run-time for each pair to the
    progress message. Append a filename to save these run-times to file.
    The idea here is to use the knowledge of run-times to split the main
    process in a number of sub-processes that can each be launched in a
    different processor of your multi-core machine. See the MATLAB
    function *split\_file4coes.m* that lives in the x2sys supplement
    source code.
**-Il**\ \|\ **a**\ \|\ **c**
    Sets the interpolation mode for estimating values at the crossover.
    Choose among:

    **l** Linear interpolation [Default].

    **a** Akima spline interpolation.

    **c** Cubic spline interpolation.

.. include:: ../../explain_-J.rst_

**-Qe**\ \|\ **i**
    Append **e** for external COEs only, and **i** for internal COEs
    only [Default is all COEs].

.. |Add_-Rgeo| replace:: For Cartesian
    data just give *xmin/xmax/ymin/ymax*. This option limits the COEs to
    those that fall inside the specified domain.
.. include:: ../../explain_-Rgeo.rst_

**-Sl**\ \|\ **u**\ \|\ **h**\ *speed*
    Defines window of track speeds. If speeds are outside this window we
    do not calculate a COE. Specify

    **-Sl** sets lower speed [Default is 0].

    **-Su** sets upper speed [Default is Infinity].

    **-Sh** does not limit the speed but sets a lower speed below which headings
    will not be computed (i.e., set to NaN) [Default calculates
    headings regardless of speed].

.. |Add_-V| unicode:: 0x0C .. just an invisible code
.. include:: ../../explain_-V.rst_

**-W**\ *size*
    Give the maximum number of data points on either side of the
    crossover to use in the spline interpolation [3].
**-Z**
    Report the values of each track at the crossover [Default reports
    the crossover value and the mean value].

.. |Add_-bo| unicode:: 0x0C .. just an invisible code
.. include:: ../../explain_-bo.rst_

.. include:: ../../explain_help.rst_

`Remarks <#toc6>`_
------------------

The COEs found are printed out to standard output in ASCII format
(unless **-bo**\ [*ncols*\ ][*type*\ ] is set). When ASCII is chosen,
the output format depends on whether or not old-style XOVER output
(**-L**) has been selected [See the **x\_over** man page for more
details]. If ASCII, then the first record contains the name of the tag
used, the second records specifies the exact command line used for this
run, and the third record contains the names of each column. For each
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

`Sign Convention <#toc7>`_
--------------------------

If track\_a and track\_b are passed on the command line, then the COE
value is Value (track\_a) - Value (track\_b).

`Precision And Format <#toc8>`_
-------------------------------

The output format of individual columns are controlled by
**FORMAT\_FLOAT\_OUT** except for geographic coordinates
(**FORMAT\_GEO\_OUT**) and absolute calendar time
(**FORMAT\_DATE\_OUT**, **FORMAT\_CLOCK\_OUT**). Make sure these are set
to give you enough significant digits to achieve the desired precision.

`Examples <#toc9>`_
-------------------

To compute all internal crossovers in the gmt-formatted file c2104.gmt,
and using the tag GMT, try

x2sys\_cross c2104.gmt -TGMT > c2104.d

To find the crossover locations with bathymetry between the two MGD77
files A13232.mgd77 and A99938.mgd77, using the MGD77 tag, try

x2sys\_cross A13232.mgd77 A99938.mgd77 -Qe -TMGD77 > crossovers.d

`References <#toc10>`_
----------------------

Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys
package. *Computers and Geosciences*, **36**, 348-354.

Wessel, P. (1989), XOVER: A cross-over error detector for track data,
*Computers and Geosciences*, **15**\ (3), 333-346.

`See Also <#toc11>`_
--------------------

`*GMT*\ (1) <GMT.html>`_ , `*x2sys\_binlist*\ (1) <x2sys_binlist.html>`_
, `*x2sys\_init*\ (1) <x2sys_init.html>`_ ,
`*x2sys\_datalist*\ (1) <x2sys_datalist.html>`_ ,
`*x2sys\_get*\ (1) <x2sys_get.html>`_ ,
`*x2sys\_list*\ (1) <x2sys_list.html>`_ ,
`*x2sys\_put*\ (1) <x2sys_put.html>`_ ,
`*x2sys\_report*\ (1) <x2sys_report.html>`_ ,
`*x2sys\_solve*\ (1) <x2sys_solve.html>`_ ,
`*x\_over*\ (1) <x_over.html>`_
