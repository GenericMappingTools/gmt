.. index:: ! talwani3d

*********
talwani3d
*********

.. only:: not man

    talwani3d - Compute geopotential anomalies over 3-D bodies by the method of Talwani

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt talwani3d** [ *modeltable* ]
[ |-A| ] [ |-D|\ *rho* ] ]
[ |-F|\ **f**\ \|\ **n**\ [*lat*]\ \|\ **v** ] 
[ |-G|\ *outfile* ]
[ |SYN_OPT-I| ]
[ |-M|\ [**h**]\ [**v**] ]
[ |-N|\ *trackfile* ]
[ |SYN_OPT-R| ]
[ |-Z|\ *level*\ \|\ *obsgrid* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
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

**talwani3d** will read the multi-segment *modeltable* from file (or standard input).
This file contains horizontal contours of a 3-D body at different *z*-levels, with one contour
per segment.  Each segment header must contain the parameters *zlevel rho*, which
states the *z* level of the contour and the density of this slice (optionally, individual slice
densities may be overridden by a fixed density contrast given via **-D**).
We can compute anomalies on an equidistant grid (by specifying a new grid with
**-R** and **-I** or provide an observation grid with desired elevations) or at arbitrary
output points specified via **-N**.  Choose between free-air anomalies, vertical
gravity gradient anomalies, or geoid anomalies.  Options are available to control
axes units and direction.


Required Arguments
------------------

*modeltable*
    The file describing the horizontal contours of the bodies.  Contours will be
    automatically closed if not already closed, and repeated vertices will be eliminated.
    The segment header for each slice will be examined for the pair *zlevel rho*, i.e.,
    the depth level of the slice and a density contrast in kg/m^3; see **-D** for overriding this value.

.. _-I:

.. include:: ../../explain_-I.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

Optional Arguments
------------------

.. _-A:

**-A**
    The *z*-axis should be positive upwards [Default is down].

.. _-D:

**-D**\ *unit*
    Sets a fixed density contrast that overrides any individual slice settings in the model file, in kg/m^3.

.. _-F:

**-F**\ **f**\ \|\ **n**\ \|\ **v**
    Specify desired gravitational field component.  Choose between **f** (free-air anomaly) [Default],
    **n** (geoid; optionally append average latitude for normal gravity reference value [Default is
    mid-grid (or mid-profile if **-N**)]) or **v** (vertical gravity gradient).

.. _-G:

**-G**\ *outfile*
    Specify the name of the output data (for grids, see GRID FILE FORMATS below).
    Required when an equidistant grid is implied for output.  If **-N** is used
    then output is written to stdout unless **-G** specifies an output file.

.. _-M:

**-M**\ [**h**]\ [**v**]
    Sets distance units used.  Append **h** to indicate that both horizontal distances are in km [m],
    and append **z** to indicate vertical distances are in km [m].

.. _-N:

**-N**\ *trackfile*
    Specifies individual (x, y[, z]) locations where we wish to compute the predicted value.  When this option
    is used there are no grids and the output data records are written to stdout.  If *trackfile*
    has 3 columns we take the *z* value as our observation level; this level may be overridden via **-Z**.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: ../../explain_-V.rst_

.. _-Z:

**-Z**\ *level*\ \|\ *obsgrid*
    Set observation level, either as a constant or variable by giveing the name of a grid with observation
    levels.  If the latter is used then this grid determines the output grid region as well [0].

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: ../../explain_-bi.rst_

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

To compute the free-air anomalies on a grid over a 3-D body that has been contoured
and saved to body3d.txt, using 1700 kg/m^3 as the fixed density contrast, with
horizontal distances in km and vertical distances in meters, try

::

    gmt talwani3d -R-200/200/-200/200 -I2 -Mh -G3dgrav.nc body3d.txt -D1700 -Ff

To obtain the vertical gravity gradient anomaly along the track in crossing.txt
for the same model, try

::

    gmt talwani3d -Ncrossing.txt -Mh body3d.txt -D1700 -Fv > vgg_crossing.txt


Finally, the geoid anomaly along the same track in crossing.txt
for the same model (at 30S) is written to n_crossing.txt by

::

    gmt talwani3d -Ncrossing.txt -Mh body3d.txt -D1700 -Fn-30 -Gn_crossing.txt


References
----------

Kim, S.-S., and P. Wessel, 2016, New analytic solutions for modeling vertical
gravity gradient anomalies, *Geochem. Geophys. Geosyst., 17*, 
`http://dx.doi.org/10.1002/2016GC006263 <http://dx.doi.org/10.1002/2016GC006263>`_.

Talwani, M., and M. Ewing, 1960, Rapid computation of gravitational attraction of
three-dimensional bodies of arbitrary shape, *Geophysics, 25*, 203-225.

See Also
--------

:doc:`gmt.conf </gmt.conf>`, :doc:`gmt </gmt>`,
:doc:`grdmath </grdmath>`, :doc:`gravfft </supplements/potential/gravfft>`,
:doc:`gmtgravmag3d </supplements/potential/gmtgravmag3d>`,
:doc:`grdgravmag3d </supplements/potential/grdgravmag3d>`,
:doc:`talwani2d </supplements/potential/talwani2d>`
