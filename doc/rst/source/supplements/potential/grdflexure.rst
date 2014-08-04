.. index:: ! grdflexure

**********
grdflexure
**********

.. only:: not man

    grdflexure - Compute flexural deformation of 3-D surfaces for various rheologies

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**grdflexure** *topogrd* **-D**\ *rm*/*rl*\ [/*ri*]\ /*rw* **-E**\ *Te*\ [**u**] **-G**\ *outgrid*
[ **-Cp**\ *poisson* ] [ **-Cy**\ *Young* ] [ **-F**\ *nu_a*\ [/*h_a*/*nu_m*] ]
**-N**\ [**f**\ \|\ **q**\ \|\ **s**\ \|\ *nx*/*ny*][**+a**\ \|\ **d**\ \|\ **h** \|\ **l**][**+e**\ \|\ **n**\ \|\ **m**][**+t**\ *width*][**+w**\ [*suffix*]][\ **+z**\ [**p**]]
[ **-S**\ *beta* ] [ **-T**\ [**l**]\ *t0*\ [/*t1*/*dt*] ]
[ |SYN_OPT-V| ]
[ **-W**\ *wd*] [ **-Z**\ *zm*]
[ **-fg** ]

|No-spaces|

Description
-----------

**grdflexure** computes the flexural response to loads using a range
of user-selectable rheologies.

Required Arguments
------------------

*topogrd*
    2-D binary grid file with the topography of the load (in meters);
    See GRID FILE FORMATS below).
    If **-T** is used, *topogrd* may be a filename template with a
    floating point format (C syntax) and a different load file name
    will be set and loaded for each time step.

**-D**\ *rm*/*rl*\ [/*ri*]\ /*rw*
    Sets density for mantle, load, infill (optionally, otherwise it is
    assumed to equal the load density), and water.  If *ri* differs from
    *rl* then an approximate solution will be found.  If *ri* is not given
    then it defaults to *rl*.

**-E**\ *Te*
    Sets the elastic plate thickness (in meter); append **k** for km.
    If the elastic thickness exceeds 1e10 it will be interpreted as
    the flexural rigidity D (by default D is computed from *Re*, Young's
    modulus, and Poisson's ratio; see **-C** to change these values).
**-G**\ *outfile*
    Specify the name of the output grid file; see GRID FILE FORMATS below).
    If **-T** is set then *outfile* must be a filename template that contains
    a floating point format (C syntax) and we use the corresponding time
    (in units specified in **-T**) to generate the file name.

Optional Arguments
------------------

**-Cp**\ *poisson*
    Change the current value of Poisson's ratio [0.25].

**-Cy**\ *Young*
    Change the current value of Young's modulus [7.0e10 N/m^2].

**-F**\ *nu_a*[/*h_a*/*nu_m*]
    Specify a firmoviscous model in conjuncton with an elastic plate
    thickness specified via **-E**.  Just give one viscosity (*nu_a*)
    for an elastic plate over a viscous half-space, or also append
    the thickness of the asthenosphere, with the first viscosity being
    asthenospheric and the latter that of the lower mantle 

.. include:: ../../explain_fft.rst_

**-S**\ *beta*
    Specify a starved moat fraction in the 0-1 range, where 1 means the moat is fully
    filled with material of density *ri* while 0 means it is only filled with
    material of density *rw* (i.e., just water) [1].

**-T**\ [**l**]\ *start*/*stop*/*dt*
    Specify *start*, *stop*, and time increment (*dt*) for sequence of calculations
    [Default is one step, with no time dependency].  For a single specific time, just
    give *start*. The unit is years; append **k** for kyr and **M** for Myr.
    For a logarithmic time scale, use **-Tl** and specify *n* steps instead of *dt*.
    We then write a separate grid file for each time step.

**-W**\ *wd*
    Set reference depth to the undeformed flexed surface in m [0].  Append **k** to indicate
    km.

**-Z**\ *zm*
    Specify reference depth to flexed surface (e.g., Moho) in m; append **k** for km.
    Must be positive. [0].

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

**-fg**
   Geographic grids (dimensions of longitude, latitude) will be converted to
   meters via a "Flat Earth" approximation using the current ellipsoid parameters.

.. include:: ../../explain_help.rst_

.. include:: ../../explain_grd_inout.rst_

Grid Distance Units
-------------------

If the grid does not have meter as the horizontal unit, append **+u**\ *unit*
to the input file name to convert from the specified unit to meter.  If your
grid is geographic, convert distances to meters by supplying **-fg** instead.

Considerations
--------------

netCDF COARDS grids will automatically be recognized as geographic. For
other grids geographical grids were you want to convert degrees into
meters, select **-fg**. If the data are close to either pole, you should
consider projecting the grid file onto a rectangular coordinate system
using :doc:`grdproject </grdproject>`.

Plate Flexure Notes
-------------------

The FFT solution to plate flexure requires the infill density to equal
the load density.  This is typically only true directly beneath the load; beyond the load
the infill tends to be lower-density sediments or even water (or air).  Wessel [2001]
proposed an approximation that allows for the specification of an infill density
different from the load density while still allowing for an FFT solution. Basically,
the plate flexure is solved for using the infill density as the effective load density but
the amplitudes are adjusted by the factor *A* = sqrt ((rm - ri)/(rm - rl)), which is
the theoretical difference in amplitude due to a point load using the two different
load densities.  The approximation is very good but breaks down for large
loads on weak plates, a fairy uncommon situation.

Examples
--------

To compute elastic plate flexure from the load *topo.nc*,
for a 10 km thick plate with typical densities, try

   ::

    gmt grdflexure topo.nc -Gflex.nc -E10k -D2700/3300/1035

References
----------


See Also
--------

:doc:`gmt </gmt>`, :doc:`gmtflexure </supplements/potential/gmtflexure>`
:doc:`grdfft </grdfft>`, :doc:`gravfft </supplements/potential/gravfft>`
:doc:`grdmath </grdmath>`, :doc:`grdproject </grdproject>`
