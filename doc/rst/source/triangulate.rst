.. index:: ! triangulate
.. include:: module_core_purpose.rst_

***********
triangulate
***********

|triangulate_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt triangulate** [ *table* ]
[ |-A| ]
[ |-C|\ *slpfile* ]
[ |-D|\ **x**\|\ **y** ]
[ |-E|\ *empty* ]
[ |-G|\ *outgrid* ]
[ |SYN_OPT-I| ]
[ |-J|\ *parameters* ]
[ |-L|\ *indexfile*\ [**+b**] ]
[ |-M| ]
[ |-N| ]
[ |-Q|\ [**n**] ]
[ |SYN_OPT-R| ]
[ |-S| ]
[ |-T| ]
[ |SYN_OPT-V| ]
[ |-Z| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**triangulate** reads one or more ASCII [or binary] files (or standard
input) containing *x, y*\ [*, z*] and performs Delaunay triangulation, i.e., it
finds how the points should be connected to give the most equilateral
triangulation possible. If a map projection (give |-R| and |-J|) is
chosen then it is applied before the triangulation is calculated. By
default, the output is triplets of point id numbers that make up each
triangle and is written to standard output. The id numbers refer to the
points position (line number, starting at 0 for the first line) in the
input file. As an option, you may choose to create a multiple segment
file that can be piped through :doc:`plot` to draw the triangulation
network. If |-G| |-I| are set a grid will be calculated based on the
surface defined by the planar triangles. The actual algorithm used in
the triangulations is either that of Watson [1982] or Shewchuk [1996] [Default]
(if installed; type **gmt get GMT_TRIANGULATE** to see which method is
selected). This choice is made during the GMT installation.  Furthermore,
if the Shewchuk algorithm is installed then you can also perform the
calculation of Voronoi polygons and optionally grid your data via the
natural nearest neighbor algorithm.  **Note**: For geographic data with
global or very large extent you should consider :doc:`sphtriangulate`
instead since **triangulate** is a Cartesian or small-geographic area operator
and is unaware of periodic or polar boundary conditions.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

Optional Arguments
------------------

.. _-A:

**-A**
    Compute the area of the Cartesian triangles and append the areas in the
    output segment headers [no areas calculated].  Requires |-S| and is not
    compatible with |-Q|.

.. _-C:

**-C**\ *slpfile*
    Read a slope grid (in degrees) and compute the propagated uncertainty in the
    bathymetry using the CURVE algorithm [Zambo et al, 2016].  Requires the |-G|
    option to specify the output grid.  Note that the *slpgrid* sets the domain
    for the output grid so |-R|, |-I|, [|SYN_OPT-r|\ ] are not required.
    Cannot be used in conjunction with |-D|, |-F|, |-M|, |-N|, |-Q|,
    |-S| and |-T|.

.. _-D:

**-Dx**\|\ **y**
    Take either the *x*- or *y*-derivatives of surface represented by
    the planar facets (only used when |-G| is set).

.. _-E:

**-E**\ *empty*
    Set the value assigned to empty nodes when |-G| is set [NaN].

.. _-G:

.. |Add_outgrid| replace:: Use triangulation to grid the data onto an even grid (specified with
    |-R| and |-I|). Append the name of the output grid file. The
    interpolation is performed in the original coordinates, so if your
    triangles are close to the poles you are better off projecting all
    data to a local coordinate system before using **triangulate** (this
    is true of all gridding routines) or instead select **sphtriangulate**.
    For natural nearest neighbor gridding you must add **-Qn**.
.. include:: /explain_grd_inout.rst_
    :start-after: outgrid-syntax-begins
    :end-before: outgrid-syntax-ends

.. _-I:

.. include:: explain_-I.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-L:

**-L**\ *indexfile*\ [**+b**]
    Give name of file with previously computed Delaunay information. Each record must contain
    triplets of node numbers for a triangle in the input *table* [Default computes these
    using Delaunay triangulation]. If the *indexfile* is binary and can be read the same way
    as the binary input *table* then you can append **+b** to spead up the reading
    [Default reads nodes as ASCII].

.. _-M:

**-M**
    Output triangulation network as multiple line segments separated by
    a segment header record.

.. _-N:

**-N**
    Used in conjunction with |-G| to also write the triplets of the
    ids of all the Delaunay vertices [Default only writes the grid].

.. _-Q:

**-Q**\ [**n**]
    Output the edges of the Voronoi cells instead [Default is Delaunay
    triangle edges]. Requires |-R| and is only available if linked
    with the Shewchuk [1996] library. Note that |-Z| is ignored on
    output. Optionally, append **n** for combining the edges into
    closed Voronoi polygons.

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-S:

**-S**
    Output triangles as polygon segments separated by a segment header
    record. Requires Delaunay triangulation.

.. _-T:

**-T**
    Output edges or polygons even if gridding has been selected with
    the |-G| option [Default will not output the triangulation or
    Voronoi polygons is gridding is selected].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**
    Controls whether we read (*x,y*) or (*x,y,z*) data and if *z* should be
    output when |-M| or |-S| are used [Read (*x,y*) only].

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].  Node ids are stored as double triplets.
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-qi.rst_

