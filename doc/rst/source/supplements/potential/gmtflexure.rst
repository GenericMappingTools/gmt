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
    assumed to equal the load density), and water.  If *ri@ differs from
    *rl* then an approximate solution will be found.
**-E**\ *Te*\ [**u**]\|\ *D*\|\ *file*
    Sets the elastic plate thickness (in meter); append **k** for km.
    If the elastic thickness exceeds 1e10 it will be interpreted as
    the flexural rigidity **D** (by default **D** is computed from *Te*, Young's
    modulus, and Poisson's ratio; see **-C** to change these values).
    Alternatively, supply a file with variable plate thicknesses or rigidities.

Optional Arguments
------------------

**-A**\ [**l**\|\ **r**][/*args*]
    Sets the boundary conditions at the **l**\ eft and **r**\ ight boundary.

**-Cp**\ *poisson*
    Change the current value of Poisson's ratio [0.25].

**-Cy**\ *Young*
    Change the current value of Young's modulus [7.0e10 N/m^2].

**-F**\ *force*]
    Set a constant horizontal in-plane force [0] 

**-Q**\ *args*]
    Sets the input load specifications. 

.. include:: ../../explain_fft.rst_

**-S**
    Compute the curvature along with the deflections and report them as the
    third output column [none].

**-T**\ *wfile*
    Supply a file with pre-existing deformations [undeformed surface].

**-W**\ *wd*
    Set reference depth to the undeformed flexed surface [0].

**-Z**\ *zm*
    Set the average water depth [0].

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

Plate Flexure
-------------

We solve for plate flexure using a finite difference approach.

Examples
--------

To compute elastic plate flexure from the load *topo.txt*,
for a 10 km thick plate with typical densities, try

   ::

    gmt flexure -Qttopo.txt -E10k -D2700/3300/1035 > flex.txt

References
----------


See Also
--------

:doc:`gmt </gmt>`, :doc:`grdfft </grdfft>`, :doc:`gravfft </supplements/potential/gravfft>`
:doc:`grdflexure </supplements/potential/grdflexure>`:doc:`grdmath </grdmath>`, :doc:`grdproject </grdproject>`
