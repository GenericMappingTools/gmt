*******
pssegyz
*******

pssegyz - Create imagemasked postscript from SEGY file

`Synopsis <#toc1>`_
-------------------

**pssegyz** *SEGYfile* **-J**\ *parameters*
**-Jz**\ \|\ **Z**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
**-D**\ *deviation* **-F**\ [*rgb*\ \|\ *gray*] **-W** [ **-B**\ *bias*
] [ **-C**\ *clip* ] [ **-I** ] [ **-K** ] [ **-L**\ *nsamp* ] [
**-M**\ *ntrace* ] [ **-N** ] [ **-O** ] [ **-P** ] [
**-S**\ *header\_x*/*header\_y* ] [ **-U**\ *redvel* ] [
**-V**\ [*level*\ ] ] [ **-X**\ *scale* ] [ **-Y**\ *sample\_int* ] [
**-Z** ] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**pssegyz** reads a native (IEEE) format SEGY file and produces a
*PostScript* image of the seismic data. The *imagemask* operator is used
so that the seismic data are plotted as a 1-bit deep bitmap in a single
(user-specified) color or gray shade, with a transparent background. The
bitmap resolution is taken from the current **GMT** defaults. The
seismic traces may be plotted at their true locations using information
in the trace headers (in which case order of the traces in the file is
not significant). Standard **GMT** geometry routines are used so that in
principle any map projection may be used, however it is likely that the
geographic projections will lead to unexpected results. Beware also that
some parameters have non-standard meanings, and a couple of the options
for **pssegy** are not available in **pssegyz**.

Note that the order of operations before the seismic data are plotted is
deviation\*[clip]([bias]+[normalize](sample value)). Deviation
determines how far *in the plot coordinates* a
[normalized][biased][clipped] sample value of 1 plots from the trace
location.

The SEGY file should be a disk image of the tape format (ie 3200 byte
text header, which is ignored, 400 byte binary reel header, and 240 byte
header for each trace) with samples as native real\*4 (IEEE real on all
the platforms to which I have access)

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*SEGYfile*
    Seismic data set to be imaged
**-J**\ *parameters* (\*)
    Select map projection.
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
**-D**\ *deviation*
    gives the deviation in X units of the plot for 1.0 on the scaled
    trace, This may be a single number (applied equally in X and Y
    directions) or devX/devY
**-F**\ [*rgb*\ \|\ *gray*]
    Fill trace (variable area, defaults to filling positive). rgb or
    gray gives the color with which the imagemask is filled.
**-W**
    Draw wiggle trace.
You *must* specify at least one of **-W** and **-F**.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**
    Flip the default byte-swap state (default assumes data have a
    bigendian byte-order).
**-C**\ *clip*
    Sample value at which to clip data (clipping is applied to both
    positive and negative values).
**-I**
    Fill negative rather than positive excursions.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**\ *nsamp*
    Override number of samples per trace in reel header (program
    attempts to determine number of samples from each trace header if
    possible to allow for variable length traces).
**-M**\ *ntrace*
    Override number of traces specified in reel header. Program detects
    end of file (relatively) gracefully, but this parameter limits
    number of traces that the program attempts to read.
**-N**
    Normalize trace by dividing by rms amplitude over full trace length.
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-S**\ *header\_x*/*header\_y*
    Read trace locations from trace headers: header is either c for CDP,
    o for offset, b<num> to read a long starting at byte <num> in the
    header (first byte corresponds to num=0), or a number to fix the
    location. First parameter for x, second for y. Default has X and Y
    given by trace
     number.
**-U**\ *redvel*
    Apply reduction velocity by shifting traces *upwards* by
    redvel/\|offset\|. Negative velocity removes existing reduction.
    Units should be consistent with offset in trace header and sample
    interval.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-X**\ *scale*
    Multiply trace locations by scale before plotting.
**-Y**\ *sample\_int*
    Override sample interval in reel header.
**-Z**
    Do not plot traces with zero rms amplitude.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
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

To plot the SEGY file wa1.segy with normalized traces plotted at true
offset locations, clipped at +-3 and with wiggle trace and positive
variable area shading in black, use

pssegyz wa1.segy -JX5i/-5i -D1 -Jz0.05i -E180/5 -R0/100/0/10/0/10 -C3 -N
-So -W -Fblack > segy.ps

`Bugs <#toc7>`_
---------------

Variable area involves filling four-sided figures of distressing
generality. I *know* that some of the more complex degenerate cases are
not dealt with correctly or at all; the incidence of such cases
increases as viewing angles become more oblique, and particularly as the
viewing elevation increases. Wiggle-trace plotting is not affected.

`See Also <#toc8>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ , `*pssegy*\ (1) <pssegy.html>`_
