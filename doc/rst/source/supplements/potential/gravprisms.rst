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
[ |-D|\ *density*\ [**+c**] ]
[ |-E|\ *dx*\ [/*dy*] ]
[ |-F|\ **f**\|\ **n**\ [*lat*]\|\ **v** ]
[ |-G|\ *outfile* ]
[ |-H|\ *H*/*rho_l*/*rho_h*\ [**+b**\ *boost*][**+d**\ *densify*][**+p**\ *power*] ]
[ |SYN_OPT-I| ]
[ |-L|\ *base* ]
[ |-M|\ [**h**]\ [**z**] ]
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
*dx dy* of each prism (see |-E| if all prisms have the same *x*- and *y*-dimensions).
Last column may contain individual prism densities (but will be overridden by fixed or variable density contrasts
if given via |-D|).  Alternatively, we can use |-C| to *create* the prisms needed to approximate
the entire feature (**-S**) or just the volume between two surfaces (one of which may be a constant)
that define a layer (set via |-L| and |-T|).  If a variable density model (**-H**) is selected
then each vertical prism will be broken into constant-density, stacked sub-prisms using a prescribed
vertical increment *dz*, otherwise single tall prisms are created with constant or spatially variable densities (**-D**).
We can compute anomalies on an equidistant grid (by specifying a new grid with
**-R** and |-I| or provide an observation grid with desired elevations) or at arbitrary
output points specified via |-N|.  Choose between free-air anomalies, vertical
gravity gradient anomalies, or geoid anomalies.  Options are available to control
axes units and direction.

.. figure:: /_images/GMT_seamount_prisms.*
   :width: 600 px
   :align: center

   Three density models modeled by prisms for a truncated Gaussian seamount via **-C**: (left) Constant
   density (**-D**), (middle) vertically-averaged density varying radially (**-D**), and
   (right) density varies with *r* and *z* (**-H**), requiring a stack of prisms.

Required Arguments
------------------

*table*
    The file describing the prisms with record format *x y z_lo z_hi* [ *dx dy* ] [ *rho* ],
    where the optional items are controlled by options |-E| and |-D|, respectively.
    Density contrasts can be given in :math:`\mbox{kg/m}^3` of :math:`\mbox{g/cm}^3`. **Note**: If |-C| is used then
    no *table* will be read.

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
    to *height*.  If |-H| is used to compute variable density contrasts then we must split each prism into a stack
    of sub-prisms with individual densities; thus *dz* needs to be set for the heights of these sub-prisms (the first
    and last sub-prisms in the stack may have their heights adjusted to match the limits of the surfaces).  Without
    |-H| we only create a single uniform-density prism, but those prisms may have spatially varying densities via
    |-D|.  Append **+w**\ *file* to save the prisms to a table, and optionally use **+q** to quit execution once
    that file has been saved, i.e., no geopotential calculations will take place.

.. _-D:

**-D**\ *density*\ [**+c**]
    Sets a fixed density contrast that overrides any individual prism settings in the prisms file, in
    :math:`\mbox{kg/m}^3` of :math:`\mbox{g/cm}^3`. Append **+c** to instead subtract this density from
    the individual prism densities. Alternatively, give name of an input grid with spatially varying,
    vertically-averaged prism densities. This requires |-C| and the grid must be co-registered with the
    grid provided by |-S| (or |-L| and |-T|).  **Note**: If |-H| is used then a fixed density may be set
    via |-D| provided its modifier **+c** is set. We will then compute *density contrasts* in the seamount
    relative to the fixed *density* (such as density of seawater for underwater seamounts).

.. _-E:

**-E**\ *dx*\ [/*dy*]
    If all prisms in *table* have constant x/y-dimensions then they can be set here.  In that case *table*
    must only contain the centers of each prism and the *z* range (and optionally *density*; see |-D|).
    If only *dx* is given then we set *dy = dx*. **Note**: For geographic coordinates the *dx* dimension
    is in geographic longitude increment and hence the physical width of the prism will decrease with latitude
    if *dx* stays numerically the same.

.. _-F:

**-F**\ **f**\|\ **n**\ [*lat*]\|\ **v**
    Specify desired gravitational field component.  Choose between **f** (free-air anomaly) [Default],
    **n** (geoid; optionally append average latitude for normal gravity reference value [Default is
    mid-grid (or mid-profile if |-N|)]) or **v** (vertical gravity gradient).

.. _-G:

**-G**\ *outfile*
    Specify the name of the output data (for grids, see :ref:`Grid File Formats
    <grd_inout_full>`). Required when an equidistant grid is implied for output.
    If |-N| is used then output is written to standard output unless |-G|
    specifies an output table name.

.. _-H:

**-H**\ *H*/*rho_l*/*rho_h*\ [**+b**\ *boost*][**+d**\ *densify*][**+p**\ *power*]
    Set reference seamount parameters for an *ad-hoc* variable radial density function with depth. Give
    the low and high seamount densities in :math:`\mbox{kg/m}^3` or :math:`\mbox{g/cm}^3` and the fixed reference height *H* in meters.
    Use modifiers **+d** and **+p** to change the water-pressure-driven flank density increase over the
    full reference height [0] and the variable density profile exponent *power* [1, i.e., a linear change].
    To simulate the higher starting densities in truncated guyots you can *boost* the seamount height by this factor [1].
    Requires |-S| to know the full height of the seamount.
    See :doc:`grdseamount </supplements/potential/grdseamount>` for more details.

.. _-L:

**-L**\ *base*
    Give name of the base surface grid for a layer we wish to approximate with prisms, or give
    a constant *z*-level [0].

.. _-M:

**-M**\ [**h**]\ [**z**]
    Sets distance units used.  Append **h** to indicate that both horizontal distances are in km [m],
    and append **z** to indicate vertical distances are in km [m].  If selected, we will internally
    convert any affected distance provided by data input or command line options to meters. **Note**:
    Any output will retain the original units.

.. _-N:

**-N**\ *trackfile*
    Specifies individual (x, y[, z]) locations where we wish to compute the predicted value.
    When this option is used there are no grids involved and the output data records are written
    to standard output (see **-bo** for binary output). If |-Z| is not set then *trackfile* must
    have 3 columns and we take the *z* value as our observation level; otherwise s constant level must be
    set via |-Z|. **Note**: If |-G| is used to set an output file we will write the output table
    to that file instead of standard output.

.. _-S:

**-S**\ *height*
    Give name of grid with the full seamount heights, either for making prisms or as required by |-H|.

.. _-T:

**-T**\ *top*
    Give name of the top surface grid for a layer we wish to approximate with prisms, or give a
    constant *z*-level.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *avedens*
    Give name of an output grid with spatially varying, vertically-averaged prism densities created
    by |-C| and |-H|.

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

We have prepared a set of 2828 prisms that represent a truncated Gaussian seamount of
height 6000 m, radius 30 km, with the base at z = 0 m, available in the remote file @prisms.txt.
A quick view of the 3-D model can be had via::

    gmt plot3d -R-30/30/-30/30/0/7000 -JX12c -JZ3c -Ggray -So1q+b @prisms.txt -B -Wfaint -p200/20 -pdf smt

To compute the free-air anomalies on a grid over the set of prisms given in @prisms.txt,
using 1700 :math:`\mbox{kg/m}^3` as a fixed density contrast, with horizontal distances in km and
vertical distances in meters, observed at 7000 m, try::

    gmt gravprisms -R-40/40/-40/40 -I1 -Mh -G3dgrav.nc @prisms.txt -D1700 -Ff -Z7000
    gmt grdimage 3dgrav.nc -B -pdf 3dgrav

To obtain the vertical gravity gradient anomaly along the track given in crossing.txt
for the same model, try::

    gmt math -T-30/30/0.1 T 0 MUL = crossing.txt
    gmt gravprisms -Ncrossing.txt -Mh @prisms.txt -D1700 -Fv -Z7000 > vgg_crossing.txt
    gmt plot vgg_crossing.txt -R-30/30/-50/400 -i0,3 -W1p -B -pdf vgg_crossing

Finally, redo the gravity calculation but now use the individual prism densities in the
prism file and restrict calculations to the same crossing profile, i.e.::

    gmt gravprisms -Ncrossing.txt -Mh @prisms.txt -Ff -Z7000 > faa_crossing.txt
    gmt plot faa_crossing.txt -R-30/30/0/350 -i0,3 -W1p -B -pdf faa_crossing

To build prisms using a variable density grid for an interface crossing the zero level
and obtain prisms with the negative of the given density contrast if below zero and the
positive density contrast if above zero, try::

    gmt gravprisms -TFlexure_surf.grd -C+wprisms_var.txt+q -DVariable_drho.grd

Grids Straddling Zero Level
---------------------------

When creating prisms from grids via |-C|, a special case arises when a single surface (set via
|-L| or |-T|) straddles zero.  This may happen if the surface reflects flexure beneath a
load, which has in a negative moat flanked by positive bulges.  When such a *interface* grid
is detected we build prisms going from *z* to zero for negative *z* and from 0 to *z* for
positive *z*. As we flip below zero we also change the sign of the given density contrast.
You can override this behavior by specifying the opposite layer surface either by a constant
or another grid. E.g., if |-L| specifies the base surface you can eliminate prisms exceeding zero
via **-T**\ 0, and by interchanging the |-L| and |-T| arguments you can eliminate prisms below
zero.  **Note**: When two surfaces are implied we keep the given density contrast as given.

Note on Precision
-----------------

The analytical expression for the geoid over a vertical prism (Nagy et al., 2000) is
fairly involved and contains 48 terms.  Due to various cancellations the end result
is more unstable than the simpler expressions for gravity and VGG.  Be aware that the
result may have less significant digits that you may expect.

References
----------

Grant, F. S. and West, G. F., 1965, *Interpretation Theory in Applied Geophysics*,
583 pp., McGraw-Hill.

Kim, S.-S., and P. Wessel, 2016, New analytic solutions for modeling vertical
gravity gradient anomalies, *Geochem. Geophys. Geosyst., 17*,
`https://doi.org/10.1002/2016GC006263 <https://doi.org/10.1002/2016GC006263>`_.

Nagy D., Papp G., Benedek J., 2000, The gravitational potential and its derivatives
for the prism, *J. Geod., 74*, 552–560,
`https://doi.org/10.1007/s001900000116 <https://doi.org/10.1007/s001900000116>`_.

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
