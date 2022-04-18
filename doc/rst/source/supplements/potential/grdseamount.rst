.. index:: ! grdseamount
.. include:: ../module_supplements_purpose.rst_

***********
grdseamount
***********

|grdseamount_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt grdseamount** [ *table* ]
|-G|\ *outgrid*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ [*out*/*in*][**+s**\ *scale*] ]
[ |-C|\ [**c**\|\ **d**\|\ **g**\|\ **o**\|\ **p**] ]
[ |-D|\ *unit* ]
[ |-E| ]
[ |-F|\ [*flattening*] ]
[ |-H|\ *H*/*rho_l*/*rho_h*\ [**+d**\ *densify*][**+p**\ *power*] ]
[ |-K|\ [*densitymodel*] ]
[ |-L|\ [*hn*] ]
[ |-M|\ [*list*] ]
[ |-N|\ *norm* ]
[ |-Q|\ *bmode*\ [/*fmode*]\ [**+d**] ]
[ |-S|\ [**+a**\ [*az1*/*az2*]][**+b**\ [*beta*]][**+d**\ [*hc*]][**+h**\ [*h1*/*h2*]][**+p**\ [*power*]][**+t**\ [*t0*/*t1*]][**+u**\ [*u0*]][**+v**\ [*phi*]] ]
[ |-T|\ *t0*\ [/*t1*/*dt*]\ [**+l**] ]
[ |-Z|\ *level* ]
[ |SYN_OPT-V| ]
[ |-W|\ *avedensity* ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdseamount** will compute the combined topographic shape of multiple synthetic seamounts given
their individual shape parameters.  We read from *table* (or standard input) a list of seamount
locations and sizes and can evaluate either Gaussian, parabolic, conical, polynomial or disc
shapes, which may be circular or elliptical, and optionally truncated. Various options are available
to modify the result, including an option to add in a background depth or set unmodified nodes to NaN
(more complicated backgrounds may be added separately via :doc:`grdmath </grdmath>`). The input data
must contain *lon*, *lat*, *radius*, *height* for each seamount. For elliptical features (requires
**-E**), we expect *lon*, *lat*, *azimuth*, *semi-major*, *semi-minor*, *height* instead. If
flat seamount tops is specified (via **-F**) with no value appended, then an extra column with
*flattening* is expected (**-F** cannot be used for plateaus). For temporal evolution of topography
the **-T** option may be used, in which case the data file must have two additional columns with
the start and stop time of seamount construction. In this case, you may choose to write out a
cumulative shape or just the increments produced for each time step (see **-Q**).  If land slides
are considered (**-S**), then this initial set of input columns are followed by one or more groups
of slide parameters; see **-S** for the arrangement. Finally, for mixing different seamount shapes
in the input *table* you can use the trailing text to give the shape code by using **-C** without
appending an argument.

.. _SMT_slide:

.. figure:: /_images/GMT_seamount_slide.*
   :width: 500 px
   :align: center

   Truncated polynomial seamount with the scars of two mass-wasting events on its flank.  Users can
   assign a time-span to the construction of the seamount and separate time-spans for any desired
   mass-wasting events.

Required Arguments (if **-L** is not given)
-------------------------------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. _-G:

.. |Add_outgrid| replace:: Give the name of the output grid file. If |-T| is set then *outgrid* must be
    a filename template that contains a floating point format to hold the output time (C language syntax).
    If the filename template also contains either %s (for unit name) or %c (for unit letter) then we use
    the corresponding time (in units specified in |-T|) to generate the individual file names, otherwise
    we use time in years with no unit included.
.. include:: /explain_grd_inout.rst_
    :start-after: outgrid-syntax-begins
    :end-before: outgrid-syntax-ends

.. _-I:

.. include:: ../../explain_-I.rst_

.. |Add_-R| replace:: |Add_-R_links|
.. include:: ../../explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Optional Arguments
------------------

.. _-A:

**-A**\ [*out/in*][**+s**\ *scale*]
    Build a mask grid only that may be used to manipulate gridded data sets to exclude or to isolate
    seamount observations; append outside/inside values [1/NaN]. Here, height and flattening
    are ignored and **-L**, **-N** and **-Z** are disallowed. Optionally, use **+s** to
    increase all seamount radii or semi-axes first (e.g., "bleeding the mask outwards") [1].

.. _-C:

**-C**\ [**c**\|\ **d**\|\ **g**\|\ **o**\|\ **p**]
    Select a seamount :ref:`shape function <SMT_types>`: choose among **c** (cone), **d** (disc), **g** (Gaussian)
    **o** (polynomial) and **p** (parabola) [Default is Gaussian].  All but the disc can furthermore
    be truncated via a flattening parameter *f* set by **-F**.  If **-C** is not given any argument,
    then we will read the shape code from the input file's trailing text.  If **-C**
    is not given at all, then we default to Gaussian seamounts [**g**].  **Note**: The polynomial
    model has a normalized amplitude *v* for a normalized radius :math:`u = r / r_0` that is given
    by :math:`v(u) = (1+u)^3(1-u)^3/(1+u^3)`. It is comparable to the Gaussian model (its
    volume is just ~2.4% larger), and *v* goes exactly to zero at the basal radius :math:`r_0`.

.. _SMT_types:

.. figure:: /_images/GMT_seamount_types.*
   :width: 500 px
   :align: center

   The five types of seamounts selected via option **-C**.  In all cases, :math:`h_0` is the maximum
   *height*, :math:`r_0` is the basal *radius*, :math:`h_n` is the noise floor set via **-L** [0], and
   *f* is the *flattening* set via **-F** [0]. The top radius :math:`r_t = f r_0` is only nonzero if
   there is flattening, hence it does not apply to the disc model.

.. _-D:

**-D**\ *unit*
    Append the Cartesian unit used for horizontal distances in the input file (see `Units`_).
    Not allowed for geographic data, which we automatically convert to km using a flat Earth
    assumption.

.. _-E:

**-E**
    Set :ref:`elliptical <SMT_map>` data file format. We expect input records to contain
    *lon, lat, azimuth, semi-major, semi-minor, height* (with  the latter in meter)
    for each seamount [Default is Circular data format, expecting *lon, lat, radius, height*].
    To mix circular and elliptical seamounts you must use **-E** and provide the circular
    parameters as elliptical ones via *azimuth = 0* and *semi-major = semi-minor = radius*.

.. _SMT_map:

.. figure:: /_images/GMT_seamount_map.*
   :width: 500 px
   :align: center

   Use **-E** to select elliptical rather than circular shapes in map view.  Both shapes require
   lon, lat. Circular shapes only require the radius :math:`r_0`, while elliptical ones require the
   azimuth :math:`\alpha` of the major axis as well as the major and minor semi-axes .

.. _-F:

**-F**\ [*flattening*]
    Seamounts will be truncated to guyots.  Append *flattening* from 0 (no flattening) up to, but not
    including, 1. If no argument is given then we expect to find the flattening in the last input column
    [no truncation].  Ignored if used with **-Cd**.

.. _-H:

**-H**\ *H*/*rho_l*/*rho_h*\ [**+d**\ *densify*][**+p**\ *power*]
    Set reference seamount parameters for an *ad-hoc* variable radial :ref:`density function <SMT_rho>`
    with depth. Give the low and high seamount densities in kg/m^3 or g/cm^3 and the fixed reference height
    *H* in meters. Use modifiers **+d** and **+p** to change the water-pressure-driven flank density increase
    over the full reference height [0] and set the variable density profile exponent *power* [1, i.e., a linear
    change]. Below, *h(r)* is the final height of any seamount and *z(r)* is a point inside the seamount.
    If the seamount is truncated (via **-F**) then *h(r)* refers to the original height.  **Note**: If
    **-V** is used then we report the mean density for each seamount processed.  The radial density function
    is thus defined (with :math:`\Delta \rho_s = \rho_h - \rho_l` and the *densify* setting is :math:`\Delta \rho_f`):

.. math::

    \rho(r,z) = \rho_l + \Delta \rho_f \left (\frac{H-h(r)}{H} \right ) + \Delta \rho_s \left ( \frac{h(r)-z(r)}{H} \right )^p

.. _SMT_rho:

.. figure:: /_images/GMT_seamount_density.*
   :width: 500 px
   :align: center

   A linear density distribution selected with option **-H**.  Flank density can be affected by water
   pressure if :math:`\Delta \rho_f > 0`, while the normalized internal density gradient is raised to
   power *p* to allow for nonlinear gradients.  **Note**: The reference height *H* refers to a very tall
   seamount for which the supplied densities are suitable.  Smaller seamounts will thus see lower core
   densities by virtue of being smaller.

.. _-K:

**-K**\ *densitymodel*
    Append the file name for a crossection grid with the predicted densities of the reference model.
    We use normalized coordinates (both *x* (radius) and *y* (height) go from 0 to 1 in increments
    of 0.005), yielding a fixed 201 x 201 grid. **Note**: This option can be used without creating the
    seamount grid, hence **-R**, **-I**, **-G**, and **-D** are not required.

.. _-L:

**-L**\ [*hn*]
    List *area*, *volume*, and *mean height* for each seamount; no grid will be created.
    Optionally, append *hn* for a noise-floor cutoff level below which we ignore area and volume [0].

.. _-M:

**-M**\ [*list*]
    Write the times and names of all relief grids (and density grids if **-W** is set) that were created
    to the text file *list*. Requires **-T**.  If no *list* file is given then we write to standard output.
    The leading numerical column will be time in years, while the last trailing text word is formatted time.
    The output listing is suitable as input to :doc:`grdflexure </supplements/potential/grdflexure>`.
    **Note**: The output records thus contain *time reliefgrid* [ *densitygrid* ] *timetag*.

.. _-N:

**-N**\ *norm*
    Normalize the relief grid(s) so the maximum grid height equals *norm* [no normalization].

.. _-Q:

**-Q**\ *bmode*\ [/*fmode*]\ [**+d**]
    Set build modes. Can only be used in conjunction with **-T**. The default is **-Qc**\ /**g**; append
    alternative mode settings (separated by a slash if both are specified) to change that:

        * The required *bmode* determines how we :ref:`construct <SMT_inc>` the surface: Specify **c** for cumulative
          volume through time or **i** for the incremental volume added for each time increment.

        * The optional *fmode* determines the :ref:`volume flux curve <SMT_flux>` we use: Give **c** for a constant
          volume flux or **g** for a Gaussian volume flux [Default] between the start and stop times of each feature.

    These flux models integrate to a linear and error-function volume fraction curve over time, respectively, as
    shown below. By default, we compute the exact cumulative and incremental values for the seamounts
    specified.  Append **+d** to instead approximate each incremental layer by a disc of constant thickness.

.. _SMT_inc:

.. figure:: /_images/GMT_seamount_cum_inc.*
   :width: 500 px
   :align: center

   Use *bmode* in **-Q** to choose between cumulative output (**c**; actual topography as function
   of time [left]) or incremental output (**i**; the differences in actual topography over five
   time-steps [right]).  Here, we used **-Cg** for a Gaussian model with no flattening and a Gaussian
   volume flux.

.. _SMT_flux:

.. figure:: /_images/GMT_seamount_flux.*
   :width: 500 px
   :align: center

   Use *fmode* in **-Q** to choose between a constant (**c**; dashed line) or Gaussian (**g**; heavy line)
   volume flux model.  **Note**: We adjust the error-function curve so that it goes exactly from
   0 to 1 over the time window.

.. _-S:

**-S**\ [**+a**\ [*az1*/*az2*]][**+b**\ [*beta*]][**+d**\ [*hc*]][**+h**\ [*h1*/*h2*]][**+p**\ [*power*]][**+t**\ [*t0*/*t1*]][**+u**\ [*u0*]][**+v**\ [*phi*]]

    Set parameters controlling the simulation of sectoral, rotational :ref:`land slides <SMT_slide>` by providing
    suitable modifiers. Parameters given on the command line apply to all seamounts equally.
    However, if a modifier is set but not given an argument then we read those arguments from
    the end of the input record; the order of such input arguments follows alphabetically from
    the modifier codes (and not the order the modifiers may appear on the command line). Repeat
    the slide group columns if there is more than one slide to read per seamount. Use these
    modifiers to set relevant slide parameters:

        * **+a** specifies the :ref:`azimuthal sector <SMT_specs>` affected by the slide [0/360].

        * **+b** sets a positive power coefficient :math:`\beta` for the normalized slide volume
          fraction :ref:`time-curve <SMT_psi>` :math:`\psi(\tau) = \tau^\beta` [Default is linear,
          i.e., 1]. See **+t** for definition of :math:`\tau`.

        * **+d** sets the :ref:`flank level <SMT_specs>` of the seamount where the debris deposit begins [:math:`h_1/2`].

        * **+h** sets the :ref:`lower and upper heights <SMT_specs>` of the flank source area generating the landslide.

        * **+p** activates :ref:`angular variation <SMT_azim>` in the radial slide profile; append
          a power parameter *power > 2*. **Note**: For multiple slides it is valid to provide *power*
          = 0 for some (either via command argument or read from file), which simply turns off angular
          variation for those slides. See **+u** for the radial slide profile setting.

        * **+t** sets the time span over which the slide develops via :math:`\psi(\tau)` (see **+b**),
          where :math:`\tau = (t - t_0)/(t_1 - t_0)` is the normalized time span 0-1 when the slide
          occurs; this modifier also requires **-T** to be set.

        * **+u** sets normalized radial slide :ref:`shape parameter <SMT_u0>` :math:`u_0 > 0` [0.2].

        * **+v** sets desired fractional volume :math:`\phi` of the slide (in percent) relative to
          the entire seamount volume.

    **Note**: If **+v** is set then we must compute the corresponding *u0*, hence **+u** is
    not allowed. If **+b**, **+d**, or **+u** are not set then their defaults are used for all slides.
    Currently, we support a maximum of 10 slides per seamount.

