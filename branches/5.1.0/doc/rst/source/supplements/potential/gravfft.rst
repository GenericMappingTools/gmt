.. index:: ! gravfft

*******
gravfft
*******

.. only:: not man

    gravfft - Compute gravitational attraction of 3-D surfaces in the wavenumber (or frequency) domain

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gravfft** *ingrid* [ *ingrid2* ] **-G**\ *outfile*
[ **-C**\ <n/wavelength/mean\_depth/tbw> ] [ **-A**\ *z\_offset* ] [ **-D**\ <density> ]
[ **-E**\ <n\_terms> ] [ **-F**\ [f\|g\|v\|n\|e] ] [ **-I**\ <wbctk> ]
**-N**\ [**f**\ \|\ **q**\ \|\ **s**\ \|\ *nx/ny*][**+a**\ \|\ **d**\ \|\ **h** \|\ **l**][**+e**\ \|\ **n**\ \|\ **m**][**+t**\ *width*][**+w**\ [*suffix*]][\ **+z**\ [**p**]]
[ **-Q** ] [ **-T**\ <te/rl/rm/rw>[+m] ]
[ |SYN_OPT-V| ]
[ **-Z**\ <zm>[/<zl>] ]
[ **-fg** ]

|No-spaces|

Description
-----------

**gravfft** can be used into two main modes. First one computes the
gravity/geoid response of a bathymetry file. It will take the 2-D
forward FFT of a bathymetry grid and compute it's gravity/geoid response
using full Parker's method applied to the chosen model. The available
models are the “loading from top”, or elastic plate model, and the
“loading from below” which accounts for the plate's response to a
sub-surface load (appropriate for hot spot modeling - if you believe
them). In both cases, the model parameters are set with **-T** and
**-Z** options. Second mode computes the admittance or coherence between
two grids. The output is the average in the radial direction.
Optionally, the model admittance may also be calculated. The horizontal
dimensions of the grdfiles are assumed to be in meters. Geographical
grids may be used by specifying the **-fg** option that scales degrees
to meters. If you have grids with dimensions in km, you could change
this to meters using :doc:`grdedit </grdedit>` or scale the output with
:doc:`grdmath </grdmath>`.
Given the number of choices this program offers, is difficult to state
what are options and what are required arguments. It depends on what you
are doing.

Required Arguments
------------------

*ingrid*
    2-D binary grid file to be operated on. (See GRID FILE FORMATS below).
    For cross-spectral operations, also give the second grid file *ingrd2*.
**-G**\ *outfile*
    Specify the name of the output grid file or the 1-D spectrum table
    (see **-E**). (See GRID FILE FORMATS below).

Optional Arguments
------------------

**-C**\ *<n/wavelength/mean\_depth/tbw>*
    Compute only the theoretical admittance curves of the selected model
    and exit. *n* and *wavelength* are used to compute (n \* wavelength)
    the total profile length in meters. *mean\_depth* is the mean water
    depth. Append dataflags (one or two) of *tbw* in any order. *t* =
    use “from top” model, *b* = use "from below" model. Optionally
    specify *w* to write wavelength instead of frequency.
**-D**\ *density*
    Sets density contrast across surface. Used, for example, to compute
    the gravity attraction of the water layer that can later be combined
    with the free-air anomaly to get the Bouguer anomaly. In this case
    do not use **-T**. It also implicitly sets **-N+a**
**-E**\ *n\_terms*
    Number of terms used in Parker expansion (limit is 10, otherwise
    terms depending on n will blow out the program) [Default = 1]
**-F**\ [**f**\ \|\ **g**\ \|\ **v**\ \|\ **n**\ \|\ **e**]
    Specify desired geopotential field: compute geoid rather than gravity

|
|       **f** = Free-air anomalies (mGal) [Default].
|       **g** = Geoid anomalies (m).
|       **v** = Vertical Gravity Gradient (VGG; 1 Eovtos = 0.1 mGal/km).
|       **e** = East deflections of the vertical (micro-radian).
|       **n** = North deflections of the vertical (micro-radian).

**-I**\ *<wbctk>*
    Use <ingrid2> and <topo_grd> to estimate admittance\|coherence and
    write it to stdout (-G ignored if set). This grid should contain
    gravity or geoid for the same region of <topo_grd>. Default
    computes admittance. Output contains 3 or 4 columns. Frequency
    (wavelength), admittance (coherence) one sigma error bar and,
    optionally, a theoretical admittance. Append dataflags (one to
    three) of wbct. **w** writes wavelength instead of wavenumber **k**
    Use km or wavelength unit [m] **c** computes coherence instead of
    admittance **b** writes a forth column with "loading from below"
    theoretical admittance **t** writes a forth column with "elastic
    plate" theoretical admittance

.. include:: ../../explain_fft.rst_

**-Q**
    Writes out a grid with the flexural topography (with z positive up)
    whose average was set by **-Z**\ *zm* and model parameters by **-T**
    (and output by **-G**). That is the "gravimetric Moho". **-Q**
    implicitly sets **-N+a**
