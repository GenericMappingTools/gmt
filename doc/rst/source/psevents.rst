.. index:: ! psevents
.. include:: module_core_purpose.rst_

********
psevents
********

|psevents_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt psevents**
|-J|\ *parameters*
|SYN_OPT-Rz|
|-T|\ *now*
[ *table* ]
[ |-A|\ **r**\ [*dpu*\ [**c**\|\ **i**][**+z**\ [*z*]]]\|\ **s** ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ [**j**\|\ **J**]\ *dx*\ [/*dy*][**+v**\ [*pen*]] ]
[ |-E|\ [**s**\|\ **t**\ ][**+o**\|\ **O**\ *dt*][**+r**\ [**c**\|\ **l**\|\ **q**]*dt*][**+p**\ *dt*][**+d**\ [**c**\|\ **l**\|\ **q**]\ *dt*][**+f**\ [**c**\|\ **l**\|\ **q**]\ *dt*][**+l**\ *dt*] ]
[ |-F|\ [**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+r**\ [*first*]\|\ **z**\ [*format*]] ]
[ |-G|\ *color* ]
[ |-H|\ *labelbox* ]
[ |-K| ]
[ |-L|\ [*length*\|\ **t**] ]
[ |-M|\ **i**\|\ **s**\|\ **t**\|\ **z**\ *val1*\ [**+c**\ *val2*] ]
[ |-N|\ [**c**\|\ **r**] ]
[ |-O| ] [ |-P| ]
[ |-Q|\ *prefix* ]
[ |-S|\ *symbol*\ [*size*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ *command* ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-tv| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: events.rst
    :start-after: .. module_common_begins
    :end-before: .. module_common_ends

.. include:: common_classic.rst_

Examples
--------

To show the display of events visible for May 1, 2018 given the catalog of
large (>5) magnitude earthquakes that year, using a 2-day rise time during
which we boost symbol size by a factor of 5 and wash out the color, followed
by a decay over 6 days and then a final shrinking to half size and darken the
color, we may try::

    gmt convert "https://earthquake.usgs.gov/fdsnws/event/1/query.csv?starttime=2018-01-01%2000:00:00&endtime=2018-12-31%2000:00:00&minmagnitude=5&orderby=time-asc" \
        -i2,1,3,4+s50,0 -hi1 > q.txt
    gmt makecpt -Cred,green,blue -T0,70,300,10000 > q.cpt
    gmt psevents -Rg -JG200/5/6i -Baf q.txt -SE- -Cq.cpt --TIME_UNIT=d -T2018-05-01T -Es+r2+d6 -Ms5+c0.5 -Mi1+c-0.6 -Mt+c0 -P > tlayer.ps

To convert the time-series seismic_trace.txt (time, amplitude) into a (time, amplitude, time) file that **psevents** can plot
with a variable pen (by plotting densely placed circles), we use **-i** to ensure we read the time-column twice and then use
a *dpu* of 80 pixels per cm (HD movie) and the projection parameters we will use when making the plot, e.g.,::

      gmt psevents seismic_trace.txt -R1984-09-10T03:15/1984-09-10T03:45/-15/15 -JX20cT/10c -Ar80c -i0,1,0 -f2T > seismic_trace_pts.txt

**Note**: If your :term:`PROJ_LENGTH_UNIT` is set to inch then you need to use the equivalent *dpu* of 200 pixels per inch for HD,
or you specify **-Ar**\ 200\ **i**.

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`,
:doc:`psxy`
