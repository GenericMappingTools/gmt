.. index:: ! coupe
.. include:: ../module_supplements_purpose.rst_

*******
coupe
*******

|coupe_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt coupe**
[ *table* ]
|-J|\ *parameters*
|SYN_OPT-R|
|-A|\ **a**\|\ **b**\|\ **c**\|\ **d**\ *params*\ [**+c**\ [**n**\|\ **t**]][**+d**\ *dip*][**+r**\ [**a**\|\ **e**\|\ *dx*]][**+w**\ *width*][**+z**\ [**s**]\ **a**\|\ **e**\|\ *dz*\|\ *min*/*max*]
|-S|\ *format*\ [*scale*][**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+l**][**+m**][**+o**\ *dx*\ [/*dy*]][**+s**\ *reference*]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-E|\ *fill* ]
[ |-F|\ *mode*\ [*args*] ]
[ |-G|\ *fill* ]
[ |-H|\ [*scale*] ]
[ |-I|\ [*intens*] ]
[ |-L|\ [*pen*] ]
[ |-N| ]
[ |-Q| ]
[ |-T|\ *nplane*\ [/*pen*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-tv| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads data values from *table* [or standard input] and
will plot a cross-section of focal mechanisms.
The name "coupe" comes from the French
verb “to cut”.  The best translation is a (vertical) cross section.

Unless |-Q| is used, a new file is created with the new coordinates
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

.. |Add_-J| replace:: |Add_-J_links|
.. include:: /explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-R:

.. |Add_-Rgeo| replace:: If frame is defined from cross-section parameters
   (see |-A| this option is not taken into account, but must be present.
.. include:: ../../explain_-Rgeo.rst_

.. _-A:

**-Aa**\|\ **b**\|\ **c**\|\ **d**\ *params*\ [**+c**\ [**n**\|\ **t**]][**+d**\ *dip*][**+r**\ [**a**\|\ **e**\|\ *dx*]][**+w**\ *width*][**+z**\ [**s**]\ **a**\|\ **e**\|\ *dz*\|\ *min*/*max*]
  Select the cross-section parameters:

  **-Aa**\ *lon1*/*lat1*/*lon2*/*lat2*
     *lon* and *lat* are the longitude and latitude of points 1 and 2
     limiting the length of the cross-section, *dip* is the dip of the plane
     on which the cross-section is made [90], *width* is the width in km of the
     cross-section on each side of a vertical plane or above and under an
     oblique plane [infinity], and *min* and *max* are the limits on distances from
     horizontal plane in km, along steepest descent direction. Add **+r** to get the
     plot domain from the cross-section parameters; append **a** for automatic rounding
     of the domain, **e** for the exact limits [Default], or *dx* to round the distances
     to integer multiples of *dx*. Use **+z** to control the depth range by appending
     **a** for automatic rounding, **e** to use the exact fit values [Default], *dz* to round
     depths to integer multiples of *dz*, or give desired *min/max* range. For
     **a** and *dz* you may prepend **s** to clamp the minimum depth at the surface (0).
     When automatic depth range selection is in effect we consider the size of the symbols
     so that no symbol close to the depth limits will be clipped.
     **Note**: Append **+c** to simply report the determined region and exit (no plotting takes
     places).  By default (**+cn**) we report a single numerical record with *xmin xmax ymin ymax*.
     Use **+ct** to instead report a text record in the format **-R**\ *xmin/xmax/ymin/ymax*.

  **-Ab**\ *lon1*/*lat1*/*strike*/*length*
     *lon1* and *lat1* are the longitude and latitude of the beginning of the
     cross-section, *strike* is the azimuth of the direction of the
     cross-section, and *length* is the length along which the cross-section
     is made (in km). The other parameters are the same as for **-Aa** option.

  **-Ac**\ *x1*/*y1*/*x2*/*y2*
     The same as **-Aa** option with *x* and *y* given as Cartesian coordinates.

  **-Ad**\ *x1*/*y1*/*strike*/*length*
     The same as **-Ab** option with *x* and *y* given as Cartesian coordinates.

.. _-S:

.. include:: explain_meca_-S.rst_

Optional Arguments
------------------

.. |Add_-B| replace:: |Add_-B_links|
.. include:: ../../explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *cpt*
    Give a CPT and let compressive part color be
    determined by the z-value in the third column.

.. _-E:

**-E**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Set color or fill pattern for extensive quadrants [Default is white].

.. _-F:

**-F**\ *mode*\ [*args*]
    Set one or more attributes; repeatable. The various combinations are

