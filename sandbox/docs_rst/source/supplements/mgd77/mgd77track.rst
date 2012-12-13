**********
mgd77track
**********

mgd77track - Plot track-line map of MGD77 cruises

`Synopsis <#toc1>`_
-------------------

**mgd77track** *NGDC-ids*
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] **-J**\ *parameters* [
**-A**\ [**c**\ ][*size*\ ][,\ *spacing*] ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [
**-C**\ **f**\ \|\ **g**\ \|\ **e** ] [ **-Da**\ *startdate* ] [
**-Db**\ *stopdate* ] [ **-F** ] [ **-G**\ **d**\ \|\ **t**\ *gap* ] [
**-I**\ *ignore* ] [ **-K** ] [ **-L**\ *trackticks* ] [ **-O** ] [
**-P** ] [ **-Sa**\ *startdist*\ [**u**\ ] ] [
**-Sb**\ *stopdist*\ [**u**\ ] ] [
**-TT**\ \|\ **t**\ \|\ **d**\ *ms*,\ *mc*,\ *mfs*,\ *mf*,\ *mfc* ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [ **-W**\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-c**\ *copies* ] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**mgd77track** reads NGDC MGD77 cruises and creates *PostScript* code
that will plot one or more ship tracks on a map using the specified
projection. The *PostScript* code is written to standard output.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*NGDC-ids*
    Can be one or more of five kinds of specifiers:

    1) 8-character NGDC IDs, e.g., 01010083, JA010010etc., etc.

    2) 2-character <agency> codes which will return all cruises from
    each agency.

    3) 4-character <agency><vessel> codes, which will return all cruises
    from those vessels.

    4) =<list>, where <list> is a table with NGDC IDs, one per line.

    5) If nothing is specified we return all cruises in the data base.

    (See mgd77info **-L** for agency and vessel codes). The ".mgd77" or
    ".nc" extensions will automatically be appended, if needed (use
    **-I** to ignore certain file types). Cruise files will be looked
    for first in the current directory and second in all directories
    listed in **$MGD77\_HOME**/mgd77\_paths.txt [If **$MGD77\_HOME** is
    not set it will default to **$GMT\_SHAREDIR**/mgd77].

