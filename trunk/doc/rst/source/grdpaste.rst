********
grdpaste
********

grdpaste - Join two grids along their common edge

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**grdpaste** *file\_a.nc file\_b.nc* **-G**\ *outfile.nc* [
**-V**\ [*level*\ ] ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

|No-spaces|

`Description <#toc2>`_
----------------------

**grdpaste** will combine *file\_a.nc* and *file\_b.nc* into
*outfile.nc* by pasting them together along their common edge. Files
*file\_a.nc* and *file\_b.nc* must have the same dx, dy and have one
edge in common. If in doubt, check with **grdinfo** and use **grdcut**
and/or **grdsample** if necessary to prepare the edge joint. For
geographical grids, use **-f** to handle periodic longitudes. 

`Required Arguments <#toc4>`_
-----------------------------

*file\_a.nc*
    One of two files to be pasted together.
*file\_b.nc*
    The other of two files to be pasted together.
**-G**\ *outfile.nc*
    The name for the combined output.

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout.rst_

`Examples <#toc7>`_
-------------------

Suppose file\_a.nc is 150E - 180E and 0 - 30N, and file\_b.nc is 150E -
180E, -30S - 0, then you can make outfile.nc which will be 150 - 180 and
-30S - 30N by:

    grdpaste file\_a.nc file\_b.nc -Goutfile.nc -V -fg

`See Also <#toc8>`_
-------------------

`gmt5 <gmt5.html>`_, `grdblend <grdblend.html>`_,
`grdcut <grdcut.html>`_, `grdinfo <grdinfo.html>`_,
`grdsample <grdsample.html>`_
