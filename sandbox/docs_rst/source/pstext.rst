******
pstext
******


pstext - Plot or typeset text on maps

`Synopsis <#toc1>`_
-------------------

**pstext** [ *textfiles* ] **-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] [
**-A** ] [ **-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *dx/dy*
] [ **-D**\ [**j**\ \|\ **J**]\ *dx*\ [/*dy*][\ **v**\ [*pen*\ ]] ] [
**-F**\ [**+a**\ [*angle*\ ]][\ **+f**\ [*font*\ ]][\ **+j**\ [*justify*\ ]]
] [ **-G**\ *color* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-L** ] [ **-M** ] [ **-N** ] [ **-O** ] [ **-P** ] [
**-Q**\ **l**\ \|\ **u** ] [
**-T**\ **o**\ \|\ **O**\ \|\ **c**\ \|\ **C** ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [ **-W**\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-Z** ] [ **-a**\ *col*\ =\ *name*\ [*...*\ ] ] [ **-c**\ *copies*
] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**pstext** plots text strings of variable size, font type, and
orientation. Various map projections are provided, with the option to
draw and annotate the map boundaries. *PostScript* code is written to
standard output. Greek characters, subscript, superscript, and small
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
keyboard and some accented European characters, see Section 4.16 and
Appendix F in the **GMT** Technical Reference and Cookbook. Note that
**PS\_CHAR\_ENCODING** must be set to an extended character set in your
**gmt.conf** file in order to use the accented characters. Using the
**-G** or **-W** options, a rectangle underlying the text may be plotted
(does not work for strings with sub/super scripts, symbols, or composite
characters, except in paragraph mode (**-M**)).

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-J**\ *parameters* (\*)
    Select map projection.
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.

`Optional Arguments <#toc5>`_
-----------------------------

*textfiles*
    This is one or more files containing 1 or more records with (*x*,
    *y*\ [, *font*, *angle*, *justify*], *text*). The attributes in
    brackets can alternatively be set directly via **-F**. If no files
    are given, **pstext** will read standard input. *font* is a font
    specification with format [*size*,][\ *font*,][*color*\ ] where
    *size* is text size in points, *font* is the font to use, and
    *color* sets the font color. To draw outline fonts you append
    =\ *pen* to the filo. The *angle* is measured in degrees
    counter-clockwise from horizontal, and *justify* sets the alignment.
    If *font* is not an integer, then it is taken to be a text string
    with the desired fontname (see **-L** for available fonts). The
    alignment refers to the part of the text string that will be mapped
    onto the (*x*,\ *y*) point. Choose a 2 character combination of L,
    C, R (for left, center, or right) and T, M, B for top, middle, or
    bottom. e.g., BL for lower left.
**-A**
    Angles are given as azimuths; convert them to directions using the
    current projection.
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-C**\ *dx/dy*
    Sets the clearance between the text and the surrounding box [15%].
    Only used if **-W** or **-G** are specified. Append the unit you
    want (**c**\ m, **i**\ nch, or **p**\ oint; if not given we consult
    **PROJ\_LENGTH\_UNIT**) or % for a percentage of the font size.
**-D**\ [**j**\ \|\ **J**]\ *dx*\ [/*dy*][\ **v**\ [*pen*\ ]]
    Offsets the text from the projected (*x*,\ *y*) point by *dx*,\ *dy*
    [0/0]. If *dy* is not specified then it is set equal to *dx*. Use
    **-Dj** to offset the text away from the point instead (i.e., the
    text justification will determine the direction of the shift). Using
    **-DJ** will shorten diagonal offsets at corners by
    `sqrt(2) <sqrt.2.html>`_ . Optionally, append **v** which will draw
    a line from the original point to the shifted point; append a *pen*
    to change the attributes for this line.
**-F**\ [**+a**\ [*angle*\ ]][\ **+f**\ [*font*\ ]][\ **+j**\ [*justify*\ ]]
    By default, text will be placed horizontally, using the primary
    annotation font attributes (**FONT\_ANNOT\_PRIMARY**), and centered
    on the data point. Use this option to override these defaults by
    specifying up to three text attributes (font, angle, and
    justification) directly on the command line. Use **+f** to set the
    font (size,fontname,color); if no font info is given then the input
    file must have this information in one of its columns. Use **+a** to
    set the angle; if no angle is given then the input file must have
    this as a column. Use **+j** to set the justification; if no
    justification is given then the input file must have this as a
    column. Items read from the data file should be in the same order as
    specified with the **-F** option. Example:
    **-F**\ **+f**\ 12p,Helvetica-Bold,red\ **+j+a** selects a 12p red
    Helvetica-Bold font and expects to read the justification and angle
    from the file, in that order, after *x*, *y* and before *text*.
**-G**\ *color*
    Sets the shade or color used for filling the text box [Default is no
    fill]. Alternatively, use **-Gc** to use text (and **-C**) to build
    clip paths and turn clipping on. This clipping can then be turned
    off later and the text may be finally plotted using psclip **-Ct**
    (provided only one pstext call was issued).
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**
    Lists the font-numbers and font-names available, then exits.
**-M**
    Paragraph mode. Files must be multiple segment files. Segments are
    separated by a special record whose first character must be *flag*
    [Default is ’>’]. Starting in the 3rd column, we expect to find
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
    discussed above. Separate paragraphs with a blank line.
**-N**
    Do NOT clip text at map boundaries [Default will clip].
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**
    Change all text to either **l**\ ower or **u**\ pper case [Default
    leaves all text as is].
**-T**
    Specify the shape of the textbox when using **-G** and/or **-W**.
    Choose lower case **o** to get a straight rectangle [Default].
    Choose upper case **O** to get a rounded rectangle. Choose lower
    case **c** to get a concave rectangle (only in paragraph mode).
    Choose upper case **C** to get a convex rectangle (only in paragraph
    mode).
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**\ *pen*
    Sets the pen used to draw a rectangle around the text string (see
    **-T**) [Default is width = default, color = black, style = solid].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]] (\*)
    Shift plot origin.
**-Z**
    For 3-D projections: expect each item to have its own level given in
    the 3rd column, and **-N** is implicitly set. (Not implemented for
    paragraph mode).
**-a**\ *col*\ =\ *name*\ [*...*\ ] (\*)
    Set aspatial column associations *col*\ =\ *name*.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*] (\*)
    Select perspective view. (Not implemented for paragraph mode).
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Examples <#toc6>`_
-------------------

To plot just the red outlines of the (lon at text strings) stored in the
file text.d on a Mercator plot with the given specifications, use

pstext text.d -R-30/30/-10/20 -Jm0.1i -P -F+f18p,Helvetica,-=0.5p,red -B5 > plot.ps

To add a typeset figure caption for a 3-inch wide illustration, use

pstext -R0/3/0/5 -JX3i -O -H -M -N << EOF >> figure.ps
This is an optional header record
> 0 -0.5 12 0 4 LT 13p 3i j
@%5%Figure 1.@%% This illustration shows nothing useful, but it still needs
a figure caption. Highlighted in @;255/0/0;red@;; you can see the locations
of cities where it is @\_impossible@\_ to get any good Thai food; these
are to be avoided.
EOF

`Windows Remarks <#toc7>`_
--------------------------

Note that under Windows, the percent sign (%) is a variable indicator
(like $ under Unix). To indicate a plain percentage sign in a batch
script you need to repeat it (%%); hence the font switching mechanism
(@%*font*\ % and @%%) may require twice the number of percent signs.
This only applies to text inside a script or that otherwise is processed
by DOS. Data files that are opened and read by **pstext** do not need
such duplication.

`Limitations <#toc8>`_
----------------------

In paragraph mode, the presence of composite characters and other escape
sequences may lead to unfortunate word splitting. Also, if a font is
requested with an outline pen it will not be used in paragraph mode.
The **-N** option does not adjust the BoundingBox information so you
may have to post-process the *PostScript* output with ps2raster to
obtain a correct BoundingBox.

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*gmt.conf*\ (5) <gmt.conf.5.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.5.html>`_ ,
`*ps2raster*\ (1) <ps2raster.1.html>`_ ,
`*psbasemap*\ (1) <psbasemap.1.html>`_ ,
`*pslegend*\ (1) <pslegend.1.html>`_ , `*psxy*\ (1) <psxy.1.html>`_

