.. index:: ! x2sys_get

*********
x2sys_get
*********

.. only:: not man

    x2sys_get - Get track listing from the x2sys track index databases

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_get** |-T|\ *TAG* [ |-C| ] [ |-F|\ *flags* ] [ |-G| ]
[ |-L|\ [*list*]\ [**+i**\ ] ]
[ |-N|\ *flags* ] [
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**x2sys_get** will return text records with the names of the track data files in the
x2sys data base for this TAG that match the given requirements. You may
choose a specific region and optionally ask only for tracks that meet
certain data criteria. Finally, you may select an option to list all
possible pairs that might generate crossovers.

Required Arguments
------------------

.. include:: explain_tag.rst_

Optional Arguments
------------------

.. _-C:

**-C**
    Instead of reporting the track names, just output the coordinates of
    the center of each bin that has at least one track with the
    specified data.

.. _-D:

**-D**
    Only report the track names [Default adds the availability of data for each field].

.. _-F:

**-F**\ *flags*
    Give a comma-separated list of column names (as described in the
    format definition file) that should be present. [Default selects all data columns].

.. _-G:

**-G**
    Report data flags (Y or N) for the entire track rather than just for
    the portion that is inside the region set by **-R** [Default].

.. _-L:

**-L**\ [*list*]\ [**+i**\ ]
    Crossover mode. Return a list of track pairs that should be checked
    for possible crossovers. The list is determined from the bin-index
    data base on the assumption that tracks occupying the same bin are
    very likely to intersect. By default we return all possible pairs in
    the data base. Append the name of a file with a list of tracks if
    you want to limit the output to those pairs that involve at least
    one of the track names in your list. The output is suitable for the
    **-A** option in :doc:`x2sys_cross`. By default, only external
    crossover pairs are listed. Append **+i** to include internal pairs in the list.

.. _-N:

**-N**\ *flags*
    Give a comma-separated list of column names (as described in the
    format definition file) that must be absent.

.. _-R:

.. |Add_-Rgeo| replace:: For Cartesian
    data just give *xmin/xmax/ymin/ymax*. This option limits the tracks
    to those that fall at least partly inside the specified domain.
.. include:: ../../explain_-Rgeo.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

Examples
--------

To find all the tracks associated with the tag MGD77, restricted to
occupy a certain region in the south Pacific, and have at least free air
anomalies and bathymetry, try

   ::

    gmt x2sys_get -V -TMGD77 -R180/240/-60/-30 -Ffaa,depth

To find all the tracks associated with the tag MGD77 that have depth but
not twt, try

   ::

    gmt x2sys_get -V -TMGD77 -Fdepth -Nwt

To find all the pairs associated with the tag MGD77 that might intersect
each other, but only those pairs which involves tracks in your list
new.lis, try

   ::

    gmt x2sys_get -V -TMGD77 -Lnew.lis > xpairs.lis

Note
----

The tracks that are returned all have the requested data (**-F**) within
the specified region (**-R**). Furthermore, the columns of Y and N for
other data types also reflect the content of the track portion within
the selected region, unless **-G** is set.

See Also
--------

:doc:`x2sys_binlist`,
:doc:`x2sys_cross` 
:doc:`x2sys_datalist`,
:doc:`x2sys_init`,
:doc:`x2sys_list`,
:doc:`x2sys_put`,
:doc:`x2sys_report`,
:doc:`x2sys_solve`
