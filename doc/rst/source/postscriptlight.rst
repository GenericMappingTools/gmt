.. index:: ! postscriptlight

***************
postscriptlight
***************

.. only:: not man

    PSL 6.0 - A PostScript based plotting library

Description
-----------

PSL (PostScriptLight) was created to make the generation of PostScript page
description code easier. PS is a page description language developed by
the Adobe for specifying how a printer should render a page of text or
graphics. It uses a reverse Polish notation that puts and gets items
from a stack to draws lines, text, and images and even performs
calculations. PSL is a self-contained library that presents a series
of functions that can be used to create plots. The resulting
PostScript code is ASCII text (with some exceptions for images if so
desired) and can thus be edited using any text editor. Thus, it is
possible to modify a plot file even after it has been created, e.g., to
change text strings, set new gray shades or colors, experiment with
various pen widths, etc. Furthermore, various tools exist that can parse
PostScript and let you make such edits via a graphical user interface
(e.g., Adobe Illustrator). PSL is written in C but includes FORTRAN
bindings and can therefore be called from both C and FORTRAN programs.
To use this library, you must link your plotting program with PSL.
PSL is used by the GMT graphics programs to generate PS. PSL
output is freeform PostScript that conforms to the Adobe PostScript
File Specification Version 3.0.

Before any PSL calls can be issued, the plotting system must be
initialized. This is done by calling **PSL_beginsession**, which
initializes a new PSL session; then call **PSL_setdefaults** which
sets internal variables and default settings, accepts settings for
measurement units and character encoding, and returns a pointer to a
struct PSL_CTRL which must be passed as first argument to all other
PSL functions. The measure unit for sizes and positions can be set
to be centimeter (c), inch (i), meter (m), or points
(p). A PSL session is terminated by calling
**PSL_endsession**. You may create one or more plots within the same
session. A new plot is started by calling **PSL_beginplot**, which
defines macros, sets up the plot-coordinate system, scales, and
[optionally] opens a file where all the PS code will be written.
Normally, the plot code is written to *stdout*. When all plotting to
this file is done, you finalize the plot by calling **PSL_endplot**.

A wide variety of output devices that support PostScript exist,
including many printers and large-format plotters. Many tools exists to
display PostScript on a computer screen. Open source tools such as
ghostscript can be used to convert PostScript into PDF or raster
images (e.g., TIFF, JPEG) at a user-defined resolution (DPI). In
particular, the GMT tool :doc:`psconvert` is a front-end to ghostscript and
pre-selects the optimal options for ghostscript that will render quality
PDF and images.

The PSL is fully 64-bit compliant. Integer parameters are here
specified by the type **long** to distinguish them from the 32-bit
**int**. Note that under standard 32-bit compilation they are
equivalent. Users of this library under 64-bit mode must make sure they
pass proper **long** variables (under Unix flavors) or **\_\_int64**
under Windows 64.

Units
-----

PSL can be instructed to use centimeters, inches, meters or points
as input units for the coordinates and sizes of elements to be plotted.
Any dimension that takes this setting as a unit is specified as *user
units* or *plot units* in this manual. Excluded from this are line
widths and font sizes which are always measured in *points*. The user
units can be further refined by calling **PSL_beginaxes**, giving the
user the opportunity to specify any linear coordinate frame. Changing
the coordinate frame only affects the coordinates of plotted material
indicated as measured in *plot units*, not the sizes of symbols (which
remain in *user units*), nor line widths or font sizes (which remain in
*points*).

Color
-----

PSL uses the direct color model where red, green, and blue are given
separately, each must be in the range from 0-1. If red = -1 then no fill
operation takes place. If red = -3, then pattern fill will be used, and
the green value will indicate the pattern to be used. Most plot-items
can be plotted with or without outlines. If outline is desired (i.e.,
set to 1), it will be drawn using the current line width and pattern.
PSL uses highly optimized macro substitutions and scales the
coordinates depending on the resolution of the hardcopy device so that
the output file is kept as compact as possible.

Justification
-------------

Text strings, text boxes and images can be "justified" by specifying the
corner to which the *x* and *y* coordinates of the subroutine call
apply. Nine different values are possible, as shown schematically in
this diagram:

    9------------10----------- 11

    \|                         \|

    5             6             7

    \|                          \|

    1------------ 2------------ 3

The box represents the text or image. E.g., to plot a text string with
its center at (*x*, *y*), you must use *justify* == 6. *justify* == 0
means "no justification", which generally means (*x*, *y*) is at the
bottom left. Convenience values PSL_NONE, PSL_BL, PSL_BC, PSL_BL,
PSL_ML, PSL_MC, PSL_MR, PSL_TL, PSL_TC and PSL_TR are available.

Initialization
--------------

These functions initialize or terminate the PSL system. We use the
term PSL session to indicate one instance of the PSL system (a
complicated program could run many PSL sessions concurrently as each
would operate via its own control structure). During a single session,
one or more plots may be created. Here are the functions involved in
initialization:

**struct PSL_CTRL \*New_PSL_Ctrl** (**char** *\*session*)

    This is the first function that must be called as it creates a new
    PSL session. Specifically, it will allocate a new PSL
    control structure and initialize the session default parameters. The
    pointer that is returned must be passed to all subsequent PSL
    functions.

**long \*PSL_beginsession** (**struct PSL_CTRL** *\*PSL*, **long**
*search*, **char** *\*sharedir*, **char** *\*userdir*)

    This is the second function that must be called as it initializes
    the new PSL session. Here, *search* is an integer that is passed
    as 0 in GMT but should be 1 for other users. If so we will search
    for the environmental parameters PSL_SHAREDIR and PSL_USERDIR
    should the corresponding arguments *sharedir* and *userdir* be NULL.

