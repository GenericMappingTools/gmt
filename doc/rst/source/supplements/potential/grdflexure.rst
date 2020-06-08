.. index:: ! grdflexure
.. include:: ../module_supplements_purpose.rst_

**********
grdflexure
**********

|grdflexure_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt grdflexure** *topogrd*
|-D|\ *rm*/*rl*\ [/*ri*]\ /*rw*
|-E|\ [*Te*\ [**k**][/*Te2*\ [**k**]]]
|-G|\ *outgrid*
[ |-A|\ *Nx*/*Ny*/*Nxy* ]
[ |-C|\ **p**\ *poisson* ] [ |-C|\ **y**\ *Young* ]
[ |-F|\ *nu_a*\ [/*h_a*\ [**k**]/*nu_m*] ]
[ |-L|\ *list* ]
[ |-M|\ *tm* ]
[ |-N|\ *params* ]
[ |-Q| ]
[ |-S|\ *beta* ]
[ |-T|\ *t0*\ [/*t1*/*dt*]\ \|\ *file*\ [**+l**] ]
[ |SYN_OPT-V| ]
[ |-W|\ *wd*]\ [**k**]
[ |-Z|\ *zm*]\ [**k**]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdflexure** computes the deformation due to a topographic load :math:`h(\mathbf{x})`
for five different types of rheological foundations, all involving *constant thickness thin plates*:

#. An elastic plate overlying an inviscid half-space,
#. An elastic plate overlying a viscous half-space (Firmoviscous or Kelvin-Voigt),
#. An elastic plate overlying a viscous layer over a viscous half-space (Firmoviscous or Kelvin-Voigt),
#. A viscoelastic plate overlying an inviscid half-space (Maxwell solid),
#. A general linear viscoelastic model with an initial and final elastic plate thickness overlying an inviscid half-space.

These conditions will require the *elastic* [1; :math:`\Phi_e(\mathbf{k})`],
*firmoviscous* [2,3; :math:`\Phi_{fv}(\mathbf{k},t)`],
*viscoelastic* [4; :math:`\Phi_{ve}(\mathbf{k},t)`],
and *general linear* (viscoelastic) response functions [5; :math:`\Phi_{gl}(\mathbf{k},t)`]
If the (visco)elastic plate vanishes (zero thickness) then we obtain Airy isostasy
(1,4) or a purely *viscous* response (2,3).  Temporal evolution can
also be modeled by providing incremental load grids for select times and specifying a
range of model output times.  A wide range of options allow specifying the desired
rheology and related constants, including in-plate forces.


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
    Sets density for mantle, load, infill, and water (or air).  If *ri* differs from
    *rl* then an approximate solution will be found.  If *ri* is not given
    then it defaults to *rl*.  Values may be given in km/m^3 or g/cm^3.

.. _-E:

