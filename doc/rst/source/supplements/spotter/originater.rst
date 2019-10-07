.. index:: ! originater

**********
originater
**********

.. only:: not man

    originater - Associate seamounts with nearest hotspot point sources

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt originater** [ *tables* ] |-E|\ *rotfile*\ [**+i**]
|-F|\ *hs_file*\ [**+d**] 
[ |-D|\ *d_km* ]
[ |-L|\ [*flag*] ]
[ |-N|\ *upper_age* ]
[ |-Q|\ *r/t* ]
[ |-S|\ [*n_hs*] ]
[ |-T| ]
[ |SYN_OPT-V| ]
[ |-W|\ *maxdist* ]
[ |-Z| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**originater** reads (longitude, latitude, height, radius, crustal_age)
records from *tables* [or standard input] and uses the given Absolute
Plate Motion (APM) stage or total reconstruction rotation file and the
list of hotspot locations to determine the most likely origin (hotspot)
for each seamount. It does so by calculating flowlines back in time and
determining the closest approach to all hotspots. The output consists of
the input records with four additional fields added for each of the
*n_hs* closest hotspots. The four fields are the hotspot id (e.g.,
HWI), the stage id of the flowline segment that came closest, the
pseudo-age of the seamount, and the closest distance to the hotspot (in
km). See option **-:** on how to read (latitude, longitude,height,
radius, crustal_age) files.

Required Arguments
------------------

.. _-E:

**-E**\ *rotfile*\ [**+i**]
    Give file with rotation parameters. This file must contain one
    record for each rotation; each record must be of the following
    format:

    *lon lat tstart [tstop] angle* [ *khat a b c d e f g df* ]

    where *tstart* and *tstop* are in Myr and *lon lat angle* are in
    degrees. *tstart* and *tstop* are the ages of the old and young ends
    of a stage. If *tstop* is not present in the record then a total
    reconstruction rotation is expected and *tstop* is implicitly set to
    0 and should not be specified for any of the records in the file. If
    a covariance matrix **C** for the rotation is available it must be
    specified in a format using the nine optional terms listed in
    brackets. Here, **C** = (*g*/*khat*)\*[ *a b d; b c e; d e f* ]
    which shows **C** made up of three row vectors. If the degrees of
    freedom (*df*) in fitting the rotation is 0 or not given it is set
    to 10000. Blank lines and records whose first column contains # will
    be ignored. You may append **+i** to the filename to indicate
    you wish to invert the rotations.
    Alternatively, give the filename composed of two plate IDs
    separated by a hyphen (e.g., PAC-MBL) and we will instead extract
    that rotation from the GPlates rotation database. We return an error
    if the rotation cannot be found. Append **+i** if you want to invert
    the rotations prior to use.

.. _-F:

**-F**\ *hs_file*\ [**+d**]
    Give file with hotspot locations. This file must contain one record
    for each hotspot to be considered; each record must be of the
    following format:

    lon lat hs_abbrev hs_id r t_off t_on create fit plot name

    E.g., for Hawaii this may look like

    205 20 HWI 1 25 0 90 Y Y Y Hawaii

    Most applications only need the first 4 columns which thus
    represents the minimal hotspot information record type. The
    abbreviation may be maximum 3 characters long. The id must be an
    integer from 1-32. The positional uncertainty of the hotspot is
    given by r (in km). The t_off and t_on variables are used to
    indicate the active time-span of the hotspot. The create, fit, and
    plot indicators are either Y or N and are used by some programs to
    indicate if the hotspot is included in the ID-grids used to
    determine rotations, if the hotspot chain will be used to determine
    rotations, and if the hotspot should be included in various plots.
    The name is a 32-character maximum text string with the full hotspot
    name. Blank lines and records whose first column contains # will be
    ignored. Append **+d** if we should look for hotspot drift tables
    whose name must be *hs_abbrev*\ \_drift.txt. Such files may be
    located in the current directory, the same directory as *hs_file*,
    or in the directories pointed to by GMT_DATADIR. If found then we
    interpolate to get hotspot location as a function of time [fixed].

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. _-D:

**-D**\ *d_km*
    Sets the flowline sampling interval in km. [Default is 5].

.. _-L:

**-L**\ [*flag*]
    Output closest approach for nearest hotspot only (ignores **-S**).
    Choose **-Lt** for (*time*, *dist*, *z*) [Default], **-Lw** for
    (*omega*, *dist*, *z*), and **-Ll** for (lon, lat, time, dist, z).
    Normally, *dist* is in km; use upper case modifiers **TWL** to get
    *dist* in spherical degrees.

.. _-N:

**-N**\ *upper_age*
    Set the maximum age to extend the oldest stage back in time [no extension].

.. _-Q:

**-Q**\ *r/t*
    Input files only has (*x*,\ *y*,\ *z*); specify constant values for *r*,\ *t* that
    will be implied for each record.

.. _-S:

**-S**\ [*n_hs*\ ]
    Set the number of closest hotspots to report [Default is 1].

.. _-T:

**-T**
    Truncate seamount ages exceeding the upper age set with **-N** [no truncation].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**\ *maxdist*
    Only report those seamounts whose flowlines came within *maxdist* to
    any hotspot [Default reports all seamounts].

.. _-Z:

**-Z**
    Use the hotspot ID number rather than the name tag in output records.

.. |Add_-bi| replace:: [Default is 5 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_../../explain_-V.rst_

.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: explain_geodetic.rst_

Examples
--------

To find the likely (hotspot) origins of the seamounts represented by the
(x,y,z,r,tc) points in the file seamounts.txt, using the DC85.txt Euler
poles and the pac_hs.txt list of possible hotspots, and report the 2 most
likely hotspot candidates for each seamount, run

   ::

    gmt originater seamounts.txt -S2 -EDC85.txt -Fpac_hs.txt > origins.txt

To determine the predicted age of a seamount, distances to the closest
hotspot, and echo the observed age given its location, observed age, and
a rotation model, try

   ::

    echo "1.55 -8.43 52.3" | gmt originater -FONeill_2005_hotspots.txt \
    -EOMS2005_APM_fixed.txt -Q1/120 -Lt

where 52.3 Ma is observed age. The output is 70 -95.486 52.3. To repeat
the same exercise with a moving hotspot model, try

   ::

    echo "1.55 -8.43 52.3" | gmt originater -FONeill_2005_hotspots.txt+d \
    -EOMS2005_APM_smooth.txt -Q1/120 -Lt

Now the output is 80 -213.135 52.3. Negative distances means the closest
approach was east of the hotspot.

Notes
-----

GMT distributes the EarthByte rotation model Global_EarthByte_230-0Ma_GK07_AREPS.rot.
To use an alternate rotation file, create an environmental parameters named
**GPLATES_ROTATIONS** that points to an alternate rotation file.

See Also
--------

:doc:`gmt </gmt>`,
:doc:`grdrotater`,
:doc:`grdspotter`,
:doc:`project </project>`,
:doc:`mapproject </mapproject>`,
:doc:`backtracker`,
:doc:`gmtpmodeler`,
:doc:`grdpmodeler`,
:doc:`grdrotater`,
:doc:`hotspotter`

References
----------

Wessel, P., 1999, "Hotspotting" tools released, EOS Trans. AGU, 80 (29), p. 319.