**long PSL_endsession** (**struct PSL_CTRL** *\*PSL*)

    This function terminates the active PSL session; it is the last
    function you must call in your program. Specifically, this function
    will deallocate memory used and free up resources.

**struct PSL_CTRL \*PSL_beginlayer** (**struct PSL_CTRL** *\*PSL*,
**long** *layer*)

    Adds a DSC comment by naming this layer; give a unique integer
    value. Terminate layer with PSL_endlayer

**struct PSL_CTRL \*PSL_endlayer** (**struct PSL_CTRL** *\*PSL*)

    Terminate current layer with a DSC comment.

**long PSL_fopen** (**char** *\*file*, **char** *\*mode*)

    This function simply opens a file, just like fopen. The reason it is
    replicated here is that under Windows, file pointers must be
    assigned within the same DLL as they are being used. Yes, this is
    retarded but if we do not do so then PSL will not work well under
    Windows. Under non-Windows this functions is just a macro that
    becomes fopen.

**void PSL_copy** (**struct PSL_CTRL** *\*PSL*, **char** *\*text*);

    This functions simply adds the given *test* as is to the output
    PostScript stream (or internal buffer).  It is used when the
    text may exceed the internal 4096 buffer used in PSL_command
    (which is implemented as a printf function and hence a buffer
    of fixed size is used.)

**void PSL_free** (**void** *\*ptr*)

    This function frees up the memory allocated inside PSL.
    Programmers using C/C++ should now this is a macro and there is no
    need to cast the pointer to *void \** as this will be done by the
    macro. Fortran programmers should instead call
    **PSL_freefunction**.

**void PSL_beginaxes** (**struct PSL_CTRL** *\*PSL*, **double** *llx*,
**double** *lly*, **double** *width*, **double** *height*, **double**
*x0*, **double** *y0*, **double** *x1*, **double** *y1*)

    This function sets up the mapping that takes the users data
    coordinates and converts them to the positions on the plot in
    PostScript units. This should be used when plotting data
    coordinates and is terminated with **PSL_endaxes**, which returns
    PSL to the default measurement units and scaling. Here, *llx*
    and *lly* sets the lower left position of the mapping region, while
    *width* and *height* sets the dimension of the plot area in user
    units. Finally, *x0*, *x1* and *y0*, *y1* indicate the range of the
    users x- and y-coordinates, respectively. Specify a reverse axis
    direction (e.g., to let the y-axis be positive down) by setting *y0*
    larger than *y1*, and similarly for an x-axis that increases to the
    left.

**void PSL_endaxes** (**struct PSL_CTRL** *\*PSL*)

    Terminates the map scalings initialized by **PSL_beginaxes** and
    returns PSL to standard scaling in measurement units.

**long PSL_beginplot** (**struct PSL_CTRL** *\*P*, **FILE** *\*fp*,
**long** *orientation*, **long** *overlay*, **long** *color_mode*,
**char** *origin*\ [], **double** *offset*\ [], **double**
*page_size*\ [], **char** *\*title*, **long** *font_no*\ [])

    Controls the initiation (or continuation) of a particular plot
    within the current session. Pass file pointer *fp* where the
    PostScript code will be written; if NULL then the output is
    written to *stdout*. The Fortran interface always sends to *stdout*.
    If you want to receive the PostScript back in memory then you need
    to add PSL_MEMORY to *orientation* and call **PSL_getplot** to retrieve
    the plot after you finish the plot with **PSL_endplot**.
    The *orientation* may be landscape (PSL_LANDSCAPE or 0) or portrait
    (PSL_PORTRAIT or 1). Set *overlay* to PSL_OVERLAY (0) if the
    following PostScript code should be appended to an existing plot;
    otherwise pass PSL_INIT (1) to start a new plot.
    Let *colormode* be one of PSL_RGB (0), PSL_CMYK
    (1), PSL_HSV (2) or PSL_GRAY (3); this setting controls how colors
    are presented in the PostScript code. The *origin* setting
    determines for x and y separately the origin of the specified
    offsets (next argument). Each of the two characters are either **r**
    for an offset relative to the current origin, **a** for a temporary
    adjustment of the origin which is undone during BD(PSL_endplot),
    **f** for a placement of the origin relative to the lower left corner
    of the page, **c** for a placement of the origin relative to the
    center of the page. The array *offset* specifies the offset of the
    new origin relative to the position indicated by **origin**.
    *page_size* means the physical width and height of the plotting
    media in points (typically 612 by 792 for Letter or 595 by 842 for
    A4 format). The character string *title* can be used to specify the
    **%%Title:** header in the PostScript file (or use NULL for the
    default). The array *font_no* specifies all fonts used in the plot
    (by number), or use NULL to leave out the
    **%%DocumentNeededResources:** comment in the PostScript file.

**long PSL_endplot** (**struct PSL_CTRL** *\*P*, **long** *last_page*)

    Terminates the plotting sequence and closes plot file (if other than
    *stdout*). If *last_page* == PSL_FINALIZE
    (1), then a PostScript *showpage* command
    is issued, which initiates the printing process on hardcopy devices.
    Otherwise, pass PSL_OVERLAY (0).

**long PSL_setorigin** (**struct PSL_CTRL** *\*P*, **double**
*xorigin*, **double** *yorigin*, **double** *angle*, **long** *mode*)

    Changes the coordinate system by translating by
    (*xorigin*,\ *yorigin*) followed by a *angle*-degree rotation
    (*mode*\ =PSL_FWD or 0) or alternatively the rotation followed by
    translation (*mode*\ =PSL_INV or 1).

Memory Output
-------------

