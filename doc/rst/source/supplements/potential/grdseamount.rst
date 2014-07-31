.. index:: ! grdseamount

***********
grdseamount
***********

.. only:: not man

    grdseamount - Compute synthetic seamount (Gaussian, parabolic, cone, disc, circular or elliptical) bathymetry

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**grdseamount** [ *intable* ]
|SYN_OPT-I|
|SYN_OPT-R|
[ **-A**\ [*out/in*\ ] ] [ **-Cc**\|\ **d**\ \|\ **g**\ \|\ **p** ] [ **-D**\ [*unit*\ ] ]
[ **-E** ] [ **-F**\ [*flattening*] ] [ **-G**\ *grdfile* ] [ **-L**\ [*cut*] ]
[ **-N**\ *norm* ] [ **-Q**\ *bmode*/*qmode* ] [ **-S**\ *scale* ]
[ **-T**\ [**l**]\ *t0*\ [/*t1*/*dt*] ] ] [ **-Z**\ *level* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ **-fg** ]
[ |SYN_OPT-i| ]
[ **-r** ] 

|No-spaces|

Description
-----------

**grdseamount** will compute the combined shape of multiple synthetic seamounts given their individual shape
parameters.  We read a list with seamount locations and sizes and can evaluate either
Gaussian, parabolic, conical, or disc shapes, which may be circular or elliptical, and optionally truncated.
Various scaling options are available to modify the result, including an option to add in
a background depth (more complicated backgrounds may be added via **grdmath**).
The input must contain *lon, lat, radius, height* for each seamount.
For elliptical features (**-E**) we expect *lon, lat, azimuth, semi-major, semi-minor,
 height* instead. If flattening is specified (**-F**) with no value appended
then a final column with flattening is expected (cannot be used for plateaus).
For temporal evolution of topography the **-T** option may be used, in which case the
data file must have two final columns with the start and stop time of seamount construction.
In this case you may choose to write out a cumulative shape or just the increments produced
by each time step (see **-Q**).

Required Arguments
------------------

.. include:: ../../explain_-I.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

Optional Arguments
------------------

**-A**\ [*out/in*]
    Build a mask grid, append outside/inside values [1/NaN].
    Here, height is ignored and **-L**, **-N** and **-Z** are disallowed

**-C**
    Select shape function: choose among **c** (cone), **d** (disc), **g** (Gaussian)
    and **p** (parabolic) shape [Default is Gaussian].

**-D**\ *unit*
    Append the unit used for horizontal distances in the input file (see UNITS).
    Does not apply for geographic data (**-fg**) which we convert to km.

**-E**
    Elliptical data format [Default is Circular]. Read lon, lat, azimuth,
    major, minor, height (m) for each seamount.

**-F**\ [*flattening*]
    Seamounts are to be truncated to guyots.  Append *flattening*, otherwise we expect
    to find it in last input column [no truncation].  Ignored if used with **-Cd**.

**-G**\ *grdfile*
    Specify the name of the output grid file; see GRID FILE FORMATS below).
    If **-T** is set then *grdfile* must be a filename template that contains
    a floating point format (C syntax) and we use the corresponding time
    (in units specified in **-T**) to generate the individual file names.

**-L**\ [*cut*]
    List area, volume, and mean height for each seamount; NO grid is created.
    Optionally, append the noise-floor cutoff level below which we ignore area and volume [0].

**-N**\ *norm*
    Normalize grid so maximum grid height equals *norm*.

**-Q**\ *bmode*/*qmode*
    Only to be used in conjunction with **-T**.  Append two different modes settings:
    The *bmode* determines how we construct the surface.  Specify **c** for cumulative
    volume through time, or **i** for incremental volume added for each time slice.
    The *qmode* determines the volume flux curve.  Give **g** for a Gaussian volume flux history
    or **l** for a linear volume flux history between the start and stop times of each feature.

**-S**\ *scale*
    Sets optional scale factor for radii [1].

**-T**\ [**l**]\ *start*/*stop*/*dt*
    Specify *start*, *stop*, and time increment (*dt*) for sequence of calculations
    [Default is one step, with no time dependency].  For a single specific time, just
    give *start*. The default unit is year; append **k** for kyr and **M** for Myr.
    For a logarithmic time scale, use **-Tl** and specify *n* steps instead of *dt*.
    We then write a separate grid file for each time step.

**-Z**\ *level*
    Set the background depth [0].

.. |Add_-bi| replace:: [Default is 4 input columns]. 
.. include:: ../../explain_-bi.rst_

**-fg**
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

To be added.

See Also
--------

:doc:`gmt.conf </gmt.conf>`, :doc:`gmt </gmt>`,
:doc:`grdmath </grdmath>`, :doc:`gravfft`
