.. index:: ! nswing
.. include:: ../module_supplements_purpose.rst_

******
nswing
******

.. only:: not man

    |nswing_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**nswing**
*bathy.grd* *initial.grd*
**-t**\ *dt*
[ **-1**\ *bat_lev1* ] [ **-2**\ *bat_lev2* ] [ **-3**\ *...* ]
[ |-A|\ *fname.sww* ]
[ |-C| ]
[ |-D| ]
[ |-E|\ [**p**][**m**][**+a**][,\ *decim*] ]
[ |-F|\ *x_epic/y_epic/dip/strike/rake/slip/length/width/topDepth* ]
[ |-F|\ **k**\ [**c**]\ *w/e/s/n* ]
[ |-G|\ *name*\ [**+m**],\ *int* ]
[ |-H|\ [*momentM,momentN*\ [,\ *t*]] ]
[ |-P|\ *time_jump*\ [**+t**\ *run_time_jump*] ]
[ |-L|\ [*name1,name2*] ]
[ |-M|\ [**-**\|\ **+**\ [*maskname*]] ]
[ |-N|\ *n_cycles* ]
[ |-O|\ *BCfile* ]
[ |-Q|\ *z_offset* ]
[ |SYN_OPT-R| ]
[ |-S|\ [**x**\|\ **y**\|\ **n**][**+m**][**+s**][**+a**] ]
[ |-T|\ *mareg*\|\ *x/y*\ [**+o**\ *outmaregs*][**+t**\ *int*] ]
[ |-X|\ *manning0*\ [,\ *...*] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ **-x**\ *n* ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**nswing** is a Non-linear Shallow Water model that propagates a tsunami over a
bathymetry grid. Starting from a base level bathymetry and an initial-condition
(source) grid it integrates the shallow water equations in time and writes the
resulting wave field (water level, velocity, momentum, energy, ...) either as a
series of GMT grids, a 3D netCDF file, or as time series sampled at virtual
maregraph (tide-gauge) locations. Nested grids of increasing resolution may be
used to refine the solution near the coast, and a tsunami source can be generated
on the fly from Okada fault parameters.

Required Arguments
------------------

*bathy.grd*
    The base level bathymetry grid (positive up; negative = below sea level).
    (See :ref:`Grid File Formats <grd_inout_full>`).
*initial.grd*
    The initial-condition (source) grid holding the sea surface displacement at
    time zero. (See :ref:`Grid File Formats <grd_inout_full>`).

.. _-t:

**-t**\ *dt*
    Time step (in seconds) for the simulation.

Optional Arguments
------------------

**-1**\ *bat_lev1* **-2**\ *bat_lev2* ... **-9**\ *bat_lev9*
    Nested bathymetry grids, one per nesting level. Each level refines the solution inside
    the area covered by its grid. Warning, the grids must be aligned and have a cell size
    that is an integer fraction of the previous level. The number of levels is determined
    by the number of **-1** ... **-9** options given. These grids are not trivial to create,
    the best way is to use the Mirone TINTOL tool.

.. _-A:

**-A**\ *fname.sww*
    Save the result as a *.sww* ANUGA-format file.

**-n**\ *base*
    Basename for MOST triplet files (no extension).

.. _-C:

**-C**
    Add the Coriolis effect.

.. _-D:

**-D**
    Write grids with the total water depth. These grids will have wave height
    on the ocean and water thickness on land.

.. _-E:

**-E**\ [**p**][**m**][**+a**][,\ *decim*]
    Write grids with energy, or with power if **p** is appended (**-Ep**).
    Append **m** to save only a single grid holding the max values. This can
    noticeably slow the run, so optionally append a *decim* decimator factor
    after the comma (causes aliasing visible under shaded illumination). The
    file name comes from *name* in **-G** complemented with a *_max*
    prefix; saving of multiple grids is then disabled.

    With **-G**'s 3D netCDF cube, Energy/Power replaces the sea-surface
    (*z*) variable unless **+a** is appended, in which case Energy/Power is
    written as an extra variable alongside *z* (same idea as **-S**'s
    **+a**).

.. _-F:

**-F**\ *x_epic/y_epic/dip/strike/rake/slip/length/width/topDepth*
    Okada fault parameters used to build the source. *x_epic*, *y_epic* are the
    x and y coordinates of the beginning of the fault trace; *dip*, *strike*
    (azimuth), *rake*, *slip* (m), *length*, *width* and *topDepth* (depth from
    the sea-bottom) follow. All dimensions must be in km.

    If no bathymetry grid is given, **-F** (or **-Fk**) together with **-R**
    and **-G** computes the deformation over **-R**'s grid geometry (e.g.
    **-R**\ *grid*) and writes it straight to **-G**'s name: no simulation is
    run, so **-t** and **-G**'s saving interval are not needed.

