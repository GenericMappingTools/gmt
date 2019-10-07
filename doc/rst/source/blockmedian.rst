.. index:: ! blockmedian

***********
blockmedian
***********

.. only:: not man

    Block average (*x*,\ *y*,\ *z*) data tables by L1 norm

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt blockmedian** [ *table* ]
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ *fields* ]
[ |-C| ]
[ |-E|\ [**b**] ] [ |-E|\ **r**\ \|\ **s**\ [**+l**\ \|\ **h**\ ] ]
[ |-G|\ [*grdfile*] ]
[ |-Q| ]
[ |-T|\ *quantile* ]
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

**blockmedian** reads arbitrarily located (*x*,\ *y*,\ *z*) triples [or
optionally weighted quadruples (*x*,\ *y*,\ *z*,\ *w*)] from standard
input [or *table*] and writes to standard output a median position and
value for every non-empty block in a grid region defined by the **-R**
and **-I** arguments. See **-G** for writing gridded output directly.
Either :doc:`blockmean`, **blockmedian**, or
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
    3 [or 4, see **-W**] column ASCII data table] column ASCII file(s)
    [or binary, see **-bi**] holding
    (*x*,\ *y*,\ *z*\ [,\ *w*]) data values. [\ *w*] is an optional weight
    for the data. If no file is specified, **blockmedian** will read
    from standard input.

.. _-A:

**-A**\ *fields*
    Select which fields to write to individual grids.  Requires **-G**.
    Append comma-separated codes for available fields: **z** (the median
    data z, but see **-T**), **s** (the L1 scale of the median), **l** (lowest
    value), **q25** (the 25% quartile), **q75** (the 75% quartile), **h** (highest value),
    and **w** (the output weight; requires **-W**).  Note **s**\ \|\ **l**\ \|\ **h**
    requires **-E**, while **l**\ \|\ **q25**\ \|\ **q75**\ \|\ **h** requires **-Eb**,
    and **Es**\ \|\ **r** cannot be used. [Default is just **z**].

.. _-C:

**-C**
    Use the center of the block as the output location [Default uses the
    median x and median y as location (but see **-Q**)].

.. _-E:

**-E**\ [**b**\ ]
    Provide Extended report which includes **s** (the L1 scale of the
    median, i.e., 1.4826 \* median absolute deviation [MAD]), **l**, the lowest
    value, and **h**, the high value for each block. Output order becomes
    *x*,\ *y*,\ *z*,\ *s*,\ *l*,\ *h*\ [,\ *w*]. [Default outputs
    *x*,\ *y*,\ *z*\ [,\ *w*]. For box-and-whisker calculation, use
    **-Eb** which will output
    *x*,\ *y*,\ *z*,\ *l*,\ *q25*,\ *q75*,\ *h*\ [,\ *w*], where *q25* and
    *q75* are the 25% and 75% quantiles, respectively. See **-W** for
    *w* output.
**-E**\ **r**\ \|\ **s**\ [**+l**\ \|\ **h**\ ]
    Provide source id **s** or record number **r** output, i.e., append
    the source id or record number associated with the median value. If
    tied then report the record number of the higher of the two values (i.e., **+h** is the default);
    append **+l** to instead report the record number of the lower value.
    Note that **-E** may be repeated so that both **-E**\ [**b**\ ] and
    **-E**\ **r**\ [**+l**\ \|\ **h**\ ] can be
    specified. For **-E**\ **s** we expect input records of the form
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
    (Quicker) Finds median *z* and (*x*,\ *y*) at that the median *z*
    [Default finds median *x*, median *y* independent of *z*]. Also see **-C**.

.. _-T:

**-T**\ *quantile*
    Sets the *quantile* of the distribution to be returned [Default is
    0.5 which returns the median *z*]. Here, 0 < *quantile* < 1.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**i**\ \|\ **o**][**+s**]
    Weighted modifier[s]. Unweighted input and output have 3 columns
    *x*,\ *y*,\ *z*; Weighted i/o has 4 columns *x*,\ *y*,\ *z*,\ *w*.
    Weights can be used in input to construct weighted median values for each
    block. Weight sums can be reported in output for later combining
    several runs, etc. Use **-W** for weighted i/o, **-Wi** for weighted
    input only, and **-Wo** for weighted output only. [Default uses
    unweighted i/o]. If your weights are actually uncertainties (one sigma)
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

To find 5 by 5 minute block medians from the ASCII data in ship_15.txt
and output a binary table with double precision triplets, run

   ::

    gmt blockmedian @ship_15.txt -R245/255/20/30 -I5m -bo3d > ship_5x5.b

To compute the shape of a data distribution per bin via a
box-and-whisker diagram we need the 0%, 25%, 50%, 75%, and 100%
quantiles. To do so on a global 5 by 5 degree basis from the ASCII table
mars370.txt and send output to an ASCII table, run

   ::

    gmt gmt blockmedian @mars370.txt -Rg -I5 -Eb -r > mars_5x5.txt

To determine the median and L1 scale (MAD) on the median per 10 minute bin and save these to two separate grids
called field_z.nc and field_s.nc, run

   ::

    gmt blockmedian @ship_15.txt -I10m -R-115/-105/20/30 -E -Gfield_%s.nc -Az,s

See Also
--------

:doc:`blockmean`,
:doc:`blockmode`, :doc:`gmt`,
:doc:`gmt.conf`,
:doc:`greenspline`,
:doc:`nearneighbor`,
:doc:`surface`,
:doc:`sphtriangulate`,
:doc:`triangulate`
