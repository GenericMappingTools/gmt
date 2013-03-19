**************
nearneighbor
**************

nearneighbor - Grid table data using a "Nearest neighbor" algorithm

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**nearneighbor** [ *table* ] **-G**\ *out\_grdfile*
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-N**\ *sectors*\ [/*min\_sectors*]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ]
**-S**\ *search\_radius*\ [*unit*\ ] [ **-E**\ *empty* ] [
**-V**\ [*level*\ ] ] [ **-W** ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [
**-f**\ *colinfo* ] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-n**\ [**+b**\ *BC*] ] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**nearneighbor** reads arbitrarily located (x,y,z[,w]) triples
[quadruplets] from standard input [or *table*] and uses a nearest
neighbor algorithm to assign an average value to each node that have one
or more points within a radius centered on the node. The average value
is computed as a weighted mean of the nearest point from each sector
inside the search radius. The weighting function used is w(r) = 1 / (1 +
d ^ 2), where d = 3 \* r / search\_radius and r is distance from the
node. This weight is modulated by the observation points’ weights [if
supplied]. 

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

**-G**\ *out\_grdfile*
    Give the name of the output grid file. 

.. include:: explain_-I.rst_

**-N**\ *sectors*\ [/*min\_sectors*]
    The circular area centered on each node is divided into *sectors*
    sectors. Average values will only be computed if there is at least
    one value inside at least *min\_sectors* of the sectors for a given
    node. Nodes that fail this test are assigned the value NaN (but see
    **-E**). If *min\_sectors* is omitted, each sector needs to have at
    least one value inside it. [Default is quadrant search with 50%
    coverage, i.e., *sectors* = 4 and *min\_sectors* = 2]. Note that
    only the nearest value per sector enters into the averaging, not all
    values inside the circle. 

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

**-S**\ *search\_radius*\ [*unit*]
    Sets the *search\_radius* that determines which data points are
    considered close to a node. Append the distance unit (see UNITS).

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    3 [or 4, see **-W**] column ASCII file(s) [or binary, see
    **-bi**\ [*ncols*\ ][*type*\ ]] holding (x,y,z[,w]) data values. If
    no file is specified, **nearneighbor** will read from standard
    input.
**-E**\ *empty*
    Set the value assigned to empty nodes [NaN]. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-W**
    Input data have a 4th column containing observation point weights.
    These are multiplied with the geometrical weight factor to determine
    the actual weights used in the calculations. 

.. |Add_-bi| replace:: [Default is 3 (or 4 if **-W** is set) columns]. 
.. include:: explain_-bi.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
    Append **+b**\ *BC* to set any boundary conditions to be used,
    adding **g** for geographic, **p** for periodic, or **n** for
    natural boundary conditions. For the latter two you may append **x**
    or **y** to specify just one direction, otherwise both are assumed.
    [Default is geographic if grid is geographic]. 

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_float.rst_

`Examples <#toc8>`_
-------------------

To create a gridded data set from the file seaMARCII\_bathy.lon\_lat\_z
using a 0.5 min grid, a 5 km search radius, using an octant search, and
set empty nodes to -9999:

    nearneighbor seaMARCII\_bathy.lon\_lat\_z -R242/244/-22/-20 -I0.5m
    -E-9999 -Gbathymetry.nc -S5k -N8

To make a global grid file from the data in geoid.xyz using a 1 degree
grid, a 200 km search radius, spherical distances, using an quadrant
search, and set nodes to NaN only when fewer than two quadrants contain
at least one value:

    nearneighbor geoid.xyz -R0/360/-90/90 -I1 -Lg -Ggeoid.nc -S200k -N4/2

`See Also <#toc9>`_
-------------------

`blockmean <blockmean.html>`_,
`blockmedian <blockmedian.html>`_,
`blockmode <blockmode.html>`_, `gmt <gmt.html>`_,
`surface <surface.html>`_,
`triangulate <triangulate.html>`_
