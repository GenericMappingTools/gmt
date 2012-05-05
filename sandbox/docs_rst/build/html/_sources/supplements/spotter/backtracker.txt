***********
backtracker
***********


backtracker - Generate forward and backward flowlines and hotspot tracks

`Synopsis <#toc1>`_
-------------------

**backtracker** [ *table* ] **-E**\ *rot\_file* \|
**-e**\ *lon*/*lat*/*angle* [ **-A**\ [*young*/*old*] ] [
**-Df**\ \|\ **b** ] [ **-F**\ *drift.txt* ] [
**-Lf**\ \|\ **b**\ *step* ] [ **-N**\ *upper\_age* ] [
**-Q**\ *fixed\_age* ] [ **-S**\ *filestem* ] [ **-T**\ *zero\_age* ] [
**-V**\ [*level*\ ] ] [ **-W**\ [**a**\ \|\ **t**] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**backtracker** reads (longitude, latitude, age) positions from
*infiles* [or standard input] and computes rotated (x,y,t) coordinates
using the specified rotation parameters. It can either calculate final
positions [Default] or create a sampled track (flowline or hotspot
track) between the initial and final positions. The former mode allows
additional data fields after the first 3 columns which must have
(longitude,latitude,age). See option **-:** on how to read
(latitude,longitude,age) files.

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

**-e**\ *lon*/*lat*/*angle*
    Alternatively, specify the longitude, latitude, and opening angle
    (all in degrees and separated by /) for a single total
    reconstruction rotation that should be applied to all input points
    (input ages, if present, are ignored).

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncol*\ ][**t**\ ]) data
    table file(s) holding a number of data columns. If no tables are
    given then we read from standard input.
**-A**\ [*young*/*old*]
    Used in conjunction with **-Lb**\ \|\ **f** to limit the track
    output to those sections whose predicted ages lie between the
    specified *young* and *old* limits. If **-LB**\ \|\ **F** is used
    instead then the limits apply to the stage ids (id 1 is the youngest
    stage). If no limits are specified then individual limits for each
    record are expected in columns 4 and 5 of the input file.
**-Df**\ \|\ **b**
    Set the direction to go: **-Df** will go backward in time (from
    younger to older positions), while **-Db** will go forward in time
    (from older to younger positions) [Default]. Note: For **-Db** you
    are specifying the age at the given location, whereas for **-Df**
    you are not; instead you specify the age at the reconstructed point.
**-F**\ *drift.txt*
    Supply a file with lon, lat, age records that describe the history
    of hotspot motion for the current hotspot. The reconstructions will
    only use the 3rd data input column (i.e., the age) to obtain the
    location of the hotspot at that time, via an interpolation of the
    hotspot motion history. This adjusted location is then used to
    reconstruct the point or path [No drift].
**-Lf**\ \|\ **b**\ *step*
    Specify a sampled path between initial and final position: **-Lf**
    will draw particle flowlines, while **-Lb** will draw backtrack
    (hotspot track) paths. Append sampling interval in km. If *step* < 0
    then only the rotation times will be returned. When **-LF** or
    **-LB** is used, the third output column will contain the stage id
    (1 is youngest) [Default is along-track predicted ages]. You can
    control the direction of the paths by using **-D**.
**-N**\ *upper\_age*
    Set the maximum age to extend the oldest stage rotation back in time
    [Default is no extension].
**-Q**\ *fixed\_age*
    Assign a fixed age to all positions. Only lon, lat input is expected
    [Default expects longitude, latitude, age]. Useful when the input
    are points defining isochrons.
**-S**\ *filestem*
    When **-L** is set, the tracks are normally written to *stdout* as a
    multisegment file. Specify a *filestem* to have each track written
    to *filestem.#*, where *#* is the track number. The track number is
    also copied to the 4th output column.
**-T**\ *zero\_age*
    Set the current time [Default is 0 Ma].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**\ [**a**\ \|\ **t**]
    Rotates the given input (lon,lat,t) and calculates the confidence
    ellipse for the projected point. The input point *must* have a time
    coordinate that exactly matches a particular total reconstruction
    rotation time, otherwise the point will be skipped. Append **t** or
    **a** to output time or angle, respectively, after the projected
    lon, lat. After these 2-3 items, we write azimuth, major, minor (in
    km) for the 95% confidence ellipse. See **-D** for the direction of
    rotation.
**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input. [Default is 3 input columns].
**-bo**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary output. [Default depends on settings].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] (\*)
    Select input columns.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Examples <#toc6>`_
-------------------

To backtrack the (x,y,t) points in the file seamounts.d to their origin
(presumably the hotspot), using the DC85.d Euler poles, run

**backtracker** seamounts.d **-Db** **-E**\ DC85.d > newpos.d

To project flowlines forward from the (x,y,t) points stored in several
3-column, binary, double precision files, run

**backtracker** points.\* **-Df** **-E**\ DC85.d **-Lf**\ 25 **-bo**
**-bi**\ *3* > lines.b

This file can then be plotted with **psxy**.

To compute the predicted Hawaiian hotspot track from 0 to 80 Ma every 1
Ma, given a history of hotspot motion file (HIdrift.txt) and a set of
total reconstruction rotations for the plate (PAC\_APM.d), try

echo 204 19 80 \| **backtracker** **-Df** **-E**\ PAC\_APM.d **-Lb**\ 1
> path.d

`See Also <#toc7>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ , `*project*\ (1) <project.html>`_ ,
`*grdrotater*\ (1) <grdrotater.html>`_ ,
`*grdspotter*\ (1) <grdspotter.html>`_ ,
`*mapproject*\ (1) <mapproject.html>`_ ,
`*hotspotter*\ (1) <hotspotter.html>`_ ,
`*originator*\ (1) <originator.html>`_

`References <#toc8>`_
---------------------

Wessel, P., 1999, "Hotspotting" tools released, EOS Trans. AGU, 80 (29),
p. 319.

