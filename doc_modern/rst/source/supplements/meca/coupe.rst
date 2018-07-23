.. index:: ! coupe

*******
coupe
*******

.. only:: not man

    Plot cross-sections of focal mechanisms

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt coupe** [ *files* ] |-J|\ *parameters*
|SYN_OPT-R| |-A|\ *parameters*
[ |SYN_OPT-B| ]
[ |-E|\ *color* ]
[ |-F|\ *mode*\ [*args*] ]
[ |-G|\ *color* ]
[ |-L|\ *[pen]* ]
[ |-M| ] [ |-N| ]
[ |-Q| ]
[ |-S|\ *<symbol><scale>*\ [/**d**] ]
[ |-T|\ *n* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ *cpt* ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**coupe** reads data values from *files* [or standard input] and
will plot symbols, lines or polygons on
a cross-section. Focal mechanisms may be specified and require
additional columns of data.

Unless |-Q| is used, new file is created with the new coordinates
(**x**, **y**) and the mechanism (from lower focal half-sphere for
horizontal plane, to half-sphere behind a vertical plane). When the
plane is not horizontal,
- north direction becomes upwards steepest descent direction of the plane (u)
- east direction becomes strike direction of the plane (s)
- down direction (= north^east) becomes u^s
Axis angles are defined in the same way as in horizontal plane in the new system.
Moment tensor (initially in r, t, f system that is up, south, east) is
defined in (-u^s, -u, s) system.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-J.rst_

.. _-R:

.. |Add_-Rgeo| replace:: If frame is defined from cross-section parameters
   (see **-A** this option is not taken into account, but must be present.
.. include:: ../../explain_-Rgeo.rst_

.. _-A:

**-A**
  selects the cross-section.

  **-Aa**\ *lon1/lat1/lon2/lat2/dip/p\_width/dmin/dmax*\ [**f**]
     *lon* and *lat* are the longitude and latitude of points 1 and 2
     limiting the length of the cross-section. *dip* is the dip of the plane
     on which the cross-section is made. *p\_width* is the width of the
     cross-section on each side of a vertical plane or above and under an
     oblique plane. *dmin* and *dmax* are the distances min and max from
     horizontal plane, along steepest descent direction. Add **f** to get the
     frame from the cross-section parameters.

  **-Ab**\ *lon1/lat1/strike/p\_length/dip/p\_width/dmin/dmax*\ [**f**\ ]
     *lon1* and *lat1* are the longitude and latitude of the beginning of the
     cross-section. *strike* is the azimuth of the direction of the
     cross-section. *p\_length* is the length along which the cross-section
     is made. The other parameters are the same as for **-Aa** option.

  **-Ac**\ *x1/y1/x2/y2/dip/p\_width/dmin/dmax*\ [**f**\ ]
     The same as **-Aa** option with *x* and *y* cartesian coordinates.

  **-Ad**\ *x1/y1/strike/p\_length/dip/p\_width/dmin/dmax*\ [**f**\ ]
     The same as **-Ab** option with *x* and *y* cartesian coordinates.

.. _-S:

**-S**
  selects the meaning of the columns in the data file and the figure to be plotted.

  **-Sa**\ *scale[/fontsize[/offset*\ [**u**]]]
     Focal mechanisms in Aki and Richards convention. *scale* adjusts the
     scaling of the radius of the "beach ball", which will be proportional to
     the magnitude. The *scale* is the size for magnitude = 5 in
     **PROJ_LENGTH_UNIT** (unless **c**, **i**, or **p** is appended
     to indicate that the size information is in units of cm, inches, meters,
     or points, respectively). Use the **-T** option to render the beach ball
     transparent by drawing only the nodal planes and the circumference. The
     color or shade of the compressive quadrants can be specified with the
     **-G** option. The color or shade of the extensive quadrants can be
     specified with the **-E** option. Parameters are expected to be in the
     following columns:

      **1**,\ **2**:
        longitude, latitude of event (**-:** option interchanges order)
      **3**:
        depth of event in kilometers
      **4**,\ **5**,\ **6**:
        strike, dip and rake
      **7**:
        magnitude
      **8**,\ **9**:
        not used; can be 0 0; allows use of the :doc:`meca` file format
      **10**:
        text string to appear above the beach ball (default) or under (add **u**).

  **-Sc**\ *scale*
     Focal mechanisms in Harvard CMT convention. *scale* adjusts the scaling
     of the radius of the "beach ball", which will be proportional to the
     magnitude. The *scale* is the size for magnitude = 5 (that is M0 = 4E+23
     dynes-cm.) in **PROJ_LENGTH_UNIT** (unless **c**, **i**, or
     **p** is appended to indicate that the size information is in units of
     cm, inches, meters, or points, respectively). Use the **-T** option to
     render the beach ball transparent by drawing only the nodal planes and
     the circumference. The color or shade of the compressive quadrants can
     be specified with the **-G** option. The color or shade of the extensive
     quadrants can be specified with the **-E** option. Parameters are
     expected to be in the following columns:

      **1**,\ **2**:
        longitude, latitude of event (**-:** option interchanges order)
      **3**:
        depth of event in kilometers
      **4**,\ **5**,\ **6**:
        strike, dip, and slip of plane 1
      **7**,\ **8**,\ **9**:
        strike, dip, and slip of plane 2
      **10**,\ **11**:
        mantissa and exponent of moment in dyne-cm (if magnitude is uses
        instead of scalar moment, magnitude is in column 10 and 0 must be in column 11)
      **12**,\ **13**:
        not used; can be 0 0; allows use of the :doc:`meca` file format
      **14**:
        text string to appear above the beach ball (default) or under (add **u**).

  **-Sp**\ *scale[/fontsize[/offset*\ [**u**]]]
     Focal mechanisms given with partial data on both planes. *scale* adjusts
     the scaling of the radius of the "beach ball", which will be
     proportional to the magnitude. The *scale* is the size for magnitude = 5
     in **PROJ_LENGTH_UNIT** (unless **c**, **i**, or **p** is
     appended to indicate that the size information is in units of cm,
     inches, meters, or points, respectively). The color or shade of the
     compressive quadrants can be specified with the **-G** option. The color
     or shade of the extensive quadrants can be specified with the **-E**
     option. Parameters are expected to be in the following columns:

      **1**,\ **2**:
        longitude, latitude of event (**-:** option interchanges order)
      **3**:
        depth
      **4**,\ **5**:
        strike, dip of plane 1
      **6**:
        strike of plane 2
      **7**:
        must be -1/+1 for a normal/inverse fault
      **8**:
        magnitude
      **9**,\ **10**:
        not used; can be 0 0; allows use of the :doc:`meca` file format
      **11**:
        text string to appear above the beach ball (default) or under (add **u**).

  **-Sm\|d\|z**\ *scale[/fontsize[/offset*\ [**u**]]]
     Seismic moment tensor (Harvard CMT, with zero trace). *scale* adjusts
     the scaling of the radius of the "beach ball", which will be
     proportional to the magnitude. The *scale* is the size for magnitude = 5
     (that is seismic scalar moment = 4E+23 dynes-cm) in
     **PROJ_LENGTH_UNIT** (unless **c**, **i**, or **p** is appended
     to indicate that the size information is in units of cm, inches, meters,
     or points, respectively). (**-T**\ *0* option overlays best double
     couple transparently.)
  **-Sd**\ *scale[/fontsize[/offset*\ [**u**]]]
     to plot the only double couple part of moment tensor.
  **-Sz**\ *scale[/fontsize[/offset*\ [**u**]]]
     to plot anisotropic part
     of moment tensor (zero trace). The color or shade of the compressive
     quadrants can be specified with the **-G** option. The color or shade of
     the extensive quadrants can be specified with the **-E** option.
     Parameters are expected to be in the following columns:

      **1**,\ **2**:
         longitude, latitude of event (**-:** option interchanges order)
      **3**:
         depth of event in kilometers
      **4**,\ **5**,\ **6**,\ **7**,\ **8**,\ **9**:
         mrr, mtt, mff, mrt, mrf, mtf in 10\*exponent dynes-cm
      **10**:
         exponent
      **11**,\ **12**:
         Not used; can be 0 0; allows use of the :doc:`meca` file format 
      **13**:
         Text string to appear above the beach ball (default) or under (add **u**).

  **-Sx**\ *scale[/fontsize[/offset*\ [**u**]]]
     Principal axis. *scale* adjusts the scaling of the radius of the "beach
     ball", which will be proportional to the magnitude. The *scale* is the
     size for magnitude = 5 (that is seismic scalar moment = 4\*10e+23
     dynes-cm) in **PROJ\_LENGTH\_UNIT** (unless **c**, **i**, or
     **p** is appended to indicate that the size information is in units of
     cm, inches, meters, or points, respectively). (**-T0** option overlays
     best double couple transparently.)
  **-Sy**\ *scale[/fontsize[/offset*\ [**u**]]]
     to plot the only double couple part of moment tensor.
  **-St**\ *scale[/fontsize[/offset*\ [**u**]]]
     to plot anisotropic part
     of moment tensor (zero trace). The color or shade of the compressive
     quadrants can be specified with the **-G** option. The color or shade of
     the extensive quadrants can be specified with the **-E** option.
     Parameters are expected to be in the following columns:

      **1**,\ **2**:
        longitude, latitude of event (**-:** option interchanges order)
      **3**:
        depth of event in kilometers
      **4**,\ **5**,\ **6**,\ **7**,\ **8**,\ **9**,\ **10**,\ **11**,\ **12**:
        value (in 10\*exponent dynes-cm), azimuth, plunge of the T, N, and P axes.
      **13**:
        exponent
      **14**,\ **15**:
        longitude, latitude at which to place beach ball. Entries in these
        columns are necessary with the **-C** option. Using 0,0 in columns 9
        and 10 will plot the beach ball at the longitude, latitude given in
        columns 1 and 2. The **-:** option will interchange the order of
        columns (1,2) and (9,10).
      **16**:
        Text string to appear above the beach ball (optional).

Optional Arguments
------------------

.. _-B:

.. include:: ../../explain_-B.rst_

.. _-E:

**-E**\ *color*
    Sets color or fill pattern for extensive quadrants [Default is white].

.. _-F:

**-F**\ *mode*\ [*args*]
    Sets one or more attributes; repeatable. The various combinations are

**-Fs**\ *symbol[size[/fontsize[/offset*\ [**u**]]]
   selects a symbol instead of mechanism. Choose from the following:
   (**c**) circle, (**d**) diamond, (**i**) itriangle, (**s**) square,
   (**t**) triangle, (**x**) cross. *size* is the symbol size in
   **PROJ_LENGTH_UNIT** (unless **c**, **i**, or **p** is appended
   to indicate that the size information is in units of cm, inches, meters,
   or points, respectively). If *size* must be read, it must be in column 4
   and the text string will start in column 5. Parameters are expected to
   be in the following columns:

    **1**,\ **2**:
      longitude, latitude of event (**-:** option interchanges order)
    **3**:
      depth of event in kilometers
    **4**:
      Text string to appear above the beach ball (default) or under (add **u**).

**-Fa**\ [*size*][/\ *P\_symbol*\ [/\ *T\_symbol*]]
    Computes and plots P and T axes with symbols. Optionally specify
    *size* and (separate) P and T axis symbols from the following:
    (**c**) circle, (**d**) diamond, (**h**) hexagon, (**i**) inverse
    triangle, (**p**) point, (**s**) square, (**t**) triangle, (**x**)
    cross. [Default: 6\ **p**/**cc**]

**-Fe**\ *color*
    Sets the color or fill pattern for the T axis symbol. [Default as set by |-E|]

**-Fg**\ *color*
    Sets the color or fill pattern for the P axis symbol. [Default as set by |-G|]

**-Fp**\ [*pen*]
    Draws the P axis outline using current pen (see |-W|), or sets pen attributes.

**-Fr**\ [*color*]
    Draw a box behind the label (if any). [Default fill is white]

**-Ft**\ [*pen*]
    Draws the T axis outline using current pen (see **-W**), or sets pen attributes.

.. _-G:

**-G**\ *color*
    Sets color or fill pattern for compressional quadrants [Default is black].

.. _-L:

**-L**\ [*pen*]
    Draws the "beach ball" outline using current pen (see |-W|) or
    sets pen attributes.

.. _-M:

**-M**
    Same size for any magnitude.

.. _-N:

**-N**
    Does **not** skip symbols that fall outside map border [Default plots points inside border only].

.. _-Q:

**-Q**
    Suppress the production of files with cross-section and mechanism information.

.. _-T:

**-T**\ [*num_of_planes*]
    Plots the nodal planes and outlines the bubble which is transparent.
    If *num_of_planes* is
    *0*: both nodal planes are plotted;
    *1*: only the first nodal plane is plotted;
    *2*: only the second nodal plane is plotted
    [Default: 0].

.. _-U:

.. include:: ../../explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**\ [**-**\ \|\ **+**][*pen*][*attr*] :ref:`(more ...) <-Wpen_attrib>`
    set pen attributes for text string or default pen attributes for
    fault plane edges. [Defaults: width = default, color = black, style = solid].

.. _-X:

.. include:: ../../explain_-XY.rst_

.. _-Z:

**-Z**\ *cpt*
    Give a CPT and let compressive part color be
    determined by the z-value in the third column.

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-t.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

See Also
--------

:doc:`meca`,
:doc:`velo`,
:doc:`polar`,
:doc:`gmt </gmt>`, :doc:`basemap </basemap>`,
:doc:`plot </plot>`

References
----------

Bomford, G., Geodesy, 4th ed., Oxford University Press, 1980.

Aki, K. and P. Richards, Quantitative Seismology, Freeman, 1980.

F. A. Dahlen and Jeroen Tromp, Theoretical Seismology, Princeton, 1998,
p.167. Definition of scalar moment.

Cliff Frohlich, Cliff's Nodes Concerning Plotting Nodal Lines for P, Sh and Sv

Seismological Research Letters, Volume 67, Number 1, January-February, 1996

Thorne Lay, Terry C. Wallace, Modern Global Seismology, Academic Press, 1995, p.384.

W.H. Press, S.A. Teukolsky, W.T. Vetterling, B.P. Flannery, Numerical
Recipes in C, Cambridge University press (routine jacobi)

Author
------

Genevieve Patau, `Laboratory of Seismogenesis <http://www.ipgp.fr/rech/sismogenese/>`,
Institut de Physique du Globe de Paris, Departement de Sismologie, Paris, France
