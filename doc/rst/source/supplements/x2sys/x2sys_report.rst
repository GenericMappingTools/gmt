**************
x2sys_report
**************

x2sys\_report - Report statistics from crossover data base

`Synopsis <#toc1>`_
-------------------

.. include:: ../../common_SYN_OPTs.rst_

**x2sys\_report** **-C**\ *column* **-T**\ *TAG* [ *coedbase.txt* ] [
**-A** ] [ **-I**\ [*list*\ ] ] [ **-L**\ [*corrtable*\ ] ] [
**-N**\ *nx\_min* ] [ **-Qe**\ \|\ **i** ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-S**\ *track* ] [
**-V**\ [*level*\ ] ]

`Description <#toc2>`_
----------------------

**x2sys\_report** will read the input crossover ASCII data base
*coedbase.txt* (or *stdin*) and report on the statistics of crossovers
(*n*, *mean*, *stdev*, *rms*, *weight*) for each track. Options are
available to let you exclude tracks and limit the output.

.. include:: ../../explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

*coedbase.txt*
    The name of the input ASCII crossover error data base as produced by
    **x2sys\_cross**. If not given we read standard input instead.
**-C**\ *column*
    Specify which data column you want to process. Crossovers related to
    this column name must be present in the crossover data base.

.. include:: explain_tag.rst_

`Optional Arguments <#toc5>`_
-----------------------------

**-A**
    Eliminate COEs by distributing the COE between the two tracks in
    proportion to track weight and producing (dist, adjustment) spline
    knots files for each track (for the selected *column*). Such
    adjustments may be used by **x2sys\_datalist**. The adjustment files
    are called *track.column*.adj and are placed in the
    **$X2SYS\_HOME**/*TAG* directory. For background information on how
    these adjustments are designed, see *Mittal* [1984].
**-I**\ [*list*\ ]
    Name of ASCII file with a list of track names (one per record) that
    should be excluded from consideration [Default includes all tracks].
**-L**\ [*corrtable*\ ]
    Apply optimal corrections to the chosen observable. Append the
    correction table to use [Default uses the correction table
    *TAG*\ \_corrections.txt which is expected to reside in the
    **$X2SYS\_HOME**/*TAG* directory]. For the format of this file, see
    **x2sys\_solve**.
**-N**\ *nx\_min*
    Only report data from tracks involved in at least *nx\_min*
    crossovers [all tracks].
**-Qe**\ \|\ **i**
    Append **e** for external crossovers or **i** for internal
    crossovers only [Default is external].

.. |Add_-Rgeo| replace:: For Cartesian
    data just give *xmin/xmax/ymin/ymax*. This option bases the
    statistics on those COE that fall inside the specified domain.
.. include:: ../../explain_-Rgeo.rst_

**-S**\ *track*
    Name of a single track. If given we restrict output to those
    crossovers involving this track [Default output is crossovers
    involving any track pair].

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

`Examples <#toc6>`_
-------------------

To report statistics of all the external magnetic crossovers associated
with the tag MGD77 from the file COE\_data.txt, restricted to occupy a
certain region in the south Pacific, try

x2sys\_report COE\_data.txt -V -TMGD77 -R180/240/-60/-30 -Cmag >
mag\_report.txt

To report on the faa crossovers globally that involves track 12345678, try

x2sys\_report COE\_data.txt -V -TMGD77 -Cfaa -S2345678 > faa\_report.txt

`References <#toc7>`_
---------------------

Mittal, P. K. (1984), Algorithm for error adjustment of potential field
data along a survey network, *Geophysics*, **49**\ (4), 467-469.

`See Also <#toc8>`_
-------------------

`x2sys\_binlist <x2sys_binlist.html>`_
`x2sys\_cross <x2sys_cross.html>`_
`x2sys\_datalist <x2sys_datalist.html>`_
`x2sys\_get <x2sys_get.html>`_
`x2sys\_init <x2sys_init.html>`_
`x2sys\_list <x2sys_list.html>`_
`x2sys\_put <x2sys_put.html>`_
`x2sys\_solve <x2sys_solve.html>`_
