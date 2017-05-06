.. index:: ! talwani3d

*********
talwani3d
*********

.. only:: not man

    talwani3d - Compute geopotential anomalies over 3-D bodies by the method of Talwani

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**talwani3d** [ *modeltable* ]
[ |-A| ] [ |-D|\ *rho* ] ]
[ |-F|\ **f**\ \|\ **n**\ \|\ **v** ]
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
[ **-fg** ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ **-r** ] 
[ |SYN_OPT-x| ]

|No-spaces|

Description
-----------

**talwani3d** will read the multi-segment *modeltable* from file or standard input.
This file contains contours of a 3-D body at different *z*-levels, with one contour
per segment.  The segment header must contain the parameters *zlevel rho*, which
states the *z* contour level and the density of this slice (individual slice
densities may be overridden by a fixed density contrast given via **-D**).
We can compute anomalies on an equidistant grid (by specifying a new grid with
**-R** and **-I** or provide an observation grid with elevations) or at arbitrary
output points specified via **-N**.  Chose from free-air anomalies, vertical
gravity gradient anomalies, or geoid anomalies.  Options are available to control
axes units and direction.


Required Arguments
------------------

*modeltable*
    The file describing the horizontal contours of the bodies.  Contours will be
    automatically closed if not already closed, and repeated vertices will be eliminated.

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
    Sets fixed density contrast that overrides any setting in model file, in kg/m^3.

.. _-F:

**-F**\ **f**\ \|\ **n**\ \|\ **v**
    Specify desired gravitational field component.  Choose between **f** (free-air anomaly) [Default],
    **n** (geoid) or **v** (vertical gravity gradient).

.. _-G:

**-G**\ *outfile*
    Specify the name of the output data (for grids, see GRID FILE FORMATS below).
    Required when an equidistant grid is implied for output.  If **-N** is used
    then output is written to stdout unless **G** specifies an output file.

.. _-M:

**-M**\ [**h**]\ [**v**]
    Sets units used.  Append **h** to indicate horizontal distances are in km [m],
    and append **z** to indicate vertical distances are in km [m].

.. _-N:

**-N**\ *trackfile*
    Specifies locations where we wish to compute the predicted value.  When this option
    is used there are no grids and the output data records are written to stdout.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: ../../explain_-V.rst_

.. _-Z:

**-Z**\ *level*\ \|\ *obsgrid*
    Set observation level either as a constant or give the name of a grid with observation
    levels.  If the latter is used the the grid determines the output grid region [0].

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: ../../explain_-bi.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

**-fg**
    Geographic grids (dimensions of longitude, latitude) will be converted to
    km via a "Flat Earth" approximation using the current ellipsoid parameters.

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
and saved to body.txt, using 1.7 g/cm^3 as the density contrast, try

::

    gmt talwani3d -R-200/200/-200/200 -I2 -G3dgrav.nc body.txt -D1700 -Fg

To obtain the vertical gravity gradient anomaly along the track in crossing.txt
for the same model, try

::

    gmt talwani3d -Ncrossing.txt body.txt -D1700 -Fv > vgg_crossing.txt


Finally, the geoid anomaly along the same track in crossing.txt
for the same model is written to n_crossing.txt by

::

    gmt talwani3d -Ncrossing.txt body.txt -D1700 -Fn -Gn_crossing.txt


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