**-F**\ **k**\ *west/east/south/north*
    Build a prism source with these limits and a height of 1 meter.

    - **-Fkc**\ *x/y/nx/ny* - alternatively give the prism size as centre *x/y*
      and *nx/ny* half-width cell numbers.
    - **-Fk**\ *...*\ **/**\ *RxC* - loop over a matrix of size *R* by *C*
      starting at the Lower Left Corner given by *w/e/s/n*.
    - **-Fk**\ *...*\ **/**\ *dx*\ [/*dy*] - given the *w/e/s/n* region (pixel
      registration) loop over the prisms obtained by dividing the region in
      increments of *dx/dy* (if *dy* is not given, *dy* = *dx*).

    Using **-Fk** sets the output maregraph file to netCDF, unless rows = cols = 1.

.. _-G:

**-G**\ *name*\ [**+m**],\ *int*
    Save the water level every *int* time steps in a single 3D netCDF file
    called *name*\ **.nc** (the extension is appended when *name* has none).
    Append **+m** to instead write each saved step as a separate grid; files
    are then named *name#*\ **.grd**. When doing nested grids the finest level
    is the one saved.

.. _-H:

**-H**
    Write grids with the momentum (velocity times water depth).

    - **-H**\ *fname_momentM,fname_momentN*\ [,\ *t*] - hot start using these
      moment grids. The optional *t* is the hot-start time (also needs the
      surface displacement corresponding to the time of these grids).

.. _-P:

**-P**\ *time_jump*\ [**+t**\ *run_time_jump*]
    Do not write grids or maregraphs for times before *time_jump* (seconds).
    When doing nested grids, append **+**\ *time* to NOT start nested-grid
    computations before this time has elapsed. Allowed forms: **-P**\ *t1*,
    **-P+**\ *t2*, **-P**\ *t1*\ **+**\ *t2* or **-P**\ *t1* **-P+**\ *t2*.

.. _-L:

**-L**
    Use the linear approximation in the moment conservation equations (faster
    but less accurate).

    - **-L**\ *in_fname,out_fname* - do Lagrangian tracers, where *in_fname* is
      the tracers initial-position file and *out_fname* the file to hold the
      results.

.. _-M:

**-M**\ [**-**\|\ **+**\ [*maskname*]]
    Write a grid with the max water level (name from *name* in **-G**, *_max*
    prefix). Append **-** to instead compute the maximum water retreat, written
    to a mask file (default *long_beach.grd*; append a name after **-** to
    change it, e.g. **-M-**\ *beach_long.grd*). Append **+** for a mask with the
    Run In extent (behaves like **-M-**). **-M** may be repeated, e.g.
    **-M -M- -M+** computes all three. With **-G** the *long* and *short* beach
    arrays are also saved in the *.nc* file.

.. _-N:

**-N**\ *n_cycles*
    Number of cycles in the simulation [Default 1010]. Total simulation time is *n_cycles* times the time step *dt*.

.. _-O:

**-O**\ *BCfile*
    Name of a Boundary Condition ASCII file (experimental).

.. _-Q:

**-Q**\ *z_offset*
    Apply a vertical offset to ALL bathymetry grids (e.g. to simulate tide).

.. _-R:

.. |Add_-Rgeo| replace:: |Add_-R_auto_table|
.. include:: ../../explain_-Rgeo.rst_

Output grids only in the sub-region enclosed by *west/east/south/north*.

.. _-S:

**-S**\ [**x**\|\ **y**\|\ **n**][**+m**][**+s**][**+a**]
    Write grids with the velocity (names get *_U* and *_V* suffixes). Use **x**
    or **y** to save only one component, or **n** for no velocity grids
    (maregraphs only). Append **+m** to also write velocity (vx,vy) at maregraph
    locations (needs **-T**). Append **+s** to write the max speed
    (|v|) (*_max_speed* suffix). Use the **n** flag to NOT output the U and V
    components, e.g. **-Sn+s**.

    With **-G**'s 3D netCDF cube, velocity is written as the only variable
    unless **+a** is appended, in which case the sea-surface (*z*) variable
    is also written (*z* + velocity).

.. _-T:

**-T**\ *mareg*\|\ *x/y*\ [**+o**\ *outmaregs*][**+t**\ *int*]
    Save time series (maregraphs) at virtual tide-gauge locations. *mareg* is
    the file with the (x y) locations of the virtual maregraphs. For a single
    maregraph the location may be given directly as *x/y* instead of a file
    name. Append **+o**\ *outmaregs* to set the output file name [Default is
    *maregs_out.dat*]. A *.dat* extension is added when *outmaregs* has none;
    use a *.nc* extension to write the maregraphs as a netCDF file instead.
    Append **+t**\ *int* to save every *int* simulation time steps (set by
    **-t**) [Default is every time step]. **-T** alone (without **-G**) is
    allowed and runs a simulation that only outputs the maregraph series.

.. _-X:

**-X**\ *manning0*\ [,\ *manning1*\ [,\ *...*]][**+**\ *depth*]
    Manning friction coefficients. If only one is provided, use it for all
    nesting levels; otherwise specify one per level, comma separated. Append
    **+**\ *depth* to apply Manning only at depths shallower than *depth*
    (positive up).

.. |Add_-V| replace:: |Add_-V_links|
.. include:: ../../explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-f.rst_

    When **-fg** is not given, **nswing** first checks the grid's own metadata
    (netCDF degree units) and, failing that, falls back to a coordinate-range
    heuristic; if either indicates geographic coordinates a warning is issued
    and **-fg** is set implicitly.

**-x**\ *n*
    Number of OpenMP threads to use [Default is all available cores]. Results
    are identical for any number of threads.

.. include:: ../../explain_core.rst_

.. include:: ../../explain_help.rst_

Examples
--------

To propagate a tsunami over the bathymetry *bathy.grd* given the source
*source.grd*, using a 5 second time step and saving the water level every 10
time steps to grids named *wave#.grd*, try::

    gmt nswing bathy.grd source.grd -t5 -Gwave,10

To run the same simulation but generate the source on the fly from Okada fault
parameters and sample the wave field at the virtual maregraphs listed in
*gauges.dat* every 5 time steps, try::

    gmt nswing bathy.grd -t5 -F-8/37/12/90/90/3/100/50/10 -Tgauges.dat+t5

To record only the time series at a single virtual tide gauge (no grids at
all), give its location directly to **-T**::

    gmt nswing bathy.grd source.grd -t5 -T-10.7/37.3

To compute just the Okada co-seismic deformation over the geometry of the grid
*bathy.grd* (no simulation), try::

    gmt nswing -Rbathy.grd -F94.3/2.8/25/330/90/10/250/65/10 -Gdeform.grd

A less hypothetical example that generates only a few layers in the tsu.nc cube::

    gmt grdcut @earth_relief_02m_g -R-15/-7.5/34/39.5 -Gbat.grd
    gmt nswing -Rbat.grd -F-12.49593345/35.93634937/25/58.2/90/10/215/53.75/10 -Ginit.grd
    gmt nswing bat.grd init.grd -t3 -Gtsu,10 -N100 -V

See Also
--------

:doc:`gmt </gmt>`,
:doc:`grdinterpolate </grdinterpolate>`

Reference
---------

[Validation of NSWING, a multi-core finite difference code for tsunami propagation and run-up]( https://www.researchgate.net/publication/275349940_Validation_of_NSWING_a_multi-core_finite_difference_code_for_tsunami_propagation_and_run-up)
