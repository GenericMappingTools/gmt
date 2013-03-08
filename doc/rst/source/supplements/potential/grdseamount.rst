***********
grdseamount
***********

grdseamount - Compute synthetic seamount (Gaussian or cone, circular or elliptical) bathymetry

`Synopsis <#toc1>`_
-------------------

.. include:: ../../common_SYN_OPTs.rst_

**grdseamount** [ *intable* ] **-R**\ *west*/*east*/*south*/*north*
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
[ **-A**\ [*out/in*\ ] ] [ **-C** ] [ **-G**\ *grdfile* ] [ **-L**\ [*cut*] ]
[ **-N**\ *norm* ] [ **-S**\ *scale* ] [ **-T**\ [*flattening*] ] [ **-Z**\ *level* ]
[ **-V**\ [*level*\ ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [ **-fg** ]
[ **-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] [ **r** ] 

`Description <#toc2>`_
----------------------

**grdseamount** will compute the bathymetry for synthetic seamounts given their shape
parameters.  We read a list with seamount locations and sizes and can evaluate either
Gaussian or conical shapes, which may be circular or elliptical, and optionally truncated.
Various scaling options are available to modify the result, including an option to add in
a background depth.  The input must contain *lon, lat, radius, height* for each seamount.
For elliptical features (**-E**) we expect *lon, lat, azimuth, semi-major, semi-minor,
radius, height* instead.  If flattening is specified (**TT**) then a final column with
flattening is expected.

..  include:: ../../explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

.. include:: ../../explain_-I.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [*out/in*\ ]
    Build a mask grid, append outside/inside values [1/NaN].
    Here, height is ignored and **-L**, **-N** and **-Z** are disallowed
**-C**
    Cone model [Default is Gaussian].
**-G**\ *outgrid*
    Sets name of output gridfile. 
**-L**\ [*cut*\ ]
    List area, volume, and mean height for each seamount; NO grid is created.
    Optionally, append the noise-floor cutoff level [0].
**-N**\ *norm*
    Normalize grid so maximum grid height equals *norm*.
**-S**\ *scale*
    Sets optional scale factor for radii [1].
**-T**\ [*flattening*\ ]
    Seamounts are to be truncated.  Append *flattening*, otherwise we expect
    it in last input column [no truncation].
**-Z**\ *level*
    Add in background depth [0].

.. |Add_-bi| replace:: [Default is 3 input columns]. This option only applies
    to xyz input files; see **-Z** for z tables. 
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


`Examples <#toc9>`_
-------------------

To be added.

`See Also <#toc10>`_
--------------------

`gmt.conf <../../gmt.conf.html>`_, `gmt <../../gmt.html>`_,
`grdmath <../../grdmath.html>`_, `gravfft <gravfft.html>`_
