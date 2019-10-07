.. index:: ! mgd77convert

**************
mgd77convert
**************

.. only:: not man

    mgd77convert - Convert MGD77 data to other file formats

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77convert** *NGDC-ids* |-F|\ **a**\ \|\ **c**\ \|\ **m**\ \|\ **t**
|-T|\ **a**\ \|\ **c**\ \|\ **m**\ \|\ **t**\ [**+f**]
[ |-C| ]
[ |-D| ]
[ |-L|\ [**w**][**e**][**+l**] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mgd77convert** reads versions of MGD77 files and writes the same data
in (probably) another format to a new file in the current directory.
Both pre- and post-Y2K MGD77 formats can be processed.

Required Arguments
------------------

.. include:: explain_ncid.rst_

.. _-F:

**-Fa**\ \|\ **c**\ \|\ **m**\ \|\ **t**
    Specifies the format of the input (From) files. Choose from **a**
    for standard MGD77 ASCII table (with extension .mgd77), **c** for
    the new MGD77+ netCDF format (with extension .nc), **m** for the
    new MGD77T format (extension .m77t) and **t** for a
    plain ASCII tab-separated table dump (with extension .dat). Use
    **-FC** to recover the original MGD77 setting from the MGD77+ file
    [Default will apply any E77 corrections encoded in the file].

.. _-T:

**-T**\ **a**\ \|\ **c**\ \|\ **m**\ \|\ **t**\ [**+f**]
    Specifies the format of the output (To) files. Choose from **a** for
    standard MGD77 ASCII table (with extension .mgd77), **c** for the
    new MGD77+ netCDF format (with extension .nc), **m** for the
    new MGD77T format (extension .m77t) and **t** for a plain
    ASCII tab-separated table dump (with extension .dat). We will refuse
    to create the file(s) if they already exist in the current
    directory. Append **+f** to force an override of this policy.

Optional Arguments
------------------

.. _-C:

**-C**
    Convert from NGDC two-file data sets \*.h77, \*.a77 to single file
    \*.mgd77. No other options (except **-V**) are allowed. Give one or
    more names of \*.h77 files, \*.a77 files, or just the file prefixes.

.. _-D:

**-D**
    By default, the storage types used in a MGD77+ netCDF file greatly
    exceed the precision imposed by the ASCII MGD77 format. However, for
    the five items **faa**, **eot**, **mag**, **diur** and **msd** we
    use 2-byte integers with implied precisions of 0.1 mGal, 0.1 nTesla,
    and 1 m as in the MGD77 format. It is possible that at some point
    these items will need to be stored as 4-byte ints which would allow
    precisions of 10 fTesla, 1 nGal, and 0.01 mm, respectively. This
    option activates such storage [Default uses 2-byte integers].

.. _-L:

**-L**\ [**w**\ ][**e**\ ][**+l**\ ]
    Set the level of verification reporting [none] and where to send
    such reports [stderr]. Append a combination of **w** for warnings and
    **e** for errors, and append **+l** to send such log information to stdout.
 
.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_
    
.. include:: ../../explain_help.rst_

Examples
--------

To convert a large set of a77,h77 pairs to proper mgd77 files, try

   ::

    gmt mgd77convert -C *.h77

To convert 01010047.mgd77 and 01010008.mgd77 to new netCDF .nc files,
and capture all verification messages, try

   ::

    gmt mgd77convert 01010047 01010008 -Fa -Tc -V -Lew+l > log.lis

To convert 01010047.nc back to MGD77 ASCII and make sure it is identical
to the original file, try (Bourne shell syntax)

   ::

    orig=`gmt mgd77path 01010047 -Ic`
    gmt mgd77convert 01010047 -Fc -Ta -V
    diff $orig 01010047.mgd77

To convert 01010047.nc to a plain ASCII table for manual editing,
overwriting any existing table, try

   ::

    gmt mgd77convert 01010047 -Fc -Tt+f -V

To recover the original NGDC MGD77 version of 01020051.nc and ignore any
E77 corrections, use

   ::

    gmt mgd77convert 01020051 -FC -Ta -V

File Formats
------------

**mgd77convert** handles four different formats.
(1) NGDC has now started to use the new tab-delimited version of the
MGD77 data format, called MGD77T.  In addition to all the info in old
MGD77 files it contains a few more quality flags for grav, mag,
and bathymetry.  (2) The MGD77+ netCDF format was developed to
facilitate the use of MGD77 data by scientists. It contains all the
information of the original MGD77 file and if you convert back and forth
you end up with the original. However, file sizes are typically ~30% of
the original ASCII format and is much faster to operate on. (3) The MGD77 ASCII
tables are the traditional standard for distribution of underway
geophysical data to and from the NGDC data center, now superseded by MGD77T.
Normally, only the ship-operations people and the cruise PI might be involved
in *making* an MGD77 ASCII file for transmission to NGDC; users are more interested
in *reading* such files. (4) The plain ASCII tab-separated dump is available
for users who need to manually edit the content of a MGD77 file.
This is usually easier to do when the columns are tab-separated than when
they are all crunched together in the MGD77 punch-card format.

Other Tools
-----------

The MGD77+ netCDF files are CF-1.0 and COARDS compliant and can be
examined with general-purpose tools such as ncBrowse and ncView.

See Also
--------

:doc:`mgd77manage`,
:doc:`mgd77list`,
:doc:`mgd77info`,
:doc:`mgd77track`
:doc:`x2sys_init <../x2sys/x2sys_init>`

References
----------

ncBrowse, see `<http://www.epic.noaa.gov/java/ncBrowse/>`_
ncView, see `<http://meteora.ucsd.edu/~pierce/ncview_home_page/>`_
The Marine Geophysical Data Exchange Format - "MGD77", see
`<http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt>`_
