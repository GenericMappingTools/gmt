.. _anim_02:

(2) Examining DEMs using variable illumination
----------------------------------------------

Our next animation uses a gridded topography for parts of Colorado (US);
the file is distributed with the tutorial examples. Here, we want to use
:doc:`grdimage </grdimage>` to generate a shaded-relief
image sequence in which we sweep the illumination azimuth around the
entire horizon. The resulting animation illustrates how changing the
illumination azimuth can bring out subtle features (or artifacts) in the
gridded data. The red arrow points in the direction of the illumination.

.. literalinclude:: /_verbatim/anim_02.txt
   :language: bash

As you can see, these sorts of animations are not terribly difficult to
put together, especially since our vantage point is fixed. In the next
example we will move the "camera" around and must therefore deal with
how to frame perspective views.

.. figure:: /_images/anim_02.*
   :width: 400 px
   :align: center

   Animation of a DEM using variable illumination.

