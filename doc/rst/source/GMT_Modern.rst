GMT Modern Mode
===============

.. include:: common_SYN_OPTs.rst_

Modern Mode is Experimental!

In GMT modern mode there are five new commands that are used to manage
a GMT workflow.  In modern mode many common GMT options have been made
obsolete and explicitly specifying the previous region or projection
is now implicit.  Finally, the default output format for illustrations
is now PDF, but you can change this and request more than one format
for your plots.

A GMT modern session is enabled by **gmt begin** and is terminated by
**gmt end**.  Within a session you may also use the **gmt figure* command
to control names and formats for multi-figure scripts, the **gmt subplot**
for building multi-panel illustrations, and **gmt revert** to remove
layers from the current plot.

*****
begin
*****

**gmt begin** [*prefix*] [*formats*] [ |SYN_OPT-V| ]

This command tells GMT to run in modern mode.  If your script only makes
a single plot then this is most straightforward way to specify the name
and formats of your plots.  

Optional Arguments
------------------

*prefix*
    Name-stem used to construct final figure names,  Extensions are appended
    automatically from your *formats* selection [gmtsession].

*formats*
    Give one or more comma-separated graphics extensions from the allowable graphics
    :ref:`formats <tbl-formats>` [pdf].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _tbl-formats:

    +--------+-----------------------------------------+
    | Format | Explanation                             |
    +========+=========================================+
    |  bmp   | MicroSoft BitMap.                       |
    +--------+-----------------------------------------+
    |  eps   | Encapsulated *PostScript*               |
    +--------+-----------------------------------------+
    |  jpg   | Joint Photographic Experts Group format |
    +--------+-----------------------------------------+
    |  pdf   | Portable Document Format [Default]      |
    +--------+-----------------------------------------+
    |  png   | Portable Network Graphics               |
    +--------+-----------------------------------------+
    |  ppm   | Portable Pixel Map                      |
    +--------+-----------------------------------------+
    |   ps   | Plain *PostScript*                      |
    +--------+-----------------------------------------+
    |   tif  | Tagged Image Format File                |
    +--------+-----------------------------------------+


******
figure
******

**gmt figure** *prefix* [*formats*] [*options*] [ |SYN_OPT-V| ]

The figure command tells GMT the name and format(s) to use for the current plot.
It must be issued before you start plotting to the current figure, and each
new call terminates the previous figure and changes focus to the next one.
In addition to *prefix* and *formats*, you can supply a comma-separated series of
:doc:`psconvert` options that should override the default settings provided via
:ref:`PS_CONVERT <PS_CONVERT>`. The only other available option controls verbosity.

Required Arguments
------------------

*prefix*
    Name-stem used to construct final figure names,  Extensions are appended
    automatically from your *formats* selection.

Optional Arguments
------------------

*formats*
    Give one or more comma-separated graphics extensions from the allowable graphics
    :ref:`formats <tbl-formats>` [pdf].

*options*
    Sets one or more comma-separated options arguments that
    will be passed to :doc:`psconvert` when preparing this figure [A,P].
    The valid subset of options are
    **A**\ [*args*],\ **C**\ *args*,\ **D*\ *dir*,\ **E**\ *dpi*,\ **P**\ ,\ **Q**\ *args*,\ **S**.
    See the :doc:`psconvert` documentation for details on these options.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

*******
subplot
*******

The subplot command is used to split the current figure into a matrix of sub-panels
that each may contain a single figure.  A subplot setup is started with the **begin**
mode that defines the layout of the subplots:

**gmt subplot begin** *nrows*\ **x**\ *ncols* [ **-A**\ *autolabel* ]
[ **-F**\ *width*\ /*height* ]
[ **-L**\ *layout* ] [ **-M**\ [**m**\ [\|\ **p**\ ]]\ *margins* ]
[ **-T**\ *title* ] [ |SYN_OPT-V| ]

Required Arguments
------------------

*nrows*\ **x**\ *ncols*
    Specifies the number of rows and columns of panels.  Thus each row will have
    the same number of panels.  To construct subplots with different number of
    panels per row you will need to make several subplots.

Optional Arguments
------------------

.. _-A:

**-A**\ *autolabel*
    Specify automatic tagging of each panel.  Append either a number or letter [a].
    This sets the tag of the first, top-left panel and others follow sequentially.
    Surround the number or letter by parentheses on any side if these should be typeset
    as part of the tag (Note: In UNIX shells you may need to escape these parentheses.)
    Use **+j**\ \|\ **J**\ *refpoint* to specify where the tag should be placed in the panel [TL].
    Note: **+j** sets the justification of the tag to *refpoint* while **+J** instead selects
    the mirror opposite.
    Append **+c**\ *dx*\ [/*dy*] to set the clearance between the tag and a surrounding text box
    requested via **+g** or **p**.
    Append **+g** to paint the tag's textbox with *fill* [no painting].
    Append **+o**\ *dx*\ [/*dy*] to offset the tag's reference point in the direction implied
    by the justification [4pt/4pt, i.e., 20% of the FONT_TAG size].
    Append **+p** to draw the outline of the tag's textbox using selected *pen* [no outline].
    Append **+r** to typeset your tag numbers using lowercase Roman numerals;
    use **+R** for uppercase Roman numerals [Arabic numerals].
    Append **+v** to increase tag numbers vertically down columns [horizontally across rows].  

.. _-F:

**-F**\ *width*\ /*height*
    Specify the dimension of area that the multi-panel figure may occupy [current page].

.. _-L:

**-L**\ *layout*
    Set panel layout. May be set once (**-L**) or separately for rows (**-LR**) and columns (**-LC**).
    **-L**:  Append a combination from WESNwesn to indicate which panel frames should be drawn and annotated.
    Can be used with **-LR** and **-LC** to indicate which axes should be present in addition to those
    that will be selected automatically.
    Append **+l** to make space for axes labels on all panels [no labels].
    Append **x** to only use *x*-labels or **y** for only *y*-labels [both axis will be labeled].
    **-LC**: Each panel **C**\ olumn shares a common *x*-range. Only the first (i.e., **t**\ op) and the last
    (i.e., **b**\ ottom) rows will have *x* annotations.  Append **t** or **b** to select only one of those two rows [both].
    Append **+l** if annotated *x*-axes should have a label [none]; optionally append the label if it is the same
    for the entire subplot.
    **-LR**: Each panel **R**\ ow shares common *y*-range. Only the first (i.e., **l**\ eft) and the last
    (i.e., **r**\ ight) columns will have *y*-annotations.  Append **l** or **r** to select only one of those two columns [both].
    Append **+l** if annotated *y*-axes will have a label [none]; optionally append the label if it is the same
    for the entire subplot.
    Append **+p** to make all annotation axis-parallel [horizontal]. If used you may have to set **-C** to make
    extra space for long horizontal annotations.
    Append **+t** to make space for panel titles; use **+tc** for top row titles only [no panel titles].

.. _-M:

**-M**\ [**m**\ [\|\ **p**\ ]]\ *margins*
    There are two types of margins that you can adjust.  For plots destined to be printed you may want to
    specify *media* margins because the printer cannot print all the way to the edges.  Use **-Mm** to
    specify such margins [1c].  The other margin type is for each *panel*.  This is clearance that is added
    around each panel when the plot dimensions of each panel are calculated [0.5c].  For either type, the margins
    can be a single value, a pair of values separated by slashes (setting the horizontal and vertical margins),
    or the full set of four margins for the left, right, bottom, and top margin.

.. _-T:

**-T**\ *heading*
    While individual panels can have titles (see **-L**), the entire collection of panels may also have a
    overarching *heading*.  If selected then the dimensions of panels are adjusted accordingly to allow for
    panels, annotations, labels, and heading to fit within the dimensions allotted by **-F** [no heading].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_


**gmt subplot set** *row,col* [ **-A**\ *fixedlabel*] [ **-C**\ *side*\ /*clearance*\ [**u**\ ] ] [ |SYN_OPT-V| ]

Before you start plotting into a panel you must first select the active panel.
Note: Any **-J** option passed when plotting subplots must not give the final width or scale
since the dimensions of the map is completely determined by the panel size and your region.
Specifying map width will result in an error.

Required Arguments
------------------

*row,col*
    Sets the current plot panel until further notice.  As an alternative, you may
    instead append **-c**\ *row,col* to the first plot command you issue in the panel.
    GMT maintains information about the current subplot and panel.

Optional Arguments
------------------

.. _-A:

**-A**\ *fixedlabel*
    Overrides the automatic labeling with the given string.

.. _-C:

**-C**\ *side*\ /*clearance*\ [**u**\ ]
    Reserve a space of dimension *clearance* between the margin and the plot panel on the specified
    side, using *side* values from w, e, s, or n.  The option is repeatable to set aside space
    on more than one side.  Such space will be left untouched by the main map plotting but can
    be accessed by modules plotting scales, bars, text, etc.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_


**gmt subplot end** [ |SYN_OPT-V| ]

Exits subplot mode and returns the current plot location to where it was prior to
the subplot.

Optional Arguments
------------------

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

******
revert
******

**gmt revert** [ *n*\ \|\ **all** ] [ |SYN_OPT-V| ]

This command strips off the last *n* layers of the current figure.
Giving **all* wipes the figure completely; however, it still remains
the current figure.

Optional Arguments
------------------

*n*
    Specifies the number of plot layers to revert [1].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

***
end
***

**gmt end** [ |SYN_OPT-V| ]

This command terminates the modern mode and completes the processing of all registered
figures.  The graphics will be placed in the current directory.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`
