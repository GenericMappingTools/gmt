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
|-T|\ *nframes*\|\ *min*/*max*/*inc*\ [**+n**]\|\ *timefile*\ [**+p**\ *width*]\ [**+s**\ *first*]\ [**+w**\ [*str*]]
[ |-A|\ [**+l**\ [*n*]]\ [**+s**\ *stride*] ]
[ |-D|\ *displayrate* ]
[ |-E|\ *titlepage*\ [**+d**\ *duration*\ [**s**]][**+f**\ [**i**\|\ **o**]\ *fade*\ [**s**]]\ [**+g**\ *fill*] ]
[ |-F|\ *format*\ [**+t**]\ [**+o**\ *options*]]
[ |-G|\ [*fill*]\ [**+p**\ *pen*] ]
[ |-H|\ *factor*]
[ |-I|\ *includefile* ]
[ |-K|\ [**+f**\ [**i**\|\ **o**]\ *fade*\ [**s**]]\ [**+g**\ *fill*]\ [**+p**] ]
[ |-L|\ *labelinfo* ]
[ |-M|\ [*frame*],[*format*] ]
[ |-P|\ *progress* ]
[ |-Q|\ [**s**] ]
[ **-Sb**\ *background* ]
[ **-Sf**\ *foreground* ]
[ |SYN_OPT-V| ]
[ |-Z|\ [**s**] ]
[ |-W|\ [*workdir*] ]
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
Individual frames are converted from *PostScript* plots to lossless, transparent PNG images and optionally
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
    **Note**: Your :term:`PROJ_LENGTH_UNIT` setting determines if **movie** sets
    you up to work with the SI or US canvas dimensions.  Instead of a named format you can
    request a custom format directly by giving *width*\ x\ *height*\ x\ *dpu*,
    where *dpu* is the dots-per-unit pixel density (pixel density is set automatically for the named formats).

.. _-N:

**-N**\ *prefix*
    Determines the name of the final movie file and a sub-directory with frame images (but see **-W**).
    **Note**: If the subdirectory exist then we exit immediately.  You are therefore required to remove any
    old directory by that name first.  This is done to prevent the accidental loss of valuable data.
    You can prevent this issue by using **-Z** to delete the directory after a successful run.

.. _-T:

**-T**\ *nframes*\|\ *min*/*max*/*inc*\ [**+n**]\|\ *timefile*\ [**+p**\ *width*]\ [**+s**\ *first*]\ [**+w**\ [*str*]]
    Either specify how many image frames to make, create a one-column data set width values from
    *min* to *max* every *inc* (append **+n** if *inc* is number of frames instead), or supply a file with a set of parameters,
    one record (i.e., row) per frame.  The values in the columns will be available to the
    *mainscript* as named variables **MOVIE_COL0**, **MOVIE_COL1**, etc., while any trailing text
    can be accessed via the variable **MOVIE_TEXT**.  Append **+w** to split the trailing
    string into individual words that can be accessed via variables **MOVIE_WORD0**, **MOVIE_WORD1**,
    etc. By default we use any white-space to separate words.  Append *str* to select another character(s)
    as the valid separator(s).  The number of records equals
    the number of frames. Note that the *background* script is allowed to create *timefile*,
    hence we check for its existence both before *and* after the background script has completed.  Normally,
    the frame numbering starts at 0; you can change this by appending a different starting frame
    number via **+s**\ *first*.  **Note**: All frames are still included; this modifier only affects
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

**-E**\ *titlepage*\ [**+d**\ *duration*\ [**s**]][**+f**\ [**i**\|\ **o**]\ *fade*\ [**s**]]\ [**+g**\ *fill*]
    Give a *titlepage* script that creates a static title page for the movie [no title].
    Alternatively, *titlepage* can be a *PostScript* plot (file extension .ps) of dimensions exactly matching
    the canvas size set in **-C**. You control the duration of the title sequence with **+d** and specify
    the number of frames (or append **s** for a duration in seconds instead) [4s].
    Optionally, supply the fade length via **+f**\ *fade* (in frames or seconds [1s]) as well [no fading];
    Use **+fi** and/or **+fo** to specify one-sided fading or to give unequal fade intervals [Default is same
    duration for both]. The fading affects the beginning and end of the title page *duration*. We fade from and
    to black by default; append **+g**\ *fill* to use another terminal fade color.

