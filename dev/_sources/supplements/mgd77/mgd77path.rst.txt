.. index:: ! mgd77path
.. include:: ../module_supplements_purpose.rst_

*********
mgd77path
*********

|mgd77path_purpose|

Synopsis
-------------------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77path** *GEODAS-ids* [ |-A|\ [**c**] ] [ |-D| ]
[ |-I|\ **a\|c\|m\|t** ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mgd77path** returns the full pathname to one or more MGD77 files. The
pathname returned for a given cruise may change with time due to
reshuffling of disks/subdirectories.

Required Arguments
------------------

.. include:: explain_ncid.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [**c**]
    Display the full path to each cruise [Default]. Optionally, append
    **c** which will list just the cruise IDs instead.

.. _-D:

**-D**
    Instead of cruise listings, just show the directory paths currently
    used in the search.

.. _-I:

**-I**\ **a\|c\|m\|t**
    Ignore certain data file formats from consideration. Append
    **a\|c\|m\|t** to ignore MGD77 ASCII, MGD77+ netCDF, MGD77T ASCII, or plain
    tab-separated ASCII table files, respectively. The option may be
    repeated to ignore more than one format. [Default ignores none].

.. |Add_-V| replace:: Reports the total number of cruises found. |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: ../../explain_help.rst_

Examples
--------

To obtain pathnames for cruises 01010008 and 01010007, run

::

  gmt mgd77path 01010008 01010007

To obtain pathnames for cruises 01010008 and 01010007, but only if there
are MGD77+ version in netCDF, run

::

  gmt mgd77path 01010008 01010007 -Ia -It

To see the list of active directories where MGD77 files might be stored, run

::

  gmt mgd77path -D

See Also
--------

:doc:`gmt </gmt>` :doc:`mgd77info`
:doc:`mgd77list`
:doc:`mgd77manage`
:doc:`mgd77track`

References
----------

The Marine Geophysical Data Exchange Format - MGD77, see
`<https://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt>`_.

Wessel, P., and Chandler, M. T., 2007, The mgd77 supplement to the Generic Mapping Tools,
*Comp. Geosci.*, **33**\ (1), 62-75, https://doi.org/10.1016/j.cageo.2006.05.006.
