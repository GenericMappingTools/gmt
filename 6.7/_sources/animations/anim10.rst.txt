.. _anim10:

(10) The effect of sub-pixeling
===============================

GMT animations all start with designing plots that are created using the
PostScript language.  It is therefore vector graphics with no limitations
imposed by pixel resolutions.  However, to make an animation we must render
these PostScript plots into raster images (we use PNG) and a pixel resolution
enters.  Unlike printed media (laserwriters), the dots-per-unit in an animation
is much lower, and compromizes are made when vector graphics must be turned
into pixels.  GMT's movie module (and psconvert for still images) offers the
option of sub-pixeling.  It means the image is temporarily enlarged to have
more pixels than requested, then shrunk back down.  These steps tend to make
the lower-resolution images better than the raw rendering.  Here we show
the effect of different sub-pixel settings - notice how the movies with
little or no sub-pixeling "jitters" as time goes by.
The resulting movie was presented at the Fall 2019 AGU meeting in an eLighting talk:
P. Wessel, 2019, GMT science animations for the masses, Abstract IN21B-11.
The finished movie is available in our YouTube channel as well:
https://youtu.be/FLzYVo7wXAg
The movie took ~2 minutes to render on a 24-core MacPro 2013.
Demonstrate the effect of sub-pixeling

.. literalinclude:: /_verbatim/anim10.txt
   :language: bash

..  youtube:: FLzYVo7wXAg
    :width: 100%