**-E**\ [*Te*\ [**k**][/*Te2*\ [**k**]]
    Sets the elastic plate thickness (in meter); append **k** for km.
    If the elastic thickness exceeds 1e10 it will be interpreted as
    a flexural rigidity *D* (by default, *D* is computed from *Te*, Young's
    modulus, and Poisson's ratio; see **-C** to change these values).
    If just **-E** is given and **-F** is used it means no plate is given
    and we will return a purely viscous response with or without an asthenospheric layer.
    Select a general linear viscoelastic response by supplying both an initial and
    final elastic thickness; this response also requires **-M**.

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
    Specify in-plane compressional or extensional forces in the *x*- and *y*-directions,
    as well as any shear force [no in-plane forces].  Compression is indicated by
    negative values, while extensional forces are specified using positive values.
    Values are expected in Pa·m since **N** is the depth-integrated horizontal
    stresses.

.. _-C:

**-Cp**\ *poisson*
    Change the default value of Poisson's ratio [0.25].

**-Cy**\ *Young*
    Change the default value of Young's modulus [7.0e10 N/m^2].

.. _-F:

**-F**\ *nu_a*\ [\ /*h_a*\ [**k**]/*nu_m*]
    Specify a firmoviscous model in conjunction with an elastic plate
    thickness specified via **-E**.  Just give one viscosity (*nu_a*)
    for an elastic plate over a viscous half-space, or also append
    the thickness of the asthenosphere (*h_a*) and the lower mantle
    viscosity (*nu_m*), with the first viscosity now being that of
    the asthenosphere. Give viscosities in Pa·s. If used, give the
    thickness of the asthenosphere in meter; append **k** for km.
    Cannot be used in conjunctions with **-M**.

.. _-L:

**-L**\ *list*
    Write the names and evaluation times of all grids that were created
    to the text file *list*. Requires **-T**.

.. _-N:

.. include:: ../../explain_fft.rst_

.. _-M:

**-M**\ *tm*
    Specify a viscoelastic model in conjunction with a plate
    thickness specified via **-E**.  Append the Maxwell time *tm* for the
    viscoelastic model (in years); add **k** for kyr and **M** for Myr.
    Cannot be used in conjunctions with **-F**.

.. _-Q:

**-Q**
    Do not make any flexure calculations but instead take the chosen response function
    given the parameters you selected and evaluate it for a range of wavenumbers and
    times; see the note on transfer functions below.

.. _-S:

**-S**\ *beta*
    Specify a starved moat fraction in the 0-1 range, where 1 means the moat is fully
    filled with material of density *ri* while 0 means it is only filled with
    material of density *rw* (i.e., just water) [1].

.. _-T:

**-T**\ *t0*\ [/*t1*/*dt*]\ \|\ *file*\ [**+l**]
    Specify *t0*, *t1*, and time increment (*dt*) for a sequence of calculations
    [Default is one calculation, with no time dependency].  For a single specific time, just
    give start time *t0*. Default *unit* is years; append **k** for kyr and **M** for Myr.
    For a logarithmic time scale, append **+l** and specify *n* steps instead of *dt*.
    Alternatively, give a *file* with the desired times in the first column (these times
    may have individual units appended, otherwise we assume year).
    We then write a separate model grid file for each given time step; see *-G** for output
    file template format.

.. _-W:

**-W**\ *wd*\ [**k**]
    Set reference water depth for the undeformed flexed surface in m.  Must be positive. [0].  Append **k** to indicate
    km.  If **-W** is used and your load exceeds this depth then we scale the subaerial part of the
    load to account for the change in surrounding density (air vs water).

.. _-Z:

**-Z**\ *zm*\ [**k**]
    Specify reference depth to flexed surface (e.g., Moho) in m; append **k** for km.
    Must be positive. [0].  We subtract this value from the flexed surface before
    writing the results.

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

If a Cartesian grid does not have meter as the horizontal unit, append **+u**\ *unit*
to the input file name to convert from the specified unit to meter.  E.g., appending
**+uk** to the load file name will scale the grid x,y coordinates from km to meter.  If your
grid is geographic, convert distances to meters by supplying |SYN_OPT-f| instead.
netCDF COARDS geographic grids will automatically be recognized as geographic.

Considerations
--------------

The calculations are done using a rectangular Cartesian FFT operation. If your
geographic region is close to either pole, you should consider using a Cartesian
setup instead; you can always project it back to geographic using :doc:`grdproject </grdproject>`.

Transfer Functions
------------------

If **-Q** is given we perform no actual flexure calculations and no input data file is required.
Instead, we write the chosen transfer functions :math:`\Phi(\mathbf{k},t)` to 7 separate files for
7 different Te values (1, 2, 5, 10, 20, 50, and 100 km). The first two columns are
always wavelength in km and wavenumber (in 1/m) for a 1:1:3000 km range. The transfer
functions are evaluated for 12 different response times: 1k, 2k, 5k, 10k, 20k, 50k,
100k, 200k, 500k, 1M, 2M, and 5M years. For a purely elastic response function
we only write the transfer function once per elastic thickness (in column 3).  The 7 files are named
grdflexure_transfer_function_te\ _\ *te*\ _km.txt, where *te* is replaced by the 7 elastic thicknesses
in km (and 0 if **-E**\ [0] was used for a viscous response only).

Examples
--------

We will use a Gaussian seamount load to demonstrate **grdflexure**.  First, we make
a grid of for that shape by placing a Gaussian truncated seamount at position (300,300)
with a radius of 50 km and height of 5000 m::

    echo 300 300 0  40  40  5000 | gmt grdseamount -R0/600/0/600+uk -I1000 -Gsmt.nc t.txt -Dk -E -F0.1 -Cg

To compute elastic plate flexure from the load *smt.nc*,
for a 10 km thick plate with typical densities, try::

    gmt grdflexure smt.nc -Gflex.nc -E10k -D2700/3300/1035

To see how in-plane stresses affect the result, we use **-A**.  Remember that we need to depth-
integrated forces, not pressures, hence we try::

    gmt grdflexure smt.nc -Gflex.nc -E10k -D2700/3300/1035 -A-4e11/2e11/-1e12

To compute viscoelastic plate flexure from the load *smt.nc*,
for a 20 km thick plate with typical densities and a Maxwell time of 40kyr, try::

    gmt grdflexure smt.nc -Gflex.nc -E20k -D2700/3300/1035 -M40k

To compute firmoviscous plate flexure from the load *smt.nc*,
for a 15 km thick plate with typical densities overlying a viscous mantle with viscosity 2e21, try::

    gmt grdflexure smt.nc -Gflex.nc -E15k -D2700/3300/1035 -F2e21

To compute the general linear viscoelastic plate flexure from the load *smt.nc*,
for an initial Te of 40 km and a final Te of 15 km  with typical densities and a Maxwell time of 100 kyr, try::

    gmt grdflexure smt.nc -Gflex.nc -E40k/15k -D2700/3300/1035 -M100k

To just compute the firmoviscous response functions using the specified rheological values, try::

    gmt grdflexure -D3300/2800/2800/1000 -Q -F2e20

The following are not user-reproducible but shows the kind of calculations that can be done.
To compute the firmoviscous response to a series of incremental loads given by
file name and load time in the table l.lis at the single time 1 Ma using the
specified rheological values, try::

    gmt grdflexure -T1M =l.lis -D3300/2800/2800/1000 -E5k -Gflx/smt_fv_%03.1f_%s.nc -F2e20 -Nf+a

Theory of Response Functions
----------------------------

Deformation :math:`w(\mathbf{x})` caused by topography :math:`h(\mathbf{x})` applied instantaneously to the
rheological foundation at time *t = 0* and evaluated at a later time *t* is given in the Fourier domain by

.. math::

    W(\mathbf{k},t) = \gamma \left (\frac{\rho_l - \rho_w}{\rho_m - \rho_l} \right ) H(\mathbf{k}) \Phi(\mathbf{k},t)
    = \gamma A H(\mathbf{k}) \Phi(\mathbf{k},t),

where :math:`\mathbf{k} = (k_x, k_y)` is the wavenumber vector, :math:`k_r` its magnitude, :math:`H(\mathbf{k})` is the
topographic load in the wavenumber domain, *A* is the Airy density ratio, :math:`\gamma` is a constant that depends
on the infill density, and :math:`\Phi(\mathbf{k},t)` is the response function for the selected rheology. The **grdflexure**
module read one or more loads *h*, transforms them to *H*, evaluates and applies the response function, and
inversely transform the results back to yield on or more *w* solutions.

Variable infill approximation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If :math:`\rho_i = \rho_l` then :math:`\gamma = 1`, otherwise the infill density varies spatially and the Fourier
solution is not valid. We avoid these complications by letting :math:`\rho_l = \rho_i` and increasing the deformation amplitude by

.. math::

    \gamma = \sqrt{\frac{\rho_m - \rho_i}{\rho_m - \rho_l}}.

The approximation is good except for very large loads on thin plates (*Wessel*, 2001).

.. figure:: /_images/grdflexure_approx.*
   :width: 400 px
   :align: center

   a) We want a flexural calculation that allows for different densities in the moat (:math:`\rho_i`)
   and beneath the load (:math:`\rho_l`). Unfortunately, the Fourier method requires a constant density contrast.
   b) Reusing the load density as a (higher) infill density gives an exact answer, but overestimates both the
   flexural wavelength (:math:`\lambda_l`) and the amplitude of deflection (:math:`w_l`).
   c) Reusing the infill density as a (lower) load density gives approximately the correct flexural wavelength but
   underestimates the amplitude (dashed curve). We achieve a satisfactory approximation by scaling :math:`w_i` by
   the factor :math:`\gamma` (modified from *Wessel* [2016]).