Normally, PSL will write all PostScript to the designated file stream
set in **PSL_beginplot**.  Alternatively, PSL can write all the PostScript
to an internal char * buffer which can be retrieved at the end of the plotting.
This mode can be enabled on a plot-by-plot basis by adding the flag **PSL_MEMORY**
to the variable *orientation* passed to **PSL_beginplot**.  Once we reach the
end of the plot with **PSL_endplot** the buffer will be available (see below).
One function provide the functionality for memory output.

**char * PSL_getplot** (**struct PSL_CTRL** *\*P*)

    Retrieves the pointer to the PostScript plot that is kept in memory
    when **PSL_beginplot** was instructed to use memory rather than
    stream output.  Note: It is the responsibility of the programmer to
    ensure that the object retrieved is duplicated or written or otherwise
    processed before the next call to **PSL_beginplot** or **PSL_endsession**
    either of which will destroy the memory pointed to.

`Changing Settings <#toc6>`_
----------------------------

The following functions are used to change various PSL settings and
affect the current state of parameters such as line and fill attributes.

**long PSL_define_pen** (**struct PSL_CTRL** *\*P*, **char**
*\*name*, **long** *width*, **char** *\*style*, **double** *offset*,
**double** *rgb*\ [])

    Stores the specified pen characteristics in a PostScript variable
    called *name*. This can be used to place certain pen attributes in
    the PostScript file and then retrieve them later with
    **PSL_load_pen**. This makes the stored pen the current pen.

**long PSL_define_rgb** (**struct PSL_CTRL** *\*P*, **char**
*\*name*, **double** *rgb*\ [])

    Stores the specified color in a PostScript variable called *name*.
    This can be used to place certain color values in the PostScript
    file and then retrieve them later with **PSL_load_rgb**. This
    makes the stored color the current color.

**long PSL_setexec** (**struct PSL_CTRL** *\*P*, **long** *mode*)

    If *mode* = 1 then we tell PSL to execute a custom PostScript
    procedure named PSL_completion at the start of the next overlay.
    Once executed, the function is reset to a dummy null procedure.
    Experts may define their own procedure called PSL_completion
    and insert it into the PostScript stream.  Changing the mode
    can then be used to have some tasks complete prior to the
    new overlay being generated.

**long PSL_setcolor** (**struct PSL_CTRL** *\*P*, **double**
*rgb*\ [], **long** *mode*)

    Sets the current color for all stroked (mode = PSL_IS_STROKE (0))
    or filled (mode = PSL_IS_FILL (1)) material
    to follow (lines, symbol outlines, text). *rgb* is a triplet of red,
    green and blue values in the range 0.0 through 1.0. Set the red
    color to -3.0 and the green color to the pattern number returned by
    **PSL_setimage** to select an image pattern as current paint color. For
    PDF transparency, set *rgb*\ [3] to a value between 0 (opaque) and 1
    (fully transparent).

