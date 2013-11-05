.. index:: ! spectrum1d

**********
spectrum1d
**********

.. only:: not man

    spectrum1d - Compute auto- [and cross- ] spectra from one [or two] time-series

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**spectrum1d** [ *table* ] **-S**\ *segment\_size*] [
**-C**\ [**xycnpago**\ ] ] [ **-D**\ *dt* ] [ **-L**\ [**h**\ \|\ **m**]
] [ **-N**\ [**+**\ ]\ *name\_stem* ] [ |SYN_OPT-b| ] [ **-W** ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]

|No-spaces|

Description
-----------

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
ASCII unless **-bo** is set) are as follows:

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
    Signal-to-Noise-Ratio (SNR) is coh / (1 - coh). SNR = 1 when coh = 0.5. 

Required Arguments
------------------

**-S**\ *segment\_size*]
    *segment\_size* is a radix-2 number of samples per window for
    ensemble averaging. The smallest frequency estimated is
    1.0/(\ *segment\_size* \* *dt*), while the largest is 1.0/(2 \*
    *dt*). One standard error in power spectral density is approximately
    1.0 / sqrt(\ *n\_data* / *segment\_size*), so if *segment\_size* =
    256, you need 25,600 data to get a one standard error bar of 10%.
    Cross-spectral error bars are larger and more complicated, being a
    function also of the coherency.

Optional Arguments
------------------

*table*
    One or more ASCII (or binary, see **-bi**)
    files holding X(t) [Y(t)] samples in the first 1 [or 2] columns. If
    no files are specified, **spectrum1d** will read from standard input.

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
    *dt* Set the spacing between samples in the time-series [Default = 1].

**-L**
    Leave trend alone. By default, a linear trend will be removed prior
    to the transform. Alternatively, append **m** to just remove the
    mean value or **h** to remove the mid-value.

**-N**\ [**+**]\ *name\_stem*
    *name\_stem* Supply the name stem to be used for output files
    [Default = "spectrum"]. To place all the computed output columns in
    a single table, use **-N+**. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-W**
    Write Wavelength rather than frequency in column 1 of the output
    file[s] [Default = frequency, (cycles / *dt*)]. 

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 2 output columns]. 
.. include:: explain_-bo.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Examples
--------

Suppose data.g is gravity data in mGal, sampled every 1.5 km. To write
its power spectrum, in mGal\*\*2-km, to the file data.xpower, use

   ::

    gmt spectrum1d data.g -S256 -D1.5 -Ndata

Suppose in addition to data.g you have data.t, which is topography in
meters sampled at the same points as data.g. To estimate various
features of the transfer function, considering data.t as input and
data.g as output, use

   ::

    paste data.t data.g | gmt spectrum1d -S256 -D1.5 -Ndata -C

See Also
--------

:doc:`gmt`, :doc:`grdfft`

References
----------

Bendat, J. S., and A. G. Piersol, 1986, Random Data, 2nd revised ed.,
John Wiley & Sons.

Welch, P. D., 1967, The use of Fast Fourier Transform for the estimation
of power spectra: a method based on time averaging over short, modified
periodograms, IEEE Transactions on Audio and Electroacoustics, Vol
AU-15, No 2.
