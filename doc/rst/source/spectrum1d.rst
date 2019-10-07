.. index:: ! spectrum1d

**********
spectrum1d
**********

.. only:: not man

    Compute auto- [and cross- ] spectra from one [or two] time-series

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt spectrum1d** [ *table* ] |-S|\ *segment_size*]
[ |-C|\ [**xycnpago**] ] [ |-D|\ *dt* ] [ |-L|\ [**h**\ \|\ **m**] ]
[ |-N|\ [\ *name_stem* ] ] [ |-T| ] [ |-W| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**spectrum1d** reads X [and Y] values from the first [and second]
columns on standard input [or *x[y]file*]. These values are treated as
timeseries X(t) [Y(t)] sampled at equal intervals spaced *dt* units
apart. There may be any number of lines of input. **spectrum1d** will
create file[s] containing auto- [and cross- ] spectral density estimates
by Welch's method of ensemble averaging of multiple overlapped windows,
using standard error estimates from Bendat and Piersol.

The output files have 3 columns: f or w, p, and e. f or w is the
frequency or wavelength, p is the spectral density estimate, and e is
the one standard deviation error bar size. These files are named based
on *name_stem*. If the **-C** option is used, up to eight files are
created; otherwise only one (xpower) is written. The files (which are
ASCII unless **-bo** is set) are as follows:

*name_stem*.xpower
    Power spectral density of X(t). Units of X \* X \* *dt*.
*name_stem*.ypower
    Power spectral density of Y(t). Units of Y \* Y \* *dt*.
*name_stem*.cpower
    Power spectral density of the coherent output. Units same as ypower.
*name_stem*.npower
    Power spectral density of the noise output. Units same as ypower.
*name_stem*.gain
    Gain spectrum, or modulus of the transfer function. Units of (Y / X).
*name_stem*.phase
    Phase spectrum, or phase of the transfer function. Units are
    radians.
*name_stem*.admit
    Admittance spectrum, or real part of the transfer function. Units of
    (Y / X).
*name_stem*.coh
    (Squared) coherency spectrum, or linear correlation coefficient as a
    function of frequency. Dimensionless number in [0, 1]. The
    Signal-to-Noise-Ratio (SNR) is coh / (1 - coh). SNR = 1 when coh = 0.5. 

In addition, a single file with all of the above as individual columns will
be written to *stdout* (unless disabled via **-T**).

Required Arguments
------------------

.. _-S:

**-S**\ *segment_size*]
    *segment_size* is a radix-2 number of samples per window for
    ensemble averaging. The smallest frequency estimated is
    1.0/(\ *segment_size* \* *dt*), while the largest is 1.0/(2 \*
    *dt*). One standard error in power spectral density is approximately
    1.0 / sqrt(\ *n_data* / *segment_size*), so if *segment_size* =
    256, you need 25,600 data to get a one standard error bar of 10%.
    Cross-spectral error bars are larger and more complicated, being a
    function also of the coherency.

Optional Arguments
------------------

*table*
    One or more ASCII (or binary, see **-bi**)
    files holding X(t) [Y(t)] samples in the first 1 [or 2] columns. If
    no files are specified, **spectrum1d** will read from standard input.

.. _-C:

**-C**\ [**xycnpago**]
    Read the first two columns of input as samples of two time-series,
    X(t) and Y(t). Consider Y(t) to be the output and X(t) the input in
    a linear system with noise. Estimate the optimum frequency response
    function by least squares, such that the noise output is minimized
    and the coherent output and the noise output are uncorrelated.
    Optionally specify up to 8 letters from the set { **x y c n p a g
    o** } in any order to create only those output files instead of the
    default [all]. **x** = xpower, **y** = ypower, **c** = cpower, **n**
    = npower, **p** = phase, **a** = admit, **g** = gain, **o** = coh.

.. _-D:

**-D**\ *dt*
    *dt* Set the spacing between samples in the time-series [Default = 1].

.. _-L:

**-L**
    Leave trend alone. By default, a linear trend will be removed prior
    to the transform. Alternatively, append **m** to just remove the
    mean value or **h** to remove the mid-value.

.. _-N:

**-N**\ [*name\_stem*]
    Supply an alternate name stem to be used for each individual output file [Default = "spectrum"].
    If **-N** is given with no argument then we disable the writing of individual
    output files and instead write a single composite results table to standard output.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-T:

**-T**
    Disable the writing of a single composite results table to stdout.  Only individual output
    files for each selected component (see **-C**) will be written.

.. _-W:

**-W**
    Write Wavelength rather than frequency in column 1 of the output
    file[s] [Default = frequency, (cycles / *dt*)]. 

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is 2 output columns]. 
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

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

    paste data.t data.g | gmt spectrum1d -S256 -D1.5 -Ndata -C > results.txt

Tutorial
--------

The output of spectrum1d is in units of power spectral density, and so to get units
of data-squared you must divide by delta_t, where delta_t is the sample spacing.
(There may be a factor of 2 pi somewhere, also. If you want to be sure of the
normalization, you can determine a scale factor from Parseval's theorem: the sum of
the squares of your input data should equal the sum of the squares of the outputs
from spectrum1d, if you are simply trying to get a periodogram. [See below.])

