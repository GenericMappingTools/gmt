********
segy2grd
********

segy2grd - Converting SEGY data to a GMT grid

`Synopsis <#toc1>`_
-------------------

**segy2grd** *segyfile* **-G**\ *grdfile*
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [
**-A**\ [**n**\ \|\ **z**] ] [
**-D**\ *xname*/*yname*/*zname*/*scale*/*offset*/*title*/*remark* ] [
**-M**\ [*flags*\ ] ] [ **-N**\ *nodata* ] [ **-S**\ [*zfile*\ ] ] [
**-V**\ [*level*\ ] ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [ **-r** ] [
**-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**segy2grd** reads an IEEE SEGY file and creates a binary grid file.
Either a simple mapping (equivalent to xyz2grd -Z) or a more complicated
averaging where a particular grid cell includes values from more than
one sample in the SEGY file can be done. **segy2grd** will report if
some of the nodes are not filled in with data. Such unconstrained nodes
are set to a value specified by the user [Default is NaN]. Nodes with
more than one value will be set to the average value.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*segyfile* is an IEEE floating point SEGY file. Traces are all assumed
to start at 0 time/depth.
**-G**\ *grdfile*
    *grdfile* is the name of the binary output grid file.
**-I**
    *x\_inc* [and optionally *y\_inc*] is the grid spacing. Append **m**
    to indicate minutes or **c** to indicate seconds.
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

**-A**\ [**n**\ \|\ **z**]
    Add up multiple values that belong to the same node (same as
    **-Az**). Append **n** to simply count the number of data points
    that were assigned to each node. [Default (no **-A** option) will
    calculate mean value]. Not used for simple mapping.
**-D**\ *xname*/*yname*/*zname*/*scale*/*offset*/*title*/*remark*
    Give values for *xname*, *yname*, *zname*, *scale*, *offset*,
    *title*, and *remark*. To leave some of these values untouched,
    specify = as the value.
**-M**\ [*flags*\ ]
    Fix number of traces to read in. Default tries to read 10000 traces.
    **-M**\ 0 will read number in binary header, **-M**\ *n* will
    attempt to read only *n* traces.
**-N**\ *nodata*
    No data. Set nodes with no input sample to this value [Default is
    NaN].
**-S**\ [*zfile*\ ]
    set variable spacing *header* is c for cdp, o for offset, b<number>
    for 4-byte float starting at byte number If -S not set, assumes even
    spacing of samples at the dx, dy supplied with -I
**-L**
    Override number of samples in each trace
**-X**
    applies scalar *x-scale* to coordinates in trace header to match the
    coordinates specified in -R
**-Y**
    Specifies sample interval as *s\_int* if incorrect in the SEGY file
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
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

To create a grid file from an even spaced SEGY file test.segy,

try

    segy2grd test.segy -I0.1/0.1 -Gtest.nc -R198/208/18/25 -V

    Note that this will read in 18-25s (or km) on each trace, but the
    first trace will be assumed to be at X=198

    To create a grid file from the SEGY file test.segy, locating traces
    according to the CDP number, where there are 10 CDPs per km and the
    sample interval is 0.1, try

    segy2grd test.segy -Gtest.nc -R0/100/0/10 -I0.5/0.2 -V -X0.1 -Y0.1

    Because the grid interval is larger than the SEGY file sampling, the
    individual samples will be averaged in bins

`See Also <#toc7>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ , `*grd2xyz*\ (1) <grd2xyz.html>`_ ,
`*grdedit*\ (1) <grdedit.html>`_ , `*pssegy*\ (1) <pssegy.html>`_