.. _SMT_specs:

.. figure:: /_images/GMT_seamount_specs.*
   :width: 500 px
   :align: center

   Geometry for an *ad hoc* landslide approximation (via cross-section modifiers **+d** and **+h**
   and map-view parameter **+a**). The volume of the slide material (pink) will be deposited at or
   below the toe of the surface rupture (light blue), starting at a height of :math:`h_c` and linearly
   tapering to zero at a distal point :math:`r_d`. The various radii are computed from the heights and
   the shape function of the seamount.  **Note**: :math:`h_2 > h_1` while :math:`r_1 > r_2`.

.. _-T:

**-T**\ *t0*\ [/*t1*/*dt*]\ [**+l**]
    Specify *t0*, *t1*, and time increment (*dt*) for a sequence of calculations
    [Default is one step, with no time dependency]. For a single specific time, just
    give start time *t0*. Default *unit* is years; append **k** for kyr and **M** for Myr.
    For a logarithmic time scale, append **+l** and given the number of steps *n* instead
    of increment *dt*. Alternatively, give a file with the desired times in the first column
    (these times may have individual units appended, otherwise we assume year).  If **-T** is
    set, then the input seamount table is expected to have to extra columns for the *start*
    and *stop* time following the initial seamount parameters (but before any slide
    groups; see **-S**). Because positive time is in years before present, we require
    *t0* >= *t1* time. **Note**: A grid will be written for all time-steps even
    if there are no relief or changes for one or more output times.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *avedensity*
    Give the name of the vertically averaged density grid file. If **-T** is set then *avedensity* must
    be a filename template that contains a floating point format (C syntax; see **-G** for details).  If the filename
    template also contains either %s (for unit name) or %c (for unit letter) then we use the corresponding
    time (in units specified in **-T**) to generate the individual file names, otherwise we use time in years
    with no unit. Requires **-H** to define the density model.

