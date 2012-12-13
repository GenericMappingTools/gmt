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
**FORMAT\_FLOAT\_OUT** parameter in your **gmt.conf** file, or you may
choose binary input and/or output to avoid loss of precision.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
    *x\_inc* [and optionally *y\_inc*] is the grid spacing. Optionally,
    append a suffix modifier. **Geographical (degrees) coordinates**:
    Append **m** to indicate arc minutes or **s** to indicate arc
    seconds. If one of the units **e**, **f**, **k**, **M**, **n** or
    **u** is appended instead, the increment is assumed to be given in
    meter, foot, km, Mile, nautical mile or US survey foot,
    respectively, and will be converted to the equivalent degrees
    longitude at the middle latitude of the region (the conversion
    depends on **PROJ\_ELLIPSOID**). If /*y\_inc* is given but set to 0
    it will be reset equal to *x\_inc*; otherwise it will be converted
    to degrees latitude. **All coordinates**: If **=** is appended then
    the corresponding max *x* (*east*) or *y* (*north*) may be slightly
    adjusted to fit exactly the given increment [by default the
    increment may be adjusted slightly to fit the given domain].
    Finally, instead of giving an increment you may specify the *number
    of nodes* desired by appending **+** to the supplied integer
    argument; the increment is then recalculated from the number of
    nodes and the domain. The resulting increment value depends on
    whether you have selected a gridline-registered or pixel-registered
    grid; see Appendix B for details. Note: if **-R**\ *grdfile* is used
    then the grid spacing has already been initialized; use **-I** to
    override the values.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    3 [or 4, see **-W**] column ASCII data table file(s) [or binary, see
    **-bi**\ [*ncols*\ ][*type*\ ]] holding (*x*,\ *y*,\ *z*\ [,*w*])
    data values. [*w*\ ] is an optional weight for the data. If no file
    is specified, **blockmode** will read from standard input.
**-C**
    Use the center of the block as the output location [Default uses the
    modal xy location (but see **-Q**)]. **-C** overrides **-Q**.
**-E**
    Provide Extended report which includes **s** (the L1 scale of the
    mode), **l**, the lowest value, and **h**, the high value for each
    block. Output order becomes
    *x*,\ *y*,\ *z*,\ *s*,\ *l*,\ *h*\ [,*w*]. [Default outputs
    *x*,\ *y*,\ *z*\ [,*w*]. See **-W** for *w* output.
**-E**\ **r**\ \|\ **s**\ [**-**\ ]
    Provide source id **s** or record number **r** output, i.e., append
    the source id or record number associated with the modal value. If
    tied then report the record number of the higher of the two values;
    append **-** to instead report the record number of the lower value.
    Note that both **-E** and **-E**\ **r**\ [**-**\ ] may be specified.
    For **-E**\ **s** we expect input records of the form
    *x*,\ *y*,\ *z*\ [,*w*],\ *sid*, where *sid* is an unsigned integer
    source id.
**-Q**
    (Quicker) Finds mode *z* and mean (*x*,\ *y*) [Default finds mode
    *x*, mode *y*, mode *z*].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**\ [**i**\ \|\ **o**]
    Weighted modifier[s]. Unweighted input and output has 3 columns
    *x*,\ *y*,\ *z*; Weighted i/o has 4 columns *x*,\ *y*,\ *z*,\ *w*.
    Weights can be used in input to construct weighted mean values in
    blocks. Weight sums can be reported in output for later combining
    several runs, etc. Use **-W** for weighted i/o, **-Wi** for weighted
    input only, **-Wo** for weighted output only. [Default uses
    unweighted i/o].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 3 (or 4 if **-Wi** is set)].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is 3 (or 4 if **-Wo** is set)].
    **-E** adds 3 additional columns.
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-r**
    Set pixel node registration [gridline]. Each block is the locus of
    points nearest the grid value location. For example, with
    **-R**\ 10/15/10/15 and and **-I**\ 1: with the **-r** option 10 <=
    (*x*,\ *y*) < 11 is one of 25 blocks; without it 9.5 <= (*x*,\ *y*)
    < 10.5 is one of 36 blocks.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Ascii Format Precision <#toc6>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Examples <#toc7>`_
-------------------

To find 5 by 5 minute block mode estimates from the double precision
binary data in hawaii\_b.xyg and output an ASCII table, run:

blockmode hawaii\_b.xyg -R198/208/18/25 -I5m -bi3d > hawaii\_5x5.xyg

`See Also <#toc8>`_
-------------------

`*blockmean*\ (1) <blockmean.html>`_ ,
`*blockmedian*\ (1) <blockmedian.html>`_ , `*gmt*\ (1) <gmt.html>`_ ,
`*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*nearneighbor*\ (1) <nearneighbor.html>`_ ,
`*surface*\ (1) <surface.html>`_ ,
`*triangulate*\ (1) <triangulate.html>`_
