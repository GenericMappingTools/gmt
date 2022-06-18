.. _anim12:

(12) Image and Grid Manipulations
=================================

We again use :doc:`/grdmix`, blending the NASA day and night views from the Blue and Black Marble mosaic
images, but here we recompute a day-night mask for every 1 minute in a single day, for 1440
frames.  In addition, we adjust the colors using the intensities derived
from the slopes of the earth relief grid.  Since no GMT plot is made here we do not use :doc:`/movie`
but instead use :doc:`/batch` to build the images and then create the movie directly from the set
of PNG HD images.

.. literalinclude:: /_verbatim/anim12.txt
   :language: bash

..  youtube:: X8TojLs0NYk
    :width: 100%
