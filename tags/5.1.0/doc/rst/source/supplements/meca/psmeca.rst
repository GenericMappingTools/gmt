.. index:: ! psmeca

******
psmeca
******

.. only:: not man

    psmeca - Plot focal mechanisms on maps

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

psmeca [ *table* ] **-J**\ *parameters*
|SYN_OPT-R|
[ |SYN_OPT-B| ]
[ **-C**\ [*pen*\ ][\ **P**\ *pointsize*] ] [ **-D**\ *depmin*/*depmax* ]
[ **-E**\ *fill*] [ **-F**\ *mode*\ [*args*] ] [ **-G**\ *fill*] [ **-K** ] [ **-L**\ [*pen*\ ] ]
[ **-M** ] [ **-N** ] [ **-O** ] [ **-P** ]
[ **-S**\ *<format><scale>*\ [/**d**]]
[ **-T**\ *num\_of\_plane*\ [*pen*\ ] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ **-W**\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ **-Z**\ *cptfile*]
[ |SYN_OPT-c| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**psmeca** reads data values from *files* [or standard input] and
generates PostScript code that will plot focal mechanisms on a map.
Most options are the same as for **psxy**. The PostScript code is
written to standard output.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. include:: ../../explain_-J.rst_

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

**-S**\ *<format><scale>*\ [/**d**]

Selects the meaning of the columns in the data file . In order to use
the same file to plot cross-sections, depth is in third column.
Nevertheless, it is possible to use "old style" **psvelomeca** input
files without depth in third column using the **-o** option.

**-Sa**\ *scale[/fontsize[/offset*\ [**u**\ ]]]

Focal mechanisms in Aki and Richards convention. *scale* adjusts the
scaling of the radius of the "beach ball", which will be proportional to
the magnitude. Scale is the size for magnitude = 5 in inch (unless
**c**, **i**, **m**, or **p** is appended). Use the **-T** option to
render the beach ball transparent by drawing only the nodal planes and
the circumference. The color or shade of the compressive quadrants can
be specified with the **-G** option. The color or shade of the extensive
quadrants can be specified with the **-E** option. Append **u** to have
the text appear below the beach ball (default is above). Parameters are
expected to be in the following columns:

    **1**,\ **2**:
    longitude, latitude of event (**-:** option interchanges order)
    **3**:
    depth of event in kilometers
    **4**,\ **5**,\ **6**:
    strike, dip and rake in degrees
    **7**:
    magnitude
    **8**,\ **9**:
    longitude, latitude at which to place beach ball. Entries in these
    columns are necessary with the **-C** option. Using 0,0 in columns 8
    and 9 will plot the beach ball at the longitude, latitude given in
    columns 1 and 2. The **-:** option will interchange the order of
    columns (1,2) and (8,9).
    **10**:
    Text string to appear above or below the beach ball (optional).

**-Sc**\ *scale[/fontsize[/offset*\ [**u**\ ]]]

Focal mechanisms in Harvard CMT convention. *scale* adjusts the scaling
of the radius of the "beach ball", which will be proportional to the
magnitude. Scale is the size for magnitude = 5 (that is M0 = 4.0E23
dynes-cm) in inch (unless **c**, **i**, **m**, or **p** is appended).
Use the **-T** option to render the beach ball transparent by drawing
only the nodal planes and the circumference. The color or shade of the
compressive quadrants can be specified with the **-G** option. The color
or shade of the extensive quadrants can be specified with the **-E**
option. Append **u** to have the text appear below the beach ball
(default is above). Parameters are expected to be in the following
columns:

    **1**,\ **2**:
    longitude, latitude of event (**-:** option interchanges order)
    **3**:
    depth of event in kilometers
    **4**,\ **5**,\ **6**:
    strike, dip, and rake of plane 1
    **7**,\ **8**,\ **9**:
    strike, dip, and rake of plane 2
    **10**,\ **11**:
    mantissa and exponent of moment in dyne-cm
    **12**,\ **13**:
    longitude, latitude at which to place beach ball. Entries in these
    columns are necessary with the **-C** option. Using (0,0) in columns
    12 and 13 will plot the beach ball at the longitude, latitude given
    in columns 1 and 2. The **-:** option will interchange the order of
    columns (1,2) and (12,13).
    **14**:
    Text string to appear above or below the beach ball (optional).

**-Sm\|d\|z**\ *scale[/fontsize[/offset*\ [**u**\ ]]]

Seismic moment tensor (Harvard CMT, with zero trace). *scale* adjusts
the scaling of the radius of the "beach ball", which will be
proportional to the magnitude. Scale is the size for magnitude = 5 (that
is scalar seismic moment = 4.0E23 dynes-cm) in inch (unless **c**,
**i**, **m**, or **p** is appended). (**-T**\ *0* option overlays best
double couple transparently.) Use **-Sm** to plot the Harvard CMT
seismic moment tensor with zero trace. Use **-Sd** to plot only the
double couple part of moment tensor. Use **-Sz** to plot the anisotropic
part of moment tensor (zero trace). The color or shade of the
compressive quadrants can be specified with the **-G** option. The color
or shade of the extensive quadrants can be specified with the **-E**
option. Append **u** to have the text appear below the beach ball
(default is above). Parameters are expected to be in the following
columns:

    **1**,\ **2**:
    longitude, latitude of event (**-:** option interchanges order)
    **3**:
    depth of event in kilometers
    **4**,\ **5**,\ **6**,\ **7**,\ **8**,\ **9**:
    mrr, mtt, mff, mrt, mrf, mtf in 10\*exponent dynes-cm
    **10**:
    exponent
    **11**,\ **12**:
    longitude, latitude at which to place beach ball. Entries in these
    columns are necessary with the **-C** option. Using (0,0) in columns
    11 and 12 will plot the beach ball at the longitude, latitude given
    in columns 1 and 2. The **-:** option will interchange the order of
    columns (1,2) and (11,12).
    **13**:
    Text string to appear above or below the beach ball (optional).

**-Sp**\ *scale[/fontsize[/offset*\ [**u**\ ]]]

Focal mechanisms given with partial data on both planes. *scale* adjusts
the scaling of the radius of the "beach ball", which will be
proportional to the magnitude. Scale is the size for magnitude = 5 in
inch (unless **c**, **i**, **m**, or **p** is appended). The color or
shade of the compressive quadrants can be specified with the **-G**
option. The color or shade of the extensive quadrants can be specified
with the **-E** option. Append **u** to have the text appear below the
beach ball (default is above). Parameters are expected to be in the
following columns:

    **1**,\ **2**:
    longitude, latitude of event (**-:** option interchanges order)
    **3**:
    depth of event in kilometers
    **4**,\ **5**:
    strike, dip of plane 1
    **6**:
    strike of plane 2
    **7**:
    must be -1/+1 for a normal/inverse fault
    **8**:
    magnitude
    **9**,\ **10**:
    longitude, latitude at which to place beach ball. Entries in these
    columns are necessary with the **-C** option. Using (0,0) in columns
    9 and 10 will plot the beach ball at the longitude, latitude given
    in columns 1 and 2. The **-:** option will interchange the order of
    columns (1,2) and (9,10).
    **11**:
    Text string to appear above or below the beach ball (optional).

**-Sx\|y\|t**\ *scale[/fontsize[/offset*\ [**u**\ ]]]

Principal axis. *scale* adjusts the scaling of the radius of the "beach
ball", which will be proportional to the magnitude. Scale is the size
for magnitude = 5 (that is seismic scalar moment = 4\*10e+23 dynes-cm)
in inch (unless **c**, **i**, **m**, or **p** is appended). (**-T**\ *0*
option overlays best double couple transparently.) Use **-Sx** to plot
standard Harvard CMT. Use **-Sy** to plot only the double couple part of
moment tensor. Use **-St** to plot zero trace moment tensor. The color
or shade of the compressive quadrants can be specified with the **-G**
option. The color or shade of the extensive quadrants can be specified
with the **-E** option. Append **u** to have the text appear below the
beach ball (default is above). Parameters are expected to be in the
following columns:

    **1**,\ **2**:
    longitude, latitude of event (**-:** option interchanges order)
    **3**:
    depth of event in kilometers
    **4**,\ **5**,\ **6**,\ **7**,\ **8**,\ **9**,\ **10**,\ **11**,\ **12**:
    value (in 10\*exponent dynes-cm), azimuth, plunge of T, N, P axis.
    **13**:
    exponent
    **14**,\ **15**:
    longitude, latitude at which to place beach ball. Entries in these
    columns are necessary with the **-C** option. Using (0,0) in columns
    14 and 15 will plot the beach ball at the longitude, latitude given
    in columns 1 and 2. The **-:** option will interchange the order of
    columns (1,2) and (14,15).
    **16**:
    Text string to appear above or below the beach ball (optional).

`Optional Arguments <#toc5>`_
-----------------------------

.. include:: ../../explain_-B.rst_

**-C**\ [*pen*\ ][\ **P**\ *pointsize*]
    Offsets focal mechanisms to the longitude, latitude specified in the
    last two columns of the input file before the (optional) text
    string. A small circle is plotted at the initial location and a line
    connects the beachball to the circle. Specify *pen* and/or
    *pointsize* to change the line style and/or size of the circle.
    [Defaults: *pen* as given by **-W**; *pointsize* 0].
**-D**\ *depmin/depmax*
    Plots events between depmin and depmax.
**-E**\ *fill*
    Selects filling of extensive quadrants. Usually white. Set the color
    [Default is white].
**-F**\ *mode*\ [*args*]
    Sets one or more attributes; repeatable. The various combinations are

**-Fa**\ [*size*\ ][/\ *P\_axis\_symbol*\ [*T\_axis\_symbol*\ ]]
    Computes and plots P and T axes with symbols. Optionally specify
    *size* and (separate) P and T axis symbols from the following:
    (**c**) circle, (**d**) diamond, (**h**) hexagon, (**i**) inverse
    triangle, (**p**) point, (**s**) square, (**t**) triangle, (**x**)
    cross. [Default: 6\ **p**/**cc**]
