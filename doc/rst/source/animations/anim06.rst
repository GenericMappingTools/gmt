.. _anim06:

(6) Demonstrate aliasing by sampling a chirp
--------------------------------------------

We demonstrate aliasing by sampling a linear chirp signal and then try to reconstruct
the original signal using a cubic spline interpolator through the samples.  Ideally, we
should do this via the Shannon-Whittaker sinc function but alas not in GMT yet.  As the
frequency of the chirp increases we find it harder and harder to reconstruct a reasonable
representation of the original signal from the samples.  The morale is you need to sample
data as often as you are able to.  Here, we added a title slide visible for 6 seconds, then
fade out to the animation.  The scripts are a bit longer due to lots of little details.
The finished movie is available in our YouTube channel as well:
https://youtu.be/3vB53hoLsls
The movie took ~3 minutes to render on a 24-core MacPro 2013.

.. literalinclude:: /_verbatim/anim06.txt
   :language: bash

..  youtube:: 3vB53hoLsls
    :width: 100%
