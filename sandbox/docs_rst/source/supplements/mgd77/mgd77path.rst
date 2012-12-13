*********
mgd77path
*********

mgd77path - Return paths to MGD77 cruises and directories

`Synopsis <#toc1>`_
-------------------

**mgd77path** *NGDC-ids* [ **-A**\ [**-**\ ] ] [ **-D** ] [
**-I**\ *ignore* ] [ **-V**\ [*level*\ ] ]

`Description <#toc2>`_
----------------------

**mgd77path** returns the full pathname to one or more MGD77 files. The
pathname returned for a given cruise may change with time due to
reshuffling of disks/subdirectories.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*NGDC-ids*
    Can be one or more of five kinds of specifiers:

    1) 8-character NGDC IDs, e.g., 01010083, JA010010etc., etc.

    2) 2-character <agency> codes which will return all cruises from
    each agency.

    3) 4-character <agency><vessel> codes, which will return all cruises
    from those vessels.

    4) =<list>, where <list> is a table with NGDC IDs, one per line.

    5) If nothing is specified we return all cruises in the data base.

    (See mgd77info **-L** for agency and vessel codes). The ".mgd77" or
    ".nc" extensions will automatically be appended, if needed (use
    **-I** to ignore certain file types). Cruise files will be looked
    for first in the current directory and second in all directories
    listed in **$MGD77\_HOME**/mgd77\_paths.txt [If **$MGD77\_HOME** is
    not set it will default to **$GMT\_SHAREDIR**/mgd77].

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [**-**\ ]
    Display the full path to each cruise [Default]. Optionally, append
    **-** which will list just the cruise IDs instead.
**-D**
    Instead of cruise listings, just show the directory paths currently
    used in the search.
**-I**\ *ignore*
    Ignore certain data file formats from consideration. Append
    **a\|c\|t** to ignore MGD77 ASCII, MGD77+ netCDF, or plain
    tab-separated ASCII table files, respectively. The option may be
    repeated to ignore more than one format. [Default ignores none].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c]. Reports the total number of cruises
    found.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc6>`_
-------------------

To obtain pathnames for cruises 01010008 and 01010007, run

mgd77path 01010008 01010007

To obtain pathnames for cruises 01010008 and 01010007, but only if there
are MGD77+ version in netCDF, run

mgd77path 01010008 01010007 -Ia -It

To see the list of active directories where MGD77 files might be stored,
run

mgd77path -D

`See Also <#toc7>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ `*mgd77info*\ (1) <mgd77info.html>`_
`*mgd77list*\ (1) <mgd77list.html>`_
`*mgd77manage*\ (1) <mgd77manage.html>`_
`*mgd77track*\ (1) <mgd77track.html>`_

`References <#toc8>`_
---------------------

Wessel, P., W. H. F. Smith, R. Scharroo, and J. Luis, 2011, The Generic
Mapping Tools (GMT) version 5.0.0b Technical Reference & Cookbook,
SOEST/NOAA.

Wessel, P., and W. H. F. Smith, 1998, New, Improved Version of Generic
Mapping Tools Released, EOS Trans., AGU, 79 (47), p. 579.

Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic Mapping
Tools Released, EOS Trans., AGU, 76 (33), p. 329.

Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic Mapping
Tools Released,
`http://www.agu.org/eos\_elec/95154e.html, <http://www.agu.org/eos_elec/95154e.html,>`_
Copyright 1995 by the American Geophysical Union.

Wessel, P., and W. H. F. Smith, 1991, Free Software Helps Map and
Display Data, EOS Trans., AGU, 72 (41), p. 441.

The Marine Geophysical Data Exchange Format - MGD77, see
`*http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt*. <http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt.>`_