**-Fe**\ *fill*
    Sets the color or fill pattern for the T axis symbol. [Default as
    set by **-E**]
**-Fg**\ *fill*
    Sets the color or fill pattern for the P axis symbol. [Default as
    set by **-G**]
**-Fo**
    Use the **psvelomeca** input format without depth in the third column.
**-Fp**\ [*pen*\ ]
    Draws the P axis outline using default pen (see **-W**), or sets pen attributes.
**-Fr**\ [*fill*\ ]
    Draw a box behind the label (if any). [Default fill is white]
**-Ft**\ [*pen*\ ]
    Draws the T axis outline using default pen (see **-W**), or sets pen
    attributes.
**-Fz**\ [*pen*\ ]
    Overlay zero trace moment tensor using default pen (see **-W**), or
    sets pen attributes.

**-G**\ *fill*
    Selects filling of focal mechanisms. By convention, the
    compressional quadrants of the focal mechanism beach balls are
    shaded. Set the color [Default is black].

.. include:: ../../explain_-K.rst_

**-L**\ *pen*
    Draws the "beach ball" outline with *pen* attributes instead of with
    the default pen set by **-W**.
**-M**
    Use the same size for any magnitude. Size is given with **-S**.
**-N**
    Does **not** skip symbols that fall outside frame boundary specified
    by **-R** [Default plots symbols inside frame only].

