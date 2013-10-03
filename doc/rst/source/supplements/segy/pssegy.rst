.. index:: ! pssegy

******
pssegy
******

.. only:: not man

    pssegy - Plot a SEGY file on a map

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**pssegy** *SEGYfile* **-J**\ *parameters*
|SYN_OPT-R|
**-D**\ *deviation*
**-F**\ [*rgb*\ \|\ *gray*] **-W**
[ **-B**\ *bias* ]
[ **-C**\ *clip* ]
[ **-E**\ *error* ] [ **-I** ] [ **-K** ] [ **-L**\ *nsamp* ]
[ **-M**\ *ntrace* ] [ **-N** ] [ **-O** ] [ **-P** ] [ **-S**\ *header* ]
[ **-T**\ *filename* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ **-X**\ *scale* ] [ **-Y**\ *sample_int* ] 
[ **-Z** ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]

|No-spaces|

Description
-----------

**pssegy** reads a native (IEEE) format SEGY file and produces a
PostScript image of the seismic data. The *imagemask* operator is used
so that the seismic data are plotted as a 1-bit deep bitmap in a single
(user-specified) color or gray shade, with a transparent background. The
bitmap resolution is taken from the current GMT defaults. The
seismic traces may be plotted at their true locations using information
in the trace headers (in which case order of the traces in the file is
not significant). Standard GMT geometry routines are used so that in
principle any map projection may be used, however it is likely that the
geographic projections will lead to unexpected results. Beware also that
some parameters have non-standard meanings.

Note that the order of operations before the seismic data are plotted is
deviation\*[clip]([bias]+[normalize](sample value)). Deviation
determines how far *in the plot coordinates* a
[normalized][biased][clipped] sample value of 1 plots from the trace
location.

The SEGY file should be a disk image of the tape format (ie 3200 byte
text header, which is ignored, 400 byte binary reel header, and 240 byte
header for each trace) with samples as native real\*4 (IEEE real on all
the platforms to which I have access)

Required Arguments
------------------

*SEGYfile*
    Seismic data set to be imaged

.. include:: ../../explain_-J.rst_

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

**-D**\ *deviation*
    gives the deviation in X units of the plot for 1.0 on the scaled
    trace.
**-F**\ [*rgb*\ \|\ *gray*]
    Fill trace (variable area, defaults to filling positive). rgb or
    gray gives the color with which the imagemask is filled.
**-W**
    Draw wiggle trace.

You *must* specify at least one of **-W** and **-F**.

Optional Arguments
------------------

**-A**
    Flip the default byte-swap state (default assumes data have a
    bigendian byte-order).
**-C**\ *clip*
    Sample value at which to clip data (clipping is applied to both
    positive and negative values).
**-E**\ *error*
    Allow *error* difference between requested and actual trace
    locations when using **-T** option.
**-I**
    Fill negative rather than positive excursions.

.. include:: ../../explain_-K.rst_

**-L**
    Override number of samples per trace in reel header (program
    attempts to determine number of samples from each trace header if
    possible to allow for variable length traces).
**-M**
    Override number of traces specified in reel header. Program detects
    end of file (relatively) gracefully, but this parameter limits
    number of traces that the program attempts to read.
**-N**
    Normalize trace by dividing by rms amplitude over full trace length.

.. include:: ../../explain_-O.rst_
.. include:: ../../explain_-P.rst_

**-S**\ *header*
    Read trace locations from trace headers: header is either c for CDP,
    o for offset, or b<num> to read a long starting at byte <num> in the
    header (first byte corresponds to num=0). Default has location given
    by trace number.
**-T**\ *filename*
    Plot only traces whose location corresponds to a list given in
    *filename*. Order in which traces are listed is not significant -
    the entire space is checked for each trace.
**-U**\ *redvel*
    Apply reduction velocity by shifting traces *upwards* by
    redvel/\|offset\|. Negative velocity removes existing reduction.
    Units should be consistent with offset in trace header and sample
    interval.

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

**-X**\ *scale*
    Multiply trace locations by scale before plotting.
**-Y**\ *sample_int*
    Override sample interval in reel header.
**-Z**
    Do not plot traces with zero rms amplitude.

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-t.rst_
.. include:: ../../explain_help.rst_

Examples
--------

To plot the SEGY file wa1.segy with normalized traces plotted at true
offset locations, clipped at +-3 and with wiggle trace and positive
variable area shading in black, use

   ::

    gmt pssegy wa1.segy -JX5i/-5i -R0/100/0/10 -D1 -C3 -N -So -W -Fblack > segy.ps

To plot the SEGY file wa1.segy with traces plotted at true cdp\*0.1,
clipped at +-3, with bias -1 and negative variable area shaded red, use

   ::

    gmt pssegy wa1.segy -JX5i/-5i -R0/100/0/10 -D1 -C3 -Sc -X0.1 -Fred -B-1 -I > segy.ps

See Also
--------

:doc:`gmt </gmt>`, :doc:`pssegyz`