**long PSL_setimage** (**struct PSL_CTRL** *\*P*, **long**
*image_no*, **char** *\*imagefile*, **unsigned char** *\*image*,
**long** *dpi*, **long dim[3], **double** *f_rgb*\ [], **double** *b_rgb*\ [])

    Sets up the specified image pattern as the fill to use for polygons
    and symbols. Here, *image_no* is the number of the standard PSL
    fill patterns (1-90; use a negative number when you specify an image
    *filename* instead and pass the *image* data vector and the dimensions
    of the image via the *dim* array (width, height, and bit-depth).
    The scaling (i.e., resolution in dots per inch)
    of the pattern is controlled by the image *dpi*; if set to 0 it will
    be plotted at the device resolution. The last two arguments
    apply to 1-bit images only and are otherwise ignored: You may
    replace the foreground color (the set bits) with the *f_rgb* color
    and the background color (the unset bits) with *b_rgb*.
    Alternatively, pass either color with the red component set to -1.0
    and we will instead issue an image mask that is see-through for the
    specified fore- or background component. To subsequently use the
    pattern as a pen or fill color, use **PSL_setcolor** or
    DB(PSL_setfill) with the a color *rgb* code made up of *r* = -3,
    and *b* = the pattern number returned by **PSL_setimage**.

**long PSL_setdash** (**struct PSL_CTRL** *\*P*, **char** *\*pattern*,
**double** *offset*)

    Changes the current pen style attributes. The character string
    *pattern* contains the desired pattern using a series of lengths in
    points specifying the alternating lengths of dashes and gaps in
    points. E.g., "4 2" and *offset* = 1 will plot like

        x ---- ---- ----

    where x is starting point of a line (The x is not plotted). That is,
    the line is made up of a repeating pattern of a 4 points long solid
    line and a 2 points long gap, starting 1 point after the x. To reset
    to solid line, specify *pattern* = NULL ("") and *offset* = 0.

**long PSL_setfill** (**struct PSL_CTRL** *\*P*, **double** *rgb*\ [],
**long** *outline*)

    Sets the current fill color and whether or not outline is needed for
    symbols. Special cases are handled by passing the red color as -1.0
    (no fill), -2.0 (do not change the outline setting) or -3.0 (select
    the image pattern indicated by the second (green) element of *rgb*).
    For PDF transparency, set *rgb*\ [3] to a value between 0 (opaque)
    and 1 (fully transparent). Set outline to PSL_OUTLINE
    (1) to draw the outlines of polygons and symbols using the current pen.

**long PSL_setfont** (**struct PSL_CTRL** *\*P*, **long** *fontnr*)

    Changes the current font number to *fontnr*. The fonts available
    are: 0 = Helvetica, 1 = H. Bold, 2 = H. Oblique, 3 = H.
    Bold-Oblique, 4 = Times, 5 = T. Bold, 6 = T. Italic, 7 = T. Bold
    Italic, 8 = Courier, 9 = C. Bold, 10 = C Oblique, 11 = C Bold
    Oblique, 12 = Symbol, 13 = AvantGarde-Book, 14 = A.-BookOblique, 15
    = A.-Demi, 16 = A.-DemiOblique, 17 = Bookman-Demi, 18 =
    B.-DemiItalic, 19 = B.-Light, 20 = B.-LightItalic, 21 =
    Helvetica-Narrow, 22 = H-N-Bold, 23 = H-N-Oblique, 24 =
    H-N-BoldOblique, 25 = NewCenturySchlbk-Roman, 26 = N.-Italic, 27 =
    N.-Bold, 28 = N.-BoldItalic, 29 = Palatino-Roman, 30 = P.-Italic, 31
    = P.-Bold, 32 = P.-BoldItalic, 33 = ZapfChancery-MediumItalic, 34 =
    ZapfDingbats, 35 = Ryumin-Light-EUC-H, 36 = Ryumin-Light-EUC-V, 37 =
    GothicBBB-Medium-EUC-H, and 38 = GothicBBB-Medium-EUC-V. If *fontnr*
    is outside this range, it is reset to 0.

**long PSL_setfontdims** (**struct PSL_CTRL** *\*P*, **double** *supsub*,
**double** *scaps*, **double** *sup*, **double** *sdown*)

    Changes the settings for a variety of relative font sizes and shifts
    pertaining to sub-scripts, super-scripts, and small caps.  Default
    settings are given in brackets.  Here, *supsub* sets the relative size
    of sub- and super-scripts [0.58], *scaps* sets the relative size of
    small caps [0.8], *sup* indicates the upward baseline shift for placement
    of super-scripts [0.33], while *sdown* sets the downward baseline shift
    for sub-scripts [0.33].

**long PSL_setformat** (**struct PSL_CTRL** *\*P*, **long** *n_decimals*)

    Sets the number of decimals to be used when writing color or gray
    values. The default setting of 3 gives 1000 choices per red, green,
    and blue value, which is more than the 255 choices offered by most
    24-bit platforms. Choosing a lower value will make the output file
    smaller at the expense of less color resolution. Still, a value of 2
    gives 100 x 100 x 100 = 1 million colors, more than most eyes can
    distinguish. For a setting of 1, you will have 10 nuances per
    primary color and a total of 1000 unique combinations.

**long PSL_setlinewidth** (**struct PSL_CTRL** *\*P*, **double**
*linewidth*)

    Changes the current line width in points. Specifying 0 gives the
    thinnest line possible, but this is implementation-dependent (seems
    to work fine on most PostScript printers).

**long PSL_setlinecap** (**struct PSL_CTRL** *\*P*, **long** *cap*)

    Changes the current line cap, i.e., what happens at the beginning
    and end of a line segment. PSL_BUTT_CAP (0) gives butt line caps
    [Default], PSL_ROUND_CAP (1) selects round
    caps, while PSL_SQUARE_CAP (2) results
    in square caps. Thus, the two last options will visually lengthen a
    straight line-segment by half the line width at either end.

**long PSL_setlinejoin** (**struct PSL_CTRL** *\*P*, **long** *join*)

    Changes the current linejoin setting, which handles how lines of
    finite thickness are joined together when the meet at different
    angles. PSL_MITER_JOIN (0) gives a mitered joint [Default],
    PSL_ROUND_JOIN (1) makes them round,
    while PSL_BEVEL_JOIN (2) produces bevel joins.

**long PSL_setmiterlimit** (**struct PSL_CTRL** *\*P*, **long** *limit*)

    Changes the current miter limit used for mitered joins.
    PSL_MITER_DEFAULT (35) gives the default PS miter; other values
    are interpreted as the cutoff acute angle (in degrees) when mitering
    becomes active.

**long PSL_settransparencymode** (**struct PSL_CTRL** *\*P*, **char**
*\*mode*)

    Changes the current PDF transparency rendering mode [Default is
    Normal]. Choose among Color, ColorBurn, ColorDodge, Darken,
    Difference, Exclusion, HardLight, Hue, Lighten, Luminosity,
    Multiply, Normal, Overlay, Saturation, SoftLight, and Screen.

**long PSL_setdefaults** (**struct PSL_CTRL** *\*P*, **double**
*xyscales*\ [], **double** *pagergb*\ [], **char** *\*encoding*)

    Allows changes to the PSL session settings and should be called
    immediately after **PSL_beginsession**. The *xyscales* array affect
    an overall magnification of your plot [1,1]. This can be useful if
    you design a page-sized plot but would then like to magnify (or
    shrink) it by a given factor. Change the default paper media color
    [white; 1/1/1] by specifying an alternate page color. Passing zero
    (or NULL for *pagergb*) will leave the setting unchanged. Finally,
    pass the name of the character set encoding (if NULL we select
    Standard).

**long PSL_defunits** (**struct PSL_CTRL** *\*P*, **char** *\*name*,
**double** *value*)

    Creates a PostScript variable called *name* and initializes it to
    the equivalent of *value* user units.

**long PSL_defpoints** (**struct PSL_CTRL** *\*P*, **char** *\*name*,
**double** *fontsize*)

    Creates a PostScript variable called *name* and initializes it to
    the value that corresponds to the font size (in points) given by
    *fontsize*.

`Plotting Lines And Polygons <#toc7>`_
--------------------------------------

Here are functions used to plot lines and closed polygons, which may
optionally be filled. The attributes used for drawing and filling are
set prior to calling these functions; see CHANGING SETTINGS above.

**long PSL_plotarc** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **double** *radius*, **double** *angle1*, **double**
*angle2*, **long** *type*)

    Draws a circular arc with its center at plot coordinates (*x*, *y*),
    starting from angle *angle1* and end at *angle2*. Angles must be
    given in decimal degrees. If *angle1* > *angle2*, a negative arc is
    drawn. The *radius* is in user units. The *type* determines how the
    arc is interpreted: PSL_MOVE (1) means set new
    anchor point, PSL_STROKE (2) means stroke
    the arc, PSL_MOVE + PSL_STROKE (3) means
    both, whereas PSL_DRAW (0) just adds to arc path to the current
    path.

