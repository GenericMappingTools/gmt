.. _anim01:

(1) Animation of the sine function
----------------------------------

Our first animation is not very ambitious: We wish to plot the sine
function from 0-360 and take snap shots every 10. To get a smooth curve
we must sample the function much more frequently; we settle on 10 times
more frequently than the frame spacing. We place a bright red circle at
the leading edge of the curve, and as we move forward in time (here,
angles) we dim the older circles to a dark red color. We add a label
that indicates the current angle value. Once the 36 frames are completed
we convert them to a single mp4 file.

.. literalinclude:: /_verbatim/anim01.txt
   :language: bash

Make sure you understand the purpose of all the steps in our script. In
this case we did some trial-and-error to determine the exact values to
use for the map projection, the region, the spacing around the frame,
etc. so that the final result gave a reasonable layout. Do this planning
on a single PostScript plot before running a lengthy animation script.

..  youtube:: 5m3gRhFFFLA
    :width: 100%