Elastic response function
~~~~~~~~~~~~~~~~~~~~~~~~~

The time-independent *elastic response function* is

.. math::

    \Phi_e(\mathbf{k}) = \left [ 1 + \alpha_r^4 + \epsilon_x \alpha_x^2 + \epsilon_y \alpha_y^2 + \epsilon_{xy} \alpha_{xy}^2 \right ]^{-1}, \quad \alpha_s = k_s / k,

where the *flexural wavenumber k* and constants :math:`\epsilon_s` via in-plane stresses :math:`N_x, N_y, N_{xy}` are

.. math::

    k = \left [ \frac{(\rho_m - \rho_i)g}{D} \right ]^{\frac{1}{4}}, \quad \epsilon_s = \left [ \frac{N_s}{(\rho_m - \rho_i)g} \right ]^{\frac{1}{2}},

for subscripts :math:`s = \left (x, y, xy \right )`.
In the most common scenario, :math:`N_s` are all zero and the elastic response function becomes *isotropic*:

.. math::

    \Phi_e(k_r) = \left [ 1 + \alpha_r^4 \right ]^{-1}.


Firmoviscous response function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The *firmoviscous response function* :math:`\Phi(\mathbf{k},t)` scales the magnitude of the deformation at a given wavenumber and time
and depends on rheological parameters and in-plane stresses:

.. math::

    \Phi_{fv}(\mathbf{k},t) = \Phi_e(\mathbf{k}) \left [ 1 - \exp \left \{ - \frac{(\rho_m - \rho_l) \tau(k_r)}{\rho_m\Phi_e(\mathbf{k})} t \right \} \right ].

