.. index:: ! mgd77magref

***********
mgd77magref
***********

.. only:: not man

    mgd77magref - Evaluate the IGRF or CM4 magnetic field models

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77magref** [ *inputfile* ]
[ |-A|\ [**+a**\ *alt*\ **+t**\ *date*\ **+y**] ]
[ |-C|\ *cm4file* ]
[ |-D|\ *Dstfile* ]
[ |-E|\ *f107file* ]
[ |-F|\ *flags* ]
[ |-G| ]
[ |-S|\ **c**\ \|\ **l**\ *low/high* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mgd77magref** will evaluate the IGRF or the CM4 geomagnetic models at
the specified locations and times. 

Required Arguments
------------------

None.

Optional Arguments
------------------

*inputfile*
    Contains the moments in space-time where we want to evaluate the
    magnetic reference field. The first two columns must contain
    longitude and latitude (however, see **-:** for latitude and
    longitude instead). Normally, the third and fourth columns must
    contain altitude (in km) and time, respectively, but if one or both
    of these are constant for all records they can be supplied via the
    **-A** option instead and are thus not expected in the input file.
    If no input file is given we read *stdin*. |br|
    A note about the CM4
    validity domain. The core field of CM4 is valid from 1960-2002.5 but
    the ionospheric and magnetospheric fields are computed after the
    *Dst* and *F10.7* coefficient files. We extended here those
    coefficient files up to 2006, which means that one can compute
    external contributions up until 2006 but the Secular Variation will
    be biased (non reliable). New indices files may be retrieved from
    from:ftp://ftp.ngdc.noaa.gov/STP/GEOMAGNETIC_DATA/INDICES/DST/ (the
    *Dst* coefficients) and
    http://umbra.nascom.nasa.gov/sdb/yohkoh/ys_dbase/indices_flux_raw/Penticton_Absolute/monthly/MONTHPLT.ABS 
    (The *F10.7* index file is a MONTHPLT.ABS). NOTE: since the *Dst* files
    in the .../DST/ directory are still only up to 2006, for GMT4.5.3 and after
    we extended the *Dst* until August 2009 by reformatting the data in the
    preliminary file Est_Ist_index_0_mean.pli, which is at
    ftp://ftp.ngdc.noaa.gov/STP/GEOMAGNETIC_DATA/INDICES/EST_IST/. But since
    this site is now also outdated, we now get the DST indices from 
    http://wdc.kugi.kyoto-u.ac.jp/dstae/index.html However, for the most recent
    dates those indices are "Quick Look" (the best are the "Definitive" type).
    Because the *F10.7* from the MONTHPLT.ABS file mentioned above are apparently
    no being updated, we found another place where they are, which is:
    ftp://ftp.ngdc.noaa.gov/STP/space-weather/solar-data/solar-features/
    solar-radio/noontime-flux/penticton/penticton_absolute/listings/
    listing_drao_noontime-flux-absolute_monthly.txt 

.. _-A:

**-A**\ [**+a**\ *alt*\ **+t**\ *date*\ **+y**]
    Adjusts how the input record is interpreted. Append **+a** to set a
    fixed *altitude* (in km) that should apply to all data records
    [Default expects *altitude* to be in the 3rd column of all records].
    Append **+t** to set a fixed *time* that should apply to all data
    records [Default expects *time* to be in the 4th column of all
    records]. Finally, append **+y** to indicate that all times are
    specified as decimal years [Default is ISO *date*\ T\ *colck* format, see
    :ref:`TIME_EPOCH <TIME_EPOCH>`].

.. _-C:

**-C**\ *cm4file*
    Specify an alternate CM4 coefficient file [umdl.CM4].

.. _-D:

**-D**\ *Dstfile*
    Specify an alternate file with hourly means of the Dst index for CM4
    [Dst_all.wdc]. Alternatively, simply specify a single index to
    apply for all records.

.. _-E:

**-E**\ *f107file*
    Specify an alternate file with monthly means of absolute F10.7 solar
    radio flux for CM4 [F107_mon.plt]. Alternatively, simply specify a
    single flux to apply for all records.

