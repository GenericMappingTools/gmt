*******
gmtlogo
*******

gmtlogo - Adding a GMT graphics logo overlay to an illustration

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**gmtlogo** *dx* *dy* [ **-G**\ *fill* ] [ **-W**\ [*pen*\ ] ] >> *plot.ps*

`Description <#toc2>`_
----------------------

This scrips appends the **GMT** logo as an overlay to an "open"
*PostScript* file. The logo is 2 inches wide and 1 inch high and will be
positioned relative to the current plot origin. 

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

*dx*,\ *dy*
    Sets the lower-left corner of the logo relative to current plot
    origin.

`Optional Arguments <#toc5>`_
-----------------------------

**-G**\ *fill*
    Select color or pattern for filling the underlying box [Default is
    no fill].
**-W**\ [*pen*]
    Set pen attributes for the outline of the box [Default is no
    outline].

`See Also <#toc6>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*psimage*\ (1) <psimage.html>`_