.. |Add_nodereg| replace:: (Only valid with |-G|).
.. include:: explain_nodereg.rst_

.. include:: explain_-s.rst_

.. include:: explain_-w.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

.. include:: explain_float.rst_

.. include:: explain_inside.rst_

Examples
--------

.. include:: explain_example.rst_

To triangulate the points in the file samples.xyz, store the triangle
information in a binary file, and make a grid for the given area and spacing, use

::

  gmt triangulate samples.xyz -bo -R0/30/0/30 -I2 -Gsurf.nc > samples.ijk

To draw the optimal Delaunay triangulation network based on the same
file using a 15-cm-wide Mercator map, use

::

  gmt triangulate samples.xyz -M -R-100/-90/30/34 -JM15c | gmt plot -R-100/-90/30/34 -JM15c -W0.5p -B1 -pdf network

To instead plot the Voronoi cell outlines, try

::

  gmt triangulate samples.xyz -M -Q -R-100/-90/30/34 -JM15c | gmt plot -R-100/-90/30/34 -JM15c -W0.5p -B1 -pdf cells

To combine the Voronoi outlines into polygons and paint them
according to their ID, try

::

  gmt triangulate samples.xyz -M -Qn -R-100/-90/30/34 -JM15c | \
      gmt plot -R-100/-90/30/34 -JM15c -W0.5p+cf -B1 -Ccolors.cpt -pdf polygons

To grid the data using the natural nearest neighbor algorithm, try

::

  gmt triangulate samples.xyz -Gnnn.nc -Qn -R-100/-90/30/34 -I0.5

Notes
-----

The uncertainty propagation for bathymetric grids requires both horizontal
and vertical uncertainties and these are weighted given the local slope.
See the *Zambo et al.* [2014] and *Zhou and Liu* [2004] references for more details.


See Also
--------

:doc:`gmt`,
:doc:`greenspline`,
:doc:`nearneighbor`,
:doc:`contour`,
:doc:`sphdistance`,
:doc:`sphinterpolate`,
:doc:`sphtriangulate`,
:doc:`surface`

References
----------

Shewchuk, J. R., 1996, Triangle: Engineering a 2D Quality Mesh Generator
and Delaunay Triangulator, First Workshop on Applied Computational
Geometry (Philadelphia, PA), 124-133, ACM, May 1996.

Watson, D. F., 1982, Acord: Automatic contouring of raw data, *Comp. &
Geosci.*, **8**, 97-101.

Zambo, S., Elmore, P. A., Bourgeois, B. S., and Perkins, A. L., 2016,
Uncertainty estimation for sparse data gridding algorithms,
Proceedings of the U.S. Hydro Conference,National Harbor, MD, 16-19 March 2015.

Zhou, Q., and Liu, X., 2004, Error analysis on grid-based slope and aspect
algorithms, *Photogrammetric Eng. & Remote Sensing*, **70** (8), 957-962.

`Shewchuk's Homepage <http://www.cs.cmu.edu/~quake/triangle.html>`_
