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
**gmt end**.  Within a session you may also use the **gmt figure** command
to control names and formats for multi-figure scripts, the **gmt subplot**
for building multi-panel figures, and **gmt revert** to remove
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

.. _begin-V:

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
    **A**\ [*args*],\ **C**\ *args*,\ **D**\ *dir*,\ **E**\ *dpi*,\ **P**\ ,\ **Q**\ *args*,\ **S**.
    See the :doc:`psconvert` documentation for details on these options.

.. _figure-V:

.. include:: explain_-V.rst_

*******
subplot
*******

The subplot command is used to split the current figure into a matrix of subplots
that each may contain a single figure.  A subplot setup is started with the **begin**
mode that defines the layout of the subplots:

**gmt subplot begin** *nrows*\ **x**\ *ncols*
**-F**\ [**f**\ \|\ **s**\ ]\ *width(s)*\ /*height(s)*\ [:*wfracs*\ /*hfracs*\ ][**+f**\ *fill*\ ][**+p**\ *pen*\ ][**+d**\ ]
[ **-A**\ *autolabel* ]
[ |SYN_OPT-B| ]
|-J|\ *parameters*
[ **-M**\ *margins* ]
|SYN_OPT-R|
[ **-S**\ *layout* ]
[ **-T**\ *title* ] [ |SYN_OPT-V| ]

Required Arguments
------------------

*nrows*\ **x**\ *ncols*
    Specifies the number of rows and columns of subplots.  Each row will have
    the same number of subplots.  To construct figures with different number of
    subplots per row you will need to stack separate subplots.

.. _subplot_begin-F:

**-F**\ [**f**\ \|\ **s**\ ]\ *width(s)*\ /*height(s)*\ [:*wfracs*\ /*hfracs*\ ][**+f**\ *fill*\ ][**+p**\ *pen*\ ][**+d**\ ]
    Specify the dimensions of the figure.  The subplot dimensions are then calculated from the figure
    dimensions after accounting for tick marks, annotations, labels, and margins.  If the subplot dimensions should be different
    for each row (or column) you must append a comma-separated list of relative widths (or heights)
    following the colon.  A single number means constant widths (or heights) [Default].
    Alternatively, use **-Fs** to specify
    the dimensions of each subplot directly.  Then, the figure dimensions are computed instead.
    To specify different subplot dimensions for each row (or column), give a comma-separated
    list of dimensions instead.  The number of values must either be one (constant across the rows or columns)
    or exactly match the number of rows (or columns). For geographic maps the height of each panel depends on
    your map region and projection.  There are two options: (1) Specify both **-R** and **-J** and we use these
    to compute the height of each subplot.  All subplot must share the same region and projection, or (2) you
    can select *height* based on trial and error to suit your plot layout.

Optional Arguments
------------------

.. _subplot_begin-A:

**-A**\ *autolabel*
    Specify automatic tagging of each subplot.  Append either a number or letter [a].
    This sets the tag of the first, top-left subplot and others follow sequentially.
    Surround the number or letter by parentheses on any side if these should be typeset
    as part of the tag (Note: In UNIX shells you may need to escape these parentheses.)
    Use **+j**\ \|\ **J**\ *refpoint* to specify where the tag should be placed in the subplot [TL].
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

.. include:: explain_-B.rst_

.. _subplot_set-C:

**-C**\ *side*\ /*clearance*\ [**u**\ ]
    Reserve a space of dimension *clearance* between the margin and the subplot on the specified
    side, using *side* values from w, e, s, or n.  The option is repeatable to set aside space
    on more than one side.  Such space will be left untouched by the main map plotting but can
    be accessed by modules plotting scales, bars, text, etc.  Settings apply to all panels.

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. _subplot_begin-M:

**-M**\ *margins*
    This is clearance that is added around each subplot beyond the automatic space allocated for tick marks,
    annotations, and labels [0.5c].
    The margins can be a single value, a pair of values separated by slashes (setting the horizontal and vertical margins),
    or the full set of four margins for the left, right, bottom, and top margin.

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _subplot_begin-S:

**-S**\ *layout*
    Set subplot layout for shared axes. May be set separately for rows (**-SR**) and columns (**-SC**).
    **-SC**: Each subplot **C**\ olumn shares a common *x*-range. The first (i.e., **t**\ op) and the last
    (i.e., **b**\ ottom) rows will have *x* annotations; append **t** or **b** to select only one of those two rows [both].
    Append **+l** if annotated *x*-axes should have a label [none]; optionally append the label if it is the same
    for the entire subplot.
    **-SR**: Each subplot **R**\ ow shares common *y*-range. The first (i.e., **l**\ eft) and the last
    (i.e., **r**\ ight) columns will have *y*-annotations; append **l** or **r** to select only one of those two columns [both].
    Append **+l** if annotated *y*-axes will have a label [none]; optionally, append the label if it is the same
    for the entire subplot.
    Append **+g** to add grid-lines to each subplot [off].
    Append **+p** to make all annotation axis-parallel [horizontal]; if used you may have to set **-C** to set aside
    extra space for long horizontal annotations.
    Append **+t** to make space for subplot titles; use **+tc** for top row titles only [no subplot titles].

.. _subplot_begin-T:

**-T**\ *heading*
    While individual subplots can have titles (see **-S** or **-B**), the entire figure may also have a
    overarching *heading* [no heading]. Font is determined by setting :ref:`FONT_HEADING <FONT_HEADING>`.

.. _subplot_begin-V:

.. include:: explain_-V.rst_


**gmt subplot set** *row,col* [ **-A**\ *fixedlabel*] [ **-C**\ *side*\ /*clearance*\ [**u**\ ] ] [ |SYN_OPT-V| ]

Before you start plotting you must first select the active subplot.
Note: Any **-J** option passed when plotting subplots must not give the width or scale
since the dimensions of the map is completely determined by the subplot size and your region.
Specifying map width will result in an error.  For Cartesian plots: If you want the scale
to apply equally to both dimensions then you must specify **-Jx** [The default **-JX** will
fill the subplot using unequal scales].

Required Arguments
------------------

*row,col*
    Sets the current subplot until further notice.  As an alternative, you may
    instead supply **-c**\ *row,col* to the first plot command you issue in the subplot.
    GMT maintains information about the current figure and subplot.

Optional Arguments
------------------

.. _subplot_set-A:

**-A**\ *fixedlabel*
    Overrides the automatic labeling with the given string.  No modifiers are allowed.

.. _subplot_set-C:

**-C**\ *side*\ /*clearance*\ [**u**\ ]
    Reserve a space of dimension *clearance* between the margin and the subplot on the specified
    side, using *side* values from w, e, s, or n.  The option is repeatable to set aside space
    on more than one side.  Such space will be left untouched by the main map plotting but can
    be accessed by modules plotting scales, bars, text, etc.

.. _subplot_set-V:

.. include:: explain_-V.rst_


**gmt subplot end** [ |SYN_OPT-V| ]

Exits subplot mode and returns the current plot location to where it was prior to
the subplot.

Optional Arguments
------------------

.. _subplot_end-V:

.. include:: explain_-V.rst_

******
revert
******

**gmt revert** [ *n*\ \|\ **all** ] [ |SYN_OPT-V| ]

This command strips off the last *n* layers of the current figure.
Giving *all* wipes the figure completely; however, it still remains
the current figure.

Optional Arguments
------------------

*n*
    Specifies the number of plot layers to revert [1].

.. _revert_-V:

.. include:: explain_-V.rst_

***
end
***

**gmt end** [ |SYN_OPT-V| ]

This command terminates the modern mode and completes the processing of all registered
figures.  The graphics will be placed in the current directory.

*************
Misc. Changes
*************

The two positional options **-X** and **-Y** can now access the previous plot dimensions
*w* and *h* and construct offsets that involves them.  For instance, to move the origin
up 2 cm beyond the height of the previous plot, use **-Yh**\ +2c.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`
