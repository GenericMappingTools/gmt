.. index:: ! sac

*****
sac
*****

.. only:: not man

    Plot seismograms in SAC format on maps

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt sac** [ *saclist*\ \|\ *SACfiles* ] |-J|\ *parameters*
|SYN_OPT-R|
[ |SYN_OPT-B| ]
[ |-C|\ [*t0/t1*] ]
[ |-D|\ *dx*\ [/*dy*] ]
[ |-E|\ **a**\|\ **b**\|\ **k**\|\ **d**\|\ **n**\ [*n*]\|\ **u**\ [*n*] ]
[ |-F|\ [**i**][**q**][**r**] ]
[ |-G|\ [**p**\|\ **n**][**+g**\ *fill*][**+z**\ *zero*][**+t**\ *t0/t1*] ]
[ |-M|\ *size*\ [*u*][/*alpha*] ]
[ |-Q| ]
[ |-S|\ [**i**]\ *scale*\ [*unit*] ]
[ |-T|\ [**+t**\ *n*][**+r**\ *reduce_vel*][**+s**\ *shift*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**sac** reads *SACfiles* in SAC format or reads filenames and controlling parameters
from *saclist* [or standard input] and will plot seismograms on a map.

Required Arguments
------------------

*SACfiles*
    SAC files to plot on a map. Only evenly spaced SAC data is supported.

*saclist*
    One ASCII data table file holding a number of data columns. If *saclist* is not given then we read from standard input.
    Parameters are expected to be in the following columns:

        *filename* [*X* *Y* [*pen*]]

    *filename* is the name of SAC file to plot.
    *X* and *Y* are the position of seismograms to plot on a map.
    On linear plots, the default *X* is the begin time of SAC file, which will be adjusted if **-T** option is used,
    the default *Y* is determined by **-E** option.
    On geographic plots, the default *X* and *Y* are station longitude and latitude specified in SAC header.
    The *X* and *Y* given here will override the position determined by command line options.
    *pen*, if given, will override the pen from **-W** option for current SAC file only.

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
    Read and plot seismograms in timewindow between *t0* and *t1* only.
    *t0* and *t1* are relative to a reference time specified by **-T**.
    If **-T** option is not specified, use the reference time (kzdate and kztime) defined in SAC header instead.
    If only **-C** is used, *t0/t1* is determined as *xmin/xmax* from **-R** option.

.. _-D:

**-D**\ *dx*\ [\ **/**\ *dy*]
    Offset seismogram positions by the given mount *dx/dy* [Default is no offset].
    If *dy* is not given it is set equal to *dx*.

.. _-E:

**-Ea**\|\ **b**\|\ **k**\|\ **d**\|\ **n**\ [*n*]\|\ **u**\ [*n*]
    Choose profile type (the type of Y axis).

        **a**: azimuth profile.

        **b**: back-azimuth profile.

        **k**: epicentral distance (in km) profile.

        **d**: epicentral distance (in degree) profile.

        **n**: trace number profile. The *Y* position of first trace is numbered as *n* [Default *n* is 0].

        **u**: user defined profile. The *Y* positions are determined by SAC header variable user\ *n*, default using user0.

.. _-F:

**-F**\ [**i**][**q**][**r**]
    Data preprocess before plotting.

        **i**: integral

        **q**: square

        **r**: remove mean value

    **i|q|r** can repeat multiple times. For example, **-Frii** will convert acceleration to displacement.
    The order of **i|q|r** controls the order of the data processing.

.. _-G:

**-G**\ [**p**\|\ **n**][**+g**\ *fill*][**+z**\ *zero*][**+t**\ *t0/t1*]
    Paint positive or negative portion of traces.
    If only **-G** is used, default to fill the positive portion black.

        **p**\|\ **n** controls the painting of positive **p**\ ortion or **n**\ egative portion.
        Repeat **-G** option to specify fills for positive and negative portions, respectively.

        **+g**\ *fill*: color to fill

        **+t**\ *t0/t1*: paint traces between t0 and t1 only. The reference time of *t0* and *t1* is determined by **-T** option.

        **+z**\ *zero*: define zero line. From *zero* to top is positive portion, from *zero* to bottom is negative portion.

.. _-M:

**-M**\ *size*\ [*u*][*/alpha*]
    Vertical scaling.

        *size*\ [*u*]: scale all traces *size*\ [*u*] on a map. The default unit is PROJ_LENGTH_UNIT.
        The scaling factor is defined as ``yscale = size*(north-south)/(depmax-depmin)/map_height``.

        *size*/*alpha*:

            *alpha* < 0, use the same scaling factor for all traces. The scaling factor will scale the first trace to *size*\ [*u*].

            *alpha* = 0, multiply all traces by *size*. No unit is allowed.

            *alpha* > 0, multiply all traces by ``size*r^alpha``, r is the distance range in km.

.. _-Q:

**-Q**
     Plot traces vertically.

.. _-S:

**-S**\ [**i**]\ *scale*\ [*unit*]
    Sets time scale in seconds per <unit> while plotting on geographic plots.
    Append c, i, or p to indicate cm, inch or points as the unit. Use PROJ_LENGTH_UNIT if *unit* is omitted.
    Use -S\ **i**\ *scale*\ *unit* to give the reciprocal scale, i.e. cm per second or inch per second.

.. _-T:

**-T**\ [**+t**\ *n*][**+r**\ *reduce_vel*][**+s**\ *shift*]
    Time alignment and shift.

        **+t**\ *tmark*: align all trace along time mark. *tmark* are -5(b), -4(e), -3(o), -2(a), 0-9(t0-t9).

        **+r**\ *reduce_vel*: reduce velocity in km/s.

        **+s**\ *shift*: shift all traces by *shift* seconds.

.. _-U:

.. include:: ../../explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**\ *pen*
    Set pen attributes for all traces unless overruled by *pen* specified in *saclist*.
    [Defaults: width = default, color = black, style = solid].

.. _-X:

.. include:: ../../explain_-XY.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-t.rst_

.. include:: ../../explain_help.rst_


Examples
--------

To plot a single seismogram seis.SAC (generated by SAC command ``funcgen seismogram``)
and paint positive portion black and negative portion red::

    gmt sac seis.SAC -JX10c/5c -R9/20/-2/2 -Baf -Fr -Gp+gblack -Gn+gred > single.ps

To plot several seismograms (generated by SAC command ``datagen sub tele *.z``) on a distance profile::

    gmt sac *.z -R200/1600/12/45 -JX15c/5c -Bx200+l'T(s)' -By5+lDegree -BWSen \
        -Ed -M1.5c -W0.5p,red > distance_profile.ps

To plot seismograms (generated by SAC command ``datagen sub tele *.z``) on a geographic map::

    gmt sac *.z -JM15c -R-120/-40/35/65 -Baf -M1i -S300c -K > map.ps
    saclst stlo stla f *.z | gmt plot -J -R -St0.4c -Gblack -i1,2 -O >> map.ps

See Also
--------

:doc:`meca`,
:doc:`polar`,
:doc:`coupe`,
:doc:`velo`,
:doc:`gmt </gmt>`, :doc:`basemap </basemap>`, :doc:`plot </plot>`

References
----------

Refer to `SAC User Manual <http://ds.iris.edu/files/sac-manual/>`_ for more details
on SAC format and SAC header variables.

Authors
-------

Dongdong Tian, School of Earth and Space Sciences,
University of Science and Technology of China, Hefei, Anhui, China
