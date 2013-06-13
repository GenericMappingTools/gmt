********
grdpaste
********

grdpaste - Join two grids along their common edge

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**grdpaste** *file_a.nc file_b.nc* **-G**\ *outfile.nc*
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]

|No-spaces|

Description
-----------

**grdpaste** will combine *file_a.nc* and *file_b.nc* into
*outfile.nc* by pasting them together along their common edge. Files
*file_a.nc* and *file_b.nc* must have the same dx, dy and have one
edge in common. If in doubt, check with **grdinfo** and use **grdcut**
and/or **grdsample** if necessary to prepare the edge joint. For
geographical grids, use **-f** to handle periodic longitudes. 

Required Arguments
------------------

*file_a.nc*
    One of two files to be pasted together.
*file_b.nc*
    The other of two files to be pasted together.
**-G**\ *outfile.nc*
    The name for the combined output.

Optional Arguments
------------------

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout.rst_

Examples
--------

Suppose file_a.nc is 150E - 180E and 0 - 30N, and file_b.nc is 150E -
180E, -30S - 0, then you can make outfile.nc which will be 150 - 180 and
-30S - 30N by:

   ::

    gmt grdpaste file_a.nc file_b.nc -Goutfile.nc -V -fg

See Also
--------

`gmt <gmt.html>`_, `grdblend <grdblend.html>`_,
`grdcut <grdcut.html>`_, `grdinfo <grdinfo.html>`_,
`grdsample <grdsample.html>`_
