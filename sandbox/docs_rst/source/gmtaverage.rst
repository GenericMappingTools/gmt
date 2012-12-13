**********
gmtaverage
**********

gmtaverage - Block average (*x*,\ *y*,\ *z*) data tables by mean,
median, or mode estimation

`Synopsis <#toc1>`_
-------------------

**gmtaverage** [ *xyz[w]file(s)* ]
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ]
**-Te**\ \|\ **m**\ \|\ **n**\ \|\ **o**\ \|\ **s**\ \|\ **w**\ \|\ *quantile*
[ **-C** ] [ **-E**\ [**b**\ ] ] [ **-Q** ] [ **-V**\ [*level*\ ] ] [
**-W**\ [**io**\ ] ] [ **-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**]
] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**gmtaverage** reads arbitrarily located (*x*,\ *y*,\ *z*) triples [or
optionally weighted quadruples (*x*,\ *y*,\ *z*,\ *w*)] from standard
input [or *xyz[w]file(s)*] and writes to standard output an average
position and value for every non-empty block in a grid region defined by
the **-R** and **-I** arguments. **gmtaverage** should be used as a
pre-processor before running **surface** to avoid aliasing short
wavelengths, but is also generally useful for decimating or averaging
(*x*,\ *y*,\ *z*) data. You can modify the precision of the output
format by editing the **FORMAT\_FLOAT\_OUT** parameter in your
**gmt.conf** file, or you may choose binary input and/or output using
single or double precision storage.

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
**-T**
    Specify the operator to use when computing output values. Choose
    among **e** (median), **m** (mean), **n** (number of points), **o**
    (mode), **s** (sum), **w** (weight sum), or the *quantile* of the
    distribution to be returned. Here, 0 < *quantile* < 1.

`Optional Arguments <#toc5>`_
-----------------------------

*xyz[w]file(s)*
    3 [or 4, see **-W**] column ASCII data table file(s) [or binary, see
    **-bi**\ [*ncols*\ ][*type*\ ]] holding (*x*,\ *y*,\ *z*\ [,*w*])
    data values. [*w*\ ] is an optional weight for the data. If no file
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
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**\ [**io**\ ]
    Weighted modifier[s]. Unweighted input and output has 3 columns
    *x*,\ *y*,\ *z*; Weighted i/o has 4 columns *x*,\ *y*,\ *z*,\ *w*.
    Weights can be used in input to construct weighted output values in
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

To find 5 by 5 minute block mode values from the double precision binary
data in hawaii\_b.xyg and output an ASCII table, run

gmtaverage hawaii\_b.xyg -R198/208/18/25 -I5m -To -bi3 > hawaii\_5x5.xyg

To find 5 by 5 minute block mean values from the same file, run

gmtaverage hawaii.xyg -R198/208/18/25 -I5m -Tm > hawaii\_5x5.xyg

To find the number of data points in each 5 by 5 minute block from the
same file, run

gmtaverage hawaii.xyg -R198/208/18/25 -I5m -Tn > hawaii\_5x5.xyn

To compute the shape of a data distribution per bin via a
box-and-whisker diagram we need the 0%, 25%, 50%, 75%, and 100%
quantiles. To do so on a global 5 by 5 degree basis from the ASCII table
depths.xyz and send output to an ASCII table, run

gmtaverage depths.xyz -Rg -I5 -Te -Eb -r > depths\_5x5.txt

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*nearneighbor*\ (1) <nearneighbor.html>`_ ,
`*surface*\ (1) <surface.html>`_ ,
`*triangulate*\ (1) <triangulate.html>`_
