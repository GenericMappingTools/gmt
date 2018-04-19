.. index:: ! sph2grd

*******
sph2grd
*******

.. only:: not man

    Compute grid from spherical harmonic coefficients

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt sph2grd** [ *table* ] |-G|\ *grdfile*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-D|\ [**g**\ \|\ **n**] ]
[ |-E| ]
[ |-F|\ [**k**]\ *filter* ]
[ |-N|\ [*norm*] ]
[ |-Q| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**sph2grd** reads a spherical harmonics coefficient table with records of
L, M, C[L,M], S[L,M] and evaluates the spherical harmonic model on the specified grid. 

Required Arguments
------------------

.. _-G:

**-G**\ *grdfile*
    *grdfile* is the name of the binary output grid file. (See GRID FILE FORMAT below.) 

.. _-I:

.. include:: explain_-I.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

Optional Arguments
------------------

*table*
    One or more ASCII [or binary, see **-bi**]
    files holding the spherical harmonic coefficients. We expect the
    first four columns to hold the degree L, the order M, followed by
    the cosine and sine coefficients.

.. _-D:

**-D**\ [**g**\ \|\ **n**]
    Will evaluate a derived field from a geopotential model.  Choose
    between **Dg** which will compute the gravitational field or **Dn**
    to compute the geoid [Add **-E** for anomalies on the ellipsoid].

.. _-E:

**-E**
    Evaluate expansion on the current ellipsoid [Default is sphere].

.. _-F:

**-F**\ [**k**]\ *filter*
    Filter coefficients according to one of two kinds of filter
    specifications:.  Select **-Fk** if values are given in km
    [Default is coefficient harmonic degree L]. a) Cosine band-pass: Append
    four wavelengths *lc/lp/hp/hc*.  Coefficients outside *lc/hc* are
    cut; those inside *lp/hp* are passed, while the rest are tapered.
    Replace wavelength by - to skip, e.g., **-F**-/-/50/75 is a
    low-pass filter.  b) Gaussian band-pass: Append two wavelengths
    *lo/hi* where filter amplitudes = 0.5.  Replace wavelength by -
    to skip, e.g., **-F**\ 70/- is a high-pass Gaussian filter.

.. _-N:

**-N**\ [*norm*]
    Normalization used for coefficients.  Choose among **m**: Mathematical
    normalization - inner products summed over surface equal 1 [Default].
    **g** Geodesy normalization - inner products summed over surface
    equal 4pi. **s**: Schmidt normalization - as used in geomagnetism.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 4 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_core.rst_

.. include:: explain_help.rst_

.. include:: explain_float.rst_

.. include:: explain_grd_output.rst_

.. include:: explain_grd_coord.rst_

Examples
--------

To create a 1 x 1 degree global grid file from the ASCII
coefficients in EGM96_to_360.txt, use

   ::

    gmt sph2grd EGM96_to_360.txt -GEGM96_to_360.nc -Rg -I1 -V

Reference
---------

Holmes, S. A., and Featherstone, W. E., 2002, A unified approach to
the Clenshaw summation and the recursive computation of very high
degree and order normalized associated Legendre functions:
*J. Geodesy, v. 76, p. 279-299*.

See Also
--------

:doc:`gmt`,
:doc:`grdfft`,
:doc:`grdmath`
