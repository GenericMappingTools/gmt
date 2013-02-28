***********
grdlandmask
***********

grdlandmask - Create a "wet-dry" mask grid from shoreline data base

`Synopsis <#toc1>`_
-------------------

**grdlandmask** **-G**\ *mask\_grd\_file*]
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [
**-A**\ *min\_area*\ [/*min\_level*/*max\_level*][\ **+r**\ \|\ **l**][\ **p**\ *percent*]
] [ **-D**\ *resolution*\ [**+**\ ] ] [ **-N**\ *maskvalues*\ [**o**\ ]
] [ **-V**\ [*level*\ ] ] [ **-r** ]

`Description <#toc2>`_
----------------------

**grdlandmask** reads the selected shoreline database and uses that
information to decide which nodes in the specified grid are over land or
over water. The nodes defined by the selected region and lattice spacing
will be set according to one of two criteria: (1) land vs water, `or
(2) <or.2.html>`_ the more detailed (hierarchical) ocean vs land vs lake
vs island vs pond. The resulting mask may be used in subsequent
operations involving **grdmath** to mask out data from land [or water]
areas. 

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

**-G**\ *mask\_grd\_file*]
    Name of resulting output mask grid file. (See GRID FILE FORMATS below). 

.. include:: explain_-I.rst_

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_-A| unicode:: 0x20 .. just an invisible code
.. include:: explain_-A.rst_

**-D**\ *resolution*\ [**+**\ ]
    Selects the resolution of the data set to use ((**f**)ull,
    (**h**)igh, (**i**)ntermediate, (**l**)ow, or (**c**)rude). The
    resolution drops off by ~80% between data sets. [Default is **l**].
    Append **+** to automatically select a lower resolution should the
    one requested not be available [abort if not found]. Note that
    because the coastlines differ in details a node in a mask file using
    one resolution is not guaranteed to remain inside [or outside] when
    a different resolution is selected.
**-N**\ *maskvalues*\ [**o**\ ]
    Sets the values that will be assigned to nodes. Values can be any
    number, including the textstring NaN. Append **o** to let nodes
    exactly on feature boundaries be considered outside [Default is
    inside]. Specify this information using 1 of 2 formats:

    **-N**\ *wet/dry*.

    **-N**\ *ocean/land/lake/island/pond*.

    [Default is 0/1/0/1/0 (i.e., 0/1)]. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_output.rst_

`Examples <#toc7>`_
-------------------

To set all nodes on land to NaN, and nodes over water to 1, using the
high resolution data set, do

grdlandmask -R-60/-40/-40/-30 -Dh -I5m -N1/NaN -Gland\_mask.nc -V

To make a 1x1 degree global grid with the hierarchical levels of the
nodes based on the low resolution data:

grdlandmask -R0/360/-90/90 -Dl -I1 -N0/1/2/3/4 -Glevels.nc -V
 
.. include:: explain_gshhs.rst_

`See Also <#toc9>`_
-------------------

`gmt <gmt.html>`_ , `grdmath <grdmath.html>`_ ,
`grdclip <grdclip.html>`_ , `psmask <psmask.html>`_ ,
`psclip <psclip.html>`_ , `pscoast <pscoast.html>`_
