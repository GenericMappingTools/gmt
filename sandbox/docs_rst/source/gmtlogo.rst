*******
gmtlogo
*******

gmtlogo - Adding a GMT graphics logo overlay to an illustration

`Synopsis <#toc1>`_
-------------------

**gmtlogo** *dx* *dy* [ **-G**\ *fill* ] [ **-W**\ [*pen*\ ] ] >>
*plot.ps*

`Description <#toc2>`_
----------------------

This scrips appends the **GMT** logo as an overlay to an "open"
*PostScript* file. The logo is 2 inches wide and 1 inch high and will be
positioned relative to the current plot origin.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

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
**-W**\ [*pen*\ ]
    Set pen attributes for the outline of the box [Default is no
    outline].

`See Also <#toc6>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*psimage*\ (1) <psimage.html>`_
