*********
grdvector
*********

grdvector - Plot vector field from two component grids

`Synopsis <#toc1>`_
-------------------

**grdvector** *compx.nc* *compy.nc* **-J**\ *parameters* [ **-A** ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *cptfile* ] [
**-G**\ *fill* ] [
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
] [ **-K** ] [ **-N** ] [ **-O** ] [ **-P** ] [ **-Q**\ *parameters* ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-S**\ [**l**\ ]\ *scale* ] [ **-T** ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [ **-W**\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-Z** ] [ **-c**\ *copies* ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**grdvector** reads two 2-D grid files which represents the x- and
y-components of a vector field and produces a vector field plot by
drawing vectors with orientation and length according to the information
in the files. Alternatively, polar coordinate components may be used (r,
theta). **grdvector** is basically a short-hand for using 2 calls to
**grd2xyz** and pasting the output through **psxy** **-SV**. 

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

*compx.nc*
    Contains the x-component of the vector field.
*compy.nc*
    Contains the y-component of the vector field. (See GRID FILE FORMATS below.) 

.. include:: explain_-J.rst_

`Optional Arguments <#toc5>`_
-----------------------------

**-A**
    Means grid files have polar (r, theta) components instead of
    Cartesian (x, y). 

.. include:: explain_-B.rst_

**-C**\ *cptfile*
    Use *cptfile* to assign colors based on vector length.
**-G**\ *fill*
    Sets color or shade for vector interiors [Default is no fill].
**-I**
    Only plot vectors at nodes every *x\_inc*, *y\_inc* apart (must be
    multiples of original grid spacing). Append **m** for arc minutes or
    **s** for arc seconds. [Default plots every node]. 

.. include:: explain_-K.rst_

**-N**
    Do NOT clip vectors at map boundaries [Default will clip]. 

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-Q**\ *parameters*
    Modify vector parameters. For vector heads, append vector head
    *size* [Default is 0, i.e., stick-plot]. See VECTOR ATTRIBUTES for
    specifying additional attributes. 

.. |Add_-R| replace:: Specify a subset of the grid.
.. include:: explain_-R.rst_

**-S**\ [**l**\ ]\ *scale*
    Sets scale for vector length in data units per distance measurement
    unit [1]. Append **c**, **i**, or **p** to indicate the measurement
    unit (cm, inch,or point). Prepend **l** to indicate a fixed length
    for all vectors.
**-T**
    Means azimuth should be converted to angles based on the selected
    map projection. 

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x0C .. just an invisible code
.. include:: explain_-V.rst_

**-W**\ *pen*
    Set pen attributes used for vector outlines [Default: width =
    default, color = black, style = solid]. 

.. include:: explain_-XY.rst_

**-Z**
    Means the angles provided are azimuths rather than direction
    (requires **-A**). 

.. include:: explain_-c.rst_

.. |Add_-f| unicode:: 0x0C .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_perspective| unicode:: 0x0C .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_input.rst_

.. include:: explain_vectors.rst_


`Examples <#toc8>`_
-------------------

To draw the vector field given by the files r.nc and theta.nc on a
linear plot with scale 5 cm per data unit, using vector rather than
stick plot, scale vector magnitudes so that 10 units equal 1 inch, and
center vectors on the node locations, run

grdvector r.nc theta.nc **-Jx**\ 5\ **c** -A -Q0.1i+e+jc
**-S**\ 10\ **i** > gradient.ps

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*grdcontour*\ (1) <grdcontour.html>`_ , `*psxy*\ (1) <psxy.html>`_
