.. index:: ! hotspotter

**********
hotspotter
**********

.. only:: not man

    hotspotter - Create CVA image from seamount locations

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt hotspotter** [*tables*\ ] |-E|\ *rotfile* |-G|\ *CVAgrid*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-N|\ *upper_age* ]
[ |-S| ] [ |-T| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**hotspotter** reads (longitude, latitude, amplitude, radius, age)
records from *tables* [or standard input] and calculates flowlines
using the specified stage or total reconstruction rotations. These
flowlines are convolved with the shape of the seamount (using a Gaussian
shape given amplitude and radius = 6 sigma) and added up to give a
Cumulative Volcano Amplitude grid (CVA). See option **-:** on how to
read (latitude,longitude,...) files.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. _-E:

**-E**\ *rotfile*
    Give file with rotation parameters. This file must contain one
    record for each rotation; each record must be of the following format:

    *lon lat tstart [tstop] angle* [ *khat a b c d e f g df* ]

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

.. _-G:

**-G**\ *CVAgrid*
    Specify name for output grid file.

.. _-I:

.. include:: ../../explain_-I.rst_

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

Optional Arguments
------------------

.. _-D:

**-D**\ *factor*
    Modify the sampling interval along flowlines. Default [0.5] gives
    approximately 2 points within each grid box. Smaller factors gives
    higher resolutions at the expense of longer processing time.

.. _-N:

**-N**\ *upper_age*
    Set the upper age to assign seamounts whose crustal age is unknown
    (i.e., NaN) [no upper age].

.. _-S:

**-S**
    Normalize the resulting CVA grid to percentages of the CVA maximum.

.. _-T:

**-T**
    Truncate seamount ages exceeding the upper age set with **-N** [no
    truncation].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. |Add_-bi| replace:: [Default is 5 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_../../explain_-V.rst_

.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_-ocols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: explain_geodetic.rst_

Examples
--------

To create a CVA image from the Pacific (x,y,z,r,t) data in the file
seamounts.txt, using the DC85.txt Euler poles, run

   ::

    gmt hotspotter seamounts.txt -EDC85.txt -GCVA.nc -R130/260/-66/60 -I10m -N145 -T -V

This file can then be plotted with :doc:`grdimage </grdimage>`.

Notes
-----

GMT distributes the EarthByte rotation model Global_EarthByte_230-0Ma_GK07_AREPS.rot.
To use an alternate rotation file, create an environmental parameters named
**GPLATES_ROTATIONS** that points to an alternate rotation file.

See Also
--------

:doc:`gmt </gmt>`, :doc:`grdimage </grdimage>`,
:doc:`grdrotater`,
:doc:`grdspotter`,
:doc:`project </project>`,
:doc:`mapproject </mapproject>`,
:doc:`backtracker`,
:doc:`gmtpmodeler`,
:doc:`grdpmodeler`,
:doc:`grdrotater`,
:doc:`originater`

References
----------

Wessel, P., 1999, "Hotspotting" tools released, EOS Trans. AGU, 80 (29), p. 319.

Wessel, P., 2008, Hotspotting: Principles and properties of a plate
tectonic Hough transform, Geochem. Geophys. Geosyst. 9(Q08004):
doi:10.1029/2008GC002058.

