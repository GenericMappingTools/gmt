*********
mgd77info
*********


mgd77info - Extract information about MGD77 files

`Synopsis <#toc1>`_
-------------------

**mgd77info** *NGDC-ids* [ **-C**\ [**m**\ \|\ **e**] ] [
**-E**\ [**m**\ \|\ **e**] ] [ **-I**\ *ignore* ] [
**-Mf**\ [*item*\ ]\|\ **r**\ \|\ **e**\ \|\ **h** ] [ **-L**\ [**v**\ ]
] [ **-V**\ [*level*\ ] ]

`Description <#toc2>`_
----------------------

**mgd77info** reads <legid>.[mgd77\|nc] files and produces a single
record of information about each cruise specified. The information
includes beginning and end times, total track distances in km, longitude
and latitude range, and the total number of geophysical observations.
Optionally, choose instead to see the original MGD77 header meta-data
section or its individual members.
If you need to know which tracks are crossing through a given region
and what kinds of geophysical observations are available, consider using
the x2sys tools to set up a tracks index data base (see **x2sys\_init**
for more information).

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
    3) 4-character <agency><vessel> codes, which will return all
    cruises from those vessels.
    4) =<list>, where <list> is a table with NGDC IDs, one per line.
    5) If nothing is specified we return all cruises in the data base.
    (See mgd77info **-L** for agency and vessel codes). The ".mgd77" or
    ".nc" extensions will automatically be appended, if needed (use
    **-I** to ignore certain file types). Cruise files will be looked
    for first in the current directory and second in all directories
    listed in **$MGD77\_HOME**/mgd77\_paths.txt [If **$MGD77\_HOME** is
    not set it will default to **$GMT\_SHAREDIR**/mgd77].

`Optional Arguments <#toc5>`_
-----------------------------

**-C**\ [**m**\ \|\ **e**]
    List abbreviations for all columns present in the MGD77[+] files.
    Append **m** or **e** to limit the display to the MGD77 standard or
    MGD77+ extended set only.
**-E**\ [**m**\ \|\ **e**]
    Give a one-line summary for each cruise listed.
**-Mf**\ [*item*\ ]\|\ **r**\ \|\ **e**\ \|\ **h**
    List the meta-data (header) and (if present) the MGD77+ history for
    each cruise. Append **f** for a formatted display. This will list
    individual parameters and their values, one entry per output line,
    in a format that can be searched using standard UNIX text tools.
    Alternatively, append the name of a particular parameter (you only
    need to give enough characters - starting at the beginning - to
    uniquely identify the item). Give - to display the list of all
    parameter names. You may also specify the number of a parameter. For
    the raw, punchcard-formatted MGD77 original header block, append
    **r** instead. For the MGD77+ E77 status, append **e** instead.
    Finally, for the MGD77+ history, append **h** instead.
**-I**\ *ignore*
    Ignore certain data file formats from consideration. Append
    **a\|c\|t** to ignore MGD77 ASCII, MGD77+ netCDF, or plain
    tab-separated ASCII table files, respectively. The option may be
    repeated to ignore more than one format. [Default ignores none].
**-L**\ [**v**\ ]
    No cruise information is listed. Instead, we just display a list of
    the GEODAS institution 2-character codes and their names.
    Optionally, append **v** to also display the vessels and their
    4-character codes for each institution. The following is the list of
    institutions:
    (01) LAMONT (LDEO), (02) WOODS HOLE O.I., (03) NOAA, (04) US ARMY,
    (05) NEW ZEALAND, (06) US GEOL. SURVEY, (07) OREGON ST. UNIV, (08)
    U.HAWAII SOEST, (09) US NAVY, (10) UNIV OF TEXAS, (11) RICE UNIV.,
    (12) CANADA, (13) UNIV OF CONN., (14) U.MIAMI (RSMAS), (15) SCRIPPS
    INST.OC, (16) CHINA, (17) U RHODE ISLAND, (18) DUKE UNIVERSITY, (19)
    UNITED KINGDOM, (20) U.WASHINGTON, (22) WESTERN GEOPHY., (23) TEXAS
    A&M UNIV., (24) AUSTRALIA, (25) MONACO, (29) RUSSIA, (30) SPAIN,
    (35) NIMA, (58) NETHERLANDS, (60) MIN MGMT SVC, (63) ISRAEL, (67)
    FRANCE, (71) SOUTH AFRICA, (75) US COAST GUARD, (76) BRAZIL, (77)
    INT. GRAV. BUR, (83) GERMANY, (84) ORSTOM NEW CAL, (86) CUBA, (87)
    ARGENTINA, (88) US NSF, (89) INDIA, (90) PORTUGAL, (92) FINLAND,
    (93) CHILE, (J1) HYDR DEPT JAPAN, (J2) GEOL SRVY JAPAN, (J4) UNIV
    TOKYO, (J5) KOBE UNIV, (J7) UNIV OF RYUKYUS, (J8) J.O.D.C. JAPAN,
    (J9) CHIBA UNIV, (JA) INST.POLAR RES., (ZZ) INST NOT CODED.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Examples <#toc6>`_
-------------------

To get one-line summary information about the cruises 01010047.mgd77 and
01010008.mgd77, try

**mgd77info** 01010047 01010008 **-E** > listing.lis

To see the original raw MGD77 header meta-data for cruise
01010047.mgd77, run

**mgd77info** 01010047 **-Mr**

To determine all the parameters related to Gravity during cruise
01010047.mgd77, run

**mgd77info** 01010047 **-Mf** \| grep Gravity

To determine the Magnetic sampling rate used during cruise
01010047.mgd77, run

**mgd77info** 01010047 **-Mf**\ Magnetics\_Sampling\_Rate

To see all the columns that the MGD77+ cruise 01010047.nc contains, run

**mgd77info** 01010047 **-C**

To see the E77 status of all MGD77+ cruises collected by the University
of Hawaii, run

**mgd77info** 08 **-Ia** **-Me**

`See Also <#toc7>`_
-------------------

`*mgd77list*\ (1) <mgd77list.1.html>`_ ,
`*mgd77manage*\ (1) <mgd77manage.1.html>`_ ,
`*mgd77path*\ (1) <mgd77path.1.html>`_ ,
`*mgd77track*\ (1) <mgd77track.1.html>`_
`*x2sys\_init*\ (1) <x2sys_init.1.html>`_

`References <#toc8>`_
---------------------

Wessel, P., W. H. F. Smith, R. Scharroo, and J. Luis, 2011, The Generic
Mapping Tools (GMT) version 5.0.0b Technical Reference & Cookbook,
SOEST/NOAA.

Wessel, P., and W. H. F. Smith, 1998, New, Improved Version of Generic
Mapping Tools Released, EOS Trans., AGU, 79 (47), p. 579.

Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic
Mapping Tools Released, EOS Trans., AGU, 76 (33), p. 329.

Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic
Mapping Tools Released,
`http://www.agu.org/eos\_elec/95154e.html, <http://www.agu.org/eos_elec/95154e.html,>`_
Copyright 1995 by the American Geophysical Union.

Wessel, P., and W. H. F. Smith, 1991, Free Software Helps Map and
Display Data, EOS Trans., AGU, 72 (41), p. 441.

The Marine Geophysical Data Exchange Format - MGD77, see
`*http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt*. <http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt.>`_

