.. _anim_05:

(5) Control spline gridding via eigenvalues
-------------------------------------------

Our next animation performs gridding using cubic splines but
restricts the solution to using only the first *k* eigenvalues
of the 52 that are required for an exact interpolation of this
data set consisting of 52 points.  We use
:doc:`greenspline </greenspline>` to grid the data and select
an ever-increasing number of eigenvalues, then show a contour
map of the evolving surface.  The data misfits are indicated
by the colored circles; as we approach the full solution these
all become white (no misfit). These 52 frames are well suited
for an animated GIF.

.. literalinclude:: /_verbatim/anim_05.txt
   :language: bash

.. figure:: /_images/anim_05.*
   :width: 400 px
   :align: center

   Evolution of a splined grid.
