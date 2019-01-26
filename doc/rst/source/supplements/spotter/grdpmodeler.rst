.. index:: ! grdpmodeler

***********
grdpmodeler
***********

.. only:: not man

    grdpmodeler - Evaluate a plate motion model on a geographic grid

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt grdpmodeler** |-E|\ *rot_file* **-S**\ *flags*
[ *agegrdfile* ]
[ |-F|\ *polygonfile* ]
[ |-G|\ *outgrdfile* ]
[ |-I|\ [**x**]\ *dx*\ [/*dy*] ]
[ |SYN_OPT-R| ]
[ |-T|\ *age* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdpmodeler** reads a geographical age grid and a plate motion model
and evaluates one of several model predictions. Optionally, the user may
supply a clipping polygon in multiple-segment format; then, only the
part of the grid inside the polygon is used to determine the model
prediction; the remainder of the grid is set to NaN.

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

*ingrdfile*
    Name of a grid file in geographical (lon, lat) coordinates with ages in Myr.
    If no grid is provided then you may define the domain via **-R**, **-I**, and optionally **-r**.

.. _-F:

**-F**\ *polygonfile*
    Specify a multisegment closed polygon file that describes the inside
    area of the grid where the model should be evaluated; the outside
    will be set to NaN [Default evaluates model on the entire grid].

.. _-G:

**-G**\ *outgrdfile*
    Name of output grid. This is the grid with the model predictions
    given the specified rotations. Note: If you specified more than one
    model prediction in **-S** then the filename *must* be a template
    that contains the format %s; this will be replaced with the corresponding
    tags az, dist, stage, vel, omega, dlon, dlat, lon, lat.
    If the **-G** option is not used then we create no grids and instead
    write *lon, lat, age, predictions* records to standard output.

.. _-T:

**-T**\ *age*
    Use a fixed age for model evaluation (i.e., override the ages in the
    age grid). This lets you evaluate the model at a snapshot in time.
    Required if no age grid was provided.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_help.rst_

.. include:: explain_geodetic.rst_

.. include:: ../../explain_inside.rst_

Examples
--------

We will use a grid with Pacific crust ages (pac_age.nc), a plate motion
model (Pac_APM.txt), and a polygon that contains the outline of the
present Pacific plate (pac_clip_path.txt). To evaluate the plate motion
azimuths at the present time for the Pacific, try

   ::

    gmt grdpmodeler pac_age.nc -EPac_APM.txt -V -Fpac_clip_path.txt \
                    -Gpac_dir_0.nc -Sa -T0

To determine the changes in latitude since crust formation for the
entire Pacific, try

   ::

    gmt grdpmodeler pac_age.nc -EPac_APM.txt -V -Fpac_clip_path.txt \
                    -Gpac_dlat.nc -Sy

To determine the plate motion velocities in effect when the Pacific crust was
formed, try

   ::

    gmt grdpmodeler pac_age.nc -EPac_APM.txt -V -Fpac_clip_path.txt \
                    -Gpac_vel.nc -Sv

To determine how far the crust has moved since formation, try

   ::

    gmt grdpmodeler pac_age.nc -EPac_APM.txt -V -Fpac_clip_path.txt \
                    -Gpac_dist.nc -Sd

To save the coordinates of the crust's formation to separate grids, try

   ::

    gmt grdpmodeler pac_age.nc -EPac_APM.txt -V -Fpac_clip_path.txt \
                    -Gpac_origin_%s.nc -SXY

To repeat the same exercise but save output *lon,lat,age,xorigin,yorigin* to a table, use

   ::

    gmt grdpmodeler pac_age.nc -EPac_APM.txt -V -Fpac_clip_path.txt -SXY > origin.txt

Notes
-----

GMT distributes the EarthByte rotation model Global_EarthByte_230-0Ma_GK07_AREPS.rot.
To use an alternate rotation file, create an environmental parameters named
**GPLATES_ROTATIONS** that points to an alternate rotation file.

See Also
--------

:doc:`backtracker`,
:doc:`gmtpmodeler`,
:doc:`grdrotater`,
:doc:`grdspotter`,
:doc:`hotspotter`,
:doc:`originater`,
:doc:`rotconverter`
