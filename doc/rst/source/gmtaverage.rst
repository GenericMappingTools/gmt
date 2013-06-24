**********
gmtaverage
**********

gmtaverage - Block average (*x*,\ *y*,\ *z*) data tables by mean,
median, or mode estimation

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmtaverage** [ *xyz[w]file(s)* ]
|SYN_OPT-I|
|SYN_OPT-R|
**-Te**\ \|\ **m**\ \|\ **n**\ \|\ **o**\ \|\ **s**\ \|\ **w**\ \|\ *quantile*
[ **-C** ] [ **-E**\ [**b**\ ] ] [ **-Q** ]
[ |SYN_OPT-V| ]
[ **-W**\ [**io**\ ] ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**gmtaverage** reads arbitrarily located (*x*,\ *y*,\ *z*) triples [or
optionally weighted quadruples (*x*,\ *y*,\ *z*,\ *w*)] from standard
input [or *xyz[w]file(s)*] and writes to standard output an average
position and value for every non-empty block in a grid region defined by
the **-R** and **-I** arguments. **gmtaverage** should be used as a
pre-processor before running **surface** to avoid aliasing short
wavelengths, but is also generally useful for decimating or averaging
(*x*,\ *y*,\ *z*) data. You can modify the precision of the output
format by editing the **FORMAT\_FLOAT\_OUT** parameter in your
`gmt.conf <gmt.conf.html>`_ file, or you may choose binary input and/or output using
single or double precision storage. 

Required Arguments
------------------

.. include:: explain_-I.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

**-T**
    Specify the operator to use when computing output values. Choose
    among **e** (median), **m** (mean), **n** (number of points), **o**
    (mode), **s** (sum), **w** (weight sum), or the *quantile* of the
    distribution to be returned. Here, 0 < *quantile* < 1.

Optional Arguments
------------------

*xyz[w]file(s)*
    3 [or 4, see **-W**] column ASCII data table file(s) [or binary, see
    **-bi**] holding (*x*,\ *y*,\ *z*\ [,*w*])
    data values. [*w*] is an optional weight for the data. If no file
    is specified, **gmtaverage** will read from standard input.

**-C**
    Use the center of the block as the output location [Default uses the
    mean, median, or mode x and y as location, depending on operator
    selected with **-T** (but see **-Q**)].

**-E**\ [**b**\ ]
    Provide Extended report which includes **s** (the scale of the
    reported value), **l**, the lowest value, and **h**, the high value
    for each block. Output order becomes
    *x*,\ *y*,\ *z*,\ *s*,\ *l*,\ *h*\ [,*w*]. [Default outputs
    *x*,\ *y*,\ *z*\ [,*w*]. For box-and-whisker calculation using
    **-Te**, select **-Eb** which will output
    *x*,\ *y*,\ *z*,\ *l*,\ *q25*,\ *q75*,\ *h*\ [,*w*], where *q25* and
    *q75* are the 25% and 75% quantiles, respectively. The *scale* is
    the standard deviation, the L1 scale, or the LMS scale, depending on
    **-T**. See **-W** for *w* output.

**-Q**
    (Quicker) Finds median (or mode) *z* and (*x*,\ *y*) at that median
    (or mode) *z* [Default finds median or mode *x* and *y* independent
    of *z*]. Also see **-C**. Ignored if **-Tm**, **-Tn**, **-Ts** or
    **-Tw** is used. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-W**\ [**io**\ ]
    Weighted modifier[s]. Unweighted input and output has 3 columns
    *x*,\ *y*,\ *z*; Weighted i/o has 4 columns *x*,\ *y*,\ *z*,\ *w*.
    Weights can be used in input to construct weighted output values in
    blocks. Weight sums can be reported in output for later combining
    several runs, etc. Use **-W** for weighted i/o, **-Wi** for weighted
    input only, **-Wo** for weighted output only. [Default uses
    unweighted i/o]. 

.. |Add_-bi| replace:: [Default is 3 (or 4 if **-Wi** is set)]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 3 (or 4 if **-Wo** is set)]. **-E** adds 3 additional columns. 
.. include:: explain_-bo.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. |Add_nodereg| replace:: 
    Each block is the locus of
    points nearest the grid value location. For example, with
    **-R**\ 10/15/10/15 and and **-I**\ 1: with the **-r** option 10 <=
    (*x*,\ *y*) < 11 is one of 25 blocks; without it 9.5 <= (*x*,\ *y*)
    < 10.5 is one of 36 blocks. 
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Examples
--------

To find 5 by 5 minute block mode values from the double precision binary
data in hawaii_b.xyg and output an ASCII table, run

   ::

    gmt gmtaverage hawaii_b.xyg -R198/208/18/25 -I5m -To -bi3 > hawaii_5x5.xyg

To find 5 by 5 minute block mean values from the same file, run

   ::

    gmt gmtaverage hawaii.xyg -R198/208/18/25 -I5m -Tm > hawaii_5x5.xyg

To find the number of data points in each 5 by 5 minute block from the same file, run

   ::

    gmt gmtaverage hawaii.xyg -R198/208/18/25 -I5m -Tn > hawaii_5x5.xyn

To compute the shape of a data distribution per bin via a
box-and-whisker diagram we need the 0%, 25%, 50%, 75%, and 100%
quantiles. To do so on a global 5 by 5 degree basis from the ASCII table
depths.xyz and send output to an ASCII table, run

   ::

    gmt gmtaverage depths.xyz -Rg -I5 -Te -Eb -r > depths_5x5.txt

See Also
--------

`gmt <gmt.html>`_ , `gmt.conf <gmt.conf.html>`_ ,
`nearneighbor <nearneighbor.html>`_ ,
`surface <surface.html>`_ ,
`triangulate <triangulate.html>`_