Suppose we simply take a data set, x(t), and compute the discrete Fourier transform
(DFT) of the entire data set in one go. Call this X(f). Then suppose we form X(f)
times the complex conjugate of X(f).

P_raw(f) = X(f) * X'(f), where the ' indicates complex conjugation.

P_raw is called the periodogram. The sum of the samples of the periodogram equals the
sum of the samples of the squares of x(t), by Parseval's theorem. (If you use a DFT
subroutine on a computer, usually the sum of P_raw equals the sum of x-squared, times M,
where M is the number of samples in x(t).)

Each estimate of X(f) is now formed by a weighted linear combination of all of the
x(t) values. (The weights are sometimes called "twiddle factors" in the DFT literature.)
So, no matter what the probability distribution for the x(t) values is, the probability
distribution for the X(f) values approaches [complex] Gaussian, by the Central Limit
Theorem. This means that the probability distribution for P_raw(f) approaches chi-squared
with two degrees of freedom. That reduces to an exponential distribution, and the
variance of the estimate of P_raw is proportional to the square of the mean, that is,
the expected value of P_raw.

In practice if we form P_raw, the estimates are hopelessly noisy. Thus P_raw is not useful,
and we need to do some kind of smoothing or averaging to get a useful estimate, P_useful(f).

There are several different ways to do this in the literature. One is to form P_raw and
then smooth it. Another is to form the auto-covariance function of x(t), smooth, taper and
shape it, and then take the Fourier transform of the smoothed, tapered and shaped auto-covariance.
Another is to form a parametric model for the auto-correlation structure in x(t), then compute
the spectrum of that model. This last approach is what is done in what is called the
"maximum entropy" or "Berg" or "Box-Jenkins" or "ARMA" or "ARIMA" methods.

Welch's method is a tried-and-true method. In his method, you choose a segment length,
**-S**\ *N*, so that estimates will be made from segments of length *N*. The frequency samples
(in cycles per delta_t unit) of your P_useful will then be at *k* /(*N* \* *delta_t*), 
where *k* is an integer, and you will get *N* samples (since the spectrum is an even
function of *f*, only *N*/2 of them are really useful). If the length of your entire
data set, x(t), is *M* samples long, then the variance in your P_useful will decrease
in proportion to *N/M*. Thus you need to choose *N* << *M* to get very low noise and
high confidence in P_useful. There is a trade-off here; see below.

There is an additional reduction in variance in that Welch's method uses a Von Hann
spectral window on each sample of length *N*. This reduces side lobe leakage and has
the effect of smoothing the (*N* segment) periodogram as if the X(f) had been
convolved with [1/4, 1/2, 1/4] prior to forming P_useful. But this slightly widens
the spectral bandwidth of each estimate, because the estimate at frequency sample *k*
is now a little correlated with the estimate at frequency sample k+1. (Of course this
would also happen if you simply formed P_raw and then smoothed it.)

Finally, Welch's method also uses overlapped processing. Since the Von Hann window is
large in the middle and tapers to near zero at the ends, only the middle of the segment
of length *N* contributes much to its estimate. Therefore in taking the next segment
of data, we move ahead in the x(t) sequence only *N*/2 points. In this way, the next
segment gets large weight where the segments on either side of it will get little weight,
and vice versa. This doubles the smoothing effect and ensures that (if *N* << *M*)
nearly every point in x(t) contributes with nearly equal weight in the final answer.

Welch's method of spectral estimation has been widely used and widely studied. It is very
reliable and its statistical properties are well understood. It is highly recommended in
such textbooks as "Random Data: Analysis and Measurement Procedures" by Bendat and Piersol.

In all problems of estimating parameters from data, there is a classic trade-off between
resolution and variance. If you want to try to squeeze more resolution out of your data
set, then you have to be willing to accept more noise in the estimates. The same trade-off
is evident here in Welch's method. If you want to have very low noise in the spectral
estimates, then you have to choose *N* << *M*, and this means that you get only *N*
samples of the spectrum, and the longest period that you can resolve is only *N* \* *delta_t*.
So you see that reducing the noise lowers the number of spectral samples and lowers the
longest period. Conversely, if you choose *N* approaching *M*, then you approach the
periodogram with its very bad statistical properties, but you get lots of samples and
a large fundamental period.

The other spectral estimation methods also can do a good job. Welch's method was selected
because the way it works, how one can code it, and its effects on statistical distributions,
resolution, side-lobe leakage, bias, variance, etc. are all easily understood. Some of the
other methods (e.g. Maximum Entropy) tend to hide where some of these trade-offs are
happening inside a "black box".


See Also
--------

:doc:`gmt`, :doc:`grdfft`

References
----------

Bendat, J. S., and A. G. Piersol, 1986, Random Data, 2nd revised ed., John Wiley & Sons.

Welch, P. D., 1967, The use of Fast Fourier Transform for the estimation
of power spectra: a method based on time averaging over short, modified
periodograms, IEEE Transactions on Audio and Electroacoustics, Vol AU-15, No 2.
