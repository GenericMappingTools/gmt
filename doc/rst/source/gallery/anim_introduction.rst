Introduction
=============

Here we will explore what is
involved in creating animations (i.e., movies). Of course, an animation
is nothing more than a series of individual images played back in an
orderly fashion. Here, these images will have been created with GMT.
To ensure a smooth transition from frame to frame we will be following
some general guidelines when writing our scripts. Since there is no
"movie" mode in GMT we must take care of all the book-keeping in our
script. Thus, animations may require a bit of planning and may use more
advanced scripting than the previous static examples. Note: This is a
new chapter introduced with the 4.4.0 version and should be considered
work in progress.

Most, if not all, animation scripts must deal with several specific
phases of movie making:

#. Define parameters that determine the dimension of the final movie.

#. Pre-calculate all variables, data tables, grids, or background map
   layers that are *independent* of your time variable.

#. Have a frame-number loop where each frame is created as a
   PostScript plot, then rasterized to a TIFF file of chosen
   dimension.

#. Convert the individual frames to a single movie of suitable format.

#. Clean up temporary files and eventually the individual frames.

We will discuss these phases in more detail before showing our first
example.

#. There are several coordinates that you need to consider when planning
   your movie. The first is the coordinates of your data, i.e., the
   *user coordinates*. As with all GMT plots you will transform those
   to the second set of *plot coordinates* in inches (or cm) by applying
   a suitable region and map projection. As before, you normally do this
   with a particular paper size in mind. When printed you get a
   high-resolution plot in monochrome or color. However, movies are not
   device-independent and you must finally consider the third set of
   *pixel coordinates* which specifies the resolution of the final
   movie. We control the frame size by selecting a suitable *dpi*
   setting that will scale your physical dimensions to the desired frame
   size in pixels. If you decide up front on a particular resolution
   (e.g., 480 by 320 pixels) then you should specify a paper size and
   *dpi* so that their product yields the desired pixel dimensions. For
   instance, here it might make sense to plan your plotting on a 4.8 by
   3.2 inch "paper" and use 100 *dpi* to convert it to pixels, but you
   are free to use any combination that multiplies to the desired
   dimensions. After deciding on frame size you need to consider how
   many frames your movie should have. This depends on lots of things
   such as how patient you are, how many frames per second you need and
   the time range of your animation. We recommend you use variables to
   specify the items that go into computing the number of frames so that
   you can easily test your script with a few frames before changing
   settings and running the full Hollywood production overnight.

#. Depending on what you want to display, there are usually many
   elements that do not change between frames. Examples include a
   coastline base map for background, an overlay of text legends,
   perhaps some variables that hold information that will be used during
   the movie, and possibly subsets of larger data sets. Since
   movie-making can take a long time if you are ambitious, it is best to
   compute or plot all the elements that can be done outside your main
   frame-loop rather than waste time doing the same thing over and over
   again. You are then ready for the main loop.

#. Initialize a frame counter to 0 and have a loop that continues until
   your frame counter equals the desired number of frames. You must use
   your frame counter to create a unique file name for each frame image
   so that the series of images can be lexically arranged. We recommend
   using the GMT shell function **gmt_set_framename** to format
   the frame counter with an adequate number of leading zeros; see our
   examples for details. The bulk of your main loop involves create the
   single PostScript plot for this particular frame (time). This can
   be trivial or a serious scripting exercise depending on what you want
   to show. We will give a few examples with increasing complexity. Once
   the PostScript plot is created you need to rasterize it; we
   recommend you use :doc:`psconvert </psconvert>` to
   generate a TIFF image at the agreed-upon resolution. We also
   recommend that you place all frame images in a sub-directory. You may
   increment your frame counter using **gmt_set_framenext**.

#. Once you have all your frames you are ready to combine them into an
   animation. There are two general approaches. (a) If your image
   sequence is not too long then you can convert the images into a
   single animated GIF file. This file can be included in PowerPoint
   presentations or placed on a web page and will play back as a movie
   by pausing the specified amount between frames, optionally repeating
   the entire sequence one or more times. (b) For more elaborate
   projects you will need to convert the frames into a proper movie
   format such as Quicktime, AVI, MPEG-2, MPEG-4, etc., etc. There are
   both free and commercial tools that can help with this conversion and
   they tend to be platform-specific. Most movie tools such as iMovie or
   MovieMaker can ingest still images and let you specify the frame
   duration. Under OS X we prefer to use Quicktime 7. [1]_ . Another choice is to use the free
   `ffmpeg <https://ffmpeg.org/>`_ encoder.  You will find yourself
   experimenting with compression settings and movie formats so that the
   final movie has the resolution and portability you require.

#. Finally, when all is done you should delete any temporary files
   created. However, since creating the frames may take a lot of time it
   is best to not automatically delete the frame sub directory. That way
   you can redo the frames-to-movie conversion with different settings
   until you are satisfied.

.. [1]
   While Quicktime 7 is free you must upgrade to QuickTime Pro (USD 30) to
   use the authoring functions.  At some point Apple will discontinue this tool.
