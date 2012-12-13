*******
gravfft
*******

Section: Misc. Reference Manual Pages (l)
Updated: 14 Sep 2002
`Index <#index>`_ `Return to Main
Contents <http://localhost/cgi-bin/man/man2html>`_

--------------

 

NAME
----

gravfft - Compute gravitational attraction of 3-D surfaces and
admittance/coherence between grids (ATTENTION z positive up)  

SYNOPSIS
--------

**gravfft** T<te/rl/rm/rw> -C<n/wavelength/mean\_depth/tbw> -D<density>
-G<out\_grdfile> [-E<n\_terms>] [-F] -H [-L[m\|h]] [-l[n]]
-I<second\_file>[/<wbctk>] [-M] [-N<stuff>] [-Q] [-S] [-s<scale>]
-T[c\|a] [-V] -Z<zm>[/<zl>] [-z<cte>]  

DESCRIPTION
-----------

**gravfft** can be used into two main modes. First one computes the
gravity/geoid response of a bathymetry file. It will take the 2-D
forward FFT of a bathymetry grid and compute it's gravity/geoid response
using full Parker's method applyied to the chosen model. The available
models are the "loading from top", or elastic plate model, and the
"loading from below" which accounts for the plate's response to a
sub-surface load (apropriate for hot spot modeling - by the way, kill as
many as you can because if you leave on top of a hill and have a
geophysicist or geologist neighbour that don't like you he will propose
that your are siting on top of a new one). In both cases, the model
parameters are set with **-T** and **-Z** options. Second mode computes
the admittance or coherence between two grids. The output is the average
in the radial direction. Optionaly, the model admittance may also be
calculated. If **-T** option is used, the FFT transform of a grid is
writen as two grids. One for the Real part and the other for the
Imaginary part. The horizontal dimensions of the grdfiles are assumed to
be in meters. Geographical grids may be used by specifying the **-M**
option that scales degrees to meters. If you have grdfiles with
dimensions in km, you could change this to meters using **grdedit** or
scale the output with **grdmath**. Given the number of choices this
program offers, is difficult to state what are options and what are
required arguments. It depends on what you are doing.
        No space between the option flag and the associated arguments. Use upper case for the
option flags and lower case for modifiers.

*topo\_grd*

2-D binary grd file to be operated on.

**-T**