If the foundation is an inviscid half-space, then the *relaxation parameter* :math:`\tau(k_r) = \infty`, there is no time-dependence,
and :math:`\Phi_{fv}(\mathbf{k},t) = \Phi_e(\mathbf{k})`. Otherwise, it is given by

.. math::

    \tau(k_r) = \frac{\rho_m g}{2 \eta_m k_r} \beta(k_r),

where :math:`\beta(k_r)` depends on whether we have a finite-thickness layer of thickness :math:`T_a` and viscosity
:math:`\eta_a` above the half-space of viscosity :math:`\eta_m` (*Cathles*, 1975; *Nakada*, 1986).
If no finite layer exists then :math:`\beta(k_r) = 1`, otherwise

.. math::

    \beta(k_r) = \frac{(\theta + \theta^{-1}) CS + k_r T_a (\theta - \theta^{-1}) + S^2 + C^2}{2CS\theta + (1-\theta)k_r^2 T_a^2 + \theta S^2 + C^2},

where

.. math::

    \theta = \eta_a/\eta_m, \quad S = \sinh (k_r T_a), \quad C = \cosh (k_r T_a).

Airy and viscous response function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the limit :math:`t \rightarrow \infty, \tau \rightarrow 0` and we approach the purely elastic solution

.. math::

    W(\mathbf{k}) = A \gamma H(\mathbf{k}) \Phi_e(\mathbf{k}).

Otherwise, if the plate has no strength (**-E**\ 0), then :math:`\Phi_e(\mathbf{k}) = 1` and the response function is purely *viscous* and isotropic:

.. math::

    \Phi_v(k_r,t) = \left [ 1 - \exp \left \{ - \frac{(\rho_m - \rho_l) \tau(k_r)}{\rho_m} t \right \} \right ].

For :math:`t \rightarrow \infty` (or for an inviscid half-space) we approach Airy isostasy: :math:`w(\mathbf{x}) = A h(\mathbf{x})`.

Maxwell viscoelastic response
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For case (4), the viscoelastic response function (only available for an inviscid substratum) is

.. math::

    \Phi_{ve}(\mathbf{k},t) = 1 - \left [ 1 - \Phi_e(\mathbf{k}) \right ] \exp \left \{ - \frac{t}{t_m} \Phi_e(\mathbf{k}) \right \},

where :math:`t_m` is the *Maxwell relaxation time* (*Watts*, 2001).

General linear viscoelastic response
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For case (5), the general linear viscoelastic response function (with an inviscid substratum) is (*Karner*, 1982)

.. math::

    \Phi_{gl}(\mathbf{k},t) = \Phi_f(\mathbf{k}) + \left [ \Phi_i(\mathbf{k}) - \Phi_f(\mathbf{k}) \right ] \exp \left \{ - \frac{t}{t_m} \frac{D_i \Phi_i(\mathbf{k})}{D_f \Phi_f(\mathbf{k})} \right \},

where subscripts *i* and *f* refers to the initial (*t = 0*) and final (:math:`t = \infty`) values for rigidities (:math:`D_i, D_f`)
and elastic response functions (:math:`\Phi_i, \Phi_f`).

References
----------

Cathles, L. M., 1975, *The viscosity of the earth's mantle*, Princeton University Press.

Karner, G. D., 1982, Spectral representation of isostatic models, *BMR J. Australian Geology & Geophysics, 7*, 55-62.

Nakada, M., 1986, Holocene sea levels in oceanic islands: Implications for the rheological
structure of the Earth's mantle, *Tectonophysics, 121*, 263–276,
`http://dx.doi.org/10.1016/0040-1951(86)90047-8 <http://dx.doi.org/10.1016/0040-1951(86)90047-8>`_.

Watts, A. B., 2001, *Isostasy and Flexure of the Lithosphere*, 458 pp., Cambridge University Press.

Wessel. P., 2001, Global distribution of seamounts inferred from gridded Geosat/ERS-1 altimetry,
J. Geophys. Res., 106(B9), 19,431-19,441,
`http://dx.doi.org/10.1029/2000JB000083 <http://dx.doi.org/10.1029/2000JB000083>`_.

Wessel, P., 2016, Regional–residual separation of bathymetry and revised estimates of Hawaii plume flux,
*Geophys. J. Int., 204(2)*, 932-947, `http://dx.doi.org/10.1093/gji/ggv472 <http://dx.doi.org/10.1093/gji/ggv472>`_.

See Also
--------

:doc:`gmt </gmt>`,
:doc:`gmtflexure </supplements/potential/gmtflexure>`,
:doc:`grdfft </grdfft>`,
:doc:`gravfft </supplements/potential/gravfft>`
:doc:`grdmath </grdmath>`, :doc:`grdproject </grdproject>`,
:doc:`grdseamount </supplements/potential/grdseamount>`
