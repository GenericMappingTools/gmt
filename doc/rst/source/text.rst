.. index:: ! text
.. include:: module_core_purpose.rst_

******
text
******

|text_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt text** [ *textfiles* ]
|-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ [*dx/dy*][**+to**\|\ **O**\|\ **c**\|\ **C**] ]
[ |-D|\ [**j**\|\ **J**]\ *dx*\ [/*dy*][**+v**\ [*pen*]] ]
[ |-F|\ [**+a**\ [*angle*]][**+c**\ [*justify*]][**+f**\ [*font*]][**+j**\ [*justify*]][**+h**\|\ **l**\|\ **r**\ [*first*] \|\ **t**\ *text*\|\ **z**\ [*format*]] ]
[ |-G|\ [*fill*][**+n**] ]
[ |-L| ] [ |-M| ] [ |-N| ]
[ |-Q|\ **l**\|\ **u** ]
[ |-S|\ [*dx*/*dy*/][*shade*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z| ]
[ **-a**\ *col*\ =\ *name*\ [...] ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ **-it**\ *word* ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-tv| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

**text** plots text strings of variable size, font type, and
orientation. Various map projections are provided, with the option to
draw and annotate the map boundaries.

Greek characters, subscript, superscript, and small
caps are supported as follows:

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - Symbol
     - Behavior
   * - @~
     - Toggles between the selected font and Greek (Symbol)
   * - @%\ *font*\ %
     - Switches to *font* where *font* can either be a font *number* or *name* (see |-L|) (@%% resets it)
   * - @-
     - Toggles subscripts on/off
   * - @+
     - Toggles superscript on/off
   * - @#
     - Toggles small caps on/off
   * - @;\ *color*\ ;
     - Changes the font color (@;; resets it)
   * - @:\ *size*\ :
     - Changes the font size (@:: resets it)
   * - @\_
     - Toggles underline on/off
   * - @@
     - Prints the @ sign
   * - @.
     - Prints the degree symbol

@a, @c, @e, @i, @n, @o, @s, @u, @A, @C, @E, @N, @O, and @U give various accented European
characters, as indicated in Table :ref:`escape <tbl-shorthand>`. Composite characters
(overstrike) may be indicated with the @!<char1><char2> sequence, which will print the
two characters on top of each other.

To learn the octal codes for symbols not available on the
keyboard and some accented European characters, see Section :ref:`Char-esc-seq` and
Appendix :ref:`Chart-Octal-Codes-for-Chars` in the GMT Technical Reference and Cookbook. Note that
:term:`PS_CHAR_ENCODING` must be set to an extended character set in your
:doc:`gmt.conf` file in order to use the accented characters.

Using the |-G| or |-W| options, a rectangle underlying the text may be plotted
(does not work for strings with sub/super scripts, symbols, or composite
characters, except in paragraph mode (|-M|)).

Finally, you may typeset LaTeX expressions provided they are enclosed in @[ ... @[ or <math> ... </math>;
see Chapter :doc:`/cookbook/gmt-latex` for more details.

Required Arguments
------------------

*textfiles*
    This is one or more files containing 1 or more records with (*x*
    *y* [*font* *angle* *justify*] *text*). The presence or absence of
    items in the brackets are determined by |-F|. If no files
    are given, **text** will read standard input. *font* is a font
    specification with format [*size*,][*font*,][*color*] where
    *size* is text size in points, *font* is the font to use, and
    *color* sets the font color. To draw outline fonts you append
    =\ *pen* to the font specification. The *angle* is measured in degrees
    counter-clockwise from horizontal, and *justify* sets the alignment.
    If *font* is not an integer, then it is taken to be a text string
    with the desired font name (see |-L| for available fonts). The
    alignment refers to the part of the text string that will be mapped
    onto the (*x*,\ *y*) point. Choose a 2 character combination of **L**,
    **C**, **R** (for left, center, or right) and **T**, **M**, **B**
    for top, middle, or bottom. e.g., **BL** for bottom left.

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

Optional Arguments
------------------

.. _-A:

**-A**
    Angles are given as azimuths; convert them to directions using the
    current projection.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ [*dx/dy*][**+to**\|\ **O**\|\ **c**\|\ **C**]
    Adjust the clearance between the text and the surrounding box [15%].
    Only used if |-W| or |-G| are specified. Append the unit you
    want (**c**\ m, **i**\ nch, or **p**\ oint; if not given we consult
    :term:`PROJ_LENGTH_UNIT`) or % for a percentage of the font size.
    Optionally, use modifier **+t** to set the shape of the textbox when using |-G| and/or |-W|.
    Append lower case **o** to get a straight rectangle [Default].
    Append upper case **O** to get a rounded rectangle. In paragraph
    mode (|-M|) you can also append lower case **c** to get a concave
    rectangle or append upper case **C** to get a convex rectangle.

.. _-D:

**-D**\ [**j**\|\ **J**]\ *dx*\ [/*dy*][**+v**\ [*pen*]]
    Offsets the text from the projected (*x*,\ *y*) point by *dx*,\ *dy*
    [0/0]. If *dy* is not specified then it is set equal to *dx*. Use
    **-Dj** to offset the text away from the point instead (i.e., the
    text justification will determine the direction of the shift). Using
    **-DJ** will shorten diagonal offsets at corners by
    sqrt(2). Optionally, append **+v** which will draw
    a line from the original point to the shifted point; append a *pen*
    to change the attributes for this line.  **Note**: The **-Dj**\|\ **J**
    selection cannot be used with paragraph mode (|-M|).

.. _-F:

**-F**\ [**+a**\ [*angle*]][**+c**\ [*justify*]][**+f**\ [*font*]][**+j**\ [*justify*]][**+h**\|\ **l**\|\ **r**\ [*first*]\|\ **t**\ *text*\|\ **z**\ [*format*]]
    By default, text will be placed horizontally, using the primary
    annotation font attributes (:term:`FONT_ANNOT_PRIMARY`), and centered
    on the data point. Use |-F| to override these defaults by
    specifying up to three text attributes (font, angle, and
    justification) directly on the command line. Use modifier **+f** to set the
    font ([*size*][,\ *fontname*][,\ *color*]); if no font info is given then the input
    file must have this information in one of its columns. Use **+a** to
    set the *angle*; if no angle is given then the input file must have
    this as a column. Alternatively, use **+A** to force text-baselines
    to convert into the -90/+90 range.  Use **+j** to set the justification; if no
    justification is given then the input file must have this item as a
    column. Items read from the data file should be in the same order as
    the modifiers are specified with the |-F| option. Example:
    **-F**\ **+f**\ 12p,Helvetica-Bold,red\ **+j+a** selects a 12p red
    Helvetica-Bold font and expects to read the justification and angle
    from the file, in that order, after *x* *y* and before *text*.
    In addition, the **+c**\ *justification* lets us use *x,y* coordinates extracted from the
    |-R| string instead of providing them in the input file. For example **-F+c**\ TL
    gets the *x_min*, *y_max* from the |-R| string and plots the text
    at the Upper Left corner of the map.  Normally, the text to be plotted
    comes from the data record.  Instead, use **+h** or **+l** to select the
    text as the most recent segment header or segment label, respectively in
    a multisegment input file, **+r** to use the record number (counting up from *first*),
    **+t**\ *text* to set a fixed text string (if *text* contains plus characters then the
    **+t** modifier must be the last modifier in |-F|), or **+z** to format incoming *z* values
    to a string using the supplied *format* [use :term:`FORMAT_FLOAT_MAP`].  **Note**: If |-Z| is
    in effect then the *z* value used for formatting is in the 4th, not 3rd column.
    If you only want a specific word from the trailing text and not the whole line,
    use **-it**\ *word* to indicate which word (0 is the first word) you want.

.. _-G:

**-G**\ [*fill*][**+n**]
    Sets the shade or color used for filling the text box [Default is no
    fill]. Alternatively, give no *fill* to plot text and then use the
    text dimensions (and |-C|) to build clip paths and turn clipping on.
    This clipping can then be turned off later with :doc:`clip` |-C|.
    To **not** plot the text but activate clipping, use **-G+n** instead.
    **Note**: cannot be used with LaTeX expressions.

.. _-L:

**-L**
    Lists the font-numbers and font-names available, then exits.

.. _-M:

**-M**
    Paragraph mode. Files must be multiple segment files. Segments are
    separated by a special record whose first character must be *flag*
    [Default is **>**]. Starting in the 3rd field, we expect to find
    information pertaining to the typesetting of a text paragraph (the
    remaining lines until next segment header). The information expected
    is (*x y* [*font angle justify*] *linespace parwidth parjust*),
    where *x y font angle justify* are defined above (the presence or
    absence of *font*, *angle*, and *justify* are determined by |-F|), while *linespace* and
    *parwidth* are the linespacing and paragraph width, respectively.
    The justification of the text paragraph is governed by *parjust*
    which may be **l**\ (eft), **c**\ (enter), **r**\ (ight), or
    **j**\ (ustified). The segment header is followed by one or more
    lines with paragraph text. Text may contain the escape sequences
    discussed above, although composite characters are not supported.
    Separate paragraphs with a blank line.  Note that here,
    the justification set via **-F+j** applies to the box alignment
    since the text justification is set by *parjust*.  **Note**:
    cannot be used with LaTeX expressions.

.. _-N:

**-N**
    Do **not** clip text at map boundaries [Default will clip].

.. _-Q:

**-Q**\ **l**\|\ **u**
    Change all text to either **l**\ ower or **u**\ pper case [Default
    leaves all text as is].

.. _-S:

**-S**\ [*dx*/*dy*/][*shade*]
    Plot an offset background shaded region beneath the text box. Here, *dx*/*dy*
    indicates the shift relative to the text box [4\ **p**/-4\ **p**] and *shade*
    sets the fill color to use for shading [gray50].

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**
.. _-W:

**-W**\ *pen*
    Sets the pen used to draw a rectangle around the text string (see
    |-C|) [Default is width = 0.25p, color = black, style = solid].
    **Note**: cannot be used with LaTeX expressions.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**
    For 3-D projections: expect each item to have its own level given in
    the 3rd column, and |-N| is implicitly set. (Not implemented for
    paragraph mode).  **Note**: If **-F+z** is used then the text level
    is based on the 4th data column instead.

.. include:: explain_-aspatial.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. _-i:

**-it**\ *word*
    In this module, **-it** can be used to select a specific word from the
    trailing text [Default is the entire trailing text].  The *word* indicates
    the word order, with the first word being 0.  No numerical columns can be specified.

.. |Add_perspective| replace:: (Not implemented for paragraph mode).
.. include:: explain_perspective.rst_

.. include:: explain_-qi.rst_

.. include:: explain_-tv_full.rst_

.. include:: explain_-w.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. module_common_ends

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To plot just the red outlines of the (lon lat text strings) stored in the
file text.txt on a Mercator plot with the given specifications, use::

    gmt text text.txt -R-30/30/-10/20 -Jm0.1i -F+f18p,Helvetica,-=0.5p,red -B5 -pdf plot

To plot a text at the upper left corner of a 10 cm map::

    echo TopLeft | gmt text -R1/10/1/10 -JX10 -B -F+cTL -pdf plot

To add a typeset figure caption for a 3-inch wide illustration, use::

    gmt text -R0/3/0/5 -JX3i -h1 -M -N -F+f12,Times-Roman+jLT -pdf figure << EOF
    This is an unmarked header record not starting with #
    > 0 -0.5 13p 3i j
    @%5%Figure 1.@%% This illustration shows nothing useful, but it still needs
    a figure caption. Highlighted in @;255/0/0;red@;; you can see the locations
    of cities where it is @\_impossible@\_ to get any good Thai food; these are to be avoided.
    EOF

To place a line containing a Latex equation, try::

    echo 3 3 'Use @[\Delta g = 2\pi\rho Gh@[' | gmt text -R0/6/0/6 -JX15c -B -F+f32p+a30 -pdf map

To place text with a surrounding box and an underlying, shifted shade, both using a rounded rectangle, try::

    gmt text -R0/10/0/5 -Jx1c -F+f32p+cCM+tWELCOME -B -Gyellow -Wfaint -S -C+tO -pdf map

.. include:: text_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`clip`,
:doc:`gmtcolors`,
:doc:`psconvert`,
:doc:`basemap`,
:doc:`legend`, :doc:`plot`
