.. index:: ! gravprisms
.. include:: ../module_supplements_purpose.rst_

**********
gravprisms
**********

|gravprisms_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt gravprisms** [ *table* ]
[ |-A| ]
[ |-C|\ [**+q**][**+w**\ *file*][**+z**\ *dz*] ]
[ |-D|\ *density* ]
[ |-E|\ *dx*\ /*dy* ]
[ |-F|\ **f**\|\ **n**\ [*lat*]\|\ **v** ]
[ |-G|\ *outfile* ]
[ |-H|\ *H*/*rho_l*/*rho_h*\ [**+d**\ *densify*][**+p**\ *power*] ]
[ |SYN_OPT-I| ]
[ |-L|\ *base* ]
[ |-M|\ [**h**]\ [**v**] ]
[ |-N|\ *trackfile* ]
[ |SYN_OPT-R| ]
[ |-S|\ *shapegrid* ]
[ |-T|\ *top* ]
[ |SYN_OPT-V| ]
[ |-W|\ *avedens* ]
[ |-Z|\ *level*\|\ *obsgrid* ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**gravprisms** will compute the geopotential field over vertically oriented, rectangular prisms.
We either read the multi-segment *table* from file (or standard input), which may contain up to
7 columns: The first four are *x y z_low z_high*, i.e., the center *x, y* coordinates and the
vertical range of the prism from *z_low* to *z_high*, while the next two columns hold the dimensions
*dx dy* of each prism (see **-E** if all prisms have the same *x*- and *y*-dimensions).
Last column may contain individual prism densities (but will be overridden by a fixed density contrast
if given via **-D**).  Alternatively, we can use **-C** to create the prisms needed to approximate
the entire feature (**-S**) or just the volume between two surfaces (one of which may be a constant)
that define a layer (set via **-L** and **-T**).  If a variable density model (**-H**) is selected
then each vertical prism will be broken into constant-density, stacked sub-prisms using a prescribed
vertical increment *dz*, otherwise single tall prisms are created with constant (**-D**) or spatially
varying (**-W**) densities.
We can compute anomalies on an equidistant grid (by specifying a new grid with
**-R** and **-I** or provide an observation grid with desired elevations) or at arbitrary
output points specified via **-N**.  Choose between free-air anomalies, vertical
gravity gradient anomalies, or geoid anomalies.  Options are available to control
axes units and direction.

.. figure:: /_images/GMT_seamount_prisms.*
   :width: 500 px
   :align: center

   Three density models modeled by prisms for a truncated Gaussian seamount via **-C**: (left) Constant
   density (**-D**), (middle) vertically-averaged density varying radially (**-W**), and
   (right) density varies with *r* and *z* (**-H**), requiring a stack of prisms.

Required Arguments
------------------

*table*
    The file describing the prisms with record format *x y z_lo z_hi* [ *dx dy* ] [ *rho* ],
    where the optional items are controlled by options **-E** and **-D**, respectively.
    Density contrasts can be given in kg/m^3 of g/cm^3.

.. _-I:

.. include:: ../../explain_-I.rst_

.. |Add_-R| replace:: |Add_-R_links|
.. include:: ../../explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Optional Arguments
------------------

.. _-A:

**-A**
    The *z*-axis should be positive upwards [Default is down].

.. _-C:

**-C**\ [**+q**][**+w**\ *file*][**+z**\ *dz*] ]
    Create prisms for the entire feature given by **-S**\ *height*, or just for the layer between the two surfaces
    specified via **-L**\ *base* and **-T**\ *top*. For layers, either *base* or *top* may be a constant rather
    than a grid.  If only *height* is given then we assume we will approximate the entire feature from *base* = 0
    to *height*.  If **-H** is used to compute variable density contrasts then we must split each prism into a stack
    of sub-prisms with individual densities; thus *dz* needs to be set for the heights of these sub-prisms (the first
    and last sub-prisms in the stack may have their heights adjusted to match the limits of the surfaces).  Without
    **-H** we only create a single uniform-density prism, but those prisms may have spatially varying densities via
    **-W**.  Append **+w**\ *file* to save the prisms to a table, and optionally use **+q** to quit execution once
    the file has been saved, i.e., no geopotential calculations will take place.

.. _-D:

**-D**\ *density*
    Sets a fixed density contrast that overrides any individual prism settings in the prisms file, in kg/m^3 of g/cm^3.

.. _-E:

**-E**\ *dx*\ /*dy*
    If all prisms in *table* have constant x/y-dimensions then they can be set here.  In that case *table*
    only contains the centers of each prism and the *z* range (and optionally *density*; see **-D**).

.. _-F:

**-F**\ **f**\|\ **n**\ [*lat*]\|\ **v**
    Specify desired gravitational field component.  Choose between **f** (free-air anomaly) [Default],
    **n** (geoid; optionally append average latitude for normal gravity reference value [Default is
    mid-grid (or mid-profile if **-N**)]) or **v** (vertical gravity gradient).

.. _-G:

**-G**\ *outfile*
    Specify the name of the output data (for grids, see :ref:`Grid File Formats
    <grd_inout_full>`). Required when an equidistant grid is implied for output.
    If **-N** is used then output is written to standard output unless **-G**
    specifies an output file.

.. _-H:

**-H**\ *H*/*rho_l*/*rho_h*\ [**+d**\ *densify*][**+p**\ *power*]
    Set reference seamount parameters for an *ad-hoc* variable radial density function with depth. Give
    the low and high seamount densities in kg/m^3 or g/cm^3 and the fixed reference height *H* in meters.
    Use modifers **+d** and **+p** to change the water-pressure-driven flank density increase over the
    full reference height [0] and the variable density profile exponent *power* [1, i.e., a linear change].
    Requires **-S** to know the full height of the seamount.
    See :doc:`grdseamount </supplements/potential/grdseamount>` for more details.

.. _-L:

**-L**\ *base*
    Give name of the base surface grid for a layer we wish to approximate with prisms, or give
    a constant *z*-level [0].

.. _-M:

**-M**\ [**h**]\ [**v**]
    Sets distance units used.  Append **h** to indicate that both horizontal distances are in km [m],
    and append **z** to indicate vertical distances are in km [m].  If selected, we will internally
    convert any affected distance provided by data input or command line options to meters. **Note**:
    Any output will retain the original units.

.. _-N:

**-N**\ *trackfile*
    Specifies individual (x, y[, z]) locations where we wish to compute the predicted value.  When this option
    is used there are no grids involved and the output data records are written to standard output (see **-bo** for binary output).
    If **-Z** is not set then *trackfile* must have 3 columns and we take the *z* value as our observation level; otherwise the level must be set via **-Z**.
    **Note**: If **-G** is used to set an output file we will write the output table to that file instead of standard output.

.. _-S:

**-S**\ *height*
    Give name of grid with the full seamount heights, either for making prisms or as required by **-H**.

.. _-T:

**-T**\ *top*
    Give name of the top surface grid for a layer we wish to approximate with prisms, or give a constant *z*-level.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *avedens*
    Give name of an input grid with spatially varying, vertically-averaged prism densities. Requires **-C** and
    the grid must be co-registered with the grid provided by **-S** (or **L** and **-T**).

.. _-Z:

**-Z**\ *level*\|\ *obsgrid*
    Set observation level, either as a constant or variable by giving the name of a grid with observation
    levels.  If the latter is used then this grid determines the output grid region as well [0].

.. |Add_-bo| replace:: [Default is 2 output columns].
.. include:: ../../explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

|SYN_OPT-f|
    Geographic grids (i.e., dimensions of longitude, latitude) will be converted to
    km via a "Flat Earth" approximation using the current ellipsoidal parameters.

.. |Add_-h| replace:: Not used with binary data.
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-ocols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. include:: ../../explain_core.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_distunits.rst_


Examples
--------

To compute the free-air anomalies on a grid over a set of prisms given in prisms.txt,
using 1700 kg/m^3 as the fixed density contrast, with
horizontal distances in km and vertical distances in meters, try

::

    gmt gravprisms -R-200/200/-200/200 -I2 -Mh -G3dgrav.nc prisms.txt -D1700 -Ff

To obtain the vertical gravity gradient anomaly along the track in crossing.txt
for the same model, try

::

    gmt gravprisms -Ncrossing.txt -Mh prisms.txt -D1700 -Fv > vgg_crossing.txt


Finally, the geoid anomaly along the same track in crossing.txt
for the same model (at 30S) is written to n_crossing.txt by

::

    gmt gravprisms -Ncrossing.txt -Mh prisms.txt -D1700 -Fn-30 -Gn_crossing.txt


References
----------


Grant, F. S. and West, G. F., 1965, *Interpretation Theory in Applied Geophysics*,
583 pp., McGraw-Hill.

Kim, S.-S., and P. Wessel, 2016, New analytic solutions for modeling vertical
gravity gradient anomalies, *Geochem. Geophys. Geosyst., 17*,
`https://dx.doi.org/10.1002/2016GC006263 <https://dx.doi.org/10.1002/2016GC006263>`_.

Nagy D., Papp G., Benedek J., 2000, The gravitational potential and its derivatives
for the prism, *J. Geod., 74*, 552–560,
`https://dx.doi.org/10.1007/s001900000116 <https://dx.doi.org/10.1007/s001900000116>`_.

See Also
--------

:doc:`gmt.conf </gmt.conf>`,
:doc:`gmt </gmt>`,
:doc:`grdmath </grdmath>`,
:doc:`gravfft </supplements/potential/gravfft>`,
:doc:`gmtgravmag3d </supplements/potential/gmtgravmag3d>`,
:doc:`grdgravmag3d </supplements/potential/grdgravmag3d>`,
:doc:`grdseamount </supplements/potential/grdseamount>`,
:doc:`talwani2d </supplements/potential/talwani2d>`,
:doc:`talwani3d </supplements/potential/talwani3d>`
