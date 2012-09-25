**********
spectrum1d
**********

spectrum1d - Compute auto- [and cross- ] spectra from one [or two]
time-series

`Synopsis <#toc1>`_
-------------------

**spectrum1d** [ *table* ] **-S**\ *segment\_size*] [
**-C**\ [**xycnpago**\ ] ] [ **-D**\ *dt* ] [
**-N**\ [**+**\ ]\ *name\_stem* ] [ **-V**\ [*level*\ ] ] [ **-W** ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
]

`Description <#toc2>`_
----------------------

**spectrum1d** reads X [and Y] values from the first [and second]
columns on standard input [or *x[y]file*]. These values are treated as
timeseries X(t) [Y(t)] sampled at equal intervals spaced *dt* units
apart. There may be any number of lines of input. **spectrum1d** will
create file[s] containing auto- [and cross- ] spectral density estimates
by Welch’s method of ensemble averaging of multiple overlapped windows,
using standard error estimates from Bendat and Piersol.

The output files have 3 columns: f or w, p, and e. f or w is the
frequency or wavelength, p is the spectral density estimate, and e is
the one standard deviation error bar size. These files are named based
on *name\_stem*. If the **-C** option is used, up to eight files are
created; otherwise only one (xpower) is written. The files (which are
ASCII unless **-bo**\ [*ncols*\ ][*type*\ ] is set) are as follows:

*name\_stem*.xpower
    Power spectral density of X(t). Units of X \* X \* *dt*.
*name\_stem*.ypower
    Power spectral density of Y(t). Units of Y \* Y \* *dt*.
*name\_stem*.cpower
    Power spectral density of the coherent output. Units same as ypower.
*name\_stem*.npower
    Power spectral density of the noise output. Units same as ypower.
*name\_stem*.gain
    Gain spectrum, or modulus of the transfer function. Units of (Y /
    X).
*name\_stem*.phase
    Phase spectrum, or phase of the transfer function. Units are
    radians.
*name\_stem*.admit
    Admittance spectrum, or real part of the transfer function. Units of
    (Y / X).
*name\_stem*.coh
    (Squared) coherency spectrum, or linear correlation coefficient as a
    function of frequency. Dimensionless number in [0, 1]. The
    Signal-to-Noise-Ratio (SNR) is coh / (1 - coh). SNR = 1 when coh =
    0.5.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-S**\ *segment\_size*]
    *segment\_size* is a radix-2 number of samples per window for
    ensemble averaging. The smallest frequency estimated is
    1.0/(\ *segment\_size* \* *dt*), while the largest is 1.0/(2 \*
    *dt*). One standard error in power spectral density is approximately
    1.0 / sqrt(\ *n\_data* / *segment\_size*), so if *segment\_size* =
    256, you need 25,600 data to get a one standard error bar of 10%.
    Cross-spectral error bars are larger and more complicated, being a
    function also of the coherency.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    files holding X(t) [Y(t)] samples in the first 1 [or 2] columns. If
    no files are specified, **spectrum1d** will read from standard
    input.
**-C**\ [**xycnpago**\ ]
    Read the first two columns of input as samples of two time-series,
    X(t) and Y(t). Consider Y(t) to be the output and X(t) the input in
    a linear system with noise. Estimate the optimum frequency response
    function by least squares, such that the noise output is minimized
    and the coherent output and the noise output are uncorrelated.
    Optionally specify up to 8 letters from the set { **x y c n p a g
    o** } in any order to create only those output files instead of the
    default [all]. **x** = xpower, **y** = ypower, **c** = cpower, **n**
    = npower, **p** = phase, **a** = admit, **g** = gain, **o** = coh.
**-D**\ *dt*
    *dt* Set the spacing between samples in the time-series [Default =
    1].
**-N**\ [**+**\ ]\ *name\_stem*
    *name\_stem* Supply the name stem to be used for output files
    [Default = "spectrum"]. To place all the computed output columns in
    a single table, use **-N+**.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**
    Write Wavelength rather than frequency in column 1 of the output
    file[s] [Default = frequency, (cycles / *dt*)].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is 2 output columns].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
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

Suppose data.g is gravity data in mGal, sampled every 1.5 km. To write
its power spectrum, in mGal\*\*2-km, to the file data.xpower, use

spectrum1d data.g -S256 -D1.5 -Ndata

Suppose in addition to data.g you have data.t, which is topography in
meters sampled at the same points as data.g. To estimate various
features of the transfer function, considering data.t as input and
data.g as output, use

paste data.t data.g \| spectrum1d -S256 -D1.5 -Ndata -C

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdfft*\ (1) <grdfft.html>`_

`References <#toc9>`_
---------------------

Bendat, J. S., and A. G. Piersol, 1986, Random Data, 2nd revised ed.,
John Wiley & Sons.

Welch, P. D., 1967, The use of Fast Fourier Transform for the estimation
of power spectra: a method based on time averaging over short, modified
periodograms, IEEE Transactions on Audio and Electroacoustics, Vol
AU-15, No 2.
