.. index:: ! subplot
.. include:: module_core_purpose.rst_

*******
subplot
*******

|subplot_purpose|

The **subplot** module is used to split the current figure into a rectangular layout of subplots
that each may contain a single self-contained figure.  A subplot setup is started with the **begin**
directive that defines the layout of the subplots, while positioning to a particular subplot for
plotting is done via the **set** directive.  The subplot process is completed via the **end** directive.

Synopsis (begin mode)
---------------------

.. include:: common_SYN_OPTs.rst_

**gmt subplot begin** *nrows*\ **x**\ *ncols*
|-F|\ [**f**\|\ **s**]\ *width*\ /*height*\ [**+f**\ *wfracs*\ /*hfracs*][**+af**\|\ **s**][**+c**\ *dx/dy*][**+g**\ *fill*][**+p**\ *pen*][**+w**\ *pen*]
[ |-A|\ *autotag* ]
[ |-C|\ [*side*]\ *clearance* ]
[ |-D| ]
[ |SYN_OPT-B| ]
[ |-J|\ *parameters* ]
[ |-M|\ *margins* ]
[ |SYN_OPT-R| ]
[ |-S|\ **c**\|\ **r**\ [*layout*][*mods*] ]
[ |-T|\ *title* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

The **begin** directive of subplot defines the layout of the entire multi-panel illustration.  Several
options are available to specify the systematic layout, labeling, dimensions, and more for the subplots.

Required Arguments (begin mode)
--------------------------------

*nrows*\ **x**\ *ncols*
    Specifies the number of rows and columns of subplots.  Each row will have
    the same number of subplots.
    **Note**: You are not required to place a plot in each subplot.

.. _-F:

**-F**\ [**f**\|\ **s**]\ *width(s)*\ /*height(s)*\ \ [**+f**\ *wfracs*\ /*hfracs*][**+af**\|\ **s**][**+c**\ *dx/dy*][**+g**\ *fill*][**+p**\ *pen*][**+w**\ *pen*]
    Specify the dimensions of the figure.  There are two different ways to do this:
    (**f**) Specify overall figure dimensions or (**s**) specify the dimensions of
    a single subplot.

**-Ff**
    Specify the final **f**\ igure dimensions.  The subplot dimensions are then calculated from the figure
    dimensions after accounting for the space that optional tick marks, annotations, labels, and margins occupy between subplots.
    As for other figures, annotations, ticks, and labels along the outside perimeter are not counted as part of the figure dimensions.
    To specify different subplot dimensions for each row (or column), append **+f** followed by a comma-separated list of width
    fractions, a slash, and then the list of height fractions.  For example **-Ff**\ 10c/10c\ **+f**\ 3,1/1,2 will make the first column
    three times as wide as the second, while the second row will be twice as tall as the first row.
    A single number means constant widths (or heights) [Default].

**-Fs**
    Specify the dimensions of each **s**\ ubplot directly.  Then, the figure dimensions are computed from the
    subplot dimensions after adding the space that optional tick marks, annotations, labels, and margins occupy between subplots.
    As for other figures, annotations, ticks, and labels along the outside perimeter are not counted as part of the figure dimensions.
    To specify different subplot dimensions for each row (or column), append a comma-separated list of widths,
    a slash, and then the comma-separated list of heights.  A single number means constant widths (or heights) [Default].
    For example **-Fs**\ 5c,8c/8c will make the first column 5 cm wide and the second column 8 cm wide, with
    all having a constant height of 8 cm. The number of values must either be one (constant across the rows or columns)
    or exactly match the number of rows (or columns). For geographic maps, the height of each subplot depends on
    your map region and projection.  There are two options: (1) Specify both **-R** and **-J** and we use these
    to compute the height of each subplot.  All subplots must share the same region and projection and you specify
    a zero *height*, or (2) you can select *height* based on trial and error to suit your plot layout.

    Optionally, you may draw the outline (**+p**\ *pen*) or paint (**+g**\ *fill*\) the figure rectangle behind the
    subplots, add dividing lines between panels (**+w**\ *pen*), and even expand it via **+c**.  These are most
    useful if you supply **-B+n** to **subplot begin**, meaning no ticks or annotations will take place in the subplots.
    By default (**+af**), we auto-scale the fonts and pens based on the geometric mean dimension of the entire figure.
    Append **+as** to instead determine the scaling from the geometric mean subplot dimension.

Optional Arguments (begin mode)
-------------------------------

.. _-A:

**-A**\ [*autotag*][**+c**\ *dx*\ [/*dy*]][**+g**\ *fill*][**+j**\|\ **J**\ *refpoint*][**+o**\ *dx*\ [/*dy*]][**+p**\ *pen*][**+r**\|\ **R**][**+s**\ [[*dx*/*dy*][/*shade*]]][**+v**]
    Specify automatic tagging of each subplot.  Append either a number or letter [a].
    This sets the tag of the first, top-left subplot and others follow sequentially.
    Surround the number or letter by parentheses on any side if these should be typeset
    as part of the tag (**Note**: In UNIX shells you may need to escape these parentheses.)
    Use **+j**\|\ **J**\ *refpoint* to specify where the tag should be placed in the subplot [TL].
    **Note**: **+j** sets the justification of the tag to *refpoint* (suitable for interior tags)
    while **+J** instead selects the mirror opposite (suitable for exterior tags).
    Append **+c**\ *dx*\ [/*dy*] to set the clearance between the tag and a surrounding text box
    requested via **+g** or **+p** [3p/3p, i.e., 15% of the :term:`FONT_TAG` size dimension].
    Append **+g**\ *fill* to paint the tag's text box with *fill* [no painting].
    Append **+o**\ *dx*\ [/*dy*] to offset the tag's reference point in the direction implied
    by the justification [4p/4p, i.e., 20% of the :term:`FONT_TAG` size].
    Append **+p**\ *pen* to draw the outline of the tag's text box using selected *pen* [no outline].
    Append **+r** to typeset your tag numbers using lowercase Roman numerals;
    use **+R** for uppercase Roman numerals [Arabic numerals].
    Append **+s** to draw an offset background shaded rectangle. Here, *dx*/*dy* indicates the
    shift relative to the tag box [default is **2p**\ /**-2p**] and *shade* sets the
    fill style to use for shading [default is **gray50**].
    Append **+v** to increase tag numbers vertically down columns [horizontally across rows].

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ [*side*]\ *clearance*
    Reserve a space of dimension *clearance* between the margin and the subplot on the specified
    side, using *side* values from **w**, **e**, **s**, or **n**, or **x** for both **w** and **e**
    or **y** for both **s** and **n**.  No *side* means all sides. The option is repeatable to set aside space
    on more than one side.  Such space will be left untouched by the main map plotting but can
    be accessed by modules that plot scales, bars, text, etc.  Settings specified under **begin** directive apply
    to all subplots, while settings under **set** only apply to the selected (active) subplot.  **Note**: Common options **-X**
    and **-Y** are not available during subplots; use **-C** instead.

.. _-D:

**-D**
    Use the prevailing defaults settings (via gmt.conf or **--PAR**\ =\ *value*) and the selections made
    via **-B**, **-C**, **-M** and **-S** to determine the panel sizes (if using **-Ff**) and panel spacings only, but
    do *not* draw and annotate any frames.  This option is useful if you wish to lay down a partial subplot
    with annotations and frames, but then want to plot data inside it separately later without redrawing
    the frames.  With different **-B**, **-C**, **-M** and **-S** choices the two subplots may not align, but with
    **-D** they will.  **Note**: It is assumed that **-F** stays the same [Draw and annotate frames as indicated].

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-M:

**-M**\ *margins*
    This is margin space that is added *between* neighboring subplots (i.e., the interior margins) *in addition*
    to the automatic space added for tick marks, annotations, and labels.  The margins can be specified as
    a single value (for same margin on all sides), a pair of values separated by slashes
    (for setting separate horizontal and vertical margins), or the full set of four slash-separated margins
    (for setting separate left, right, bottom, and top margins).  The actual gap created is always a sum of
    the margins for the two opposing sides (e.g., east plus west or south plus north margins) [Default is
    half the primary annotation font size, giving the full annotation font size as the default gap].

.. |Add_-R| replace:: This is useful when all subplots share a common plot domain. In this module, the chosen region
    will also become the default for any data region needed by computational modules. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-S:

**-S**\ **c**\|\ **r**\ [*layout*][*mods*]
    Set subplot *layout* for shared axes. May be set separately for rows (**-Sr**) and columns (**-Sc**).
    Considerations for **-Sc**: Use when all subplots in a **c**\ olumn share a common *x*-range. The first (i.e., **t**\ op) and the last
    (i.e., **b**\ ottom) rows will have *x* annotations; append **t** or **b** to select only one of those two rows [both].
    Append **+l** if annotated *x*-axes should have a label [none]; optionally append the label if it is the same
    for the entire subplot. Optionally, use **+s** to set a separate (secondary) label.
    Append **+t** to make space for subplot titles for each row; use **+tc** for top row titles only [no subplot titles].
    Labels and titles that depends on which row or column are specified as usual via a subplot's own **-B** setting.
    Considerations for **-Sr**: Use when all subplots in a **r**\ ow share a common *y*-range. The first (i.e., **l**\ eft) and the last
    (i.e., **r**\ ight) columns will have *y*-annotations; append **l** or **r** to select only one of those two columns [both].
    Append **+l** if annotated *y*-axes will have a label [none]; optionally, append the label if it is the same
    for the entire subplot. Optionally, use **+s** to set a separate (secondary) label.
    Append **+p** to make all annotations axis-parallel [horizontal]; if not used you may have to set **-C** to secure
    extra space for long horizontal annotations.
    Append **+w** to the **-F** argument to draw horizontal and vertical lines
    between interior panels using selected pen [no lines].

.. _-T:

**-T**\ *heading*
    While individual subplots can have titles (see **-S** or **-B**), the entire figure may also have a
    overarching *heading* [no heading]. Font is determined by setting :term:`FONT_HEADING`.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: explain_help.rst_

Synopsis (set mode)
-------------------

**gmt subplot set** [ *row,col*\|\ *index* ] [ **-A**\ *fixedtag*] [ **-C**\ *side*\ *clearance* ] [ |SYN_OPT-V| ]

Before you start plotting you must first select the active subplot.
**Note**: If any **-J** option is passed *without* the scale or width when you first are plotting
inside the subplot, then the scale of the map is automatically determined by the subplot size and your region.
For Cartesian plots: If you want the scale to apply *equally* to both dimensions
then you must specify **-Jx** [The default projection of **-JX** will fill the subplot by using unequal scales].

Optional Arguments (set mode)
-----------------------------

*row,col*
    Sets the current subplot until further notice.  **Note**: First *row* or *col* is 0, not 1. If not given we go to the next subplot by order
    specified via **-A**.  As an alternative, you may bypass the **set** mode and
    instead supply the common option **-c**\ [*row,col*] to the first plot command you issue in that subplot.
    GMT maintains information about the current figure and subplot. Also, you may give the one-dimensional
    *index* instead which starts at 0 and follows the row or column order set via **-A**.

.. _subplot_set-A:

**-A**\ *fixedtag*
    Overrides the automatic labeling with the given string.  No modifiers are allowed.
    Placement, justification, etc. are all inherited from how **-A** was specified by the
    initial **subplot begin** command.  **Note**: Overriding means you initiate the tag
    machinery with **-A** when **subplot begin** was called, otherwise the option is ignored.

.. _subplot_set-C2:

**-C**\ [*side*]\ *clearance*
    Reserve a space of dimension *clearance* between the margin and the subplot on the specified
    side, using *side* values from **w**, **e**, **s**, or **n**, or **x** for both **w** and **e**
    or **y** for both **s** and **n**.  The option is repeatable to set aside space
    on more than one side.  Such space will be left untouched by the main map plotting but can
    be accessed by modules that plot scales, bars, text, etc.  This setting overrides the common
    clearances set by **-C** during **subplot begin**.

.. _subplot_set-V:

.. include:: explain_-V.rst_
    :start-after: .. _-V:
    :end-before: **Description**

Any number of plotting command can now take place and all output will be directed to the
selected subplot.  There are a few other rules that need to be followed:
(1) The subplot machinery expects the first plotting command in a new subplot window
to take care of plotting the base frame.  The particulars of this frame may have been
specified by the **-S** option in **subplot begin**.  In either case, should you need to set or override
frame and axis parameters then you must specify these **-B** options with this first plot
command.  (2) The subplot machinery automatically uses the **-X** and **-Y** options under
the hood so these options are not available while a subplot is active.

Synopsis (end mode)
-------------------

**gmt subplot end** [ |SYN_OPT-V| ]

This command finalizes the current subplot, including any placement of tags, and updates the
gmt.history to reflect the dimensions and linear projection required to draw the entire figure
outline. This allows subsequent commands, such as colorbar, to use **-DJ** to place bars with
reference to the complete figure dimensions. We also reset
the current plot location to where it was prior to the subplot.

Optional Arguments (end mode)
-----------------------------

.. _subplot_end-V:

.. include:: explain_-V.rst_
    :start-after: .. _-V:
    :end-before: **Description**

Examples
--------

To make a minimalistic 2x2 basemap layout called panels.pdf, try::

    gmt begin panels pdf
      gmt subplot begin 2x2 -Fs8c -M5p -A -Scb -Srl -R0/80/0/10
        gmt subplot set
        gmt basemap
        gmt subplot set
        gmt basemap
        gmt subplot set
        gmt basemap
        gmt subplot set
        gmt basemap
      gmt subplot end
    gmt end show


Restrictions
------------

#. Currently, nesting of subplots is not implemented. 
#. If auto-legend option **-l** is used then you must complete plotting in one panel 
   before moving to another.
#. Specifying separate primary and secondary annotations via **-Bp** and **-Bs** have
   not yet been implemented.

See Also
--------

:doc:`begin`,
:doc:`clear`,
:doc:`docs`,
:doc:`end`,
:doc:`figure`,
:doc:`inset`,
:doc:`gmt`
