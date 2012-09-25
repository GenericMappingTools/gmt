***********
grdpmodeler
***********

grdpmodeler - Evaluate a plate model on a geographic grid

`Synopsis <#toc1>`_
-------------------

**grdpmodeler** *agegrdfile* **-E**\ *rot\_file* **-G**\ *outgrdfile*
**-S**\ **a**\ \|\ **d**\ \|\ **r**\ \|\ **w**\ \|\ **x**\ \|\ **y**\ \|\ **X**\ \|\ **Y**
[ **-F**\ *polygonfile* ] [ **-T**\ *age* ] [ **-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**grdpmodeler** reads a geographical age grid and a plate motion model
and evaluates one of several model predictions. Optionally, the user may
supply a clipping polygon in multiple-segment format; then, only the
part of the grid inside the polygon is used to determine the model
prediction; the remainder of the grid is set to NaN.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*ingrdfile*
    Name of a grid file in geographical (lon, lat) coordinates with ages
    in Myr.
**-E**\ *rotfile*
    Give file with rotation parameters. This file must contain one
    record for each rotation; each record must be of the following
    format:

    `` `` `` `` *lon lat tstart [tstop] angle* [ *khat a b c d e f g df*
    ]

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

**-G**\ *outgrdfile*
    Name of output grid. This is the grid with the model predictions
    given the specified rotations.
**-S**\ **a**\ \|\ **d**\ \|\ **r**\ \|\ **w**\ \|\ **x**\ \|\ **y**\ \|\ **X**\ \|\ **Y**
    Type of model prediction. Choose from **a** for plate motion
    azimuth, **d** for distance between current locations and their
    origin at the ridge (in km), **r** for plate motion rate (in mm/yr),
    **w** for plate rotation rate (degree/Myr), **x** change in
    longitude relative to location of crust formation, **y** change in
    latitude relative to location of crust formation, **X** longitude of
    crust formation, and **Y** latitude of crust formation.

`Optional Arguments <#toc5>`_
-----------------------------

**-F**\ *polygonfile*
    Specify a multisegment closed polygon file that describes the inside
    area of the grid where the model should be evaluated; the outside
    will be set to NaN [Default evaluates model on the entire grid].
**-T**\ *age*
    Use a fixed age for model evaluation (i.e., override the ages in the
    age grid). This lets you evaluate the model at a snapshot in time.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-:**
    Toggles between (longitude,latitude) and (latitude,longitude)
    input/output. [Default is (longitude,latitude)].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
(\*)
    Select input columns.
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

We will use a grid with Pacific crust ages (pac\_age.nc), a plate motion
model (Pac\_APM.d), and a polygon that contains the outline of the
present Pacific plate (pac\_clip\_path.d). To evaluate the plate motion
azimuths at the present time for the Pacific, try

grdpmodeler pac\_age.nc -EPac\_APM.d -V -Fpac\_clip\_path.d
-Gpac\_dir\_0.nc -Sa -T0

To determine the changes in latitude since crust formation for the
entire Pacific, try

grdpmodeler pac\_age.nc -EPac\_APM.d -V -Fpac\_clip\_path.d
-Gpac\_dlat.nc -Sy

To determine the plate motion rates in effect when the Pacific crust was
formed, try

grdpmodeler pac\_age.nc -EPac\_APM.d -V -Fpac\_clip\_path.d
-Gpac\_vel.nc -Sr

To determine how far the crust has moved since formation, try

grdpmodeler pac\_age.nc -EPac\_APM.d -V -Fpac\_clip\_path.d
-Gpac\_dist.nc -Sd

`See Also <#toc7>`_
-------------------

`*backtracker*\ (1) <backtracker.html>`_ ,
`*grdrotater*\ (1) <grdrotater.html>`_ ,
`*grdspotter*\ (1) <grdspotter.html>`_ ,
`*hotspotter*\ (1) <hotspotter.html>`_ ,
`*originator*\ (1) <originator.html>`_
`*rotconverter*\ (1) <rotconverter.html>`_
