.. index:: ! grdbarb

*********
grdbarb
*********

.. only:: not man

|grdbarb_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**grdbarb** *compx.nc* *compy.nc* **-J**\ *parameters* [ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-I|\ [**x**]\ *dx*\ [/*dy*] ]
[ |-K| ] [ |-N| ] [ |-O| ] [ |-P| ] [ |-Q|\ *parameters* ]
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
    Contains the x-components of the wind field.
*compy.nc*
    Contains the y-components of the wind field. (See GRID FILE FORMATS below.)

.. _-J:

.. |Add_-J| replace:: |Add_-J_links|
.. include:: /explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Optional Arguments
------------------

.. _-A:

**-A**
    The grid files contain (speed, theta) wind components instead of
    (u, v) components [Default is (u, v)].

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

.. _-K:

.. include:: ../../explain_-K.rst_

.. _-N:

**-N**
    Do NOT clip wind barbs at map boundaries [Default will clip]. 

.. _-O:

.. include:: ../../explain_-O.rst_

.. _-P:

.. include:: ../../explain_-P.rst_

.. _-Q:

**-Q**\ *parameters*
    Modify wind barb parameters. Append wind barb *length* [Default is 0.2i].
    See WIND BARB ATTRIBUTES for specifying additional attributes.

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

.. include:: ../../explain_grd_inout_short.rst_

.. include:: explain_windbarbs.rst_


Examples
--------

To draw the wind field given by the files r.nc and theta.nc on a
barb plot with 0.1 inch length and centered on the node locations, run::

 gmt grdbarb r.nc theta.nc -Jx5c -A -Q0.1i+jc > gradient.ps

To plot a geographic data sets given the files comp_x.nc and comp_y.nc,
and only plot every 3rd node in either direction, try::

 gmt grdbarb comp_x.nc comp_y.nc -Ix3 -JH0/20c -Q0.1i+jc > globe.ps

See Also
--------

:doc:`gmt </gmt>`, :doc:`gmtcolors </gmtcolors>`,
:doc:`grdvector </grdvector>`, :doc:`psbarb`
