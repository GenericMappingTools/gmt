.. index:: ! mgd77track

**********
mgd77track
**********

.. only:: not man

    mgd77track - Plot track-line map of MGD77 cruises

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**mgd77track** *NGDC-ids*
|SYN_OPT-R|
|-J|\ *parameters*
[ |-A|\ [**c**][*size*][**+i**\ *spacing*] ]
[ |SYN_OPT-B| ]
[ **-Da**\ *startdate* ] 
[ |-D|\ **b**\ *stopdate* ]
[ |-F| ]
[ |-G|\ **d**\ \|\ **t**\ \|\ **n**\ *gap* ]
[ |-I|\ *ignore* ]
[ |-K| ]
[ |-L|\ *trackticks* ]
[ |SYN_OPT-O| ]
[ |SYN_OPT-P| ]
[ |-S|\ **a**\ *startdist*\ [**u**] ]
[ |-S|\ **b**\ *stopdist*\ [**u**] ]
[ |-T|\ **T**\ \|\ **t**\ \|\ **d**\ *ms*,\ *mc*,\ *mfs*,\ *mf*,\ *mfc* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: mgd77track_common.rst_

.. include:: ../../common_classic_sup.rst_

Examples
--------

To generate a Mercator plot of the track of the cruise 01010007 in the
area 70W to 20E, 40S to 20N, using a Mercator scale of 0.1inch/degree,
label the tracks with 10 points characters, annotate the boundaries
every 10 degrees, draw gridlines every 5 degrees, and mark the track
every day and 1000 km, with ticks every 6 hours and 250 km, and send the
plot to the default printer, enter the following command:

   ::

    gmt mgd77track 01010007 -R70W/20E/40S/20N -Jm0.1 -B10g5 -A10 \
                   -La1da1000kf6hf250k \| lpr

.. include:: mgd77track_notes.rst_

See Also
--------

:doc:`mgd77info`,
:doc:`psbasemap </psbasemap>`,
:doc:`mgd77list`
