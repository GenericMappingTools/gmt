.. index:: ! blockmean

*********
blockmean
*********

.. only:: not man

    Block average (*x*,\ *y*,\ *z*) data tables by L2 norm

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt blockmean** [ *table* ]
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ *fields* ]
[ |-C| ]
[ |-E|\ [**+p**\ \|\ **P**] ]
[ |-G|\ [*grdfile*] ]
[ |SYN_OPT-V| ]
[ |-W|\ [**i**\ \|\ **o**][**+s**] ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**blockmean** reads arbitrarily located (*x*,\ *y*,\ *z*) triples [or
optionally weighted quadruples (*x*,\ *y*,\ *z*,\ *w*)] from standard
input [or *table*] and writes to standard output a mean position and
value for every non-empty block in a grid region defined by the **-R**
and **-I** arguments. See **-G** for writing gridded output directly.
Either **blockmean**, :doc:`blockmedian`, or
:doc:`blockmode` should be used as a pre-processor before running
:doc:`surface` to avoid aliasing short wavelengths. These routines are also
generally useful for decimating or averaging (*x*,\ *y*,\ *z*) data. You
can modify the precision of the output format by editing the
:ref:`FORMAT_FLOAT_OUT <FORMAT_FLOAT_OUT>` parameter in your :doc:`gmt.conf` file, or you may
choose binary input and/or output to avoid loss of precision.

Required Arguments
------------------

.. _-I:

.. include:: explain_-I.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

Optional Arguments
------------------

*table*
    3 [or 4, see **-W**] column ASCII data table file(s) [or binary, see
    **-bi**] holding (*x*,\ *y*,\ *z*\ [,\ *w*])
    data values. [\ *w*] is an optional weight for the data. If no file
    is specified, **blockmean** will read from standard input.

.. _-A:

**-A**\ *fields*
    Select which fields to write to individual grids.  Requires **-G**.
    Append comma-separated codes for available fields: **z** (the mean
    data z, but see **-S**), **s** (standard deviation), **l** (lowest
    value), **h** (highest value) and **w** (the output weight; requires **-W**).
    Note **s**\ \|\ **l**\ \|\ **h** requires **-E** [Default is just **z**].

.. _-C:

**-C**
    Use the center of the block as the output location [Default uses the mean location].

.. _-E:

**-E**\ [**+p**\ \|\ **P**]
    Provide Extended report which includes **s** (the standard deviation
    about the mean), **l**, the lowest value, and **h**, the high value
    for each block. Output order becomes
    *x*,\ *y*,\ *z*,\ *s*,\ *l*,\ *h*\ [,\ *w*]. [Default outputs
    *x*,\ *y*,\ *z*\ [,\ *w*]. See **-W** for *w* output.
    If **-E+p**\ \|\ **P** are used then input data uncertainties are expected and *s*
    becomes the propagated error of the weighted (**+p**) or simple (**+P**) *z* mean.

.. _-G:

**-G**\ *grdfile*
    Write one or more fields directly to grids; no table data are written to
    standard output.  If more than one fields are specified via **-A** then
    *grdfile* must contain the format flag %s so that we can embed the field
    code in the file names.

.. _-S:

**-S**\ [**m**\ \|\ **n**\ \|\ **s**\ \|\ **w**]
    Use **-Sn** to report the number of input points inside each block,
    **-Ss** to report the sum of all *z*-values inside a block, **-Sw**
    to report the sum of weights [Default (or **-Sm** reports mean value]. 

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**i**\ \|\ **o**][**+s**]
    Weighted modifier[s]. Unweighted input and output have 3 columns
    *x*,\ *y*,\ *z*; Weighted i/o has 4 columns *x*,\ *y*,\ *z*,\ *w*.
    Weights can be used in input to construct weighted mean values for
    each block. Weight sums can be reported in output for later combining
    several runs, etc. Use **-W** for weighted i/o, **-Wi** for weighted
    input only, and **-Wo** for weighted output only. [Default uses
    unweighted i/o]. If your weights are actually uncertainties (one sigma)
    then append **+s** and we compute weight = 1/sigma.

.. |Add_-bi| replace:: [Default is 3 (or 4 if **-Wi** is set)].
.. include:: explain_-bi.rst_
   
 
.. |Add_-bo| replace:: [Default is 3 (or 4 if **-Wo** is set)]. **-E** adds 3 additional columns.
   The **-Sn** option will work with only 2 input columns (x and y).

.. include:: explain_-bo.rst_
 
.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_
.. include:: explain_-ocols.rst_

.. |Add_nodereg| replace:: 
    Each block is the locus of points nearest the grid value location. Consider an example with
    **-R**\ 10/15/10/15 and **-I**\ 1: With the |SYN_OPT-r| option, 10 <=
    (*x*,\ *y*) < 11 is one of 25 blocks; without it 9.5 <= (*x*,\ *y*)
    < 10.5 is one of 36 blocks.
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_
.. include:: explain_help.rst_
.. include:: explain_precision.rst_

Examples
--------

To find 5 by 5 minute block mean values from the ASCII data in ship_15.txt, run

   ::

    gmt blockmean @ship_15.txt -R245/255/20/30 -I5m > ship_5x5.txt

To determine how many values were found in each 5x5 minute bin, try

   ::

    gmt blockmean @ship_15.txt -R245/255/20/30 -I5m -Sn > ship_5x5_count.txt

To determine the mean and standard deviation per 10 minute bin and save these to two separate grids
called field_z.nc and field_s.nc, run

   ::

    gmt blockmean @ship_15.txt -I10m -R-115/-105/20/30 -E -Gfield_%s.nc -Az,s

See Also
--------

:doc:`blockmedian`,
:doc:`blockmode`,
:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`greenspline`,
:doc:`nearneighbor`,
:doc:`sphtriangulate`,
:doc:`surface`,
:doc:`triangulate`
