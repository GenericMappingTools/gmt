.. index:: ! blockmode

*********
blockmode
*********

.. only:: not man

    Block average (*x*,\ *y*,\ *z*) data tables by mode estimation

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt blockmode** [ *table* ]
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ *fields* ]
[ |-C| ]
[ |-D|\ [*width*]\ [**+c**][**+a**\ \|\ **+l**\ \|\ **+h** ]
[ |-E|\ **r**\ \|\ **s**\ [**+l**\ \|\ **h**\ ] ]
[ |-G|\ [*grdfile*] ]
[ |-Q| ]
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

**blockmode** reads arbitrarily located (*x*,\ *y*,\ *z*) triples [or
optionally weighted quadruples (*x*,\ *y*,\ *z*,\ *w*)] from standard
input [or *table*] and writes to standard output mode estimates of
position and value for every non-empty block in a grid region defined by
the **-R** and **-I** arguments. See **-G** for writing gridded output directly.
Either :doc:`blockmean`, :doc:`blockmedian`,
or **blockmode** should be used as a pre-processor before running
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
    is specified, **blockmode** will read from standard input.

.. _-A:

**-A**\ *fields*
    Select which fields to write to individual grids.  Requires **-G**.
    Append comma-separated codes for available fields: **z** (the modal
    data z), **s** (the L1 scale of the mode), **l** (lowest
    value), **h** (highest value) and **w** (the output weight; requires **-W**).
    Note **s**\ \|\ **l**\ \|\ **h** requires **-E**, and **Es**\ \|\ **r**
    cannot be used. [Default is just **z**].

.. _-C:

**-C**
    Use the center of the block as the output location [Default uses the
    modal xy location (but see **-Q**)]. **-C** overrides **-Q**.

.. _-D:

**-D**\ [*width*]\ [**+c**][**+a**\ \|\ **+l**\ \|\ **+h** ]
    Perform unweighted mode calculation via histogram binning, using the
    specified histogram *width*. Append **+c** to center bins so that
    their mid point is a multiple of *width* [uncentered].
    If multiple modes are found for a block we return the average mode [**+a**].
    Append **+l** or **+h** to return the low of high mode instead, respectively.
    If *width* is not given it will default to 1 provided your data set only
    contains integers. Also, for integer data and integer bin *width* we
    enforce bin centering (**+c**) and select the lowest mode (**+l**) if
    there are multiples. [Default mode is normally the Least Median of Squares (LMS) statistic].

.. _-E:

**-E**
    Provide Extended report which includes **s** (the L1 scale of the
    mode), **l**, the lowest value, and **h**, the high value for each
    block. Output order becomes
    *x*,\ *y*,\ *z*,\ *s*,\ *l*,\ *h*\ [,\ *w*]. [Default outputs
    *x*,\ *y*,\ *z*\ [,\ *w*]. See **-W** for *w* output.
**-E**\ **r**\ \|\ **s**\ [**+l**\ \|\ **h**\ ]
    Provide source id **s** or record number **r** output, i.e., append
    the source id or record number associated with the modal value. If
    tied then report the record number of the higher of the two values (i.e., **+h** is the default);
    append **+l** to instead report the record number of the lower value.
    Note that **-E** may be repeated so that both both **-E** and
    **-E**\ **r**\ [**+l**\ \|\ **h**\ ] may be specified.
    For **-E**\ **s** we expect input records of the form
    *x*,\ *y*,\ *z*\ [,\ *w*],\ *sid*, where *sid* is an unsigned integer
    source id.

.. _-G:

**-G**\ *grdfile*
    Write one or more fields directly to grids; no table data are written to
    standard output.  If more than one fields are specified via **-A** then
    *grdfile* must contain the format flag %s so that we can embed the field
    code in the file names.

.. _-Q:

**-Q**
    (Quicker) Finds mode *z* and mean (*x*,\ *y*) [Default finds mode
    *x*, mode *y*, mode *z*]. 

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**i**\ \|\ **o**][**+s**]
    Weighted modifier[s]. Unweighted input and output have 3 columns
    *x*,\ *y*,\ *z*; Weighted i/o has 4 columns *x*,\ *y*,\ *z*,\ *w*.
    Weights can be used in input to construct weighted modal values for each
    block. Weight sums can be reported in output for later combining
    several runs, etc. Use **-W** for weighted i/o, **-Wi** for weighted
    input only, and **-Wo** for weighted output only. [Default uses unweighted i/o]. 
    If your weights are actually uncertainties (one sigma)
    then append **+s** and we compute weight = 1/sigma.

.. |Add_-bi| replace:: [Default is 3 (or 4 if **-Wi** is set)].
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 3 (or 4 if **-Wo** is set)]. **-E** adds 3 additional columns.
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

To find 5 by 5 minute block mode from the ASCII data in ship_15.txt
and output a binary table with double precision triplets, run

   ::

    gmt blockmedian @ship_15.txt -R245/255/20/30 -I5m -bo3d > ship_5x5.b

To determine the most frequently occurring values per 2x2 block using histogram binning, with
data representing integer counts, try

   ::

    gmt blockmode @ship_15.txt -R245/255/20/30 -I2  -r -C -D

To determine the mode and L1 scale (MAD) on the mode per 10 minute bin and save these to two separate grids
called field_z.nc and field_s.nc, run

   ::

    gmt blockmode @ship_15.txt -I10m -R-115/-105/20/30 -E -Gfield_%s.nc -Az,s

See Also
--------

:doc:`blockmean`,
:doc:`blockmedian`, :doc:`gmt`,
:doc:`gmt.conf`,
:doc:`greenspline`,
:doc:`nearneighbor`,
:doc:`sphtriangulate`,
:doc:`surface`,
:doc:`triangulate`