**-S**
    Computes predicted gravity or geoid grid due to a subplate load
    produced by the current bathymetry and the theoretical model. The
    necessary parameters are set within **-T** and **-Z** options. The
    number of powers in Parker expansion is restricted to 1 (default's).
    See an example further down.
**-T**\ *<te/rl/rm/rw>[+m]*
    Compute the isostatic compensation from the topography load (input grid file) on
    an elastic plate of thickness *te*. Also append densities for load, mantle, and
    water in SI units. Give average mantle depth via **-Z**. If the elastic thickness
    is > 1e10 it will be interpreted as the flexural rigidity (by default it is
    computed from *te* and Young modulus). Optionaly, append *+m* to write a grid
    with the Moho's geopotential effect (see **-F**) from model selected by **-T**. 
    If *te* = 0 then the Airy response is returned. **-T** implicitly sets **-N+a**
**-Z**\ *<zm>[/<zl>]*
    Moho [and swell] average compensation depths. For the “load from
    top” model you only have to provide *zm*, but for the “loading from
    below” don't forget *zl*.

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

**-fg**
   Geographic grids (dimensions of longitude, latitude) will be converted to
   meters via a "Flat Earth" approximation using the current ellipsoid parameters.

.. include:: ../../explain_help.rst_

.. include:: ../../explain_grd_inout.rst_

Grid Distance Units
-------------------

If the grid does not have meter as the horizontal unit, append **+u**\ *unit*
to the input file name to convert from the specified unit to meter.  If your
grid is geographic, convert distances to meters by supplying **-fg** instead.

Considerations
--------------

netCDF COARDS grids will automatically be recognized as geographic. For
other grids geographical grids were you want to convert degrees into
meters, select **-fg**. If the data are close to either pole, you should
consider projecting the grid file onto a rectangular coordinate system
using **grdproject**.

Examples
--------

To compute the effect of the water layer above the bat.grd bathymetry
using 2700 and 1035 for the densities of crust and water and writing the
result on water_g.grd (computing up to the fourth power of bathymetry
in Parker expansion):

   ::

    gmt gravfft bat.grd -D1665 -Gwater_g.grd -E4

Now subtract it to your free-air anomaly faa.grd and you’ll get the
Bouguer anomaly. You may wonder why we are subtracting and not adding.
After all the Bouger anomaly pretends to correct the mass deficiency
presented by the water layer, so we should add because water is less
dense than the rocks below. The answer relyies on the way gravity
effects are computed by the Parker's method and practical aspects of
using the FFT.

   ::

    gmt grdmath faa.grd water_g.grd SUB = bouguer.grd

Want an MBA anomaly? Well compute the crust mantle contribution and add
it to the sea-bottom anomaly. Assuming a 6 km thick crust of density
2700 and a mantle with 3300 density we could repeat the command used to
compute the water layer anomaly, using 600 (3300 - 2700) as the density
contrast. But we now have a problem because we need to know the mean
moho depth. That is when **-A** option comes in hand. Notice that we
didn't need to do that before because mean water depth was computed
directly from data. (notice also the negative sign of the argument to
**-A**, remember z positive up):

   ::

    gmt gravfft bat.grd -D600 -Gmoho_g.grd -A-6000

Now, add it to the sea-bottom anomaly to obtain the MBA anomaly. That is:

   ::

    gmt grdmath water_g.grd moho_g.grd ADD = mba.grd

To compute the Moho gravity effect of an elastic plate bat.grd with Te =
7 km, density of 2700, over a mantle of density 3300, at an averge depth
of 9 km

   ::

    gmt gravfft bat.grd -Gelastic.grd -T7000/2700/3300/1035+m -Z9000

If you add now the sea-bottom and Moho’s effects, you’ll get the full
gravity response of your isostatic model. We will use here only the
first term in Parker expansion (default).

   ::

    gmt gravfft bat.grd -D1665 -Gwater_g.grd
    gmt gravfft bat.grd -Gelastic.grd -T7000/2700/3300/1035+m -Z9000
    gmt grdmath water_g.grd elastic.grd ADD = model.grd

The same result can be obtained directly by the next command. However,
PAY ATTENTION to the following. I don't yet know if it's because of a
bug or due to some limitation, but the fact is that the following and
the previous commands only give the same result if **-E**\ 1 (the
default) is used. For higher powers of bathymetry in Parker expansion,
only the above example seams to give the correct result.

   ::

    gmt gravfft bat.grd -Gmodel.grd -T7000/2700/3300/1035 -Z9000

And what would be the geoid anomaly produced by a load at 50 km depth,
below the a region whose bathymetry is given by bat.grd, a Moho at 9 km
depth and the same densities as before?

   ::

    gmt gravfft topo.grd -Gswell_geoid.grd -T7000/2700/3300/1035 -Fg -Z9000/50000 -S

To compute the admittance between the topo.grd bathymetry and faa.grd
free-air anomaly grid using the elastic plate model of a crust of 6 km
mean thickness with 10 km efective elastic thickness in a region of 3 km
mean water depth:

   ::

    gmt gravfft topo.grd faa.grd -It -T10000/2700/3300/1035 -Z9000

To compute the admittance between the topo.grd bathymetry and geoid.grd
geoid grid with the "loading from below" (LFB) model with the same as
above and sub-surface load at 40 km, but assuming now the grids are in
geographic and we want wavelengths instead of frequency:

   ::

    gmt gravfft topo.grd geoid.grd -Ibw -T10000/2700/3300/1035 -Z9000/40000 -fg

To compute the gravity theoretical admittance of a LFB along a 1000 km
long profile using the same parameters as above

   ::

    gmt gravfft -C400/5000/3000/b -T10000/2700/3300/1035 -Z9000/40000

References
----------

Luis, J.F. and M.C. Neves. 2006, "The isostatic compensation of the
Azores Plateau: a 3D admittance and coherence analysis. J. Geotermal
Vulc. Res. Volume 156, Issues 1-2, Pages 10-22,
`http://dx.doi.org/10.1016/j.jvolgeores.2006.03.010 <http://dx.doi.org/10.1016/j.jvolgeores.2006.03.010>`_

See Also
--------

:doc:`gmt </gmt>`, :doc:`grdfft </grdfft>`,
:doc:`grdmath </grdmath>`, :doc:`grdproject </grdproject>`
