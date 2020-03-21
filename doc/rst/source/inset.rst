.. index:: ! inset
.. include:: module_core_purpose.rst_

*****
inset
*****

|inset_purpose|

The **inset** module is used to carve out a sub-region of the current plot canvas and
restrict further plotting to that section of the canvas.  The inset setup is started with the **begin**
directive that defines the placement and size of the inset.  Subsequent plot commands will be directed
to that window.  The inset is completed via the **end** directive, which reverts operations to the full
canvas and restores the plot region and map projection that was in effect prior to the setup of the inset.

Synopsis (begin mode)
---------------------

.. include:: common_SYN_OPTs.rst_

**gmt inset begin**
|-D|\ *inset-box*
[ |-F|\ *box* ]
[ |-M|\ *margins* ]
[ |-N| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

The **begin** directive of **inset** defines the dimension and placement of the inset canvas.  It
records the current region and projection so that we may return to the initial
plot environment when  the inset is completed.  The user may select any plot region
and projection once plotting in the inset, but if the first command uses
? as scale or width then we adjust the scale or width to fill the inset as best
as possible, given the inset size and margins (if selected).


Required Arguments
------------------

.. _-D:

**-D**\ *xmin/xmax/ymin/ymax*\ [**+r**][**+u**\ *unit*]] \| **-D**\ [**g**\|\ **j**\|\ **J**\|\ **n**\|\ **x**]\ *refpoint*\ **+w**\ *width*\ [/*height*][**+j**\ *justify*][**+o**\ *dx*\ [/*dy*]]
    Define the map inset rectangle on the map.  Specify the rectangle in one of three ways:

    .. include:: explain_refpoint.rst_

    Alternatively, Give *west/east/south/north* of geographic rectangle bounded by parallels
    and meridians; append **+r** if the coordinates instead are the lower left and
    upper right corners of the desired rectangle. (Or, give *xmin/xmax/ymin/ymax*
    of bounding rectangle in projected coordinates and optionally append **+u**\ *unit* [Default coordinate unit is meter (e)].
    Append **+w**\ *width*\ [/*height*] of bounding rectangle or box in plot coordinates (inches, cm, etc.).
    By default, the anchor point on the scale is assumed to be the bottom left corner (BL), but this
    can be changed by appending **+j** followed by a 2-char justification code *justify* (see :doc:`text`).
    **Note**: If **-Dj** is used then *justify* defaults to the same as *refpoint*,
    if **-DJ** is used then *justify* defaults to the mirror opposite of *refpoint*.
    Specify inset box attributes via the **-F** option [outline only].

Optional Arguments
------------------

.. _-F:

**-F**\ [**+c**\ *clearances*][**+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][**+p**\ [*pen*]][**+r**\ [*radius*]][**+s**\ [[*dx*/*dy*/][*shade*]]]
    Without further options, draws a rectangular border around the map inset using
    :term:`MAP_FRAME_PEN`; specify a different pen with **+p**\ *pen*.
    Add **+g**\ *fill* to fill the logo box [no fill].
    Append **+c**\ *clearance* where *clearance* is either *gap*, *xgap*\ /\ *ygap*,
    or *lgap*\ /\ *rgap*\ /\ *bgap*\ /\ *tgap* where these items are uniform, separate in
    x- and y-direction, or individual side spacings between logo and border.
    Append **+i** to draw a secondary, inner border as well. We use a uniform
    *gap* between borders of 2\ **p** and the :term:`MAP_DEFAULT_PEN`
    unless other values are specified. Append **+r** to draw rounded
    rectangular borders instead, with a 6\ **p** corner radius. You can
    override this radius by appending another value. Finally, append
    **+s** to draw an offset background shaded region. Here, *dx*/*dy*
    indicates the shift relative to the foreground frame
    [4\ **p**/-4\ **p**] and *shade* sets the fill style to use for shading [gray50].

.. _-M:

**-M**\ *margins*
    This is clearance that is added around the inside of the inset.  Plotting will take place
    within the inner region only. The margins can be a single value, a pair of values separated by slashes
    (for setting separate horizontal and vertical margins), or the full set of four margins (for setting
    separate left, right, bottom, and top margins) [no margins].

.. _-N:

**-N**
    Do NOT clip features extruding outside map inset boundaries [Default will clip].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help_nopar.rst_

Synopsis (end mode)
-------------------

**gmt inset end** [ |SYN_OPT-V| ]

The **end** directive finalizes the current inset, which returns the plotting environment to
the state prior to the start of the inset.  The previous region and map projection will be
in effect going forward.

Optional Arguments
------------------

.. _inset_end-V:

.. include:: explain_-V.rst_
.. include:: explain_help_nopar.rst_


Examples
--------

To make a simple basemap plot called inset.pdf that demonstrates the inset module, try

   ::

    gmt begin inset pdf
      gmt basemap -R0/40/20/60 -JM6.5i -Bafg -B+glightgreen
      gmt inset begin -DjTR+w2.5i+o0.2i -F+gpink+p0.5p -M0.25i
        gmt basemap -Rg -JA20/20/2i -Bafg
        gmt text -F+f18p+cTR+tINSET -Dj-0.15i -N
      gmt inset end
      gmt text -F+f18p+cBL+tMAP -Dj0.2i
    gmt end

See Also
--------

:doc:`begin`,
:doc:`clear`,
:doc:`docs`,
:doc:`end`,
:doc:`figure`,
:doc:`gmt`,
:doc:`subplot`
