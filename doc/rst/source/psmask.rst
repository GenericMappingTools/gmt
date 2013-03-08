******
psmask
******

psmask - Use data tables to clip or mask map areas with no coverage

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**psmask** [ *table* ]
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-D**\ *dumpfile* ] [
**-G**\ *fill* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-N** ] [ **-O** ] [ **-P** ] [ **-Q**\ *cut* ] [
**-S**\ *search\_radius*\ [*unit*\ ] ] [ **-T** ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [ **-c**\ *copies* ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-r** ] [ **-t**\ [*transp*\ ] ] [ **-:**\ [**i**\ \|\ **o**] ]

**psmask** **-C** [ **-K** ] [ **-O** ]

`Description <#toc2>`_
----------------------

**psmask** reads a (*x*,\ *y*,\ *z*) file [or standard input] and uses
this information to find out which grid cells are reliable. Only grid
cells which have one or more data points are considered reliable. As an
option, you may specify a radius of influence. Then, all grid cells that
are within *radius* of a data point are considered reliable.
Furthermore, an option is provided to reverse the sense of the test.
Having found the reliable/not reliable points, **psmask** will either
paint tiles to mask these nodes (with the **-T** switch), or use
contouring to create polygons that will clip out regions of no interest.
When clipping is initiated, it will stay in effect until turned off by a
second call to **psmask** using the **-C** option. 

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

.. include:: explain_-I.rst_

.. include:: explain_-J.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. include:: explain_-B.rst_

**-C**
    Mark end of existing clip path. No input file is needed. Implicitly
    sets **-O**. Also supply **-X** and **-Y** settings if you have
    moved since the clip started.
**-D**\ *dumpfile*
    Dump the (x,y) coordinates of each clipping polygon to one or more
    output files (or *stdout* if *template* is not given). No plotting
    will take place. If *template* contains the C-format specifier %d
    (including modifications like %05d) then polygons will be written to
    different files; otherwise all polygons are written to the specified
    file (*template*). The files are ASCII unless
    **-bo**\ [*ncols*\ ][*type*\ ] is used. See **-Q** to exclude small
    polygons from consideration. 

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

**-G**\ *fill*
    Paint the clip polygons (or tiles) with a selected fill [Default is no fill]. 

.. include:: explain_-Jz.rst_

.. include:: explain_-K.rst_

**-N**
    Invert the sense of the test, i.e., clip regions where there is data coverage. 

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-Q**
    Do not dump polygons with less than *cut* number of points [Dumps
    all polygons]. Only applicable if **-D** has been specified.
**-S**\ *search\_radius*\ [*unit*\ ]
    Sets radius of influence. Grid nodes within *radius* of a data point
    are considered reliable. [Default is 0, which means that only grid
    cells with data in them are reliable]. Append the distance unit (see
    UNITS).
**-T**
    Plot tiles instead of clip polygons. Use **-G** to set tile color or
    pattern. Cannot be used with **-D**. 

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-XY.rst_

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. include:: explain_-c.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: explain_-h.rst_
    
.. include:: explain_-icols.rst_

.. include:: explain_perspective.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_-t.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

`Examples <#toc7>`_
-------------------

To make an overlay *PostScript* file that will mask out the regions of a
contour map where there is no control data using clip polygons, use:

psmask africa\_grav.xyg -R20/40/20/40 -I5m -JM10i -O -K > mask.ps

We do it again, but this time we wish to save the clipping polygons to
file all\_pols.txt:

psmask africa\_grav.xyg -R20/40/20/40 -I5m -Dall\_pols.txt

A repeat of the first example but this time we use white tiling:

psmask africa\_grav.xyg -R20/40/20/40 -I5m -JM10i -T -O -K -Gwhite >
mask.ps

`See Also <#toc8>`_
-------------------

`gmt <gmt.html>`_ , `gmtcolors <gmtcolors.html>`_ ,
`grdmask <grdmask.html>`_ , `surface <surface.html>`_ ,
`psbasemap <psbasemap.html>`_ , `psclip <psclip.html>`_
