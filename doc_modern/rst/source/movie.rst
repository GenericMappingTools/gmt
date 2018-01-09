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
[ |-F|\ *format*\ ]
[ |-G|\ *fill*\ ]
[ |-Q|\ [*frame*] ]
[ **Sb**\ *backgroundscript* ]
[ **Sf**\ *foregroundscript* ]
[ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

We can generate GMT animation sequences using a single-plot script that are repeated
for each frame with some variation.  The **movie** module simplifies and hides many of
the steps normally needed to set up a full-blown animation script.  Instead, we can
focus on composing the main plot and let the loop and assembly of images into a movie
take place in the background.

Required Arguments
------------------

*mainscript*
    Name of a stand-alone GMT modern script that makes the frame-dependent plot.  The
    script may access frame parameters, such as frame number and others, and may be
    written using the Bourne shell (.sh), The Bourne again shell (.bash), the csh (.csh)
    or DOS batch scripts (.bat).  The script language is inferred from the file extension
    and we supply hidden scripts using the same language.

.. _-N:

**-N**\ *prefix*
    Determines the name of sub-directory with frame images as well as the final movie file.

.. _-T:

**-T**\ *frames*\ \|\ *timefile*
    Either specify now many image frames to make or supply a file with a set of parameters,
    one record per row (i.e., frame).  The values in the columns will be available to the
    *mainscript* as variables GMT_MOVIE_VAL1, GMT_MOVIE_VAL2, etc., while any trailing text
    can be accessed as GMT_MOVIE_STRING.  The number of records sets the number of frames.
    Note that the *background* script is allowed to create the *timefile*.

.. _-W:

**-W**\ *papersize*
    Specify the custom paper size used to compose the movie frames. You can choose from a
    a set of known preset formats or you can set a custom layout.  The recognized 16:9 ratio
    formats (with paper size and pixel dimensions in parenthesis) are
    **4k** (24 x 13.5 cm *or* 9.6 x 5.4 inch; 3840 x 2160),
    **1080p** (24 x 13.5 cm *or* 9.6 x 5.4 inch; 1920 x 1080),
    **720p** (24 x 13.5 cm *or* 9.6 x 5.4 inch; 1280 x 720), or
    **540p** (24 x 13.5 cm *or* 9.6 x 5.4 inch; 960 x 540).
    The recognized 4:3 ratio formats are
    **480p** (24 x 18 cm *or* 9.6 x 7.2 inch; 640 x 480) or
    **360p** (24 x 18 cm *or* 9.6 x 7.2 inch; 480 x 360).
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

**-F**\ *format*
    Set the movie format for the final product.  Choose either gif (animated GIF) or mp4 (MPEG-4 video) [none].

.. _-G:

**-G**\ *fill*
    Set the canvas fill before plotting commences [none].

.. _-Q:

**-Q**\ [*frame*]
    Dry-run; no movie is made but all the helper scripts are left in the *prefix* directory for examination.
    Alternatively, append a *frame* number and we will make a single frame plot and exit.

.. _-Sb:

**-Sb**\ *backgroundscript*
    The optional GMT modern mode *backgroundscript* (in the same scripting language as *mainscript*) can be
    used for one or two purposes: (1) It may create files (such as *timefile*) that will be needed by *mainscript*
    to make the movie, and (2) It may make a static background plot that should form the basis for all frames.
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

Several parameters are automatically assigned and can be used by *mainscript* in making the frame plot.
GMT_MOVIE_FRAME: The current frame number,
GMT_MOVIE_NFRAMES: The total number of frames,
GMT_MOVIE_WIDTH: The width of the paper,
GMT_MOVIE_HEIGHT: The height of the paper,
GMT_MOVIE_DPU: The current dots-per-unit.

Furthermore, the scripts will be able to find any files in the starting directory as well as files produced
by *mainscript* and the optional scripts set via **-S**.

Examples
--------

To make an animated GIF movie based on the script globe.sh, which spins a globe using the
frame number to compute a view angle, using 360 frames and a custom square 600x600 image, try

   ::

    gmt movie globe.sh -Nglobe -T360 -Fgif -W6ix6ix100

See Also
--------

:doc:`begin`,
:doc:`clear`,
:doc:`end`,
:doc:`figure`,
:doc:`subplot`,
:doc:`gmt`
