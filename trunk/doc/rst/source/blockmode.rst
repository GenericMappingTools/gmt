*********
blockmode
*********

blockmode - Block average (*x*,\ *y*,\ *z*) data tables by mode
estimation

`Synopsis <#toc1>`_
-------------------

**blockmode** [ *table* ]
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] [ **-C** ] [
**-E** ] [ **-E**\ **r**\ \|\ **s**\ [**-**\ ] ] [ **-Q** ] [
**-V**\ [*level*\ ] ] [ **-W**\ [**i**\ \|\ **o**] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**blockmode** reads arbitrarily located (*x*,\ *y*,\ *z*) triples [or
optionally weighted quadruples (*x*,\ *y*,\ *z*,\ *w*)] from standard
input [or *table*] and writes to standard output mode estimates of
position and value for every non-empty block in a grid region defined by
the **-R** and **-I** arguments. Either **blockmean**, **blockmedian**,
or **blockmode** should be used as a pre-processor before running
**surface** to avoid aliasing short wavelengths. These routines are also
generally useful for decimating or averaging (*x*,\ *y*,\ *z*) data. You
can modify the precision of the output format by editing the
**FORMAT\_FLOAT\_OUT** parameter in your `gmt.conf <gmt.conf.html>`_ file, or you may
choose binary input and/or output to avoid loss of precision. 

..  include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

.. include:: explain_-I.rst_

.. |Add_-R| unicode:: 0x0C .. just an invisible code
.. include:: explain_-R.rst_

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    3 [or 4, see **-W**] column ASCII data table file(s) [or binary, see
    **-bi**\ [*ncols*\ ][*type*\ ]] holding (*x*,\ *y*,\ *z*\ [,\ *w*])
    data values. [\ *w*] is an optional weight for the data. If no file
    is specified, **blockmode** will read from standard input.
**-C**
    Use the center of the block as the output location [Default uses the
    modal xy location (but see **-Q**)]. **-C** overrides **-Q**.
**-E**
    Provide Extended report which includes **s** (the L1 scale of the
    mode), **l**, the lowest value, and **h**, the high value for each
    block. Output order becomes
    *x*,\ *y*,\ *z*,\ *s*,\ *l*,\ *h*\ [,\ *w*]. [Default outputs
    *x*,\ *y*,\ *z*\ [,\ *w*]. See **-W** for *w* output.
**-E**\ **r**\ \|\ **s**\ [**-**\ ]
    Provide source id **s** or record number **r** output, i.e., append
    the source id or record number associated with the modal value. If
    tied then report the record number of the higher of the two values;
    append **-** to instead report the record number of the lower value.
    Note that both **-E** and **-E**\ **r**\ [**-**\ ] may be specified.
    For **-E**\ **s** we expect input records of the form
    *x*,\ *y*,\ *z*\ [,\ *w*],\ *sid*, where *sid* is an unsigned integer
    source id.
**-Q**
    (Quicker) Finds mode *z* and mean (*x*,\ *y*) [Default finds mode
    *x*, mode *y*, mode *z*]. 

.. |Add_-V| unicode:: 0x0C .. just an invisible code
.. include:: explain_-V.rst_

**-W**\ [**i**\ \|\ **o**]
    Weighted modifier[s]. Unweighted input and output has 3 columns
    *x*,\ *y*,\ *z*; Weighted i/o has 4 columns *x*,\ *y*,\ *z*,\ *w*.
    Weights can be used in input to construct weighted mean values in
    blocks. Weight sums can be reported in output for later combining
    several runs, etc. Use **-W** for weighted i/o, **-Wi** for weighted
    input only, **-Wo** for weighted output only. [Default uses
    unweighted i/o]. 

.. |Add_-bi| replace:: [Default is 3 (or 4 if **-Wi** is set)].
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 3 (or 4 if **-Wo** is set)]. **-E** adds 3 additional columns.
.. include:: explain_-bo.rst_

.. |Add_-f| unicode:: 0x0C .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x0C .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_
.. include:: explain_-ocols.rst_

.. |Add_nodereg| replace:: 
    Each block is the locus of points nearest the grid value location. For example, with
    **-R**\ 10/15/10/15 and **-I**\ 1: with the **-r** option 10 <=
    (*x*,\ *y*) < 11 is one of 25 blocks; without it 9.5 <= (*x*,\ *y*)
    < 10.5 is one of 36 blocks.
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_
.. include:: explain_help.rst_
.. include:: explain_precision.rst_

`Examples <#toc7>`_
-------------------

To find 5 by 5 minute block mode estimates from the double precision
binary data in hawaii\_b.xyg and output an ASCII table, run:

blockmode hawaii\_b.xyg -R198/208/18/25 -I5m -bi3d > hawaii\_5x5.xyg

See Also
--------

`blockmean <blockmean.html>`_ ,
`blockmedian <blockmedian.html>`_ , `gmt <gmt.html>`_ ,
`gmt.conf <gmt.conf.html>`_ ,
`nearneighbor <nearneighbor.html>`_ ,
`surface <surface.html>`_ ,
`triangulate <triangulate.html>`_
