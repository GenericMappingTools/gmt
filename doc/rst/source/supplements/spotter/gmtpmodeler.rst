.. index:: ! pmodeler

********
pmodeler
********

.. only:: not man

    pmodeler - Evaluate a plate motion model at given locations

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt pmodeler** *table* |-E|\ *rot_file* **-S**\ *flags*
[ |-F|\ *polygonfile* ]
[ |-T|\ *age* ]
[ |SYN_OPT-V| ]
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

**pmodeler** reads a table with *lon*, *lat* and optionally *age* triplets
and a plate motion model
and evaluates one of several model predictions. Optionally, the user may
supply a clipping polygon in multiple-segment format; then, only the
part of the points inside the polygon are used to determine the model
prediction.  The results are written to standard output.

Required Arguments
------------------

*table*
    Name of one or more tables with geographical (lon, lat) coordinates and optionally
    a third column with ages in Myr.  If no file is given then we read from standard input.

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
    be ignored. You may prepend a leading + to the filename to indicate
    you wish to invert the rotations.
    Alternatively, give the filename composed of two plate IDs
    separated by a hyphen (e.g., PAC-MBL) and we will instead extract
    that rotation from the GPlates rotation database. We return an error
    if the rotation cannot be found.

.. _-S:

**-S**\ *flags*
    Type of model prediction(s). Append one or more items: choose from **a** for plate motion
    azimuth, **d** for great-circle distance between current location and its
    origin at the ridge (in km), **s** for plate motion model stage ID (1 is youngest),
    **v** for plate motion rate (in mm/yr),
    **w** for plate rotation rate (degree/Myr), **x** for change in
    longitude relative to location of crust formation, **y** for change in
    latitude relative to location of crust formation, **X** for longitude of
    crust formation, and **Y** for latitude of crust formation.
    If no arguments are given we default to all [**adsvwxyXY**].

Optional Arguments
------------------

.. _-F:

**-F**\ *polygonfile*
    Specify a multisegment closed polygon file that describes the
    area where the model should be evaluated; points outside
    will be skipped [use all data points].

.. _-T:

**-T**\ *age*
    Use a fixed age for model evaluation (i.e., override the ages given in the
    input table). This lets you evaluate the model at a snapshot in time, and is
    a required option if the input table does not contain ages.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_-ocols.rst_
.. include:: ../../explain_help.rst_

.. include:: explain_geodetic.rst_

.. include:: ../../explain_inside.rst_

Examples
--------

We will use a table with locations and ages of Pacific crust (pac_age.txt), a plate motion
model (Pac_APM.txt), and a polygon that contains the outline of the
present Pacific plate (pac_clip_path.txt). To evaluate the plate motion
azimuths at the present time for the Pacific, try

   ::

    gmt pmodeler pac_age.txt -EPac_APM.txt -V -Fpac_clip_path.txt \
                     -Sa -T0 > pac_dir_0.txt

To determine the changes in latitude since crust formation for the
entire Pacific, try

   ::

    gmt pmodeler pac_age.txt -EPac_APM.txt -V -Fpac_clip_path.txt \
                    -Sy > pac_dlat.txt

To determine the plate motion velocities in effect when the Pacific crust was
formed, try

   ::

    gmt pmodeler pac_age.txt -EPac_APM.txt -V -Fpac_clip_path.txt \
                    -Sv > pac_vel.txt 

To determine how far the crust has moved since formation, try

   ::

    gmt pmodeler pac_age.txt -EPac_APM.txt -V -Fpac_clip_path.txt \
                    -Sd > pac_dist.txt

To save the coordinates of the crust's formation, try

   ::

    gmt pmodeler pac_age.txt -EPac_APM.txt -V -Fpac_clip_path.txt \
                    -SXY > ac_origin_xy.txt 

Notes
-----

GMT distributes the EarthByte rotation model Global_EarthByte_230-0Ma_GK07_AREPS.rot.
To use an alternate rotation file, create an environmental parameters named
**GPLATES_ROTATIONS** that points to an alternate rotation file.

See Also
--------

:doc:`backtracker`,
:doc:`grdpmodeler`,
:doc:`grdrotater`,
:doc:`grdspotter`,
:doc:`hotspotter`,
:doc:`originater`,
:doc:`rotconverter`
