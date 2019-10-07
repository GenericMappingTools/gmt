.. index:: ! mgd77sniffer

**************
mgd77sniffer
**************

.. only:: not man

    mgd77sniffer - Along-track quality control of MGD77 cruises

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77sniffer** *NGDC-ids* [ |-A|\ *fieldabbrev*,\ *scale*,\ *offset* ]
[ |-C|\ *maxspd* ]
[ |-D|\ **d**\ \|\ **e**\ \|\ **E**\ \|\ **f**\ \|\ **l**\ \|\ **m**\ \|\ **s**\ \|\ **v**\ [*r*] ]
[ |-E| ]
[ |-G|\ *fieldabbrev*,\ *imggrid*,\ *scale*,\ *mode* or |-G|\ *fieldabbrev*,\ *grid* ]
[ |-H| ]
[ |-I|\ *fieldabbrev*,\ *rec1*,\ *recN* ]
[ |-L|\ *custom-limits-file* ]
[ |-M| ]
[ |-N| ]
[ |SYN_OPT-R| ]
[ |-S|\ **d**\ \|\ **s**\ \|\ **t** ]
[ |-T|\ *gap* ]
[ |SYN_OPT-V| ]
[ |-W|\ **c**\ \|\ **g**\ \|\ **o**\ \|\ **s**\ \|\ **t**\ \|\ **v**\ \|\ **x** ]
[ |-Z|\ *level* ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-do| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mgd77sniffer** scans old (pre-Y2K) and new format ASCII MGD77 files
for errors using point-by-point sanity checking, along-track detection
of excessive slopes, and optional comparison of cruise data with global
gravity and predicted bathymetry grids. Detected data problems are
output by default as verbose descriptions of each detected error, often
resulting in multiple messages per scanned record. Data problems are
optionally output (**-De** option) using a computer-parseable format
(see E77 ERROR FORMAT description below). Default error thresholds are
derived from histograms of all MGD77 geophysical data collected between
1952 and January, 2006. Thresholds are adjustable with the **-L**
option. Grids for comparison with cruise data may be downloaded via the web. 

Required Arguments
------------------

.. include:: explain_ncid.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ *fieldabbrev*,\ *scale*,\ *offset*
    Apply scale factor and DC adjustment to specified data field. Allows
    adjustment of cruise data prior to along-track analysis. CAUTION:
    data must be thoroughly examined before applying these global data
    adjustments. May not be used for multiple cruises.

.. _-C:

**-C**\ *maxspd*
    Set maximum ship speed in m/s, or knots with **-N** option. Ship
    speeds exceeding 10 m/s (~20 knots) are flagged as excessive by default.

.. _-D:

**-D**\ **d**\ \|\ **e**\ \|\ **E**\ \|\ **f**\ \|\ **l**\ \|\ **m**\ \|\ **s**\ \|\ **v**\ [*r*]
    Suppress default warning output and only dump cruise data row-by-row
    such as values, gradients, grid-cruise differences, E77 error
    summaries for each record, re-created MGD77 records or sniffer
    limits. Append r to include all records (default omits records where
    navigation errors were detected).

    **-Dd** output differences between cruise and grid data. Requires
    **-G** option. Output columns include:

    *lat lon dist cruiseZ gridZ diff [cruiseZ2 gridZ2 diff2 ...]*

    Note: grid values are subtracted from cruise data so a positive
    difference implies cruise > grid. For multiple grid comparison,
    *cruiseZ gridZ diff* are repeated for each grid comparison in
    command line order.

    **-De** output E77 error classification format. Error output is
    divided into (1) a header containing information
    globally applicable to the cruise and (2) individual
    error records summarizing all errors encountered in each cruise
    record. mgd77sniffer writes E77 directly to <ngdc\_id.e77> file
    handle. See **E77 ERROR FORMAT** below for additional details.

    **-DE** Same as **-De** but no regression tests will be carried out.

    **-Df** output delta Z (change in geophysical field) column and
    delta S (change in distance) for each geophysical field. Distance
    between observations often differ for different fields depending on
    instrument sampling rate, so ds is included for each geophysical
    observation. Output columns include:

    *d[twt] ds d[depth] ds d[mtf1] ds d[mtf2] ds d[mag] ds d[diur] ds
    d[msd] ds d[gobs] ds d[eot] ds d[faa] ds*

    **-Dl** display mgd77sniffer limits. Customize this output to create
    a custom limits file for the **-L** option. No additional arguments
    are required. Output columns include:

    *fieldabbrev min max maxSlope maxArea*

    **-Dm** output MGD77 format records in Y2K-compliant MGD77 format

    **-Dn** output distance to coast for each record. Requires the **-Gnav**
    option. Output columns include:

    *lat lon dist distToCoast*

    **-Ds** output calculated gradients for speed and geophysical
    fields. Gradients correspond to the gradient type selected in the
    **-S** option (spatial derivatives by default). Output columns include:

    *speed d[twt] d[depth] d[mtf1] d[mtf2] d[mag] d[diur] d[msd] d[gobs] d[eot] d[faa]*

    See **MGD77 FIELD INFO** below for field and abbreviations descriptions.

    **-Dv** display values for the twelve position and geophysical
    fields for each MGD77 data record (in this order):

    *lat lon twt depth mtf1 mtf2 mag diur msens gobs eot faa*

    See below for **MGD77 FIELD INFO**.

.. _-E:

**-E**
    Reverse navigation quality flags (good to bad and vice versa). May
    be necessary when a majority of navigation fixes are erroneously
    flagged bad, which can happen when a cruise's first navigation fix
    is extremely erroneous. Caution! This will affect sniffer output and
    should only be attempted after careful manual navigation review.

.. _-G:

**-G**\ *information*
    Compare cruise data to GMT or IMG grids. Use one of the formats below.
    **-G**\ *fieldabbrev*,\ *imggrid*,\ *scale*,\ *mode*
    Compare cruise data to the specified grid in Sandwell/Smith Mercator
    format. Requires a valid MGD77 field abbreviation (see **MGD77 FIELD
    INFO** below) followed by a comma, the path (if not in current
    directory) and grid filename, a scale to multiply the data (1 or
    0.1), and mode which stand for the following: (0) Img files with no
    constraint code, returns data at all points, (1) Img file with
    constraints coded, return data at all points, (2) Img file with
    constraints coded, return data only at constrained points and NaN
    elsewhere, and (3) Img file with constraints coded,
    return 1 at constraints and 0 elsewhere.
    **-G**\ *fieldabbrev*,\ *grid*
    Compare cruise data to the specified grid. Requires a valid MGD77
    field abbreviation (see **MGD77 FIELD INFO** below) followed by a
    comma, then the path (if not in current directory) and grid
    filename. Multiple grid comparison is supported by using separate
    **-G** calls for each grid. See **GRID FILE INFO** below.

    Grid comparison activates several additional error checks. (1)
    Re-weighted Least Squares Regression of ship versus grid data
    determines slope and DC shift, which when differing from expected 1
    and 0, respectively, may indicate incorrectly scaled ship data,
    including incorrect units or instrument drift as well as erroneous
    gravity tie-in. (2) Accumulated ship grid offsets are computed
    along-track and excessive offsets are flagged according to *maxArea*
    threshold (use **-L** option to adjust *maxArea*). Warning:
    predicted bathymetry grids are constrained by cruise data so grids
    and cruise data are not always independent. Comparison of cruise
    bathymetry with predicted bathymetry grids also activates a
    "navigation crossing over land" check.

.. _-H:

**-H**
    (with **-G**\ \|\ **g** only) disable (or force) decimation during
    RLS analysis of ship and gridded data. By default mgd77sniffer
    analyses both the full and decimated data sets then reports RLS
    statistics for the higher correlation regression.

    **-Hb** analyze both (default), report better of two.

    **-Hd** to disable data decimation (equivalent to -H with no argument).

    **-Hf** to force data decimation.

.. _-I:

**-I**\ *fieldabbrev*,\ *rec1*,\ *recN*
    Append a field abbreviation and the first and last record in a range
    of records that should be flagged as bad (and set to NaN prior to
    the analysis). Repeat as many times as needed. May not be used for
    multiple cruises.

.. _-L:

**-L**\ *custom-limits-file*
    Override mgd77sniffer default error detection limits. Supply path
    and filename to the custom limits file. Rows not beginning with a
    valid MGD77 field abbreviation are ignored. Field abbreviations are
    listed below in exact form under MGD77 FIELD INFO. Multiple field
    limits may be modified using one default file, one field per line.
    Field min, max, max slope and max area may be changed for each
    field. Max slope pertains to the gradient type selected using the
    **-S** option. Max area is used by the **-G** option as the
    threshold for flagging excessive offsets from the specified grid.
    Dump defaults **-Dl** to view syntax or to quickly create an
    editable custom limits file.

    Example custom default file contents (see below for units):

    +--------------+--------+---------+------------+-----------+
    | # abbrev     | min    | max     | maxSlope   | maxArea   |
    +--------------+--------+---------+------------+-----------+
    | twt          | 0      | 15      | 1          | 0         |
    +--------------+--------+---------+------------+-----------+
    | depth        | 0      | 11000   | 500        | 5000      |
    +--------------+--------+---------+------------+-----------+
    | mag          | -800   | 800     | --         | --        |
    +--------------+--------+---------+------------+-----------+
    | faa          | -300   | 300     | 100        | 2500      |
    +--------------+--------+---------+------------+-----------+

    Use a dash '-' to retain a default limit. Hint: to test your custom
    limits, try: mgd77sniffer **-Dl** **-L**\ <yourlimitsfile>


.. _-M:

**-M**
    Adjust navigation on land threshold (meters inland) [100].

.. _-N:

**-N**
    Use nautical units.

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-S**\ **d**\ \|\ **s**\ \|\ **t**
    Specify gradient type for along-track excessive slope checking.
    **-Sd** Calculate change in z values along track (dz). Output is
    given in geophysical units, e.g., mGal.
    **-Ss** Calculate spatial gradients (dz/ds). Output is given in
    geophysical units per km along the survey track, e.g., mGal/km.
    **-St** Calculate time gradients (dz/dt) [default]. Output is given
    in geophysical units per second along the survey track, e.g., mGal/sec.

.. _-T:

**-T**\ *gap*
    Adjusts mgd77sniffer gap handling. By default, data gaps greater
    than 5 km are skipped. Set to zero to de-activate gap skipping.

.. _-W:

**-W**\ **c**\ \|\ **g**\ \|\ **o**\ \|\ **s**\ \|\ **t**\ \|\ **v**\ \|\ **x**
    Print out only certain warning types for verbose error messages.
    Comma delimit any combination of **c\|g\|o\|s\|t\|v\|x**: where
    (**c**) type code warnings, (**g**)radient out of range,
    (**o**)ffsets from grid (requires **-G**\ \|\ **g**), (**s**)peed
    out of range, (**t**)ime warnings, (**v**)alue out of range, (**x**)
    warning summaries. By default ALL warning messages are printed.Not
    compatible with any **-D** options. 

.. _-V:

.. _-Z:

**-Z**
    Flag regression statistics that are outside the specified confidence
    level. (i.e., **-Z**\ 5 flags coefficients m, b, rms, and r that fall outside 95%.) 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. |Add_-bo| replace:: Output binary data for **-D**\ d\|f\|s\|v option.
.. include:: ../../explain_-bo.rst_

.. |Add_-do| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-do.rst_

.. include:: ../../explain_-n.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_grdresample2.rst_

Mgd77 Field Info
----------------

+-------------+------------------+-------------+
| *Field*     | *Abbreviation*   | *Units*     |
+-------------+------------------+-------------+
| Two-way     | Travel           | Time        |
+-------------+------------------+-------------+
| Corrected   | Depth            | depth       |
+-------------+------------------+-------------+
| Mag         | Total            | Field1      |
+-------------+------------------+-------------+
| Mag         | Total            | Field2      |
+-------------+------------------+-------------+
| Residual    | Magnetic         | mag         |
+-------------+------------------+-------------+
| Diurnal     | Correction       | diur        |
+-------------+------------------+-------------+
| Mag         | Sensor           | Depth/Alt   |
+-------------+------------------+-------------+
| Observed    | Gravity          | gobs        |
+-------------+------------------+-------------+
| Eotvos      | Correction       | eot         |
+-------------+------------------+-------------+
| Free        | Air              | Anomaly     |
+-------------+------------------+-------------+

Grid File Info
--------------

For |-G| the grids must either be in the img format used by Sandwell & Smith,
which is a spherical Mercator 2-byte grid with no header, or any regular grid type
supported by GMT.

E77 Error Format
----------------

**Header**
    Information pertaining to an entire cruise, such as NGDC and survey
    institution identification codes, cruise examination time, two-way
    travel time corrector information, data precision warnings, as well
    as systematic scales, DC shifts and correlation coefficients from
    global grid comparisons are reported as E77 header information.

    **Sample**

    # Cruise 08010039 ID 74010908 MGD77 FILE VERSION: 19801230 N_RECS: 3066

    # Examined: Wed Oct 3 16:30:13 2007 by mtchandl

    # Arguments: -De -Gdepth,/data/GRIDS/etopo5_hdr.i2

    N Errata table verification status

    # mgd77manage applies corrections if the errata table is verified
    (toggle 'N' above to 'Y' after review)

    # For instructions on E77 format and usage, see
    `http://gmt.soest.hawaii.edu/mgd77/errata.php <http://gmt.soest.hawaii.edu/mgd77/errata.php>`_

    # Verified by:

    # Comments:

    # Errata: Header

    Y-E-08010039-H13-02: Invalid Magnetics Sampling Rate: (99) [ ]

    Y-W-08010039-H13-10: Survey year (1975) outside magnetic reference
    field IGRF 1965 time range (1965-1970)

    Y-I-08010039-depth-00: RLS m: 1.00053 b: 0 rms: 127.851 r: 0.973422
    significant: 1 decimation: 0

    Y-W-08010039-twt-09: More recent bathymetry correction table available

    Y-W-08010039-mtf1-10: Integer precision

    Y-W-08010039-mag-10: Integer precision

**Error Record**
    Individual error records have strict format. Included is a time or
    distance column followed by record number, a formatted error code
    string, and finally a verbose description of errors detected in the
    record. Three error classes are encoded into the error code string
    with different alphabetic characters representing unique error
    types. See below for error code format description.

    **Format**
    <time/distance> <record number> <error code string> <description>

    **Sample**

    # Errata: Data

    Y  08010039  1975-05-10T22:16:05.88  74  C-0-0 NAV: excessive speed

**Error Code Description**
    Each of the three error classes is separated by a dash **-** and
    described by a combination of alphabetic characters or 0 signifying
    no detected problems.

    Error classes: NAV-VAL-GRAD

    **Error Class Descriptions**

    NAV (navigation):

    0 - fine

    A - time out of range

    B - time decreasing

    C - excessive speed

    D - above sea level

    E - lat undefined

    F - lon undefined

    VAL (value):

    0 - fine

    K - twt invalid

    L - depth invalid

    O - mtf1 invalid

    etc.

    GRAD (gradient):

    0 - fine

    K - d[twt] excessive

    L - d[depth] excessive

    O - d[mtf1] excessive
     etc.

    The NAV error class has unique cases while VAL and GRAD classes are
    described by alphabetic characters for each of the 24 numeric fields
    in MGD77 format order.

    MGD77 bit-pattern w/ E77 alpha characters

    \|-------------------------------------------------\|----------\|

    \| X W V U T S R Q P O N M L K J I H G F E D C B A \| E77 Code \|

    \| - - - - - - - - - - - - - - - - - - - - - - - - \| - - - - -\|

    \| n f e g m d m m m m b b d t p l l m h d m y t d \| F I \|

    \| q a o o s i s a t t t c e w t o a i o a o e z r \| i D \|

    \| c a t b d u e g f f c c p t c n t n u y n a t \| e \|

    \| s r n 2 1 t r t r \| l \|

    \| s h h \| d \|

    \| - - - - - - - - - - - - - - - - - - - - - - - - \| - - - - -\|

    \| 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \| Bit place\|

    \| - G C G C C - G G G - - G G - - - T T T T T - - \| Bit type \|

    \|-------------------------------------------------\|----------\|

    Bit types: (G)eophysical, (C)orrection, (T)ime

Examples
--------

To scan for excessive values or gradients, try

   ::

    gmt mgd77sniffer 08010001

To dump cruise gradients, try

   ::

    gmt mgd77sniffer 08010001 -Ds

To compare cruise depth with ETOPO5 bathymetry and gravity with
Sandwell/Smith 2 min gravity version 11, try

   ::

    mgd77sniffer 08010001 -Gdepth,/data/GRIDS/etopo5_hdr.i2 \
                 -Gfaa,/data/GRIDS/grav.11.2.img,0.1,1

See Also
--------

:doc:`mgd77list`,
:doc:`mgd77track`
:doc:`x2sys_init <../x2sys/x2sys_init>`

References
----------

The Marine Geophysical Data Exchange Format - MGD77, see
`<http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt.>`_