.. _-F:

**-F**\ *flags*
    Selects output items; *flags* is a string made up of one or more of
    these characters:

    **r** means output all input columns before adding the items below

    **t** means list total field (nT).

    **h** means list horizontal field (nT).

    **x** means list X component (nT, positive north).

    **y** means list Y component (nT, positive east).

    **z** means list Z component (nT, positive down).

    **d** means list declination (deg, clockwise from north).

    **i** means list inclination (deg, positive down).

    Append one or more number to indicate the requested field contribution(s):

    **0** means IGRF field (no combinations allowed)

    **1** means CM4 Core field

    **2** means CM4 Lithospheric field

    **3** means CM4 Primary Magnetospheric field

    **4** means CM4 Induced Magnetospheric field

    **5** means CM4 Primary ionospheric field

    **6** means CM4 Induced ionospheric field

    **7** means CM4 Toroidal field

    **9** means Core field from IGRF and other contributions from CM4.
    DO NOT USE BOTH 0 AND 9.

    Appending several numbers (1-7) will add up the different
    contributions. For example **-Ft**/**12** computes the total field
    due to Core and Lithospheric sources. Two special cases are allowed,
    which mix which Core field from IGRF and other sources from CM4.
    **-Ft**/**934** computes Core field due to IGRF plus terms 3 and 4
    from CM4 (but you can add others). **-Ft**/**934** the same as above
    but output the field components. The data is written out in the
    order they appear in *flags* [Default is **-Frthxyzdi**/**1**].

.. _-G:

**-G**
    Specifies that coordinates are geocentric [geodetic].

.. _-L:

**-L**
    Computes J field vectors from certain external sources.

    **r** means output all input columns before adding the items below (all in Ampers/m).

    **t** means list magnitude field.

    **x** means list X component.

    **y** means list Y component.

    **z** means list Z or current function Psi.

    Append a number to indicate the requested J contribution:

    **1** means Induced Magnetospheric field.

    **2** means Primary ionospheric field.

    **3** means Induced ionospheric field.

    **4** means Poloidal field.

.. _-S:

**-Sc**\ *low/high*
    Limits the wavelengths of the core field contribution to the band
    indicated by the low and high spherical harmonic order [1/13].

**-Sl**\ *low/high*
    Limits the wavelengths of the lithosphere field contribution to the
    band indicated by the low and high spherical harmonic order [14/65].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_
    
.. |Add_-bi| replace:: [Default is 4 input columns unless **-A** is used]. 
.. include:: ../../explain_-bi.rst_

.. |Add_-bo| replace:: [Default is reflected by **-F**]. 
.. include:: ../../explain_-bo.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

Time Settings
-------------

If binary input files are used then absolute time are stored as time
relative to the selected epoch. However, since the epoch used is not
stored in the data files there can be problems decoding the correct
time. The mgd77 supplement uses the Unix time system as its default;
thus you should make sure that binary data files with time uses the same
system (see the GMT default TIME_SYSTEM).

Examples
--------

To get the CM4 Total field, Declination and Inclination due to all but
lithospheric and toroidal field at a one point location and decimal time
2000.0, try

   ::

    echo -28 38 0 2000.0 | gmt mgd77magref -A+y -Ftdi/13456

To do the same as above but at noon (Universal Time) of first May 2001, try

   ::

    echo -28 38 0 2001-05-01T12:00:00 | gmt mgd77magref -Ftdi/13456

See Also
--------

:doc:`gmt </gmt>`, :doc:`mgd77info`,
:doc:`mgd77list`, :doc:`mgd77manage`,
:doc:`mgd77track`, :doc:`gmt.conf </gmt.conf>`

References
----------

Comprehensive Modeling of the Geomagnetic Field, see
`http://denali.gsfc.nasa.gov/cm/ <http://denali.gsfc.nasa.gov/cm/>`_

The International Geomagnetic Reference Field (IGRF), see
`http://www.iugg.org/IAGA/iaga\_pages/pubs\_prods/igrf.htm <http://www.iugg.org/IAGA/iaga_pages/pubs_prods/igrf.htm>`_
