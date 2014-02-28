.. index:: ! grdrotater

**********
grdrotater
**********

.. only:: not man

    grdrotater - Finite rotation reconstruction of geographic grid

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**grdrotate** *ingrdfile* **-E**\ *rotfile* \|
**-e**\ *lon*/*lat*/*angle* **-G**\ *outgrdfile* [ **-F**\ *polygonfile* ]
[ **-N** ]
[ |SYN_OPT-R| ]
[ **-S** ] [ **-T**\ *age* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**grdrotater** reads a geographical grid and reconstructs it given a
total reconstruction rotation. Optionally, the user may supply a
clipping polygon in multiple-segment format; then, only the part of the
grid inside the polygon is used to determine the return grid region. The
outline of the projected region is returned on stdout provided the
rotated region is not the entire globe.

Required Arguments
------------------

*ingrdfile*
    Name of a grid file in geographical (lon, lat) coordinates.
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
    Alternatively, give the filename composed of two plate IDs
    separated by a hyphen (e.g., PAC-MBL) and we will instead extract
    that rotation from the GPlates rotation database. We return an error
    if the rotation cannot be found.

**-e**\ *lon*/*lat*/*angle*
    Alternatively, specify the longitude, latitude, and opening angle
    (all in degrees and separated by /) for a single total
    reconstruction.
**-G**\ *outgrdfile*
    Name of output grid. This is the grid with the data reconstructed
    according to the specified rotation.

Optional Arguments
------------------

**-F**\ *polygonfile*
    Specify a multisegment closed polygon file that describes the inside
    area of the grid that should be projected [Default projects entire
    grid].
**-N**
    Do Not output the rotated polygon outline [Default will write it to stdout].

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

**-S**
    Skip the rotation of the grid, just rotate the polygon outline
    (requires **-F**).
**-T**\ *age*
    Sets the desired age of reconstruction when **-E** is given.

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: ../../explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: ../../explain_-bo.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_colon.rst_
.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_-n.rst_
.. include:: ../../explain_help.rst_
.. include:: ../../explain_grdresample2.rst_

Examples
--------

To rotate the data defined by grid topo.nc and the polygon outline
clip_path.d, using a total reconstruction rotation with pole at (135.5,
-33.0) and a rotation angle of 37.3 degrees and bicubic interpolation, try

   ::

    gmt grdrotater topo.nc -e135.5/-33/37.3 -V -Fclip_path.d -Grot_topo.nc > rot_clip_path.d

To rotate the entire grid faa.nc back to 32 Ma using the rotation file
*rotations.txt* and a bilinear interpolation, try

   ::

    gmt grdrotater faa.nc -Erotations.txt -T32 -V -Grot_faa.nc -nl > rot_faa_path.d

To just see how the outline of the grid large.nc will plot after the
same rotation, try

   ::

    gmt grdrotater large.nc -Erotations.txt -T32 -V -S \| psxy -Rg -JH180/6i -B30 -W0.5p \| gv -

Let say you have rotated gridA.nc and gridB.nc, restricting each
rotation to nodes inside polygons polyA.d and polyB.d, respectively,
using rotation A = (123W,22S,16,4) and rotation B = (108W, 16S, -14.5),
yielding rotated grids rot_gridA.nc and rot_gridB.nc. To determine the
region of overlap between the rotated grids, we use grdmath:

   ::

    gmt grdmath 1 rot_gridA.nc ISNAN SUB 1 rot_gridB.nc ISNAN SUB 2 EQ = overlap.nc

The grid overlap.nc now has 1s in the regions of overlap and 0
elsewhere. You can use it as a mask or use grdcontour **-D** to extract
a polygon (contour).

See Also
--------

:doc:`backtracker`,
:doc:`grdspotter`,
:doc:`hotspotter`,
:doc:`originator`,
:doc:`rotconverter`
