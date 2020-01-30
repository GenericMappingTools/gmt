.. index:: ! movie
.. include:: module_core_purpose.rst_

*****
movie
*****

|movie_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt movie** *mainscript*
|-C|\ *canvas*
|-N|\ *prefix*
|-T|\ *nframes*\|\ *min*/*max*/*inc*\ [**+n**]\|\ *timefile*\ [**+p**\ *width*]\ [**+s**\ *first*]\ [**+w**]
[ |-A|\ [**+l**\ [*n*]]\ [**+s**\ *stride*] ]
[ |-D|\ *displayrate* ]
[ |-E|\ *titlepage*\ [**+d**\ *duration*\ [**s**]][**+f**\ [**+i**\|\ **o**]\ *fade*\ [**s**]] ]
[ |-F|\ *format*\ [**+o**\ *options*]]
[ |-G|\ [*fill*]\ [**+p**\ *pen*] ]
[ |-H|\ *factor*]
[ |-I|\ *includefile* ]
[ |-K|\ [**+i**\|\ **o**]\ *fade*\ [**s**]\ [**+p**] ]
[ |-L|\ *labelinfo* ]
[ |-M|\ [*frame*],[*format*] ]
[ |-P|\ *progress* ]
[ |-Q|\ [**s**] ]
[ **-Sb**\ *background* ]
[ **-Sf**\ *foreground* ]
[ |SYN_OPT-V| ]
[ |-Z| ]
[ |-W|\ *workdir* ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

The **movie** module can generate GMT animation sequences using a single-plot script
that is repeated for all frames, with some variation using specific frame variables.  The
module simplifies (and hides) most of the steps normally needed to set up a full-blown
animation job.  Instead, the user can focus on composing the main frame plot and let the
parallel execution of frames and assembly of images into a movie take place in the background.
Individual frames are converted from PostScript plots to lossless, transparent PNG images and optionally
assembled into an animation (this last step requires external tools that must be present in
your path; see Technical Details below).  For opaque PNG images, simply specify a background
color via **-G**.

Required Arguments
------------------

*mainscript*
    Name of a stand-alone GMT modern mode script that makes the frame-dependent plot.  The
    script may access frame variables, such as frame number and others, and may be
    written using the Bourne shell (.sh), the Bourne again shell (.bash), the csh (.csh)
    or DOS batch language (.bat).  The script language is inferred from the file extension
    and we build hidden movie scripts using the same language.  Parameters that can be accessed
    are discussed below.

.. _-C:

**-C**\ *canvassize*
    Specify the canvas size used when composing the movie frames. You can choose from a
    set of preset formats or specify a custom layout.  The named 16:9 ratio
    formats have a canvas dimension of 24 x 13.5 cm *or* 9.6 x 5.4 inch and are
    (with pixel dimensions given in parenthesis):
    **4320p** (7680 x 4320), **2160p** (3840 x 2160), **1080p** (1920 x 1080), **720p** (1280 x 720),
    **540p** (960 x 540), **480p** (854 x 480), **360p** (640 x 360), and **240p** (426 x 240).
    We also accept **8k** or **uhd-2** to mean **4320p**, **4k** or **uhd** to mean **2160p**, and **hd** to mean **1080p**.
    The recognized 4:3 ratio formats have a canvas dimension of 24 x 18 cm *or* 9.6 x 7.2 inch
    and are (with pixel dimensions given in parenthesis):
    **uxga** (1600 x 1200), **sxga+** (1400 x 1050), **xga** (1024 x 768),
    **svga** (800 x 600), and **dvd** (640 x 480).
    Note: Your :term:`PROJ_LENGTH_UNIT` setting determines if **movie** sets
    you up to work with the SI or US canvas dimensions.  Instead of a named format you can
    request a custom format directly by giving *width*\ [*unit*]\ x\ *height*\ [*unit*]\ x\ *dpu*,
    where *dpu* is the dots-per-unit pixel density (pixel density is set automatically for the named formats).

.. _-N:

**-N**\ *prefix*
    Determines the name of the final movie file and a sub-directory with frame images (but see **-W**).
    Note: If the subdirectory exist then we exit immediately.  You are therefore required to remove any
    old directory by that name first.  This is done to prevent the accidental loss of valuable data.

.. _-T:

**-T**\ *nframes*\|\ *min*/*max*/*inc*\ [**+n**]\|\ *timefile*\ [**+p**\ *width*]\ [**+s**\ *first*]\ [**+w**]
    Either specify how many image frames to make, create a one-column data set width values from
    *min* to *max* every *inc* (append **+n** if *inc* is number of frames instead), or supply a file with a set of parameters,
    one record (i.e., row) per frame.  The values in the columns will be available to the
    *mainscript* as named variables **MOVIE_COL0**, **MOVIE_COL1**, etc., while any trailing text
    can be accessed via the variable **MOVIE_TEXT**.  Append **+w** to split the trailing
    string into individual words that can be accessed via variables **MOVIE_WORD0**, **MOVIE_WORD1**,
    etc. The number of records equals
    the number of frames. Note that the *background* script is allowed to create *timefile*,
    hence we check for its existence both before *and* after the background script has completed.  Normally,
    the frame numbering starts at 0; you can change this by appending a different starting frame
    number via **+s**\ *first*.  Note: All frames are still included; this modifier only affects
    the numbering of the given frames.  Finally, **+p** can be used to set the tag *width* of the format
    used in naming frames.  For instance, name_000010.png has a tag width of 6.  By default, this
    is automatically set but if you are splitting large jobs across several computers then you
    must use the same tag width for all names.


Optional Arguments
------------------

.. _-A:

**-A**\ [**+l**\ [*n*]]\ [**+s**\ *stride*]
    Build an animated GIF file.  You may specify if the movie should play more than once (i.e., loop)
    via **+l** and if so append how many times to repeat [infinite].  If a video product is also
    selected (**-F**) then you can limit the frames being used to make the GIF file.  Append **+s**\ *stride*
    to only use every *stride* frame, with *stride* being one of a fixed set of strides: 2, 5, 10,
    20, 50, 100, 200, and 500.

.. _-D:

**-D**\ *displayrate*
    Set the display frame rate in frames per seconds for the final animation [24].

.. _-E:

**-E**\ *titlepage*\ [**+d**\ *duration*\ [**s**]][**+f**\ [**i**\|\ **o**]\ *fade*\ [**s**]]
    Give *titlepage* script that creates a static title page for the movie [no title].
    Alternatively, *titlepage* can be a PostScript plot layer of dimensions exactly matching the cancas size.
    Control how long it should be displayed with **+d** in number of frames (append *s** for duration in seconds instead) [4s].
    Optionally, supply *fade* **i**\ n and **o**\ ut durations (in frames or seconds [1s]) as well [no fading].
    Fading affects the beginning and end of the title page *duration*.

.. _-F:

**-F**\ *format*\ [**+o**\ *options*]
    Set the format of the final video product.  Repeatable.  Choose either **mp4** (MPEG-4 movie) or
    **webm** (WebM movie).  You may optionally add additional ffmpeg encoding settings for this format
    via the **+o** modifier (in quotes if more than one word). If **none** is chosen then no PNGs will
    be created at all; this requires **-M**.

.. _-G:

**-G**\ [*fill*]\ [**+p**\ *pen*] :ref:`(more ...) <-Gfill_attrib>`
    Set the canvas color or fill before plotting commences [none].
    Optionally, append **+p** to draw the canvas outline with *pen* [no outline].

.. _-H:

**-H**\ *factor*
    Given the finite dots-per-unit used to rasterize PostScript frames to PNGs, the quantizing of features
    to discrete pixel will lead to rounding.  Some of this is mitigated by the anti-aliasing settings.  However,
    changes from frame to frame is outside the control of the individual frame rasterization and we
    find that, in particular, moving text may appear jittery when seen in the final animation.  You can mitigate
    this effect by selecting a scale *factor* that, in effect, temporarily increases the effective dots-per-unit
    by *factor*, rasterizes the frame, then downsamples the image by the same factor at the end.  The larger
    the *factor*, the smoother the transitions.  Because processing time increases with *factor* we suggest you
    try values in the 2-5 range.  Note that images can also suffer from quantizing when the original data have
    much higher resolution than your final frame pixel dimensions.  The **-H** option may then be used to smooth the
    result to avoid aliasing [no downsampling].  This effect is called `sub-pixel rendering <https://en.wikipedia.org/wiki/Subpixel_rendering>`.

.. _-I:

**-I**\ *includefile*
    Insert the contents of *includefile* into the movie_init.sh script that is accessed by all movie scripts.
    This mechanism is used to add information (typically constant variable assignments) that the *mainscript*
    and any optional **-S** scripts rely on.

.. _-K:


**-K**\ [**+i**\|\ **o**]\ *fade*\ [**s**]\ [**+p**]
    Add fading in and out for the main animation sequence [no fading]. Append
    the length of the fading in number of frames (or seconds by appending **s**) [1s].
    For different lengths of fading in and out you can repeat the **-K** option
    by appending the **i** or **o** directives.  Normally, fading will affect the
    first and last animation frames.  Append **+p** to preserve these by instead
    fading in and out on only the first and last (repeated) animation frames.

.. _-L:

**-L**\ *labelinfo*
    Automatic labeling of individual frames.  Repeatable up to 32 labels.  Places the chosen label at the frame perimeter:
    **e** selects the elapsed time in seconds as the label; append **+s**\ *scale* to set the length
    in seconds of each frame [Default is 1/*framerate*],
    **s**\ *string* uses the fixed text *string* as the label,
    **f** selects the running frame number as the label, **p** selects the percentage of progress so far,
    **c**\ *col* uses the value in column
    number *col* of *timefile* as label (first column is 0), while **t**\ *col* uses word number
    *col* from the trailing text in *timefile* (first word is 0).  Note: If you use **-Lc**
    with an absolute time column, then the format of the timestamp will depend on the two default settings
    :term:`FORMAT_DATE_MAP` and :term:`FORMAT_CLOCK_MAP`.  By default,
    both *date* and *time* are displayed (with a space between); set one of the settings to "-" to skip that component.
    Append **+c**\ *dx*\ [/*dy*] for the clearance between label and bounding box; only
    used if **+g** or **+p** are set.  Append units **c**\|\ **i**\|\ **p** or % of the font size [15%].
    Append **+f** to use a specific *font* [:term:`FONT_TAG`].
    Append **+g** to fill the label bounding box with *fill* color [no fill].
    Use **+j**\ *refpoint* to specify where the label should be plotted [TL].
    Append **+o**\ *dx*\ [/*dy*] to offset label in direction implied by *justify*. Append units
    **c**\|\ **i**\|\ **p** or % of the font size [20% of font size].
    Append **+p** to draw the outline of the bounding box using selected *pen* [no outline].
    Append **+t** to provide a *format* statement to be used with the label item selected [no special formatting].
    If **-Lt** is used then the format statement must contain a %s-like format, else it may have an integer (%d)
    or floating point  (%e, %f, %g) format specification.

.. _-M:

**-M**\ [*frame*],[*format*]
    In addition to making the animation sequence, select a single master frame [0] for a cover page.  The master frame will
    be written to the current directory with name *prefix.format*, where *format* can one of the
    graphics extensions from the allowable graphics :ref:`formats <tbl-formats>` [pdf].

.. _-P:

**-P**\ *progress*
    Automatic placement of progress indicator(s). Repeatable up to 32 indicators.  Places the chosen indicator at the frame perimeter.
    Select from six indicators called a-f [a].  Indicators a-c are different types of circular indicators while d-f are
    linear (axis-like) indicators.  Specify dimension of the indicator with **+w**\ *width* [5% of max canvas dimension for
    circular indicators and 60% of relevant canvas dimension for the linear indicators] and placement via **+j**\ *justify*
    [TR for circular and BC for axes]. Indicators b-f can optionally add annotations if modifier **+a** is used, append one of
    **e**\|\ **f**\|\ **p**\|\ **s**\|\ **c**\ *col* \|\ **t**\ *col* to indicate what should be annotated (see **-L**
    for more information on what these are); append **+f** to use a specific *font* [:term:`FONT_ANNOT_SECONDARY` scaled as needed].
    Append **+o**\ *dx*\ [/*dy*] to offset indicator in direction implied by *justify*.  Append **+g** to set moving item *fill* color [see below for defaults].
    Use **+p**\ *pen* to set moving item *pen*.  For corresponding static fill and pen, use **+G** and **+P** instead.

.. _-Q:

**-Q**\ [**s**]
    Debugging: Leave all files and directories we create behind for inspection.  Alternatively, append **s** to
    only build the movie scripts but not perform any execution.  One exception involves the optional
    background script derived from **-Sb** which is always executed since it may produce data needed when
    building the movie scripts.

.. _-Sb:

**-Sb**\ *background*
    The optional GMT modern mode *background* (written in the same scripting language as *mainscript*) can be
    used for one or two purposes: (1) It may create files (such as *timefile*) that will be needed by *mainscript*
    to make the movie, and (2) It may make a static background plot that should form the background for all frames.
    If a plot is generated the script must make sure it uses the same positioning (i.e., **-X -Y**) as the main script
    so that the layered plot will stack correctly (unless you actually want a different offset).  Alternatively,
    *background* can be a PostScript plot layer of dimensions exactly matching the cancas size.

.. _-Sf:

**-Sf**\ *foreground*
    The optional GMT modern mode *foreground* (written in the same scripting language as *mainscript*) can be
    used to make a static foreground plot that should be overlain on all frames.  Make sure the script uses the same
    positioning (i.e., **-X -Y**) as the main script so that the layers will stack correctly.  Alternatively,
    *foreground* can be a PostScript plot layer of dimensions exactly matching the cancas size.

.. _movie-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *workdir*
    By default, all temporary files and frame PNG file are built in the subdirectory *prefix* set via **-N**.
    You can override that selection by giving another *workdir* as a relative or full directory path.

.. _-Z:

**-Z**
    Erase the entire *prefix* directory after assembling the final movie [Default leaves directory with all images;
    the script files, parameter files, and layer PostScript files are all removed (but see **-Q**)].

.. _-cores:

**-x**\ [[-]\ *n*]
    Limit the number of cores used when making the individual frames.
    By default we try to use all available cores.  Append *n* to only use *n* cores
    (if too large it will be truncated to the maximum cores available).  Finally,
    give a negative *n* to select (all - *n*) cores (or at least 1 if *n* equals or exceeds all).
    The parallel processing does not depend on OpenMP.

.. include:: explain_help.rst_

Parameters
----------

Several parameters are automatically assigned and can be used when composing *mainscript* and the optional
*background* and *foreground* scripts. There are two sets of parameters: Those that are constants
and those that change with the frame number.  The constants are accessible by all the scripts:
**MOVIE_WIDTH**\ : The width of the canvas,
**MOVIE_HEIGHT**\ : The height of the canvas,
**MOVIE_DPU**\ : The current dots-per-unit,
**MOVIE_RATE**\ : The current number of frames per second,
**MOVIE_NFRAMES**\ : The total number of frames.
Also, if **-I** was used then any static parameters listed there will be available to all the scripts as well.
In addition, the *mainscript* also has access to parameters that vary with the frame counter:
**MOVIE_FRAME**\ : The current frame number (an integer, e.g., 136),
**MOVIE_TAG**\ : The formatted frame number (a string, e.g., 000136), and
**MOVIE_NAME**\ : The name prefix for the current frame (i.e., *prefix*\ _\ **MOVIE_TAG**),
Furthermore, if a *timefile* was given then variables **MOVIE_COL0**\ , **MOVIE_COL1**\ , etc. are
also set, yielding one variable per column in *timefile*.  If *timefile* has trailing text then that text can
be accessed via the variable **MOVIE_TEXT**, and if word-splitting was explicitly requested by **-T+w** or
implicitly by selecting word labels in **-F** or **-P**) then
the trailing text is also split into individual word parameters **MOVIE_WORD0**\ , **MOVIE_WORD1**\ , etc.

Data Files
----------

The movie scripts will be able to find any files present in the starting directory when **movie** was initiated,
as well as any new files produced by *mainscript* or the optional scripts set via **-S**.
No path specification is needed to access these files.  Other files may
require full paths unless their directories were already included in the :term:`DIR_DATA` setting.

Your Canvas
-----------

As you can see from **-C**, unless you specified a custom format you are given a canvas size that is either 24 x 13.5 cm (16:9)
or 24 x 18 cm (4:3).  If your :term:`PROJ_LENGTH_UNIT` setting is inch then the custom canvas sizes are just
slightly (1.6%) larger than the corresponding SI sizes (9.6 x 5.4" or 9.6 x 7.2"); this has no effect on the size of the movie
frames but allow us to use good sizes that work well with the dpu chosen.  You should compose your plots using
the given canvas size, and **movie** will make proper conversions of the canvas to image pixel dimensions. It is your responsibility
to use **-X -Y** to allow for suitable margins and any positioning of items on the canvas.  To minimize processing time it is
recommended that any static part of the movie be considered either a static background (to be made once by *background*) and/or
a static foreground (to be made once by *foreground*); **movie** will then assemble these layers per frame.  Also, any computation of
static data files to be used in the loop over frames can be produced by *background*.  Any data or variables that depend on the
frame number must be computed or set by *mainscript* or provided via the parameters as discussed above.

External PostScript Layers
--------------------------

Instead of passing GMT modern scripts to **-S** you can alternatively provide the name of PostScript
plot layer files. Note that these must exactly match the canvas size.  As a simple example, if you are
making a HD movie using the US unit dimensions then a background pink layer would be created by::

    gmt basemap -R0/9.6/0/5.4 -Jx1i -B+gpink -X0 -Y0 --PS_MEDIA=9.6ix5.4i -ps background

Note the canvas selection via :term:`PS_MEDIA`, the matching region and projection, and
the zero location of the origin.

Technical Details
-----------------

The **movie** module creates several hidden script files that are used in
the generation of the images (here we have left the file extension off since it depends on the
scripting language used): *movie_init* (initializes variables related to canvas size and dots-per-unit,
and includes the contents of the optional *includefile*), *movie_preflight* (optional since it derives
from **-Sb** and computes needed data files and possibly a background layer), *movie_postflight*
(optional since it derives from **-Sf** and builds a foreground layer), *movie_frame* (accepts a frame counter
argument and builds the frame image), and *movie_cleanup* (removes temporary files at the end of the
run). For each frame there is a separate *movie_params_######* script that provides frame-specific
variables (e.g., frame number and anything given via **-T**).  The pre- and post-flight scripts have
access to the information in *movie_init* while the frame script in addition has access to the frame-
specific parameter file.  Using the **-Q** option will just produce these scripts which you can then examine.

The conversion of PNG frames to an animated GIF (**-F**\ gif) relies on `GraphicsMagick <http://www.graphicsmagick.org/>`_.
Thus, **gm** must be accessible via your standard search path. Likewise, the conversion of
PNG frames to an MP4 (**-F**\ mp4) or WebM (**-F**\ webm) movie relies on `FFmpeg <https://www.ffmpeg.org/>`_.

Hints for Movie Makers
----------------------

Composing movies is relatively simple but you have to think in terms of variables.
Examine the examples we have described.  Then, start by making a single plot script (your *mainscript*) and identify which
things should change with time (i.e., with the frame number).  Create variables for these values. If they
are among the listed parameters that **movie** creates then use those names.  Unless you only
require the frame number you will need to make a file that you can pass to **-T**.  This file should
then have all the values you need, per frame (i.e., row), with values across all the columns you need.
If you need to assign various fixed variables that do not change with time then your *mainscript*
will look shorter and cleaner if you offload those assignments to a separate *includefile* (**-I**).
To test your movie, start by using options **-F**\ none **-Q -M** to ensure your master frame page looks correct.
This page shows you one frame of your movie (you can select which frame via the **-M** arguments).  Fix any
issues with your use of variables and options until this works.  You can then try to remove **-Q**.
We recommend you make a very short (i.e., **-T**) and small (i.e., **-C**) movie so you don't have to wait very
long to see the result.  Once things are working you can beef up number of frames and movie quality.

Color Table Usage
-----------------

Because **movie** launches individual frame plots as separate sessions running in parallel, we cannot
utilize the current CPT (i.e., the last CPT created directly by :doc:`makecpt` or :doc:`grd2cpt`, or
indirectly by :doc:`grdimage` or  :doc:`grdview`).  Instead, you must create CPTs using explicit
files and pass those names to the modules that require CPT information.  In modern mode, this means
you need to use the **-H** option in :doc:`makecpt` or :doc:`grd2cpt` in order to redirect their output
to named files.

Progress Indicators
-------------------

.. figure:: /_images/GMT_movie_progress.*
   :width: 500 px
   :align: center

   The six types of movie progress indicators.  All have default sizes, placements, colors and pens (shown)
   but these can be overridden by the corresponding modifiers (see below).

The letters a-f select one of the six progress indicators shown above.
Indicator a) needs a static [lightgreen] and moving [lightred]
*fill* (set via **+G** and **+g**); there is no label option.
Indicator b) takes a static [lightblue] and moving [blue] *pen* (set via **+P** and **+p**),
and if **+a** is set we place a centered label with a font size scaled to 30%
of indicator size (unless **+f** was set which is used as given).
Indicator c) takes a static [dashed darkred, pen width is 1% of indicator size] and moving [red]
*pen* (default pen width is 5% of indicator size) for a circular arrow (head size is 20% of indicator size), with a central
label (if given **+a**) with a font size 30% of indicator size (unless **+f** was set which we will honor).
Indicator d) takes a static [black] and moving [yellow, width 0.5% of length] *pen* for a rounded line with a cross-mark. If
label is requested (**+a**) we use a font size that is twice the static pen thickness (unless **+f** was set).
Indicator e) takes a static [red] and moving [lightgreen] *pen*. If labels are requested (**+a**) we
use a font size that is twice the static pen thickness (unless **+f** was set).
Finally, indicator f) takes a *pen* for the static axis [black] and a *fill* for the moving triangle [red];
the triangle size is scaled to twice the axis width (see below), and a font size scaled to thrice the axis width.
Note for indicators d-f: If percentage labels are selected (**+ap**), then the axes display a unit label,
otherwise no unit label is supplied.  The indicators d-f are horizontal for all *justify* codes except for **ML** and **MR**.
The default pen thickness for the linear static lines is the smallest of 2.5% of their lengths and 8p (1.5% and 3p for f).
If no size is specified (**+w**) then we default to 5% of cancas width for the three circular indicators and
60% of the relevant canvas dimension for the linear indicators.

Title Sequence and Fading
-------------------------

.. figure:: /_images/GMT_title_fade.*
   :width: 500 px
   :align: center

   The fade-level (0 means black, 100 means normal visibility) for the complete movie, including
   an optional title sequence.

The complete movie may have a leading title sequence (**-E**) of given *duration*. A short section
at the beginning and end may be designated to fade in/out via black.  The main animation
sequence may also have fade in/out (**-K**). Here, you can choose to fade in/out during the beginning and end section of
the animation or you can "freeze" the first and last animation frame and only fade in/out using
those static images (via modifier **+p** to preserve the whole animation sequence).

Examples
--------

To make an animated GIF movie based on the script globe.sh, which simply spins a globe using the
frame number to serve as the view longitude, using a custom square 600x600 pixel canvas and 360 frames,
place a frame counter in the top left corner, and place a progress indicator in the top right corner, try::

    gmt movie globe.sh -Nglobe -T360 -Agif -C6ix6ix100 -Lf -P

Here, the globe.sh bash script simply plots a map with :doc:`coast` but uses the frame number variable
as the center longitude::

    gmt begin
       gmt coast -Rg -JG${MOVIE_FRAME}/20/${MOVIE_WIDTH} -Gmaroon -Sturquoise -Bg -X0 -Y0
    gmt end

As the automatic frame loop is executed the different frames will be produced with different
longitudes.  The equivalent DOS batch script setup would be::

    gmt movie globe.bat -Nglobe -T360 -Agif -C6ix6ix100 -Lf -P

Now, the globe.bat DOS script is simply::

    gmt begin
       gmt coast -Rg -JG%MOVIE_FRAME%/20/%MOVIE_WIDTH% -Gmaroon -Sturquoise -Bg -X0 -Y0
    gmt end

i.e., the syntax of how variables are used vary according to the scripting language. At the
end of the execution we find the animated GIF globe.gif and a directory (called globe) that contains all 360 PNG images.
Note that there is no information in the globe scripts that reflects the name of the plot, the canvas size,
the dimensions of the rasterized PostScript, and so on.  That information is hidden from the user;
the actual movie scripts that execute are derived from the user-provided scripts and supply
the extra machinery. The **movie** module automatically manages the parallel execution loop over all frames using
all available cores.

Longer Examples
---------------

To explore more elaborate movies, see the Animations examples under our :doc:`Gallery <gallery>`.

Other Movie Formats
-------------------

As configured, **movie** only offers the MP4 and WebM formats for movies.  The conversion is performed by the
tool `FFmpeg <https://www.ffmpeg.org/>`_, which has more codecs and processing options than there are children in China.
If you wish to run ffmpeg with other options, select mp4 and run **movie** with verbose information on (**-Vi**).
At the end it will print the ffmpeg command used.  You can copy, paste, and modify this command to
select other codecs, bit-rates, and arguments.  You can also use the PNG sequence as input to tools such
as QuickTime Pro, iMovie, MovieMaker, and similar commercial programs to make a movie that way.

Manipulating Multiple Movies
----------------------------

If you are making a series of similar movies, you can use ffmpeg to paste and stitch them into a single movie.
Assume we have four movies called movie_1.mp4, movie_2.mp4, movie_3.mp4, and movie_4.mp4, and you wish to combine
them into a 2x2 panel showing the four movies simultaneously.  You would first combine movies (1,2) and (3,4)
horizontally, then combine the two resulting strips vertically::

    ffmpeg -i movie_1.mp4 -i movie_2.mp4 -filter_complex hstack=inputs=2 top.mp4
    ffmpeg -i movie_3.mp4 -i movie_4.mp4 -filter_complex hstack=inputs=2 bottom.mp4
    ffmpeg -i top.mp4 -i bottom.mp4 -filter_complex vstack=inputs=2 four_movies.mp4

For more information on such manipulations, see the FFmpeg documentation.

See Also
--------

:doc:`gmt`,
:doc:`events`,
:doc:`psconvert`
