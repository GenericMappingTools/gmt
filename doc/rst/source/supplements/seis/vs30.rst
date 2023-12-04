.. index:: ! grdvs30
.. include:: ../module_supplements_purpose.rst_

*******
grdvs30
*******

|vs30_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt grdvs30** *ingrid* |-G|\ *outgrid*
|-C|\ [**-C**\ *val*\|\ *fname*[**+g**]]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-W|\ [**-W**\ *water_vel*] ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

Takes one topographic grid in geographical coordinates and a constant craton value or a craton file
which are taken as weights ranging from 1 (one) on stable shields (craton) and 0 in active tectonic regions
-- values in between will be computed as the weighted average of the craton and tectonic models.

An optional argument *water_vel* is the value that water-covered areas will be set to; the default is 600.


Required Arguments
------------------

*ingrid*
    This is the input grid file.

.. _-C:

**-C**\ *val*\|\ *fname*[**+g**]
    Argument *val* can be one of these three:
       - A value *val* between 0 and 1, where 0 means a stable Craton and 1 an Active region.
       - The name of a multi-segment file with the *cratons* polygons. In this case the polygons will be
         feed to grdmask to compute a cratons/active tectonic mask. Use **-C**\ *@cratons.xy* to download
         a cratons file from the GMT server.
       - The name of a grid with the cratons/active tectonic regions. In this case the **+g** suffix
         is mandatory to indicate that we are reading a grid.

.. _-G:

**-G**\ *outgrid*
    This is the output grid file.


Optional Arguments
------------------

.. _-R:

.. |Add_-R| replace:: This defines the subregion to be operated out.
.. include:: ../../explain_-R.rst_

.. _-W:

**-W**\ *water_vel*
    *water_vel* sets the Vs30 value used in areas designated as water in the landmask [default=600]

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-r.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_


Examples
--------

To compute a Vs30 estimate of the *topo.grd* grid and a craton value of 0, do::

    gmt grdvs30 topo.grd -C0 -Gvs30.grd -V


Reference
---------

- https://github.com/usgs/earthquake-global_vs30/blob/master/src/grad2vs30.c

See Also
--------

:doc:`grdshake`,
:doc:`gmt </gmt>`
