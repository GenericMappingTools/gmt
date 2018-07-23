.. index:: ! grdflexure

**********
grdflexure
**********

.. only:: not man

    grdflexure - Compute flexural deformation of 3-D surfaces for various rheologies

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt grdflexure** *topogrd* |-D|\ *rm*/*rl*\ [/*ri*]\ /*rw* |-E|\ *Te*\ [**u**] |-G|\ *outgrid*
[ |-A|\ *Nx*/*Ny*/*Nxy* ]
[ |-C|\ **p**\ *poisson* ] [ |-C|\ **y**\ *Young* ]
[ |-F|\ *nu_a*\ [/*h_a*/*nu_m*] ]
[ |-L|\ *list* ]
[ |-N|\ [**f**\ \|\ **q**\ \|\ **s**\ \|\ *nx*/*ny*][**+a**\ \|\ **d**\ \|\ **h**\ \|\ **l**][**+e**\ \|\ **n**\ \|\ **m**][**+t**\ *width*][**+w**\ [*suffix*]][\ **+z**\ [**p**]]
[ |-S|\ *beta* ] [ **-T**\ *t0*\ [**u**]\ [/*t1*\ [**u**]/*dt*\ [**u**]\ \|\ *file*]\  \|\ *n*]\ [**+l**] ]
[ |SYN_OPT-V| ]
[ |-W|\ *wd*]
[ |-Z|\ *zm*]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdflexure** computes the flexural response to loads using a range
of user-selectable rheologies.  User may select from elastic, viscoelastic,
or firmoviscous (with one or two viscous layers).  Temporal evolution can
also be modeled by providing incremental load grids and specifying a
range of model output times.

Required Arguments
------------------

*topogrd*
    2-D binary grid file with the topography of the load (in meters);
    See GRID FILE FORMATS below.
    If **-T** is used, *topogrd* may be a filename template with a
    floating point format (C syntax) and a different load file name
    will be set and loaded for each time step.  The load times thus
    coincide with the times given via **-T** (but not all times need
    to have a corresponding file).  Alternatively, give *topogrd* as
    =\ *flist*, where *flist* is an ASCII table with one *topogrd* filename
    and load time per record.  These load times can be different from
    the evaluation times given via **-T**.  For load time format, see
    **-T**.

.. _-D:

**-D**\ *rm*/*rl*\ [/*ri*]\ /*rw*
    Sets density for mantle, load, infill (optional, otherwise it is
    assumed to equal the load density), and water or air.  If *ri* differs from
    *rl* then an approximate solution will be found.  If *ri* is not given
    then it defaults to *rl*.

.. _-E:

**-E**\ *Te*
    Sets the elastic plate thickness (in meter); append **k** for km.
    If the elastic thickness exceeds 1e10 it will be interpreted as
    a flexural rigidity D (by default D is computed from *Te*, Young's
    modulus, and Poisson's ratio; see **-C** to change these values).

.. _-G:

**-G**\ *outfile*
    If **-T** is set then *grdfile* must be a filename template that contains
    a floating point format (C syntax).  If the filename template also contains
    either %s (for unit name) or %c (for unit letter) then we use the corresponding time
    (in units specified in **-T**) to generate the individual file names, otherwise
    we use time in years with no unit.

Optional Arguments
------------------

.. _-A:

**-A**\ *Nx*/*Ny*/*Nxy*
    Specify in-plane compressional or extensional forces in the x- and y-directions,
    as well as any shear force [no in-plane forces].  Compression is indicated by
    negative values, while extensional forces are specified using positive values.

.. _-C:

**-Cp**\ *poisson*
    Change the current value of Poisson's ratio [0.25].

**-Cy**\ *Young*
    Change the current value of Young's modulus [7.0e10 N/m^2].

.. _-F:

**-F**\ *nu_a*\ [\ /*h_a*/*nu_m*]
    Specify a firmoviscous model in conjunction with an elastic plate
    thickness specified via **-E**.  Just give one viscosity (*nu_a*)
    for an elastic plate over a viscous half-space, or also append
    the thickness of the asthenosphere (*h_a*) and the lower mantle
    viscosity (*nu_m*), with the first viscosity now being that of
    the asthenosphere. Give viscosities in Pa*s. If used, give the
    thickness of the asthenosphere in meter; append **k** for km.

.. _-N:

.. include:: ../../explain_fft.rst_

.. _-L:

**-L**\ *list*
    Write the names and evaluation times of all grids that were created
    to the text file *list*. Requires **-T**.

.. _-M:

**-M**\ *tm*
    Specify a viscoelastic model in conjunction with an elastic plate
    thickness specified via **-E**.  Append the Maxwell time *tm* for the
    viscoelastic model (in ).

.. _-S:

**-S**\ *beta*
    Specify a starved moat fraction in the 0-1 range, where 1 means the moat is fully
    filled with material of density *ri* while 0 means it is only filled with
    material of density *rw* (i.e., just water) [1].

.. _-T:

**-T**\ *t0*\ [**u**]\ [/*t1*\ [**u**]/*dt*\ [**u**]\ \|\ *file*]\ \|\ *n*]\ [**+l**]
    Specify *t0*, *t1*, and time increment (*dt*) for sequence of calculations
    [Default is one step, with no time dependency].  For a single specific time, just
    give start time *t0*. The unit is years; append **k** for kyr and **M** for Myr.
    For a logarithmic time scale, append **+l** and specify *n* steps instead of *dt*.
    Alternatively, give a *file* with the desired times in the first column (these times
    may have individual units appended, otherwise we assume year).
    We then write a separate model grid file for each given time step.

.. _-W:

**-W**\ *wd*
    Set reference depth to the undeformed flexed surface in m [0].  Append **k** to indicate
    km.

.. _-Z:

**-Z**\ *zm*
    Specify reference depth to flexed surface (e.g., Moho) in m; append **k** for km.
    Must be positive. [0].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

|SYN_OPT-f|
   Geographic grids (dimensions of longitude, latitude) will be converted to
   meters via a "Flat Earth" approximation using the current ellipsoid parameters.

.. include:: ../../explain_help.rst_

.. include:: ../../explain_grd_inout_short.rst_

Grid Distance Units
-------------------

If the grid does not have meter as the horizontal unit, append **+u**\ *unit*
to the input file name to convert from the specified unit to meter.  If your
grid is geographic, convert distances to meters by supplying |SYN_OPT-f| instead.

Considerations
--------------

netCDF COARDS grids will automatically be recognized as geographic. For
other grids geographical grids were you want to convert degrees into
meters, select |SYN_OPT-f|. If the data are close to either pole, you should
consider projecting the grid file onto a rectangular coordinate system
using :doc:`grdproject </grdproject>`.

Plate Flexure Notes
-------------------

The FFT solution to plate flexure requires the infill density to equal
the load density.  This is typically only true directly beneath the load; beyond the load
the infill tends to be lower-density sediments or even water (or air).  Wessel [2001, 2016]
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

To compute the firmoviscous response to a series of incremental loads given by
file name and load time in the table l.lis at the single time 1 Ma using the
specified rheological values, try

::

    gmt grdflexure -T1M =l.lis -D3300/2800/2800/1000 -E5k -Gflx/smt_fv_%03.1f_%s.nc -F2e20 -Nf+a

References
----------

Cathles, L. M., 1975, *The viscosity of the earth's mantle*, Princeton University Press.

Wessel. P., 2001, Global distribution of seamounts inferred from gridded Geosat/ERS-1 altimetry,
J. Geophys. Res., 106(B9), 19,431-19,441,
`http://dx.doi.org/10.1029/2000JB000083 <http://dx.doi.org/10.1029/2000JB000083>`_.

Wessel, P., 2016, Regional–residual separation of bathymetry and revised estimates
of Hawaii plume flux, *Geophys. J. Int., 204(2)*, 932-947,
`http://dx.doi.org/10.1093/gji/ggv472 <http://dx.doi.org/10.1093/gji/ggv472>`_.


See Also
--------

:doc:`gmt </gmt>`, :doc:`grdfft </grdfft>`,
:doc:`gravfft </supplements/potential/gravfft>`
:doc:`grdmath </grdmath>`, :doc:`grdproject </grdproject>`,
:doc:`grdseamount </supplements/potential/grdseamount>`
