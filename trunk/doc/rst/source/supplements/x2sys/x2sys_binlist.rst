***************
x2sys_binlist
***************

x2sys\_binlist - Create bin index listing from track data files

`Synopsis <#toc1>`_
-------------------

.. include:: ../../common_SYN_OPTs.rst_

**x2sys\_binlist** *track(s)* **-T**\ *TAG* [ **-D** ] [ **-E** ] [
**-V**\ [*level*\ ] ]

`Description <#toc2>`_
----------------------

**x2sys\_binlist** reads one or more track data files and produces a
multisegment ASCII track bin-index file (tbf) with the track name in the
header and one data record per bin crossed; these records contain *lon*,
*lat*, *index*, *flags*\ [, *dist*], where *lon*, *lat* are the
coordinates of the center of the bin, the *index* is the 1-D number of
the bin, and *flags* is a bitflag that describes which data fields were
available in this bin. The optional *dist* requires **-D**. The input
files can be of any format, which must be described and passed with the
**-T** option. The bin-index listing is a crude representation of where
the track goes and is used by the data archivist to build an x2sys track
data base for miscellaneous track queries, such as when needing to
determine which tracks should be compared in a crossover analysis. You
must run **x2sys\_init** to initialize the tag before you can run the
indexing.

.. include:: ../../explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

.. include:: explain_track.rst_
.. include:: explain_tag.rst_

`Optional Arguments <#toc5>`_
-----------------------------

**-D**
    Calculate the length of trackline segments per bin [Default skips
    this step]. The length fragments are given as the 5th output column
    (after the *flags*). The length units are obtained via the TAB
    setting (see **x2sys\_init**).
**-E**
    Convert geographic data to a cylindrical equal-area projection prior
    to binning. Basically, we apply the projection
    **-JY**\ *lon0*/37:04:17.166076/360, where *lon0* is the
    mid-longitude of the region. Requires **-D**, geographical data, and
    a global region (e.g., **-Rg** or **-Rd**). This option is useful
    for statistics related to trackline density but should not be used
    when preparing bin-index files for the x2sys track data bases.

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

`Examples <#toc6>`_
-------------------

To create a bin index file from the MGD77 file 01030061.mgd77 using the
settings associated with the tag MGD77, do

x2sys\_binlist 01030061.mgd77 -TMGD77 > 01030061.tbf

To create a track bin index file of all MGD77+ files residing in the
current directory using the settings associated with the tag MGD77+ and
calculate track distances, run

x2sys\_binlist \*.nc -TMGD77+ -D > all.tbf

`See Also <#toc7>`_
-------------------

`x2sys\_cross <x2sys_cross.html>`_ ,
`x2sys\_datalist <x2sys_datalist.html>`_ ,
`x2sys\_get <x2sys_get.html>`_ ,
`x2sys\_init <x2sys_init.html>`_ ,
`x2sys\_put <x2sys_put.html>`_ ,
`x2sys\_report <x2sys_report.html>`_ ,
`x2sys\_solve <x2sys_solve.html>`_
