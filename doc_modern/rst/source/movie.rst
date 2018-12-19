.. index:: ! movie

*****
movie
*****

.. only:: not man

    Create animation sequences and movies

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt movie** *mainscript*
|-C|\ *canvas*
|-N|\ *prefix*
|-T|\ *frames*\ \|\ *timefile*\ [**+p**\ *width*]\ [**+s**\ *first*]\ [**+w**]
[ |-A|\ [**+l**\ [*n*]]\ [**+s**\ *stride*] ] 
[ |-D|\ *displayrate*
[ |-F|\ *format*\ [**+o**\ *options*\ ]]
[ |-G|\ *fill*\ ]
[ |-H|\ *factor*\ ]
[ |-I|\ *includefile* ]
[ |-L|\ *labelinfo* ]
[ |-M|\ [*frame*],[*format*] ]
[ |-Q|\ [**s**] ]
[ **Sb**\ *backgroundscript* ]
[ **Sf**\ *foregroundscript* ]
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
Individual frames are converted from PostScript plots to lossless PNG images and optionally
assembled into an animation (this step requires external tools that must be present in
your path; see Technical Details below).

Required Arguments
------------------

*mainscript*
    Name of a stand-alone GMT modern script that makes the frame-dependent plot.  The
    script may access frame variables, such as frame number and others, and may be
    written using the Bourne shell (.sh), the Bourne again shell (.bash), the csh (.csh)
    or DOS batch language (.bat).  The script language is inferred from the file extension
    and we build hidden movie scripts using the same language.  Parameters that can be accessed
    are discussed below.

.. _-C:

**-C**\ *papersize*
    Specify the canvas size used when composing the movie frames. You can choose from a
    set of known preset formats or you can set a custom layout.  The named 16:9 ratio
    formats have a canvas dimension of 24 x 13.5 cm *or* 9.6 x 5.4 inch and are
    (with pixel dimensions given in parenthesis):
    **4320p** (7680 x 4320), **2160p** (3840 x 2160), **1080p** (1920 x 1080), **720p** (1280 x 720),
    **540p** (960 x 540), **480p** (854 x 480), **360p** (640 x 360), and **240p** (426 x 240).
    We also accept **8k** to mean **4320p**, **4k** or **uhd** to mean **2160p** and **hd** to mean **1080p**.
    The recognized 4:3 ratio formats have a canvas dimension of 24 x 18 cm *or* 9.6 x 7.2 inch
    and are (with pixel dimensions given in parenthesis):
    **uxga** (1600 x 1200), **sxga+** (1400 x 1050), **xga** (1024 x 768),
    **svga** (800 x 600), and **dvd** (640 x 480).
    Note: Your :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>` setting determines if movie sets
    you up to work with the SI or US canvas dimensions.  Instead of a named format you can
    request a custom format directly by giving *width*\ [*unit*]\ x\ *height*\ [*unit*]\ x\ *dpu*,
    where *dpu* is the dots-per-unit pixel density.

.. _-N:

**-N**\ *prefix*
    Determines the name of a sub-directory with frame images as well as the final movie file.
    Note: If the subdirectory exist then we exit immediately.  You are therefore required to remove any
    old directory by that name first.  This is done to prevent the accidental loss of valuable data.

.. _-T:

**-T**\ *frames*\ \|\ *timefile*\ [**+p**\ *width*]\ [**+s**\ *first*]\ [**+w**]
    Either specify how many image frames to make or supply a file with a set of parameters,
    one record per frame (i.e., row).  The values in the columns will be available to the
    *mainscript* as named variables **MOVIE_COL1**, **MOVIE_COL2**, etc., while any trailing text
    can be accessed via the variable **MOVIE_TEXT**.  Append **+w** to also split the trailing
    string into individual words that can be accessed via **MOVIE_WORD1**, **MOVIE_WORD2**, etc. The number of records equals
    the number of frames. Note that the *background* script is allowed to create the *timefile*,
    hence we check of its existence both before and after the background script has run.  Normally,
    the frame numbering starts at 0; you can change this by appending a different starting frame
    number via **+s**\ *first*.  Note: All frames are still included; this modifier only affects
    the numbering of the given frames.  Finally, **+p** can be used to set the tag *width* of the format
    used in naming frames.  For instance, name_000010.png has a tag width of 6.  By default, this
    is automatically set but if you are splitting large jobs across several computers then you
    will want to have the same tag width for all names [automatic].


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

.. _-F:

**-F**\ *format* [**+o**\ *options*\ ]
    Set the format of the final video product.  Repeatable.  Choose either **mp4** (MPEG-4 movie) or
    **webm** (WebM movie).  You may optionally add additional ffmpeg encoding settings for this format
    via the **+o** modifier (in quotes if more than one word). If **none** is chosen then no PNGs will
    be created at all; this requires **-M**.

.. _-G:

**-G**\ *fill*
    Set the canvas color or fill before plotting commences [none].

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
    result to avoid aliasing [no downsampling].

.. _-I:

**-I**\ *includefile*
    Insert the contents of *includefile* into the movie_init.sh script that is accessed by all movie scripts.
    This mechanism is used to add information (typically constant variable assignments) that the *mainscript*
    and any optional **-S** scripts rely on.

.. _-L:

**-L**\ *labelinfo*

    Automatic labeling of individual frames.  Repeatable.  Places the chosen label at the frame perimeter:
    **e** selects the elapsed time in seconds as the label; append **+s**\ *scale* to set the length
    in seconds of each frame [Default is 1/*framerate*],
    **f** selects the running frame number as the label, **c**\ *col* uses the value in column
    number *col* of *timefile* as label (first column is 1), while **t**\ *col* uses word number
    *col* from the trailing text in *timefile* (requires **-T**\ ...\ **+w**).
    The label font is controlled via :ref:`FONT_TAG <FONT_TAG>`.
    Append **+c**\ *dx*\ [/*dy*] for the clearance between label and bounding box; only
    used if **+g** or **+p** are set.  Append units **c**\ \|\ **i**\ \|\ **p** or % of the font size [15%].
    Append **+f** to use a specific *font* [:ref:`FONT_TAG <FONT_TAG>`].
    Append **+g** to fill the label bounding box with *fill* color [no fill].
    Use **+j**\ *refpoint* to specify where the label should be plotted [TL].
    Append **+o**\ *dx*\ [/*dy*] to offset label in direction implied by *justify*. Append units
    **c**\ \|\ **i**\ \|\ **p** or % of the font size [20% of font size].
    Append **+p** to draw the outline of the bounding box using selected *pen* [no outline].
    Append **+t** to provide a *format* statement to be used with the label item selected [no special formatting].
    If **-Lt** is used then the format statement must contain a %s-like format, else it may have an integer (%d)
    or floating point  (%e, %f, %g) format specification.

.. _-M:

**-M**\ [*frame*],[*format*]
    In addition to making the animation sequence, select a single frame for a cover page.  This frame will
    be written to the current directory with name *prefix.format*, where *format* can one of the
    graphics extensions from the allowable graphics :ref:`formats <tbl-formats>` [pdf].

.. _-Q:

**-Q**\ [**s**]
    Debugging: Leave all files and directories we create behind for inspection.  Alternatively, append **s** to
    only build the movie scripts but not perform any execution.  One exception involves the optional
    background script derived from **-Sb** which is always executed since it may produce data needed when
    building the movie scripts.

.. _-Sb:

**-Sb**\ *backgroundscript*
    The optional GMT modern mode *backgroundscript* (written in the same scripting language as *mainscript*) can be
    used for one or two purposes: (1) It may create files (such as *timefile*) that will be needed by *mainscript*
    to make the movie, and (2) It may make a static background plot that should form the background for all frames.
    If a plot is generated the script must make sure it uses the same positioning (i.e., **-X -Y**) as the main script
    so that the layered plot will stack correctly (unless you actually want a different offset).

.. _-Sf:

**-Sf**\ *foregroundscript*
    The optional GMT modern mode *foregroundscript* (written in the same scripting language as *mainscript*) can be
    used to make a static foreground plot that should be overlain on all frames.  Make sure the script uses the same
    positioning (i.e., **-X -Y**) as the main script so that the layers will stack correctly.

.. _movie-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *workdir*
    In addition to the current directory, the *prefix* directory, and any directories specified via the
    GMT defaults setting :ref:`DIR_DATA <DIR_DATA>`, add *workdir* as a place to scan for data files needed by the scripts.

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

.. include:: explain_help.rst_

Parameters
----------

Several parameters are automatically assigned and can be used when composing *mainscript* and the optional
*backgroundscript* and *foregroundscript* scripts. There are two sets of parameters: Those that are constants
and those that change with the frame number.  The constants are accessible by all the scripts:
**MOVIE_WIDTH**\ : The width of the canvas,
**MOVIE_HEIGHT**\ : The height of the canvas,
**MOVIE_DPU**\ : The current dots-per-unit,
**MOVIE_RATE**\ : The current number of frames per second,
**MOVIE_NFRAMES**\ : The total number of frames.
Also, if **-I** was used then any static parameters listed there will be available to all the scripts as well.
In addition, the *mainscript* also has access to parameters that vary with the frame counter:
**MOVIE_FRAME**\ : The current frame number (an integer),
**MOVIE_TAG**\ : The formatted frame number (a string, e.g., 000136), and
**MOVIE_NAME**\ : The name prefix for the current frame (i.e., *prefix*\ _\ **MOVIE_TAG**),
Furthermore, if a *timefile* was given then variables **MOVIE_COL1**\ , **MOVIE_COL2**\ , etc. are
also set, yielding one variable per column in *timefile*.  If *timefile* has trailing text then that text can
be accessed via the variable **MOVIE_TEXT**, and if word-splitting was requested in **-T** with the **+w** modifier then
the trailing text is also split into individual word parameters **MOVIE_WORD1**\ , **MOVIE_WORD2**\ , etc.

Data Files
----------

The movie scripts will be able to find any files present in the starting directory when **movie** was initiated,
as well as any new files produced by *mainscript* or the optional scripts set via **-S**.
No path specification is needed to access these files.  Other files may
require full paths unless their directories were already included in the :ref:`DIR_DATA <DIR_DATA>` setting
or specified via **-W**.

Your Canvas
-----------

As you can see from **-C**, unless you specified a custom format you are given a canvas size that is either 24 x 13.5 cm (16:9)
or 24 x 18 cm (4:3).  If your :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>` setting is inch then the custom canvas sizes are just
slightly (1.6%) larger than the corresponding SI sizes (9.6 x 5.4" or 9.6 x 7.2"); this has no effect on the size of the movie
frames but allow us to use good sizes that work well with the dpu chosen.  You should compose your plots using
the given canvas size, and movie will make proper conversions of the canvas to image pixel dimensions. It is your responsibility
to use **-X -Y** to allow for suitable margins and any positioning of items on the canvas.  To minimize processing time it is
recommended that any static part of the movie be considered either a static background (to be made once by *backgroundscript*) and/or
a static foreground (to be made once by *foregroundscript*); **movie** will then assemble these layers per frame.  Also, any computation of
static data files to be used in the loop over frames can be produced by *backgroundscript*.  Any data or variables that depend on the
frame number must be computed or set by *mainscript* or provided via the parameters as discussed above.

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

The conversion of PNG frames to an animated GIF (**-F**\ gif) relies on GraphicsMagick (http://www.graphicsmagick.org). 
Thus, "gm" must be accessible via your standard search path. Likewise, the conversion of
PNG frames to an MP4 movie (**-F**\ mp4) or WebM movie (**-F**\ webm) relies on ffmpeg (https://www.ffmpeg.org). 

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
To test your movie, start by using options **-Q -M** to ensure your cover page looks correct.
This page shows you one frame of your movie (you can select which frame via the **-M** arguments).  Fix any
issues with your use of variables and options until this works.  You can then try to remove **-Q**.
We recommend you make a very short (i.e., **-T**) and small (i.e., **-C**) movie so you don't have to wait very
long to see the result.  Once things are working you can beef up number of frames and movie
quality.

Examples
--------

To make an animated GIF movie based on the script globe.sh, which simply spins a globe using the
frame number to serve as the view longitude, using a custom square 600x600 pixel canvas and 360 frames, try

   ::

    gmt movie globe.sh -Nglobe -T360 -Agif -C6ix6ix100

Here, the globe.sh bash script simply plots a map with :doc:`coast` but uses the frame number variable
as the center longitude:

   ::

    gmt begin
       gmt coast -Rg -JG${MOVIE_FRAME}/20/${MOVIE_WIDTH} -Gmaroon -Sturquoise -Bg -X0 -Y0
    gmt end

As the automatic frame loop is executed the different frames will be produced with different
longitudes.  The equivalent DOS batch script setup would be

  ::

    gmt movie globe.bat -Nglobe -T360 -Agif -C6ix6ix100

Now, the globe.bat DOS script is simply

   ::

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

To explore more elaborate movies, see the Animations examples under our Gallery.

Other Movie Formats
-------------------

As configured, movie only offers the MP4 and WebM formats for movies.  The conversion is performed by the
tool ffmpeg (https://www.ffmpeg.org), which has more codecs and processing options than there are children in China.
If you wish to run ffmpeg with other options, select mp4 and run movie with long verbose (**-Vl**).
At the end it will print the ffmpeg command used.  You can copy, paste, and modify this command to
select other codecs, bit-rates, and arguments.  You can also use the PNG sequence as input to tools such
as QuickTime Pro, iMovie, MovieMaker, and similar commercial programs to make a movie that way.

See Also
--------

:doc:`gmt`,
:doc:`psconvert`