**long PSL_plotline** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **long** *n*, **long** *type*)

    Assemble a continuous line through *n* points whose the plot
    coordinates are in the *x*, *y* arrays. To continue an existing
    line, use *type* = PSL_DRAW (0), or if this is the first segment in
    a multisegment path, set *type* = PSL_MOVE (1).
    To end the segments and draw the lines, add PSL_STROKE
    (2). Thus, for a single segment, *type* must
    be PSL_MOVE + PSL_STROKE (3). The line is
    drawn using the current pen attributes. Add PSL_CLOSE
    (8) to *type* to close the first and last point
    by the PostScript operators; this is done automatically if the
    first and last point are equal.

**long PSL_plotpoint** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **long** *type*)

    Moves the pen from the current to the specified plot coordinates
    (*x*, *y*) and optionally draws and strokes the line, depending on
    *type*. Specify *type* as either a move (PSL_MOVE, 1), or draw
    (PSL_DRAW, 2), or draw and stroke (PSL_DRAW + PSL_STOKE, 3) using
    current pen attributes. It the coordinates are relative to the
    current point add PSL_REL (4) to *type*.

**long PSL_plotbox** (**struct PSL_CTRL** *\*P*, **double** *x0*,
**double** *y0*, **double** *x1*, **double** *y1*)

    Creates a closed box with opposite corners at plot coordinates
    (*x0*,\ *y1*) and (*x1*,\ *y1*). The box may be filled and its
    outline stroked depending on the current settings for fill and pen
    attributes.

**long PSL_plotpolygon** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **long** *n*)

    Creates a closed polygon through *n* points whose plot coordinates
    are in the *x*, *y* arrays. The polygon may be filled and its
    outline stroked depending on the current settings for fill and pen
    attributes.

**long PSL_plotsegment** (**struct PSL_CTRL** *\*P*, **double** *x0*,
**double** *y0*, **double** *x1*, **double** *y1*)

    Draws a line segment between the two points (plot coordinates) using
    the current pen attributes.

`Plotting Symbols <#toc8>`_
---------------------------

Here are functions used to plot various geometric symbols or constructs.

**long PSL_plotaxis** (**struct PSL_CTRL** *\*P*, **double**
*tickval*, **char** *\*label*, **double** *fontsize*, **long** *side*)

    Plots a basic axis with tick marks, annotations, and label. Assumes
    that **PSL_beginaxes** has been called to set up positioning and
    user data ranges. Annotations will be set using the *fontsize* in
    points. *side* can be 0, 1, 2, or 3, which selects lower x-axis,
    right y-axis, upper x-axis, or left y-axis, respectively. The
    *label* font size is set to 1.5 times the *fontsize*.

