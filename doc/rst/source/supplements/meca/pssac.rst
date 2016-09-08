.. index:: ! pssac

*****
pssac
*****

.. only:: not man

    pssac - Plot seismograms in SAC format on maps

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_


**psvelo** *<saclist>\ |\ <sacfiles>* |-J|\ *parameters*
|SYN_OPT-R|
[ |SYN_OPT-B| ]
[ |-C|\ [*t0/t1*] ]
[ |-D|\ *dx*\ [\ **/**\ *dy*] ]
[ |-E|\ **a**\|\ **b**\|\ **k**\|\ **d**\|\ **n**\ [*<n>*]\|\ **u**\ [*<n>*] ]
[ |-F|\ [*i*][*q*][*r*] ]
[ |-G|\ [**p**\|\ **n**][**+g**\ *fill*][**+z**\ *zero*][**+t**\ *t0/t1*] ]
[ |-K| ]
[ |-M|\ *size*\ [*u*][**/**\ *alpha*] ]
[ |-O| ] [ |-P| ]
[ |-T|\ [**+t**\ *n*][**+r**\ *reduce_vel*][**+s**\ *shift*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-h| ]
[ **-m**\ *sec_per_inch* ]
[ |SYN_OPT-t| ]
[ **-v** ]

|No-spaces|

Description
-----------

**pssac** reads data values from *sac files* [or standard input] and
generates PostScript ...

Required Arguments
------------------

	*sacfiles* are the name of SAC files to plot on maps. Only evenly spaced SAC data is supported. *saclist* is
	an ASCII file (or stdin) which contains the name of SAC files to plot and controlling parameters. Each record
	has 1, 3 or 4 items:  *filename*, [*X* *Y*], [*pen*].
           
        *filename* is the name of SAC file to plot.

        *X* and *Y* are the location of the trace on the plot.
        On linear plots, the default *X* is the begin time of SAC file, which will be adjusted if **-T** option is used,
        the default *Y* will be adjusted if **-E** option is used.
        On geographic plots, the default *X* and *Y* are determinted by stlo and stla from SAC header.
        The *X* and *Y* given here will override the position determined by command line options.

        *pen*, is given, it will override the pen from **-W** option for current SAC file only.

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-J.rst_

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_


Optional Arguments
------------------

.. _-B:

.. include:: ../../explain_-B.rst_

.. _-C:

**-C**\ [*t0/t1*]
    Cut data in timewindow between *t0* and *t1*. These are relative to a reference time specified by **-T**.
    If **-T** option is not specified, use reference time in SAC header instead.
    If only **-C** is used, *t0/t1* is determined as *xmin/xmax* from **-R** option.

.. _-D:

**-D**\ *dx*\ [\ **/**\ *dy*]
    Offset seismogram locations by the given mount *dx/dy* [Default is no offset].
    If *dy* is not given it is set equal to *dx*.

.. _-E:


**-Ea**\|\ **b**\|\ **k**\|\ **d**\|\ **n**\ [*\<n\>*]\|\ **u**\ [*\<n\>*]

    Determine profile type (the type of Y axis).

        *a*: azimuth profile
        *b*: back-azimuth profile
        *k*: epicentral distance (in km) profile
        *d*: epicentral distance (in degree) profile
        *n*: traces are numbered from *<n>* to *<n>+N* in y-axis, default value of *<n>* is 0
        *u*: Y location is determined from SAC header user\ *<n>*, default using user0.

.. _-F:

**-F**\ [*i*][*q*][*r*]

    Data preprocess before plotting.

        *i*: integral
        *q*: square
        *r*: remove mean value

    *i|q|r* can repeat mutiple times. **-Frii** will convert accerate to displacement.
    The order of *i|q|r* controls the order of the data processing.

.. _-G:

**-G**\ [**p**\|\ **n**][**+g**\ *fill*][**+z**\ *zero*][**+t**\ *t0/t1*]

    Paint positive or negative portion of traces.
    If only **-G** is used, default to fill the positive portion black.

        **p**\|\ **n** controls the painting of postive portion or negative portion. Repeat **-G** option to specify fills for positive and negative portions, respectively.

        **+g**\ *fill*: color to fill

        **+t**\ *t0/t1*: paint traces between t0 and t1 only. The reference time of t0 and t1 is determined by **-T** option.

        **+z**\ *zero*: define zero line. From *zero* to top is positive portion, from *zero* to bottom is negative portion.

.. _-K:

.. include:: ../../explain_-K.rst_

.. _-M:

**-M**\ *size*\ [*u*][*/alpha*]
    Vertical scaling.

        *size*\ [*u*]: each trace will scaled to *size*\ [*u*]. The default unit is PROJ_LENGTH_UNIT. The scale factor is defined as ``yscale = size*(north-south)/(depmax-depmin)/map_height``
        *size*/*alpha*:

            *alpha* < 0, use the same scale factor for all trace. The scale factor scale the first trace to *size*\ [*u*]
            *alpha* = 0, yscale = size, no unit is allowed.
            *alpha* > 0, yscale = size*r^alpha, r is the distance range in km.

.. _-O:

.. include:: ../../explain_-O.rst_

.. _-P:

.. include:: ../../explain_-P.rst_

.. _-T:

**-T**\ [**+t**\ *n*][**+r**\ *reduce_vel*][**+s**\ *shift*]
    Time alignment and shift.

        **+t**\ *tmark*: align all trace along time mark. <tmark> are -5(b), -3(o), -2(a), 0-9(t0-t9).
        **+r**\ *reduce_vel*: reduce velocity in km/s.
        **+s**\ *shift*: shift all traces by <shift> seconds

.. _-U:

.. include:: ../../explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**\ *pen*
    Set pen attributes for all lines and the outline of symbols
    [Defaults: width = default, color = black, style = solid]. This
    setting applies to **-C**, **-L**, **-T**, **-p**, **-t**, and
    **-z**, unless overruled by options to those arguments.

**-m**\ *sec_per_inch*
    Time scaling while plotting on maps.

**-v**
     Plot traces vertically.
