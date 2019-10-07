.. index:: ! mgd77manage

***********
mgd77manage
***********

.. only:: not man

    mgd77manage - Manage the content of MGD77+ files

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77manage** *NGDC-ids*
[ |-A|\ **a**\ \|\ **c**\ \|\ **d**\ \|\ **D**\ \|\ **e**\ \|\ **E**\ \|\ **g**\ \|\ **i**\ \|\ **n**\ \|\ **t**\ \|\ **T**\ *fileinfo*\ [**+f**]\ ] 
[ |-D|\ *abbrev1*,\ *abbrev2*,...) ]
[ |-E|\ *empty* ]
[ |-F| ]
[ |-I|\ *abbrev*/*name*/*unit*/**t**/*scale*/*offset*/*comment* ]
[ |-N|\ *unit* ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-j| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mgd77manage** deals with maintaining extra custom columns in MGD77+
netCDF files. You can either delete one or more columns, add a new
column, update an existing column with new data, or supply error
correction information (\*.e77 files). New data may come from a table
(ASCII unless **-bi** is used), be based on
existing columns and certain theoretical expressions, or they may be
obtained by sampling a grid (choose between GMT grid or a Sandwell/Smith
Mercator \*.img grid) along track. The new data will be appended to the
MGD77+ file in the form of an extra data column of specified type. The
data file will be modified; no new file will be created. For the big
issues, see the DISCUSSION section below. 

Required Arguments
------------------

.. include:: explain_ncid.rst_


Optional Arguments
------------------

.. _-A:

**-A**\ **a**\ \|\ **c**\ \|\ **d**\ \|\ **D**\ \|\ **e**\ \|\ **E**\ \|\ **g**\ \|\ **i**\ \|\ **n**\ \|\ **t**\ \|\ **T**\ *fileinfo*\ [**+f**\ ]
    Add a new data column. If an existing column with the same
    abbreviation already exists in the file we will cowardly refuse to
    update the file. Append **+f** to overcome this reluctance
    (However, sometimes an existing column cannot be upgraded without
    first deleting it; if so you will be warned). Select a column source
    code among **a**, **c**, **d**, **D**, **e**, **g**, **i**, **n**,
    **t**, or **T**; detailed descriptions for each choice follow:

    **a** Append filename of a single column table to add. File must
    have the same number of rows as the MGD77+ file. If no file is given
    we read from stdin instead.

    **c** Create a new column that derives from existing data or
    formulas for corrections and reference fields. Append **c** for the
    Carter corrections subtracted from uncorrected depths, **g** for the
    IGF gravity reference field (a.k.a "normal gravity"), **m** for the
    IGRF total field magnetic reference field, and **r** for recomputed
    magnetic anomaly (append 1 or 2 to specify which total field column
    to use [1]). For gravity we choose the reference field based on the
    parameter Gravity Theoretical Formula Code in the cruise's MGD77
    header. If this is not set or is invalid we default to the IGF 1980.
    You can override this behavior by appending the desired code: 1 =
    Heiskanen 1924, 2 = International 1930, 3 = IGF1967, or 4 = IGF1980.

    **d** Append filename of a two-column table with the first column
    holding distances along track and the second column holding data
    values. If no file is given we read from stdin instead. Records with
    matching distances in the MGD77+ file will be assigned the new
    values; at other distances we set them to NaN. Alternatively, give
    upper case **D** instead and we will interpolate the column at all
    record distances. See **-N** for choosing distance units and **-j**
    for choosing how distances are calculated.

    **e** Expects to find an e77 error/correction log from
    :doc:`mgd77sniffer` with the name *NGDC_ID*.e77 in the current
    directory or in $MGD77_HOME/E77; this file will examined and used
    to make modifications to the header values, specify a systematic
    correction for certain columns (such as scale and offset), specify
    that a certain anomaly should be recalculated from the observations
    (e.g., recalculate mag from mtf1 and the latest IGRF), and add or
    update the special column **flag** which may hold bitflags (0 =
    GOOD, 1 = BAD) for each data field in the standard MGD77 data set.
    Any fixed correction terms found (such as needing to scale a field
    by 0.1 or 10 because the source agency used incorrect units) will be
    written as attributes to the netCDF MGD77+ file and applied when the
    data are read by :doc:`mgd77list`. Ephemeral corrections such as those
    determined by crossover analysis are not kept in the data files but
    reside in correction tables (see :doc:`mgd77list` for details). By
    default, the first character of each header line in the e77 file
    (which is ?, Y or N) will be consulted to see if the corresponding
    adjustment should be applied. If any undecided settings are found
    (i.i, ?) we will abort and make no changes. Only records marked Y
    will be processed. You can override this behavior by appending one
    or more modifiers to the **-Ae** command: **h** will ignore all
    header corrections, **f** will ignore all fixed systematic trend
    corrections, **n**, **v**, and **s** will ignore bitflags pertaining
    to navigation, data values, and data slopes, respectively. Use
    **-Ae+f** to replace any existing E77 corrections in the file with
    the new values. Finally, e77 corrections will not be applied if the
    E77 file has not been verified. Use **-AE** to ignore the
    verification status.

    **g** Sample a GMT geographic (lon, lat) grid along the track given
    by the MGD77+ file using bicubic interpolation (however, see
    **-n**). Append name of a GMT grid file.

    **i** Sample a Sandwell/Smith Mercator \*.img grid along the track
    given by the MGD77+ file using bicubic interpolation (however, see
    **-n**). Append the img grid filename, followed by the
    comma-separated data scale (typically 1 or 0.1), the IMG file mode
    (0-3), and optionally the img grid max latitude [80.738]. The modes
    stand for the following: (0) Img files with no constraint code,
    returns data at all points, (1) Img file with constraints coded,
    return data at all points, (2) Img file with constraints coded,
    return data only at constrained points and NaN elsewhere, and
    (3) Img file with constraints coded, return 1 at constraints and 0 elsewhere.

    **n** Append filename of a two-column table with the first column
    holding the record number (0 to nrows - 1) and the second column
    holding data values. If no file is given we read from stdin instead.
    Records with matching record numbers in the MGD77+ file will be
    assigned the new values; at other records we set them to NaN.

    **t** Append filename of a two-column table with the first column
    holding absolute times along track and the second column holding
    data values. If no file is given we read from stdin instead. Records
    with matching times in the MGD77+ file will be assigned the new
    values; at other times we set them to NaN. Alternatively, give upper
    case **T** instead and we will interpolate the column at all record
    times.

.. _-D:

**-D**\ *abbrev1*,\ *abbrev2*,...)
    Give a comma-separated list of column abbreviations that you want to
    delete from the MGD77+ files. Do NOT use this option to remove
    columns that you are replacing with new data (use **-A...+f** instead).
    Because we cannot remove variables from netCDF files we must create
    a new file without the columns to be deleted. Once the file is
    successfully created we temporarily rename the old file, change the
    new filename to the old filename, and finally remove the old,
    renamed file.

