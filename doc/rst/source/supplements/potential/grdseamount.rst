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
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ [*out*/*in*] ]
[ |-C|\ [**c**\|\ **d**\|\ **g**\|\ **p**] ]
[ |-D|\ *unit* ]
[ |-E| ]
[ |-F|\ [*flattening*] ]
[ |-G|\ *grdfile* ]
[ |-L|\ [*cut*] ]
[ |-M|\ [*list*] ]
[ |-N|\ *norm* ]
[ |-Q|\ *bmode*/*fmode*\ [**+d**] ]
[ |-S|\ *scale* ]
[ |-T|\ *t0*\ [/*t1*/*dt*]\ [**+l**] ]
[ |-Z|\ *level* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdseamount** will compute the combined shape of multiple synthetic seamounts given their individual shape
parameters.  We read from *table* (or stdin) a list of seamount locations and sizes and can evaluate either
Gaussian, parabolic, conical, or disc shapes, which may be circular or elliptical, and optionally truncated.
Various scaling options are available to modify the result, including an option to add in
a background depth (more complicated backgrounds may be added via :doc:`grdmath </grdmath>`).
The input must contain *lon*, *lat*, *radius*, *height* for each seamount.
For elliptical features (**-E**) we expect *lon*, *lat*, *azimuth*, *semi-major*, *semi-minor*,
*height* instead. If flattening is specified (**-F**) with no value appended
then a final column with *flattening* is expected (cannot be used for plateaus).
For temporal evolution of topography the **-T** option may be used, in which case the
data file must have two final columns with the start and stop time of seamount construction.
In this case you may choose to write out a cumulative shape or just the increments produced
by each time step (see **-Q**).

Required Arguments
------------------

.. _-I:

.. include:: ../../explain_-I.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [*out/in*]
    Build a mask grid, append outside/inside values [1/NaN].
    Here, height and flattening are ignored and **-L**, **-N** and **-Z** are disallowed.

.. _-C:

**-C**\ [**c**\|\ **d**\|\ **g**\|\ **p**]
    Select seamount shape function: choose among **c** (cone), **d** (disc), **g** (Gaussian)
    and **p** (parabolic) shape [Default is Gaussian].  All but the disc can furthermore
    be truncated via a flattening parameter *f* set by **-F**.  If **-C** is not given any
    argument then we will read the shape code from the last input column.  If **-C** is not given
    at all then we default to Gaussian shapes [**g**].

.. figure:: /_images/GMT_seamount_types.*
   :width: 500 px
   :align: center

   The four types of seamounts selectable via option **-C**.  In all cases, :math:`h_0` is the maximum
   *height*, :math:`r_0` is the basal *radius*, :math:`h_c` is the noise floor set via **-L** [0], and
   *f* is the *flattening* set via **-F** [0]. The top radius :math:`r_t` is only nonzero if there is
   flattening and hence does not apply to the disc model.

.. _-D:

**-D**\ *unit*
    Append the unit used for horizontal distances in the input file (see `Units`_).
    Does not apply for geographic data (|SYN_OPT-f|) which we convert to km.

.. _-E:

**-E**
    Elliptical data format. We expect the input records to contain
    *lon, lat, azimuth, major, minor, height* (with  the latter in m)
    for each seamount.  [Default is Circular data format, expecting
    *lon, lat, radius, height*].

.. figure:: /_images/GMT_seamount_map.*
   :width: 500 px
   :align: center

   Use **-E** to select elliptical rather than circular shape in map view.  Both shapes require
   lon, lat. Circular only requires the radius :math:`r_0` while elliptical requires the azimuth
   :math:`\alpha` and the major and minor semi-axes .

.. _-F:

**-F**\ [*flattening*]
    Seamounts are to be truncated to guyots.  Append *flattening* from 0 (no flattening) to 1 (no feature!), otherwise we expect
    to find it in last input column [no truncation].  Ignored if used with **-Cd**.

.. _-G:

**-G**\ *grdfile*
    Specify the name of the output grid file; see GRID FILE FORMATS below).
    If **-T** is set then *grdfile* must be a filename template that contains
    a floating point format (C syntax).  If the filename template also contains
    either %s (for unit name) or %c (for unit letter) then we use the corresponding time
    (in units specified in **-T**) to generate the individual file names, otherwise
    we use time in years with no unit.

