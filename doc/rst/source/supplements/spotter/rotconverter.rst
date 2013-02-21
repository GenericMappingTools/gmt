**************
rotconverter
**************

rotconverter - Manipulate total reconstruction and stage rotations

`Synopsis <#toc1>`_
-------------------

**rotconverter** [ **+**\ \|\ **-** ] *rotA* [ **+**\ \|\ **-** *rotB* ]
[ **+**\ \|\ **-** *rotC* ] ... [ **-A** ] [ **-D** ] [
**-E**\ [*fact*\ ] ] [ **-F**\ *out* ] [ **-G** ] [ **-N** ] [ **-S** ]
[ **-T** ] [ **-V**\ [*level*\ ] ] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ]

`Description <#toc2>`_
----------------------

**rotconverter** reads one or more plate motion models (stage or total
reconstruction rotations) stored in the given files. If more than one
plate motion model is given we will add or subtract them in the order
they were listed. The minus sign means we should first transpose the
rotation and then add it to the previous rotation. If a file cannot be
opened we will attempt to decode the file name as a single rotation
whose parameters are separated by slashes.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*rotX*
    Name of a file with a plate motion model. Separate several files
    with desired operator (+ or -). The very first file may also have a
    leading minus operator to imply a transpose. We also recognize
    filenames of the form A-B, where both A and B are uppercase plate
    abbreviations as used by GPlates, to indicate we should look up the
    rotation between the two plates in the GPlates rotation file (e.g.,
    PAC-MBL). If any of the specified rotation models cannot be opened
    as a file, we will try to decode the file name as
    *lon/lat/tstart*\ [*/tstop*\ ]/\ *angle* for a single rotation given
    on the command line. The *tstop* argument is required for stage
    poles only. For a single total reconstruction rotation without any
    time information, give *lon/lat/angle* only.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**
    Indicate that times are actually just opening angles [times in Myr].
**-D**
    Report longitudes use the -180/+180 range [Default is 0/360].
**-E**\ [*fact*\ ]
    Scale opening angles by *fact* on output. Requires stage pole output
    (see **-F**).
**-F**\ *out*
    Specify the output format for rotations. The *out* flag must be
    either **t** or **s** for total reconstruction or stage rotations,
    respectively. [Default is **-Ft** (output contains total
    reconstruction rotations)].
**-G**
    Output final rotations in the Plates4 format used by GPlates
    [Default is spotter format].
**-N**
    Place all output poles in the northern hemisphere [Default reports
    positive rotation angles].
**-S**
    Place all output poles in the southern hemisphere [Default reports
    positive rotation angles].
**-T**
    Transpose the final result, i.e., change the sign of the rotation
    angles.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c]. Report statistics of extracted
    rotations.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc6>`_
-------------------

To convert the total reconstruction rotations in the file
model\_total\_reconstruction.APM to stage poles, run

rotconverter model\_total\_reconstruction.APM -Fs > model\_stages.APM

To obtain Nazca motion relative to Pacific hotspots by adding the motion
of Nazca relative to a fixed Pacific to the Pacific-Hotspot reference
model DC85\_stages.d, and report the result as total reconstruction
reconstruction poles in the northern hemisphere, try

rotconverter DC85\_stages.APM + Pac\_Naz\_stages.RPM -N -Ft >
Naz\_HS\_total reconstruction.APM

To add the final rotations ROT(150.1, 70.5, -20.3) and ROT (145.0, 40.0,
11.4), try

rotconverter 150.1/70.5/-20.3 + 145/40/11.4

 which prints out 157.32, -80.44, 11.97.

To make stage rotations suitable for generating flowlines (fracture
zones) from a model of relative plate motions PL1-PL2.RPM, assuming
symmetric spreading,, try

rotconverter PL1-PL2.RPM -E -Fs > PL1-PL2\_half.RPM
 rotconverter - PL1-PL2.RPM -E -Fs > PL2-PL1\_half.RPM

To compute rotations for India relative to a fixed Africa using the
plate circuit India-Central Indian Basin-Antarctica-Africa, based on the
GPlates rotations database, try

rotconverter IND-CIB CIB-ANT ANT-AFR > India\_AFrica.RPM

`See Also <#toc7>`_
-------------------

`*backtracker*\ (1) <backtracker.html>`_ ,
`*grdrotater*\ (1) <grdrotater.html>`_ ,
`*grdspotter*\ (1) <grdspotter.html>`_ ,
`*hotspotter*\ (1) <hotspotter.html>`_ ,
`*originator*\ (1) <originator.html>`_
