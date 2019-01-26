.. index:: ! talwani2d

*********
talwani2d
*********

.. only:: not man

    talwani2d - Compute geopotential anomalies over 2-D bodies by the method of Talwani

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt talwani2d** [ *modeltable* ]
[ |-A| ] [ |-D|\ *rho* ] ]
[ |-F|\ **f**\ \|\ **n**\ [*lat*]\ \|\ **v** ] 
[ |-M|\ [**h**]\ [**v**] ]
[ |-N|\ *trackfile* ]
[ |-T|\ [\ *min/max*\ /]\ *inc*\ [**n**] \|\ |-T|\ *file*\ \|\ *list* ]
[ |-Z|\ *level*\ [*ymin*\ /*ymax*\ ] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**talwani2d** will read the multi-segment *modeltable* from file (or standard input).
This file contains cross-sections of one or more 2-D bodies, with one polygon
per segment.  The segment header must contain the parameter *rho*, which
states the the density of this body (individual body
densities may be overridden by a fixed constant density contrast given via an optional **-D**).
We can compute anomalies on an equidistant lattice (by specifying a lattice with
**-T**) or provide arbitrary output points specified in a file via **-N**.
Choose between free-air anomalies, vertical gravity gradient anomalies, or geoid anomalies.
Options are available to control axes units and direction.


Required Arguments
------------------

*modeltable*
    The file describing cross-sectional polygons of one or more bodies.  Polygons
    will be automatically closed if not already closed, and repeated vertices will
    be eliminated.  The segment header for each body will be examined for a density
    parameter in kg/m^3; see **-D** for overriding this value.

Optional Arguments
------------------

.. _-A:

**-A**
    The *z*-axis should be positive upwards [Default is down].

.. _-D:

**-D**\ *unit*
    Sets a fixed density contrast that overrides any per-body settings in the model file, in kg/m^3.

.. _-F:

**-F**\ **f**\ \|\ **n**\ [*lat*]\ \|\ **v**
    Specify desired gravitational field component.  Choose between **f** (free-air anomaly) [Default],
    **n** (geoid; optionally append average latitude for normal gravity reference value [45])
    or **v** (vertical gravity gradient).

.. _-M:

**-M**\ [**h**]\ [**v**]
    Sets distance units used.  Append **h** to indicate horizontal distances are in km [m],
    and append **z** to indicate vertical distances are in km [m].

.. _-N:

**-N**\ *trackfile*
    Specifies locations where we wish to compute the predicted value.  When this option
    is used you cannot use **-T** to set an equidistant lattice. The output data records are written to stdout.

.. _-T:

**-T**\ [\ *min/max*\ /]\ *inc*\ [**n**] \|\ |-T|\ *file*\ \|\ *list*
    Specify an equidistant output lattice.
    For details on array creation, see `Generate 1D Array`_.

.. _-Z:

**-Z**\ *level*\ [*ymin*\ /*ymax*\ ]
    Set a constant observation level [0].  Optionally, and for gravity anomalies only (**-Ff**),
    append the finite extent limits of a 2.5-D body.

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: ../../explain_-bi.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-ocols.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: ../../explain_-V.rst_

.. include:: ../../explain_core.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_distunits.rst_

.. include:: ../../explain_array.rst_

Examples
--------

To compute the free-air anomalies on a equidistant profile over a 2-D body that has been contoured
and saved to body2d.txt, using 1700 kg/m^3 as a constant density contrast, with all distances in meters,
try

::

    gmt talwani2d -T-200/200/2 body2d.txt -D1700 -Ff > 2dgrav.txt

To obtain the vertical gravity gradient anomaly along the track given by the file crossing.txt
for the same model, try

::

    gmt talwani2d -Ncrossing.txt body2d.txt -D1700 -Fv > vgg_crossing.txt


The geoid anomaly for the same setup, evaluated at 60N, is given by

::

    gmt talwani2d -Ncrossing.txt body2d.txt -D1700 -Fn60 > n_crossing.txt


Notes
-----

#. The 2-D geoid anomaly is a logarithmic potential and thus has no natural
   reference level.  We simply remove the most negative (if density contrast
   is positive) or positive (if density contrast is negative) computed value
   from all values, rendering the entire anomaly positive (or negative).  You
   can use :doc:`gmtmath </gmtmath>` to change the zero level to suit your needs.

References
----------

Rasmussen, R., and L. B. Pedersen (1979), End corrections in potential field modeling,
*Geophys. Prospect., 27*, 749-760.

Chapman, M. E., 1979, Techniques for interpretation of geoid anomalies,
*J. Geophys. Res., 84(B8)*, 3793-3801.

Kim, S.-S., and P. Wessel, 2016, New analytic solutions for modeling vertical
gravity gradient anomalies, *Geochem. Geophys. Geosyst., 17*, 
`http://dx.doi.org/10.1002/2016GC006263 <http://dx.doi.org/10.1002/2016GC006263>`_.

Talwani, M., J. L. Worzel, and M. Landisman, 1959, Rapid gravity computations for
two-dimensional bodies with application to the Mendocino submarine fracture zone,
*J. Geophys. Res., 64*, 49-59.

See Also
--------

:doc:`gmt.conf </gmt.conf>`, :doc:`gmt </gmt>`,
:doc:`grdmath </grdmath>`, :doc:`gmtmath </gmtmath>`,
:doc:`gravfft </supplements/potential/gravfft>`,
:doc:`gmtgravmag3d </supplements/potential/gmtgravmag3d>`,
:doc:`grdgravmag3d </supplements/potential/grdgravmag3d>`,
:doc:`talwani3d </supplements/potential/talwani3d>`
