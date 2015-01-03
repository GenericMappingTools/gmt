.. index:: ! gmtlogo

*******
gmtlogo
*******

.. only:: not man

    gmtlogo - Place the GMT graphics logo on a map

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmtlogo** [ **-D**\ [**g**\ \|\ **j**\ \|\ **n**\ \|\ **x**]\ *anchor*\ [/*justify*]\ [/*dx*/*dy*] ]
[ **-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]*pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]] ]
[ **-J**\ *parameters* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ]
[ **-O** ] [ **-P** ] [ **-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**] ]
[ **-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*] ] [ **-W**\ *width* ]
[ **-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**]] ]
[ **-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**]] ]
[ **-c**\ *copies* ]
[ **-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*] ]
[ **-t**\ [*transp*] ]
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

**-D**\ [**g**\ \|\ **j**\ \|\ **n**\ \|\ **x**]\ *anchor*\ [/*justify*]\ [/*dx*/*dy*]
    Sets *anchor* position *x0*/*y0* of the image using one of four coordinate systems:
    (1) Use **-Dg** for map (user) coordinates, (2) use **-Dj** for setting *anchor* via
    a 2-char justification code that refers to the (invisible) map domain rectangle,
    (3) use **-Dn** for normalized (0-1) coordinates, or (4) use **-Dx** for plot coordinates
    (inches, cm, etc.).  All but **-Dx** requires both **-R** and **-J** to be specified.
    By default, the *anchor* is assumed to be the lower left corner of the logo, but this
    can be changed by specifying a 2-char justification code *justify* (see :doc:`pstext`).
    Finally, you can offset the logo by *dx*/*dy* away from the *anchor* point in the
    direction implied by *justify* [LB].

**-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]]
    Without further options, draws a rectangular border around the logo using
    **MAP\_FRAME\_PEN**; specify a different pen with **+p**\ *pen*.
    Add **+g**\ *fill* to fill the logo box [no fill].
    Append **+i** to draw a secondary, inner border as well. We use a
    *gap* between borders of 2\ **p** and the **MAP\_DEFAULTS\_PEN**
    unless other values are specified. Append **+r** to draw rounded
    rectangular borders instead, with a 6\ **p** corner radius. You can
    override this radius by appending another value. Finally, append
    **+s** to draw an offset background shaded region. Here, *dx*/*dy*
    indicates the shift relative to the foreground frame
    [4\ **p**/-4\ **p**] and *shade* sets the fill style to use for shading.

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

.. |Add_-R| replace:: (Used only with **-p**)
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

.. |Add_perspective| replace:: (Requires **-R** and **-J** for proper functioning).
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_


See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`psimage`
