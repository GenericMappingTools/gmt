.. index:: ! gmtlogo
.. include:: module_core_purpose.rst_

****
logo
****

|gmtlogo_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt logo** [ |-D|\ [**g**\|\ **j**\|\ **J**\|\ **n**\|\ **x**]\ *refpoint*\ [**+h**\ *height*\|\ **+w**\ *width*\ ][**+j**\ *justify*]\ [**+o**\ *dx*\ [/*dy*]] ]
[ |-F|\ [**+c**\ *clearances*][**+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][**+p**\ [*pen*]][**+r**\ [*radius*]][**+s**\ [[*dx*/*dy*/][*shade*]]] ]
[ |-J|\ *parameters* ] [ |-J|\ **z**\|\ **Z**\ *parameters* ]
[ |SYN_OPT-Rz| ]
[ |-S|\ [**l**\|\ **n**\|\ **u**] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

This module plots the GMT logo on a map. By default, the GMT logo is 144 points (2 inches wide) and 72 points (1 inch) high and will be
positioned relative to the current plot origin. Use various options to change this and to place a
transparent or opaque rectangular map panel behind the GMT logo.

.. figure:: /_images/GMT_coverlogo.*
   :width: 200 px
   :align: center

   Standard presentation of the GMT logo.


Required Arguments
------------------

None.

Optional Arguments
------------------

.. _-D:

**-D**\ [**g**\|\ **j**\|\ **J**\|\ **n**\|\ **x**]\ *refpoint*\ [**+h**\ *height*\|\ **+w**\ *width*\ ][**+j**\ *justify*]\ [**+o**\ *dx*\ [/*dy*]]

    Sets reference point on the map for the image using one of four coordinate systems:

    - **g** - Set *refpoint* in map (user) coordinates.
    - **j** - Set *refpoint* via a 2-char justification code that refers to the (invisible) map domain rectangle (see :doc:`text`).
      The logo is justified to fit on the inside of the map domain rectangle.
    - **J** - Same as **j**, but instead justifies the logo on the outside of the map domain rectangle.
    - **n** - Set *refpoint* using normalized (0-1) coordinates.
    - **x** - Set *refpoint* using plot coordinates (inches, cm, etc.).

    All but **-Dx** requires both |-R| and |-J| to be specified. Modifiers control size and adjustments:

    - **+w** - Append *width* to set the width of the GMT logo in plot coordinates (inches, cm, etc.).
    - **+h** - Append *height* to instead specify the logo height.
    - **+j** - By default, the anchor point on the GMT logo is assumed to be the bottom left corner (BL), but this
      can be changed by appending the 2-char justification code *justify* (see :doc:`text`).
    - **+o** - Offset the GMT logo by *dx*/*dy* away from the *refpoint* point in
      the direction implied by *justify* (or the direction implied by **-Dj** or **-DJ**).
   
    **Note**: (1) If **-Dj** is used then *justify* defaults to the same as *refpoint*, while
    if **-DJ** is used then *justify* defaults to the mirror opposite of *refpoint*.
    (2) Since the aspect ratio is fixed, only one of **+h** and **+w** can be specified.
 
.. _-F:

**-F**\ [**+c**\ *clearances*][**+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][**+p**\ [*pen*]][**+r**\ [*radius*]]\
[**+s**\ [[*dx*/*dy*/][*shade*]]]

    Without further options, draws a rectangular border around the GMT logo using :term:`MAP_FRAME_PEN`. The following
    modifiers can be appended to |-F|, with additional explanation and examples provided in the :ref:`Background-panel`
    cookbook section:

    .. include:: explain_-F_box.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

.. _-S:

**-S**\ [**l**\|\ **n**\|\ **u**]
    Control what is written beneath the map portion of the logo.
    Append **l** (or skip |-S| entirely) to plot the text label "The Generic Mapping Tools"
    beneath the logo. Append **n** to skip the label placement, and append
    **u** to place the URL to the GMT website instead [plot the label].

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. module_common_ends

Examples
--------

.. include:: oneliner_info.rst_

To plot the GMT logo of a 144-point width as a stand-alone pdf plot, use::

    gmt logo -pdf logo

To append a GMT logo overlay in the upper right corner of the current map, but
scaled up to be 6 cm wide and offset by 0.25 cm from the border, try::

    gmt begin map
    gmt ...<plot the map using -R -J>
    gmt logo -DjTR+o0.25c+w6c
    gmt end show

Notes
-----

To instead plot the GMT QR code that links to https://www.generic-mapping-tools.org/ just plot the
custom symbols **QR** or **QR_transparent** in :doc:`plot`.

See Also
--------

:doc:`gmt`, :doc:`legend`,
:doc:`image`, :doc:`colorbar`, :doc:`plot`
