.. index:: ! x2sys_binlist

***************
x2sys_binlist
***************

.. only:: not man

    x2sys_binlist - Create bin index listing from track data files

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_binlist** *track(s)* |-T|\ *TAG*
[ |-D| ]
[ |-E| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**x2sys_binlist** reads one or more track data files and produces a
multisegment ASCII track bin-index file (tbf) with the track name in the
header and one data record per bin crossed; these records contain *lon*,
*lat*, *index*, *flags*\ [, *dist*], where *lon*, *lat* are the
coordinates of the center of the bin, the *index* is the 1-D number of
the bin, and *flags* is a bitflag that describes which data fields were
available in this bin. The optional *dist* requires |-D|. The input
files can be of any format, which must be described and passed with the
|-T| option. The bin-index listing is a crude representation of where
the track goes and is used by the data archivist to build an x2sys track
data base for miscellaneous track queries, such as when needing to
determine which tracks should be compared in a crossover analysis. You
must run :doc:`x2sys_init` to initialize the tag before you can run the
indexing.

Required Arguments
------------------

.. include:: explain_track.rst_
.. include:: explain_tag.rst_

Optional Arguments
------------------

.. _-D:

**-D**
    Calculate the length of track-line segments per bin [Default skips
    this step]. The length fragments are given as the 5th output column
    (after the *flags*). The length units are obtained via the TAG
    setting (see :doc:`x2sys_init`).

.. _-E:

**-E**
    Convert geographic data to a cylindrical equal-area projection prior
    to binning. Basically, we apply the projection
    **-JY**\ *lon0*/37:04:17.166076/360, where *lon0* is the
    mid-longitude of the region. Requires **-D**, geographical data, and
    a global region (e.g., **-Rg** or **-Rd**). This option is useful
    for statistics related to track-line density but should not be used
    when preparing bin-index files for the x2sys track data bases.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

Examples
--------

To create a bin index file from the MGD77 file 01030061.mgd77 using the
settings associated with the tag MGD77, do

   ::

    gmt x2sys_binlist 01030061.mgd77 -TMGD77 > 01030061.tbf

To create a track bin index file of all MGD77+ files residing in the
current directory using the settings associated with the tag MGD77+ and
calculate track distances, run

   ::

    gmt x2sys_binlist *.nc -TMGD77+ -D > all.tbf

See Also
--------

:doc:`x2sys_cross`,
:doc:`x2sys_datalist`,
:doc:`x2sys_get`,
:doc:`x2sys_init`,
:doc:`x2sys_put`,
:doc:`x2sys_report`,
:doc:`x2sys_solve`
