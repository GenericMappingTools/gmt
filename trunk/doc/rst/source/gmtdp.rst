*****
gmtdp
*****

gmtdp - Line reduction using the Douglas-Peucker algorithm

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**gmtdp** [ *table* ] **-T**\ *tolerance*\ [*unit*\ ] [
**-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-bo**\ [*ncols*\ ][*type*\ ] ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo*
] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-:**\ [**i**\ \|\ **o**] ]

|No-spaces|

`Description <#toc2>`_
----------------------

**gmtdp** reads one or more data files and apply the Douglas-Peucker
line simplification algorithm. The method recursively subdivides a
polygon until a run of points can be replaced by a straight line
segment, with no point in that run deviating from the straight line by
more than the tolerance. Have a look at this site to get a visual
insight on how the algorithm works
(http://geometryalgorithms.com/Archive/algorithm\_0205/algorithm\_0205.htm)

`Required Arguments <#toc4>`_
-----------------------------

**-T**\ *tolerance*\ [*unit*\ ]
    Specifies the maximum mismatch tolerance in the user units. If the
    data is not Cartesian then append the distance unit (see UNITS).

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_
 
.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

`Examples <#toc8>`_
-------------------

To reduce the geographic line segment.d using a tolerance of 2 km, run

    gmtdp segment.d -T2k > new\_segment.d

To reduce the Cartesian lines xylines.d using a tolerance of 0.45 and
write the reduced lines to file new\_xylines.d, run

    gmtdp xylines.d -T0.45 > new\_xylines.d

`Bugs <#toc9>`_
---------------

One known issue with the Douglas-Peucker has to do with crossovers.
Specifically, it cannot be guaranteed that the reduced line does not
cross itself. Depending on how many lines you are considering it is also
possible that reduced lines may intersect other reduced lines. Finally,
the current implementation only does Flat Earth calculations even if you
specify spherical; **gmtdp** will issue a warning and reset the
calculation mode to Flat Earth.

`References <#toc10>`_
----------------------

Douglas, D. H., and T. K. Peucker, Algorithms for the reduction of the
number of points required to represent a digitized line of its
caricature, *Can. Cartogr.*, **10**, 112-122, 1973.

This implementation of the algorithm has been kindly provided by Dr.
Gary J. Robinson, Environmental Systems Science Centre, University of
Reading, Reading, UK (gazza@mail.nerc-essc.ac.uk); his subroutine forms
the basis for this program.

`See Also <#toc11>`_
--------------------

`gmt5 <gmt5.html>`_
