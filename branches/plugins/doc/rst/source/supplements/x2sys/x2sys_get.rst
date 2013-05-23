*********
x2sys_get
*********

x2sys\_get - Get track listing from the x2sys track index databases

`Synopsis <#toc1>`_
-------------------

.. include:: ../../common_SYN_OPTs.rst_

**x2sys\_get** **-T**\ *TAG* [ **-C** ] [ **-F**\ *flags* ] [ **-G** ] [
**-L**\ [**+**\ ][*list*\ ] ] [ **-N**\ *flags* ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-V**\ [*level*\ ]
] ]

|No-spaces|

`Description <#toc2>`_
----------------------

**x2sys\_get** will return the names of the track data files in the
x2sys data base for this TAG that match the given requirements. You may
choose a specific region and optionally ask only for tracks that meet
certain data criteria. Finally, you may select an option to list all
possible pairs that might generate crossovers.

`Required Arguments <#toc4>`_
-----------------------------

.. include:: explain_tag.rst_

`Optional Arguments <#toc5>`_
-----------------------------

**-C**
    Instead of reporting the track names, just output the coordinates of
    the center of each bin that has at least one track with the
    specified data.
**-D**
    Only report the track names [Default adds the availability of data for each field].
**-F**\ *flags*
    Give a comma-separated list of column names (as described in the
    definition file) that should be present. [Default selects all data columns].
**-G**
    Report data flags (Y or N) for the entire track rather than just for
    the portion that is inside the region set by **-R** [Default].
**-L**\ [**+**\ ][*list*\ ]
    Crossover mode. Return a list of track pairs that should be checked
    for possible crossovers. The list is determined from the bin-index
    data base on the assumption that tracks occupying the same bin are
    very likely to intersect. By default we return all possible pairs in
    the data base. Append the name of a file with a list of tracks if
    you want to limit the output to those pairs that involve at least
    one of the track names in your list. The output is suitable for the
    **-A** option in **x2sys\_cross**. By default, only external
    crossover pairs are listed. Use **-L+** to include internal pairs in
    the list.
**-N**\ *flags*
    Give a comma-separated list of column names (as described in the
    definition file) that must be absent.

.. |Add_-Rgeo| replace:: For Cartesian
    data just give *xmin/xmax/ymin/ymax*. This option limits the tracks
    to those that fall at least partly inside the specified domain.
.. include:: ../../explain_-Rgeo.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

`Examples <#toc6>`_
-------------------

To find all the tracks associated with the tag MGD77, restricted to
occupy a certain region in the south Pacific, and have at least free air
anomalies and bathymetry, try

    x2sys\_get -V -TMGD77 -R180/240/-60/-30 -Ffaa,depth

To find all the tracks associated with the tag MGD77 that have depth but
not twt, try

    x2sys\_get -V -TMGD77 -Fdepth -Nwt

To find all the pairs associated with the tag MGD77 that might intersect
each other, but only those pairs which involves tracks in your list
new.lis, try

    x2sys\_get -V -TMGD77 -Lnew.lis > xpairs.lis

`Note <#toc7>`_
---------------

The tracks that are returned all have the requested data (**-F**) within
the specified region (**-R**). Furthermore, the columns of Y and N for
other data types also reflect the content of the track portion within
the selected region, unless **-G** is set.

`See Also <#toc8>`_
-------------------

`x2sys\_binlist <x2sys_binlist.html>`_,
`x2sys\_cross <x2sys_cross.html>`_ 
`x2sys\_datalist <x2sys_datalist.html>`_,
`x2sys\_init <x2sys_init.html>`_,
`x2sys\_list <x2sys_list.html>`_,
`x2sys\_put <x2sys_put.html>`_,
`x2sys\_report <x2sys_report.html>`_,
`x2sys\_solve <x2sys_solve.html>`_