.. _-Z:

**-Z**\ *level*
    Set the background water depth [0]. As all seamounts have positive relief, you may
    use a larger negative *level* to place them under water. Alternatively, append NaN
    if you wish to flag unused nodes (not allowed if **-Qi** is used).

.. |Add_-bi| replace:: [Default is 4 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

|SYN_OPT-f|
    Geographic grids (dimensions of longitude, latitude) will be converted to
    km via a "Flat Earth" approximation using the current ellipsoid parameters.

.. |Add_-h| replace:: Not used with binary data.
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_distunits.rst_


Slide simulation specifics
--------------------------

The simulation of land slides via **-S** is not a physical model (apart from preserving mass).  Instead,
we approximate the shapes of rotational landslides and their evolution via simple geometric shapes and functions.
The :ref:`radial slide height <SMT_u0>` is modeled as

.. math::

    h_s(r) = h_s(u) = h_1 + (h_2 - h_1)q(u) = h_1 + (h_2 - h_1) u_0 \left (\frac{1 + u_0}{u + u_0} - 1\right ),

where :math:`u = (r - r_2)/(r_1 - r_2)` is the normalized horizontal distance of the rupture
surface.  This shape can be modulated by using **+u** to change :math:`u_0`.

.. _SMT_u0:

.. figure:: /_images/GMT_seamount_u0.*
   :width: 500 px
   :align: center

   A variety of normalized radial slide shapes :math:`q(u)` is available by varying :math:`u_0` (via modifier **+u**).
   For instance, the slide area for a conical seamount would be the area between the flank (dashed line) and the
   selected curve. A smaller :math:`u_0` will cut more deeply into the seamount.

By default, the radial slide profile is fixed regardless of where in the slide sector we look.  However,
gentle or more abrupt :ref:`angular variation <SMT_azim>` can be added to :math:`h_s(r)` via the function

.. math::

    s(\alpha) = s(\gamma) = 1 - \lvert \gamma \rvert ^p,

where :math:`\gamma = 2 (\alpha - \alpha_1)/(\alpha_2 - \alpha_1) - 1` and *p* is the power exponent
that can be set via **+p**. When enabled, we use :math:`s(\alpha)` to scale the radial slide profile
so that it starts of at zero at the two sectoral locations and then grows more (larger *p*) or less
(smaller *p*) rapidly as we enter the slide sector.  This modifier has the effect of smoothing the step
functions we otherwise encounter as we enter the slide sector.

.. _SMT_azim:

.. figure:: /_images/GMT_seamount_azim.*
   :width: 500 px
   :align: center

   A range of azimuthal amplitude variation in radial slide height :math:`h_s(r)` can be achieved by modulating
   the power parameter, *p* (via modifier **+p**). This variation means the slide volume is reduced by
   :math:`1 - \bar{s}` (dashed lines). E.g., for *p = 2* the slide volume is only 67% of the volume it
   would have been if there was no azimuthal variation (i.e., *s = 1* [Default]).

Finally, an observed landslide may not have occurred instantly but developed over a finite time period.  We can
simulate that by distributing the total slide volume over this time. The normalized volume rate distribution
is simulated by

.. math::

    \psi(t) = \psi(\tau) = \tau^\beta = \left (\frac{t - t_0}{t_1 - t_0} \right )^\beta,

where :math:`\tau` is the normalized time during a slide event.  We use this function to compute the portion
of the slide volume that should be deposited at a given time *t*.  Modifier **+b** is used to specify the
power exponent :math:`\beta` [1].

.. _SMT_psi:

.. figure:: /_images/GMT_seamount_psi.*
   :width: 500 px
   :align: center

   We can control how quickly a slide evolves over time by manipulating the :math:`\psi(\tau)` function
   (via modifier **+b**). A linear curve means mass redistribution is taking place at a constant
   rate during the slide duration. Adjust :math:`\beta` to have the bulk of the redistribution
   take place early (:math:`\beta < 1`) or closer to the end (:math:`\beta > 1`) of the event.

Notes
-----

Because the Gaussian curve only drops to 1.11% of its peak amplitude at the base radius (3 sigma)
of a seamount, we actually evaluate Gaussian curves all the way to 4 sigma so that the amplitude
drops to 0.034% of peak height before we jump to zero.  This prevents the otherwise very noticeable
step at the base of the seamount.

Examples
--------

To compute the shape of a circular Gaussian seamount located at 1W, 2S on a 1 arc minute grid for a basal radius
of 30 km and a height of 4500 meters, try::

    echo 1W 2S 30 4500 | gmt grdseamount -R1:30W/0:30W/2:30S/1:30S -I1m -Ggeo_circ_smt.nc
    gmt grdimage geo_circ_smt.nc -B -png circ

To compute the shape of an elliptical conic seamount located at 200,400 on a 1x1 Cartesian grid for basal
semi-major axis of 35 and semi-minor axis of 20, with an azimuth of 29 for the major axis,  a height of
3700 meters, and some flattening (0.15), try::

    echo 200 400 29 35 20 3700 | gmt grdseamount -R150/250/350/450 -I1 -E -F0.15 -Gcat_ell_smt.nc
    gmt grdview cat_ell_smt.nc -B -Qi -I+d -JZ2c -p195/20 -png ell

To compute the incremental loads from two elliptical, truncated Gaussian seamounts being constructed
from 3 Ma to 2 Ma and 2.8 M to 1.9 Ma using a constant volumetric production rate,
and output an incremental grid every 0.1 Myr from 3 Ma to 1.9 Ma, and internally use meters for
horizontal distances, we can try::

    cat << EOF > t.txt
    #lon lat azimuth, semi-major, semi-minor, height tstart tend
    0	0	-20	120	60	5000	3.0M	2M
    50	80	-40	110	50	4000	2.8M	21.9M
    EOF
    gmt grdseamount -R-1024/1022/-1122/924+uk -I2000 -Gsmt_%3.1f_%s.nc t.txt -T3M/1.9M/0.1M -Qi/c -Dk -E -F0.2 -Cg -Mfiles.txt

The file files.txt will contain records with numerical time, gridfile, and formatted-time.

**Note**: There are many more examples of use, including working with :doc:`grdflexure </supplements/potential/grdflexure>`, in the `test directory <https://github.com/GenericMappingTools/gmt/tree/master/test/potential>`_ for the potential supplement.

See Also
--------

:doc:`gmt.conf </gmt.conf>`,
:doc:`gmt </gmt>`,
:doc:`grdmath </grdmath>`,
:doc:`gravfft </supplements/potential/gravfft>`,
:doc:`grdflexure </supplements/potential/grdflexure>`

Reference
---------

Smith, J. R.  and Wessel, P., 2000, Isostatic consequences of giant landslides on the Hawaiian Ridge,
Pure Appl. Geophys., 157, 1097-1114,
`https://doi.org/10.1007/s000240050019 <https://doi.org/10.1007/s000240050019>`_.
