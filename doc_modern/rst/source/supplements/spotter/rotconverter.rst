.. index:: ! rotconverter

**************
rotconverter
**************

.. only:: not man

    rotconverter - Manipulate total reconstruction and stage rotations

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt rotconverter** [ **+**\ \|\ **-** ] *rotA* [ **+**\ \|\ **-** *rotB* ]
[ **+**\ \|\ **-** *rotC* ] ... [ |-A| ] [ |-D| ]
[ |-F|\ *out* ]
[ |-G| ]
[ |-M|\ [*fact*] ]
[ |-N| ] [ |-S| ]
[ |-T| ] [ |-W| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**rotconverter** reads one or more plate motion models (stage or total
reconstruction rotations) stored in the given files. If more than one
plate motion model is given we will add or subtract them in the order
they were listed. The minus sign means we should first transpose the
rotation and then add it to the previous rotation. If a file cannot be
opened we will attempt to decode the file name as a single rotation
whose parameters are separated by slashes.

Required Arguments
------------------

*rotX*
    Name of a file with a plate motion model. Separate several files
    with desired operator (+ or -). The very first file may also have a
    leading minus operator to imply a transpose. We also recognize
    filenames of the form A-B, where both A and B are uppercase plate
    abbreviations as used by GPlates, to indicate we should look up the
    rotation between the two plates in the GPlates rotation file (e.g.,
    PAC-MBL). If any of the specified rotation models cannot be opened
    as a file, we will try to decode the file name as
    *lon/lat/tstart*\ [*/tstop*]/\ *angle* for a single rotation given
    on the command line. The *tstop* argument is required for stage
    poles only. For a single total reconstruction rotation without any
    time information, give *lon/lat/angle* only.

Optional Arguments
------------------

.. _-A:

**-A**
    Indicate that times are actually just opening angles [times in Myr].

.. _-D:

**-D**
    Report longitudes use the -180/+180 range [Default is 0/360].

.. _-F:

**-F**\ *out*
    Specify the output format for rotations. The *out* flag must be
    either **t** or **s** for total reconstruction or stage rotations,
    respectively. [Default is **-Ft** (output contains total
    reconstruction rotations)].

.. _-G:

**-G**
    Output final rotations in the Plates4 format used by GPlates [Default is spotter format].

.. _-M:

**-M**\ [*fact*]
    Scale opening angles by *fact* on output [0.5]. 
    Typically used to get half-rates needed for flowlines.
    Requires stage pole output (see **-F**).

.. _-N:

**-N**
    Place all output poles in the northern hemisphere [Default reports positive rotation angles].

.. _-S:

**-S**
    Place all output poles in the southern hemisphere [Default reports positive rotation angles].

.. _-T:

**-T**
    Transpose the final result, i.e., change the sign of the rotation angles.

.. _-W:

**-W**
    Ensure all output rotations have negative opening angles [Default reports positive rotation angles].

.. _-V:

.. |Add_-V| replace:: Report statistics of extracted rotations.
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

Limitations
-----------

Note that only one of |-N|, |-S|, and |-W| can be used at the same time.

Examples
--------

To convert the total reconstruction rotations in the file
model_total_reconstruction.APM to stage poles, run

   ::

    gmt rotconverter model_total_reconstruction.APM -Fs > model_stages.APM

To obtain Nazca motion relative to Pacific hotspots by adding the motion
of Nazca relative to a fixed Pacific to the Pacific-Hotspot reference
model DC85_stages.d, and report the result as total reconstruction
reconstruction poles in the northern hemisphere, try

   ::

    gmt rotconverter DC85_stages.APM + Pac_Naz_stages.RPM -N -Ft > \
                     Naz_HS_total reconstruction.APM

To add the final rotations ROT(150.1, 70.5, -20.3) and ROT (145.0, 40.0, 11.4), try

   ::

    gmt rotconverter 150.1/70.5/-20.3 + 145/40/11.4

which prints out 157.32, -80.44, 11.97.

To make stage rotations suitable for generating flowlines (fracture
zones) from a model of relative plate motions PL1-PL2.RPM, assuming
symmetric spreading, try

   ::

    gmt rotconverter PL1-PL2.RPM -M -Fs > PL1-PL2_half.RPM
    gmt rotconverter - PL1-PL2.RPM -M -Fs > PL2-PL1_half.RPM

To compute rotations for India relative to a fixed Africa using the
plate circuit India-Central Indian Basin-Antarctica-Africa, based on the
GPlates rotations database, try

   ::

    gmt rotconverter IND-CIB CIB-ANT ANT-AFR > India_Africa.RPM

Notes
-----

GMT distributes the EarthByte rotation model Global_EarthByte_230-0Ma_GK07_AREPS.rot.
To use an alternate rotation file, create an environmental parameters named
**GPLATES_ROTATIONS** that points to an alternate rotation file.

See Also
--------

:doc:`backtracker`,
:doc:`grdrotater`,
:doc:`grdspotter`,
:doc:`gmtpmodeler`,
:doc:`grdpmodeler`,
:doc:`grdrotater`,
:doc:`hotspotter`,
:doc:`originater`