.. _-E:

**-E**\ *empty*
    Give a single character that will be repeated to fill empty string
    values, e.g., "9" will yield a string like "99999..." [9].

.. _-F:

**-F**
    Force mode. When this mode is active you are empowered to delete or
    replace even the standard MGD77 set of columns. You better know what
    you are doing!

.. _-I:

**-I**\ *abbrev*/*name*/*unit*/**t**/*scale*/*offset*/*comment*
    In addition to file information we must specify additional
    information about the extra column. Specify a short (16 char or
    less, using lower case letters, digits, or underscores only)
    abbreviation for the selected data, its more descriptive name, the
    data unit, the data type 1-character code (**b**\ yte, **s**\ hort,
    **f**\ loat, **i**\ nt, **d**\ ouble, or **t**\ ext) you want used
    for storage in the netCDF file, any scale and offset we should apply
    to the data to make them fit inside the range implied by the chosen
    storage type, and a general comment (< 128 characters) regarding
    what these data represent. Note: If text data type is selected then
    the terms "values" in the **-A** discussion refer to your text data.
    Furthermore, the discussion on interpolation does not apply and the
    NaN value becomes a "no string" value (see **-E** for what this is).
    Place quotes around terms with more than one word (e.g., "Corrected Depth").

.. _-N:

