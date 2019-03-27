.. index:: ! gravfft

*******
gravfft
*******

.. only:: not man

    gravfft - Spectral calculations of gravity, isostasy, admittance, and coherence for grids

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt gravfft** *ingrid* [ *ingrid2* ] |-G|\ *outfile*
[ |-C|\ *n/wavelength/mean\_depth/tbw* ]
[ |-D|\ *density*\ \|\ *rhogrid* ]
[ |-E|\ *n_terms* ]
[ |-F|\ [**f**\ [**+s**]\ \|\ **b**\ \|\ **g**\ \|\ **v**\ \|\ **n**\ \|\ **e**] ]
[ |-I|\ **w**\ \|\ **b**\ \|\ **c**\ \|\ **t** \|\ **k** ]
[ |-N|\ *params* ]
[ |-Q| ]
[ |-T|\ *te/rl/rm/rw*\ [*/ri*]\ [**+m**] ]
[ |SYN_OPT-V| ]
[ |-W|\ *wd*]
[ |-Z|\ *zm*\ [*zl*] ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**gravfft** can be used into three main modes. Mode 1: Simply
compute the geopotential due to the surface given in the topo.grd file.
Requires a density contrast (**-D**) and possibly a different observation
level (**-W**).  It will take the 2-D
forward FFT of the grid and 
use the full Parker's method up to the chosen terms.  Mode 2: Compute the
geopotential response due to flexure of the topography file. It will take the 2-D
forward FFT of the grid and use the  full Parker's method applied to the chosen
isostatic model.  The available
models are the "loading from top", or elastic plate model, and the
"loading from below" which accounts for the plate's response to a
sub-surface load (appropriate for hot spot modeling - if you believe
them). In both cases, the model parameters are set with **-T** and
**-Z** options. Mode 3: compute the admittance or coherence between
two grids. The output is the average in the radial direction.
Optionally, the model admittance may also be calculated. The horizontal
dimensions of the grdfiles are assumed to be in meters. Geographical
grids may be used by specifying the |SYN_OPT-f| option that scales degrees
to meters. If you have grids with dimensions in km, you could change
this to meters using :doc:`grdedit </grdedit>` or scale the output with
:doc:`grdmath </grdmath>`.
Given the number of choices this program offers, is difficult to state
what are options and what are required arguments. It depends on what you
are doing; see the examples for further guidance.

Required Arguments
------------------

*ingrid*
    2-D binary grid file to be operated on. (See GRID FILE FORMATS below).
    For cross-spectral operations, also give the second grid file *ingrd2*.

.. _-G:

**-G**\ *outfile*
    Specify the name of the output grid file or the 1-D spectrum table
    (see **-E**). (See GRID FILE FORMATS below).

Optional Arguments
------------------

.. _-C:

**-C**\ *n/wavelength/mean\_depth/tbw*
    Compute only the theoretical admittance curves of the selected model
    and exit. *n* and *wavelength* are used to compute (n \* wavelength)
    the total profile length in meters. *mean\_depth* is the mean water
    depth. Append dataflags (one or two) of *tbw* in any order. *t* =
    use "from top" model, *b* = use "from below" model. Optionally
    specify *w* to write wavelength instead of frequency.

.. _-D:

**-D**\ *density*\ \|\ *rhogrid*
    Sets density contrast across surface. Used, for example, to compute
    the gravity attraction of the water layer that can later be combined
    with the free-air anomaly to get the Bouguer anomaly. In this case
    do not use **-T**. It also implicitly sets **-N+h**.  Alternatively,
    specify a co-registered grid with density contrasts if a variable
    density contrast is required.

.. _-E:

**-E**\ *n_terms*
    Number of terms used in Parker expansion (limit is 10, otherwise
    terms depending on n will blow out the program) [Default = 3]

.. _-F:

**-F**\ [**f**\ [**+s**]\ \|\ **b**\ \|\ **g**\ \|\ **v**\ \|\ **n**\ \|\ **e**]
    Specify desired geopotential field: compute geoid rather than gravity

       **f** = Free-air anomalies (mGal) [Default].  Append **+s** to add
       in the slab implied when removing the mean value from the topography.
       This requires zero topography to mean no mass anomaly.

       **b** = Bouguer gravity anomalies (mGal).

       **g** = Geoid anomalies (m).

       **v** = Vertical Gravity Gradient (VGG; 1 Eotvos = 0.1 mGal/km).

       **e** = East deflections of the vertical (micro-radian).

       **n** = North deflections of the vertical (micro-radian).

.. _-I:

**-I**\ **w**\ \|\ **b**\ \|\ **c**\ \|\ **t** \|\ **k**
    Use *ingrd2* and *ingrd1* (a grid with topography/bathymetry) to estimate admittance\|coherence and
    write it to stdout (**-G** ignored if set). This grid should contain
    gravity or geoid for the same region of *ingrd1*. Default
    computes admittance. Output contains 3 or 4 columns. Frequency
    (wavelength), admittance (coherence) one sigma error bar and,
    optionally, a theoretical admittance. Append dataflags (one to
    three) from **w**\ \|\ **b**\ \|\ **c**\ \|\ **t**.
    **w** writes wavelength instead of wavenumber, **k**
    selects km for wavelength unit [m], **c** computes coherence instead of
    admittance, **b** writes a fourth column with "loading from below"
    theoretical admittance, and **t** writes a fourth column with "elastic
    plate" theoretical admittance.

.. include:: ../../explain_fft.rst_

.. _-Q:

**-Q**
    Writes out a grid with the flexural topography (with z positive up)
    whose average depth was set by **-Z**\ *zm* and model parameters by **-T**
    (and output by **-G**). That is the "gravimetric Moho". **-Q**
    implicitly sets **-N+h**

.. _-S:

**-S**
    Computes predicted gravity or geoid grid due to a subplate load
    produced by the current bathymetry and the theoretical model. The
    necessary parameters are set within **-T** and **-Z** options. The
    number of powers in Parker expansion is restricted to 1.
    See an example further down.

.. _-T:

**-T**\ *te/rl/rm/rw*\ [*/ri*]\ [**+m**]
    Compute the isostatic compensation from the topography load (input grid file) on
    an elastic plate of thickness *te*. Also append densities for load, mantle,
    water and infill in SI units. If *ri* is not provided it defaults to *rl*.
    Give average mantle depth via **-Z**. If the elastic thickness
    is > 1e10 it will be interpreted as the flexural rigidity (by default it is
    computed from *te* and Young modulus). Optionally, append *+m* to write a grid
    with the Moho's geopotential effect (see **-F**) from model selected by **-T**. 
    If *te* = 0 then the Airy response is returned. **-T+m** implicitly sets **-N+h**

.. _-W:

**-W**\ *wd*
    Set water depth (or observation height) relative to topography [0].  Append **k** to indicate km.

.. _-Z:

**-Z**\ *zm*\ [*zl*]
    Moho [and swell] average compensation depths (in meters positive dows -- the depth). For the "load from
    top" model you only have to provide *zm*, but for the "loading from below" don't forget *zl*.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

|SYN_OPT-f|
   Geographic grids (dimensions of longitude, latitude) will be converted to
   meters via a "Flat Earth" approximation using the current ellipsoid parameters.

.. include:: ../../explain_help.rst_

.. include:: ../../explain_grd_inout_short.rst_

Grid Distance Units
-------------------

If the grid does not have meter as the horizontal unit, append **+u**\ *unit*
to the input file name to convert from the specified unit to meter.  If your
grid is geographic, convert distances to meters by supplying |SYN_OPT-f| instead.

Considerations
--------------

netCDF COARDS grids will automatically be recognized as geographic. For
other grids geographical grids were you want to convert degrees into
meters, select |SYN_OPT-f|. If the data are close to either pole, you should
consider projecting the grid file onto a rectangular coordinate system
using :doc:`grdproject </grdproject>`.

Plate Flexure
-------------

The FFT solution to elastic plate flexure requires the infill density to equal
the load density.  This is typically only true directly beneath the load; beyond the load
the infill tends to be lower-density sediments or even water (or air).  Wessel [2001]
proposed an approximation that allows for the specification of an infill density
different from the load density while still allowing for an FFT solution. Basically,
the plate flexure is solved for using the infill density as the effective load density but
the amplitudes are adjusted by a factor *A* = sqrt ((rm - ri)/(rm - rl)), which is
the theoretical difference in amplitude due to a point load using the two different
load densities.  The approximation is very good but breaks down for large
loads on weak plates, a fairy uncommon situation.

Examples
--------

To compute the effect of the water layer above the bat.grd bathymetry
using 2700 and 1035 for the densities of crust and water and writing the
result on water_g.grd (computing up to the fourth power of bathymetry
in Parker expansion):

   ::

    gmt gravfft bat.grd -D1665 -Gwater_g.grd -E4

Now subtract it from your free-air anomaly faa.grd and you will get the
Bouguer anomaly. You may wonder why we are subtracting and not adding.
After all the Bouguer anomaly pretends to correct the mass deficiency
presented by the water layer, so we should add because water is less
dense than the rocks below. The answer relies on the way gravity
effects are computed by the Parker's method and practical aspects of
using the FFT.

   ::

    gmt grdmath faa.grd water_g.grd SUB = bouguer.grd

Want an MBA anomaly? Well compute the crust mantle contribution and add
it to the sea-bottom anomaly. Assuming a 6 km thick crust of density
2700 and a mantle with 3300 density we could repeat the command used to
compute the water layer anomaly, using 600 (3300 - 2700) as the density
contrast. But we now have a problem because we need to know the mean
Moho depth. That is when the scale/offset that can be appended to the grid's name
comes in hand. Notice that we didn't need to do that before because mean water
depth was computed directly from data (notice also the negative sign of the
offset due to the fact that *z* is positive up):

   ::

    gmt gravfft bat.grd=nf/1/-6000 -D600 -Gmoho_g.grd

Now, subtract it from the Bouguer to obtain the MBA anomaly. That is:

   ::

    gmt grdmath bouguer.grd moho_g.grd SUB = mba.grd

To compute the Moho gravity effect of an elastic plate bat.grd with Te =
7 km, density of 2700, over a mantle of density 3300, at an average depth
of 9 km

   ::

    gmt gravfft bat.grd -Gelastic.grd -T7000/2700/3300/1035+m -Z9000

If you add now the sea-bottom and Moho's effects, you will get the full
gravity response of your isostatic model. We will use here only the
first term in Parker expansion.

   ::

    gmt gravfft bat.grd -D1665 -Gwater_g.grd -E1
    gmt gravfft bat.grd -Gelastic.grd -T7000/2700/3300/1035+m -Z9000 -E1
    gmt grdmath water_g.grd elastic.grd ADD = model.grd

The same result can be obtained directly by the next command. However,
PAY ATTENTION to the following. I don't yet know if it's because of a
bug or due to some limitation, but the fact is that the following and
the previous commands only give the same result if **-E**\ 1 is used.
For higher powers of bathymetry in Parker expansion,
only the above example seams to give the correct result.

   ::

    gmt gravfft bat.grd -Gmodel.grd -T7000/2700/3300/1035 -Z9000 -E1

And what would be the geoid anomaly produced by a load at 50 km depth,
below a region whose bathymetry is given by bat.grd, a Moho at 9 km
depth and the same densities as before?

   ::

    gmt gravfft topo.grd -Gswell_geoid.grd -T7000/2700/3300/1035 -Fg -Z9000/50000 -S -E1

To compute the admittance between the topo.grd bathymetry and faa.grd
free-air anomaly grid using the elastic plate model of a crust of 6 km
mean thickness with 10 km effective elastic thickness in a region of 3 km
mean water depth:

   ::

    gmt gravfft topo.grd faa.grd -It -T10000/2700/3300/1035 -Z9000

To compute the admittance between the topo.grd bathymetry and geoid.grd
geoid grid with the "loading from below" (LFB) model with the same as
above and sub-surface load at 40 km, but assuming now the grids are in
geographic and we want wavelengths instead of frequency:

   ::

    gmt gravfft topo.grd geoid.grd -Ibw -T10000/2700/3300/1035 -Z9000/40000 -fg

To compute the gravity theoretical admittance of a LFB along a 2000 km
long profile using the same parameters as above

   ::

    gmt gravfft -C400/5000/3000/b -T10000/2700/3300/1035 -Z9000/40000

References
----------

Luis, J.F. and M.C. Neves. 2006, The isostatic compensation of the
Azores Plateau: a 3D admittance and coherence analysis. J. Geothermal
Volc. Res. Volume 156, Issues 1-2, Pages 10-22,
`http://dx.doi.org/10.1016/j.jvolgeores.2006.03.010 <http://dx.doi.org/10.1016/j.jvolgeores.2006.03.010>`_

Parker, R. L., 1972, The rapid calculation of potential anomalies, Geophys. J., 31, 447-455.

Wessel. P., 2001, Global distribution of seamounts inferred from gridded Geosat/ERS-1 altimetry,
J. Geophys. Res., 106(B9), 19,431-19,441,
`http://dx.doi.org/10.1029/2000JB000083 <http://dx.doi.org/110.1029/2000JB000083>`_

See Also
--------

:doc:`gmt </gmt>`, :doc:`grdfft </grdfft>`,
:doc:`grdmath </grdmath>`, :doc:`grdproject </grdproject>`
