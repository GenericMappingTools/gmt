********
gshhg_dp
********

gshhg\_dp - Line reduction using the Douglas-Peucker algorithm

`Synopsis <#toc1>`_
-------------------

**gshhg\_dp** *input.b tolerance output.b* [ **-v** ]

`Description <#toc2>`_
----------------------

**gshhg\_dp** reads the binary coastline (GSHHG) or political boundary
or river (WDBII) files and and reduces the complexity of the line by
applying the Douglas-Peucker algorithm. It automatically handles
byte-swabbing between different architectures.

`Required Arguments <#toc3>`_
-----------------------------

*input.b*
    GSHHG or WDBII binary data file as distributed with the GSHHG data
    supplement. Any of the 5 standard resolutions (full, high,
    intermediate, low, crude) can be used.
*tolerance*
    tolerance is maximum mismatch in km. The larger the value the more
    reduction will take place.
*output.b*
    The reducted data set.

`Optional Arguments <#toc4>`_
-----------------------------

**-v**
    Reports progress and statistics while running.

`Examples <#toc5>`_
-------------------

To simplify the full GSHHG data set with a custom tolerance of 2 km, try

gshhg\_dp gshhs\_f.b 2 gshhs\_2km.b

`References <#toc6>`_
---------------------

Douglas, D. H., and T. K. Peucker, Algorithms for the reduction of the
number of points required to represent a digitized line of its
caricature, *Can. Cartogr.*, **10**, 112-122, 1973.

`Author <#toc7>`_
-----------------

This implementation of the D-P algorithm has been kindly provided by Dr.
Gary J. Robinson, Environmental Systems Science Centre, University of
Reading, Reading, UK (gazza@mail.nerc-essc.ac.uk); his subroutine forms
the basis for this program.

`See Also <#toc8>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ , `*gshhg*\ (1) <gshhg.html>`_ ,
`*gshhgtograss*\ (1) <gshhgtograss.html>`_
