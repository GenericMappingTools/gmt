.. index:: ! grdrotater

**********
grdrotater
**********

.. only:: not man

    grdrotater - Finite rotation reconstruction of geographic grid

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt grdrotater** *ingrdfile* |-E|\ *rot_file*\|\ *lon*/*lat*/*angle*
|-G|\ *outgrdfile*
[ |-A|\ *region* ]
[ |-D|\ *rotoutline* ]
[ |-F|\ *polygonfile* ]
[ |-N| ]
[ |SYN_OPT-R| ]
[ |-S| ]
[ |-T|\ *ages* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdrotater** reads a geographical grid and reconstructs it given
total reconstruction rotations. Optionally, the user may supply a
clipping polygon in multiple-segment format; then, only the part of the
grid inside the polygon is used to determine the reconstructed region. The
outlines of the reconstructed region is also returned provided the
rotated region is not the entire globe.

Required Arguments
------------------

*ingrdfile*
    Name of a grid file in geographical (lon, lat) coordinates.

.. _-E:

**-E**\ *rotfile*
    Give file with rotation parameters. This file must contain one
    record for each rotation; each record must be of the following
    format:

    *lon lat tstart [tstop] angle* [ *khat a b c d e f g df* ]

    where *tstart* and *tstop* are in Myr and *lon lat angle* are in
    degrees. *tstart* and *tstop* are the ages of the old and young ends
    of a stage. If *tstop* is not present in the record then a total
    reconstruction rotation is expected and *tstop* is implicitly set to
    0 and should not be specified for any of the records in the file. If
    a covariance matrix **C** for the rotation is available it must be
    specified in a format using the nine optional terms listed in
    brackets. Here, **C** = (*g*/*khat*)\*[ *a b d; b c e; d e f* ]
    which shows **C** made up of three row vectors. If the degrees of
    freedom (*df*) in fitting the rotation is 0 or not given it is set
    to 10000. Blank lines and records whose first column contains # will
    be ignored. You may prepend a leading + to the filename to indicate
    you wish to invert the rotations.
    Alternative 1: Give the filename composed of two plate IDs
    separated by a hyphen (e.g., PAC-MBL) and we will instead extract
    that rotation from the GPlates rotation database. We return an error
    if the rotation cannot be found.
    Alternative 2: Specify *lon*/*lat*/*angle*, i.e., the longitude,
    latitude, and opening angle (all in degrees and separated by /) for
    a single total reconstruction rotation.

.. _-G:

**-G**\ *outgrdfile*
    Name of output grid. This is the grid with the data reconstructed
    according to the specified rotation. If more than one reconstruction
    time is implied then *outgrdfile* must contain a C-format specifier
    to format a floating point number (reconstruction time) to text.

Optional Arguments
------------------

.. _-A:

**-A**\ *region*
    Specify directly the region of the rotated grid.  By default, the
    output grid has a region that exactly matches the extent of the rotated
    domain, but **-A** can be used to crop or extend this region to that
    provided via *region*.

.. _-D:

**-D**\ *rotoutline*
    Name of the grid polygon outline file. This represents the outline
    of the grid reconstructed to the specified time. If more than one reconstruction
    time is implied then *rotoutline* must contain a C-format specifier
    to format a floating point number (reconstruction time) to text.
    If only one time is implied and **-D** is not set then we write the
    polygon to stdout (but see **-N**).

.. _-F:

**-F**\ *polygonfile*
    Specify a multisegment closed polygon file that describes the inside
    area of the grid that should be projected [Default projects entire grid].

.. _-N:

**-N**
    Do Not output the rotated polygon outline [Default will write it to
    stdout, or to a file via **-D**\ ].

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-S**
    Skip the rotation of the grid, just rotate the polygon outline
    (requires **-F** if no grid is provided).

.. _-T:

**-T**\ *ages*
    Sets the desired reconstruction times.  For a single time append
    the desired time.  For an equidistant range of reconstruction times
    give **-T**\ *start*\ /\ *stop*\ /\ *inc*. Append **+n** if *inc* should
    be interpreted to mean *npoints* instead.
    For an non-equidistant set of reconstruction times please pass them
    via the first column in a file, e.g., **-T**\ *agefile*.  If no **-T**
    option is given and **-E** specified a rotation file then we equate
    the rotation file times with the reconstruction times.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: ../../explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-d.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_colon.rst_
.. include:: ../../explain_-n.rst_
.. include:: ../../explain_help.rst_
.. include:: ../../explain_grdresample2.rst_

.. include:: explain_geodetic.rst_

Examples
--------

To rotate the data defined by grid topo.nc and the polygon outline
clip_path.txt, using a total reconstruction rotation with pole at (135.5,
-33.0) and a rotation angle of 37.3 degrees and bicubic interpolation, try

   ::

    gmt grdrotater topo.nc -E135.5/-33/37.3 -V -Fclip_path.txt -Grot_topo.nc > rot_clip_path.txt

To rotate the entire grid faa.nc back to 32 Ma using the rotation file
*rotations.txt* and a bilinear interpolation, try

   ::

    gmt grdrotater faa.nc -Erotations.txt -T32 -V -Grot_faa.nc -nl > rot_faa_path.txt

To just see how the outline of the grid large.nc will plot after the
same rotation, try

   ::

    gmt grdrotater large.nc -Erotations.txt -T32 -V -S \| plot -Rg -JH180/6i -B30 -W0.5p \| gv -

To rotate the grid topo.nc back to 100 Ma using the rotation file
*rotations.txt* and request a reconstruction every 10 Myr, saving
both grids and outlines to filenames that derive from templates, try

   ::

    gmt grdrotater topo.nc -Erotations.txt -T10/100/10 -V -Grot_topo_%g.nc -Drot_topo_path_%g.txt

Let say you have rotated gridA.nc and gridB.nc, restricting each
rotation to nodes inside polygons polyA.txt and polyB.txt, respectively,
using rotation A = (123W,22S,16,4) and rotation B = (108W, 16S, -14.5),
yielding rotated grids rot_gridA.nc and rot_gridB.nc. To determine the
region of overlap between the rotated grids, we use :doc:`grdmath </grdmath>`:

   ::

    gmt grdmath 1 rot_gridA.nc ISNAN SUB 1 rot_gridB.nc ISNAN SUB 2 EQ = overlap.nc

The grid overlap.nc now has 1s in the regions of overlap and 0
elsewhere. You can use it as a mask or use :doc:`grdcontour </grdcontour>` **-D** to extract
a polygon (i.e., a contour).

Notes
-----

GMT distributes the EarthByte rotation model Global_EarthByte_230-0Ma_GK07_AREPS.rot.
To use an alternate rotation file, create an environmental parameters named
**GPLATES_ROTATIONS** that points to an alternate rotation file.

See Also
--------

:doc:`backtracker`,
:doc:`grdcontour </grdcontour>`,
:doc:`gmtpmodeler`,
:doc:`grdmath </grdmath>`,
:doc:`grdpmodeler`,
:doc:`grdspotter`,
:doc:`hotspotter`,
:doc:`originater`,
:doc:`rotconverter`