.. _-F:

**-F**\ *format*\ [**+t**]\ [**+o**\ *options*]
    Set the format of the final video product.  Repeatable.  Choose either **mp4** (MPEG-4 movie) or
    **webm** (WebM movie).  You may optionally add additional FFmpeg encoding settings for this format
    via the **+o** modifier (in quotes if more than one word). If **none** is chosen then no PNGs will
    be created at all; this requires **-M**.  Choose **+t** to generate transparent PNG images [opaque].

.. _-G:

**-G**\ [*fill*]\ [**+p**\ *pen*] :ref:`(more ...) <-Gfill_attrib>`
    Set the canvas color or fill before plotting commences [none].
    Optionally, append **+p** to draw the canvas outline with *pen* [no outline].

.. _-H:

**-H**\ *factor*
    Given the finite dots-per-unit used to rasterize *PostScript* frames to PNGs, the quantizing of features
    to discrete pixel will lead to rounding.  Some of this is mitigated by the anti-aliasing settings.  However,
    changes from frame to frame is outside the control of the individual frame rasterization and we
    find that, in particular, moving text may appear jittery when seen in the final animation.  You can mitigate
    this effect by selecting a scale *factor* that, in effect, temporarily increases the effective dots-per-unit
    by *factor*, rasterizes the frame, then downsamples the image by the same factor at the end.  The larger
    the *factor*, the smoother the transitions.  Because processing time increases with *factor* we suggest you
    try values in the 2-5 range.  Note that images can also suffer from quantizing when the original data have
    much higher resolution than your final frame pixel dimensions.  The **-H** option may then be used to smooth the
    result to avoid aliasing [no downsampling].  This effect is called `subpixel <https://en.wikipedia.org/wiki/Subpixel_rendering>`_ rendering.

.. _-I:

**-I**\ *includefile*
    Insert the contents of *includefile* into the movie_init.sh script that is accessed by all movie scripts.
    This mechanism is used to add information (typically constant variable assignments) that the *mainscript*
    and any optional **-S** scripts rely on.

.. _-K:


**-K**\ [**+f**\ [**i**\|\ **o**]\ *fade*\ [**s**]]\ [**+g**\ *fill*]\ [**+p**] ]
    Add fading in and out for the main animation sequence [no fading]. Append
    the length of the fading in number of frames (or seconds by appending **s**) [1s].
    Use **+fi** and/or **+fo** to specify one-sided fading or to give unequal fade
    intervals [Default is same duration for both].  Normally, fading will be overlaid on the
    first and last *fade* frames of the main animation.  Append **+p** to *preserve*
    these frames by fading over only the first and last (repeated) animation frames instead.
    Append **i** or **o** to only preserve the frame involved during the fade in or fade out instead.
    We fade from and to black by default; append **+g**\ *fill* to use another terminal fade color.

.. _-L:

