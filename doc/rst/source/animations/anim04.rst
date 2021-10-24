.. _anim04:

(4) Flying from NY to Miami at night
------------------------------------

Our next animation simulates what an imaginary satellite might see as it
passes in a great circle from New York to Miami at an altitude of 160
km during the night. We use the general perspective view projection with
:doc:`grdimage </grdimage>` and use
:doc:`project </project>` to create a great circle path
between the two cities, sampled every 1 km. The main part of the script
will make the HD-quality frames from different view points and add frame
numbers to each frame. As this animation generates 1768 frames we use
FFmpeg to turn the image sequence into a MPEG-4 movie. **Note**: At the moment,
:doc:`grdview </grdview>` cannot use general perspective view projection to
allow "fly-through" animations like Fledermaus; we hope to add this functionality
in a future version.

.. literalinclude:: /_verbatim/anim04.txt
   :language: bash


.. Need to include the following 0 px wide dummy image for video poster

.. only:: html

   .. image:: /_images/anim04.png
      :width: 0 px

.. raw:: html

   <div class="figure align-center">
     <video width="720" height="480" poster="../_images/anim04.png" controls>
       <source src="../_static/anim04.mp4" type="video/mp4">
       Your browser does not support the video tag.
     </video>
     <p class="caption">Satellite view at night.</p>
   </div>
