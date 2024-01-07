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
[ |-C|\ [*side*]\ *clearance* ]
[ |-F|\ *box* ]
[ |-N| ]
[ |SYN_OPT-R| ]
[ |-J|\ *parameters* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

The **begin** directive of **inset** defines the dimension and placement of the inset canvas.  It
records the current region and projection so that we may return to the initial
plot environment when  the inset is completed.  The user may select any plot region
and projection once plotting in the inset, but if the first command uses a projection
that leaves off the scale or width then we supply a scale or width to fill the inset as best
as possible, given the inset size and margins (if selected). **Note**: If you wish to let
the inset dimensions be determined by the region and projection that will be used to draw in
the inset, then give these arguments on the **gmt inset begin** command.


Required Arguments (begin mode)
-------------------------------

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
    Specify inset box attributes via the |-F| option [outline only].

Optional Arguments (begin mode)
-------------------------------

.. _-C:

**-C**\ [*side*]\ *clearance*
    Reserve a space of dimension *clearance* between the actual inset plot area and the given inset box on the specified
    side, using *side* values from **w**, **e**, **s**, or **n**, or **x** for both **w** and **e**
    or **y** for both **s** and **n**.  No *side* means all sides. The option is repeatable to set aside space
    on more than one side.  Alternatively, if all sides are to be set you can also give a pair of values separated by slashes
    (for setting separate horizontal and vertical margins), or the full set of four separate margins. Such space
    will be left untouched by the inset map plotting.  Append units as desired [Default is set by :term:`PROJ_LENGTH_UNIT`].

.. _-F:

**-F**\ [**+c**\ *clearances*][**+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][**+p**\ [*pen*]][**+r**\ [*radius*]]\
[**+s**\ [[*dx*/*dy*/][*shade*]]]

    Without further options, draws a rectangular border around the map inset using :term:`MAP_FRAME_PEN`. The following
    modifiers can be appended to |-F|, with additional explanation and examples provided in the :ref:`Background-panel`
    cookbook section:

    .. include:: explain_-F_box.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-N:

**-N**
    Do **not** clip features extruding outside map inset boundaries [Default will clip].

.. |Add_-R| replace:: This is useful when you want the inset **-R -J** to also determine the inset size. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: explain_help_nopar.rst_

Synopsis (end mode)
-------------------

**gmt inset end** [ |SYN_OPT-V| ]

The **end** directive finalizes the current inset, which returns the plotting environment to
the state prior to the start of the inset.  The previous region and map projection will be
in effect going forward.

Optional Arguments (end mode)
-----------------------------

.. _inset_end-V:

.. include:: explain_-V.rst_
    :start-after: .. _-V:
    :end-before: **Description**

.. include:: explain_help_nopar.rst_


Examples
--------

To make a simple basemap plot called inset.pdf that demonstrates the inset module, try

::

  gmt begin inset pdf
    gmt basemap -R0/40/20/60 -JM6.5i -Bafg -B+glightgreen
    gmt inset begin -DjTR+w2.5i+o0.2i -F+gpink+p0.5p -C0.25i
      gmt basemap -Rg -JA20/20/ -Bafg
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