**-L**\ *labelinfo*
    Automatic labeling of individual frames.  Repeatable up to 32 labels.  Places the chosen label at the frame perimeter:
    **e** selects the elapsed time in seconds as the label; append **+s**\ *scale* to set the length
    in seconds of each frame [Default is 1/*framerate*],
    **s**\ *string* uses the fixed text *string* as the label,
    **f** selects the running frame number as the label, **p** selects the percentage of progress so far,
    **c**\ *col* uses the value in column
    number *col* of *timefile* as label (first column is 0), while **t**\ *col* uses word number
    *col* from the trailing text in *timefile* (first word is 0).  **Note**: If you use **-Lc**
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
    *background* can be a *PostScript* plot layer of dimensions exactly matching the canvas size.

.. _-Sf:

**-Sf**\ *foreground*
    The optional GMT modern mode *foreground* (written in the same scripting language as *mainscript*) can be
    used to make a static foreground plot that should be overlain on all frames.  Make sure the script uses the same
    positioning (i.e., **-X -Y**) as the main script so that the layers will stack correctly.  Alternatively,
    *foreground* can be a *PostScript* plot layer of dimensions exactly matching the canvas size.

.. _movie-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [*workdir*]
    By default, all temporary files and frame PNG file are created in the subdirectory *prefix* set via **-N**.
    You can override that selection by giving another *workdir* as a relative or full directory path. If no
    path is given then we create a working directory in the system temp folder named *prefix*.  The main benefit
    of a working directory is to avoid endless syncing by agents like DropBox or TimeMachine, or to avoid
    problems related to low space in the main directory.

.. _-Z:

**-Z**\ [**s**]
    Erase the entire *prefix* directory after assembling the final movie [Default leaves directory with all images;
    the temporary script files, parameter files, and layer *PostScript* files are all removed (but see **-Q**)].
    If your *mainscript* and all input scripts via **-E**, **-I**, and **-S** should be deleted as well then
    append **s**.

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
**MOVIE_WIDTH**\ : The width of the canvas (the full movie frame),
**MOVIE_HEIGHT**\ : The height of the canvas (the full movie frame),
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

Plotting Temporal Changes
-------------------------

A movie is not very interesting if nothing changes.  For the animation to change you need to have your *mainscript*
either access a *different* data set as the frame counter changes, or you need to plot only a varying *subset* of a data set,
showing only the part that should be displayed in each frame.  There are several strategies you can use to
accomplish these effects:

#. Your *timefile* passed to **-T** may have names of specific data files and you simply have your *mainscript*
   use the relevant **MOVIE_TEXT** or **MOVIE_WORD?** to access the frame-specific file name.
#. You have a single data table which includes absolute time for each record, and you wish to plot these *events* as time
   moves forward, yet not displaying events still in the future.  This effect is achieved via the module :doc:`events`.
#. You have a 3-D grid (or a stack of 2-D grids) and you want to interpolate along the axis perpendicular to the
   2-D slices (e.g., time, or it could be depth).  In this situation you will use the module :doc:`grdinterpolate`
   to have the *mainscript* obtain a slice for the correct time (this may be an interpolation between two different
   times or depths) and make the frame plot with this temporary grid file.
#. You may be creating data on the fly using :doc:`gmtmath` or :doc:`grdmath`, or perhaps processing data slightly
   differently per frame (using parameters in the *timefile*) and displaying these or the changes between frames.

Your Canvas
-----------

As you can see from **-C**, unless you specified a custom format you are given a canvas size that is either 24 x 13.5 cm (16:9)
or 24 x 18 cm (4:3).  If your :term:`PROJ_LENGTH_UNIT` setting is inch then the custom canvas sizes are just
slightly (1.6%) larger than the corresponding SI sizes (9.6 x 5.4" or 9.6 x 7.2"); this has no effect on the size of the movie
frames but allow us to use good sizes that work well with the *dpu* chosen.  You should compose your plots using
the given canvas size, and **movie** will make proper conversions of the canvas to image pixel dimensions. It is your responsibility
to use **-X -Y** to allow for suitable margins and any positioning of items on the canvas.  To minimize processing time it is
recommended that any static part of the movie be considered either a static background (to be made once by *background*) and/or
a static foreground (to be made once by *foreground*); **movie** will then assemble these layers per frame.  Also, any computation of
static data files to be used in the loop over frames can be produced by *background*.  Any data or variables that depend on the
frame number must be computed or set by *mainscript* or provided via the parameters as discussed above.  **Note**: Using
the variables **MOVIE_WIDTH** or **MOVIE_HIGHT** to set plot dimensions may lead to clipping against the canvas since these are also the
exact canvas dimensions.

External *PostScript* Layers
----------------------------

Instead of passing GMT modern scripts to **-S** you can alternatively provide the name of *PostScript*
plot layer files. Note that these must exactly match the canvas size.  As a simple example, if you are
making a HD movie using the US unit dimensions then a background pink layer would be created by::

    gmt basemap -R0/9.6/0/5.4 -Jx1i -B+gpink -X0 -Y0 --PS_MEDIA=9.6ix5.4i -ps background

Note the canvas selection via :term:`PS_MEDIA`, the matching region and projection, and
the zero location of the origin.

Basemap Frames
--------------

Some map projections will by default draw a *fancy* map frame; this feature is under the control of :term:`MAP_FRAME_TYPE`.
However, whether a *fancy* or *plain* frame is actually drawn also depends on the projection center *latitude*.
Thus, if your movie varies the projection center latitude by changing the view, you should set the frame
setting to *plain* as part of your setup.

Transparent Images
------------------

By default, **movie** will build *opaque* PNG images which can then be assembled into an animation.
The vast majority of movies is likely to be made that way. Use the **-F** option's experimental modifier
**+t** to make *transparent* PNG images.  Currently, very few video codecs support
transparency.  It is claimed both H.265 (HECV) and VP9 (Webm) offer this capability; we have only been
able to verify the latter by viewing a transparent webm movie in Chrome. Animated GIFs can be built from
transparent PNGs as well and here each additional frame accumulate in the final movie. Experts may create
transparent PNGs and create movies in professional tools that support a movie alpha channel.

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
access to the information in *movie_init* while the frame script in addition has access to the frame-specific
parameter file.  Using the **-Q** option will just produce these scripts which you can then examine.
**Note**: The *mainscript* is duplicated per frame and each copy is run simultaneously on all available cores.
Multi-treaded GMT modules will therefore be limited to a single core as well.

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
If no size is specified (**+w**) then we default to 5% of canvas width for the three circular indicators and
60% of the relevant canvas dimension for the linear indicators.

Title Sequence and Fading
-------------------------

.. figure:: /_images/GMT_title_fade.*
   :width: 500 px
   :align: center

   The fade-level (0 means black, 100 means normal visibility) for the complete movie, including
   an optional title sequence.

The complete movie may have an optional leading title sequence (**-E**) of a given *duration*. A short section
at the beginning and/or end of this duration may be designated to fade in/out via the designated fade
color [black].  The main animation sequence may also have an optional fade in and/or out section (**-K**). Here, you
can choose to fade on top of the animation or you can "freeze" the first and/or last animation frame and only fade over
those static images (via modifier **+p**) in order to preserve the whole animation sequence.

Examples
--------

To make an animated GIF movie based on the script globe.sh, which simply spins a globe using the
frame number to serve as the view longitude, using a custom square 600 by 600 pixel canvas and 360 frames,
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
the dimensions of the rasterized *PostScript*, and so on.  That information is hidden from the user;
the actual movie scripts that execute are derived from the user-provided scripts and supply
the extra machinery. The **movie** module automatically manages the parallel execution loop over all frames using
all available cores.

Longer Examples
---------------

To explore more elaborate movies, see the Animations examples under our :doc:`GMT Animations <animations>` or view
high-resolution movies on the GMT `Youtube <https://www.youtube.com/channel/UCo1drOh0OZPcB7S8TmIyf8Q/>`_ channel.

Other Movie Formats
-------------------

As configured, **movie** only offers the MP4 and WebM formats for movies.  The conversion is performed by the
tool `FFmpeg <https://www.ffmpeg.org/>`_, which has more codecs and processing options than there are children in China.
If you wish to run FFmpeg with other options, run **movie** with one of the two video formats.
At the end it will print the FFmpeg command used.  You can copy, paste, and modify this command to
select other codecs, bit-rates, and arguments.  You can also use the PNG sequence as input to tools such
as QuickTime Player, iMovie, MovieMaker, and other commercial programs to make a movie that way.

Remaking Movie with Existing PNG Frames
---------------------------------------

Perhaps you made your movie and then decided you want to change the frame rate or adjust something else in
how the movie is put together from all the still images.  If you kept all the frame images then
you do not have to rerun the whole render process.  Assuming you want a MP4 movie and that you
want to rerun just the ffmpeg command, here is an example::

    ffmpeg -loglevel warning -f image2 -framerate 24 -y -i "mydir/myimages_%04d.png" -vcodec libx264 -pix_fmt yuv420p mymovie.mp4

This command is also written out when movie performs this step.
For other movie formats you will need to consult the `FFmpeg <https://www.ffmpeg.org/>`_ documentation.
**Note**: On Windows, the percentage character is special (like the dollar sign under shells) so you will need to enter two (%%).

Manipulating Multiple Movies
----------------------------

If you are making a series of similar movies, you can use FFmpeg to paste and stitch them into a single movie.
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
:doc:`gmtmath`,
:doc:`grdinterpolate`,
:doc:`grdmath`,
:doc:`events`
