.. index:: ! segyz

*******
segyz
*******

.. only:: not man

    segyz - Create imagemasked postscript from SEGY file

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt segyz** *SEGYfile* |-J|\ *parameters*
|-J|\ **z**\ \|\ **Z**\ *parameters*
|SYN_OPT-Rz|
|-D|\ *deviation* |-F|\ [*color*] **-W**
[ |-C|\ *clip* ]
[ |-I| ] [ |-L|\ *nsamp* ]
[ |-M|\ *ntrace* ]
[ |-N| ]
[ |-Q|\ *<mode><value>* ]
[ |-S|\ *header_x*/*header_y* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**segyz** reads a native (IEEE) format SEGY file and produces a
plot of the seismic data. The *imagemask* operator is used
so that the seismic data are plotted as a 1-bit deep bitmap in a single
(user-specified) color or gray shade, with a transparent background. The
bitmap resolution is taken from the current GMT defaults. The
seismic traces may be plotted at their true locations using information
in the trace headers (in which case order of the traces in the file is
not significant). Standard GMT geometry routines are used so that in
principle any map projection may be used, however it is likely that the
geographic projections will lead to unexpected results. Beware that a
couple of the options for **segy** are not available in **segyz**.

Note that the order of operations before the seismic data are plotted is
deviation\*[clip]([bias]+[normalize](sample value)). Deviation
determines how far *in the plot coordinates* a
[normalized][biased][clipped] sample value of 1 plots from the trace
location.

The SEGY file should be a disk image of the tape format (i.e., 3200 byte
text header, which is ignored, 400 byte binary reel header, and 240 byte
header for each trace) with samples as native real\*4 (IEEE real on all
the platforms to which I have access).

Required Arguments
------------------

*SEGYfile*
    Seismic SEGY data set to be imaged.

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-J.rst_

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

.. _-D:

**-D**\ *deviation*
    gives the deviation in X units of the plot for 1.0 on the scaled
    trace, This may be a single number (applied equally in X and Y
    directions) or the pair *devX*\ /*devY*.

.. _-F:

**-F**\ [*color*]
    Fill trace (variable area, defaults to filling positive). Specify the
    *color* with which the imagemask is filled.

.. _-W:

**-W**
    Draw wiggle trace.

You *must* specify at least one of **-W** and **-F**.

Optional Arguments
------------------

.. _-A:

**-A**
    Flip the default byte-swap state (default assumes data have a
    bigendian byte-order).

.. _-C:

**-C**\ *clip*
    Sample value at which to clip data (clipping is applied to both
    positive and negative values).

.. _-I:

**-I**
    Fill negative rather than positive excursions.

.. _-L:

**-L**\ *nsamp*
    Override number of samples per trace in reel header (program
    attempts to determine number of samples from each trace header if
    possible to allow for variable length traces).

.. _-M:

**-M**\ *ntrace*
    Override number of traces specified in reel header. Program detects
    end of file (relatively) gracefully, but this parameter limits
    number of traces that the program attempts to read.

.. _-N:

**-N**
    Normalize trace by dividing by rms amplitude over full trace length.

.. _-Q:

**-Q**\ *<mode><value>*
    Can be used to change 5 different settings depending on *mode*:
       **-Qb**\ *bias* to bias scaled traces (-Qb-0.1 subtracts 0.1 from values).

       **-Qi**\ *dpi* sets the dots-per-inch resolution of the image [300].

       **-Qu**\ *redvel* to apply reduction velocity (-ve removes reduction already present).

       **-Qx**\ *mult* to multiply trace locations by *mult*.

       **-Qy**\ *dy* to override sample interval in reel header.

.. _-S:

**-S**\ *header_x*/*header_y*
    Read trace locations from trace headers: headers is either **c** for CDP,
    **o** for offset, **b** \*num* to read a long starting at byte *num* in the
    header (first byte corresponds to num=0), or a number to fix the
    location. First parameter for x, second for y. Default has X and Y
    given by trace number.

.. _-U:

.. include:: ../../explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-X:

.. include:: ../../explain_-XY.rst_

.. _-Z:

**-Z**
    Do not plot traces with zero rms amplitude.

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-t.rst_
.. include:: ../../explain_help.rst_

Examples
--------

To plot the SEGY file wa1.segy with normalized traces plotted at true
offset locations, clipped at ±3 and with wiggle trace and positive
variable area shading in black, use

   ::

    gmt segyz wa1.segy -JX5i/-5i -D1 -Jz0.05i -E180/5 -R0/100/0/10/0/10 \
            -C3 -N -So -W -Fblack -pdf segy

Bugs
----

Variable area involves filling four-sided figures of distressing
generality. I *know* that some of the more complex degenerate cases are
not dealt with correctly or at all; the incidence of such cases
increases as viewing angles become more oblique, and particularly as the
viewing elevation increases. Wiggle-trace plotting is not affected.

See Also
--------

:doc:`gmt </gmt>`,
:doc:`segy`,
:doc:`segy2grd`
