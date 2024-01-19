.. index:: ! grdbarb
.. include:: ../module_supplements_purpose.rst_

*******
grdbarb
*******

.. only:: not man

|grdbarb_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**grdbarb**
*compx.nc* *compy.nc*
**-J**\ *parameters*
[ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-I|\ [**x**]\ *dx*\ [/*dy*] ]
[ |-N| ]
[ |-Q|\ *length*\ [**+a**\ *angle*][**+g**\ -\|\ *fill*][**+jb**\|\ **c**\|\ **e**][**+p**\ -\|\ *pen*][**+s**\ *scale*][**+w**\ *width*] ]
[ |SYN_OPT-R| ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]

|No-spaces|

Description
-----------

**grdbarb** reads two 2-D grid files which represents the *x*\ - and
*y*\ -components of a wind field and produces a wind field plot by
drawing wind barbs with orientation and barbs according to the information
in the files. Alternatively, polar coordinate *r*, *theta* grids may be given
instead.

Required Arguments
------------------

*compx.nc*
    Contains the x-components of the wind field. (See :ref:`Grid File Formats
    <grd_inout_full>`).
*compy.nc*
    Contains the y-components of the wind field. (See :ref:`Grid File Formats
    <grd_inout_full>`).

.. _-J:

.. |Add_-J| replace:: |Add_-J_links|
.. include:: /explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Optional Arguments
------------------

.. _-A:

**-A**
    The grid files contain polar (speed, theta) wind components instead of
    Cartesian (u, v) components [Default is (u, v)].

.. |Add_-B| replace:: |Add_-B_links|
.. include:: ../../explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ [*cpt*]
    Use *cpt* to assign colors based on wind speed. Alternatively,
    supply the name of a GMT color master dynamic CPT [rainbow] to
    automatically determine a continuous CPT from
    the grid's z-range.  If the dynamic CPT has a default range then
    that range will be imposed instead.
    Yet another option is to specify -Ccolor1,color2[,color3,...]
    to build a linear continuous cpt from those colors automatically.
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).

.. _-G:

**-G**\ *fill*
    Sets color or shade for wind barb interiors [Default is no fill].

.. _-I:

**-I**\ [**x**]\ *dx*\ [/*dy*]
    Only plot wind barbs at nodes every *x\_inc*, *y\_inc* apart (must be
    multiples of original grid spacing). Append **m** for arc minutes or
    **s** for arc seconds.  Alternatively, use **-Ix** to specify the
    multiples *multx*\ [/*multy*] directly [Default plots every node].

.. include:: ../../explain_-K.rst_

.. _-N:

**-N**
    Do NOT clip wind barbs at map boundaries [Default will clip].

.. _-Q:

**-Q**\ *length*\ [**+a**\ *angle*][**+g**\ -\|\ *fill*][**+jb**\|\ **c**\|\ **e**][**+p**\ -\|\ *pen*][**+s**\ *scale*][**+w**\ *width*]
    Modify wind barb parameters. Append wind barb *length* [Default is 0.5c].
    Several modifiers may be appended to specify the placement of barbs, their shapes, and the
    justification of the wind barb. Below, left and right refers to the
    side of the wind barb line when viewed from the start point to the
    end point of the segment. Chose among these modifiers:

    - **+a** - Set the angle of the wind barb [120].
    - **+g** - Turn off *fill* (if -) or set the wind
      barb fill [Default fill is used, which may be no fill].
    - **+p** - Sets the wind barb pen attributes. If *pen* has a
      leading - then the outline is not drawn [Default pen is used, and
      outline is drawn].
    - **+j** - Determines how the input *x*,\ *y* point relates to the
      wind barb. Choose from **b**\ eginning [default], **e**\ nd, or **c**\ enter.
    - **+s** - Set the wind speed which corresponds to a long barb [default 5].
    - **+w** - Set the *width* of wind barbs.
 
.. _-R:

.. |Add_-Rgeo| replace:: |Add_-R_auto_table|
.. include:: ../../explain_-Rgeo.rst_

.. _-T:

**-T**
    Means the azimuths of Cartesian data sets should be adjusted according to the
    signs of the scales in the x- and y-directions [Leave alone].  This option can
    be used to convert vector azimuths in cases when a negative scale is used in
    one of both directions (e.g., positive down).

.. |Add_-U| replace:: |Add_-U_links|
.. include:: ../../explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *pen*
    Set pen attributes used for wind barb outlines [Default: width =
    default, color = black, style = solid].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: ../../explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**
    The theta grid provided contains azimuths rather than directions (implies **-A**).

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-f.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-t.rst_

.. include:: ../../explain_help.rst_

.. include:: ../../explain_grd_inout.rst_


Examples
--------

.. include:: ../../explain_example.rst_

.. include:: ../../oneliner_info.rst_

To draw the wind field given by the files r.nc and theta.nc on a
barb plot with 0.1 inch length and centered on the node locations, run::

 gmt grdbarb r.nc theta.nc -Jx5c -A -Q0.1i+jc -png gradient

To plot a geographic data sets given the files comp_x.nc and comp_y.nc,
and only plot every 3rd node in either direction, try::

 gmt grdbarb comp_x.nc comp_y.nc -Ix3 -JH0/20c -Q0.1i+jc -png globe

See Also
--------

:doc:`gmt </gmt>`, :doc:`gmtcolors </gmtcolors>`,
:doc:`grdvector </grdvector>`, :doc:`barb`