.. _-L:

**-L**\ [*cut*]
    List area, volume, and mean height for each seamount; No grid is created.
    Optionally, append the noise-floor cutoff level below which we ignore area and volume [0].

.. _-M:

**-M**\ [*list*]
    Write the times and names of all grids that were created to the text file *list*.
    Requires **-T**.  If not *list* file is given then we write to standard output.

.. _-N:

**-N**\ *norm*
    Normalize grid so maximum grid height equals *norm* [no normalization]

.. _-Q:

**-Q**\ *bmode*/*fmode*\ [**+d**]
    Only to be used in conjunction with **-T**.  Append two different modes settings:
    The *bmode* determines how we construct the surface.  Specify **c** for cumulative
    volume through time [Default], or **i** for incremental volume added for each time slice.
    The *fmode* determines the volume flux curve we use.  Give **c** for a constant volume flux or
    **g** for a Gaussian volume flux [Default] between the start and stop times of each feature. These fluxes
    integrate to a linear or error-function volume fraction over time, respectively, as shown below.
    By default we compute the exact cumulative and incremental values for the seamounts specified.  Append
    **+d** to instead approximate each incremental layer by a disc of constant thickness.

.. figure:: /_images/GMT_seamount_cum_inc.*
   :width: 500 px
   :align: center

   Use *bmode* in **-Q** to choose between cumulative output (**c**; actual topography as function
   of time [left]) or incremental output (**i**; the difference in actual topography over five
   time-steps [right]).  Here we used **-Cg** for a Gaussian model with no flattening and a linear volume flux.

.. figure:: /_images/GMT_seamount_flux.*
   :width: 500 px
   :align: center

   Use *fmode* in **-Q** to choose between a constant (**c**; dashed line) or Gaussian (**g**; heavy line)
   volume flux model.

.. _-S:

**-S**\ *scale*
    Sets optional scale factor for radii [1].

.. _-T:

**-T**\ *t0*\ [/*t1*/*dt*]\ [**+l**]
    Specify *t0*, *t1*, and time increment (*dt*) for sequence of calculations
    [Default is one step, with no time dependency].  For a single specific time, just
    give start time *t0*. Default *unit* is years; append **k** for kyr and **M** for Myr.
    For a logarithmic time scale, append **+l** and specify *n* steps instead of *dt*.
    Alternatively, give a file with the desired times in the first column (these times
    may have individual units appended, otherwise we assume year).  Note that a grid
    will be written for all time-steps even if there are no loads or no changes.

.. _-Z:

**-Z**\ *level*
    Set the background depth [0].

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

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: ../../explain_-V.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_distunits.rst_


Examples
--------

To compute the incremental loads from two elliptical, truncated Gaussian seamounts being constructed
from 3 Ma to 2 Ma and 2.8 M to 1.9 Ma using a constant volumetric production rate,
and output an incremental grid every 0.1 Myr from 3 Ma to 1.9 Ma, we can try:

::

    cat << EOF > t.txt
    #lon lat azimuth, semi-major, semi-minor, height tstart tend
    0	0	-20	120	60	5000	3.0M	2M
    50	80	-40	110	50	4000	2.8M	21.9M
    EOF
    gmt grdseamount -Rk-1024/1022/-1122/924 -I2000 -Gsmt_%3.1f_%s.nc t.txt -T3M/1.9M/0.1M -Qi/c -Dk -E -F0.2 -Cg -Ml.lis

The file l.lis will contain records with numerical time, gridfile, and unit time.

See Also
--------

:doc:`gmt.conf </gmt.conf>`, :doc:`gmt </gmt>`,
:doc:`grdmath </grdmath>`, :doc:`gravfft </supplements/potential/gravfft>`,
:doc:`gmtflexure </supplements/potential/gmtflexure>`
