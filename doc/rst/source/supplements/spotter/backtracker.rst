.. index:: ! backtracker

***********
backtracker
***********

.. only:: not man

    backtracker - Generate forward and backward flowlines and hotspot tracks

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt backtracker** [ *table* ] |-E|\ *rot_file*\|\ *lon*/*lat*/*angle*
[ |-A|\ [*young*/*old*] ]
[ |-D|\ **f**\ \|\ **b** ]
[ |-F|\ *drift.txt* ]
[ |-L|\ **f**\ \|\ **b**\ [*step*] ]
[ |-M|\ [*fact*] ]
[ |-N|\ *upper_age* ]
[ |-Q|\ *fixed_age* ]
[ |-S|\ *filestem* ]
[ |-T|\ *zero_age* ]
[ |SYN_OPT-V| ]
[ |-W|\ [**a**\ \|\ **t**] ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**backtracker** reads (longitude, latitude, age) positions from
*infiles* [or standard input] and computes rotated (x,y,t) coordinates
using the specified rotation parameters. It can either calculate final
positions [Default] or create a sampled track (flowline or hotspot
track) between the initial and final positions. The former mode allows
additional data fields after the first 3 columns which must have
(longitude,latitude,age). See option **-:** on how to read
(latitude,longitude,age) files.

Required Arguments
------------------

.. _-E:

**-E**\ *rotfile*
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
    Alternative 1: Give the filename composed of two plate IDs
    separated by a hyphen (e.g., PAC-MBL) and we will instead extract
    that rotation from the GPlates rotation database. We return an error
    if the rotation cannot be found.
    Alternative 2: Specify *lon*/*lat*/*angle*, i.e., the longitude,
    latitude, and opening angle (all in degrees and separated by /) for
    a single total reconstruction rotation.

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. _-A:

**-A**\ [*young*/*old*]
    Used in conjunction with **-Lb**\ \|\ **f** to limit the track
    output to those sections whose predicted ages lie between the
    specified *young* and *old* limits. If **-LB**\ \|\ **F** is used
    instead then the limits apply to the stage ids (id 1 is the youngest
    stage). If no limits are specified then individual limits for each
    record are expected in columns 4 and 5 of the input file.

.. _-D:

**-Df**\ \|\ **b**
    Set the direction to go: **-Df** will go backward in time (from
    younger to older positions), while **-Db** will go forward in time
    (from older to younger positions) [Default]. Note: For **-Db** you
    are specifying the age at the given location, whereas for **-Df**
    you are not; instead you specify the age at the reconstructed point.

.. _-F:

**-F**\ *drift.txt*
    Supply a file with (lon, lat, age) records that describe the history
    of hotspot motion for the current hotspot. The reconstructions will
    use the 3rd data input column (i.e., the age) to obtain the
    location of the hotspot at that time, via an interpolation of the
    hotspot motion history. Input data locations are then adjusted by the
    change in hotspot location when reconstructing the point or path [No drift].
    Note: (1) When **-F** is used the **-L** *step* values will be in time (Myr).
    (2) Drift is only considered when backtracking a point (**-Db**) or predicting
    seamount trails (**-Df** **-Lb**). (3) Cannot be used with **-M**.

.. _-L:

**-Lf**\ \|\ **b**\ [*step*]
    Specify a sampled path between initial and final position: **-Lf**
    will draw particle flowlines, while **-Lb** will draw backtrack
    (hotspot track) paths. Append sampling interval in km. If *step* < 0 or not provided
    then only the rotation times will be returned. When **-LF** or
    **-LB** is used, the third output column will contain the stage id
    (1 is youngest) [Default is along-track predicted ages]. You can
    control the direction of the paths by using **-D**.

.. _-M:

**-M**\ [*fact*]
    Scale opening angles by *fact* on output [0.5]. 
    Typically used to get half-rates needed for flowlines.

.. _-N:

**-N**\ *upper_age*
    Set the maximum age to extend the oldest stage rotation back in time
    [Default is no extension].

.. _-Q:

**-Q**\ *fixed_age*
    Assign a fixed age to all positions. Only lon, lat input is expected
    [Default expects longitude, latitude, age]. Useful when the input
    are points defining isochrons.

.. _-S:

**-S**\ *filestem*
    When **-L** is set, the tracks are normally written to *stdout* as a
    multisegment file. Specify a *filestem* to have each track written
    to *filestem.#*, where *#* is the track number. The track number is
    also copied to the 4th output column.

.. _-T:

**-T**\ *zero_age*
    Set the current time [Default is 0 Ma].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**\ [**a**\ \|\ **t**]
    Rotates the given input (lon,lat,t) and calculates the confidence
    ellipse for the projected point. The input point *must* have a time
    coordinate that exactly matches a particular total reconstruction
    rotation time, otherwise the point will be skipped. Append **t** or
    **a** to output time or angle, respectively, after the projected
    lon, lat. After these 2-3 items, we write azimuth, major, minor (in
    km) for the 95% confidence ellipse. See **-D** for the direction of
    rotation.

.. |Add_-bi| replace:: [Default is 3 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: ../../explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_-ocols.rst_
.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: explain_geodetic.rst_

Examples
--------

To backtrack the (x,y,t) points in the file seamounts.txt to their origin
(presumably the hotspot), using the DC85.txt Euler poles, run

   ::

    gmt backtracker seamounts.txt -Db -EDC85.txt > newpos.txt

To project flowlines forward from the (x,y,t) points stored in several
3-column, binary, double precision files, run

   ::

    gmt backtracker points.* -Df -EDC85.txt -Lf25 -bo -bi3 > lines.b

This file can then be plotted with :doc:`plot </plot>`.
To compute the predicted Hawaiian hotspot track from 0 to 80 Ma every 1
Ma, given a history of hotspot motion file (HIdrift.txt) and a set of
total reconstruction rotations for the plate (PAC_APM.txt), try

   ::

    echo 204 19 80 | gmt backtracker -Df -EPAC_APM.txt -Lb1 > path.txt

To predict Hawaiian-Emperor seamount trail using the Pacific absolute plate
and plume motion from Doubrovine et al. (2012), use

   ::

    echo -155.2872 19.3972 80 | gmt backtracker -Df -Lb1 -ED2012.txt -FD2012_HI_drift.txt > traildrift.txt

To predict the Hawaiian-Emperor seamount trail that would have resulted if no plume drift had been in effect,
using the Pacific absolute plate motion model from Doubrovine et al. (2012), use

   ::

    echo -155.2872 19.3972 80 | gmt backtracker -Df -Lb1 -ED2012.txt > trail.txt

Notes
-----

GMT distributes the EarthByte rotation model Global_EarthByte_230-0Ma_GK07_AREPS.rot.
To use an alternate rotation file, create an environmental parameters named
**GPLATES_ROTATIONS** that points to an alternate rotation file.

See Also
--------

:doc:`gmt </gmt>` ,
:doc:`gmtpmodeler`,
:doc:`grdpmodeler`,
:doc:`grdrotater`,
:doc:`grdspotter`,
:doc:`hotspotter`,
:doc:`mapproject </mapproject>`,
:doc:`originater`,
:doc:`project </project>`,
:doc:`plot </plot>`

References
----------

Wessel, P., 1999, "Hotspotting" tools released, EOS Trans. AGU, 80 (29),
p. 319.

Doubrovine, P. V., B. Steinberger, and T. H. Torsvik, 2012, Absolute plate motions in a reference frame defined by moving hot spots in the Pacific, Atlantic, and Indian oceans, J. Geophys. Res., 117(B09101), doi:10.1029/2011jb009072.
