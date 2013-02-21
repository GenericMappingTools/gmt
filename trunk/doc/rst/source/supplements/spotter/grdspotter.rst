**********
grdspotter
**********

grdspotter - Create CVA image from a gravity or topography grid

`Synopsis <#toc1>`_
-------------------

**grdspotter** [*grdfile*\ ] **-E**\ *rotfile* **-G**\ *CVAgrid*
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [ **-A**\ *agegrid* ] [
**-B**\ *n\_try* ] [ **-D**\ *DIgrid* ] [ **-L**\ *IDgrid* ] [ **-M** ]
[ **-N**\ *upper\_age* ] [ **-P**\ *PAgrid* ] [ **-Q**\ *IDinfo* ] [
**-S** ] [ **-T**\ **t**\ \|\ **u**\ *fixed\_val* ] [
**-V**\ [*level*\ ] ] [ **-Z**\ *z\_min*\ [/*z\_max*\ [/*z\_inc*]] ] [
**-r** ]

`Description <#toc2>`_
----------------------

**grdspotter** reads a grid file with residual bathymetry or gravity and
calculates flowlines from each node that exceeds a minimum value using
the specified rotations file. These flowlines are then convolved with
the volume of the prism represented by each grid node and added up to
give a Cumulative Volcano Amplitude grid (CVA).

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*grdfile*
    Data grid to be processed, typically residual bathymetry or free-air
    anomalies.
**-E**\ *rotfile*
    Give file with rotation parameters. This file must contain one
    record for each rotation; each record must be of the following
    format:

    *lon lat tstart [tstop] angle* [ *khat a b c d e f g df* ]

    where *tstart* and *tstop* are in Myr and *lon lat angle* are in
    degrees. *tstart* and *tstop* are the ages of the old and young ends
    of a stage. If *tstop* is not present in the record then a total
    reconstruction rotation is expected and *tstop* is implicitly set to
    0 and should not be specified for any of the records in the file. If
    a covariance matrix **C** for the rotation is available it must be
    specified in a format using the nine optional terms listed in
    brackets. Here, **C** = (*g*/*khat*)\*[ *a b d; b c e; d e f* ]
    which shows **C** made up of three row vectors. If the degrees of
    freedom (*df*) in fitting the rotation is 0 or not given it is set
    to 10000. Blank lines and records whose first column contains # will
    be ignored. You may prepend a leading + to the filename to indicate
    you wish to invert the rotations.
     Alternatively, give the filename composed of two plate IDs
    separated by a hyphen (e.g., PAC-MBL) and we will instead extract
    that rotation from the GPlates rotation database. We return an error
    if the rotation cannot be found.

**-G**
    Specify name for output CVA grid file.
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
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
    *west*, *east*, *south*, and *north* specify the region of interest,
    and you may specify them in decimal degrees or in
    [+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left
    and upper right map coordinates are given instead of w/e/s/n. The
    two shorthands **-Rg** and **-Rd** stand for global domain (0/360
    and -180/+180 in longitude respectively, with -90/+90 in latitude).
    Alternatively, specify the name of an existing grid file and the
    **-R** settings (and grid spacing, if applicable) are copied from
    the grid. Using **-R**\ *unit* expects projected (Cartesian)
    coordinates compatible with chosen **-J** and we inversely project
    to determine actual rectangular geographic region.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ *agegrid*
    Supply a crustal age grid that is co-registered with the input data
    grid. These ages become the upper ages to use when constructing
    flowlines [Default extend flowlines back to oldest age found in the
    rotation file; but see **-N**].
**-B**\ *n\_try*
    Get *n\_try* bootstrap estimates of the maximum CVA location; the
    longitude and latitude results are written to stdout [Default is no
    bootstrapping]. Cannot be used with **-M**.
**-D**\ *DIgrid*
    Use flowlines to determine the maximum CVA encountered along each
    flowline and create a Data Importance (DI) grid with these values at
    the originating nodes.
**-L**\ *IDgrid*
    Supply a co-registered grid with seamount chain IDs for each node.
    This option requires that you also use **-Q**.
**-M**
    Do not attempt to keep all flowlines in memory when using **-D**
    and/or **-P**. Should you run out of memory you can use this option
    to compute flowlines on-the-fly. It will be slower as we no longer
    can reuse the flowlines calculated for the CVA step. Cannot be used
    with **-B** or the multi-slice mode in **-Z**.
**-N**\ *upper\_age*
    Set the upper age to assign to nodes whose crustal age is unknown
    (i.e., NaN) [no upper age]. Also see **-A**.
**-P**\ *PAgrid*
    Use flowlines to determine the flowline age at the CVA maximum for
    each node and create a Predicted Age (PA) grid with these values at
    the originating nodes.
**-Q**\ *IDinfo*
    Either `give (1) <give.html>`_ a single ID to use `or
    (2) <or.2.html>`_ the name of a file with a list of IDs to use
    [Default uses all IDs]. Each line would be TAG ID [w e s n]. The
    *w/e/s/n* zoom box is optional; if specified it means we only trace
    the flowline if inside this region [Default uses region set by
    **-R**]. Requires **-L**.
**-S**
    Normalize the resulting CVA grid to percentages of the CVA maximum.
    This also normalizes the DI grid (if requested).
**-T**\ **t**\ \|\ **u**\ *fixed\_val*
    Selects ways to adjust ages; repeatable. Choose from **-Tt** to
    truncate crustal ages given via the **-A** option that exceed the
    upper age set with **-N** [no truncation], or **-Tu**\ *fixed\_val*
    which means that after a node passes the test implied by **-Z**, we
    use this *fixed\_val* instead in the calculations. [Default uses
    individual node values].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-Z**\ *z\_min*\ [/*z\_max*\ [/*z\_inc*]]
    Ignore nodes with z-values lower than *z\_min* [0] and optionally
    larger than *z\_max* [Inf]. Give *z\_min/z\_max/z\_inc* to make
    separate CVA grids for each *z*-slice [Default makes one CVA grid].
    Multi-slicing cannot be used with **-M**.
**-r**
    Set pixel node registration [gridline].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc6>`_
-------------------

To create a CVA image from the Pacific topography grid
Pac\_res\_topo.nc, using the DC85.d Euler poles, and only output a grid
for the specified domain, run

grdspotter Pac\_res\_topo.nc -EDC85.d -GCVA.nc -R190/220/15/25 -I2m
-N145 -Tt -V

This file can then be plotted with **grdimage**.

`See Also <#toc7>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ , `*grdimage*\ (1) <grdimage.html>`_ ,
`*project*\ (1) <project.html>`_ ,
`*mapproject*\ (1) <mapproject.html>`_ ,
`*backtracker*\ (1) <backtracker.html>`_ ,
`*hotspotter*\ (1) <hotspotter.html>`_ ,
`*originator*\ (1) <originator.html>`_

`References <#toc8>`_
---------------------

Wessel, P., 1999, "Hotspotting" tools released, EOS Trans. AGU, 80 (29),
p. 319.

Wessel, P., 2008, Hotspotting: Principles and properties of a plate
tectonic Hough transform, Geochem. Geophys. Geosyst. 9(Q08004):
doi:10.1029/2008GC002058.
