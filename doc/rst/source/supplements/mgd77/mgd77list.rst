.. index:: ! mgd77list

*********
mgd77list
*********

.. only:: not man

    mgd77list - Extract data from MGD77 files

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77list** *NGDC-ids* |-F|\ *columns*\ [,\ *logic*][:\ *bittests*]
[ |-A|\ **c**\ \|\ **d**\ \|\ **f**\ \|\ **m**\ \|\ **t**\ *code*\ [**+f**\ ] ]
[ |-D|\ **A**\ \|\ **a**\ *startdate* ]
[ |-D|\ **B**\ \|\ **b**\ *stopdate* ]
[ |-E| ]
[ |-G|\ **a**\ *startrec* ]
[ |-G|\ **b**\ *stoprec* ]
[ |-I|\ *ignore* ]
[ |-L|\ [*corrtable*] ]
[ |-N|\ **d**\ \|\ **s**\ *unit* ]
[ |-Q|\ **a**\ \|\ **c**\ \|\ **v**\ *min*/*max* ]
[ |SYN_OPT-R| ]
[ |-S|\ **a**\ *startdist*\ [unit] ]
[ |-S|\ **b**\ *stopdist*\ [unit] ]
[ |-T|\ [**m**\ \|\ **e**] ]
[ |SYN_OPT-V| ]
[ |-W|\ *weight* ]
[ |-Z|\ **n**\ \|\ **p** ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-j| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mgd77list** reads MGD77 files and produces an ASCII
[or binary] table. The MGD77 files contain track
information such as leg-id, time and position, geophysical observables
such as gravity, magnetics, and bathymetry, and control codes and
corrections such as Eotvos and diurnal corrections. The MGD77+ extended
netCDF files may also contain additional user columns (for a listing of
available columns, use :doc:`mgd77info` **-C**, and to learn how to add
your own custom columns, see :doc:`mgd77manage`). The user may extract any
combination of these parameters, any of 8 computed quantities (distance,
heading, course-change, velocity, Carter correction, Eotvos correction
and gravity and magnetic global reference fields), calendar sub-units of
time (year, month, day, hour, min, sec), the NGDC id, and finally a
preset weight (see **-W**). A sub-section can be specified by passing
time- or distance-intervals along track or by selecting a geographical
region. Finally, each output record may be required to pass any number
of logical tests involving data values or bit flags. If multiple cruises
are requested then they are separated by segment headers.

Required Arguments
------------------

.. include:: explain_ncid.rst_

.. _-F:

**-F**\ *columns*\ [,\ *logic*][:\ *bittests*]
    The required *columns* string must be a comma-separated list of
    parameter abbreviations given in the desired output order. Any
    parameters given in UPPER case must not be NaN in a record for
    output to occur. Unless specified separately, the output format (if
    ASCII) is controlled by the GMT parameter :ref:`FORMAT_FLOAT_OUT <FORMAT_FLOAT_OUT>`.
    The available column abbreviations for information stored in the
    files (some columns may be NaN) are:

    **recno**
        The record number counter.
    **drt**
        The digital record type, usually 3 or 5 (for Y2K-compliant cruises).
    **id**
        The survey ID string (leg name).
    **ngdcid**
        The 8-character NGDC cruise ID string (usually the file prefix).
    **time**
        Choose between Absolute calendar time (**atime**, the default) in
        the format dictated by the GMT parameters :ref:`FORMAT_DATE_OUT <FORMAT_DATE_OUT>` and
        :ref:`FORMAT_CLOCK_OUT <FORMAT_CLOCK_OUT>`, Relative time (**rtime**) in the format
        dictated by the GMT parameters :ref:`FORMAT_FLOAT_OUT <FORMAT_FLOAT_OUT>` and
        :ref:`TIME_SYSTEM <TIME_SYSTEM>` (or :ref:`TIME_EPOCH <TIME_EPOCH>` and :ref:`TIME_UNIT <TIME_UNIT>`)), or
        Fractional year (**ytime**) in the format dictated by
        :ref:`FORMAT_FLOAT_OUT <FORMAT_FLOAT_OUT>`.
    **lon**
        Longitude in the format dictated by the GMT parameter
        :ref:`FORMAT_GEO_OUT <FORMAT_GEO_OUT>`.
    **lat**
        Longitude in the format dictated by the GMT parameter
        :ref:`FORMAT_GEO_OUT <FORMAT_GEO_OUT>`.
    **twt**
        Two-Way Travel time (in s).
    **depth**
        Corrected bathymetry (in m, positive below sea level).
    **mtf1**
        Magnetic Total Field intensity from sensor 1 (in nTesla).
    **mtf2**
        Magnetic Total Field intensity from sensor 2 (in nTesla).
    **mag**
        Residual magnetic anomaly (in nTesla).
    **gobs**
        Observed gravity (in mGal).
    **faa**
        Free-air gravity anomaly (in mGal).
    **ptc**
        Position Type Code (1 = fix, 3 = interpolated, 9 = unspecified).
    **bcc**
        Bathymetric Correction Code, indicating the procedure used to
        convert travel time to depth. (01-55 = Matthews' zone used to
        correct the depth, 59 = Matthews' corrections used but the zones is
        unspecified in the data record, 60 = S. Kuwahara formula for T-S, 61
        = Wilson formula for T-S, 62 = Del Grosso formula for T-S, 63 =
        Carter's tables, 88 = Other, described in header sections, 99 =
        unspecified).
    **btc**
        Bathymetric Type Code, indicating how the bathymetry value was
        obtained (1 = observed, 3 = interpolated, 9 = unspecified).
    **msens**
        Magnetic sensor for used to evaluate the residual field (1 = 1st or
        leading sensor, 2 = 2nd or trailing sensor, 9 = unspecified).
    **msd**
        Depth (or altitude) of the magnetic sensor (in m, positive below
        sealevel).
    **diur**
        Magnetic diurnal correction (in nTesla).
    **eot**
        Eotvos correction (in mGal).
    **sln**
        Seismic Line Number string.
    **sspn**
        Seismic Shot Point Number string.
    **nqc**
        Navigation Quality Code (5 = suspected, by source institution, 6 =
        suspected, by NGDC, 9 = no problems identified).

    In addition, the following derived navigational quantities can be requested:

    **year**
        The year of each record.
    **month**
        The month of each record.
    **day**
        The day of the month of each record.
    **hour**
        The hour of each record.
    **min**
        The minutes of each record.
    **sec**
        The decimal seconds of each record.
    **dist**
        Along-track distance from start of leg. For method of calculation,
        see **-j** [spherical great circle distances], and for distance
        units, see **-N** [km].
    **az**
        Ship azimuth (heading) measured clockwise from north (in degrees).
    **cc**
        Ship course change (change in heading) measured clockwise from north
        (in degrees).
    **vel**
        Ship speed; see **-N** for units [m/s].

    Finally, the following computed quantities can be requested:

    **weight**
        Weight assigned to this data set (see **-W**).
    **carter**
        Carter depth correction, if **twt** is present in file (in m). Sign:
        Correction is to be added to uncorrected depths to yield a corrected
        depth.
    **igrf**
        International geomagnetic reference field (total field) (in nTesla).
    **ngrav**
        International Gravity reference Field ("normal gravity") (in mGal).
        Field is selected based on the parameter Gravity Theoretical Formula
        Code in the cruise's MGD77 header. If this is not set or is invalid
        we default to the IGF 1980. Alternatively, specify the field
        directly using **-Af** (see that option for more details).
    **ceot**
        Calculated Eotvos correction from navigation, using E =
        7.5038 \* V \* cos(lat) \* sin(az) + 0.004154 \* V^2
        (in mGal). Sign: Correction is to be added to uncorrected faa to
        yield a corrected faa.

    The following short-hand flags are also recognized:

    **mgd77**
        This results in all 27 MGD77 fields being written out in the official MGD77 order.
    **mgd77t**
        The full set of all 26 columns in the MGD77T specification.
    **geo**
        This limits the output to 10 fields (**time**, **lon**, **lat** plus
        the seven geophysical observations **twt**, **depth**, **mtf1**,
        **mtf2**, **mag**, **gobs**, and **faa**).
    **dat**
        As **mgd77t** but in plain table order.
    **all**
        This returns all data columns in the file.
    **allt**
        As mgd77t but with time items written as a date-time string.

    By appending **+** to any of these set we will also append **dist**,
    **azim**, **cc**, **vel**, and **weight** as listed above.
    As an option, logical tests may be added for any of the observations
    by appending ,\ *logic*, which is itself composed of one or more
    comma-separated instructions of the form *par*\ **OP**\ *value*,
    where *par* is one of the parameters listed above, **OP** is a
    logical operator (<, <=, =, !=, >=, >, \|), and *value* is a
    constant used in the comparison. Floating point parameters are
    compared numerically; character parameters are compared lexically
    (after leading and trailing blanks have been removed). The bit
    comparison (\|) means that at least one of the bits in *value* must
    be turned on in *par*. At least one of the tests must be true for
    the record to be output, except for tests using UPPER case
    parameters which all must be true for output to occur. Notes:
    (1) Specifying a test does not imply that the corresponding column will
    be included in the output stream; it must be present in *columns*
    for that to occur. (2) Some of the operators are special UNIX
    characters and you are advised to place quotes around the entire
    argument to **-F**.  (3) The logical tests only apply to observed
    data; derived data (such as distances, velocities, etc.) must be
    limited using program options such as **-D**, **-Q**, **-S**, etc.

    Finally, for MGD77+ files you may optionally append :*bittests*
    which is : (a colon) followed by one or more comma-separated +-*col*
    terms. This compares specific bitflags only for each listed column.
    Here, + means the chosen bit must be 1 (ON) whereas - means it must
    be 0 (OFF). All bit tests given must be passed. By default, MGD77+
    files that have the special **MGD77_flags** column present will use
    those flags, and observations associated with ON-bits (meaning they
    are flagged as bad) will be set to NaN; append : with no trailing
    information to turn this behavior off (i.e., no bit flags will be
    consulted).  Note that these record-based flags are different from
    any systematic corrections along track; the latter are deactivated
    by **-T**.

Optional Arguments
------------------

.. _-A:

**-A**\ **c**\ \|\ **d**\ \|\ **f**\ \|\ **m**\ \|\ **t**\ *code*\ [**+f**\ ]
    By default, corrected depth (**depth**), magnetic residual anomaly
    (**mag**), free-air gravity anomaly (**faa**), and the derived
    quantity Carter depth correction (**carter**) are all output as is
    (if selected in **-F**); this option adjusts that behavior. For each
    of these columns there are 2-4 ways to adjust the data. Append
    **c**\ (arter), **d**\ (epth), **f**\ (aa), or **m**\ (ag) and
    select the *code* for the procedure you want applied. You may select
    more than one procedure for a data column by summing their numerical
    *code*\ s (1, 2, 4, and 8). E.g., **-Ac**\ 3 will first try method
    **-Ac**\ 1 to estimate a Carter correction but if **depth** is NaN
    we will next try **-Ac**\ 2 which only uses **twt**. In all cases,
    if any of the values required by an adjustment procedure is NaN then
    the result will be NaN. This is also true if the original anomaly is
    NaN. Append **+f** to recalculate anomalies even if the anomaly in
    the file is NaN. Additionally, you can use **-At** to create fake
    times for cruises that has no time; these are based on distances and
    cruise duration.
**-Ac**
    Determines how the **carter** correction term is calculated. Below,
    C(\ **twt**) stands for the Carter-corrected depth (it also depends
    on **lon**, **lat**), U(\ **twt**, *v*) is the uncorrected depth (=
    **twt** \* *v* / 2) using as *v* the "Assumed Sound Velocity"
    parameter in the MGD77 header (if it is a valid velocity, otherwise
    we default to 1500 m/s); alternatively, append your preferred
    velocity *v* in m/s, TU(\ **depth**, *v*) is the 2-way travel time
    estimated from the (presumably) uncorrected **depth**, and
    TC(\ **depth**) is the 2-way travel time obtained by inverting the
    (presumably) corrected **depth** using the Carter correction
    formula. Select from

    **-Ac1**\ [,\ *v*] returns difference between U(\ **twt**, *v*) and **depth** [Default].

    **-Ac2**\ [,\ *v*] returns difference between U(\ **twt**, *v*) and Carter (**twt**).

    **-Ac4**\ [,\ *v*] returns difference between (assumed uncorrected) **depth** and Carter (TU(**depth**)).

    **-Ac8**\ [,\ *v*] returns difference between U(TC(\ **depth**), *v*) and **depth**.

**-Ad**
    Determines how the **depth** column output is obtained:

    **-Ad1** returns **depth** as stored in the data set [Default].

    **-Ad2**\ [,\ *v*] returns calculated uncorrected depth U(\ **twt**, *v*).

    **-Ad4** returns calculated corrected depth C(\ **twt**).

**-Af**
    Determines how the **faa** column output is obtained. If **ngrav**
    (i.e., the International Gravity reference Field (IGF), or "normal
    gravity") is required it is selected based on the MGD77 header
    parameter "Theoretical Gravity Formula Code"; if this code is not
    present or is invalid we default to 4. Alternatively, append the
    preferred *field* (1-4) to select 1 (Heiskanen 1924), 2 (IGF 1930),
    3 (IGF 1967) or 4 (IGF 1980). Select from

    **-Af1**\ [,\ *field*] returns **faa** as stored in the data set
    [Default]. Optionally, sets the IGF *field* to use if you also have
    requested **ngrav** as an output column in **-F**.

    **-Af2**\ [,\ *field*] returns the difference between **gobs** and
    **ngrav** (with optional *field* directive).

    **-Af4**\ [,\ *field*] returns the combination of **gobs** + **eot** -
    **ngrav** (with optional *field* directive).

    **-Af8**\ [,\ *field*] returns the combination of **gobs** +
    **pred_eot** - **ngrav** (with optional *field* directive).

**-Am**
    Determines how the **mag** column output is obtained. There may be
    one or two total field measurements in the file (**mtf1** and
    **mtf2**), and the column **msens** may state which one is the
    leading sensor (1 or 2; it may also be undefined). Select from

    **-Am1** returns **mag** as stored in the data set [Default].

    **-Am2** returns the difference between **mgfx** and **igrf**, where
    **x** is the leading sensor (**1** or **2**) indicated by the
    **msens** data field (defaults to **1** if unspecified).

    **-Am4** returns the difference between **mgfx** and **igrf**, where
    **x** is the sensor (**2** or **1**) *not* indicated by the
    **msens** data field (defaults to **2** if unspecified).

    **-Amc**\ *offset*\ [*unit*\ ] applies a correction to compensate
    for the fact that the magnetic field was not acquired at the same
    position as the ship's position [i.e., the navigation]. This is
    accomplished by re-interpolating the total magnetic field to what it
    would have been if it were measured at the ship's position (remember,
    it probably was measured *offset* meters behind). Due to this
    interpolation step, bad navigation, namely too many repeated points,
    may cause trouble.  Measures are taken to minimize this effect but
    they aren't 100% fool proof. The interpolation method is controlled
    by the GMT default GMT_INTERPOLANT. Append **e** for meter, **f**
    for feet, **k** for km, **M** for miles, **n** for nautical miles,
    or **u** for survey feet [Default is **e** (meters)].

.. _-D:

**-Da**\ *startdate*
    Do not list data collected before *startdate*
    (yyyy-mm-dd\ **T**\ [hh:mm:ss]) [Default is start of cruise]. Use
    **-DA** to exclude records whose time is undefined (i.e., NaN).
    [Default reports those records].
**-Db**\ *stopdate*
    Do not list data collected on or after *stopdate*
    (yyyy-mm-dd\ **T**\ [hh:mm:ss]). [Default is end of cruise]. Use **-DB**
    to exclude records whose time is undefined (i.e., NaN). [Default
    reports those records].

.. _-E:

**-E**
    Exact match: Only output records that match all the requested
    geophysical columns [Default outputs records that matches at least
    one of the observed columns].

.. _-G:

**-Ga**\ *startrec*
    Do not list records before *startrec* [Default is 0, the first
    record].
**-Gb**\ *stoprec*
    Do not list data after *stoprec*. [Default is the last record].

.. _-I:

**-I**\ *ignore*
    Ignore certain data file formats from consideration. Append
    **a\|c\|m\|t** to ignore MGD77 ASCII, MGD77+ netCDF, MGD77T ASCII, or plain
    tab-separated ASCII table files, respectively. The option may be
    repeated to ignore more than one format. [Default ignores none].

.. _-L:

**-L**\ [*corrtable*]
    Apply optimal corrections to columns where such corrections are
    available. Append the correction table to use [Default uses the
    correction table mgd77_corrections.txt in the **$MGD77_HOME**
    directory]. For the format of this file, see CORRECTIONS below.

**-n**
    Issue a segment header record with cruise ID for each cruise.

.. _-N:

**-Nd**\ \|\ **s**\ *unit*
    Append **d** for distance or **s** for speed, then give the desired
    *unit* as **e** (meter or m/s), **f** (feet or feet/s), **k** (km or
    km/hr), **m** (miles or miles/hr), **n** (nautical miles or knots),
    or **u** (survey feet or sfeet/s). [Default is **-Ndk** **-Nse** (km
    and m/s)].

.. _-Q:

**-Qa**\ *min*/*max*
    Specify an accepted range (*min*/*max*) of azimuths. Records whose
    track azimuth falls outside this range are ignored [0/360].
**-Qc**\ *min*/*max*
    Specify an accepted range (*min*/*max*) of course changes. Records
    whose track course change falls outside this range are ignored
    [-360/+360]. Use **-QC** to take the absolute value of the course
    change before the test [Default uses signed course changes].
**-Qv**\ *min*/*max*
    Specify an accepted range (*min*/*max*; or just *min* if there is no
    upper limit) of velocities. Records whose track speed falls outside
    this range are ignored [0/infinity]. 

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-Sa**\ *startdist*\ [unit]
    Do not list data that are less than *startdist* meter along track
    from port of departure. Append **e** for meter, **f** for feet,
    **k** for km, **m** for miles, **n** for nautical miles, or **u**
    for survey feet [Default is 0e (meters)].
**-Sb**\ *stopdist*\ [unit]
    Do not list data that are *stopdist* or more meters along track from
    port of departure. Append **e** for meter, **f** for feet, **k** for
    km, **m** for miles, **n** for nautical miles, or **u** for survey
    feet [Default is end of track].

.. _-T:

**-T**\ [**m**\ \|\ **e**]
    Turns OFF the otherwise automatic adjustment of values based on
    correction terms that are stored in the MGD77+ file and used to
    counteract such things as wrong units used by the source institution
    when creating the original MGD77 file from which the MGD77+ file
    derives (the option has no effect on plain MGD77 ASCII files).
    Append **m** or **e** to limit the option to the MGD77 or extended
    columns set only [Default applies to both].  Note that record-based
    E77 flags (MGD77+ format only) are not considered systematic corrections.
    Instead, the application of these bit-flags can be controlled via
    the : (colon) modifier to OPT(F).

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**\ *weight*
    Set the weight for these data. Weight output option must be set in
    **-F**. This is useful if the data are to be processed with the
    weighted averaging techniques offered by :doc:`blockmean </blockmean>`,
    :doc:`blockmedian </blockmedian>`, and :doc:`blockmode </blockmode>` [1].

.. _-Z:

**-Z**\ **n**\ \|\ **p**
    Append the sign you want for **depth**, **carter**, and **msd**
    values below sea level (**-Zn** gives negative bathymetry) [Default
    is **-Zp** for positive down]. 

.. |Add_-bo| replace:: ignored if **-bo** is selected. Likewise,
    string-fields cannot be selected. Note that if time is one of the
    binary output columns it will be stored as Unix-time (seconds since
    1970). To read this information in GMT to obtain absolute calendar
    time will require you to use **-**\ **-TIME_SYSTEM=1**.
.. include:: ../../explain_-bo.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_distcalc.rst_

.. include:: ../../explain_help.rst_

Examples
--------

To get a (distance, heading, gravity, bathymetry) listing from
01010047.mgd77, starting at June 3 1971 20:45 and ending at distance =
5000 km, use the following command:

   ::

    gmt mgd77list 01010047 -Da1971-06-03T20:45 -Sb5000 -Fdist,azim,faa,depth > myfile.txt

To make input for :doc:`blockmean </blockmean>` and :doc:`surface </surface>` using free-air anomalies
from all the cruises listed in the file cruises.lis, but only the data
that are inside the specified area, and make the output binary:

   ::

    gmt mgd77list `cat cruises.lis` -Flon,lat,faa -R-40/-30/25/35 -bo > allgrav.b

To extract the locations of depths exceeding 9000 meter that were not
interpolated (**btc** != 1) from all the cruises listed in the file
cruises.lis:

   ::

    gmt mgd77list `cat cruises.lis` -F"depth,DEPTH>9000,BTC!=1" > really_deep.txt

To extract dist, faa, and grav12_2 from records whose depths are
shallower than 3 km and where none of the requested fields are NaN, from
all the MGD77+ netCDF files whose cruise ids are listed in the file
cruises.lis, we try

   ::

    gmt mgd77list `cat cruises.lis` -E -Ia -F"dist,faa,grav12_2,depth<3000" > \
        shallow_grav.txt

To extract dist, faa, and grav12_2 from all the MGD77+ netCDF files
whose cruise ids are listed in the file cruises.lis, but only retrieve
records whose bitflag for faa indicates BAD values, we try

   ::

    gmt mgd77list `cat cruises.lis` -E -Ia -F"dist,faa,grav12_2:+faa" > bad_grav.txt

To output lon, lat, mag, and faa from all the cruises listed in the file
cruises.lis, but recalculate the two residuals based on the latest
reference fields, try:

   ::

    gmt mgd77list `cat cruises.lis` -Flon,lat,mag,faa -Af2,4 -Am2 > data.txt

Recalculated Anomalies
----------------------

When recalculated anomalies are requested (either explicitly via the
**-A** option or implicitly via E77 metadata in the MGD77+ file) we only
do so for the records whose original anomaly was not a NaN. This
restriction is implemented since many anomaly columns contains
corrections, usually in the form of hand-edited changes, that cannot be
duplicated from the corresponding observation.

Igrf
----

The IGRF calculations are based on a Fortran program written by Susan
Macmillan, British Geological Survey, translated to C via f2c by Joaquim
Luis, U Algarve, and adapted to GMT-style by Paul Wessel.

Igf
---

The equations used are reproduced here using coefficients extracted
directly from the source code (let us know if you find errors):

(1) g = 978052.0 \* [1 + 0.005285 \* sin^2(lat) - 7e-6 \* sin^2(2\*lat)
+ 27e-6 \* cos^2(lat) \* cos^2(lon-18)]

(2) g = 978049.0 \* [1 + 0.0052884 \* sin^2(lat) - 0.0000059 \*
sin^2(2\*lat)]

(3) g = 978031.846 \* [1 + 0.0053024 \* sin^2(lat) - 0.0000058 \*
sin^2(2\*lat)]

(4) g = 978032.67714 \* [(1 + 0.00193185138639 \* sin^2(lat)) / sqrt (1
- 0.00669437999013 \* sin^2(lat))]

Corrections
-----------

The correction table is an ASCII file with coefficients and parameters
needed to carry out corrections. Comment records beginning with # are
allowed. All correction records are of the form

*cruiseID observation correction*

where *cruiseID* is a NGDC prefix, *observation* is one of the
abbreviations for geophysical observations listed under **-F** above,
and *correction* consists of one or more *term*\ s that will be summed
up and then **subtracted** from the observation before output. Each
*term* must have this exact syntax:

*factor*\ [\*[*function*]([*scale*](\ *abbrev*\ [-*origin*]))[^\ *power*]]

where terms in brackets are optional (the brackets themselves are not
used but regular parentheses must be used as indicated). No spaces are
allowed except between *term*\ s. The *factor* is the amplitude of the
basis function, while the optional *function* can be one of sin, cos, or
exp. The optional *scale* and *origin* can be used to translate the
argument (before giving it to the optional function). The argument
*abbrev* is one of the abbreviations for observations listed above. If
*origin* is given as **T** it means that we should replace it with the
value of *abbrev* for the very first record in the file (this is usually
only done for *time*). If the first record entry is NaN we revert
*origin* to zero. Optionally, raise the entire expression to the given
*power*, before multiplying by the amplitude. The following is an
example of fictitious corrections to the cruise 99999999, implying the
**depth** should have the Carter correction removed, **faa** should have
a linear trend removed, the magnetic anomaly (**mag**) should be
corrected by a strange dependency on ship heading and latitude, and
**gob**\ s needs to have 10 mGal added (hence given as -10):

99999999 depth 1.0\*((carter))

99999999 faa 14.1 1e-5\*((time-T))

99999999 mag 0.5\*cos(0.5\*(azim-19))^2 1.0\*exp(-1e-3(lat))^1.5

99999999 gobs -10

See Also
--------

:doc:`blockmean </blockmean>`,
:doc:`blockmedian </blockmedian>`,
:doc:`blockmode </blockmode>`,
:doc:`mgd77convert`,
:doc:`mgd77info`,
:doc:`mgd77manage`,
:doc:`mgd77track`,
:doc:`surface </surface>`,

References
----------

The Marine Geophysical Data Exchange Format - MGD77, see
`<http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt>`_

IGRF, see `<http://www.ngdc.noaa.gov/IAGA/vmod/igrf/>`_
