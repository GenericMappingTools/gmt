.. index:: ! subplot

*******
subplot
*******

.. only:: not man

    Manage figure subplot configuration and selection

The subplot command is used to split the current figure into a matrix of subplots
that each may contain a single figure.  A subplot setup is started with the **begin**
mode that defines the layout of the subplots:

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt subplot begin** *nrows*\ **x**\ *ncols*
**-F**\ [**f**\ \|\ **s**\ ]\ *width*\ /*height*
[ **-A**\ *autolabel* ]
[ |SYN_OPT-B| ]
[ |-J|\ *parameters* ]
[ **-M**\ *margins* ]
[ |SYN_OPT-R| ]
[ **-S**\ *layout* ]
[ **-T**\ *title* ]
[ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

The begin mode of subplot defines the layout of the entire multi-panel illustration.

Required Arguments
------------------

*nrows*\ **x**\ *ncols*
    Specifies the number of rows and columns of subplots.  Each row will have
    the same number of subplots.  To construct figures with different number of
    subplots per row you will need to stack separate subplots.

.. _subplot_begin-F:

**-F**\ [**f**\ \|\ **s**\ ]\ *width(s)*\ /*height(s)*\ [:*wfracs*\ /*hfracs*\ ]
    Specify the dimensions of the figure.  There are two different ways to do this: (**f**) Specify overall figure dimension
    or (**s**) specify the dimension of a single panel.

**-Ff**
    Specify the final figure dimensions.  The subplot dimensions are then calculated from the figure
    dimensions after accounting for the space that optional tick marks, annotations, labels, and margins occupy between panels.
    The annotations, ticks, and labels along the outside perimeter are not counted as part of the figure dimensions.
    To specify different subplot dimensions for each row (or column), append a colon followed by a comma-separated list of width
    fractions, a slash, and then the list of height fractions.  For example **–Ff**\ 4i/4i:3,1/1,2 will make the first column
    three times as wide as the second, while the second row will be twice as tall as the first row.
    A single number means constant widths (or heights) [Default].

**-Fs**
    Specify the dimensions of each subplot directly.  Then, the figure dimensions are computed from the
    subplot dimensions after adding the space that optional tick marks, annotations, labels, and margins occupy between panels.
    The annotations, ticks, and labels along the outside perimeter are not counted as part of the figure dimensions.
    To specify different subplot dimensions for each row (or column),  append a colon followed by a comma-separated list of widths,
    a slash, and then the list of heights.  A single number means constant widths (or heights) [Default].
    For example **–Fs**\ 2i,3i/3i will make the first column 2 inches wide and the second column 3 inches wide, with
    all having a constant height of 3 inches. The number of values must either be one (constant across the rows or columns)
    or exactly match the number of rows (or columns). For geographic maps, the height of each panel depends on
    your map region and projection.  There are two options: (1) Specify both **-R** and **-J** and we use these
    to compute the height of each subplot.  All subplot must share the same region and projection and you specify
    a zero *height*, or (2) you can select *height* based on trial and error to suit your plot layout.

Optional Arguments
------------------

.. _subplot_begin-A:

**-A**\ *autolabel*
    Specify automatic tagging of each subplot.  Append either a number or letter [a].
    This sets the tag of the first, top-left subplot and others follow sequentially.
    Surround the number or letter by parentheses on any side if these should be typeset
    as part of the tag (Note: In UNIX shells you may need to escape these parentheses.)
    Use **+j**\ \|\ **J**\ *refpoint* to specify where the tag should be placed in the subplot [TL].
    Note: **+j** sets the justification of the tag to *refpoint* (suitable for interior tags)
    while **+J** instead selects the mirror opposite (suitable for exterior tags).
    Append **+c**\ *dx*\ [/*dy*] to set the clearance between the tag and a surrounding text box
    requested via **+g** or **p** [3pt/3pt, i.e., 15% of the FONT_TAG size].
    Append **+g** to paint the tag's text box with *fill* [no painting].
    Append **+o**\ *dx*\ [/*dy*] to offset the tag's reference point in the direction implied
    by the justification [4pt/4pt, i.e., 20% of the FONT_TAG size].
    Append **+p** to draw the outline of the tag's text box using selected *pen* [no outline].
    Append **+r** to typeset your tag numbers using lowercase Roman numerals;
    use **+R** for uppercase Roman numerals [Arabic numerals].
    Append **+v** to increase tag numbers vertically down columns [horizontally across rows].  

.. include:: explain_-B.rst_

.. _subplot_set-C1:

**-C**\ *side*\ /*clearance*\ [**u**\ ]
    Reserve a space of dimension *clearance* between the margin and the subplot on the specified
    side, using *side* values from **w**, **e**, **s**, or **n**.  The option is repeatable to set aside space
    on more than one side.  Such space will be left untouched by the main map plotting but can
    be accessed by modules that plot scales, bars, text, etc.  Settings specified here apply to all panels.

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. _subplot_begin-M:

**-M**\ *margins*
    This is clearance that is added around each subplot beyond the automatic space allocated for tick marks,
    annotations, and labels [0.5c].
    The margins can be a single value, a pair of values separated by slashes (for setting separate horizontal and vertical margins),
    or the full set of four margins (for setting separate left, right, bottom, and top margins).

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _subplot_begin-S:

**-S**\ *layout*
    Set subplot layout for shared axes. May be set separately for rows (**-SR**) and columns (**-SC**).
    **-SC**: Use when all subplots in a **C**\ olumn share a common *x*-range. The first (i.e., **t**\ op) and the last
    (i.e., **b**\ ottom) rows will have *x* annotations; append **t** or **b** to select only one of those two rows [both].
    Append **+l** if annotated *x*-axes should have a label [none]; optionally append the label if it is the same
    for the entire subplot.
    **-SR**: Use when all subplots in a **R**\ ow share common *y*-range. The first (i.e., **l**\ eft) and the last
    (i.e., **r**\ ight) columns will have *y*-annotations; append **l** or **r** to select only one of those two columns [both].
    Append **+l** if annotated *y*-axes will have a label [none]; optionally, append the label if it is the same
    for the entire subplot.
    Append **+p** to make all annotation axis-parallel [horizontal]; if used you may have to set **-C** to secure
    extra space for long horizontal annotations.
    Append **+t** to make space for subplot titles for each row; use **+tc** for top row titles only [no subplot titles].

.. _subplot_begin-T:

**-T**\ *heading*
    While individual subplots can have titles (see **-S** or **-B**), the entire figure may also have a
    overarching *heading* [no heading]. Font is determined by setting :ref:`FONT_HEADING <FONT_HEADING>`.

.. _subplot_begin-V:

.. include:: explain_-V.rst_

**gmt subplot set** *row,col* [ **-A**\ *fixedlabel*] [ **-C**\ *side*\ /*clearance*\ [**u**\ ] ] [ |SYN_OPT-V| ]

Before you start plotting you must first select the active subplot panel.
Note: Any **-J** option passed when plotting subplots must not give the width or scale
since the dimensions of the map are completely determined by the subplot size and your region.
Specifying map width will result in an error.  For Cartesian plots: If you want the scale
to apply *equally* to both dimensions then you must specify **-Jx** [The default **-JX** will
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
    Placement, justification, etc are all inherited from the initial **subplot begin** command.

.. _subplot_set-C2:

**-C**\ *side*\ /*clearance*\ [**u**\ ]
    Reserve a space of dimension *clearance* between the margin and the subplot on the specified
    side, using *side* values from **w**, **e**, **s**, or **n**.  The option is repeatable to set aside space
    on more than one side.  Such space will be left untouched by the main map plotting but can
    be accessed by modules that plot scales, bars, text, etc.  This setting overrides the common
    clearances set during **subplot begin**.

.. _subplot_set-V:

.. include:: explain_-V.rst_


Any number of plotting command can now take place and all output will be directed to the
selected subplot panel.  There are a few other rules that need to be followed:
(1) The subplot machinery expects the first plotting command in a new subplot window
to take care of plotting the base frame.  The particulars of this frame may have been 
specified by **subplot begin**.  In either case, should you need to set or override
frame and axis parameters then you must specify these **-B** options with this first plot
command.  (2) The subplot machinery automatically uses the **-X** and **-Y** options under
the hood so these are not available while a subplot is active.

**gmt subplot end** [ |SYN_OPT-V| ]

Exits subplot mode and returns the current plot location to where it was prior to
the subplot.

Optional Arguments
------------------

.. _subplot_end-V:

.. include:: explain_-V.rst_

***
end
***

**gmt end** [ |SYN_OPT-V| ]

This command terminates the modern mode and finalizes the processing of all registered
figures.  The final graphics will be placed in the current directory.

Examples
--------

To set up a 2x2 set of panels that all should be 2 by 2 inches in size, use

   ::

    gmt subplot to be added


See Also
--------

:doc:`begin`,
:doc:`clear`,
:doc:`end`,
:doc:`figure`,
:doc:`gmt`
