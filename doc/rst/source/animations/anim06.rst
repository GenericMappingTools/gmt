.. _anim06:

(6) Demonstrate aliasing by sampling a chirp
--------------------------------------------

We demonstrate how aliasing works by sampling a chirp
at a fixed interval of 0.5 seconds.  We try to reconstruct
the signal by using a cubic spline on the sampled values.
This works well for at the start but once we approach the
Nyquist frequency it breaks down badly.

.. literalinclude:: /_verbatim/anim06.txt
   :language: bash


.. Need to include the following 0 px wide dummy image for video poster

.. only:: html

   .. image:: /_images/anim06.png
      :width: 0 px

.. raw:: html

   <div class="figure align-center">
     <video width="720" height="480" poster="../_images/anim06.png" controls>
       <source src="../_static/anim06.mp4" type="video/mp4">
       Your browser does not support the video tag.
     </video>
     <p class="caption">Demonstrate aliasing by sampling a chirp.</p>
   </div>
