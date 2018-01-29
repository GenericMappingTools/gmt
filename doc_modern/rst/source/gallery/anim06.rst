.. _anim_06:

(6) Demonstrate aliasing by sampling a chirp
--------------------------------------------

We demonstrate how aliasing works by sampling a chirp
at a fixed interval of 0.5 seconds.  We try to reconstruct
the signal by using a cubic spline on the sampled values.
This works well for at the start but once we approach the
Nyquist frequency it breaks down badly.

.. literalinclude:: /_verbatim/anim_06.txt
   :language: bash

.. figure:: /_images/anim_06.*
   :width: 400 px
   :align: center

   The effect of aliasing when sampling a chirp.
