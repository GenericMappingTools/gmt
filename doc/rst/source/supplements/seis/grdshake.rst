.. index:: ! grdshake
.. include:: ../module_supplements_purpose.rst_

********
grdshake
********

|grdshake_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt grdshake** *ingrid* |-G|\ *outgrid*
|-L|\ *fault.dat*
|-M|\ *mag*
[ |-C|\ *a,v,i* ]
[ |-F|\ *mecatype* ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

Takes Vs30 velocity grid and compute the Peak Ground Acceleration/Velocity and Intensity 


Required Arguments
------------------

*ingrid*
    This is the input grid file.

.. _-G:

**-G**\ *outgrid*
    This is the output grid file. If more than one component is set via |-C| then <outgrid> must contain %%s to format component code.

.. _-L:

**-L**\ *fault.dat*
    Name of a file with the coordinates of the fault trace.

.. _-M:

**-M**\ *mag*
    Select the magnitude of the event.


Optional Arguments
------------------

.. _-C:

**-C**\ *a,v,i*
    List of comma-separated components to be written as grids (requires |-G|). Choose from, *a*\ (cceleration),
    *v*\ (elocity), *i*\ (ntensity) [Default is *i*].

.. _-F:

**-F**\ *1*\|\ *2*\|\ *3*\|\ *4*
    Select the focal mechanism type (e.g. **-F**\1 or **-F**\2 ...)
       - 1 unknown [Default].
       - 2 strike-slip.
       - 3 normal.
       - 4 thrust.

.. _-R:

.. |Add_-R| replace:: This defines the subregion to be operated out.
.. include:: ../../explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_


Examples
--------

To compute the intensity grid using the previously computed Vs30 velocities (*vs30.grd*) of an
event with magnitude 7 occurred along a fault whose trace is coordinates are provide in the
*line.dat* file, do::

    gmt grdshake vs30.grd -Gshake_intensity.grd -Lline.dat -Ci -M7 -V


See Also
--------

:doc:`grdvs30`,
:doc:`gmt </gmt>`
