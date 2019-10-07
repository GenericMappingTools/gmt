.. index:: ! x2sys_report

**************
x2sys_report
**************

.. only:: not man

    x2sys_report - Report statistics from crossover data base

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_report** |-C|\ *column* |-T|\ *TAG* [ *coedbase.txt* ]
[ |-A| ]
[ |-I|\ [*list*\ ] ]
[ |-L|\ [*corrtable*] ]
[ |-N|\ *nx_min* ]
[ |-Q|\ **e**\ \|\ **i** ]
[ [ |SYN_OPT-R| ]
[ |-S|\ *track* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**x2sys_report** will read the input crossover ASCII data base
*coedbase.txt* (or *stdin*) and report on the statistics of crossovers
(*n*, *mean*, *stdev*, *rms*, *weight*) for each track. Options are
available to let you exclude tracks and limit the output.

Required Arguments
------------------

*coedbase.txt*
    The name of the input ASCII crossover error data base as produced by
    :doc:`x2sys_cross`. If not given we read standard input instead.

.. _-C:

**-C**\ *column*
    Specify which data column you want to process. Crossovers related to
    this column name must be present in the crossover data base.

.. include:: explain_tag.rst_

Optional Arguments
------------------

.. _-A:

**-A**
    Eliminate COEs by distributing the COE between the two tracks in
    proportion to track weight and producing (dist, adjustment) spline
    knots files for each track (for the selected *column*). Such
    adjustments may be used by :doc:`x2sys_datalist`. The adjustment files
    are called *track.column*.adj and are placed in the
    **$X2SYS_HOME**/*TAG* directory. For background information on how
    these adjustments are designed, see *Mittal* [1984].

.. _-I:

**-I**\ [*list*]
    Name of ASCII file with a list of track names (one per record) that
    should be excluded from consideration [Default includes all tracks].

.. _-L:

**-L**\ [*corrtable*]
    Apply optimal corrections to the chosen observable. Append the
    correction table to use [Default uses the correction table
    *TAG*\ \_corrections.txt which is expected to reside in the
    **$X2SYS_HOME**/*TAG* directory]. For the format of this file, see
    :doc:`x2sys_solve`.

.. _-N:

**-N**\ *nx_min*
    Only report data from tracks involved in at least *nx_min* crossovers [all tracks].

.. _-Q:

**-Qe**\ \|\ **i**
    Append **e** for external crossovers or **i** for internal
    crossovers only [Default is external].

.. _-R:

.. |Add_-Rgeo| replace:: For Cartesian
    data just give *xmin/xmax/ymin/ymax*. This option bases the
    statistics on those COE that fall inside the specified domain.
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-S**\ *track*
    Name of a single track. If given we restrict output to those
    crossovers involving this track [Default output is crossovers
    involving any track pair].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

Examples
--------

To report statistics of all the external magnetic crossovers associated
with the tag MGD77 from the file COE_data.txt, restricted to occupy a
certain region in the south Pacific, try

   ::

    gmt x2sys_report COE_data.txt -V -TMGD77 -R180/240/-60/-30 -Cmag > mag_report.txt

To report on the faa crossovers globally that involves track 12345678, try

   ::

    gmt x2sys_report COE_data.txt -V -TMGD77 -Cfaa -S2345678 > faa_report.txt

References
----------

Mittal, P. K. (1984), Algorithm for error adjustment of potential field
data along a survey network, *Geophysics*, **49**\ (4), 467-469.

See Also
--------

:doc:`x2sys_binlist`
:doc:`x2sys_cross`
:doc:`x2sys_datalist`
:doc:`x2sys_get`
:doc:`x2sys_init`
:doc:`x2sys_list`
:doc:`x2sys_put`
:doc:`x2sys_solve`
