.. index:: ! segy2grd

********
segy2grd
********

.. only:: not man

    segy2grd - Converting SEGY data to a GMT grid

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**segy2grd** *segyfile* **-G**\ *grdfile*
|SYN_OPT-I|
|SYN_OPT-R|
[ **-A**\ [**n**\ \|\ **z**] ]
[ **-D**\ *xname*/*yname*/*zname*/*scale*/*offset*/*title*/*remark* ]
[ **-M**\ [*flags*\ ] ] [ **-N**\ *nodata* ] [ **-S**\ [*zfile*] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**segy2grd** reads an IEEE SEGY file and creates a binary grid file.
Either a simple mapping (equivalent to xyz2grd -Z) or a more complicated
averaging where a particular grid cell includes values from more than
one sample in the SEGY file can be done. **segy2grd** will report if
some of the nodes are not filled in with data. Such unconstrained nodes
are set to a value specified by the user [Default is NaN]. Nodes with
more than one value will be set to the average value.

Required Arguments
------------------

*segyfile* is an IEEE floating point SEGY file. Traces are all assumed to start at 0 time/depth.

**-G**\ *grdfile*
    *grdfile* is the name of the binary output grid file.
**-I**
    *x_inc* [and optionally *y_inc*] is the grid spacing. Append **m**
    to indicate minutes or **c** to indicate seconds.

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

Optional Arguments
------------------

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
    Specifies sample interval as *s_int* if incorrect in the SEGY file

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_nodereg.rst_

.. include:: ../../explain_help.rst_

Examples
--------

To create a grid file from an even spaced SEGY file test.segy, try

   ::

    gmt segy2grd test.segy -I0.1/0.1 -Gtest.nc -R198/208/18/25 -V

Note that this will read in 18-25s (or km) on each trace, but the
first trace will be assumed to be at X=198

To create a grid file from the SEGY file test.segy, locating traces
according to the CDP number, where there are 10 CDPs per km and the
sample interval is 0.1, try

   ::

    gmt segy2grd test.segy -Gtest.nc -R0/100/0/10 -I0.5/0.2 -V -X0.1 -Y0.1

Because the grid interval is larger than the SEGY file sampling, the
individual samples will be averaged in bins

See Also
--------

:doc:`gmt </gmt>`, :doc:`grd2xyz </grd2xyz>`,
:doc:`grdedit </grdedit>`, :doc:`pssegy`
