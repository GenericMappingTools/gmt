.. _anim13:

(13) Animating time-series (seismograms)
=========================================

Here we demonstrate how one can animate a time-series so that the line grows in length and the
pen thickness and color/intensity changes as it is being written.  The trick we use is to convert the
line into a series of densely-spaced points, using the dpi of the desired movie to determine the point
resolution.  These points are then animated like other points.  We use :doc:`/events` to preprocess
the three components of a seismogram from a 2020 earthquake in the Aleutians and then animate it using
real time.  The three components evolve over time in each subplot panel while the rest of the presentation
includes a source map and some metadata, plus a timer and progress indicator.

.. literalinclude:: /_verbatim/anim13.txt
   :language: bash

..  youtube:: S-kRGxwOGJw
    :width: 100%
