.. index:: ! gmtselect

******
select
******

.. only:: not man

    Select data table subsets based on multiple spatial criteria

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt select** [ *table* ]
[ |-A|\ *min_area*\ [/*min_level*/*max_level*][\ **+ag**\ \|\ **i**\ \|\ **s**\ \|\ **S**][**+r**\ \|\ **l**][**p**\ *percent*] ]
[ |-C|\ *pointfile*\ **+d**\ *dist*\ [*unit*] ]
[ |-D|\ *resolution*\ [**+f**] ]
[ |-E|\ [**fn**] ]
[ |-F|\ *polygonfile* ]
[ |-G|\ *gridmask* ]
[ |-I|\ [**cfglrsz**] ]
[ |-J|\ *parameters* ]
[ |-L|\ *linefile*\ **+d**\ *dist*\ [*unit*]\ [**+p**] ]
[ |-N|\ *maskvalues* ]
[ |SYN_OPT-R| ]
[ |-Z|\ *min*\ [/*max*]\ [**+a**]\ [**+c**\ *col*]\ [**+i**] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**select** is a filter that reads (x, y) or (longitude, latitude) positions from the first 2 columns of *infiles*
[or standard input] and uses a combination of 1-7 criteria to pass or reject the records. Records can be
selected based on whether or not they are 1) inside a rectangular region (**-R** [and **-J**]), 2) within
*dist* km of any point in *pointfile*, 3) within *dist* km of any line in *linefile*, 4) inside one of the
polygons in the *polygonfile*, 5) inside geographical features (based on coastlines), 6) has z-values
within a given range, or 7) inside bins of a grid mask whose nodes are non-zero. The sense of the tests can
be reversed for each of these 6 criteria by using the **-I** option. See option **-:** on how to read
(y,x) or (latitude,longitude) files.  Note: If no projection information is used then you must supply **-fg**
to tell **select** that your data are geographical.

Required Arguments
------------------

None

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

.. |Add_-A| replace:: Ignored unless **-N** is set.
.. include:: explain_-A.rst_

.. _-C:

**-C**\ *pointfile*\ **+d**\ *dist*\ [*unit*]
    Pass all records whose location is within *dist* of any of the
    points in the ASCII file *pointfile*. If *dist* is zero then the 3rd
    column of *pointfile* must have each point's individual radius of
    influence. Distances are Cartesian and in user units; specify
    **-fg** to indicate spherical distances and append a distance unit
    (see :ref:`Unit_attributes`). Alternatively, if **-R** and **-J** are used then
    geographic coordinates are projected to map coordinates (in cm,
    inch, or points, as determined by :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>`) before
    Cartesian distances are compared to *dist*.

.. _-D:

**-D**\ *resolution*\ [**+f**]
    Ignored unless **-N** is set. Selects the resolution of the
    coastline data set to use ((**f**)ull, (**h**)igh,
    (**i**)ntermediate, (**l**)ow, or (**c**)rude). The resolution drops
    off by ~80% between data sets. [Default is **l**]. Append (**+f**) to
    automatically select a lower resolution should the one requested not
    be available [abort if not found]. Note that because the coastlines
    differ in details it is not guaranteed that a point will remain
    inside [or outside] when a different resolution is selected.

.. _-E:

**-E**\ [**fn**]
    Specify how points exactly on a polygon boundary should be
    considered. By default, such points are considered to be inside the
    polygon. Append **f** and/or **n** to change this behavior for the
    **-F** and/or **-N** options, respectively, so that boundary points are
    considered to be outside.

.. _-F:

**-F**\ *polygonfile*
    Pass all records whose location is within one of the closed polygons
    in the multiple-segment file *polygonfile*. For spherical polygons
    (lon, lat), make sure no consecutive points are separated by 180
    degrees or more in longitude. Note that *polygonfile* must be in
    ASCII regardless of whether **-bi** is used.

.. _-G:

**-G**\ *gridmask*
    Pass all locations that are inside the valid data area of the grid *gridmask*.
	Nodes that are outside are either NaN or zero.

.. _-I:

**-I**\ [**cflrsz**]
    Reverses the sense of the test for each of the criteria specified:

    **c** select records NOT inside any point's circle of influence.

    **f** select records NOT inside any of the polygons.

    **g** will pass records inside the cells with z equal zero of the grid mask in **-G**.

    **l** select records NOT within the specified distance of any line.

    **r** select records NOT inside the specified rectangular region.

    **s** select records NOT considered inside as specified by **-N**
    (and **-A**, **-D**).

    **z** select records NOT within the range specified by **-Z**.

.. _-J:
 
.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. _-L:

**-L**\ *linefile*\ **+d**\ *dist*\ [*unit*]\ [**+p**]
    Pass all records whose location is within *dist* of any of the line
    segments in the ASCII multiple-segment file *linefile*. If *dist* is
    zero then we will scan each sub-header in the *linefile* for an
    embedded **-D**\ *dist* setting that sets each line's individual
    distance value. Distances are Cartesian and in user units; specify
    **-fg** to indicate spherical distances append a distance unit (see
    :ref:`Unit_attributes`). Alternatively, if **-R** and **-J** are used then geographic
    coordinates are projected to map coordinates (in cm, inch, m, or
    points, as determined by :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>`) before Cartesian
    distances are compared to *dist*. Append **+p** to ensure only points
    whose orthogonal projections onto the nearest line-segment fall
    within the segments endpoints [Default considers points "beyond" the
    line's endpoints.

.. _-N:

**-N**\ *maskvalues*
    Pass all records whose location is inside specified geographical
    features. Specify if records should be skipped (s) or kept (k) using
    1 of 2 formats:

    **-N**\ *wet/dry*.

    **-N**\ *ocean/land/lake/island/pond*.

    [Default is s/k/s/k/s (i.e., s/k), which passes all points on dry land]. 

.. _-R:

.. |Add_-R| replace:: If no map projection is supplied we implicitly set **-Jx**\ 1. 
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-Z:

**-Z**\ *min*\ [/*max*]\ [**+a**]\ [**+c**\ *col*]\ [**+i**]
    Pass all records whose 3rd column (*z*; *col* = 2) lies within the given range
    or is NaN (use **-s** to skip NaN records).
    If *max* is omitted then we test if *z* equals *min* instead.  This means
    equality within 5 ULPs (unit of least precision; http://en.wikipedia.org/wiki/Unit_in_the_last_place).
    Input file must have at least three columns. To indicate no limit on
    min or max, specify a hyphen (-). If your 3rd column is absolute
    time then remember to supply **-f**\ 2T. To specify another column, append
    **+c**\ *col*, and to specify several tests just repeat the **Z** option as
    many times has you have columns to test. Note: when more than one **Z** option
    is given then the **Iz** option cannot be used.  In the case of multiple tests
    you may use these modifiers as well: **a** passes any record that passes at least
    one of your *z* tests [all tests must pass], and **i** reverses the tests to pass
    record with *z* value NOT in the given range.  Finally, if **+c** is not used
    then it is automatically incremented for each new **-Z** option, starting with 2.

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-s.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

This note applies to ASCII output only in combination with binary or
netCDF input or the **-:** option. See also the note below.

Note On Processing ASCII Input Records
--------------------------------------

Unless you are using the **-:** option, selected ASCII input records are
copied verbatim to output. That means that options like **-foT** and
settings like :ref:`FORMAT_FLOAT_OUT <FORMAT_FLOAT_OUT>` and :ref:`FORMAT_GEO_OUT <FORMAT_GEO_OUT>` will not
have any effect on the output. On the other hand, it allows selecting
records with diverse content, including character strings, quoted or
not, comments, and other non-numerical content.

Note On Distances
-----------------

If options **-C** or **-L** are selected then distances are Cartesian
and in user units; use **-fg** to imply spherical distances in km and
geographical (lon, lat) coordinates. Alternatively, specify **-R** and
**-J** to measure projected Cartesian distances in map units (cm, inch,
or points, as determined by :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>`).

This program has evolved over the years. Originally, the **-R** and
**-J** were mandatory in order to handle geographic data, but now there
is full support for spherical calculations. Thus, **-J** should only be
used if you want the tests to be applied on projected data and not the
original coordinates. If **-J** is used the distances given via **-C**
and **-L** are projected distances.

Note On Segments
----------------

Segment headers in the input files are copied to output if one or more
records from a segment passes the test. Selection is always done point
by point, not by segment.  That means only points from a segment that
pass the test will be included in the output.  If you wish to clip the lines
and include the new boundary points at the segment ends you must use
:doc:`gmtspatial` instead.

Examples
--------

To extract the subset of data set that is within 300 km of any of the
points in pts.txt but more than 100 km away from the lines in lines.txt, run

   ::

    gmt select lonlatfile -fg -Cpts.txt+d300k -Llines.txt+d100k -Il > subset.txt

Here, you must specify **-fg** so the program knows you are processing
geographical data.

To keep all points in data.txt within the specified region, except the
points on land (as determined by the high-resolution coastlines), use

   ::

    gmt select data.txt -R120/121/22/24 -Dh -Nk/s > subset.txt

To return all points in quakes.txt that are inside or on the spherical
polygon lonlatpath.txt, try

   ::

    gmt select quakes.txt -Flonlatpath.txt -fg > subset1.txt

To return all points in stations.txt that are within 5 cm of the point in
origin.txt for a certain projection, try

   ::

    gmt select stations.txt -Corigin.txt+d5 -R20/50/-10/20 -JM20c \
    --PROJ_LENGTH_UNIT=cm > subset2.txt

To return all points in quakes.txt that are inside the grid topo.nc
where the values are nonzero, try

   ::

    gmt select quakes.txt -Gtopo.nc > subset2.txt

The pass all records whose 3rd column values fall in the range 10-50
and 5th column values are all negative, try

   ::

    gmt select dataset.txt -Z10/50 -Z-/0+c4 > subset3.txt


.. include:: explain_gshhs.rst_

.. include:: explain_inside.rst_


See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`gmtconvert`,
:doc:`gmtsimplify`,
:doc:`gmtspatial`,
:doc:`grdlandmask`,
:doc:`coast`