**long PSL_plotsymbol** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **double** *size*\ [], **long** *symbol*)

    Plots a simple geometric symbol centered on plot coordinates (*x*,
    *y*). The argument *symbol* selects the geometric symbol to use.
    Most symbols are scaled to fit inside a circle of diameter given as
    *size*\ [0], but some symbols take additional parameters. Choose
    from these 1-parameter symbols using the predefined self-explanatory
    integer values PSL_CIRCLE, PSL_DIAMOND, PSL_HEXAGON,
    PSL_INVTRIANGLE, PSL_OCTAGON, PSL_PENTAGON, PSL_SQUARE,
    PSL_STAR, and PSL_TRIANGLE; these may all be filled and stroked if
    **PSL_setfill** has been called first. In addition, you can choose
    several line-only symbols that cannot be filled. They are
    PSL_CROSS, PSL_DOT, PSL_PLUS, PSL_XDASH, and PSL_YDASH.
    Finally, more complicated symbols require more than one parameter to
    be passed via *size*. These are PSL_ELLIPSE (*size* is expected to
    contain the three parameter *angle*, *major*, and *minor* axes,
    which defines an ellipse with its major axis rotated by *angle*
    degrees), PSL_MANGLE (*size* is expected to contain the 10
    parameters *radius*, *angle1*, and *angle2* for the math angle
    specification, followed by *tailwidth*, *headlength*, *headwidth*,
    *shape*, *status*, *trim1* and *trim2* (see PSL_VECTOR below for explanation),
    PSL_WEDGE (*size* is expected to contain the three parameter
    *radius*, *angle1*, and *angle2* for the sector specification),
    PSL_RECT (*size* is expected to contain the two dimensions *width*
    and *height*), PSL_RNDRECT (*size* is expected to contain the two
    dimensions *width* and *height* and the *radius* of the corners),
    PSL_ROTRECT (*size* is expected to contain the three parameter
    *angle*, *width*, and *height*, with rotation relative to the
    horizontal), and PSL_VECTOR (*size* is expected to contain the 9
    parameters *x_tip*, *y_tip*, *tailwidth*, *headlength*,
    *headwidth*, *shape*, *status*, *head1*, *head2*, *trim1*, and *trim2*.
    Here (*x_tip*,\ *y_tip*) are
    the coordinates to the head of the vector, while (*x*, *y*) are
    those of the tail. *shape* can take on values from 0-1 and specifies
    how far the intersection point between the base of a straight vector
    head and the vector line is moved toward the tip. 0.0 gives a
    triangular head, 1.0 gives an arrow shaped head. The *status* value
    is a bit-flag being the sum of several possible contributions:
    PSL_VEC_RIGHT (2) = only draw right half
    of vector head, PSL_VEC_BEGIN (4) =
    place vector head at beginning of vector,
    PSL_VEC_END (8) = place vector head at end of vector,
    PSL_VEC_JUST_B (0) = align vector beginning at (x,y),
    PSL_VEC_JUST_C (16) = align vector center at (x,y),
    PSL_VEC_JUST_E (32) = align vector end at (x,y),
    PSL_VEC_JUST_S (64) = align vector center at (x,y),
    PSL_VEC_OUTLINE (128) = draw vector head outline using default
    pen, PSL_VEC_FILL (512) = fill vector head using default fill,
    PSL_VEC_MARC90 (2048) = if angles subtend 90, draw straight angle
    symbol (PSL_MANGLE only). The symbol may be filled and its outline
    stroked depending on the current settings for fill and pen
    attributes.  The parameters *head1* and *head2* determines
    what kind of vector head will be plotted at the two ends (if selected).
    0 = normal vector head, 1 = circle, 2 = terminal crossbar.
    Finally, *trim1* and *trim2* adjust the start and end location of
    the vector.

`Plotting Images <#toc9>`_
--------------------------

Here are functions used to read and plot various images.

**long PSL_plotbitimage** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **double** *xsize*, **double** *ysize*, **int**
*justify*, **unsigned char** *buffer*, **long** *nx*, **long** *ny*,
**double** *f_rgb*\ [], **double** *b_rgb*\ [])

    Plots a 1-bit image image at plot coordinates (*x*, *y*) justified
    as per the argument *justify* (see **JUSTIFICATION** for details).
    The target size of the image is given by *xsize* and *ysize* in user
    units. If one of these is specified as zero, the corresponding size
    is adjusted to the other such that the aspect ratio of the original
    image is retained. *buffer* is an unsigned character array in
    scanline orientation with 8 pixels per byte. *nx*, *ny* refers to
    the number of pixels in the image. The rowlength of *buffer* must be
    an integral number of 8; pad with zeros. *buffer*\ [0] is upper left
    corner. You may replace the foreground color (the set bits) with the
    *f_rgb* color and the background color (the unset bits) with
    *b_rgb*. Alternatively, pass either color with the red component
    set to -1.0 and we will instead issue an image mask that is
    see-through for the specified fore- or background component. See the
    Adobe Systems PostScript Reference Manual for more details.

**long PSL_plotcolorimage** (**struct PSL_CTRL** *\*P*, **double**
*x*, **double** *y*, **double** *xsize*, **double** *ysize*, **int**
*justify*, **unsigned char** *\*buffer*, **long** *nx*, **long** *ny*,
**long** *depth*)

    Plots a 1-, 2-, 4-, 8-, or 24-bit deep image at plot coordinates
    (*x*, *y*) justified as per the argument *justify* (see
    **JUSTIFICATION** for details). The target size of the image is
    given by *xsize* and *ysize* in user units. If one of these is
    specified as zero, the corresponding size is adjusted to the other
    such that the aspect ratio of the original image is retained. This
    functions sets up a call to the PostScript colorimage or image
    operators. The pixel values are stored in *buffer*, an unsigned
    character array in scanline orientation with gray shade or r/g/b
    values (0-255). *buffer*\ [0] is the upper left corner. *depth* is
    number of bits per pixel (24, 8, 4, 2, or 1). *nx*, *ny* refers to
    the number of pixels in image. The rowlength of *buffer* must be an
    integral number of 8/\ *Idepth*. E.g. if *depth* = 4, then
    *buffer*\ [j]/16 gives shade for pixel[2j-1] and *buffer*\ [j%16
    (mod 16) gives shade for pixel[2j]. When *-depth* is passed instead
    then "hardware" interpolation of the image is requested (this is
    implementation dependent). If *-nx* is passed with 8- (or 24-) bit
    images then the first one (or three) bytes of *buffer* holds the
    gray (or r/g/b) color for pixels that are to be masked out using the
    PS Level 3 Color Mask method. See the Adobe Systems PostScript
    Reference Manual for more details.

**long PSL_plotepsimage** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **double** *xsize*, **double** *ysize*, **int**
*justify*, **unsigned char** *\*buffer*, **long** *size*, **long** *nx*,
**long** *ny*, **long** *ox*, **long** *oy*)

    Plots an Encapsulated PostScript (EPS) image at plot coordinates
    (*x*, *y*) justified as per the argument *justify* (see
    **JUSTIFICATION** for details). The target size of the image is
    given by *xsize* and *ysize* in user units. If one of these is
    specified as zero, the corresponding size is adjusted to the other
    such that the aspect ratio of the original image is retained. The
    EPS file is stored in *buffer* and has *size* bytes. This function
    simply includes the image in the PostScript output stream within
    an appropriate wrapper. Specify position of lower left corner and
    size of image. *nx*, *ny*, *ox*, *oy* refers to the width, height
    and origin (lower left corner) of the BoundingBox in points.

**long PSL_loadeps** (**struct PSL_CTRL** *\*P*, **char** *\*file*,
**struct imageinfo** *\*header*, **unsigned char** *\*\*image*)

    Reads the image contents of the EPS file given by the *file name*.
    The *header* is filled with dimensional information.  If *image*
    is NULL we return just with header, otherwise we read and return
    the entire EPS content via *image*.

Plotting Text
-------------

Here are functions used to read and plot text strings and paragraphs.
This can be somewhat complicated since we rely on the PostScript
interpreter to determine the exact dimensions of text items given the
font chosen. For perfect alignment you may have to resort to calculate
offsets explicitly using **long PSL_deftextdim**, **PSL_set_height**
and others and issue calculations with **PSL_setcommand**.

**long PSL_plottext** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **double** *fontsize*, **char** *\*text*, **double**
*angle*, **long** *justify*, **long** *mode*)

    The *text* is plotted starting at plot coordinates (*x*, *y*) and
    will make an *angle* with the horizontal. The point (*x*, *y*) maps
    onto different points of the text-string by giving various values
    for *justify* (see **JUSTIFICATION** for details). If *justify* is
    negative, then all leading and trailing blanks are stripped before
    plotting. Certain character sequences (flags) have special meaning
    to **PSL_plottext**. @~ toggles between current font and the
    Mathematical Symbols font. @%\ *no*\ % selects font *no* while @%%
    resets to the previous font. @- turns subscript on/off, @+ turns
    superscript on/off, @# turns small caps on/off, and @\\ will make a
    composite character of the following two character. @;\ *r/g/b*;
    changes the font color while @;; resets it [optionally append
    =\ *transparency* to change the transparency (0--100) of the text
    (the Default is opaque or 0)], @:\ *size*: changes the font size
    (@:: resets it), and @\_ toggles underline on/off. If *text* is NULL
    then we assume **PSL_plottextbox** was called first. Give
    *fontsize* in points. Normally, the text is typed using solid
    characters in the current color (set by **PSL_setcolor**). To draw
    outlined characters, set *mode* == 1; the outline will get the
    current color and the text is filled with the current fill color
    (set by **PSL_setfill**). Use *mode* == 2 if the current fill is a
    pattern. Use *mode* == 3 to achieve the same as *mode* == 1, while
    preventing the outline from obsuring any filled text font; the outline
    will hence be reduced to half the selected width. If the text is not
    filled, *mode* == 3 operates the same as *mode* == 1.
    If *fontsize* is negative it means that the current point
    has already been set before **PSL_plottext** was called and that
    (*x*, *y*) should be ignored.

