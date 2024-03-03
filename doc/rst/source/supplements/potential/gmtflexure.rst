.. index:: ! gmtflexure
.. include:: ../module_supplements_purpose.rst_

*******
flexure
*******

|gmtflexure_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt flexure** |-D|\ *rm*/*rl*\ [/*ri*]\ /*rw*
|-E|\ *Te*\ [**k**]\|\ *D*\|\ *file*
|-Q|\ *args*
[ |-A|\ [**l**\|\ **r**]\ *bc*\ [/*args*] ]
[ |-C|\ **p**\|\ **y**\ *value* ]
[ |-F|\ *force* ]
[ |-L| ]
[ |-M|\ [**h**][**v**] ]
[ |-S| ]
[ |-T|\ *wfile*]
[ |SYN_OPT-V| ]
[ |-W|\ *wd*\ [**k**]]
[ |-Z|\ *zm*\ [**k**]]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**flexure** computes the flexural response to 2-D loads using a range
of user-selectable options, such as boundary conditions, pre-existing
deformations, variable rigidity and restoring force, and more.  The solutions
are obtained using finite difference approximations to the differential
equations [*Bodine*,\ 1980].

Required Arguments
------------------

.. _-D:

**-D**\ *rm*/*rl*\ [/*ri*]\ /*rw*
    Sets density for mantle, load, infill (optionally, otherwise it is
    assumed to equal the load density), and water.  If *ri* is not given
    then it defaults to *rl*.

.. _-E:

**-E**\ *Te*\ [**k**]\|\ *D*\|\ *file*
    Sets the elastic plate thickness (in meter); append **k** for km.
    If the elastic thickness exceeds 1e10 it will be interpreted as
    a flexural rigidity *D* instead (by default *D* is computed from *Te*, Young's
    modulus, and Poisson's ratio; see |-C| to change these values).
    Alternatively, supply a *file* with variable plate thicknesses or rigidities.
    The file must be co-registered with any file given via |-Q|.

.. _-Q:

**-Qn**\|\ **q**\|\ **t**\ [*args*]
    Sets the vertical load specification. Choose among these three options:
    **-Qn** means there is no input load file and that any deformation is
    simply driven by the boundary conditions set via |-A|.  If no rigidity or
    elastic thickness file is given via |-E| then you must also append arguments
    to create the locations used for the calculations; for details on array creation,
    see `Generate 1-D Array`_.
    **-Qq**\ [*loadfile*] is a file (or standard input if not given) with (x,load in Pa)
    for all equidistant data locations.  Finally, **-Qt**\ [*topofile*] is a file
    (or standard input if not given) with (x,load in m or km, positive up); see |-M| for
    topography unit used [m].

Optional Arguments
------------------

.. _-A:

**-A**\ [**l**\|\ **r**]\ *bc*\ [/*args*]
    Sets the boundary conditions at the **l**\ eft and **r**\ ight boundary.
    The *bc* can be one of four codes: 0 selects the infinity condition, were
    both the deflection and its slope are set to zero.  1 selects the periodic
    condition where both the first and third derivatives of the deflection are set to zero.
    2 selects the clamped condition where *args* (if given) sets the deflection
    value [0] (and its first derivative is set to zero), while 3 selects the free condition
    where *args* is given as *moment*/*force* which specify the end bending
    moment and vertical shear force [0/0].  Use SI units for any optional arguments.

.. _-C:

**-C**\ **p**\|\ **y**\ *value*
    Append **p** or **y** to change the current value of Poisson's ratio [0.25]
    or Young's modulus [7.0e10 N/m\ :sup:`2`], respectively.

.. _-F:

**-F**\ *force*
    Set a constant horizontal in-plane force, in Pa m [0].

.. _-L:

**-L**
    Use a variable restoring force that depends on sign of the flexure [constant].

.. _-M:

**-M**\ [**h**][**v**]
    Optionally append one or both of **h** and **v**: Use **h** to indicated that all
    horizontal distances are in km [meters] and **v** to
    indicate that all vertical deflections are in km [meters].

.. _-S:

**-S**
    Compute the curvature along with the deflections and report them via the
    third output column [none].

.. _-T:

**-T**\ *wfile*
    Supply a file with pre-existing deformations [undeformed surface].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *wd*\ [**k**]
    Specify water depth in m; append **k** for km.  Must be positive [0].
    Any subaerial topography (i.e., amplitudes in the input relief that
    exceeds this depth) will be scaled via the densities set in |-D|
    to compensate for the larger density contrast with air.

.. _-Z:

**-Z**\ *zm*\ [**k**]
    Undeformed plate flexure means *z = 0*. Specify the distance between the
    observation level [*z = 0*] and the undeformed flexed surface in m; append **k** for km.
    Must be positive [0]. We subtract this value from the flexed surface before output.
    Thus, if the observation level is at sealevel and you are looking a seafloor deformation
    in 5 km of water, use -Z5k and the undeformed surface will have *z = -5000* on output.

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-bi.rst_

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-ocols.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_array.rst_

.. include:: explain_geometry.rst_

Note on Units
-------------

The |-M| option controls the units used in all input and output files.
However, this option does *not* control values given on the command line
to the |-E|, |-W|, and |-Z| options.  These are assumed to be in
meters unless an optional **k** for km is appended.

Plate Flexure Notes
-------------------

We solve for plate flexure using a finite difference approach. This method can
accommodate situations such as variable rigidity, restoring force that depends
on the deflection being positive or negative, pre-existing deformation, and
different boundary conditions.

Examples
--------

To compute elastic plate flexure from the topography load in *topo.txt*,
for a 10 km thick plate with typical densities, try

::

  gmt flexure -Qttopo.txt -E10k -D2700/3300/1035 > flex.txt

References
----------

Bodine, J. H., 1980, *Numerical computation of plate flexure in marine geophysics*,
Tech. Rep. CU-1-80, Columbia University.

See Also
--------

:doc:`gmt </gmt>`, :doc:`gravfft </supplements/potential/gravfft>`,
:doc:`grdflexure </supplements/potential/grdflexure>`, :doc:`grdmath </grdmath>`
