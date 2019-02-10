.. index:: ! grdseamount

***********
grdseamount
***********

.. only:: not man

    grdseamount - Create synthetic seamounts (Gaussian, parabolic, cone or disc, circular or elliptical)

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt grdseamount** [ *intable* ]
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ [*out*/*in*\ ] ]
[ |-C|\ **c**\|\ **d**\ \|\ **g**\ \|\ **p** ]
[ |-D|\ [*unit*\ ] ]
[ |-E| ]
[ |-F|\ [*flattening*] ]
[ |-G|\ *grdfile* ]
[ |-L|\ [*cut*] ]
[ |-M|\ *list* ] [ |-N|\ *norm* ]
[ |-Q|\ *bmode*/*qmode* ]
[ |-S|\ *scale* ]
[ |-T|\ *t0*\ [**u**]\ [/*t1*\ [**u**]/*dt*\ [**u**]\ \|\ *n*]\ [**+l**] ]
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
parameters.  We read a list with seamount locations and sizes and can evaluate either
Gaussian, parabolic, conical, or disc shapes, which may be circular or elliptical, and optionally truncated.
Various scaling options are available to modify the result, including an option to add in
a background depth (more complicated backgrounds may be added via :doc:`grdmath </grdmath>`).
The input must contain *lon*, *lat*, *radius*, *height* for each seamount.
For elliptical features (**-E**) we expect *lon*, *lat*, *azimuth*, *semi-major*, *semi-minor*,
*height* instead. If flattening is specified (**-F**) with no value appended
then a final column with flattening is expected (cannot be used for plateaus).
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
    Here, height is ignored and **-L**, **-N** and **-Z** are disallowed.

.. _-C:

**-C**
    Select shape function: choose among **c** (cone), **d** (disc), **g** (Gaussian)
    and **p** (parabolic) shape [Default is Gaussian].

.. _-D:

**-D**\ *unit*
    Append the unit used for horizontal distances in the input file (see :ref:`Unit_attributes`).
    Does not apply for geographic data (|SYN_OPT-f|) which we convert to km.

.. _-E:

**-E**
    Elliptical data format. We expect the input records to contain
    *lon, lat, azimuth, major, minor, height* (with  the latter in m)
    for each seamount.  [Default is Circular data format, expecting
    *lon, lat, radius, height*\ ].

.. _-F:

**-F**\ [*flattening*]
    Seamounts are to be truncated to guyots.  Append *flattening*, otherwise we expect
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

**-M**\ *list*
    Write the names of all grids that were created to the text file *list*.
    Requires **-T**.

.. _-N:

**-N**\ *norm*
    Normalize grid so maximum grid height equals *norm*.

.. _-Q:

**-Q**\ *bmode*/*qmode*
    Only to be used in conjunction with **-T**.  Append two different modes settings:
    The *bmode* determines how we construct the surface.  Specify **c** for cumulative
    volume through time, or **i** for incremental volume added for each time slice.
    The *qmode* determines the volume flux curve.  Give **g** for a Gaussian volume flux history
    or **l** for a linear volume flux history between the start and stop times of each feature.

.. _-S:

**-S**\ *scale*
    Sets optional scale factor for radii [1].

.. _-T:

**-T**\ *t0*\ [**u**]\ [/*t1*\ [**u**]/*dt*\ [**u**]\ \|\ *n*]\ [**+l**]
    Specify *t0*, *t1*, and time increment (*dt*) for sequence of calculations
    [Default is one step, with no time dependency].  For a single specific time, just
    give start time *t0*. The unit is years; append **k** for kyr and **M** for Myr.
    For a logarithmic time scale, append **+l** and specify *n* steps instead of *dt*.
    Alternatively, give a file with the desired times in the first column (these times
    may have individual units appended, otherwise we assume year).  Note that the grid
    for *t0* (if a range is given) is not written as it is zero and marks the start of
    the building history.

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
from 3 Ma to 2 Ma and 2.8 M to 1.9 Ma using a linear volumetric production rate,
and output an incremental grid every 0.1 Myr from 3 Ma to 1.9 Ma, we can try:

::

    cat << EOF > t.txt
    #lon lat azimuth, semi-major, semi-minor, height tstart tend
    0	0	-20	120	60	5000	3.0M	2M
    50	80	-40	110	50	4000	2.8M	21.9M
    EOF
    gmt grdseamount -Rk-1024/1022/-1122/924 -I2000 -Gsmt_%3.1f_%s.nc t.txt -T3M/1.9M/0.1M -Qi/l -Dk -E -F0.2 -Cg -Ml.lis

See Also
--------

:doc:`gmt.conf </gmt.conf>`, :doc:`gmt </gmt>`,
:doc:`grdmath </grdmath>`, :doc:`gravfft </supplements/potential/gravfft>`,
:doc:`gmtflexure </supplements/potential/gmtflexure>`