**long PSL_plottextbox** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **double** *fontsize*, **char** *\*text*, **double**
*angle*, **long** *justify*, **double** *offset*\ [], **long** *mode*)

    This function is used in conjugation with **PSL_plottext** when a
    box surrounding the text string is desired. Taking most of the
    arguments of **PSL_plottext**, the user must also specify *mode* to
    indicate whether the box needs rounded (PSL_YES = 1) or straight
    (PSL_NO = 0) corners. The box will be colored with the current fill
    style set by **PSL_setfill**. That means, if an outline is desired,
    and the color of the inside of the box should be set with that
    routine. The outline will be drawn with the current pen color (and
    width). The *offset* array holds the horizontal and vertical
    distance gaps between text and the surrounding text box in distance
    units. The smaller of the two determined the radius of the rounded
    corners (if requested).

**long PSL_deftextdim** (**struct PSL_CTRL** *\*P*, **char**
*\*prefix*, **double** *fontsize*, **char** *\*text*)

    Computes the dimensions (width and height) required by the selected
    *text* given the current font and its *fontsize* (in points). The
    values are stored as PostScript variables called *prefix*\ \_w and
    *prefix*\ \_h, respectively. This function can be used to compute
    dimensions and, via BF(PSL_setcommand), calculate chances to
    position a particular item should be plotted. For instance, if you
    compute a position this way and wish to plot the text there, pass
    the coordinates to **PSL_plottext** as NaNs. If *prefix* is BF(-w),
    BF(-h), BF(-d) or BF(-b), no PostScript variables will be
    assigned, but the values of width, height, depth, or both width and
    height will be left on the PostScript stack.

**long PSL_setparagraph** (**struct PSL_CTRL** *\*P*, **double**
*line_space*, **double** *par_width*, **long** *par_just*)

    Initialize common settings to be used when typesetting paragraphs of
    text with **PSL_plotparagraph**. Specify the line spacing (1 equals
    the font size) and paragraph width (in distance units). Text can be
    aligned left (PSL_BL), centered (PSL_BC), right (PSL_BR), or
    justified (PSL_JUST) and is controlled by *par_just*.

**long PSL_plotparagraphbox** (**struct PSL_CTRL** *\*P*,
**double** *x*, **double** *y*, **double** *fontsize*, **char**
*\*text*, **double** *angle*, **long** *justify*, **double**
*offset*\ [], **long** *mode*)

    Computes and plots the text rectangle for a paragraph using the
    specified *fontsize* (in points). Here, *text* is an array of
    the text to be typeset, using the settings initialized by
    **PSL_setparagraph**. The escape sequences described for
    **PSL_plottext** can be used to modify the text. Separate text
    into several paragraphs by appending \\r to the last item in a
    paragraph. The whole text block is positioned at plot
    coordinates *x*, *y*, which is mapped to a point on the block
    specified by *justify* (see **JUSTIFICATION** for details). The
    whole block is then shifted by the amounts *shift*\ []. The box
    will be plotted using the current fill and outline settings. The
    *offset* array holds the horizontal and vertical distance gaps
    between text and the surrounding text box in distance units. Use
    *mode* to indicate whether the box should be straight
    (PSL_RECT_STRAIGHT = 0), rounded (PSL_RECT_ROUNDED = 1),
    convex (PSL_RECT_CONVEX = 2) or concave (PSL_RECT_CONCAVE = 3).

**long PSL_plotparagraph** (**struct PSL_CTRL** *\*P*, **double**
*x*, **double** *y*, **double** *fontsize*, **char** *\*text*,
**double** *angle*, **long** *justify*, **long** *mode*)

    Typesets paragraphs of text using the specified *fontsize* (in
    points). Here, *text* is an array of the text to be typeset,
    using the settings initialized by **PSL_setparagraph**. The
    escape sequences described for **PSL_plottext** can be used to
    modify the text. Separate text into several paragraphs by
    appending \\r to the last item in a paragraph. The whole text
    block is positioned at plot coordinates *x*, *y*, which is
    mapped to a point on the block specified by *justify* (see
    **JUSTIFICATION** for details). See **PSL_plotparagraphbox**
    for laying down the surrounding text rectangle first.

