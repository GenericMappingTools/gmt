**********
mgd77track
**********

mgd77track - Plot track-line map of MGD77 cruises

`Synopsis <#toc1>`_
-------------------

.. include:: ../../common_SYN_OPTs.rst_

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
**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**mgd77track** reads NGDC MGD77 cruises and creates *PostScript* code
that will plot one or more ship tracks on a map using the specified
projection. The *PostScript* code is written to standard output.

.. include:: ../../explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

.. include:: explain_ncid.rst_

.. include:: ../../explain_-J.rst_
 
.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

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
    
.. include:: ../../explain_-B.rst_

**-C**\ **f**\ \|\ **g**\ \|\ **e**
    Select procedure for along-track distance calculation:
        **f** Flat Earth distances.
        **g** Great circle distances [Default].
        **e** Geodesic distances on current GMT ellipsoid.
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

.. include:: ../../explain_-K.rst_

**-L**\ *trackticks*
    To put time/distance log-marks on the track. E.g.
    **a**\ 500\ **ka**\ 24\ **ht**\ 6\ **h** means (**a**)nnotate every
    500 km (**k**) and 24 `**h**\ (ours) <h.ours.html>`_ , with
    (**t**)ickmarks every 500 km and 6 hours. Alternatively you may use
    the modifiers **d** (days) and **n** (nautical miles). 

.. include:: ../../explain_-O.rst_

.. include:: ../../explain_-P.rst_

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
    
.. include:: ../../explain_-U.rst_

**-W**\ *pen*
    Append *pen* used for the trackline. [Default is 0.25p,black].
    [Default is solid]. 

.. include:: ../../explain_-XY.rst_
 
.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_
 
.. include:: ../../explain_-c.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_
 
.. include:: ../../explain_-t.rst_
    
.. include:: ../../explain_help.rst_

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

The Marine Geophysical Data Exchange Format - MGD77, see
`*http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt* <http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt>`_