**-Fs**\ *symbol*\ [*size*]
    Select a symbol instead of mechanism. Choose from the following:
    (**c**) circle, (**d**) diamond, (**i**) itriangle, (**s**) square,
    (**t**) triangle, (**x**) cross. *size* is the symbol size in
    :term:`PROJ_LENGTH_UNIT` (unless **c**, **i**, or **p** is appended
    to indicate that the size information is in units of cm, inches, meters,
    or points, respectively). If *size* must be read, it must be in column 4
    and the text string will start in column 5.

    Parameters are expected to be in the following columns:

    **1**,\ **2**:
      longitude, latitude of event (**-:** option interchanges order)
    **3**:
      depth of event in kilometers
    **4**:
      Text string to appear near the beach ball

**-Fa**\ [*size*\ [/*Psymbol*\ [*Tsymbol*]]]
    Compute and plot P and T axes with symbols. Optionally specify
    *size* and (separate) P and T axis symbols from the following:
    (**c**) circle, (**d**) diamond, (**h**) hexagon, (**i**) inverse
    triangle, (**p**) point, (**s**) square, (**t**) triangle, (**x**)
    cross. [Default: 6\ **p**/**cc**]

**-Fe**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Set the color or fill pattern for the T axis symbol. [Default as set by |-E|]

**-Fg**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Set the color or fill pattern for the P axis symbol. [Default as set by |-G|]

**-Fp**\ [*pen*]
    Draw the P axis outline using current pen (see |-W|), or sets pen attributes.

**-Fr**\ [*fill*]
    Draw a box behind the label (if any). [Default fill is white]

**-Ft**\ [*pen*]
    Draw the T axis outline using current pen (see |-W|), or sets pen attributes.

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Set color or fill pattern for compressional quadrants [Default is black].

.. _-H:

**-H**\ [*scale*]
    Scale symbol sizes and pen widths on a per-record basis using the *scale* read from the
    data set, given as the first column after the (optional) *size* columns [no scaling].
    The symbol size is either provided by |-S| or via the input *size* column.  Alternatively,
    append a constant *scale* that should be used instead of reading a scale column.

.. _-I:

**-I**\ *intens*
    Use the supplied *intens* value (nominally in the -1 to +1 range) to
    modulate the compressional fill color by simulating illumination [none].
    If no intensity is provided we will instead read *intens* from an extra
    data column after the required input columns determined by |-S|.

.. _-L:

**-L**\ [*pen*]
    Draw the "beach ball" outline using current pen (see |-W|) or
    sets pen attributes.

.. _-N:

**-N**
    Does **not** skip symbols that fall outside map border [Default plots points inside border only].

.. _-Q:

**-Q**
    Suppress the production of files with cross-section and mechanism information.

.. _-T:

**-T**\ [*nplane*][**/**\ *pen*]
    Plot the nodal planes and outlines the bubble which is transparent.
    If *nplane* is

    *0*: both nodal planes are plotted;

    *1*: only the first nodal plane is plotted;

    *2*: only the second nodal plane is plotted.

    Append **/**\ *pen* to set the pen attributes for this feature.
    Default pen is as set by |-W|. [Default: 0].

    For double couple mechanisms, the |-T| option renders the beach ball transparent
    by drawing only the nodal planes and the circumference.
    For non-double couple mechanisms, **-T**\ *0* option overlays best double couple transparently.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: ../../explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*pen*] :ref:`(more ...) <-Wpen_attrib>`
    Set pen attributes for text string or default pen attributes for
    fault plane edges. [Defaults: 0.25p,black,solid].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: ../../explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-qi.rst_

.. include:: ../../explain_-tv_full.rst_

.. include:: ../../explain_colon.rst_

.. include:: ../../explain_help.rst_

.. include:: seis_extra_cols.rst_

.. module_common_ends

Examples
--------

The following will plot a cross section of three focal mechanisms::

    gmt coupe << END -Sa1c -Aa111/33/119/33+d90+w500+z0/50+r -Q \
		-JX15c/-8c -Bxaf+l"Distance (km)" -Byaf+l"Depth (km)" -BWSen -png test
    112 32 25  30  90   0  4  Strike-slip
    115 34 15  30  60  90  5  Reverse
    118 32 45  30  60 -90  6  Normal
    END

.. include:: meca_notes.rst_

See Also
--------

:doc:`meca`,
:doc:`polar`,
:doc:`gmt </gmt>`, :doc:`basemap </basemap>`,
:doc:`plot </plot>`
