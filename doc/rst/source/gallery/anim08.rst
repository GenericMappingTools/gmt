.. _anim08:

(8) One year (2018) of Pacific seismicity events
-----------------------------------------------------

Here we wish to focus on how to plot events, i.e., symbols
that should be visible for a finite (or infinite) timespan
depending on what they represent.  We obtain one year (2018)
of magnitude 5 or higher seismicity and make a movie with a new frame every two days,
highlighting new quakes as they appear and letting them shrink and
darken a few days after the event.

.. literalinclude:: /_verbatim/anim08.txt
   :language: bash

.. Need to include the following 0 px wide dummy image for video poster

.. only:: html

   .. image:: /_images/anim08.png
      :width: 0 px

.. raw:: html

   <div class="figure align-center">
     <video width="720" height="480" poster="../_images/anim08.png" controls>
       <source src="../_static/anim08.mp4" type="video/mp4">
       Your browser does not support the video tag.
     </video>
     <p class="caption">One year (2018) of Pacific seismicity events.</p>
   </div>
