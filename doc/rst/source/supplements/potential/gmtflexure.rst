.. index:: ! gmtflexure

**********
gmtflexure
**********

.. only:: not man

    gmtflexure - Compute flexural deformation of 2-D loads

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmtflexure** **-D**\ *rm/rl/ri/rw* **-E**\ *Te*\ [**u**]\|\ *D*\|\ *file*
[ **-A**\ [**l**\|\ **r**][/*args*] ] [ **-Cp**\ *poisson* ] [ **-Cy**\ *Young* ] [ **-F**\ *force* ]
[ **-Q**\ *args*\ ][ **-S** ] [ **-T**\ *wfile*\ ]
[ |SYN_OPT-V| ]
[ **-W**\ *wd*] [ **-Z**\ *zm*]
[ **-fg** ]

|No-spaces|

Description
-----------

**gmtflexure** computes the flexural response to 2-D loads using a range
of user-selectable options, such as boundary conditions, pre-existing
deformations, variable rigidity and restoring force, and more.

Required Arguments
------------------

**-D**\ *rm*/*rl*/\ [*ri* \]/*rw*
    Sets density for mantle, load, infill (optionally, otherwise it is
    assumed to equal the load density), and water.  If *ri* differs from
    *rl* then an approximate solution will be found.

**-E**\ *Te*\ [**u**]\|\ *D*\|\ *file*
    Sets the elastic plate thickness (in meter); append **k** for km.
    If the elastic thickness exceeds 1e10 it will be interpreted as
    a flexural rigidity **D** instead (by default **D** is computed from *Te*, Young's
    modulus, and Poisson's ratio; see **-C** to change these values).
    Alternatively, supply a *file* with variable plate thicknesses or rigidities.
    The file must be co-registered with any file given via **-Q**.

Optional Arguments
------------------

**-A**\ [**l**\|\ **r**]*bc*[/*args*]
    Sets the boundary conditions at the **l**\ eft and **r**\ ight boundary.
    The *bc* can be one of four codes: 0 means the infinity condition, were
    both the deflection and its slope is set to zero.  1 means the periodic
    condition where both the first and third derivatives are set to zero.
    2 means the clamped condition where *args* (if given) sets the deflection
    value [0] (and the first derivative is set to zero).  3 means the free condition
    where *args* is given as *moment*/*force* that specifies the end bending
    moment and vertical shear force [0/0].  Use SI units for any optional arguments.

**-Cp**\ *poisson*
    Change the current value of Poisson's ratio [0.25].

**-Cy**\ *Young*
    Change the current value of Young's modulus [7.0e10 N/m^2].

**-F**\ *force*]
    Set a constant horizontal in-plane force, in Pa m [0] 

**-Q**\ *args*]
    Sets the input load specifications. Choose among these three options:
    **-Qn** means there is no input load file and that any deformation is
    simply driven by the boundary conditions set via **-A**.  If no rigidity or
    elastic thickness file is given via **-E** then you must also append *min*/*max*/*inc*
    **Qn** to initiate the locations used for the calculations.  Append **+** to *inc*
    to indicate the number of points instead.
    **-Qq**\ [*loadfile*] is a file (or stdin if not given) with (x,load in Pa)
    for all equidistant data locations.  Finally, **-Qt**\ [*topofile*] is a file
    (or stdin if not given) with (x,load in m or km, positive up); see **-M** for
    topography unit used [m].

.. include:: ../../explain_fft.rst_

**-S**
    Compute the curvature along with the deflections and report them via the
    third output column [none].

**-T**\ *wfile*
    Supply a file with pre-existing deformations [undeformed surface].

**-W**\ *wd*
    Specify water depth in m; append k for km.  Must be positive [0].
    Any subarial topography will be scaled via the densities set in **-D**
    to compensate for the larger density contrast with air.

**-Z**\ *zm*
    Specify reference depth to flexed surface in m; append k for km.  Must be positive [0].
    We add this value to the flexed surface before output.

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

Note on Units
-------------

The **-M** option controls the units used in all input and output files.
However, this option does *not* control values given on the command line
to the **-E**, **-W**, and **-Z** options.  These are assumed to be in
meters unless an optional *k* for km is appended.

Plate Flexure
-------------

We solve for plate flexure using a finite difference approach. This method can
accommodate situations such as variable rigidity, restoring force that depends
on the deflection being positive or negative, and different boundary conditions

Examples
--------

To compute elastic plate flexure from the topography load in *topo.txt*,
for a 10 km thick plate with typical densities, try

   ::

    gmt flexure -Qttopo.txt -E10k -D2700/3300/1035 > flex.txt

References
----------


See Also
--------

:doc:`gmt </gmt>`, :doc:`grdfft </grdfft>`, :doc:`gravfft </supplements/potential/gravfft>`
:doc:`grdflexure </supplements/potential/grdflexure>`:doc:`grdmath </grdmath>`, :doc:`grdproject </grdproject>`