**-N**\ *unit*
    Append the distance unit (see :ref:`Unit_attributes`). [Default is **-Nk** (km)].
    Only relevant when **-Ag**\ \|\ **i** is selected. 

.. _-R:

.. |Add_-R| replace:: Only relevant when **-Ag**\ \|\ **i** is selected. 
.. include:: ../../explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_
    
.. |Add_-bi| replace:: 
    This applies to the input 1- or 2-column data files specified under some
    of the **-A** options. The binary input option is only available for
    numerical data columns. 
.. include:: ../../explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. include:: ../../explain_distcalc.rst_

.. include:: ../../explain_-n.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_distunits.rst_

.. include:: ../../explain_grdresample2.rst_

Examples
--------

To append Geosat/ERS-1 gravity version 11.2 as an extra data column in
the cruises 01010047.nc and 01010008.nc, storing the values as mGal\*10
in a 2-byte short integer, try

   ::

    gmt mgd77manage 01010047 01010008 -Ai10/1/grav.11.2.img \
        -Isatgrav/"Geosat/ERS-1 gravity"/"mGal"/s/10/0/"Sandwell/Smith version 11.2" -V

To append a filtered version of magnetics as an extra data column of
type float for the cruise 01010047.nc, and interpolate the filtered data
at the times given in the MGD77+ file, try

   ::

    gmt mgd77manage 01010047 -ATmymag.tm -Ifiltmag/"Intermediate-wavelength \
        magnetic residuals"/"nTesla"/f/1/0/"Useful for looking for isochrons" -V

To delete the existing extra columns satfaa, coastdist, and satvgg from
all MGD77+ files, try

   ::

    gmt mgd77manage =allmgd77.lis -Dsatfaa,coastdist,satvgg -V

To create a 4-byte float column with the correct IGRF reference field in
all MGD77+ files, try

   ::

    gmt mgd77manage =allmgd77.lis -Acm -Iigrf/"IGRF reference \
        field"/"nTesla"/f/1/0/"IGRF version 10 for 1990-2010" -V

Discussion
----------

**1. Preamble**

The mgd77 supplement is an attempt to (1) improve on the
limited functionality of the existing mgg supplement, (2) incorporate
some of the ideas from Scripps' gmt+ supplement by allowing extra data
columns, and (3) add new capabilities for managing marine
geophysical trackline data stored in an architecture-independent CF-1.0-
and COARDS-compliant netCDF file format. Here are some of the underlying
ideas and steps you need to take to maintain your files.

**2. Introduction**

Our starting point is the MGD77 ASCII data files distributed from NGDC
on CD-ROMS, DVD-ROMS, and via FTP. Using Geodas to install the files
locally we choose the "Carter corrected depth" option which will fill in
the depth column using the two-way travel-times and the Carter tables if
twt is present. This step yields ~5000 individual cruise files. Place
these in one or more sub-directories of your choice, list these
sub-directories (one per line) in the file mgd77_paths.txt, and place
that file in the directory pointed to by **$MGD77_HOME**; if not set
this variable defaults to **$GMT_SHAREDIR**/mgd77.

**3. Conversion**

Convert the ASCII MGD77 files to the new netCDF MGD77+ format using
:doc:`mgd77convert`. Typically, you will make a list of all the cruises to
be converted (with or without extension), and you then run

    mgd77convert =cruises.lis -Fa -Tc -V -Lwe+l > log.txt

The verbose settings will ensure that all problems found during
conversion will be reported. The new \*.nc files may also be placed in
one or more separate sub-directories and these should also be listed in
the mgd77_paths.txt file. We suggest you place the directories with
\*.nc files ahead of the \*.mgd77 directories. When you later want to
limit a search to files of a certain extension you should use the **-I**
option.

**4. Adding new columns**