**long PSL_plottextline** (**struct PSL_CTRL** *\*P*, **double**
*\*xpath*, **double** *\*ypath*, **long** *\*np*, **long** *nseg*,
**void** *\*arg1*\, **void** *\*arg2*\, **char** *\*text*\ [],
**double** *angle*\ [], **long** *n_per_seg*\ [], **double** *fontsize,
**long** *justify*, **double** *offset*\ [], **long** *mode*)

    Please text along one or more path segments. The function does
    different things depending on the bit flags in *mode*. A key
    distinction occurs if the bit flag contains the bit PSL_TXT_CURVED
    (64) which means we wish to typeset the text along a variable and curved
    baseline given by the segments in *xpath, ypath*; otherwise we set
    straight text (possibly at an angle) and the *xpath, ypath* are
    not considered for text placement [If no line drawing is desired
    then these two arrays may be NULL].  We will describe the action
    taken for each bit value.  Multiple values may be passed at the
    same time and we processes from low to high bit.
    PSL_TXT_INIT: When mode contains this bit (1) we will initialize
    all the required variables and store them in the PostScript file.
    PSL_TXT_SHOW: We wish to see the text strings (otherwise they may
    only serve as guides to set up clip paths).
    PSL_TXT_CLIP_ON: Use the text and the paths to set up clip paths.
    PSL_TXT_DRAW: Draw the lines defined by the *xpath, ypath* arrays.
    PSL_TXT_CLIP_OFF: Turn the text path clipping off.
    We pass the text strings via *text*.  The locations of text plotting
    depends on whether PSL_TXT_CURVED is selected.  If it is then
    you must pass as *arg1* the *node* array indicating at which
    node in the *xpath, ypath* array the text will be plotted; let
    *arg2* be NULL. For
    straight baselines you must instead pass another set of x,y
    coordinates with the locations of the text label placements
    via *arg1, arg2*.
    Each label has its own entry in the
    *angle* array. The *text* is an array of text pointers to the
    individual text items. The
    *offset* array holds the x and y distance gaps between text and
    the surrounding text box in user units (the clip path is the
    combination of all these text boxes). Use *justify* to specify
    how the text string relates to the coordinates (see
    BF(JUSTIFICATION) for details).
    PSL_TXT_FILLBOX (128) will fill the text box (this requires you
    to first define the text box rgb color with **PSL_define_rgb**
    by setting a local PostScript variable that must be called PSL_setboxrgb).
    PSL_TXT_DRAWBOX (256) will draw the text box outlines (this requires
    you to first define the text box pen with **PSL_define_pen** by setting a local
    PostScript variable that must be called PSL_setboxpen). Before
    calling this function you must also initialize a PSL array for
    line pens and text fonts.

Clipping
--------

Here are functions used to activate and deactivate clipping regions.

**long PSL_beginclipping** (**struct PSL_CTRL** *\*P*, **double** *x*,
**double** *y*, **long** *n*, **double** *rgb*\ [], **long** *flag*)

    Sets up a user-definable clip path as a series on *n* points with
    plot coordinates (*x*, *y*). Plotting outside this polygon will be
    clipped until **PSL_endclipping** is called. If *rgb*\ [0] = -1 the
    inside of the path is left empty, otherwise it is filled with the
    specified color. *flag* is used to create complex clip paths
    consisting of several disconnected regions, and takes on values 0-3.
    *flag* = PSL_PEN_MOVE_ABS (1) means
    this is the first path in a multisegment clip path. *flag* =
    PSL_PEN_DRAW_ABS (2) means this is
    the last segment. Thus, for a single path, *flag* =
    PSL_PEN_DRAW_AND_STROKE_ABS (3).

**long PSL_endclipping** (**struct PSL_CTRL** *\*P*, **long** *mode*)

    Depending on the *mode* it restores the clip path. The *mode* values
    can be: -*n* will restore *n* levels of text-based clipping, *n*
    will restore *n* levels of polygon clipping, PSL_ALL_CLIP_TXT
    will undo all levels of text-based clipping, and PSL_ALL_CLIP_POL
    will undo all levels of polygon-based clipping.

`Miscellaneous Functions <#toc12>`_
-----------------------------------

Here are functions used to issue comments or to pass custom PostScript
commands directly to the output PostScript file. In C these functions
are declared as macros and they can accept a variable number of
arguments. However, from FORTRAN only a single text argument may be
passed.

**long PSL_setcommand** (**struct PSL_CTRL** *\*P*, **char** *\*text*)
    Writes a raw PostScript command to the PostScript output file,
    e.g., "1 setlinejoin.

**long PSL_comment** (**struct PSL_CTRL** *\*P*, **char** *\*text*)
    Writes a comment (*text*) to the PostScript output file, e.g.,
    "Start of graph 20. The comment are prefixed with with %% .

Authors
-------

Paul Wessel, School of Ocean and Earth Science and Technology,
`http://www.soest.hawaii.edu. <http://www.soest.hawaii.edu.>`_

Remko Scharroo, EUMETSAT, Darmstadt, Germany,
`http://www.eumetsat.int. <http://www.eumetsat.int.>`_

Bugs
----

Caveat Emptor: The authors are **not** responsible for any disasters,
suicide attempts, or ulcers caused by correct **or** incorrect use of
PSL. If you find bugs, please report them to the authors by
electronic mail. Be sure to provide enough detail so that we can
recreate the problem.

See Also
--------

:doc:`psconvert`

References
----------

Adobe Systems Inc., 1990, PostScript language reference manual, 2nd
edition, Addison-Wesley, (ISBN 0-201-18127-4).
