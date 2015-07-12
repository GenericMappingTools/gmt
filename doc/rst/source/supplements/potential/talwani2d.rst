.. index:: ! talwani2d

*********
talwani2d
*********

.. only:: not man

    talwani2d - Compute free-air anomalies or vertical gravity gradients over 2-D bodies

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**talwani2d** [ *modeltable* ]
[ **-A** ] [ **-D**\ *rho* ] ]
[ **-F**\ **f**\ \|\ **n**\ \|\ **v** ] 
[ **-M**\ [**h**]\ [**v**] ] [ **-N**\ *trackfile* ]
[ **-Z**\ *level*\ [*ymin*\ /*ymax*\ ] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ **-fg** ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-x| ]

|No-spaces|

Description
-----------

**talwani2d** will read the multi-segment *modeltable* from file or standard input.
This file contains crossections of one or more 2-D bodies, with one polygon
per segment.  The segment header must contain the parameter *rho*, which
states the the density of this body (individual body
densities may be overridden by a fixed density contrast given via **-D**).
We can compute anomalies on an equidistant lattice (by specifying a lattice with
**-T** or provide arbitrary
output points specified in a file via **-N**.  Chose from free-air anomalies, vertical
gravity gradient anomalies, or geoid anomalies.  Options are available to control axes units and direction.


Required Arguments
------------------

*modeltable*
    The file describing the bodies.

Optional Arguments
------------------

**-A**
    The *z*-axis should be positive upwards [Default is down].

**-D**\ *unit*
    Sets fixed density contrast that overrides any setting in model file, in kg/m^3.

**-F**\ **f**\ \|\ **n**\ \|\ **v**
    Specify desired geopotential field component.  Choose between **f** (free-air anomaly) [Default],
    **n** (geoid) or **v** (vertical gravity gradient).

**-M**\ [**h**]\ [**v**]
    Sets units used.  Append **h** to indicate horizontal distances are in km [m],
    and append **z** to indicate vertical distances are in km [m].

**-N**\ *trackfile*
    Specifies locations where we wish to compute the predicted value.  When this option
    is used there are no grids and the output data records are written to stdout.

**-Z**\ *level*\ [*ymin*\ /*ymax*\ ]
    Set observation level as a constant [0].  Optionally, and for gravity anomalies only,
    append the finite extent limits of a 2.5-D body.

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: ../../explain_-bi.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-ocols.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: ../../explain_-V.rst_

.. include:: ../../explain_core.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_distunits.rst_


Examples
--------

To compute the free-air anomalies on a grid over a 2-D body that has been contoured
and saved to body.txt, using 1.7 g/cm^3 as the density contrast, try

::

    gmt talwani2d -R-200/200/2 body.txt -D1700 -Fg > 2dgrav.txt

To obtain the vertical gravity gradient anomaly along the track in crossing.txt
for the same model, try

::

    gmt talwani2d -Ncrossing.txt body.txt -D1700 -Fv > vgg_crossing.txt


See Also
--------

:doc:`gmt.conf </gmt.conf>`, :doc:`gmt </gmt>`,
:doc:`grdmath </grdmath>`, :doc:`gravfft </supplements/potential/gravfft>`,
:doc:`gmtgravmag3d </supplements/potential/gmtgravmag3d>`,
:doc:`grdgravmag3d </supplements/potential/grdgravmag3d>`,
:doc:`talwani3d </supplements/potential/talwani3d>`
