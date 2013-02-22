************
grdgravmag3d
************

grdgravmag3d - Compute the gravity effect of a grid by the method of
Okabe

`Synopsis <#toc1>`_
-------------------

**grdgravmag3d** [ **-C**\ *density* ] [ **-D** ] [ **-F**\ *xy\_file* ]
[ **-G**\ *outputgrid.nc* ] [ **-L**\ *z\_observation* ] [ **-M** ]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [ **-S**\ *radius* ] [
**-Z**\ *level* ] [ **-V**\ [*level*\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

`Description <#toc2>`_
----------------------

**grdgravmag3d** will compute the gravity anomaly of a body described by
one or (optionally) two grids The output can either be along a given set
of xy locations or on a grid. This method is not particularly fast but
allows computing the anomaly of arbitrarily complex shapes.

.. include:: ../../explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

**-C**\ *density*
    Sets body density in SI. This option is mutually exclusive with
    **-H**.
**-F**\ *xy\_file*
    Provide locations where the anomaly will be computed. Note this
    option is mutually exlusive with **-G**.
**-G**\ *outgrid.nc*
    Output the gravity anomaly at nodes of this grid file.

.. |Add_-R| unicode:: 0x0C .. just an invisible code
.. include:: ../../explain_-R.rst_

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_-V| unicode:: 0x0C .. just an invisible code
.. include:: ../../explain_-V.rst_

**-L**
    sets level of observation [Default = 0]. That is the height (z) at
    which anomalies are computed.
**-Z**
    level of reference plane [Default = 0]. Use this option when the
    triangles describe a non-closed surface and the volume is deffined
    from each triangle and this reference level. An example will be the
    whater depth to compute a Bouguer anomaly.

.. |Add_-f| unicode:: 0x0C .. just an invisible code
.. include:: ../../explain_-f.rst_

.. include:: ../../explain_help.rst_

`Examples <#toc6>`_
-------------------

Suppose you want to compute the gravity effect of the phantom "Sandy
Island" together with its not phantom seamount

grdgravmag3d sandy\_bat.grd -C1700 -Z-4300 -M -I1m -Gsandy\_okb.grd -V

`See Also <#toc7>`_
-------------------

`GMT <GMT.html>`_ , `gmtgravmag3d <gmtgravmag3d.html>`_

`Reference <#toc8>`_
--------------------

Okabe, M., Analytical expressions for gravity anomalies due to
polyhedral bodies and translation into magnetic anomalies, *Geophysics*,
44, (1979), p 730-741.
