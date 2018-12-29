.. index:: ! text

******
text
******

.. only:: not man

    Plot or typeset text on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt text** [ *textfiles* ] |-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A| ]
|SYN_OPT-B|
[ |-C|\ [*dx/dy*\ ][\ **+t**\ o\|\O\|\c\|C\ ] ]
[ |-D|\ [**j**\ \|\ **J**]\ *dx*\ [/*dy*][\ **+v**\ [*pen*]] ]
[ |-F|\ [**+a**\ [*angle*]][\ **+c**\ [*justify*]][\ **+f**\ [*font*]][\ **+j**\ [*justify*]][\ **+h**\ \|\ **+l**\|\ **+r**\ [*first*] \|\ **+t**\ *text*\ \|\ **+z**\ [*format*]] ] 
[ |-G|\ *color* ]
[ |-L| ] [ |-M| ] [ |-N| ]
[ |-Q|\ **l**\ \|\ **u** ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-U| ]
[ |-Z| ] [ **-a**\ *col*\ =\ *name*\ [...] ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**text** plots text strings of variable size, font type, and
orientation. Various map projections are provided, with the option to
draw and annotate the map boundaries. Greek characters, subscript, superscript, and small
caps are supported as follows: The sequence @~ toggles between the
selected font and Greek (Symbol). @%\ *no*\ % sets the font to *no*; @%%
resets the font to the starting font, @- toggles subscripts on/off, @+
toggles superscript on/off, @# toggles small caps on/off, @;\ *color*;
changes the font color (@;; resets it), @:\ *size*: changes the font
size (@:: resets it), and @\_ toggles underline on/off. @@ prints the @
sign. @e, @o, @a, @E, @O, @A give the accented Scandinavian characters.
Composite characters (overstrike) may be indicated with the
@!<char1><char2> sequence, which will print the two characters on top of
each other. To learn the octal codes for symbols not available on the
keyboard and some accented European characters, see Section :ref:`Char-esc-seq` and
Appendix :ref:`Chart-Octal-Codes-for-Chars` in the GMT Technical Reference and Cookbook. Note that
:ref:`PS_CHAR_ENCODING <PS_CHAR_ENCODING>` must be set to an extended character set in your
:doc:`gmt.conf` file in order to use the accented characters. Using the
**-G** or **-W** options, a rectangle underlying the text may be plotted
(does not work for strings with sub/super scripts, symbols, or composite
characters, except in paragraph mode (**-M**)). 

Required Arguments
------------------

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

*textfiles*
    This is one or more files containing 1 or more records with (*x*,
    *y*\ [, *font*, *angle*, *justify*], *text*). The attributes in
    brackets can alternatively be set directly via **-F**. If no files
    are given, **text** will read standard input. *font* is a font
    specification with format [*size*,][\ *font*,][*color*\ ] where
    *size* is text size in points, *font* is the font to use, and
    *color* sets the font color. To draw outline fonts you append
    =\ *pen* to the font specification. The *angle* is measured in degrees
    counter-clockwise from horizontal, and *justify* sets the alignment.
    If *font* is not an integer, then it is taken to be a text string
    with the desired font name (see **-L** for available fonts). The
    alignment refers to the part of the text string that will be mapped
    onto the (*x*,\ *y*) point. Choose a 2 character combination of L,
    C, R (for left, center, or right) and T, M, B for top, middle, or
    bottom. e.g., BL for lower left.

.. _-A:

**-A**
    Angles are given as azimuths; convert them to directions using the
    current projection. 

.. _-B:

.. include:: explain_-B.rst_

.. _-C:

**-C**\ [*dx/dy*\ ][\ **+t**\ o\|\O\|\c\|C\ ]
    Adjust the clearance between the text and the surrounding box [15%].
    Only used if **-W** or **-G** are specified. Append the unit you
    want (**c**\ m, **i**\ nch, or **p**\ oint; if not given we consult
    :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>`) or % for a percentage of the font size.
    Optionally, use modifier **+t** to set the shape of the textbox when using **-G** and/or **-W**.
    Choose lower case o to get a straight rectangle [Default].
    Choose upper case O to get a rounded rectangle. In paragraph
    mode (**-M**) you can also choose lower case c to get a concave
    rectangle or upper case C to get a convex rectangle. 

.. _-D:

**-D**\ [**j**\ \|\ **J**]\ *dx*\ [/*dy*][\ **+v**\ [*pen*\ ]]
    Offsets the text from the projected (*x*,\ *y*) point by *dx*,\ *dy*
    [0/0]. If *dy* is not specified then it is set equal to *dx*. Use
    **-Dj** to offset the text away from the point instead (i.e., the
    text justification will determine the direction of the shift). Using
    **-DJ** will shorten diagonal offsets at corners by
    sqrt(2). Optionally, append **+v** which will draw
    a line from the original point to the shifted point; append a *pen*
    to change the attributes for this line.

.. _-F:

**-F**\ [**+a**\ [*angle*]][\ **+c**\ [*justify*]][\ **+f**\ [*font*]][\ **+j**\ [*justify*]][\ **+h**\ \|\ **+l**\|\ **+r**\ [*first*] \|\ **+t**\ *text*\ \|\ **+z**\ [*format*]]
    By default, text will be placed horizontally, using the primary
    annotation font attributes (:ref:`FONT_ANNOT_PRIMARY <FONT_ANNOT_PRIMARY>`), and centered
    on the data point. Use this option to override these defaults by
    specifying up to three text attributes (font, angle, and
    justification) directly on the command line. Use **+f** to set the
    font (size,fontname,color); if no font info is given then the input
    file must have this information in one of its columns. Use **+a** to
    set the angle; if no angle is given then the input file must have
    this as a column. Alternatively, use **+A** to force text-baselines
    to convert into the -90/+90 range.  Use **+j** to set the justification; if no
    justification is given then the input file must have this as a
    column. Items read from the data file should be in the same order as
    specified with the **-F** option. Example:
    **-F**\ **+f**\ 12p,Helvetica-Bold,red\ **+j+a** selects a 12p red
    Helvetica-Bold font and expects to read the justification and angle
    from the file, in that order, after *x*, *y* and before *text*.
    In addition, the **+c** justification lets us use x,y coordinates extracted from the
    **-R** string instead of providing them in the input file. For example **-F+c**\ TL
    gets the *x_min*, *y_max* from the **-R** string and plots the text
    at the Upper Left corner of the map.  Normally, the text to be plotted
    comes from the data record.  Instead, use **+h** or **+l** to select the
    text as the most recent segment header or segment label, respectively in
    a multisegment input file, **+r** to use the record number (counting up from *first*),
    **+t**\ *text* to set a fixed text string, or **+z** to format incoming *z* values
    to a string using the supplied *format* [use FORMAT_FLOAT_MAP].  Note: If **-Z** is
    in effect then the *z* value used for formatting is in the 4th, not 3rd column.

.. _-G:

**-G**\ *color*
    Sets the shade or color used for filling the text box [Default is no
    fill]. Alternatively, use **-Gc** to plot the text and then use the
    text dimensions (and **-C**) to build clip paths and turn clipping on.
    This clipping can then be turned off later with :doc:`clip` **-C**.
    To **not** plot the text but activate clipping, use **-GC** instead.

.. include:: explain_-Jz.rst_

.. _-L:

**-L**
    Lists the font-numbers and font-names available, then exits.

.. _-M:

**-M**
    Paragraph mode. Files must be multiple segment files. Segments are
    separated by a special record whose first character must be *flag*
    [Default is **>**]. Starting in the 3rd column, we expect to find
    information pertaining to the typesetting of a text paragraph (the
    remaining lines until next segment header). The information expected
    is (*x y* [*font angle justify*\ ] *linespace parwidth parjust*),
    where *x y font angle justify* are defined above (*font*, *angle*,
    and *justify* can be set via **-F**), while *linespace* and
    *parwidth* are the linespacing and paragraph width, respectively.
    The justification of the text paragraph is governed by *parjust*
    which may be **l**\ (eft), **c**\ (enter), **r**\ (ight), or
    **j**\ (ustified). The segment header is followed by one or more
    lines with paragraph text. Text may contain the escape sequences
    discussed above. Separate paragraphs with a blank line.  Note that
    here, the justification set via **-F+j** applies to the box alignment
    since the text justification is set by *parjust*.

.. _-N:

**-N**
    Do NOT clip text at map boundaries [Default will clip]. 

.. _-Q:

**-Q**\ **l**\ \|\ **u**
    Change all text to either **l**\ ower or **u**\ pper case [Default
    leaves all text as is].

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *pen*
    Sets the pen used to draw a rectangle around the text string (see
    **-C**) [Default is width = default, color = black, style = solid].

.. _-X:

.. include:: explain_-XY.rst_

.. _-Z:

**-Z**
    For 3-D projections: expect each item to have its own level given in
    the 3rd column, and **-N** is implicitly set. (Not implemented for
    paragraph mode). 

.. include:: explain_-aspatial.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_colon.rst_

.. |Add_perspective| replace:: (Not implemented for paragraph mode). 
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

Examples
--------

To plot just the red outlines of the (lon lat text strings) stored in the
file text.txt on a Mercator plot with the given specifications, use

   ::

    gmt text text.txt -R-30/30/-10/20 -Jm0.1i -F+f18p,Helvetica,-=0.5p,red -B5 -pdf plot

To plot a text at the upper left corner of a 10 cm map

   ::

    echo TopLeft | gmt text -R1/10/1/10 -JX10 -F+cTL -pdf plot

To add a typeset figure caption for a 3-inch wide illustration, use

   ::

    gmt text -R0/3/0/5 -JX3i -O -h1 -M -N -F+f12,Times-Roman+jLT << EOF >> figure.ps


   ::

    This is an unmarked header record not starting with #
    > 0 -0.5 13p 3i j
    @%5%Figure 1.@%% This illustration shows nothing useful, but it still needs
    a figure caption. Highlighted in @;255/0/0;red@;; you can see the locations
    of cities where it is @\_impossible@\_ to get any good Thai food; these are to be avoided.
    EOF

Windows Remarks
---------------

Note that under Windows, the percent sign (%) is a variable indicator
(like $ under Unix). To indicate a plain percentage sign in a batch
script you need to repeat it (%%); hence the font switching mechanism
(@%\ *font*\ % and @%%) may require twice the number of percent signs.
This only applies to text inside a script or that otherwise is processed
by DOS. Data files that are opened and read by **text** do not need
such duplication.

Limitations
-----------

In paragraph mode, the presence of composite characters and other escape
sequences may lead to unfortunate word splitting. Also, if a font is
requested with an outline pen it will not be used in paragraph mode.
Note if any single word is wider than your chosen paragraph width then
the paragraph width is automatically enlarged to fit the widest word.

Use from external interface
---------------------------

When **text** is called from external interfaces then we impose the following
condition on the **-F** setting: We require that **+a** (read angle from
input), if specified, must appear before either of **+f** (read font from input) and **+j**
(read justification from input), if these are present.  This is necessary because the
angle is a numerical column while font and justification must be encoded as part of the
trailing text.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`clip`,
:doc:`gmtcolors`,
:doc:`psconvert`,
:doc:`basemap`,
:doc:`legend`, :doc:`plot`
