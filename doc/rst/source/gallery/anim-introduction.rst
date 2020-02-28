Introduction
=============

Here we will explore what is
involved in creating animations (i.e., movies). Of course, an animation
is nothing more than a series of individual images played back in an
orderly fashion. Here, these images will have been created with GMT.
A GMT movie is made with the :doc:`movie </movie>` module that takes care of all the
book-keeping of making a movie (advancing frame counters, converting each
plot to a raster image, assembling the images into a movie).  The user
is left to focus on the creation of a main frame script (that will have access
to special variables to know which frame it is as well as user-defined data)
and optional scripts that
prepares files for the movie, lays down a constant background plot, and
appends a constant foreground plot.  The :doc:`movie </movie>` module explains
the available options and gives simple examples.  Below are more advanced
movie examples.  You can generate anything from tiny animated gif files
for your PowerPoint or KeyNote presentations or a full-featured movie with
thousands of frames at HD or 4k resolution.  Note: Several of the movie
examples have been purposefully made simpler by selecting lower frame rates
and coarser grids so that the automatic building of the documentation (which
includes the animations) does not take excessive time.
