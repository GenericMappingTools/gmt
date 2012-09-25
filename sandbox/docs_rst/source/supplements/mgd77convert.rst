**************
mgd77convert
**************

mgd77convert - Convert MGD77 data to other file formats

`Synopsis <#toc1>`_
-------------------

**mgd77convert** *NGDC-ids* **-Fa**\ \|\ **c**\ \|\ **t**
**-T**\ [**+**\ ]\ **a**\ \|\ **b**\ \|\ **t** [ **-C** ] [ **-D** ] [
**-L**\ [**w**\ ][**e**\ ][**+**\ ] ] [ **-V**\ [*level*\ ] ]

`Description <#toc2>`_
----------------------

**mgd77convert** reads versions of MGD77 files and writes the same data
in (probably) another format to a new file in the current directory.
Both pre- and post-Y2K MGD77 formats can be processed.

`Required Arguments <#toc3>`_
-----------------------------

*NGDC-ids*
    Can be one or more of five kinds of specifiers:
     1) 8-character NGDC IDs, e.g., 01010083, JA010010etc., etc.
     2) 2-character <agency> codes which will return all cruises from
    each agency.
     3) 4-character <agency><vessel> codes, which will return all
    cruises from those vessels.
     4) =<list>, where <list> is a table with NGDC IDs, one per line.
     5) If nothing is specified we return all cruises in the data base.
     (See mgd77info **-L** for agency and vessel codes). The ".mgd77" or
    ".nc" extensions will automatically be appended, if needed (use
    **-I** to ignore certain file types). Cruise files will be looked
    for first in the current directory and second in all directories
    listed in **$MGD77\_HOME**/mgd77\_paths.txt [If **$MGD77\_HOME** is
    not set it will default to **$GMT\_SHAREDIR**/mgd77].
**-Fa**\ \|\ **c**\ \|\ **t**
    Specifies the format of the input (From) files. Choose from **a**
    for standard MGD77 ASCII table (with extension .mgd77), **c** for
    the new MGD77+ netCDF format (with extension .nc), and **t** for a
    plain ASCII tab-separated table dump (with extension .dat). Use
    **-FC** to recover the original MGD77 setting from the MGD77+ file
    [Default will apply any E77 corrections encoded in the file].
**-T**\ [**+**\ ]\ **a**\ \|\ **b**\ \|\ **t**
    Specifies the format of the output (To) files. Choose from **a** for
    standard MGD77 ASCII table (with extension .mgd77), **c** for the
    new MGD77+ netCDF format (with extension .nc), and **t** for a plain
    ASCII tab-separated table dump (with extension .dat). We will refuse
    to create the file(s) if they already exist in the current
    directory. Prepend **+** to override this policy.

`Optional Arguments <#toc4>`_
-----------------------------

**-C**
    Convert from NGDC two-file data sets \*.h77, \*.a77 to single file
    \*.mgd77. No other options (except **-V**) are allowed. Give one or
    more names of \*.h77 files, \*.a77 files, or just the file prefixes.
**-D**
    By default, the storage types used in a MGD77+ netCDF file greatly
    exceed the precision imposed by the ASCII MGD77 format. However, for
    the five items **faa**, **eot**, **mag**, **diur** and **msd** we
    use 2-byte integers with implied precisions of 0.1 mGal, 0.1 nTesla,
    and 1 m as in the MGD77 format. It is possible that at some point
    these items will need to be stored as 4-byte ints which would allow
    precisions of 10 fTesla, 1 nGal, and 0.01 mm, respectively. This
    option activates such storage [Default uses 2-byte integers].
**-L**\ [**w**\ ][**e**\ ][**+**\ ]
    Set the level of verification reporting [none] and where to send
    such reports [stderr]. Append a combination of **w** for warnings,
    **e** for errors, and **+** to send such log information to stdout.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc5>`_
-------------------

To convert a large set of a77,h77 pairs to proper mgd77 files, try

mgd77convert -C \*.h77

To convert 01010047.mgd77 and 01010008.mgd77 to new netCDF .nc files,
and capture all verification messages, try

mgd77convert 01010047 01010008 -Fa -Tc -V -Lew+ > log.lis

To convert 01010047.nc back to MGD77 ASCII and make sure it is identical
to the original file, try (Bourne shell syntax)

orig=‘mgd77path 01010047 -Ic‘

mgd77convert 01010047 -Fc -Ta -V

diff $orig 01010047.mgd77

To convert 01010047.nc to a plain ASCII table for manual editing,
overwriting any existing table, try

mgd77convert 01010047 -Fc -T+t -V

To recover the original NGDC MGD77 version of 01020051.nc and ignore any
E77 corrections, use

mgd77convert 01020051 -FC -Ta -V

`File Formats <#toc6>`_
-----------------------

**mgd77convert** handles three different formats. (1) The MGD77 ASCII
tables are the established standard for distribution of underway
geophysical data to and from the NGDC data center. Normally, only the
ship-operations people and the cruise PI might be involved in *making*
an MGD77 ASCII file for transmission to NGDC; users are more interested
in *reading* such files. (2) The MGD77+ netCDF format was developed to
fascilitate the use of MGD77 data by scientists. It contains all the
information of the original MGD77 file and if you convert back and forth
you end up with the original. However, file sizes are typically ~30% of
the original ASCII format and is much faster to operate on. (3) The
plain ASCII tab-separated dump is available for users who need to
manually edit the content of a MGD77 file. This is usually easier to do
when the columns are tab-separated than when they are all crunched
together in the MGD77 punch-card format.

`Other Tools <#toc7>`_
----------------------

The MGD77+ netCDF files are CF-1.0 and COARDS compliant and can be
examined with general-purpose tools such as ncBrowse and ncView.

`See Also <#toc8>`_
-------------------

`*mgd77manage*\ (1) <mgd77manage.html>`_ ,
`*mgd77list*\ (1) <mgd77list.html>`_ ,
`*mgd77sample*\ (1) <mgd77sample.html>`_ ,
`*mgd77track*\ (1) <mgd77track.html>`_
`*x2sys\_init*\ (1) <x2sys_init.html>`_

`References <#toc9>`_
---------------------

Wessel, P., W. H. F. Smith, R. Scharroo, and J. Luis, 2011, The Generic
Mapping Tools (GMT) version 5.0.0b Technical Reference & Cookbook,
SOEST/NOAA.
 Wessel, P., and W. H. F. Smith, 1998, New, Improved Version of Generic
Mapping Tools Released, EOS Trans., AGU, 79 (47), p. 579.
 Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic
Mapping Tools Released, EOS Trans., AGU, 76 (33), p. 329.
 Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic
Mapping Tools Released,
`http://www.agu.org/eos\_elec/95154e.html, <http://www.agu.org/eos_elec/95154e.html,>`_
Copyright 1995 by the American Geophysical Union.
 Wessel, P., and W. H. F. Smith, 1991, Free Software Helps Map and
Display Data, EOS Trans., AGU, 72 (41), p. 441.
 ncBrowse, see
`*http://www.epic.noaa.gov/java/ncBrowse/* <http://www.epic.noaa.gov/java/ncBrowse/>`_
 ncView, see
`*http://meteora.ucsd.edu/~pierce/ncview\_home\_page.html* <http://meteora.ucsd.edu/~pierce/ncview_home_page.html>`_
 The Marine Geophysical Data Exchange Format - "MGD77", see
`*http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt* <http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt>`_

