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
|-N|\ *prefix*
|-T|\ *frames*\ \|\ *timefile*
|-W|\ *papersize*
[ |-A|\ *rate*\ [**+l**\ [*n*]] ] 
[ |-E| ]
[ |-F|\ *format*\ [**+o**\ *options*\ ]]
[ |-G|\ *fill*\ ]
[ |-Q|\ [*frame*] ]
[ **Sb**\ *backgroundscript* ]
[ **Sf**\ *foregroundscript* ]
[ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

We can generate GMT animation sequences using a single-plot script that is repeated
for each frame, with some variation using specific frame bariables.  The **movie**
module simplifies and hides most of the steps normally needed to set up a full-blown
animation job.  Instead, the user can focus on composing the main plot and let the
parallel execution of frames and assembly of images into a movie take place in the background.

Required Arguments
------------------

*mainscript*
    Name of a stand-alone GMT modern script that makes the frame-dependent plot.  The
    script may access frame parameters, such as frame number and others, and may be
    written using the Bourne shell (.sh), the Bourne again shell (.bash), the csh (.csh)
    or DOS batch language (.bat).  The script language is inferred from the file extension
    and we build hidden scripts using the same language.  Parameters that can be accessed
    are discussed below.

.. _-N:

**-N**\ *prefix*
    Determines the name of a sub-directory with frame images as well as the final movie file.
    Note: If the subdirectory exist then we exit immediately, so make sure you remove any
    old directory first.  This is done to prevent the accidental loss of valuable data.

.. _-T:

**-T**\ *frames*\ \|\ *timefile*
    Either specify how many image frames to make or supply a file with a set of parameters,
    one record per frame (i.e., row).  The values in the columns will be available to the
    *mainscript* as named variables GMT_MOVIE_VAL1, GMT_MOVIE_VAL2, etc., while any trailing text
    can be accessed via the variable GMT_MOVIE_STRING.  The number of records equals the number of frames.
    Note that the *background* script is allowed to create the *timefile*.

.. _-W:

**-W**\ *papersize*
    Specify the custom paper size used to compose the movie frames. You can choose from a
    a set of known preset formats or you can set a custom layout.  The recognized 16:9 ratio
    formats (with paper size and pixel dimensions in parenthesis) are
    **2160p** (24 x 13.5 cm *or* 9.6 x 5.4 inch; 3840 x 2160),
    **1080p** (24 x 13.5 cm *or* 9.6 x 5.4 inch; 1920 x 1080),
    **720p** (24 x 13.5 cm *or* 9.6 x 5.4 inch; 1280 x 720), or
    **540p** (24 x 13.5 cm *or* 9.6 x 5.4 inch; 960 x 540).
    We also accept **4k** or **uhd** for **2160p** and **hd** for **1080p**.
    The recognized 4:3 ratio formats are
    **480p** (24 x 18 cm *or* 9.6 x 7.2 inch; 640 x 480) or
    **360p** (24 x 18 cm *or* 9.6 x 7.2 inch; 480 x 360).
    We also accept **dvd** for **480p**.
    Note: :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>` determines if you are expected to
    work with the SI or US dimensions.  Alternatively, set a custom format directly by
    giving *width*\ x\ *height*\ x\ *dpu* for a custom frame dimension, where *dpu* is
    the dots per unit pixel density.


Optional Arguments
------------------

.. _-A:

**-A**\ *rate*\ [**+l**\ [*n*]]
    Set the frame rate in frames per seconds for the final movie (if selected) [24].
    Optionally, for animated GIF movies you may control if the movie should loop (**+l**)
    and how many loops to set [infinite].

.. _-E:

**-E**
    Erase the *prefix* directory after assembling the final movie [leave directory with all images;
    script files, parameter files, and layer PostScript files are removed].

.. _-F:

**-F**\ *format* [**+o**\ *options*\ ]
    Set the movie format for the final product.  Choose either gif (animated GIF), mp4 (MPEG-4),
    or none [Default].  For mp4 you may optionally add additional ffmpeg encoding options via the **+o** modifier.

.. _-G:

**-G**\ *fill*
    Set the canvas fill before plotting commences [none].

.. _-Q:

**-Q**\ [*frame*]
    Dry-run; no movie is made but all the helper scripts are left in the *prefix* directory for examination.
    Any background and foreground scripts set via **-S** will be run since they may produce data needed for
    building the scripts.  Alternatively, append a *frame* number and we will make that single frame plot and
    exit.  In this case we will leave the frame sub-directory intact so any temporary files in it may be examined.

.. _-Sb:

**-Sb**\ *backgroundscript*
    The optional GMT modern mode *backgroundscript* (in the same scripting language as *mainscript*) can be
    used for one or two purposes: (1) It may create files (such as *timefile*) that will be needed by *mainscript*
    to make the movie, and (2) It may make a static background plot that should form the background for all frames.
    If a plot is generated it should make sure it uses the same positioning (i.e., **-X -Y**) as the main script
    so that they will stack correctly.

.. _-Sf:

**-Sf**\ *foregroundscript*
    The optional GMT modern mode *foregroundscript* (in the same scripting language as *mainscript*) can be
    used to make a static foreground plot that should be overlain on all frames.  Make sure it uses the same
    positioning (i.e., **-X -Y**) as the main script so that they will stack correctly.

.. _movie-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

Parameters
----------

Several parameters are automatically assigned and can be used by *mainscript* and the optional
*backgroundscript* and *foregroundscript* scripts in making the frame plot.
GMT_MOVIE_WIDTH: The width of the paper,
GMT_MOVIE_HEIGHT: The height of the paper,
GMT_MOVIE_DPU: The current dots-per-unit.
In addition, the *mainscript* also has access to additional parameters
GMT_MOVIE_FRAME: The current frame number,
GMT_MOVIE_NFRAMES: The total number of frames
Finally, if a *timefile* was given then variables GMT_MOVIE_VAL1, GMT_MOVIE_VAL2, etc are
also set, one per column in *timefile*.
Furthermore, the scripts will be able to find any files present in the starting directory
as well as any new files produced by *mainscript* and the optional scripts set via **-S**.

Examples
--------

To make an animated GIF movie based on the script globe.sh, which spins a globe using the
frame number to compute a view angle, using 360 frames and a custom square 600x600 image, try

   ::

    gmt movie globe.sh -Nglobe -T360 -Fgif -W6ix6ix100

Here, the globe.sh bash script is simply

   ::

    gmt begin
       gmt pscoast -Rg -JG${GMT_MOVIE_FRAME}/20/4i -Gmaroon -Sturquoise -P -Bg -X0 -Y0
    gmt end

where we use the frame number as our longitude.  The equivalent DOS script setup would be

  ::

    gmt movie globe.bat -Nglobe -T360 -Fgif -W6ix6ix100

Now, the globe.bat DOS script is simply

   ::

    gmt begin
       gmt pscoast -Rg -JG%GMT_MOVIE_FRAME%/20/4i -Gmaroon -Sturquoise -P -Bg -X0 -Y0
    gmt end

i.e., the syntax of how variables are used vary according to the scripting language.
Note that there is no information set here to reflect the name of the plot, the paper size,
the dimensions of the rasterized PostScript, and so on.  That is hidden from the user;
the actual scripts that execute are derived from the user-provided scripts and supply
the extra machinery.

Other Movie Formats
-------------------

As configured, movie only offers a MP4 format for movies.  The conversion is performed by the
tool ffmpeg (https://www.ffmpeg.org) which as more codecs and processing options than there are children in China.
If you wish to run ffmpeg with other selections, simply run movie with long verbose (**-Vl**) and
at the end it will print the ffmpeg command.  You can copy, paste, and modify this command to
select other codecs and bit-rates.  You can also use the PNG sequence as input to tools such
as QuickTime Pro, iMovie, MovieMaker, and similar commercial programs.

See Also
--------

:doc:`begin`,
:doc:`clear`,
:doc:`end`,
:doc:`figure`,
:doc:`subplot`,
:doc:`gmt`
