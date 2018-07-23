.. index:: ! mgd77info

*********
mgd77info
*********

.. only:: not man

    mgd77info - Extract information about MGD77 files

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77info** *NGDC-ids*
[ |-C|\ [**m**\ \|\ **e**] ]
[ |-E|\ [**m**\ \|\ **e**] ] [ **-I**\ *ignore* ]
[ |-M|\ **f**\ [*item*]\|\ **r**\ \|\ **e**\ \|\ **h** ]
[ |-L|\ [**v**\ ] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mgd77info** reads <legid>.[mgd77\|nc] files and produces a single
record of information about each cruise specified. The information
includes beginning and end times, total track distances in km, longitude
and latitude range, and the total number of geophysical observations.
Optionally, choose instead to see the original MGD77 header meta-data
section or its individual members.

If you need to know which tracks are crossing through a given region and
what kinds of geophysical observations are available, consider using the
x2sys tools to set up a tracks index data base (see **x2sys\_init** for
more information). 

Required Arguments
------------------

.. include:: explain_ncid.rst_

Optional Arguments
------------------

.. _-C:

**-C**\ [**m**\ \|\ **e**]
    List abbreviations for all columns present in the MGD77[+] files.
    Append **m** or **e** to limit the display to the MGD77 standard or
    MGD77+ extended set only.

.. _-E:

**-E**\ [**m**\ \|\ **e**]
    Give a one-line summary for each cruise listed.

.. _-M:

**-Mf**\ [*item*]\|\ **r**\ \|\ **e**\ \|\ **h**
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

.. _-I:

**-I**\ *ignore*
    Ignore certain data file formats from consideration. Append
    **a\|c\|m\|t** to ignore MGD77 ASCII, MGD77+ netCDF, MGD77T ASCII or plain
    tab-separated ASCII table files, respectively. The option may be
    repeated to ignore more than one format. [Default ignores none].

.. _-L:

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
 
.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_
    
.. include:: ../../explain_help.rst_

Examples
--------

To get one-line summary information about the cruises 01010047.mgd77 and
01010008.mgd77, try

   ::

    gmt mgd77info 01010047 01010008 -E > listing.lis

To see the original raw MGD77 header meta-data for cruise 01010047.mgd77, run

   ::

    gmt mgd77info 01010047 -Mr

To determine all the parameters related to Gravity during cruise 01010047.mgd77, run

   ::

    gmt mgd77info 01010047 -Mf | grep Gravity

To determine the Magnetic sampling rate used during cruise 01010047.mgd77, run

   ::

    gmt mgd77info 01010047 -MfMagnetics_Sampling_Rate

To see all the columns that the MGD77+ cruise 01010047.nc contains, run

   ::

    gmt mgd77info 01010047 -C

To see the E77 status of all MGD77+ cruises collected by the University
of Hawaii (institution 08), run

   ::

    gmt mgd77info 08 -Ia -Me

See Also
--------

:doc:`mgd77list`,
:doc:`mgd77manage`,
:doc:`mgd77path`,
:doc:`mgd77track`,
:doc:`x2sys_init <../x2sys/x2sys_init>`

References
----------

The Marine Geophysical Data Exchange Format - MGD77, see
`http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt. <http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt.>`_