**-J**\ *parameters* (\*)
    Select map projection.
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
    *west*, *east*, *south*, and *north* specify the region of interest,
    and you may specify them in decimal degrees or in
    [+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left
    and upper right map coordinates are given instead of w/e/s/n. The
    two shorthands **-Rg** and **-Rd** stand for global domain (0/360
    and -180/+180 in longitude respectively, with -90/+90 in latitude).
    Alternatively, specify the name of an existing grid file and the
    **-R** settings (and grid spacing, if applicable) are copied from
    the grid. Using **-R**\ *unit* expects projected (Cartesian)
    coordinates compatible with chosen **-J** and we inversely project
    to determine actual rectangular geographic region.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [**c**\ ][*size*\ ][,\ *spacing*]
    Append **c** to annotate using the MGD77 cruise ID [Default uses the
    filename prefix]. Optional *size* is the font size in points. The
    leg annotation font is controlled by **FONT\_LABEL**. By default,
    each leg is annotated every time it enters the map region.
    Alternatively, append ,\ *spacing* to place this label every
    *spacing* units apart along the track. Append one of the units **k**
    (km), **n** (nautical mile), **d** (day), or **h** (hour).
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-C**\ **f**\ \|\ **g**\ \|\ **e**
    Select procedure for along-track distance calculation:
     `` `` `` `` **f** Flat Earth distances.
     `` `` `` `` **g** Great circle distances [Default].
     `` `` `` `` **e** Geodesic distances on current GMT ellipsoid.
**-Da**\ *startdate*
    Do not plot data collected before *startdate*
    (yyyy-mm-ddBD(T)[hh:mm:ss]) [Default is first day].
**-Db**\ *stopdate*
    Do not plot data collected after *stopdate*
    (yyyy-mm-ddBD(T)[hh:mm:ss]). [Default is last day].
**-F**
    Do not apply the error bit flags if present in a MGD77+ file
    [Default will apply these flags upon reading the data].
**-G**\ **d**\ \|\ **t**\ *gap*
    Let successive point separations exceeding **d**\ *gap* (km) or
    **t**\ *gap* (minutes) indicate a break in the track where we should
    not draw a line [no gaps recognized]. Repeat to use both types of
    gap checking.
**-I**\ *ignore*
    Ignore certain data file formats from consideration. Append
    **a\|c\|t** to ignore MGD77 ASCII, MGD77+ netCDF, or plain table
    files, respectively. The option may be repeated to ignore more than
    one format. [Default ignores none].
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**\ *trackticks*
    To put time/distance log-marks on the track. E.g.
    **a**\ 500\ **ka**\ 24\ **ht**\ 6\ **h** means (**a**)nnotate every
    500 km (**k**) and 24 `**h**\ (ours) <h.ours.html>`_ , with
    (**t**)ickmarks every 500 km and 6 hours. Alternatively you may use
    the modifiers **d** (days) and **n** (nautical miles).
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Sa**\ *startdist*\ [**u**\ ]
    Do not plot data that are less than *startdist* meter along track
    from port of departure. Append **k** for km, **m** for miles, or
    **n** for nautical miles [Default is 0 meters].
**-Sb**\ *stopdist*\ [**u**\ ]
    Do not plot data that are more than *stopdist* meter along track
    from port of departure. Append **k** for km, **m** for miles, or
    **n** for nautical miles [Default is end of track].
**-TT**\ \|\ **t**\ \|\ **d**\ *ms*,\ *mc*,\ *mfs*,\ *mf*,\ *mfc*
    Controls the attributes of the three kinds of markers (**T** for the
    first time marker in a new day, **t** for additional time markers in
    the same day, and **d** for distance markers). For each of these you
    can specify the 5 comma-separated attributes *markersize*,
    *markercolor*, *markerfontsize*, *markerfont*, and
    *markerfontcolor*. Repeat the **-T** option for each marker type.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-W**\ *pen*
    Append *pen* used for the trackline. [Default is 0.25p,black].
    [Default is solid].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
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

To generate a Mercator plot of the track of the cruise 01010007 in the
area 70W to 20E, 40S to 20N, using a Mercator scale of 0.1inch/degree,
label the tracks with 10 points characters, annotate the boundaries
every 10 degrees, draw gridlines every 5 degrees, and mark the track
every day and 1000 km, with ticks every 6 hours and 250 km, and send the
plot to the default printer, enter the following command:

mgd77track 01010007 -R70W/20E/40S/20N **-Jm**\ 0.1 -B10g5 -A10
-La1da1000kf6hf250k \| lpr

`See Also <#toc7>`_
-------------------

`*mgd77info*\ (1) <mgd77info.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_
`*mgd77list*\ (1) <mgd77list.html>`_

`References <#toc8>`_
---------------------

Wessel, P., W. H. F. Smith, R. Scharroo, and J. Luis, 2011, The Generic
Mapping Tools (GMT) version 5.0.0b Technical Reference & Cookbook,
SOEST/NOAA.

Wessel, P., and W. H. F. Smith, 1998, New, Improved Version of Generic
Mapping Tools Released, EOS Trans., AGU, 79 (47), p. 579.

Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic Mapping
Tools Released, EOS Trans., AGU, 76 (33), p. 329.

Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic Mapping
Tools Released,
`http://www.agu.org/eos\_elec/95154e.html, <http://www.agu.org/eos_elec/95154e.html,>`_
Copyright 1995 by the American Geophysical Union.

Wessel, P., and W. H. F. Smith, 1991, Free Software Helps Map and
Display Data, EOS Trans., AGU, 72 (41), p. 441.

The Marine Geophysical Data Exchange Format - MGD77, see
`*http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt* <http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt>`_
