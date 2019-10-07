.. index:: ! x2sys_init

**********
x2sys_init
**********

.. only:: not man

    x2sys_init - Initialize a new x2sys track database

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_init** *TAG* |-D|\ *fmtfile*
[ |-E|\ *suffix* ]
[ |-F| ]
[ |-G|\ **d**\ \|\ **g** ]
[ |-I|\ *dx*\ [/*dy*] ]
[ |-N|\ **d**\ \|\ **s**\ *unit* ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-W|\ **t**\ \|\ **d**\ *gap* ]
[ |SYN_OPT-j| ] 
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**x2sys_init** is the starting point for anyone wishing to use x2sys;
it initializes a set of data bases that are particular to one kind of
track data. These data, their associated data bases, and key parameters
are given a short-hand notation called an x2sys TAG. The TAG keeps track
of settings such as file format, whether the data are geographic or not,
and the binning resolution for track indices. Running **x2sys_init** is
a prerequisite to running any of the other x2sys programs, such as
:doc:`x2sys_binlist`, which will create a crude representation of where
each data track go within the domain and which observations are
available; this information serves as input to :doc:`x2sys_put` which
updates the track data base. Then, :doc:`x2sys_get` can be used to find
which tracks and data are available inside a given region. With that
list of tracks you can use :doc:`x2sys_cross` to calculate track
crossovers, use :doc:`x2sys_report` to report crossover statistics or
:doc:`x2sys_list` to pull out selected crossover information that
:doc:`x2sys_solve` can use to determine track-specific systematic
corrections. These corrections may be used with :doc:`x2sys_datalist` to
extract corrected data values for use in subsequent work.  Because you
can run **x2sys_init** you *must* set the environmental parameter
X2SYS_HOME to a directory where you have write permission, which  is
where x2sys can keep track of your settings.

Required Arguments
------------------

*TAG*
    The unique name of this data type x2sys TAG.

.. _-D:

**-D**\ *fmtfile*
    Format definition file prefix for this data set [See Format Definition Files below
    for more information]. Specify full path if the file is not in the
    current directory.

Optional Arguments
------------------

.. _-E:

**-E**\ *suffix*
    Specifies the file extension (suffix) for these data files. If not
    given we use the format definition file prefix as the suffix (see **-D**).

.. _-F:

**-F**
    Force creating new files if old ones are present [Default will abort
    if old TAG files are found].

.. _-G:

**-Gd**\ \|\ **g**
    Selects geographical coordinates. Append **d** for discontinuity at
    the Dateline (makes longitude go from -180 to + 180) or **g** for
    discontinuity at Greenwich (makes longitude go from 0 to 360
    [Default]). If not given we assume the data are Cartesian.

.. _-I:

**-I**\ *dx*\ [/*dy*]
    *x_inc* [and optionally *y_inc*] is the grid spacing. Append **m**
    to indicate minutes or **s** to indicate seconds for geographic
    data. These spacings refer to the binning used in the track
    bin-index data base.

.. _-N:

**-Nd**\ \|\ **s**\ *unit*
    Sets the units used for distance and speed when requested by other
    programs. Append **d** for distance or **s** for speed, then give
    the desired *unit* as **c** (Cartesian userdist or
    userdist/usertime), **e** (meters or m/s), **f** (feet or feet/s),
    **k** (km or kms/hr), **m** (miles or miles/hr), **n** (nautical
    miles or knots) or **u** (survey feet or survey feet/s). [Default is
    **-Ndk** **-Nse** (km and m/s) if **-G** is set and **-Ndc** and
    **-Nsc** otherwise (Cartesian units)].

.. _-R:

.. |Add_-Rgeo| replace:: For Cartesian
    data just give *xmin/xmax/ymin/ymax*. This option bases the
    statistics on those COE that fall inside the specified domain.
.. include:: ../../explain_-Rgeo.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

**-Wt**\ \|\ **d**\ *gap*
    Give **t** or **d** and append the corresponding maximum time gap
    (in user units; this is typically seconds [Infinity]), or distance
    (for units, see |-N|) gap [Infinity]) allowed between the two data
    points immediately on either side of a crossover. If these limits
    are exceeded then a data gap is assumed and no COE will be determined.

.. include:: ../../explain_distcalc.rst_

.. include:: ../../explain_help.rst_

Format Definition Files
-----------------------

These \*.fmt files contain information about the data file format and
have two sections: (1) header information and (2) column
information. All header information starts with the character # in the
first column, immediately followed by an upper-case directive. If the
directive takes an argument it is separated by white-space. You may
append a trailing # comments. Five directives are recognized:

**ASCII** states that the data files are in ASCII format.

**BINARY** states that the data files are native binary files.

**NETCDF** states that the data files are COARDS-compliant 1-D netCDF files.

**SKIP** takes an integer argument which is either the number of lines
to skip (when reading ASCII files) or the number of bytes to skip (when
reading native binary files). Not used with netCDF files.

**GEO** indicates that these files are geographic data sets, with
periodicities in the *x*-coordinate (longitudes). Alternatively, use **-G**.

**MULTISEG** means each track consists of multiple segments separated by
a GMT segment header (alternatively, use **-m** when defining the
system TAG). Not used with netCDF files.

The column information consists of one line per column in the order the
columns appear in the data file. For each column you must provide seven
attributes:

*name type NaN NaN-proxy scale offset oformat*

*name* is the name of the column variable. You must
use the special names *lon* (or *x* if Cartesian) and *lat* (or *y*) for
the two required coordinate columns, *time* when optional absolute time data
are present, and *rtime* when relative time data are given (make sure the
GMT defaults **TIME_UNIT** and **TIME_EPOCH** are set properly). Regardless
of input time flavor, we will write absolute time on output.

*type* is always **a** for ASCII representations of numbers, whereas for
binary files you may choose among **c** for signed 1-byte character
(-127,+128), **u** for unsigned byte (0-255), **h** for signed 2-byte
integers (-32768,+32767), **i** for signed 4-byte integers
(-2,147,483,648,+2,147,483,647), **f** for 4-byte floating points and
**d** for 8-byte double precision floating points. For netCDF, simply
use **d** as netCDF will automatically handle type-conversions during reading.

*NaN* is Y if certain values (e.g, -9999) are to be replaced by NAN, and N otherwise.

*NaN-proxy* is that special value (e.g., -9999).

*scale* is used to multiply the data after reading.

*offset* is used to add to the scaled data.

*oformat* is a C-style format string used to print values from this column.

If you give - as the *oformat* then GMT's formatting machinery
will be used instead (i.e., :ref:`FORMAT_FLOAT_OUT <FORMAT_FLOAT_OUT>`,
:ref:`FORMAT_GEO_MAP <FORMAT_GEO_MAP>`, :ref:`FORMAT_DATE_MAP <FORMAT_DATE_MAP>`,
:ref:`FORMAT_CLOCK_MAP <FORMAT_CLOCK_MAP>`).
Some file formats already have definition files premade. These include
mgd77 (for plain ASCII MGD77 data files), mgd77+ (for enhanced MGD77+
netCDF files), gmt (for old mgg supplement binary files), xy (for plain
ASCII x, y tables), xyz (same, with one z-column), geo (for plain ASCII
longitude, latitude files), and geoz (same, with one z-column).

Examples
--------

If you have a large set of track data files you can organize them using
the x2sys tools. Here we will outline the steps. Let us assume that your
track data file format consist of 2 header records with text information
followed by any number of identically formatted data records with 6
columns (lat, lon, time, obs1, obs2, obs3) and that files are called
\*.trk. We will call this the "line" format. First, we create the
line.fmt file:

======  ====  ===  =========  =====  ======  ========
# Format define file for the line format
-----------------------------------------------------
# SKIP 2                      # Skip 2 header records
-----------------  ----------------------------------
# GEO                         # Data are geographic
-----------------  ----------------------------------
#name   type  NaN  NaN-proxy  scale  offset  oformat
======  ====  ===  =========  =====  ======  ========
lat      a     N     0           1     0     %9.5f
lon      a     N     0           1     0     %10.5f
time     a     N     0           1     0     %7.1f
obs1     a     N     0           1     0     %7.2f
obs2     a     N     0           1     0     %7.2f
obs3     a     N     0           1     0     %7.2f
======  ====  ===  =========  =====  ======  ========

Next we create the TAG and the TAG directory with the databases for
these line track files. Assuming these contain geographic data and that
we want to keep track of the data distribution at a 1 x 1 degree
resolution, with distances in km calculated along geodesics and with
speeds given in knots, we may run

   ::

    gmt x2sys_init LINE -V -G -Dline -Rg -je -Ndk -Nsn -I1/1 -Etrk

where we have selected LINE to be our x2sys tag. When x2sys tools try to
read your line data files they will first look in the current directory
and second look in the file *TAG*\ \_paths.txt for a list of additional
directories to examine. Therefore, create such a file (here
LINE_paths.txt) and stick the full paths to your data directories
there. All TAG-related files (format definition files, tag files, and track
data bases created) will be expected to be in the directory pointed to
by **$X2SYS_HOME**/*TAG* (in our case **$X2SYS_HOME**/LINE). Note that
the argument to **-D** must contain the full path if the \*.fmt file is
not in the current directory. **x2sys_init** will copy this file to the
**$X2SYS_HOME**/*TAG* directory where all other x2sys tools will expect
to find it.

**Create tbf file(s):**
    Once the (empty) TAG databases have been initialized we go through a
    two-step process to populate them. First we run :doc:`x2sys_binlist`
    on all our track files to create one (or more) multisegment track
    bin-index files (tbf). These contain information on which 1 x 1
    degree bins (or any other blocksize; see **-I**) each track has
    visited and which observations (in your case obs1, obs2, obs3) were
    actually observed (not all tracks may have all three kinds of
    observations everywhere). For instance, if your tracks are listed in
    the file tracks.lis we may run this command:

      ::

       gmt x2sys_binlist -V -TLINE =tracks.lis > tracks.tbf

**Update index data base:**
    Next, the track bin-index files are fed to :doc:`x2sys_put` which will
    insert the information into the TAG databases:

      ::

       gmt x2sys_put -V -TLINE tracks.tbf

**Search for data:**
    You may now use :doc:`x2sys_get` to find all the tracks within a
    certain sub-region, and optionally limit the search to those tracks
    that have a particular combination of observables. E.g., to find all
    the tracks which has both obs1 and obs3 inside the specified region, run

      ::

       gmt x2sys_get -V -TLINE -R20/40/-40/-20 -Fobs1,obs3 > tracks.tbf

**MGD77[+] or GMT:**
    Format definition files already exist for MGD77 files (both standard ASCII
    and enhanced netCDF-based MGD77+ files) and the old \*.gmt files
    manipulated by the mgg supplements; for these data sets the **-j**
    and **-N** will default to great circle distance calculation in km
    and speed in m/s. There are also format definition files for plain x,y[,z]
    and lon,lat[,z] tracks. To initiate new track databases to be used
    with MGD77 data from NGDC, try

      ::

       gmt x2sys_init MGD77 -V -Dmgd77 -Emgd77 -Rd -Gd -Nsn -I1/1 -Wt900 -Wd5

    where we have chosen a 15 minute (900 sec) or 5 km threshold to
    indicate a data gap and selected knots as the speed; the other steps
    are similar.

**Binary files:**
    Let us pretend that your line files actually are binary files with a
    128-byte header structure (to be skipped) followed by the data
    records and where *lon*, *lat*, *time* are double precision numbers
    while the three observations are 2-byte integers which must be
    multiplied by 0.1. Finally, the first two observations may be -32768
    which means there is no data available. All that is needed is a
    different line.fmt file:

    ======  ====  ===  =========  =====  ======  ========
    # Format define file for the binary line format
    -----------------------------------------------------
    # BINARY                      # File is now binary
    -----------------  ----------------------------------
    # SKIP 128                    # Skip 128 bytes
    -----------------  ----------------------------------
    # GEO                         # Data are geographic
    -----------------  ----------------------------------
    #name   type  NaN  NaN-proxy  scale  offset  oformat
    ======  ====  ===  =========  =====  ======  ========
    lon      d     N   0          1      0       %10.5f
    lat      d     N   0          1      0       %9.5f
    time     d     N   0          1      0       %7.1f
    obs1     h     Y   -32768     0.1    0       %6.1f
    obs2     h     Y   -32768     0.1    0       %6.1f
    obs3     h     N   0          0.1    0       %6.1f
    ======  ====  ===  =========  =====  ======  ========

    The rest of the steps are identical.

**COARDS 1-D netCDF files:**
    Finally, suppose that your line files actually are netCDF files that
    conform to the COARDS convention, with data columns named *lon*,
    *lat*, *time*, *obs1*, *obs2*, and *obs3*. All that is needed is a
    different line.fmt file:

    ======  ====  ===  =========  =====  ======  ========
    # Format define file for the netCDF COARDS line format
    -----------------------------------------------------
    # NETCDF                      # File is now netCDF
    -----------------  ----------------------------------
    # GEO                         # Data are geographic
    -----------------  ----------------------------------
    #name   type  NaN  NaN-proxy  scale  offset  oformat
    ======  ====  ===  =========  =====  ======  ========
    lon      d     N   0          1      0       %10.5f
    lat      d     N   0          1      0       %9.5f
    time     d     N   0          1      0       %7.1f
    obs1     d     N   0          1      0       %6.1f
    obs2     d     N   0          1      0       %6.1f
    obs3     d     N   0          1      0       %6.1f
    ======  ====  ===  =========  =====  ======  ========

    Note we use no scaling or NAN proxies since those issues are usually
    handled internally in the netCDF format description.

Deprecated behavior
-------------------

The Format Definition Files used to have extension .def but since that is also used
by GMT's symbol macro files we have deprecated that extension and now use .fmt.
However, old .def files are still being read.

See Also
--------

:doc:`x2sys_binlist`,
:doc:`x2sys_datalist`,
:doc:`x2sys_get`,
:doc:`x2sys_list`,
:doc:`x2sys_put`,
:doc:`x2sys_report`,
:doc:`x2sys_solve`,
:doc:`x2sys_cross`
