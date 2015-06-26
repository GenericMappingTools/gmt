.. index:: ! gmtlogo

*******
gmtlogo
*******

.. only:: not man

    gmtlogo - Place the GMT graphics logo on a map

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmtlogo** [ **-D**\ [**g**\ \|\ **j**\ \|\ **n**\ \|\ **x**]\ *refpoint*\ [**+j**\ *justify*]\ [**+o**\ *dx*\ [/*dy*]] ]
[ **-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]*pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]] ]
[ **-J**\ *parameters* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ]
[ **-O** ] [ **-P** ]
[ |SYN_OPT-Rz| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ **-W**\ *width* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-t| ]
|No-spaces|

Description
-----------

This module plots the GMT logo on a map. By default, the logo is 2 inches wide and 1 inch high and will be
positioned relative to the current plot origin. Use various options to change this and to place a
transparent or opaque rectangular map panel behind the logo.

Required Arguments
------------------

None.

Optional Arguments
------------------

**-D**\ [**g**\ \|\ **j**\ \|\ **n**\ \|\ **x**]\ *refpoint*\ [**+j**\ *justify*]\ [**+o**\ *dx*\ [/*dy*]]
    Sets reference point on the map for the image using one of four coordinate systems:
    (1) Use **-Dg** for map (user) coordinates, (2) use **-Dj** for setting *refpoint* via
    a 2-char justification code that refers to the (invisible) map domain rectangle,
    (3) use **-Dn** for normalized (0-1) coordinates, or (4) use **-Dx** for plot coordinates
    (inches, cm, etc.).  All but **-Dx** requires both **-R** and **-J** to be specified.
    By default, the *anchor* point on the logo is assumed to be the lower left corner, but this
    can be changed by specifying a 2-char justification code *justify* (see :doc:`pstext`).
    Finally, you can offset the logo by *dx*/*dy* away from the *refpoint* point in the
    direction implied by *justify* [LB].

**-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]]
    Without further options, draws a rectangular border around the logo using
    **MAP\_FRAME\_PEN**; specify a different pen with **+p**\ *pen*.
    Add **+g**\ *fill* to fill the logo box [no fill].
    Append **+c**\ *clearance* where *clearance* is either *gap*, *xgap*\ /\ *ygap*,
    or *lgap*\ /\ *rgap*\ /\ *bgap*\ /\ *tgap* where these items are uniform, separate in
    x- and y-direction, or individual side spacings between logo and border. 
    Append **+i** to draw a secondary, inner border as well. We use a uniform
    *gap* between borders of 2\ **p** and the **MAP\_DEFAULTS\_PEN**
    unless other values are specified. Append **+r** to draw rounded
    rectangular borders instead, with a 6\ **p** corner radius. You can
    override this radius by appending another value. Finally, append
    **+s** to draw an offset background shaded region. Here, *dx*/*dy*
    indicates the shift relative to the foreground frame
    [4\ **p**/-4\ **p**] and *shade* sets the fill style to use for shading.

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-W**\ *width*
    Sets the width of the logo in plot coordinates
    (inches, cm, etc.). Default width is 2 inches.

.. include:: explain_-XY.rst_

.. include:: explain_-c.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

Examples
--------

To plot the GMT logo at the default size as a stand-alone plot, use

   ::

    gmt logo -P > logo.ps

To apped a GMT logo overlay in the upper right corner of the current map, but
scaled up to be 3 inches wide and offset by 0.1 inches from the border, try

   ::

    gmt logo -O -K -W3i -R -J -DjTR/0.1i/0.1i >> bigmap.ps


See Also
--------

:doc:`gmt`, :doc:`pslegend`,
:doc:`psimage`, :doc:`psscale`