Computes the isostatic compensation due to topo load. Append elastic
thickness and densities of load, mantle, and water, all in SI units.
Give average mantle depth via **-Z**. If *te* = 0 then the Airy response
is returned. If the elastic thickness is > 1e10 it will be interpreted
as the flexural rigidity (by default it's computed from Te and Young
modulus, which is set in the program to be 1e+11). Also, number of
powers in Parker expansion is restricted to 1 (default's). See an
example further down.

**-C**

Compute only the theoretical admittance curves of the chosed model and
exit. *n* and *wavelength* are used to compute (*n* \* *wavelength*) the
total profile length in meters. *mean\_depth* is the mean water depth.
Append dataflags (one or two) of *tbw* in any order. **t** = use "from
top" model, **b** = use "from below" model. Optionally specify **w** to
write wavelength instead of frequency.

**-D**

Sets density contrast across surface. Used, for example, to compute the
gravity atraction of the water layer that can later be combined with the
free-air anomaly to get the Bouguer anomaly. In this case do not use
**-T**. It also implicitly sets **-L**

**-F**

compute geoid rather than gravity

**-G**

Specify the name of the output grd file for operations that do create
one.

**-H**

writes a grid with the Moho's gravity\|geoid effect from model selcted
by **-T** (and output by **-G**). **-H** implicitly sets **-L**

**-I**

Use *second\_file* and *topo\_grd* to estimate admittance or coherence
and write it to stdout (**-G** ignored if set). This grid should contain
gravity or geoid for the same region of *topo\_grd*. Default computes
admittance. Output contains 3 or 4 columns. Frequency (or wavelength),
admittance (or coherence) one sigma error bar (but don't beleave on it)
and, optionally, a theoretical admittance. Append dataflags (one to
three) of *tbwk* in any order. **t** = use "from top" model, **b** = use
"from below" model, **c** = computes coherence instead of admittance,
**k** = frequency (or wavelength) in units of 1/km (km).

**-Q**

writes out a grid with the flexural topography (with z positive up)
whose average was set by **-Z**\ *zm* and model parameters by **-T**
(and output by -G). That is the "gravimetric Moho". **-Q** implicitly
sets **-L**

**-S**

Computes predicted gravity or geoide grid due to a subplate load
produced by the current bathymetry and the theoretical model. The
necessary parametrs are set within **-T** and **-Z** options. The number
of powers in Parker expansion is restricted to 1 (default's). See an
example further down.

**-T**

Writes the FFT of the input grid in the form of two grids. One for the
real part and the other for the imaginary. The grid names are built from
the input grid name with \_real and \_imag appended. These grids have
the DC at the center and coordinates are the kx,ky frequencies.
Appending *a* or *c* will make the program write a csh script
(admit\_coher.sc) that computes, respectively, admittance or coherence
by an alternative (but much slower than the one used with -I) method
based on grid interpolations using grdtrack. Read comments/instructions
on the script.

**-Z**

Moho [and swell] average compensation depths. For the "load from top"
model you only have to provide *zm*, but for the "loading from below"
don't forget *zl*.

 

OPTIONS
-------

**-E**

number of terms used in Parker expansion (limit is 10, otherwise terms
depending on n will blow out the program) [Default = 1]

Leave trend alone. By default, a linear least squares plane trend will
be removed prior to the transform. It applies both to bathymetry as well
as *second\_file* [Default removes plane].

**-l**

Removes half-way from bathymetry data [Default removes mean]. Append *n*
to do not remove any constant from input bathymetry data.

**-M**

Map units. Choose this option if your grdfile is a geographical grid and
you want to convert degrees into meters. If the data are close to either
pole, you should consider projecting the grdfile onto a rectangular
coordinate system using **grdproject**.

**-N**

Choose or inquire about suitable grid dimensions for FFT.

**-Nf** will force the FFT to use the dimensions of the data.

**-Nq** will inquire about more suitable dimensions.

**-N<nx>/<ny>** will do FFT on array size *<nx>/<ny>* (Must be >=
grdfile size).

Default chooses dimensions >= data which optimize speed, accuracy of
FFT. If FFT dimensions > grdfile dimensions, data are extended and
tapered to zero.

**-Ns** will will print out a table with FFT suitable dimensions and
exits the program.

**-V**

Selects verbose mode, which will send progress reports to stderr
[Default runs "silently"].

**-z**

add a constant to the bathymetry data (not to *second\_file*) before
doing anything else. Pay attention to the sign of added constant,
remember z positive up.

 

EXAMPLES
--------

To compute the effect of the water layer above the bat.grd bathymetry
using 2600 and 1035 for the densities of crust and water and writing the
result on water\_g.grd (computing up to the sixth power of bathymetry in
Parker expansion):

**gravfft** bat.grd **-D**\ 1565 **-G**\ water\_g.grd **-E6**

Now subtract it to your free-air anomaly faa.grd and you'll get the
Bouguer anomaly. You may wonder why we are subtracting and not adding.
After all the Bouger anomaly pretends to correct the mass deficiency
presented by the water layer, so we should add because water is less
dense than the rocks below. The answer relyies on the way gravity
effects are computed by the Parker's method and pratical aspects of
using the FFT.

**grdmath** faa.grd water\_g.grd SUB = bouguer.grd

Want an MBA anomaly? Well compute the crust mantle contribution and add
it to the sea-bottom anomaly. Addmiting a 6 km thick crust of density
2600 and a mantle with 3300 density we could repeat the command used to
compute the water layer anomaly, using 700 (3300 - 2600) as the density
contrast. But we now have a problem because we need to know the mean
moho depth. That is when **-z** option comes in hand. Notice that we
didn't need to do that before because mean water depth was computed
directly from data. (notice also the negative sign of the argument to
**-z**, remember z positive up):

**gravfft** bat.grd **-D**\ 700 **-G**\ moho\_g.grd **-z**-6000

Now, add it to the sea-bottom anomaly to obtain the MBA anomaly. That
is:

**grdmath** water\_g.grd moho\_g.grd ADD = mba.grd

To compute the Moho gravity effect of an elastic plate bat.grd with Te =
7 km, density of 2600, over a mantle of density 3300, at an averge depth
of 9 km

**gravfft** bat.grd **-G**\ elastic.grd **-T**\ 7000/2600/3300/1035 -H
-Z9000

If you add now the sea-bottom and Moho's effects, you'll get the full
gravity response of your isostatic model. We will use here only the
first term in Parker expansion (default).

**gravfft** bat.grd **-D**\ 1565 **-G**\ water\_g.grd

**gravfft** bat.grd **-G**\ elastic.grd **-T**\ 7000/2600/3300/1035 -H
-Z9000

**grdmath** water\_g.grd elastic.grd ADD = model.grd

The same result can be obtained directly by the next command. However,
PAY ATTENTION to the following. I don't yet know if it's because of a
bug or due to some limitation (mine probably), but the fact is that the
following and the previous commands only give the same result if
**-E**\ 1 (the dafault) is used. For higher powers of bathymetry in
Parker expansion, only the above example seams to give the correct
result.

**gravfft** bat.grd **-G**\ model.grd **-T**\ 7000/2600/3300/1035 -Z9000
**-L**

And what would be the geoid anomaly produced by a load at 50 km depth,
below the a region whose bathymetry is given by bat.grd, a Moho at 9 km
depth and the same densities as before?

**gravfft** topo.grd **-G**\ swell\_geoid.grd
**-T**\ 7000/2600/3300/1035 **-F** **-Z**\ 9000/50000 **-S**

To compute the admittance between the topo.grd bathymetry and faa.grd
free-air anomaly grid using the elastic plate model of a crust of 6 km
mean thickness with 10 km efective elastic thickness in a region of 3 km
mean water depth:

**gravfft** topo.grd **-I**\ faa.grd/t **-T**\ 10000/2600/3300/1035
**-Z**\ 9000

To compute the admittance between the topo.grd bathymetry and geoid.grd
geoid grid with the "loading from below" (LFB) model with the same as
above and sub-surface load at 40 km, but assuming now the grids are in
geographic and we want wavelengths instead of frequency:

**gravfft** topo.grd **-I**\ geoid.grd/bw **-T**\ 10000/2600/3300/1035
**-Z**\ 9000/40000 **-M**

To compute the gravity theoretical admittance of a LFB along a 1000 km
long profile using the same parameters as above

**gravfft** **-C**\ 400/5000/3000/b **-T**\ 10000/2600/3300/1035
**-Z**\ 9000/40000

To write the FFT transform (in the form of two grids) of the faa.grd
grid and also a csh script to compute the coherence by the alternative
way (read instructions in admit\_coher.sc):

**gravfft** faa.grd **-T**\ c

Want a grid with the gravity power spectrum? That's easy now (after
having run the previous command)

**grdmath** faa\_real.grd faa\_imag R2 = power.grd  

SEE ALSO
--------

*`gmt <http://localhost/cgi-bin/man/man2html?l+gmt>`_*\ (l),
*`grdedit <http://localhost/cgi-bin/man/man2html?l+grdedit>`_*\ (l),
*`grdmath <http://localhost/cgi-bin/man/man2html?l+grdmath>`_*\ (l),
*`grdproject <http://localhost/cgi-bin/man/man2html?l+grdproject>`_*\ (l)
 

REFERENCES
----------

--------------

 

Index
-----

`NAME <#lbAB>`_

`SYNOPSIS <#lbAC>`_

`DESCRIPTION <#lbAD>`_

`OPTIONS <#lbAE>`_

`EXAMPLES <#lbAF>`_

`SEE ALSO <#lbAG>`_

`REFERENCES <#lbAH>`_

--------------

This document was created by
`man2html <http://localhost/cgi-bin/man/man2html>`_, using the manual
pages.
 Time: GMT, February 09, 2005
