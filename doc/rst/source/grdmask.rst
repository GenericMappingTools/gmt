*******
grdmask
*******

grdmask - Create mask grid from polygons or point coverage

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**grdmask** *pathfiles* **-G**\ *mask\_grd\_file*]
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [
**-A**\ [**m**\ \|\ **p**] ] [
**-N**\ [**z**\ \|\ **Z**\ \|\ **p**\ \|\ **P**]\ *values* ] [
**-S**\ *search\_radius*\ [*unit*\ ] ] [ **-V**\ [*level*\ ] ] [
**-bi**\ [*ncols*\ ][*type*\ ] ] [ **-f**\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**grdmask** can operate in two different modes. 1. It reads one or more
xy-files that each define a closed polygon. The nodes defined by the
specified region and lattice spacing will be set equal to one of three
possible values depending on whether the node is outside, on the polygon
perimeter, or inside the polygon. The resulting mask may be used in
subsequent operations involving **grdmath** to mask out data from
polygonal areas. 2. The xy-files simply represent data point locations
and the mask is set to the inside or outside value depending on whether
a node is within a maximum distance from the nearest data point. If the
distance specified is zero then only the nodes nearest each data point
are considered "inside". 

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

*pathfiles*
    The name of 1 or more ASCII [or binary, see
    **-bi**\ [*ncols*\ ][*type*\ ]] files holding the polygon(s) or data points.
**-G**\ *mask\_grd\_file*]
    Name of resulting output mask grid file. (See GRID FILE FORMATS below). 

.. include:: explain_-I.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [**m**\ \|\ **p**]
    If the input data are geographic (as indicated by **-fi**) then the
    sides in the polygons will be approximated by great circle arcs.
    When using the **-A** sides will be regarded as straight lines.
    Alternatively, append **m** to have sides first follow meridians,
    then parallels. Or append **p** to first follow parallels, then
    meridians.
**-N**\ [**z**\ \|\ **Z**\ \|\ **p**\ \|\ **P**]\ *values*
    Sets the *out/edge/in* that will be assigned to nodes that are
    *out*\ side the polygons, on the *edge*, or *in*\ side. Values can
    be any number, including the textstring NaN [Default is 0/0/1].
    Optionally, use **Nz** to set polygon insides to the z-value
    obtained from the data (either segment header **-Z**\ *zval*,
    **-L**\ *header* or via **-a**\ Z=\ *name*); use **-NZ** to consider
    the polygon boundary as part of the inside. Alternatively, use
    **-Np** to use a running number as polygon ID; optionally append
    start of the sequence [0]. Here, **-NP** includes the polygon
    perimeter as inside. Note:
    **-N**\ **z**\ \|\ **Z**\ \|\ **p**\ \|\ **P** cannot be used in
    conjunction with **-S**; they also all optionally accept /*out* [0].
**-S**\ *search\_radius*\ [*unit*\ ]
    Set nodes to inside, on edge, or outside depending on their distance
    to the nearest data point. Nodes within *radius* [0] from the
    nearest data point are considered inside; append a distance unit
    (see UNITS). If *radius* is given as **z** then we instead read
    individual radii from the 3rd input column. If **-S** is not set
    then we consider the input data to define closed polygon(s) instead.
 
.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_grd_output.rst_

.. include:: explain_grd_coord.rst_

`Examples <#toc9>`_
-------------------

To set all nodes inside and on the polygons coastline\_\*.xy to 0, and
outside points to 1, do

    grdmask coastline\_\*.xy -R-60/-40/-40/-30 -I5m -N1/0/0 -Gland\_mask.nc=nb -V

To set nodes within 50 km of data points to 1 and other nodes to NaN, do

    grdmask data.xyz -R-60/-40/-40/-30 -I5m -NNaN/1/1 -S50k -Gdata\_mask.nc=nb -V

To assign polygon IDs to the gridnodes using the insides of the polygons
in plates.gmt, based on the attribute POL\_ID, do

    grdmask plates.gmt -R-40/40/-40/40 -I2m -Nz -Gplate\_IDs.nc -aZ=POL\_ID -V

Same exercise, but instead compute running polygon IDs starting at 100, do

    grdmask plates.gmt -R-40/40/-40/40 -I2m -Np100 -Gplate\_IDs.nc -V

`See Also <#toc10>`_
--------------------

`gmt <gmt.html>`_, `grdlandmask <grdlandmask.html>`_,
`grdmath <grdmath.html>`_, `grdclip <grdclip.html>`_,
`psmask <psmask.html>`_, `psclip <psclip.html>`_
