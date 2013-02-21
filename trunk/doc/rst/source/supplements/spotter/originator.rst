**********
originator
**********

originator - Associate seamounts with nearest hotspot point sources

`Synopsis <#toc1>`_
-------------------

**originator** [ *table* ] **-E**\ [**+**\ ]\ *rotfile*
**-F**\ [**+**\ ]\ *hs\_file* [ **-D**\ *d\_km* ] [ **-L**\ [*flag*\ ] ]
[ **-N**\ *upper\_age* ] [ **-Q**\ *r/t* ] [ **-S**\ [*n\_hs*\ ] ] [
**-T** ] [ **-V**\ [*level*\ ] ] **-W**\ *maxdist* ] [ **-Z** ] [
**-bi**\ [*ncols*\ ][*type*\ ] ] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**originator** reads (longitude, latitude, height, radius, crustal\_age)
records from *infiles* [or standard input] and uses the given Absolute
Plate Motion (APM) stage or total reconstruction rotation file and the
list of hotspot locations to determine the most likely origin (hotspot)
for each seamount. It does so by calculating flowlines back in time and
determining the closest approach to all hotspots. The output consists of
the input records with four additional fields added for each of the
*n\_hs* closest hotspots. The four fields are the hotspot id (e.g.,
HWI), the stage id of the flowline segment that came closest, the
pseudo-age of the seamount, and the closest distance to the hotspot (in
km). See option **-:** on how to read (latitude, longitude,height,
radius, crustal\_age) files.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

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
    if the rotation cannot be found. Prepend **+** if you want to invert
    the rotations prior to use.

**-F**\ *file*
    Give file with hotspot locations. This file must contain one record
    for each hotspot to be considered; each record must be of the
    following format:

    lon lat hs\_abbrev hs\_id r t\_off t\_on create fit plot name

    E.g., for Hawaii this may look like

    205 20 HWI 1 25 0 90 Y Y Y Hawaii

    Most applications only need the first 4 columns which thus
    represents the minimal hotspot information record type. The
    abbreviation may be maximum 3 characters long. The id must be an
    integer from 1-32. The positional uncertainty of the hotspot is
    given by r (in km). The t\_off and t\_on variables are used to
    indicate the active time-span of the hotspot. The create, fit, and
    plot indicators are either Y or N and are used by some programs to
    indicate if the hotspot is included in the ID-grids used to
    determine rotations, if the hotspot chain will be used to determine
    rotations, and if the hotspot should be included in various plots.
    The name is a 32-character maximum text string with the full hotspot
    name. Blank lines and records whose first column contains # will be
    ignored. Prepend **+** if we should look for hotspot drift tables
    whose name must be *hs\_abbrev*\ \_drift.txt. Such files may be
    located in the current directory, the same directory as *hs\_file*,
    or in the directories pointed to by GMT\_DATADIR. If found then we
    interpolate to get hotspot location as a function of time [fixed].

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-D**\ *d\_km*
    Sets the flowline sampling interval in km. [Default is 5].
**-L**\ [*flag*\ ]
    Output closest approach for nearest hotspot only (ignores **-S**).
    Choose **-Lt** for (*time*, *dist*, *z*) [Default], **-Lw** for
    (*omega*, *dist*, *z*), and **-Ll** for (lon, lat, time, dist, z).
    Normally, *dist* is in km; use upper case modifiers **TWL** to get
    *dist* in spherical degrees.
**-N**\ *upper\_age*
    Set the maximum age to extend the oldest stage back in time [no
    extension].
**-Q**\ *r/t*
Input files only has (*x*,\ *y*,\ *z*); specify constant values for
*r*,\ *t* that
    will be implied for each record.
**-S**\ [*n\_hs*\ ]
    Set the number of closest hotspots to report [Default is 1].
**-T**
    Truncate seamount ages exceeding the upper age set with **-N** [no
    truncation].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**\ *maxdist*
    Only report those seamounts whose flowlines came within *maxdist* to
    any hotspot [Default reports all seamounts].
**-Z**
    Use the hotspot ID number rather than the name tag in output
    records.
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 5 input columns].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
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

To find the likely (hotspot) origins of the seamounts represented by the
(x,y,z,r,tc) points in the file seamounts.d, using the DC85.d Euler
poles and the pac\_hs.d list of possible hotspots, and report the 2 most
likely hotspot candidates for each seamount, run

originator seamounts.d -S2 -EDC85.d -Fpac\_hs.d > origins.d

To determine the predicted age of a seamount, distances to the closest
hotspot, and echo the observed age given its location, observed age, and
a rotation model, try

echo "1.55 -8.43 52.3" \| originator -FONeill\_2005\_hotspots.txt
-EOMS2005\_APM\_fixed.txt -Q1/120 -Lt

where 52.3 Ma is observed age. The output is 70 -95.486 52.3. To repeat
the same exercise with a moving hotspot model, try

echo "1.55 -8.43 52.3" \| originator -F+ONeill\_2005\_hotspots.txt
-EOMS2005\_APM\_smooth.txt -Q1/120 -Lt

Now the output is 80 -213.135 52.3. Negative distances means the closest
approach was east of the hotspot.

`See Also <#toc7>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ , `*grdrotater*\ (1) <grdrotater.html>`_ ,
`*grdspotter*\ (1) <grdspotter.html>`_ ,
`*project*\ (1) <project.html>`_ ,
`*mapproject*\ (1) <mapproject.html>`_ ,
`*backtracker*\ (1) <backtracker.html>`_ ,
`*hotspotter*\ (1) <hotspotter.html>`_

`References <#toc8>`_
---------------------

Wessel, P., 1999, "Hotspotting" tools released, EOS Trans. AGU, 80 (29),
p. 319.

