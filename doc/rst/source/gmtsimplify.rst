.. index:: ! gmtsimplify

********
simplify
********

.. only:: not man

    Line reduction using the Douglas-Peucker algorithm

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt simplify** [ *table* ] |-T|\ *tolerance*\ [*unit*]
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

**simplify** reads one or more data files and apply the Douglas-Peucker
line simplification algorithm. The method recursively subdivides a
polygon until a run of points can be replaced by a straight line
segment, with no point in that run deviating from the straight line by
more than the tolerance. Have a look at this site to get a visual
insight on how the algorithm works
(http://geometryalgorithms.com/Archive/algorithm_0205/algorithm_0205.htm)

Required Arguments
------------------

.. _-T:

**-T**\ *tolerance*\ [*unit*]
    Specifies the maximum mismatch tolerance in the user units. If the
    data are not Cartesian then append a suitable distance unit (see :ref:`Unit_attributes`).

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

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

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

Examples
--------

To reduce the geographic line segment.txt using a tolerance of 2 km, run

   ::

    gmt simplify segment.txt -T2k > new_segment.txt

To reduce the Cartesian lines xylines.txt using a tolerance of 0.45 and
write the reduced lines to file new_xylines.txt, run

   ::

    gmt simplify xylines.txt -T0.45 > new_xylines.txt

Notes
-----

There is a slight difference in how **simplify** processes lines versus
closed polygons.  Segments that are explicitly closed will be considered
polygons, otherwise we treat them as line segments.  Hence, segments
recognized as polygons may reduce to a 3-point polygon with no area;
these are suppressed from the output.

Bugs
----

One known issue with the Douglas-Peucker has to do with crossovers.
Specifically, it cannot be guaranteed that the reduced line does not
cross itself. Depending on how many lines you are considering it is also
possible that reduced lines may intersect other reduced lines. Finally,
the current implementation only does Flat Earth calculations even if you
specify spherical; **simplify** will issue a warning and reset the
calculation mode to Flat Earth.

References
----------

Douglas, D. H., and T. K. Peucker, Algorithms for the reduction of the
number of points required to represent a digitized line of its
caricature, *Can. Cartogr.*, **10**, 112-122, 1973.

This implementation of the algorithm has been kindly provided by Dr.
`Gary J. Robinson <http://www.met.reading.ac.uk/users/users/1327/>`_,
Department of Meteorology, University of Reading, Reading, UK; his
subroutine forms the basis for this program.

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`gmtconnect`,
:doc:`gmtconvert`,
:doc:`gmtselect`
