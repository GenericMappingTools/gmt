.. index:: ! segy2grd

********
segy2grd
********

.. only:: not man

    segy2grd - Converting SEGY data to a GMT grid

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt segy2grd** *segyfile* |-G|\ *grdfile*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ [**n**\ \|\ **z**] ]
[ |-D|\ [**+x**\ *xname*][**+y**\ *yname*][**+z**\ *zname*][**+s**\ *scale*][**+o**\ *offset*][**+n**\ *invalid*][**+t**\ *title*][**+r**\ *remark*] ]
[ |-L|\ [*nsamp*\ ] ]
[ |-M|\ [*ntraces*\ ] ]
[ |-N|\ *nodata* ]
[ |-Q|\ *<mode><value>* ]
[ |-S|\ [*header*] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**segy2grd** reads an IEEE SEGY file and creates a binary grid file.
Either a simple mapping (equivalent to :doc:`xyz2grd </xyz2grd>` -Z) or a more complicated
averaging where a particular grid cell includes values from more than
one sample in the SEGY file can be done. **segy2grd** will report if
some of the nodes are not filled in with data. Such unconstrained nodes
are set to a value specified by the user [Default is NaN]. Nodes with
more than one value will be set to the average value.

Required Arguments
------------------

*segyfile* is an IEEE floating point SEGY file. Traces are all assumed to start at 0 time/depth.

.. _-G:

**-G**\ *grdfile*
    *grdfile* is the name of the binary output grid file.

.. _-I:

**-I**
    *x_inc* [and optionally *y_inc*] is the grid spacing. Append **m**
    to indicate minutes or **s** to indicate seconds.

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [**n**\ \|\ **z**]
    Add up multiple values that belong to the same node (same as
    **-Az**). Append **n** to simply count the number of data points
    that were assigned to each node. [Default (no **-A** option) will
    calculate mean value]. Not used for simple mapping.

.. _-D:

.. include:: ../../explain_-D_cap.rst_

.. _-L:

**-L**
    Let *nsamp* override number of samples in each trace.

.. _-M:

**-M**\ [*ntraces*\ ]
    Fix number of traces to read in. Default tries to read 10000 traces.
    **-M**\ 0 will read number in binary header, **-M**\ *ntraces* will
    attempt to read only *n* traces.

.. _-N:

**-N**\ *nodata*
    No data. Set nodes with no input sample to this value [Default is
    NaN].

.. _-Q:

**-Q**\ *<mode><value>*
    Can be used to change two different settings depending on *mode*:
       **-Qx**\ *x-scale* applies scalar *x-scale* to coordinates in trace
       header to match the coordinates specified in **-R**.

       **-Qy**\ *s_int* specifies sample interval as *s_int* if incorrect in the SEGY file.

.. _-S:

**-S**\ [*header*\ ]
    Set variable spacing; *header* is **c** for cdp, **o** for offset, or **b**\ *number*
    for 4-byte float starting at byte *number*. If **-S** not set, assumes even
    spacing of samples at the *x_inc, y_inc* supplied with **-I**.

.. _-V:

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

    gmt segy2grd test.segy -Gtest.nc -R0/100/0/10 -I0.5/0.2 -V -Qx0.1 -Qy0.1

Because the grid interval is larger than the SEGY file sampling, the
individual samples will be averaged in bins

See Also
--------

:doc:`gmt </gmt>`, :doc:`grd2xyz </grd2xyz>`,
:doc:`grdedit </grdedit>`, :doc:`segy`,
:doc:`xyz2grd </xyz2grd>`