.. include:: ../../explain_-O.rst_
.. include:: ../../explain_-P.rst_

**-T**\ [*num\_of\_planes*\ ][\ **/**\ *pen*]
    Plots the nodal planes and outlines the bubble which is transparent.
    If *num\_of\_planes* is

    *0*: both nodal planes are plotted;

    *1*: only the first nodal plane is plotted;

    *2*: only the second nodal plane is plotted.

    Append **/**\ *pen* to set the pen attributes for this feature.
    Default pen is as set by **-W**.

.. include:: ../../explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

**-W**\ *pen*
    Set pen attributes for all lines and the outline of symbols
    [Defaults: width = default, color = black, style = solid]. This
    setting applies to **-C**, **-L**, **-T**, **-p**, **-t**, and
    **-z**, unless overruled by options to those arguments.

.. include:: ../../explain_-XY.rst_

**-Z**\ *cptfile*
    Give a color palette file and let compressive part color be
    determined by the z-value in the third column.

.. include:: ../../explain_-c.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_
.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

Examples
--------

The following file should give a normal-faulting CMT mechanism:

   ::

    gmt psmeca -R239/240/34/35.2 -Jm4c -Sc0.4 -h1 << END > test.ps
    lon lat depth str dip slip st dip slip mant exp plon plat
    239.384 34.556 12. 180 18 -88 0 72 -90 5.5 0 0 0
    END

See Also
--------

:doc:`gmt </gmt>`, :doc:`psbasemap </psbasemap>`, :doc:`psxy </psxy>`

`References <#toc8>`_
---------------------

Bomford, G., Geodesy, 4th ed., Oxford University Press, 1980.

Aki, K. and P. Richards, Quantitative Seismology, Freeman, 1980.

F. A. Dahlen and Jeroen Tromp, Theoretical Seismology, Princeton, 1998,
p.167.

Cliff Frohlich, Cliff’s Nodes Concerning Plotting Nodal Lines for P, Sh
and Sv

Seismological Research Letters, Volume 67, Number 1, January-February,
1996

Thorne Lay, Terry C. Wallace, Modern Global Seismology, Academic Press,
1995, p.384.

W.H. Press, S.A. Teukolsky, W.T. Vetterling, B.P. Flannery, Numerical
Recipes in C, Cambridge University press (routine jacobi)

`Authors <#toc9>`_
------------------

Genevieve Patau, `Laboratory of Seismogenesis <http://www.ipgp.fr/rech/sismogenese/>`,
Institut de Physique du Globe de Paris, Departement de Sismologie, Paris, France
