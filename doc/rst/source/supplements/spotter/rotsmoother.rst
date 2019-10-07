.. index:: ! rotsmoother

***********
rotsmoother
***********

.. only:: not man

    rotsmoother - Get mean rotations and covariance matrices from set of finite rotations

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt rotsmoother** [ *rottable* ]
[ |-A| ]
[ |-C| ]
[ |-N| ]
[ |-S| ]
[ |-T|\ *ages* ]
[ |SYN_OPT-V| ]
[ |-W| ]
[ |-Z| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**rotsmoother** reads a table of total reconstructions and computes mean
rotations (and optionally covariance matrices) for sub-groups of rotations
based on rotation age.

Required Arguments
------------------

*rottable*
    Name of a rotation table containing (lon, lat, time, angle, [weight]) values.

Optional Arguments
------------------

.. _-A:

**-A**
    Use opening angles as a proxy for time.  Suitable when no time can be assigned to the
    rotations.  In this case, input is expected to contain *lon lat angle* [*weight*] records
    and **-T** settings refer to angles instead of time.
    [Default expects *lon lat time angle* [*weight*] and **-T** refers to time].

.. _-C:

**-C**
    Compute covariance matrix for each mean rotation.  This is done by converting each
    finite rotation to a quaternion, determining the mean quaternion (rotation) and
    the consider all rotations as perturbation to the mean rotation.  From these
    perturbations we determine the covariance matrix.
   

.. _-N:

**-N**
    Ensure all poles are in northern hemisphere [Default ensures positive opening angles].

.. _-S:

**-S**
    Ensure all poles are in southern hemisphere [Default ensures positive opening angles].

.. _-T:

**-T**\ *ages*
    Sets the desired groups of ages.  For a single time append
    the desired time.  For an equidistant range of reconstruction ages
    give **-T**\ *start*\ /\ *stop*\ /\ *inc* or **-T**\ *start*\ /\ *stop*\ /\ *npoints*\ **+n**.
    For an non-equidistant set of reconstruction ages please pass them
    via the first column in a file, e.g., **-T**\ *agefile*.  The ages indicate
    read or generated becomes the bin-boundaries and we output the average time of
    all rotations inside each bin.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**
    Expect weights in last column for a weighted mean rotation [no weights].

.. _-Z:

**-Z**
    Report negative opening angles [positive].

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: ../../explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_
.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_-ocols.rst_
.. include:: ../../explain_-s.rst_
.. include:: ../../explain_colon.rst_
.. include:: ../../explain_-n.rst_
.. include:: ../../explain_help.rst_

.. include:: explain_geodetic.rst_

Examples
--------

To smooth rotation groups in increments of 3 Myr and ensure northern hemisphere poles, try

   ::

    gmt rotsmoother rotations.txt -N -T3/3/30 -V > rot_means.txt


See Also
--------

:doc:`backtracker`,
:doc:`gmtpmodeler`,
:doc:`grdpmodeler`,
:doc:`grdspotter`,
:doc:`hotspotter`,
:doc:`originater`,
:doc:`rotconverter`