**mgd77manage** will allow you to add additional data columns to your
\*.nc files. These can be anything, including text strings, but most
likely are numerical values sampled along the track from a supplied grid
or an existing column that have been filtered or manipulated for a
particular purpose. The format supports up to 32 such extra columns. See
this man page for how to add columns. You may later decide to remove
some of these columns or update the data associated with a certain
column. Data extraction tools such as :doc:`mgd77list` can be used to
extract a mix of standard MGD77 columns (navigation, time, and the usual
geophysical observations) and your custom columns.

**5. Error sources**

Before we discuss how to correct errors we will first list the different
classes of errors associated with MGD77 data: (1) Header record errors
occur when some of the information fields in the header do not comply
with the MGD77 specification or required information is missing.
:doc:`mgd77convert` will list these errors when the extended verbose
setting is selected. These errors typically do not affect the data and
are instead errors in the *meta-data* (2). Fixed
systematic errors occur when a particular data column, despite the MGD77
specification, has been encoded incorrectly. This usually means the data
will be off by a constant factor such as 10 or 0.1, or in some cases
even 1.8288 which converts fathoms to meters. (3) Unknown systematic
errors occur when the instrument that recorded the data or the
processing that followed introduced signals that appear to be systematic
functions of time along track, latitude, heading, or some other
combination of terms that have a physical or logical explanation. These
terms may sometimes be resolved by data analysis techniques such as
along-track and across-track investigations, and will result in
correction terms that when applied to the data will remove these
unwanted signals in an optimal way. Because these correction terms may
change when new data are considered in their determination, such
corrections are considered to be ephemeral. (4) Individual data points
or sequences of data may violate rules such as being outside of possible
ranges or in other ways violate sanity. Furthermore, sequences of points
that may be within valid ranges may give rise to data gradients that are
unreasonable. The status of every point can therefore be determined and
this gives rise to bitflags GOOD or BAD. Our policy is that error
sources 1, 2, and 4 will be corrected by supplying the information as
meta-data in the relevant \*.nc files, whereas the corrections for error
source 3 (because they will constantly be improved) will be maintained
in a separate list of corrections.

**6. Finding errors**

The :doc:`mgd77sniffer` is a tool that does a thorough along-track sanity
check of the original MGD77 ASCII files and produces a corresponding
\*.e77 error log. All problems found are encoded in the error log, and
recommended fixed correction terms are given, if needed. An analyst may
verify that the suggested corrections are indeed valid (we only want to
correct truly obvious unit errors), edit these error logs and modify
such correction terms and activate them by changing the relevant code
key (see :doc:`mgd77sniffer` for more details). **mgd77manage** can ingest
these error logs and (1) correct bad header records given
the suggestions in the log, (2) insert scale/offset correction terms to
be used when reading certain columns, and (3) insert any
bit-flags found. Rerun this step if you later find other problems as all
E77 settings or flags will be recreated based on the latest E77 log.

**7. Error corrections**

The extraction program :doc:`mgd77list` allows for corrections to be
applied on-the-fly when data are requested. First, data with BAD
bitflags are suppressed. Second, data with fixed systematic correction
terms are corrected accordingly. Third, data with ephemeral correction
terms will have those corrections applied (if a correction table is
supplied). All of these steps require the presence of the relevant
meta-data and all can be overruled by the user. In addition, users may
add their own bitflags as separate data columns and use
:doc:`mgd77list`'s logical tests to further dictate which data are
suppressed from output.

Credits
-------

The IGRF calculations are based on a Fortran program written by Susan
Macmillan, British Geological Survey, translated to C via f2c by Joaquim
Luis, and adapted to GMT style by Paul Wessel.

See Also
--------

:doc:`mgd77convert`,
:doc:`mgd77list`,
:doc:`mgd77info`,
:doc:`mgd77sniffer`
:doc:`mgd77track`
:doc:`x2sys_init <../x2sys/x2sys_init>`

References
----------

The Marine Geophysical Data Exchange Format - MGD77, see
`<http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt>`_

IGRF, see `<http://www.ngdc.noaa.gov/IAGA/vmod/igrf.html>`_
